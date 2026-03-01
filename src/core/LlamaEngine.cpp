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
    LlamaEngine::LlamaEngine(const std::string &model_path, const EngineConfig &config, Myelin::IO::DatabaseManager &shared_db, EmbeddingEngine &emb_engine) : cfg(config), db(shared_db), emb_engine(emb_engine), n_past(0)
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

        // --- RAG: Busca semântica antes de montar o prompt ---
        std::vector<float> query_vector = emb_engine.get_embedding(user_input);
        std::string semantic_context = "";

        if (!query_vector.empty()) {
            semantic_context = db.search_semantic_context(query_vector, 0.70f, 3);
        }

        if (is_first_run) {
            std::string system_instr = Myelin::IO::InstructionLoader::load_from_folder(cfg.instructions_path);

            prompt = "<|start_header_id|>system<|end_header_id|>\n\n" + system_instr;

            if (!semantic_context.empty()) {
                prompt += "\n\n### MEMÓRIAS RECUPERADAS (ORDEM DE RELEVÂNCIA) ###\n";
                prompt += "Use as datas abaixo para entender o contexto temporal das informações:\n";
                prompt += semantic_context;
                prompt += "\n### FIM DAS MEMÓRIAS ###\n";
            }

            prompt += "<|eot_id|><|start_header_id|>user<|end_header_id|>\n\n" + user_input + "<|eot_id|><|start_header_id|>assistant<|end_header_id|>\n\n";
        } else {
            prompt = "<|start_header_id|>user<|end_header_id|>\n\n" + user_input + "<|eot_id|><|start_header_id|>assistant<|end_header_id|>\n\n";
        }

        std::vector<llama_token> tokens(prompt.length() + 32);
        int n_tokens = llama_tokenize(vocab, prompt.c_str(), (int)prompt.length(), tokens.data(), (int)tokens.size(), is_first_run, true);
        if (n_tokens < 0)
        {
            tokens.resize(-n_tokens);
            n_tokens = llama_tokenize(vocab, prompt.c_str(), (int)prompt.length(), tokens.data(), (int)tokens.size(), is_first_run, true);
        }
        tokens.resize(n_tokens);

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
                batch.logits[j] = (j == n_eval - 1); // Logits apenas no último do batch
            }

            if (llama_decode(ctx, batch) != 0)
            {
                Myelin::IO::Logger::log(Myelin::IO::Logger::ERR, "Falha no decode.");
                llama_batch_free(batch);
                return;
            }
            n_past += n_eval;
            llama_batch_free(batch);
        }

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

        // --- SALVAMENTO PROTEGIDO ---
        // 1. Salva User Input (já geramos o vetor no início do RAG)
       auto trim = [](std::string s) {
            s.erase(0, s.find_first_not_of(" \n\r\t"));
            s.erase(s.find_last_not_of(" \n\r\t") + 1);
            return s;
        };

        std::string clean_user = trim(user_input);
        std::string clean_ai = trim(full_response);

        if (!clean_user.empty()) {
            db.add_message("user", clean_user, query_vector);
        }

        if (!clean_ai.empty()) {
            std::vector<float> ai_emb = emb_engine.get_embedding(clean_ai);
            db.add_message("assistant", clean_ai, ai_emb);
        }

        // std::vector<float> user_emb = emb_engine.get_embedding(user_input);
        // std::cout << "[Debug] Tamanho do embedding gerado: " << user_emb.size() << std::endl;
        // db.add_message("user", user_input, user_emb);

        llama_sampler_free(smpl);
        std::cout << std::endl;
    }
}