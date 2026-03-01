#include <vector>
#include <cmath>

typedef std::vector<float> Embedding;

namespace Myelin::Utils {
    class VectorUtils {
        public:
            /**
             * Calcula a precisão entre 2 vetores
             */
            static float cosine_similarity(const Embedding& v1, const Embedding& v2) {
                    float dot_product = 0.0, norm_a = 0.0, norm_b = 0.0;
                    for (size_t i = 0; i < v1.size(); ++i) {
                        dot_product += v1[i] * v2[i];
                        norm_a += v1[i] * v1[i];
                        norm_b += v2[i] * v2[i];
                    }
                    return dot_product / (std::sqrt(norm_a) * std::sqrt(norm_b));
                }

            /**
             * @param hours_age: Idade da memória em horas
             * @param halflife_hours: Em quanto tempo a importância da memória cai pela metade?
             * (Sugestão: 720 horas = 30 dias)
             */
            static float time_decay(float hours_age, float halflife_hours = 720.0f) {
                return std::pow(0.5f, hours_age / halflife_hours);
            }

                // static float cosine_similarity(const std::vector<float>& v1, const std::vector<float>& v2) {
                //     if (v1.size() != v2.size() || v1.empty()) return 0.0f;

                //     float dot_product = 0.0f, norm_a = 0.0f, norm_b = 0.0f;
                //     for (size_t i = 0; i < v1.size(); ++i) {
                //         dot_product += v1[i] * v2[i];
                //         norm_a += v1[i] * v1[i];
                //         norm_b += v2[i] * v2[i];
                //     }
                    
                //     float denominator = std::sqrt(norm_a) * std::sqrt(norm_b);
                //     return (denominator == 0) ? 0.0f : dot_product / denominator;
                // }
        private:

    };
}