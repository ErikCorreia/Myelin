#ifndef MEMORY_ARCHIVIST_HPP
#define MEMORY_ARCHIVIST_HPP

#include "DatabaseManager.hpp"
#include "EmbeddingEngine.hpp"
#include <vector>
#include <string>
#include <future>

namespace Myelin::Core
{
    class MemoryArchivist
    {

    public:
        MemoryArchivist(Myelin::IO::DatabaseManager &db, EmbeddingEngine &emb);
        std::string retrieve(const std::string &input, std::vector<float> &out_user_emb, int session_id);
        int sessionVerify(const int &current_session_id);
        void store(int session_id, const std::string &user_in, const std::string &ai_out, const std::vector<float> &user_emb);
        void updateLastResponseScore(const std::string &last_response, const std::string &user_input);

    private:
        std::mutex engine_mutex;
        std::future<void> evaluation_task;

        Myelin::IO::DatabaseManager &db;
        EmbeddingEngine &emb_engine;
    };
}
#endif