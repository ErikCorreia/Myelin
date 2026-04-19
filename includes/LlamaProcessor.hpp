#ifndef LLAMA_PROCESSOR_HPP
#define LLAMA_PROCESSOR_HPP

#include "llama.h"
#include <string>
#include <vector>
#include <functional>

namespace Myelin::Core
{
    using TokenCallback = std::function<void(const std::string &)>;

    class LlamaProcessor
    {
    public:
        LlamaProcessor(const std::string &model_path, int n_ctx, int n_batch, int n_threads);
        ~LlamaProcessor();

        std::vector<llama_token> tokenize(const std::string &text, bool add_special);
        bool decodeBatch(const std::vector<llama_token> &tokens, int &n_past, int n_batch_limit);
        std::string streamResponse(int &n_past, int max_tokens, TokenCallback onTokenFound);

        const struct llama_vocab *get_vocab() const { return vocab; }

    private:
        llama_model *model;
        llama_context *ctx;
        const struct llama_vocab *vocab;
    };
}
#endif