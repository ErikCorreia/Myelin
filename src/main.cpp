#include "Logger.hpp"
#include "LlamaEngine.hpp"
#include "ConfigLoader.hpp"
#include <iostream>

using namespace Myelin::IO;
using namespace Myelin::Core;

int main(){
    Logger::log(Logger::INFO, "Myelin inicializando...");
    
    // Myelin::Core::ChatHistory history;
    // MemoryManager memory("/home/erik/Myelin/chat_history.json");
    DatabaseManager db("/home/erik/Myelin/memory/myelin.db");

    try {
        EngineConfig config = ConfigLoader::load_from_file("../config.ini");
        
        Logger::log(Logger::INFO, "Modelo configurado: " + config.model_path);
        
        // memory.load_from_disk();
        // history.load_from_file();

        LlamaEngine ai(config.model_path, config, db);
        std::string input;

        while (true)
        {
            std::cout << "\nVocê: ";
            std::getline(std::cin, input);
             if(input == "sair") break;
            // if (!std::getline(std::cin, input) || input == "sair") break;

            ai.generateResponse(input);
        }
    } catch(const std::exception& e){
        std::cout << "\nO programa encontrou um erro e será fechado. Pressione Enter para sair...";
        std::cin.get();
        return 1;
    }
}