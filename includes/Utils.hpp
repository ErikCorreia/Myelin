#ifndef MYELIN_UTILS_HPP
#define MYELIN_UTILS_HPP

#include <string>
#include <chrono>
#include <iomanip>
#include <sstream>

namespace Myelin::Utils
{
    inline std::string get_timestamp()
    {
        auto now = std::chrono::system_clock::now();
        auto in_time_t = std::chrono::system_clock::to_time_t(now);
        std::stringstream ss;
        // Formato compatível com SQLite: YYYY-MM-DD HH:MM:SS
        ss << std::put_time(std::localtime(&in_time_t), "%Y-%m-%d %H:%M:%S");
        return ss.str();
    }
}

#endif