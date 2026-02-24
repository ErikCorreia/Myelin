#include "Logger.hpp"
#include "LlamaEngine.hpp"
#include "ConfigLoader.hpp"
#include <iostream>

using namespace Myelin::IO;
using namespace Myelin::Core;

int main(){
    Logger::log(Logger::INFO, "Myelin inicializando...");
    
    Myelin::Core::ChatHistory history;

    try {
        EngineConfig config = ConfigLoader::load_from_file("../config.ini");
        
        Logger::log(Logger::INFO, "Modelo configurado: " + config.model_path);
        
        history.load_from_file();
        // std::cout << "Memória carregada. Conversas anteriores: " << history.get_messages().size() << std::endl;

        LlamaEngine ai(config.model_path, config, history);
        std::string input;

        while (true)
        {
            std::cout << "\nVocê: ";
            std::getline(std::cin, input);
            if(input == "sair") break;

            ai.generateResponse(input);
        }
    } catch(const std::exception& e){
        std::cout << "\nO programa encontrou um erro e será fechado. Pressione Enter para sair...";
        std::cin.get();
        return 1;
    }
}