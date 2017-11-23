#ifndef BRDF_CL
#define BRDF_CL

#define LAMBERTIAN        (1 << 0)
#define MICROFACET        (1 << 1)
#define MIRROR            (1 << 2)

// Max type of this struct is float3 (aka float4)
// So aligned on 16 bytes
typedef struct Brdf {
    float3 albedo;             // [0  - 15]
    float3 reflection;         // [16 - 31]
    float roughness;           // [32 - 35]
    char type;                 // [36]
    char albedo_map_index;     // [37]
    char roughness_map_index;  // [38]
    char reflection_map_index; // [39]
    char normal_map_index;     // [40]
    char packed_metal_rough;   // [41]
    char use_metalness;        // [42]
    char metalness_map_index;  // [43]
    float metalness;           // [44 - 47]
    // Options must be 16-aligned
    // 48 bytes total
} Brdf;

#define RNG_SEED_ARGS uint* seed_x, uint* seed_y
#define RNG_SEED seed_x, seed_y

float3 Sample_Mirror_f(float3 reflectance, float3 outgoing_dir, float3* incoming_dir, float* pdf, float3 normal, RNG_SEED_ARGS);
float3 Sample_Lambertian_f(float3 albedo, float3 outgoing_dir, float3* incoming_dir, float* pdf, float3 normal, RNG_SEED_ARGS);
//float3 Sample_Lambertian_f(Brdf brdf, float3 outgoing_dir, float3* incoming_dir, float* pdf, float3 normal, uint* seed_x, uint* seed_y);
float3 Sample_Microfacet_f(float roughness, float3 reflection, float3 outgoing_dir, float3* incoming_dir, float* pdf, float3 normal, RNG_SEED_ARGS);
float3 GetRandomHemisphereDirectionCosine(RNG_SEED_ARGS);
float3 GetRandomHemisphereDirectionUniform(RNG_SEED_ARGS);
float3 BeckmannSample(float roughness, RNG_SEED_ARGS);
float Beckmann(const float3 normal, const float3 half_vector, const float roughness);
float GeometryCookTorrance(const float3 normal, const float3 outgoing_dir, const float3 incoming_dir);
float3 WorldToTangent(float3 normal, float3 vec);
float3 reflect(float3 vec, float3 normal);
float3 Fresnel(const float3 F0, const float3 incoming_dir, const float3 normal);
float getRandom(RNG_SEED_ARGS);
char SampleBrdfType(float* weight, char brdf_type, char brdf_bitfield, RNG_SEED_ARGS);

#endif