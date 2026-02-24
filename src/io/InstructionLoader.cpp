#include "InstructionLoader.hpp"
#include "Logger.hpp"
#include <filesystem>
#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>
#include <algorithm>

namespace fs = std::filesystem;

namespace Myelin::IO
{
    std::string InstructionLoader::load_from_folder(const std::string &folder_path)
    {
        std::string combined_instructions;
        std::vector<fs::path> file_paths;

        try
        {
            if (!fs::exists(folder_path))
            {
                Myelin::IO::Logger::log(Myelin::IO::Logger::ERR, "Pasta de instruções não encontrada: " + folder_path);
                return "";
            }

            for (const auto &entry : fs::directory_iterator(folder_path))
            {
                if (entry.is_regular_file() && entry.path().extension() == ".md")
                {
                    file_paths.push_back(entry.path());
                }
            }

            std::sort(file_paths.begin(), file_paths.end());

            for (const auto &path : file_paths)
            {
                std::ifstream file(path, std::ios::in | std::ios::binary); 
                if (file.is_open())
                {
                    std::stringstream buffer;
                    buffer << file.rdbuf();
                    std::string content = buffer.str();

                    Myelin::IO::Logger::log(Myelin::IO::Logger::INFO,
                                            "Carregando: " + path.filename().string() + " (" + std::to_string(content.length()) + " bytes)");

                    combined_instructions += content + "\n\n";
                }
                else
                {
                    Myelin::IO::Logger::log(Myelin::IO::Logger::ERR, "Falha ao abrir arquivo: " + path.filename().string());
                }
            }
        }
        catch (const fs::filesystem_error &e)
        {
            Myelin::IO::Logger::log(Myelin::IO::Logger::ERR, "Erro de sistema de arquivos: " + std::string(e.what()));
        }

        Myelin::IO::Logger::log(Myelin::IO::Logger::INFO, "Total de instruções combinadas: " + std::to_string(combined_instructions.length()) + " caracteres.");
        return combined_instructions;
    }
}
