#ifndef DATABASE_MANAGER_HPP
#define DATABASE_MANAGER_HPP

#include <sqlite3.h>
#include <string>
#include <vector>
#include <iostream>

namespace Myelin::IO
{
    struct Message
    {
        int id;
        std::string role;
        std::string content;
        std::string timestamp;
    };

    class DatabaseManager
    {
    private:
        sqlite3 *db;
        std::string db_path;

        bool execute_query(const std::string &query);
        void create_tables();

    public:
        DatabaseManager(const std::string &path);
        ~DatabaseManager();

        bool add_message(const std::string &role, const std::string &content, const std::vector<float> &embedding);

        std::string search_keyword_context(const std::string &query, int limit = 3);
        std::vector<Message> get_recent_history(int limit = 10);

        std::string search_semantic_context(const std::vector<float> &query_vector, float threshold, int limit);
        std::string get_recent_chats(int limit);
        bool update_last_ai_score(int score);

        void clear_history();
    };
}

#endif