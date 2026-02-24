#ifndef CONFIG_LOADER_HPP
    #define CONFIG_LOADER_HPP

    #include "LlamaEngine.hpp"
    #include <string>
    #include <map>

    using namespace Myelin::Core;

    namespace Myelin::IO 
    {
        class ConfigLoader {
            public:
                static EngineConfig load_from_file(const std::string& filename);
        };
    }
#endif