#include <iostream>
#include "Random.h"

Random Random::instance = Random();

float Random::GetUniformRandom() {
    return distribution(generator);
}

Vec3 Random::GetWorldRandomHemisphereDirection(Vec3 normal) {

    Vec3 b, t;
    normal.createBitangentAndTangent(b, t);

    float r1 = distribution(generator);
    float r2 = distribution(generator);

    Vec3 sample = GetRandomHemisphereDirection(r1, r2);
    Vec3 sample_dir(
            sample.x * b.x + sample.y * normal.x + sample.z * t.x,
            sample.x * b.y + sample.y * normal.y + sample.z * t.y,
            sample.x * b.z + sample.y * normal.z + sample.z * t.z);
    return sample_dir;
}

/*
 * Uniform sampling over Hemisphere
 * x = sin(Theta) * cos(Phi)
 * y = cos(Theta)
 * z = sin(Theta) * sin(Phi)
 */
Vec3 Random::GetRandomHemisphereDirection(float cos_theta, float phi) const {

    // Compute sin(theta) from cos(theta) using
    // cos(x)² + sin((x)² = 1

    phi *= 2 * M_PI_F; // [0, 1] to [0, 2PI]

//    float sin_theta = sqrtf(1 - cos_theta * cos_theta);
//    float x = sin_theta * COS_LOOKUP(phi - M_PI_F);
//    float z = cos_theta * SIN_LOOKUP(phi - M_PI_F);
//    return Vec3(x, cos_theta, z);

    // This one faster
#ifdef USE_TRIGO_LOOKUP
    float cos_phi = COS_LOOKUP(phi - M_PI_F); // Macros expect [-PI, PI] values
    float sin_phi = SIN_LOOKUP(phi - M_PI_F);
#else
    float cos_phi = std::cos(phi);
    float sin_phi = std::sin(phi);
#endif

    float sin_theta = sqrtf(1 - cos_theta * cos_theta);
    return Vec3(sin_theta * cos_phi,
                cos_theta,
                sin_theta * sin_phi);
}

/*
 * Uniform sampling over Hemisphere
 * x = sin(Theta) * cos(Phi)
 * y = cos(Theta)
 * z = sin(Theta) * sin(Phi)
 */
Vec3 GetRandomHemisphereDirectionCosineSampling(float u1, float u2) {

    // Compute sin(theta) from cos(theta) using
    // cos(x)² + sin((x)² = 1

    const float r = sqrtf(u1);
    const float theta = 2 * M_PI_F * u2;

    const float x = r * cosf(theta);
    const float z = r * sinf(theta);

    return Vec3(x, sqrtf(1.f - u1), z);

//    float sin_theta = sqrtf(1 - cos_theta * cos_theta);
//    float x = sin_theta * cos_lut[(int)(COS_LUT_LENGTH * phi)];
//    float z = cos_theta * sin_lut[(int)(COS_LUT_LENGTH * phi)];
//    return Vec3(x, cos_theta, z);

}


/**
 * theta = arctan(sqrt(-m² * log(1 - u)))
 * phi = 2PI * u
 *
 */
Vec3 Random::BeckmannSample(float roughness) {
    // Compute tan^2(theta) and phi for Beckmann distribution sample
    float tan2Theta, phi;
    float u1 = Random::GetInstance().GetUniformRandom();
    float u2 = Random::GetInstance().GetUniformRandom();

    phi = u2 * 2 * M_PI_F; // [0, 1] to [0, 2PI]

    float logSample = std::log(u1 + 0.0000001f);
//    if (std::isinf(logSample))
//        logSample = 0;
    /*float theta = atanf(sqrt(-(roughness*roughness) * logSample));
    float cosTheta = cosf(theta);
    float sinTheta = sinf(theta);*/
    tan2Theta = -roughness * roughness * logSample;

    // Map sampled Beckmann angles to normal direction _wh_
    float cosTheta = 1 / std::sqrt(1 + tan2Theta);
    float sinTheta = std::sqrt(std::max(0.f, 1 - cosTheta * cosTheta));

#ifdef USE_TRIGO_LOOKUP
    Vec3 wh {sinTheta * COS_LOOKUP(phi - M_PI_F), // Macros expect [-PI, PI] inputs
             cosTheta,
             sinTheta * SIN_LOOKUP(phi - M_PI_F)};
#else
    Vec3 wh {sinTheta * std::cos(phi),
             cosTheta,
             sinTheta * std::sin(phi)};
#endif

//        if (!SameHemisphere(wo, wh)) wh = -wh;

    return wh;
}
