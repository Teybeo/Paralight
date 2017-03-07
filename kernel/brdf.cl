#include "brdf.h"
#include "macros.h"

//bool isZero(float3 vec) {
//    return  vec.x == 0
//            && vec.y == 0
//            && vec.z == 0;
//}
float3 Sample_Mirror_f(float3 reflectance, float3 outgoing_dir, float3* incoming_dir, float* pdf, float3 normal, RNG_SEED_ARGS) {

    *incoming_dir = reflect(outgoing_dir, normal);

    *pdf = 1;

    return reflectance;
}

float3 Sample_Lambertian_f(float3 albedo, float3 outgoing_dir, float3* incoming_dir, float* pdf, float3 normal, RNG_SEED_ARGS) {

    // Cosine Hemisphere sampling, PDF = cos(theta) / Pi
    *incoming_dir = WorldToTangent(normal, GetRandomHemisphereDirectionCosine(RNG_SEED));
    float cos_theta = dot(*incoming_dir, normal);
    *pdf = cos_theta / M_PI_F;

    return albedo / M_PI_F;
}

float3 Sample_Microfacet_f(float roughness, float3 reflection, float3 outgoing_dir, float3* incoming_dir, float* pdf, float3 normal, RNG_SEED_ARGS) {

    float3 micro_normal = BeckmannSample(roughness, RNG_SEED);
    micro_normal = normalize(micro_normal);

    micro_normal = WorldToTangent(normal, micro_normal);
    micro_normal = normalize(micro_normal);

    *incoming_dir = reflect(outgoing_dir, micro_normal);
    *incoming_dir = normalize(*incoming_dir);

//    DEBUG_PIXEL(200, 200)
//    if (length(*incoming_dir) > 1.f) {
//        printf("non-unit vec length: %.20f\n", length(*incoming_dir));
//        PRINT_VEC(*incoming_dir)
//    }
    if (dot(normal, *incoming_dir) <= 0)
        return 0;
    if (dot(normal, outgoing_dir) <= 0)
        return 0;
    // Uniform sampling
//    incoming_dir = Random::GetInstance().GetWorldRandomHemisphereDirection(normal);

    // H == M, as long as i and o are not under the M hemishpere, guaranteed from above checks
    float3 half_vector = micro_normal;

    float3 fresnel = Fresnel(reflection, *incoming_dir, micro_normal);
    float ndf = Beckmann(normal, micro_normal, roughness);
    float geom = GeometryCookTorrance(normal, outgoing_dir, *incoming_dir);
//    float geom = GeometrySmithOrGGX(normal, outgoing_dir, *incoming_dir);
    float denominator = 4.f * dot(normal, outgoing_dir) * dot(normal, *incoming_dir);

    *pdf = (ndf * dot(normal, half_vector)) / (4.f * dot(half_vector, outgoing_dir));
    *pdf = *pdf ? *pdf : 1; // Avoid divid by 0
//    pdf = 1 / (2 * M_PI_F);

//    *pdf = 1;
//    return (geom / 10.f);
//    return (ndf / 10.f);
//    return ((denominator) / 1.f);
//    return ((fresnel * geom * ndf) / 1.f);
    return (fresnel * geom * ndf) / denominator;
//    return (float3)(denominator, denominator, denominator);
}
/**
  * theta = arctan(sqrt(-m² * log(1 - u)))
  * phi = 2PI * u
  */
float3 BeckmannSample(float roughness, RNG_SEED_ARGS) {
    // Compute tan^2(theta) and phi for Beckmann distribution sample
    float tan2Theta, phi;
    float u1 = getRandom(RNG_SEED);
    float u2 = getRandom(RNG_SEED);
    phi = u2 * 2.f * M_PI_F;
    float logSample = log(u1 + 0.0000001f);

    /*float theta = atanf(sqrt(-(roughness*roughness) * logSample));
    float cos_theta = cosf(theta);
    float sin_theta = sinf(theta);*/
//    roughness = min(10.f, roughness);
    tan2Theta = -roughness * roughness * logSample;


    // Map sampled Beckmann angles to normal direction _wh_
    float cos_theta = 1.f / sqrt(1.f + tan2Theta);
    float sin_theta = sqrt(max(0.f, 1.f - cos_theta * cos_theta));

    float3 wh = {cos(phi) * sin_theta, cos_theta, sin(phi) * sin_theta};

    return wh;
}
/*
 * a = angle between N and H
 * exp(-tan(a)² / m²) / ( PI * m² * cos(a)^4)
 */
float Beckmann(float3 normal, float3 half_vector, float roughness) {
    float n_dot_h = min(1.f, dot(normal, half_vector));
//    float n_dot_h = min(0.9999f, dot(normal, half_vector));
//    if (n_dot_h < 0.001f)
//        return 0.f;
    float alpha = acos(n_dot_h);
    float tan_a_2 = tan(alpha) * tan(alpha);
    float cos_a_4 = cos(alpha) * cos(alpha) * cos(alpha) * cos(alpha);
//    float roughness_2 = max(0.0001f, roughness * roughness);
    float roughness_2 = roughness * roughness;

    return exp(-tan_a_2 / roughness_2) / (M_PI_F * roughness_2 * cos_a_4);
}

float GeometryCookTorrance(const float3 normal, const float3 outgoing_dir, const float3 incoming_dir) {

    float3 half_vector = normalize(incoming_dir + outgoing_dir);

    float n_dot_h = dot(normal, half_vector);
    float v_dot_h = dot(outgoing_dir, half_vector);
    float n_dot_v = dot(normal, outgoing_dir);
    float n_dot_l = dot(normal, incoming_dir);

    float res = (2.f * n_dot_h) / v_dot_h;
    return min(1.f, min(res * n_dot_v, res * n_dot_l));
}
/*
 * Uniform sampling over Hemisphere
 * x = sin(Theta) * cos(Phi)
 * y = cos(Theta)
 * z = sin(Theta) * sin(Phi)
 */
float3 GetRandomHemisphereDirectionUniform(RNG_SEED_ARGS) {

    // cos(r1) == r1
    float cos_theta = getRandom(RNG_SEED);
    float r2        = getRandom(RNG_SEED);

    // Compute sin(theta) from cos(theta) using
    // cos(x)² + sin((x)² = 1
    float phi = r2 * M_PI_F * 2.f;

    float cos_phi = cos(phi);
    float sin_phi = sin(phi);

    float sin_theta = sqrt(1.f - (cos_theta * cos_theta));
    return normalize((float3){cos_phi * sin_theta, cos_theta, sin_phi * sin_theta});
}
/**
 * Cosine sampling over Hemisphere
 */
float3 GetRandomHemisphereDirectionCosine(RNG_SEED_ARGS) {

    float u1 = getRandom(RNG_SEED);
    float u2 = getRandom(RNG_SEED);

    const float r = sqrt(u1);
    const float theta = 2.f * M_PI_F * u2;

    const float x = r * cos(theta);
    const float z = r * sin(theta);

    return (float3)(x, sqrt(max(0.f, 1.f - u1)), z);
}
// Schlick approximation
/**
 * if dot < 0, (1 - dot) > 1, fresnel > 1
 * F0 + (1 - F0) * (1 - a)^5
 */
float3 Fresnel(const float3 F0, const float3 incoming_dir, const float3 normal) {

    float n_dot_i = max(dot(incoming_dir, normal), 0.f);
    return F0 + (1.f - F0) * powr((1.f - n_dot_i), 5.f);
}

/**
 * Reflect vec around this vector
 * Both vectors must be normalized
 */
float3 reflect(float3 vec, float3 normal) {

    return (dot(vec, normal) * normal * 2.f) - vec;
}
/**
 * Transform a vector into the space tangent to the normal
 */
float3 WorldToTangent(float3 normal, float3 vec) {
    // TODO: Some vec are not normalized, check where they come from
//    vec = (float3)(0, 1, 0);
    vec = normalize(vec);
    float3 b, t;
    if (fabs(normal.x) > fabs(normal.y))
        t = normalize((float3){-normal.z, 0, normal.x});
    else
        t = normalize((float3){0, -normal.z, normal.y});

    b = normalize(cross(normal, t));
    return normalize((float3) {
            vec.x * b.x + vec.y * normal.x + vec.z * t.x,
            vec.x * b.y + vec.y * normal.y + vec.z * t.y,
            vec.x * b.z + vec.y * normal.z + vec.z * t.z
    });
}


// random number generator from https://github.com/gz/rust-raytracer
float getRandom(unsigned int *seed0, unsigned int *seed1) {
    *seed0 = 36969 * ((*seed0) & 65535) + ((*seed0) >> 16);  // hash the seeds using bitwise AND and bitshifts
    *seed1 = 18000 * ((*seed1) & 65535) + ((*seed1) >> 16);

    unsigned int ires = ((*seed0) << 16) + (*seed1);

    // Convert to float
    union {
        float f;
        unsigned int ui;
    } res;

    res.ui = (ires & 0x007fffff) | 0x40000000;  // bitwise AND, bitwise OR

    return (res.f - 2.f) * 0.5f;
}

char SampleBrdfType(float* weight, char brdf_type, char brdf_bitfield, RNG_SEED_ARGS) {

    // popcount count the numbers of bit set to 1
    char matching_brdf_count = popcount(brdf_type & brdf_bitfield);

    // 0 matching brdf
    if (matching_brdf_count == 0) {
        return 0;
    }
    // 1 matching brdf, select and return it
    else if (matching_brdf_count == 1) {
        *weight = 1;
        return brdf_type & brdf_bitfield;
    }
    // 2 matchings brdfs, choose one at random and weight 2x more
    else {
        *weight = 2;
        float rand = getRandom(RNG_SEED);
        return (rand > 0.5f) ? LAMBERTIAN : MICROFACET;
        // This assumes the only possible dual combination is LAMBERTIAN + MICROFACET
    }

}
