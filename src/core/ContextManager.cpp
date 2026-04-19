#include "ContextManager.hpp"
#include "InstructionLoader.hpp"

namespace Myelin::Core
{
    ContextManager::ContextManager(const std::string &instr_path) : instructions_path(instr_path) {}

    std::string ContextManager::assemble(const std::string &input, const std::string &memories, int n_past)
    {
        if (n_past > 0)
        {
            return "<|start_header_id|>user<|end_header_id|>\n\n" + input + "<|eot_id|><|start_header_id|>assistant<|end_header_id|>\n\n";
        }

        std::string system = Myelin::IO::InstructionLoader::load_from_folder(instructions_path);
        std::string prompt = "<|start_header_id|>system<|end_header_id|>\n\n" + system;

        if (!memories.empty())
        {
            prompt += "\n\n### MEMÓRIAS ###\n" + memories + "\n### FIM ###\n";
        }

        prompt += "<|eot_id|><|start_header_id|>user<|end_header_id|>\n\n" + input + "<|eot_id|><|start_header_id|>assistant<|end_header_id|>\n\n";
        return prompt;
    }
}