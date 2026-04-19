#include "MemoryArchivist.hpp"

namespace Myelin::Core
{
    MemoryArchivist::MemoryArchivist(Myelin::IO::DatabaseManager &db, EmbeddingEngine &emb) : db(db), emb_engine(emb) {}

    std::string MemoryArchivist::retrieve(const std::string &input, std::vector<float> &out_user_emb)
    {
        out_user_emb = emb_engine.get_embedding(input);
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
}