#ifndef CPPHTTPLIB_OPENSSL_SUPPORT
#define CPPHTTPLIB_OPENSSL_SUPPORT
#endif

#include "MemoryEvaluator.hpp"
#include <algorithm>
#include <iostream>
#include "httplib.h"
#include <nlohmann/json.hpp>

namespace Myelin::Core
{
    bool MemoryEvaluator::contains(const std::string &text, const std::vector<std::string> &keywords)
    {
        std::string lower = text;
        std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);

        lower.erase(std::remove_if(lower.begin(), lower.end(),
                                   [](unsigned char c)
                                   { return std::ispunct(c) || std::isspace(c); }),
                    lower.end());

        for (const auto &k : keywords)
        {
            std::string clean_k = k;
            clean_k.erase(std::remove_if(clean_k.begin(), clean_k.end(), ::isspace), clean_k.end());

            if (lower.find(clean_k) != std::string::npos)
                return true;
        }
        return false;
    }

    int MemoryEvaluator::evaluate(const std::string &ai_res, const std::string &user_in, EmbeddingEngine &emb_engine)
    {
        int score = 50; // Base neutra

        // Keywords (Soma acumulativa explícita)
        std::vector<std::string> positive = {"boa", "legal", "exato", "exata", "correto", "correta", "perfeito", "perfeita", "sim"};
        std::vector<std::string> negative = {"errado", "errada", "nao", "mentira", "falso", "falsa"};

        bool has_pos = contains(user_in, positive);
        bool has_neg = contains(user_in, negative);

        if (has_pos)
            score += 30; // 50 + 30 = 80
        if (has_neg)
            score -= 40; // 50 - 40 = 10

        // Semântica (Log para ver o valor real)
        float approval = compareSemantic(user_in, "Sim, isso mesmo, perfeito, correto, exato", emb_engine);

        // Se a similaridade for maior que 0.5 (mais flexível para palavras curtas)
        if (approval > 0.50f)
        {
            score += 15;
        }

        if (ai_res.length() > 40)
        {
            score += 5;
        }

        // Penalidade para respostas da IA muito curtas
        if (ai_res.length() < 20)
        {
            score -= 10;
        }

        // LOG DE CALIBRAÇÃO FINAL
        // std::cout << "[EVAL-DEBUG] Base: 50 | Keyword Pos: " << (has_pos ? "SIM (+30)" : "NAO")
        //           << " | Semantica: " << approval << " | Final: " << score << std::endl;

        return std::clamp(score, 0, 100);
    }

    float MemoryEvaluator::compareSemantic(const std::string &text, const std::string &concept, EmbeddingEngine &emb_engine)
    {
        auto v1 = emb_engine.get_embedding(text);
        auto v2 = emb_engine.get_embedding(concept);

        if (v1.empty() || v2.empty())
            return 0.0f;

        return Myelin::Utils::VectorUtils::cosine_similarity(v1, v2);
    }
}