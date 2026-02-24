#ifndef LOGGER_HPP
#define LOGGER_HPP

#include <string>
#include <fstream>
#include <sstream>

namespace Myelin::IO {
    class Logger {
        public:
            enum Level { INFO, WARN, ERR };
            static void log(Level level, const std::string& message);
        
        private:
            static std::string get_timestamp();
    };
}

#endif