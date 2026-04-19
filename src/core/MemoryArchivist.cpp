#include "MemoryArchivist.hpp"
#include "MemoryEvaluator.hpp"
#include <future>

namespace Myelin::Core
{
    MemoryArchivist::MemoryArchivist(Myelin::IO::DatabaseManager &db, EmbeddingEngine &emb) : db(db), emb_engine(emb) {}
    std::string MemoryArchivist::retrieve(const std::string &input, std::vector<float> &out_user_emb)
    {
        // Bloqueia o acesso ao motor de embedding para evitar Race Condition
        {
            std::lock_guard<std::mutex> lock(this->engine_mutex);
            out_user_emb = emb_engine.get_embedding(input);
        }

        if (out_user_emb.empty())
            return "";

        // 0.38 para ser mais "inteligente" na busca
        std::string semantic = db.search_semantic_context(out_user_emb, 0.38f, 5);

        // Busca por mensagens recentes
        std::string recent = db.get_recent_chats(5);

        return "--- MENSAGENS RECENTES ---\n" + recent +
               "\n--- MEMÓRIAS RELEVANTES ---\n" + semantic;
    }

    void MemoryArchivist::store(const std::string &user_in, const std::string &ai_out, const std::vector<float> &user_emb)
    {
        if (!user_in.empty())
            db.add_message("user", user_in, user_emb);
        if (!ai_out.empty())
        {
            std::vector<float> ai_emb = emb_engine.get_embedding(ai_out);
            db.add_message("assistant", ai_out, ai_emb);
        }
    }

    void MemoryArchivist::updateLastResponseScore(const std::string &last_ai_res, const std::string &current_user_in)
    {
        if (last_ai_res.empty())
            return;

        evaluation_task = std::async(std::launch::async, [this, last_ai_res, current_user_in]()
                                     {
        // Bloqueia o acesso ao motor enquanto a thread de avaliação trabalha
        std::lock_guard<std::mutex> lock(this->engine_mutex);
        
        int text_score = MemoryEvaluator::evaluate(last_ai_res, current_user_in, this->emb_engine);

        int final_score = text_score;

        std::cout << ">>> GRAVANDO NO BANCO: " << final_score << std::endl;
        db.update_last_ai_score(final_score); });
    }
}