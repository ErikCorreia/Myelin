#ifndef DATABASE_MANAGER_HPP
#define DATABASE_MANAGER_HPP

#include <sqlite3.h>
#include <string>
#include <vector>
#include <iostream>

namespace Myelin::IO {
    struct Message {
        int id;
        std::string role;
        std::string content;
        std::string timestamp;
    };
    
    class DatabaseManager {
    private:
        sqlite3* db;
        std::string db_path;
    
        bool execute_query(const std::string& query);
        void create_tables();
    
    public:
        DatabaseManager(const std::string& path);
        ~DatabaseManager();
    
        bool add_message(const std::string& role, const std::string& content);
    
        std::vector<Message> get_recent_history(int limit = 10);

        void clear_history();
    };
}

#endif