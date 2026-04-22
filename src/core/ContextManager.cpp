#include "ContextManager.hpp"
#include "InstructionLoader.hpp"

#include <fstream>
#include <algorithm>
#include <sstream>

namespace Myelin::Core
{
    ContextManager::ContextManager(const std::string &instr_path) : instructions_path(instr_path) {}

    std::string ContextManager::trim(const std::string &s)
    {
        size_t first = s.find_first_not_of(" \t\r\n");
        if (first == std::string::npos)
            return "";
        size_t last = s.find_last_not_of(" \t\r\n");
        return s.substr(first, (last - first + 1));
    }

    std::string ContextManager::clean_str(const std::string &s)
    {
        std::string res = s;
        res.erase(std::remove(res.begin(), res.end(), '\r'), res.end());
        size_t first = res.find_first_not_of(" \t");
        if (first == std::string::npos)
            return "";
        size_t last = res.find_last_not_of(" \t");
        return res.substr(first, (last - first + 1));
    }

    std::string ContextManager::format_identity_ini(const std::string &ini_path)
    {
        std::ifstream file(ini_path);
        if (!file.is_open())
            return "";

        std::stringstream ss;
        ss << "### IDENTITY_CONFIG (Rigorously Follow) ###\n";

        std::string line;
        while (std::getline(file, line))
        {
            line = clean_str(line);
            if (line.empty() || line[0] == '#' || line[0] == ';')
                continue;

            if (line[0] == '[' && line.back() == ']')
            {
                ss << "\nCATEGORY_" << line.substr(1, line.size() - 2) << ":\n";
            }
            else
            {
                auto pos = line.find('=');
                if (pos != std::string::npos)
                {
                    ss << "  - " << clean_str(line.substr(0, pos))
                       << ": " << clean_str(line.substr(pos + 1)) << "\n";
                }
            }
        }
        ss << "###########################################\n\n";
        return ss.str();
    }

    std::string ContextManager::assemble(const std::string &input, const std::string &memories, int n_past)
    {
        if (n_past > 0)
        {
            return "<|start_header_id|>user<|end_header_id|>\n\n" + input + "<|eot_id|><|start_header_id|>assistant<|end_header_id|>\n\n";
        }

        std::string identity = format_identity_ini("../.myelin/identity.ini");

        std::string system = Myelin::IO::InstructionLoader::load_from_folder(instructions_path);

        std::string prompt = "<|start_header_id|>system<|end_header_id|>\n\n" + identity + system;

        if (!memories.empty())
        {
            prompt += "\n\n[DADOS DE MEMÓRIA DE LONGO PRAZO]\n" + memories +
                      "\n[USE OS DADOS ACIMA PARA RESPONDER COM PRECISÃO]\n";
        }

        prompt += "<|eot_id|><|start_header_id|>user<|end_header_id|>\n\n" + input + "<|eot_id|><|start_header_id|>assistant<|end_header_id|>\n\n";
        return prompt;
    }
}