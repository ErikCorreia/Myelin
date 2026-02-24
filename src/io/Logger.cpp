#include "Logger.hpp"
#include <iostream>
#include <chrono>
#include <iomanip>

namespace Myelin::IO {
    void Logger::log(Level level, const std::string& message) {
        std::string label;
        switch (level) {
            case INFO:  label = "[INFO]"; break;
            case WARN:  label = "[WARN]"; break;
            case ERR: label = "[ERROR]"; break;
        }

        // Log no Console
        // std::cout << label << " " << message << std::endl;

        // Log no Arquivo (myelin.log)
        std::ofstream log_file("myelin.log", std::ios::app);
        if (log_file.is_open()) {
            auto now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
            log_file << std::put_time(std::localtime(&now), "%Y-%m-%d %H:%M:%S") 
                     << " " << label << " " << message << std::endl;
        }
    }
    
    std::string Logger::get_timestamp() {
        auto now = std::chrono::system_clock::now();
        auto in_time_t = std::chrono::system_clock::to_time_t(now);
        std::stringstream ss;
        ss << std::put_time(std::localtime(&in_time_t), "%Y-%m-%d %H:%M:%S");
        return ss.str();
    }
}