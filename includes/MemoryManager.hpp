#ifndef MYELIN_MEMORY_MANAGER_HPP
#define MYELIN_MEMORY_MANAGER_HPP

#include <string>
#include <vector>
#include <nlohmann/json.hpp>

namespace Myelin::IO
{
    struct Message
    {
        std::string role;
        std::string content;
        std::string timestamp;
    };

    class MemoryManager
    {
    private:
        std::string name;
        std::string file_path;
        std::vector<Message> history;

    public:
        MemoryManager(const std::string &path);
        void add_message(const std::string &role, const std::string &content);
        void save_to_disk();
        void load_from_disk();

        std::string get_full_prompt(const std::string &content);
    };
}
#endif