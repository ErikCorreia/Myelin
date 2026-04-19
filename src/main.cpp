#include "Logger.hpp"
#include "MyelinEngine.hpp"
#include "ConfigLoader.hpp"
#include "EmbeddingEngine.hpp"
#include <iostream>

using namespace Myelin::IO;
using namespace Myelin::Core;

int main()
{
    Logger::log(Logger::INFO, "Myelin inicializando...");
    DatabaseManager db("/home/erik/projects/Myelin/memory/myelin.db");

    EmbeddingEngine emb_engine("/home/erik/projects/Myelin/models/nomic-embed-text-v1.5.Q2_K.gguf");

    try
    {
        EngineConfig config = ConfigLoader::load_from_file("../config.ini");

        Logger::log(Logger::INFO, "Modelo configurado: " + config.model_path);

        MyelinEngine ai(config.model_path, config, db, emb_engine);
        std::string input;

        while (true)
        {
            std::cout << "\nVocê: ";
            std::getline(std::cin, input);
            if (input == "sair")
                break;

            std::cout << "Myelin: ";
            ai.generateResponse(input, [](const std::string &token)
                                { std::cout << token << std::flush; });
            std::cout << std::endl;
        }
    }
    catch (const std::exception &e)
    {
        Logger::log(Logger::ERR, e.what());
        std::cout << "\nO programa encontrou um erro e será fechado. Pressione Enter para sair..." << e.what();
        std::cin.get();
        return 1;
    }
}