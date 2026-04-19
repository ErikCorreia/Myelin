#ifndef MYELIN_ENGINE_HPP
#define MYELIN_ENGINE_HPP

#include "LlamaProcessor.hpp"
#include "ContextManager.hpp"
#include "MemoryArchivist.hpp"

#include <string>
#include <vector>
#include <functional>

namespace Myelin::Core
{
    using TokenCallback = std::function<void(const std::string &)>;

    struct EngineConfig
    {
        int n_batch = 2048;
        int n_ctx = 2048;
        int n_threads = 6;
        int n_threads_batch = 12;
        int max_tokens = 200;
        std::string model_path = "models";
        std::string instructions_path = "instructions";
    };

    class MyelinEngine
    {
    public:
        MyelinEngine(const std::string &model_path, const EngineConfig &config, ::Myelin::IO::DatabaseManager &db, EmbeddingEngine &emb);
        void generateResponse(const std::string &user_input, TokenCallback onTokenFound);

    private:
        int n_past = 0;
        EngineConfig cfg;
        LlamaProcessor processor;
        ContextManager contextManager;
        MemoryArchivist archivist;
    };
}
#endif
