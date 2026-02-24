#include "ConfigLoader.hpp"
#include "Logger.hpp"
#include <fstream>
#include <sstream>
#include <string>
#include <algorithm>

namespace Myelin::IO {
    EngineConfig ConfigLoader::load_from_file(const std::string& filename) {
        EngineConfig cfg;
        std::ifstream file(filename);
        
        if (!file.is_open()) {
            return cfg;
        }
    
        std::string line;
        while (std::getline(file, line)) {
            line.erase(std::remove(line.begin(), line.end(), '\r'), line.end());
    
            if (line.empty() || line[0] == '#' || line[0] == ';') continue;
    
            auto delimiter_pos = line.find('=');
            if (delimiter_pos != std::string::npos) {
                std::string key = line.substr(0, delimiter_pos);
                std::string value = line.substr(delimiter_pos + 1);
    
                auto trim = [](std::string& s) {
                    size_t first = s.find_first_not_of(" \t");
                    if (first == std::string::npos) return std::string("");
                    size_t last = s.find_last_not_of(" \t");
                    return s.substr(first, (last - first + 1));
                };
    
                key = trim(key);
                value = trim(value);
    
                value.erase(std::remove(value.begin(), value.end(), '\"'), value.end());
    
                if (key == "model_path"){
                    cfg.model_path = value;
                    Logger::log(Logger::INFO, "Model Path configurado via INI: " + cfg.model_path);
                }
                else if (key == "instructions_path"){
                    cfg.instructions_path = value;
                    Logger::log(Logger::INFO, "Instructions Path configurado via INI: " + cfg.instructions_path);
                }
                else if (key == "n_threads"){
                    cfg.n_threads = std::stoi(value);
                    Logger::log(Logger::INFO, "N Threads configurado via INI: " + std::to_string(cfg.n_threads));
                }
                else if (key == "n_threads_batch"){
                    cfg.n_threads_batch = std::stoi(value);
                    Logger::log(Logger::INFO, "N Threads configurado via INI: " + std::to_string(cfg.n_threads_batch));
                }
                else if (key == "n_ctx") {
                    cfg.n_ctx = std::stoi(value);
                    Logger::log(Logger::INFO, "N CTX configurado via INI: " + std::to_string(cfg.n_ctx));
                }

                else if (key == "n_batch"){
                    cfg.n_batch = std::stoi(value);
                    Logger::log(Logger::INFO, "Batch configurado via INI: " + std::to_string(cfg.n_batch));
                }

                else if (key == "max_tokens") {
                    cfg.max_tokens = std::stoi(value);
                    Logger::log(Logger::INFO, "Max Tokesn configurado via INI: " + std::to_string(cfg.max_tokens));
                }
            }
        }
        return cfg;
    }
}