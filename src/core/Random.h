#ifndef OPENCL_RANDOM_H
#define OPENCL_RANDOM_H

#include <random>
#include "math/Vec3.h"
#include "math/TrigoLut.h"

//#define COSINE_SAMPLING

class Random {

    std::minstd_rand generator; // Linear Congruential generator
    std::uniform_real_distribution<float> distribution;

    static Random instance;
    Random() {
        // Standard says the range is [0, 1[ but implementation can still produce 1.f
        // https://gcc.gnu.org/bugzilla/show_bug.cgi?id=63176
        distribution = std::uniform_real_distribution<float>(0, 0.999999f);
    }
public:

    static Random& GetInstance() {
        return instance;
    }

    float GetUniformRandom();

    Vec3 BeckmannSample(float roughness);
    Vec3 GetWorldRandomHemishpereDirectionUniform(Vec3 normal);
    Vec3 GetWorldRandomHemishpereDirectionCosine(Vec3 normal);
};



#endif //OPENCL_RANDOM_H
