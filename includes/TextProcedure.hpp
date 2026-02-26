#ifndef TEXT_PROCESSOR_HPP
    #define TEXT_PROCESSOR_HPP

    #include <string>
    #include <vector>
    #include <unordered_set>
    #include <sstream>
    #include <algorithm>

    namespace Myelin::IO {

        class TextProcessor
        {
        public:
            static std::vector<std::string> get_keywords(const std::string &input)
            {
                // Lista básica de stop words em português
                static const std::unordered_set<std::string> stop_words = {
                    "o", "a", "os", "as", "um", "uma", "de", "do", "da", "em", "no", "na",
                    "que", "que", "com", "por", "para", "é", "dos", "das", "meu", "seu",
                    "eu", "voce", "você", "nós", "eles", "esta", "está", "tem", "com", "foi"};
    
                std::vector<std::string> keywords;
                std::string word;
                std::stringstream ss(input);
    
                while (ss >> word)
                {
                    // Converte para minúsculo e remove pontuação básica
                    std::transform(word.begin(), word.end(), word.begin(), ::tolower);
                    word.erase(std::remove_if(word.begin(), word.end(), ::ispunct), word.end());
    
                    // Só adiciona se não for stop word e tiver mais de 2 letras
                    if (word.length() > 2 && stop_words.find(word) == stop_words.end())
                    {
                        keywords.push_back(word);
                    }
                }
                return keywords;
            }
        };
    }

#endif