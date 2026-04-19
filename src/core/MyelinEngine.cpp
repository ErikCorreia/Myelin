#include "MyelinEngine.hpp"
#include "InstructionLoader.hpp"
#include "MyelinEngine.hpp"
#include "Logger.hpp"

#include <iostream>

namespace Myelin::Core
{
    MyelinEngine::MyelinEngine(const std::string &model_path, const EngineConfig &config, Myelin::IO::DatabaseManager &db, EmbeddingEngine &emb)
        : cfg(config),
          processor(model_path, config.n_ctx, config.n_batch, config.n_threads),
          contextManager(config.instructions_path),
          archivist(db, emb) {}

    void MyelinEngine::generateResponse(const std::string &user_input, TokenCallback onTokenFound)
    {
        Myelin::IO::Logger::log(Myelin::IO::Logger::INFO, "Myelin: Orquestrando resposta...");

        /**
         * Memória e RAG
         * Recuperacao de interacoes anteriores para montar o contexto da solicitacao
         */
        std::vector<float> user_emb;
        std::string memories = archivist.retrieve(user_input, user_emb);

        /**
         * Montagem do Prompt
         * Monta a estrutura prompt antes de enviar para inferencia
         */
        std::string prompt = contextManager.assemble(user_input, memories, n_past);

        /**
         * Inferência
         * Processa a entrada do usuario
         */
        std::vector<llama_token> tokens = processor.tokenize(prompt, n_past == 0);
        if (!processor.decodeBatch(tokens, n_past, cfg.n_batch))
            return;

        /**
         * Streaming
         */
        std::string ai_response = processor.streamResponse(n_past, cfg.max_tokens, onTokenFound);

        /**
         * Camada de persistencia (Memoria)
         * Registra entrada e saida + vetor
         */
        archivist.store(user_input, ai_response, user_emb);
    }
}
