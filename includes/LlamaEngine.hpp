#ifndef LLAMA_ENGINE_HPP
    #define LLAMA_ENGINE_HPP
    
    #include "llama.h"
    #include "ChatHistory.hpp"
    #include "MemoryManager.hpp"
    #include <string>
    #include <vector>

    namespace Myelin::Core {
        struct EngineConfig {
            int n_batch = 2048;
            int n_ctx = 2048;
            int n_threads = 6;
            int n_threads_batch = 12;
            int max_tokens = 200;
            std::string model_path = "models";
            std::string instructions_path = "instructions";
        };
    
        class LlamaEngine {
            public:
                LlamaEngine(const std::string& model_path, const EngineConfig& cfg, Myelin::IO::MemoryManager& shared_history);
                ~LlamaEngine();
    
                void generateResponse(const std::string& user_input);
            private:
                int n_past = 0;
                llama_model* model;
                llama_context* ctx;
                // Myelin::Core::ChatHistory history;
                Myelin::IO::MemoryManager memory;
                const struct llama_vocab* vocab;
                EngineConfig cfg;
    
                std::string load_system_prompt(const std::string& filename);
    
                void setup_params();
        };
    }

#endif