#include "DatabaseManager.hpp"
#include "TextProcedure.hpp"
#include "VectorUtils.hpp"

namespace Myelin::IO
{

    DatabaseManager::DatabaseManager(const std::string &path) : db_path(path) {
        if (sqlite3_open(path.c_str(), &db) != SQLITE_OK)
        {
            throw std::runtime_error("Nao foi possivel abrir o banco de dados SQLite.");
        }
        create_tables();
    }

    DatabaseManager::~DatabaseManager() {
        if (db)
            sqlite3_close(db);
    }

    void DatabaseManager::create_tables() {
        std::string sql =
            "CREATE TABLE IF NOT EXISTS chat_history ("
                "id INTEGER PRIMARY KEY AUTOINCREMENT,"
                "role TEXT NOT NULL,"
                "content TEXT NOT NULL,"
                "embedding BLOB,"
                "timestamp DATETIME DEFAULT CURRENT_TIMESTAMP"
            ");";
        execute_query(sql);
    }

    bool DatabaseManager::execute_query(const std::string &query) {
        char *err_msg = nullptr;
        if (sqlite3_exec(db, query.c_str(), nullptr, nullptr, &err_msg) != SQLITE_OK)
        {
            std::cerr << "Erro SQL: " << err_msg << std::endl;
            sqlite3_free(err_msg);
            return false;
        }
        return true;
    }

    bool DatabaseManager::add_message(const std::string &role, const std::string &content, std::vector<float>& embedding) {
        const char* sql = "INSERT INTO chat_history (role, content, embedding) VALUES (?, ?, ?);";
        sqlite3_stmt* stmt;

        if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK)  return false;

        sqlite3_bind_text(stmt, 1, role.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 2, content.c_str(), -1, SQLITE_STATIC);

        if (!embedding.empty()) {
            int bytes = embedding.size() * sizeof(float);
            sqlite3_bind_blob(stmt, 3, embedding.data(), bytes, SQLITE_STATIC);
        } else {
            sqlite3_bind_null(stmt, 3);
        }

        bool success = (sqlite3_step(stmt) == SQLITE_DONE);
        sqlite3_finalize(stmt);
        return success;
    }

    std::vector<Message> DatabaseManager::get_recent_history(int limit) {
        std::vector<Message> history;
        std::string sql = "SELECT id, role, content, timestamp FROM ("
                          "SELECT * FROM chat_history ORDER BY id DESC LIMIT " +
                          std::to_string(limit) +
                          ") ORDER BY id ASC;";

        sqlite3_stmt *stmt;
        if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) == SQLITE_OK)
        {
            while (sqlite3_step(stmt) == SQLITE_ROW)
            {
                Message msg;
                msg.id = sqlite3_column_int(stmt, 0);
                msg.role = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 1));
                msg.content = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 2));
                msg.timestamp = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 3));
                history.push_back(msg);
            }
        }
        sqlite3_finalize(stmt);
        return history;
    }

    std::string DatabaseManager::search_keyword_context(const std::string &query, int limit) {
        auto keywords = TextProcessor::get_keywords(query);
        if (keywords.empty())
            return "";

        std::string context = "";
        // Vamos buscar pela palavra mais longa da pergunta (geralmente a mais importante)
        std::string main_keyword = *std::max_element(keywords.begin(), keywords.end(), [](const std::string &a, const std::string &b) {
            return a.length() < b.length();
        });

        std::string sql = "SELECT role, content FROM chat_history "
                          "WHERE content LIKE ? AND role != 'system' "
                          "ORDER BY id DESC LIMIT ?;";

        sqlite3_stmt *stmt;
        if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
            std::string search_term = "%" + main_keyword + "%";
            sqlite3_bind_text(stmt, 1, search_term.c_str(), -1, SQLITE_STATIC);
            sqlite3_bind_int(stmt, 2, limit);

            while (sqlite3_step(stmt) == SQLITE_ROW) {
                std::string role = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 0));
                std::string content = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 1));
                context += "[" + role + " disse anteriormente]: " + content + "\n";
            }
        }
        
        sqlite3_finalize(stmt);

        return context.empty() ? "" : "\nContexto de memória recuperado:\n" + context;
    }

    std::string DatabaseManager::search_semantic_context(const std::vector<float>& query_vector, float threshold, int limit) {
        std::string context = "";
        std::string sql = "SELECT role, content, embedding, datetime(timestamp, 'localtime') FROM chat_history WHERE embedding IS NOT NULL;";
        
        sqlite3_stmt* stmt;
        if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
            std::vector<std::pair<float, std::string>> matches;

            while (sqlite3_step(stmt) == SQLITE_ROW) {
                std::string role = (const char*)sqlite3_column_text(stmt, 0);
                std::string content = (const char*)sqlite3_column_text(stmt, 1);
                std::string date = (const char*)sqlite3_column_text(stmt, 3);
                
                const float* blob_data = (const float*)sqlite3_column_blob(stmt, 2);
                int blob_size = sqlite3_column_bytes(stmt, 2) / sizeof(float);
                std::vector<float> stored_vector(blob_data, blob_data + blob_size);

                float score = Myelin::Utils::VectorUtils::cosine_similarity(query_vector, stored_vector);

                if (score >= threshold) {
                    std::string memory_with_time = "[Memória de " + date + " - " + role + "]: " + content;
                    matches.push_back({score, memory_with_time});
                }
            }
            sqlite3_finalize(stmt);

            std::sort(matches.rbegin(), matches.rend());

            for (int i = 0; i < std::min((int)matches.size(), limit); ++i) {
                context += matches[i].second + "\n";
            }
        }
        return context;
    }
}