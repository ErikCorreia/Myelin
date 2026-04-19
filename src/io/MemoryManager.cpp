#include <fstream>
#include <iostream>
#include "MemoryManager.hpp"
#include "Logger.hpp"

namespace Myelin::IO
{
    MemoryManager::MemoryManager(const std::string &path) : file_path(path) {}

    std::string get_current_timestamp()
    {
        auto now = std::chrono::system_clock::now();
        auto in_time_t = std::chrono::system_clock::to_time_t(now);
        std::stringstream ss;
        ss << std::put_time(std::localtime(&in_time_t), "%Y-%m-%d %H:%M:%S");
        return ss.str();
    }
    void MemoryManager::add_message(const std::string &role, const std::string &content)
    {
        history.push_back({role, content, get_current_timestamp()});
        save_to_disk();
    }

    void MemoryManager::save_to_disk()
    {
        try
        {
            nlohmann::json root;
            root["session_id"] = "session_" + get_current_timestamp();
            root["user_profile"] = {
                {"name", "Erik"},
                {"role_preference", "C++ Developer"},
                {"environment", "WSL2"}};

            root["messages"] = nlohmann::json::array();
            for (const auto &msg : history)
            {
                root["messages"].push_back({{"role", msg.role},
                                            {"content", msg.content},
                                            {"timestamp", msg.timestamp}});
            }

            std::ofstream file(file_path, std::ios::out | std::ios::trunc);
            if (file.is_open())
            {
                file << root.dump(4);
                file.close();
            }
        }
        catch (const std::exception &e)
        {
            std::cerr << "ERRO: Não foi possível abrir o arquivo para escrita: " << e.what() << std::endl;
        }
    }

    void MemoryManager::load_from_disk()
    {
        if (!std::filesystem::exists(file_path))
            return;

        std::ifstream file(file_path);
        if (!file.is_open() || file.peek() == std::ifstream::traits_type::eof())
            return;

        try
        {
            nlohmann::json root;
            file >> root;

            history.clear();
            if (root.is_object() && root.contains("messages") && root["messages"].is_array())
            {

                for (const auto &item : root["messages"])
                {
                    std::string role = item.value("role", "");
                    std::string content = item.value("content", "");

                    if (!role.empty() && !content.empty())
                    {
                        history.push_back({role,
                                           content,
                                           item.value("timestamp", "")});
                    }
                }
            }
        }
        catch (const nlohmann::json::parse_error &e)
        {
            std::cerr << "Falha ao carregar JSON: " << e.what() << std::endl;
        }
    }

    std::string MemoryManager::get_full_prompt(const std::string &system_instruction)
    {
        std::string prompt = "<|start_header_id|>system<|end_header_id|>\n\n" + system_instruction + "<|eot_id|>";

        for (const auto &msg : history)
        {
            prompt += "<|start_header_id|>" + msg.role + "<|end_header_id|>\n\n" + msg.content + "<|eot_id|>";
        }

        prompt += "<|start_header_id|>assistant<|end_header_id|>\n\n";

        return prompt;
    }
}