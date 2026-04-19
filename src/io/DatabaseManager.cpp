#include "DatabaseManager.hpp"
#include "TextProcedure.hpp"
#include "VectorUtils.hpp"
#include <chrono>
#include <ctime>

namespace Myelin::IO
{

    DatabaseManager::DatabaseManager(const std::string &path) : db_path(path)
    {
        if (sqlite3_open(path.c_str(), &db) != SQLITE_OK)
        {
            throw std::runtime_error("Nao foi possivel abrir o banco de dados SQLite.");
        }
        create_tables();
    }

    DatabaseManager::~DatabaseManager()
    {
        if (db)
            sqlite3_close(db);
    }

    void DatabaseManager::create_tables()
    {
        execute_query(
            "CREATE TABLE IF NOT EXISTS chat_sessions ("
            "id INTEGER PRIMARY KEY AUTOINCREMENT,"
            "title TEXT,"
            "timestamp DATETIME DEFAULT CURRENT_TIMESTAMP"
            ");");

        execute_query(
            "CREATE TABLE IF NOT EXISTS chat_history ("
            "id INTEGER PRIMARY KEY AUTOINCREMENT,"
            "session_id INTEGER,"
            "role TEXT NOT NULL,"
            "content TEXT NOT NULL,"
            "embedding BLOB,"
            "score INTEGER DEFAULT 50,"
            "timestamp DATETIME DEFAULT CURRENT_TIMESTAMP,"
            "FOREIGN KEY(session_id) REFERENCES chat_sessions(id)"
            ");");
    }

    bool DatabaseManager::execute_query(const std::string &query)
    {
        char *err_msg = nullptr;
        if (sqlite3_exec(db, query.c_str(), nullptr, nullptr, &err_msg) != SQLITE_OK)
        {
            std::cerr << "Erro SQL: " << err_msg << std::endl;
            sqlite3_free(err_msg);
            return false;
        }
        return true;
    }

    int DatabaseManager::create_session(const std::string &title)
    {
        std::string sql = "INSERT INTO chat_sessions (title) VALUES (?);";
        sqlite3_stmt *stmt;
        int session_id = -1;

        if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) == SQLITE_OK)
        {
            sqlite3_bind_text(stmt, 1, title.c_str(), -1, SQLITE_TRANSIENT);
            if (sqlite3_step(stmt) == SQLITE_DONE)
            {
                session_id = (int)sqlite3_last_insert_rowid(db);
            }
        }
        sqlite3_finalize(stmt);
        return session_id;
    }

    // bool DatabaseManager::add_message(int session_id, const std::string &role, const std::string &content, const std::vector<float> &embedding)
    // {
    //     const char *sql = "INSERT INTO chat_history (role, content, embedding) VALUES (?, ?, ?);";
    //     sqlite3_stmt *stmt;

    //     if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK)
    //         return false;

    //     sqlite3_bind_text(stmt, 1, role.c_str(), -1, SQLITE_STATIC);
    //     sqlite3_bind_text(stmt, 2, content.c_str(), -1, SQLITE_STATIC);

    //     if (!embedding.empty())
    //     {
    //         int bytes = embedding.size() * sizeof(float);
    //         sqlite3_bind_blob(stmt, 3, embedding.data(), bytes, SQLITE_STATIC);
    //     }
    //     else
    //     {
    //         sqlite3_bind_null(stmt, 3);
    //     }

    //     bool success = (sqlite3_step(stmt) == SQLITE_DONE);
    //     sqlite3_finalize(stmt);
    //     return success;
    // }

    bool DatabaseManager::add_message(int session_id, const std::string &role, const std::string &content, const std::vector<float> &embedding)
    {
        std::string sql = "INSERT INTO chat_history (session_id, role, content, embedding) VALUES (?, ?, ?, ?);";
        sqlite3_stmt *stmt;

        if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) == SQLITE_OK)
        {
            sqlite3_bind_int(stmt, 1, session_id);
            sqlite3_bind_text(stmt, 2, role.c_str(), -1, SQLITE_TRANSIENT);
            sqlite3_bind_text(stmt, 3, content.c_str(), -1, SQLITE_TRANSIENT);

            // Converte vector para blob
            sqlite3_bind_blob(stmt, 4, embedding.data(), embedding.size() * sizeof(float), SQLITE_TRANSIENT);

            bool success = (sqlite3_step(stmt) == SQLITE_DONE);
            sqlite3_finalize(stmt);
            return success;
        }
        return false;
    }

    std::string DatabaseManager::get_session_chats(int session_id, int limit)
    {
        std::string sql = "SELECT role, content FROM (SELECT * FROM chat_history WHERE session_id = ? ORDER BY id DESC LIMIT ?) ORDER BY id ASC;";
        sqlite3_stmt *stmt;
        std::string history;

        if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) == SQLITE_OK)
        {
            sqlite3_bind_int(stmt, 1, session_id);
            sqlite3_bind_int(stmt, 2, limit);

            while (sqlite3_step(stmt) == SQLITE_ROW)
            {
                std::string role = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 0));
                std::string content = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 1));
                history += "[" + role + "]: " + content + "\n";
            }
        }
        sqlite3_finalize(stmt);
        return history;
    }

    std::vector<Message> DatabaseManager::get_recent_history(int limit)
    {
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

    bool DatabaseManager::update_last_ai_score(int score)
    {
        std::string sql = "UPDATE chat_history SET score = ? WHERE role = 'assistant' "
                          "AND id = (SELECT MAX(id) FROM chat_history WHERE role = 'assistant');";
        sqlite3_stmt *stmt;
        if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) == SQLITE_OK)
        {
            sqlite3_bind_int(stmt, 1, score);
            sqlite3_step(stmt);
            sqlite3_finalize(stmt);
            return true;
        }
        return false;
    }

    std::string DatabaseManager::search_keyword_context(const std::string &query, int limit)
    {
        auto keywords = TextProcessor::get_keywords(query);
        if (keywords.empty())
            return "";

        std::string context = "";
        std::string main_keyword = *std::max_element(keywords.begin(), keywords.end(), [](const std::string &a, const std::string &b)
                                                     { return a.length() < b.length(); });

        std::string sql = "SELECT role, content FROM chat_history "
                          "WHERE content LIKE ? AND role != 'system' "
                          "ORDER BY id DESC LIMIT ?;";

        sqlite3_stmt *stmt;
        if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) == SQLITE_OK)
        {
            std::string search_term = "%" + main_keyword + "%";
            sqlite3_bind_text(stmt, 1, search_term.c_str(), -1, SQLITE_STATIC);
            sqlite3_bind_int(stmt, 2, limit);

            while (sqlite3_step(stmt) == SQLITE_ROW)
            {
                std::string role = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 0));
                std::string content = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 1));
                context += "[" + role + " disse anteriormente]: " + content + "\n";
            }
        }

        sqlite3_finalize(stmt);

        return context.empty() ? "" : "\nContexto de memória recuperado:\n" + context;
    }

    std::string DatabaseManager::search_semantic_global(const std::vector<float> &query_vector, float threshold, int limit, int exclude_session)
    {
        std::string context = "";
        auto now = std::chrono::system_clock::now();
        std::time_t now_time = std::chrono::system_clock::to_time_t(now);

        std::string sql = "SELECT role, content, embedding, strftime('%s', timestamp), "
                          "datetime(timestamp, 'localtime'), score FROM chat_history "
                          "WHERE embedding IS NOT NULL AND session_id != ?;";

        sqlite3_stmt *stmt;
        if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) == SQLITE_OK)
        {
            sqlite3_bind_int(stmt, 1, exclude_session);

            std::vector<std::pair<float, std::string>> matches;

            while (sqlite3_step(stmt) == SQLITE_ROW)
            {
                int stored_score = sqlite3_column_int(stmt, 5);
                long long msg_time = sqlite3_column_int64(stmt, 3);
                float diff_seconds = static_cast<float>(now_time - msg_time);
                float hours_age = diff_seconds / 3600.0f;

                const char *role_ptr = (const char *)sqlite3_column_text(stmt, 0);
                std::string role = role_ptr ? role_ptr : "unknown";

                const char *content_ptr = (const char *)sqlite3_column_text(stmt, 1);
                std::string content = content_ptr ? content_ptr : "";

                const char *date_ptr = (const char *)sqlite3_column_text(stmt, 4);
                std::string date_str = date_ptr ? date_ptr : "data desconhecida";

                const void *blob_ptr = sqlite3_column_blob(stmt, 2);
                if (!blob_ptr)
                    continue;

                int blob_size = sqlite3_column_bytes(stmt, 2) / sizeof(float);
                const float *float_ptr = static_cast<const float *>(blob_ptr);
                std::vector<float> stored_vector(float_ptr, float_ptr + blob_size);

                float similarity = Myelin::Utils::VectorUtils::cosine_similarity(query_vector, stored_vector);
                float decay_factor = Myelin::Utils::VectorUtils::time_decay(hours_age, 168.0f);
                float score_multiplier = (static_cast<float>(stored_score) / 50.0f);

                float final_score = similarity * decay_factor * score_multiplier;

                if (final_score >= threshold)
                {
                    std::string prefix = (stored_score < 40) ? "[Memória Incerta de " : "[Memória de ";
                    std::string memory_entry = prefix + date_str + " - " + role + "]: " + content;

                    matches.push_back({final_score, memory_entry});
                }
            }
            sqlite3_finalize(stmt);

            std::sort(matches.rbegin(), matches.rend());

            for (int i = 0; i < std::min((int)matches.size(), limit); ++i)
            {
                context += matches[i].second + "\n";
            }
        }
        return context;
    }

    /**
     * @deprecated
     */
    std::string DatabaseManager::search_semantic_context(const std::vector<float> &query_vector, float threshold, int limit)
    {
        std::string context = "";
        auto now = std::chrono::system_clock::now();
        std::time_t now_time = std::chrono::system_clock::to_time_t(now);

        // Adicionado 'score' na consulta (índice 5)
        std::string sql = "SELECT role, content, embedding, strftime('%s', timestamp), datetime(timestamp, 'localtime'), score FROM chat_history WHERE embedding IS NOT NULL;";

        sqlite3_stmt *stmt;
        if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) == SQLITE_OK)
        {
            std::vector<std::pair<float, std::string>> matches;

            while (sqlite3_step(stmt) == SQLITE_ROW)
            {
                int stored_score = sqlite3_column_int(stmt, 5);

                long long msg_time = sqlite3_column_int64(stmt, 3);
                float diff_seconds = static_cast<float>(now_time - msg_time);
                float hours_age = diff_seconds / 3600.0f;

                const char *role_ptr = (const char *)sqlite3_column_text(stmt, 0);
                std::string role = role_ptr ? role_ptr : "unknown";

                const char *content_ptr = (const char *)sqlite3_column_text(stmt, 1);
                std::string content = content_ptr ? content_ptr : "";

                const char *date_ptr = (const char *)sqlite3_column_text(stmt, 4);
                std::string date_str = date_ptr ? date_ptr : "data desconhecida";

                const void *blob_ptr = sqlite3_column_blob(stmt, 2);
                if (!blob_ptr)
                    continue;

                int blob_size = sqlite3_column_bytes(stmt, 2) / sizeof(float);
                const float *float_ptr = static_cast<const float *>(blob_ptr);
                std::vector<float> stored_vector(float_ptr, float_ptr + blob_size);

                float similarity = Myelin::Utils::VectorUtils::cosine_similarity(query_vector, stored_vector);
                float decay_factor = Myelin::Utils::VectorUtils::time_decay(hours_age, 168.0f);

                // Aplicação do Multiplicador de Recompensa (Score)
                // 50 é o neutro (1.0x). Valores menores punem a memória, maiores dão bônus.
                float score_multiplier = (static_cast<float>(stored_score) / 50.0f);
                float final_score = similarity * decay_factor * score_multiplier;

                if (final_score >= threshold)
                {
                    // Define o prefixo baseado na qualidade da memória (score < 40 é considerada incerta)
                    std::string prefix = (stored_score < 40) ? "[Memória Incerta de " : "[Memória de ";
                    std::string memory_entry = prefix + date_str + " - " + role + "]: " + content;

                    matches.push_back({final_score, memory_entry});
                }
            }

            sqlite3_finalize(stmt);

            // Ordena por pontuação final (da maior para a menor)
            std::sort(matches.rbegin(), matches.rend());

            for (int i = 0; i < std::min((int)matches.size(), limit); ++i)
            {
                context += matches[i].second + "\n";
            }
        }
        return context;
    }

    std::string DatabaseManager::get_recent_chats(int limit)
    {
        std::string sql = "SELECT role, content FROM chat_history ORDER BY id DESC LIMIT ?;";
        sqlite3_stmt *stmt;
        std::string history;

        if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) == SQLITE_OK)
        {
            sqlite3_bind_int(stmt, 1, limit);

            // Vetor temporário para inverter a ordem (deixar cronológico)
            std::vector<std::string> lines;
            while (sqlite3_step(stmt) == SQLITE_ROW)
            {
                std::string role = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 0));
                std::string content = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 1));
                lines.push_back("[" + role + "]: " + content);
            }
            sqlite3_finalize(stmt);

            for (auto it = lines.rbegin(); it != lines.rend(); ++it)
            {
                history += *it + "\n";
            }
        }
        return history;
    }
}