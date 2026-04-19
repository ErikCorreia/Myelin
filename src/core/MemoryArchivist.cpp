#include "MemoryArchivist.hpp"
#include "MemoryEvaluator.hpp"
#include "Utils.hpp"
#include <future>

namespace Myelin::Core
{
    MemoryArchivist::MemoryArchivist(Myelin::IO::DatabaseManager &db, EmbeddingEngine &emb) : db(db), emb_engine(emb) {}

    int MemoryArchivist::sessionVerify(const int &current_session_id)
    {
        if (current_session_id <= 0)
        {
            return db.create_session("Conversa em " + Myelin::Utils::get_timestamp());
        }

        return current_session_id;
    }

    std::string MemoryArchivist::retrieve(const std::string &input, std::vector<float> &out_user_emb, int session_id)
    {
        // Bloqueia o acesso ao motor de embedding para evitar Race Condition
        {
            std::lock_guard<std::mutex> lock(this->engine_mutex);
            out_user_emb = emb_engine.get_embedding(input);
        }

        if (out_user_emb.empty())
            return "";

        std::string global = db.search_semantic_global(out_user_emb, 0.35f, 5, session_id);

        std::string current = db.get_session_chats(session_id, 6);

        std::string context;
        if (!global.empty())
        {
            context += "### MEMÓRIAS DE SESSÕES ANTERIORES (FATOS CONHECIDOS) ###\n" + global + "\n";
        }
        if (!current.empty())
        {
            context += "### DIÁLOGO DA SESSÃO ATUAL ###\n" + current;
        }

        return context;
    }

    void MemoryArchivist::store(int session_id, const std::string &user_in, const std::string &ai_out, const std::vector<float> &user_emb)
    {
        if (!user_in.empty())
            db.add_message(session_id, "user", user_in, user_emb);

        if (!ai_out.empty())
        {
            std::vector<float> ai_emb = emb_engine.get_embedding(ai_out);
            db.add_message(session_id, "assistant", ai_out, ai_emb);
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