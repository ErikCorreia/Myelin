#include <stdexcept>
#include <iostream>
#include "Logger.hpp"
#include "EmbeddingEngine.hpp"

using namespace Myelin::IO;

namespace Myelin::Core
{

    EmbeddingEngine::EmbeddingEngine(const std::string &model_path)
    {
        llama_log_set([](ggml_log_level level, const char * text, void * user_data) {
           Logger::log(Logger::WARN, std::to_string(level) + " | " + text);
        }, nullptr);

        auto m_params = llama_model_default_params();
        model = llama_model_load_from_file(model_path.c_str(), m_params);

        if (!model)
        {
            throw std::runtime_error("Falha ao carregar modelo");
            Logger::log(Logger::ERR, "Falha ao carregar modelo gerador de embedding");
        }

        auto c_params = llama_context_default_params();
        c_params.embeddings = true;
        c_params.n_ctx = 512;

        ctx = llama_init_from_model(model, c_params);
        vocab = (llama_vocab *)llama_model_get_vocab(model);
    }

    std::vector<float> EmbeddingEngine::get_embedding(const std::string &text)
    {
        if (text.empty())
            return {};

        std::vector<llama_token> tokens(text.length() + 32);
        int n_tokens = llama_tokenize(vocab, text.c_str(), text.length(), tokens.data(), tokens.size(), true, false);
        if (n_tokens <= 0)
            return {};
        tokens.resize(n_tokens);

        llama_batch batch = llama_batch_init(n_tokens, 0, 1);
        batch.n_tokens = n_tokens;

        for (int i = 0; i < n_tokens; i++)
        {
            batch.token[i] = tokens[i];
            batch.pos[i] = i;
            batch.n_seq_id[i] = 1;
            batch.seq_id[i][0] = 0;
            batch.logits[i] = true;
        }

        if (llama_decode(ctx, batch) != 0)
        {
            llama_batch_free(batch);
            return {};
        }

        int n_embd = llama_model_n_embd(model);
        const float *embd = llama_get_embeddings_seq(ctx, 0);

        if (embd == nullptr) {
            embd = llama_get_embeddings_ith(ctx, n_tokens - 1);
        }

        std::vector<float> result;
        if (embd)
        {
            result.assign(embd, embd + n_embd);
        }

        llama_batch_free(batch);
        return result;
    }

    EmbeddingEngine::~EmbeddingEngine()
    {
        if (ctx)
            llama_free(ctx);
        if (model)
            llama_model_free(model);
    }

}