#include "DatabaseManager.hpp"

namespace Myelin::IO {

    DatabaseManager::DatabaseManager(const std::string& path) : db_path(path) {
        if (sqlite3_open(path.c_str(), &db) != SQLITE_OK) {
            throw std::runtime_error("Nao foi possivel abrir o banco de dados SQLite.");
        }
        create_tables();
    }
    
    DatabaseManager::~DatabaseManager() {
        if (db) sqlite3_close(db);
    }
    
    void DatabaseManager::create_tables() {
        std::string sql = 
            "CREATE TABLE IF NOT EXISTS chat_history ("
            "id INTEGER PRIMARY KEY AUTOINCREMENT,"
            "role TEXT NOT NULL,"
            "content TEXT NOT NULL,"
            "timestamp DATETIME DEFAULT CURRENT_TIMESTAMP"
            ");";
        execute_query(sql);
    }
    
    bool DatabaseManager::execute_query(const std::string& query) {
        char* err_msg = nullptr;
        if (sqlite3_exec(db, query.c_str(), nullptr, nullptr, &err_msg) != SQLITE_OK) {
            std::cerr << "Erro SQL: " << err_msg << std::endl;
            sqlite3_free(err_msg);
            return false;
        }
        return true;
    }
    
    bool DatabaseManager::add_message(const std::string& role, const std::string& content) {
        const char* sql = "INSERT INTO chat_history (role, content) VALUES (?, ?);";
        sqlite3_stmt* stmt;
    
        if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) return false;
    
        sqlite3_bind_text(stmt, 1, role.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 2, content.c_str(), -1, SQLITE_STATIC);
    
        bool success = (sqlite3_step(stmt) == SQLITE_DONE);
        sqlite3_finalize(stmt);
        return success;
    }
    
    std::vector<Message> DatabaseManager::get_recent_history(int limit) {
        std::vector<Message> history;
        std::string sql = "SELECT id, role, content, timestamp FROM ("
                          "SELECT * FROM chat_history ORDER BY id DESC LIMIT " + std::to_string(limit) +
                          ") ORDER BY id ASC;";
        
        sqlite3_stmt* stmt;
        if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
            while (sqlite3_step(stmt) == SQLITE_ROW) {
                Message msg;
                msg.id = sqlite3_column_int(stmt, 0);
                msg.role = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
                msg.content = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
                msg.timestamp = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
                history.push_back(msg);
            }
        }
        sqlite3_finalize(stmt);
        return history;
    }
}