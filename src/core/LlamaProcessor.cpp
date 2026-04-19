#include "LlamaProcessor.hpp"
#include <stdexcept>

namespace Myelin::Core
{
    LlamaProcessor::LlamaProcessor(const std::string &model_path, int n_ctx, int n_batch, int n_threads)
    {
        llama_backend_init();
        auto m_params = llama_model_default_params();
        model = llama_model_load_from_file(model_path.c_str(), m_params);
        if (!model)
            throw std::runtime_error("Erro ao carregar modelo.");

        auto c_params = llama_context_default_params();
        c_params.n_ctx = n_ctx;
        c_params.n_batch = n_batch;
        c_params.n_threads = n_threads;
        ctx = llama_init_from_model(model, c_params);
        vocab = llama_model_get_vocab(model);
    }

    LlamaProcessor::~LlamaProcessor()
    {
        if (ctx)
            llama_free(ctx);
        if (model)
            llama_model_free(model);
        llama_backend_free();
    }

    std::vector<llama_token> LlamaProcessor::tokenize(const std::string &text, bool add_special)
    {
        std::vector<llama_token> tokens(text.length() + 32);
        int n = llama_tokenize(vocab, text.c_str(), text.length(), tokens.data(), tokens.size(), add_special, true);
        if (n < 0)
        {
            tokens.resize(-n);
            n = llama_tokenize(vocab, text.c_str(), text.length(), tokens.data(), tokens.size(), add_special, true);
        }
        tokens.resize(n);
        return tokens;
    }

    bool LlamaProcessor::decodeBatch(const std::vector<llama_token> &tokens, int &n_past, int n_batch_limit)
    {
        for (int i = 0; i < (int)tokens.size(); i += n_batch_limit)
        {
            int n_eval = std::min((int)tokens.size() - i, n_batch_limit);
            llama_batch batch = llama_batch_init(n_eval, 0, 1);
            batch.n_tokens = n_eval;

            for (int j = 0; j < n_eval; j++)
            {
                batch.token[j] = tokens[i + j];
                batch.pos[j] = n_past + j;
                batch.n_seq_id[j] = 1;
                batch.seq_id[j][0] = 0;
                batch.logits[j] = (j == n_eval - 1);
            }

            if (llama_decode(ctx, batch) != 0)
            {
                llama_batch_free(batch);
                return false;
            }
            n_past += n_eval;
            llama_batch_free(batch);
        }
        return true;
    }

    std::string LlamaProcessor::streamResponse(int &n_past, int max_tokens, TokenCallback onTokenFound)
    {
        struct llama_sampler *smpl = llama_sampler_init_greedy();
        std::string full_text = "";

        for (int i = 0; i < max_tokens; i++)
        {
            llama_token curr = llama_sampler_sample(smpl, ctx, -1);
            if (llama_vocab_is_eog(vocab, curr))
                break;

            char buf[256];
            int n = llama_token_to_piece(vocab, curr, buf, sizeof(buf), 0, true);
            if (n > 0)
            {
                std::string piece(buf, n);
                full_text += piece;
                if (onTokenFound)
                    onTokenFound(piece);
            }

            llama_batch next = llama_batch_init(1, 0, 1);
            next.n_tokens = 1;
            next.token[0] = curr;
            next.pos[0] = n_past++;
            next.n_seq_id[0] = 1;
            next.seq_id[0][0] = 0;
            next.logits[0] = true;

            if (llama_decode(ctx, next) != 0)
            {
                llama_batch_free(next);
                break;
            }
            llama_batch_free(next);
        }
        llama_sampler_free(smpl);
        return full_text;
    }
}