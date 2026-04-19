#ifndef CONTEXT_MANAGER_HPP
#define CONTEXT_MANAGER_HPP

#include <string>

namespace Myelin::Core
{
    class ContextManager
    {
    public:
        ContextManager(const std::string &instr_path);
        std::string assemble(const std::string &input, const std::string &memories, int n_past);

    private:
        std::string instructions_path;
    };
}
#endif