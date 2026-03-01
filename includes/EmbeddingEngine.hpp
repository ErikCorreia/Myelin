#ifndef EMBEDDING_ENGINE_HPP
    #define EMBEDDING_ENGINE_HPP

    #include "llama.h"
    #include <string>
    #include <vector>

    namespace Myelin::Core {
        class EmbeddingEngine {
            private:
                llama_model*    model;
                llama_context*  ctx;
                llama_vocab*    vocab;

            public:
                EmbeddingEngine(const std::string& model_path);
                ~EmbeddingEngine();

                /**
                 * Transformar texto em vetores
                 */
                std::vector<float> get_embedding(const std::string& text);
        };   
    }
#endif