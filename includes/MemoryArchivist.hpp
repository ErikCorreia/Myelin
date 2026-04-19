#ifndef MEMORY_ARCHIVIST_HPP
#define MEMORY_ARCHIVIST_HPP

#include "DatabaseManager.hpp"
#include "EmbeddingEngine.hpp"
#include <vector>
#include <string>

namespace Myelin::Core
{
    class MemoryArchivist
    {
    public:
        MemoryArchivist(Myelin::IO::DatabaseManager &db, EmbeddingEngine &emb);
        std::string retrieve(const std::string &input, std::vector<float> &out_user_emb);
        void store(const std::string &user_in, const std::string &ai_out, const std::vector<float> &user_emb);

    private:
        Myelin::IO::DatabaseManager &db;
        EmbeddingEngine &emb_engine;
    };
}
#endif