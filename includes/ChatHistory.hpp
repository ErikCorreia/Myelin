#ifndef CHAT_HISTORY_HPP
    #define CHAT_HISTORY_HPP

    #include <vector>
    #include <string>
    #include <fstream>

    namespace Myelin::Core {
        struct Message {
            std::string role;
            std::string content;
        };

        class ChatHistory {
            private:
                std::vector<Message> messages;
                std::string history_path = "/home/erik/Myelin/memory/chat_history.txt";
                const size_t max_history = 10;
            public:
                void add_message(const std::string &role, const std::string &content);

                std::string get_formatted_history() const;
                void clear();

                void save_to_file();
                void load_from_file();
        };

    };

#endif