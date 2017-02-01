#ifndef OPENCL_RANDOM_H
#define OPENCL_RANDOM_H

#include <random>
#include "math/Vec3.h"
#include "math/TrigoLut.h"

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
    Vec3 GetWorldRandomHemisphereDirection(Vec3 normal);
    Vec3 GetRandomHemisphereDirection(float cos_theta, float phi) const;

    float GetUniformRandom();

    Vec3 BeckmannSample(float roughness);
};



#endif //OPENCL_RANDOM_H
