#ifndef INSTRUCTION_LOADER_HPP
    #define INSTRUCTION_LOADER_HPP

    #include <string>
    #include <vector>

    namespace Myelin::IO {
        class InstructionLoader {
            public:
                static std::string load_from_folder(const std::string& folder_path);
        };
    }
#endif