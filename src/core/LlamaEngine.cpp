#include <iostream>
#include <fstream>
#include <sstream>
#include <filesystem>

#include "LlamaEngine.hpp"
#include "InstructionLoader.hpp"
#include "Logger.hpp"

namespace fs = std::filesystem;

namespace Myelin::Core
{
    // LlamaEngine::LlamaEngine(const std::string &model_path, const EngineConfig& config, ChatHistory& shared_history) : cfg(config), history(shared_history), n_past(0)
    LlamaEngine::LlamaEngine(const std::string& model_path, const EngineConfig& config, Myelin::IO::MemoryManager& shared_memory) : cfg(config), memory(shared_memory), n_past(0)
    {
        llama_backend_init();

        auto m_params = llama_model_default_params();

        model = llama_model_load_from_file(model_path.c_str(), m_params);

        if (!model)
        {
            throw std::runtime_error("Falha ao carregar modelo.");
        }

        auto c_params = llama_context_default_params();
        c_params.n_batch = cfg.n_batch;
        c_params.n_ctx = cfg.n_ctx;
        c_params.n_threads = cfg.n_threads;
        c_params.n_threads_batch = cfg.n_threads_batch;

        ctx = llama_init_from_model(model, c_params);
        vocab = llama_model_get_vocab(model);
    }

    LlamaEngine::~LlamaEngine()
    {
        if (ctx)
            llama_free(ctx);
        if (model)
            llama_model_free(model);
        llama_backend_free();
    };

    void LlamaEngine::generateResponse(const std::string &user_input)
    {
        auto start_time = std::chrono::high_resolution_clock::now();
        Myelin::IO::Logger::log(Myelin::IO::Logger::INFO, "Myelin: Processando entrada...");

        std::string prompt;
        bool is_first_run = (n_past == 0);

        if (is_first_run)
        {
            std::string system_instructions = Myelin::IO::InstructionLoader::load_from_folder(cfg.instructions_path);
            Myelin::IO::Logger::log(Myelin::IO::Logger::INFO, "Instruções carregadas. Tamanho em caracteres: " + std::to_string(system_instructions.length()));

            prompt = memory.get_full_prompt(system_instructions);

            prompt += "<|start_header_id|>user<|end_header_id|>\n\n" + user_input + "<|eot_id|><|start_header_id|>assistant<|end_header_id|>\n\n";
        }
        else
        {
            prompt = "<|start_header_id|>user<|end_header_id|>\n\n" + user_input + "<|eot_id|><|start_header_id|>assistant<|end_header_id|>\n\n";
        }

        // Tokenização
        std::vector<llama_token> tokens(prompt.length() + 32);
        int n_tokens = llama_tokenize(vocab, prompt.c_str(), (int)prompt.length(), tokens.data(), (int)tokens.size(), false, true);
        if (n_tokens < 0)
        {
            tokens.resize(-n_tokens);
            n_tokens = llama_tokenize(vocab, prompt.c_str(), (int)prompt.length(), tokens.data(), (int)tokens.size(), false, true);
        }
        tokens.resize(n_tokens);

        // Decode do Prompt
        for (int i = 0; i < (int)tokens.size(); i += cfg.n_batch)
        {
            int n_eval = std::min((int)tokens.size() - i, cfg.n_batch);
            llama_batch batch = llama_batch_init(n_eval, 0, 1);
            batch.n_tokens = n_eval;

            for (int j = 0; j < n_eval; j++)
            {
                batch.token[j] = tokens[i + j];
                batch.pos[j] = n_past + j;
                batch.n_seq_id[j] = 1;
                batch.seq_id[j][0] = 0;
                batch.logits[j] = false;
            }

            Myelin::IO::Logger::log(Myelin::IO::Logger::INFO, "Total de tokens integrados ao KV Cache: " + std::to_string(n_past));

            // Ativa logit apenas no último token do batch para o sampler
            batch.logits[n_eval - 1] = true;

            if (llama_decode(ctx, batch) != 0)
            {
                Myelin::IO::Logger::log(Myelin::IO::Logger::ERR, "Falha no decode.");
                llama_batch_free(batch);
                return;
            }
            n_past += n_eval;
            llama_batch_free(batch);
        }

        auto end_prompt = std::chrono::high_resolution_clock::now();
        Myelin::IO::Logger::log(Myelin::IO::Logger::INFO, "Prompt OK (" + std::to_string(std::chrono::duration<double>(end_prompt - start_time).count()) + "s)");

        // Geração
        struct llama_sampler *smpl = llama_sampler_init_greedy();
        std::string full_response = "";
        std::cout << "Myelin: ";

        for (int i = 0; i < cfg.max_tokens; i++)
        {
            llama_token curr = llama_sampler_sample(smpl, ctx, -1);
            if (llama_vocab_is_eog(vocab, curr))
                break;

            char buf[256];
            int n = llama_token_to_piece(vocab, curr, buf, sizeof(buf), 0, true);
            if (n > 0)
            {
                std::string piece(buf, n);
                std::cout << piece << std::flush;
                full_response += piece;
            }

            // --- DECODE MANUAL DO NOVO TOKEN (Evita Crash) ---
            llama_batch next_batch = llama_batch_init(1, 0, 1);
            next_batch.n_tokens = 1;
            next_batch.token[0] = curr;
            next_batch.pos[0] = n_past;
            next_batch.n_seq_id[0] = 1;
            next_batch.seq_id[0][0] = 0;
            next_batch.logits[0] = true;

            if (llama_decode(ctx, next_batch) != 0)
            {
                llama_batch_free(next_batch);
                break;
            }
            n_past++;
            llama_batch_free(next_batch);
        }

        memory.add_message("user", user_input);
        memory.add_message("assistant", full_response);
        llama_sampler_free(smpl);
        std::cout << std::endl;
    }
}