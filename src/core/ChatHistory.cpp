#include "ChatHistory.hpp"
#include <filesystem>
#include <iostream>

namespace fs = std::filesystem;

namespace Myelin::Core
{

    void ChatHistory::add_message(const std::string &role, const std::string &content)
    {
        messages.push_back({role, content});
        save_to_file();

        if (messages.size() > max_history * 2)
        {
            messages.erase(messages.begin(), messages.begin() + 2);
        }
    }

    std::string ChatHistory::get_formatted_history() const
    {
        std::string history_str = "";
        for (const auto &msg : messages)
        {
            history_str += "<|start_header_id|>" + msg.role + "<|end_header_id|>\n\n" + msg.content + "<|eot_id|>";
        }

        return history_str;
    }

    void ChatHistory::save_to_file()
    {
        std::filesystem::create_directories("memory"); // Garante que a pasta existe
        std::ofstream file(history_path);
        if (file.is_open())
        {
            for (const auto &msg : messages)
            {
                file << msg.role << ": " << msg.content << "\n---\n";
            }
            file.close();
        }
    }

    void ChatHistory::load_from_file()
    {
        std::ifstream file(history_path);
        if (!file.is_open())
            return;

        messages.clear();
        std::string line, role, content;
        while (std::getline(file, line))
        {
            if (line == "---")
            {
                if (!role.empty())
                    messages.push_back({role, content});
                role = "";
                content = "";
            }
            else if (role.empty())
            {
                size_t pos = line.find(": ");
                if (pos != std::string::npos)
                {
                    role = line.substr(0, pos);
                    content = line.substr(pos + 2);
                }
            }
            else
            {
                content += "\n" + line;
            }
        }
    }

    void ChatHistory::clear()
    {
        messages.clear();
    }

}