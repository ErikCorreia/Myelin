#include "Logger.hpp"
#include "LlamaEngine.hpp"
#include "ConfigLoader.hpp"
#include "EmbeddingEngine.hpp"
#include <iostream>


using namespace Myelin::IO;
using namespace Myelin::Core;

int main(){
    Logger::log(Logger::INFO, "Myelin inicializando...");
    DatabaseManager db("/home/erik/Myelin/memory/myelin.db");
    
    EmbeddingEngine emb_engine("/home/erik/Myelin/models/nomic-embed-text-v1.5.f16.gguf");

    try {
        EngineConfig config = ConfigLoader::load_from_file("../config.ini");
        
        Logger::log(Logger::INFO, "Modelo configurado: " + config.model_path);

        LlamaEngine ai(config.model_path, config, db, emb_engine);
        std::string input;

        while (true)
        {
            std::cout << "\nVocê: ";
            std::getline(std::cin, input);
             if(input == "sair") break;
            ai.generateResponse(input);
        }
    } catch(const std::exception& e){
        Logger::log(Logger::ERR, e.what());
        std::cout << "\nO programa encontrou um erro e será fechado. Pressione Enter para sair...";
        std::cin.get();
        return 1;
    }
}