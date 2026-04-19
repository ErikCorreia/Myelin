#include "MyelinEngine.hpp"
#include "MyelinEngine.hpp"
#include "Logger.hpp"

#include <future>
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

        const int current_session_id = archivist.sessionVerify(current_session_id);

        archivist.updateLastResponseScore(last_response, user_input);

        /**
         * Memória e RAG
         * Recuperacao de interacoes anteriores para montar o contexto da solicitacao
         */
        std::vector<float> user_emb;
        std::string context = archivist.retrieve(user_input, user_emb, current_session_id);

        /**
         * Montagem do Prompt
         * Monta a estrutura prompt antes de enviar para inferencia
         */
        std::string prompt = contextManager.assemble(user_input, context, n_past);

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
        archivist.store(current_session_id, user_input, ai_response, user_emb);

        last_response = ai_response;
    }
}
