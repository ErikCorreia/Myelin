#ifndef MEMORY_EVALUATOR_HPP
#define MEMORY_EVALUATOR_HPP

#include <string>
#include <vector>
#include <EmbeddingEngine.hpp>
#include <VectorUtils.hpp>

namespace Myelin::Core
{
    class MemoryEvaluator
    {
    public:
        // Retorna uma pontuação de 0 a 100
        static int evaluate(const std::string &ai_res, const std::string &user_in, EmbeddingEngine &emb_engine);
        static float compareSemantic(const std::string &text, const std::string &concept, EmbeddingEngine &emb_engine);

    private:
        static bool contains(const std::string &text, const std::vector<std::string> &keywords);
    };
}
#endif