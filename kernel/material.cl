#include "material.h"

void EvaluateMaterial(float3* ray_direction, float3* material, int index, float3 normal, float3 shading_normal, float2 uv, global Brdf* brdfs, char brdf_bitfield, global char* texture_array, global TextureInfo* info_array, RNG_SEED_ARGS) {

//    float3 ratio = 1;
    float brdf_weight;
    float3 outgoing_dir = -*ray_direction;
    float pdf = 1;

    char sampled_brdf_type = SampleBrdfType(&brdf_weight, brdfs[index].type, brdf_bitfield, RNG_SEED);

    *material = brdf_weight;

    if (sampled_brdf_type == LAMBERTIAN) {

        float3 albedo = EvaluateParameter(brdfs[index].albedo, brdfs[index].albedo_map_index, uv, texture_array, info_array);
        float3 f = Sample_Lambertian_f(albedo, outgoing_dir, ray_direction, &pdf, shading_normal, RNG_SEED);
//        float cos_factor = max(dot(normal, *ray_direction), 0.f);
        float cos_factor = max(dot(shading_normal, *ray_direction), 0.f);
//        float cos_factor = max(dot(shading_normal, *ray_direction), 0.f) * (dot(normal, *ray_direction) > 0);

        // For lambertian + microfacet weighting, lambertian need to be scaled by 1 - fresnel
        // As fresnel is included into microfacet brdf, need to use fresnel with the same distribution
        // That's why we're taking a microfacet sample inside the lambertian... yes dirty :<
        if ((brdfs[index].type & brdf_bitfield) & MICROFACET) {
            float3 reflection = EvaluateParameter(brdfs[index].reflection, brdfs[index].reflection_map_index, uv, texture_array, info_array);
            float roughness = EvaluateParameter(brdfs[index].roughness, brdfs[index].roughness_map_index, uv, texture_array, info_array).x;

            float3 micro_normal = BeckmannSample(roughness, RNG_SEED);
            micro_normal = WorldToTangent(normal, micro_normal);
            float3 specular_ray = normalize(reflect(outgoing_dir, micro_normal));

            float3 half_vector = normalize(specular_ray + outgoing_dir);
            f *= 1.f - Fresnel(reflection, specular_ray, half_vector);
        }
        *material = (f * cos_factor) / pdf;

    } else if (sampled_brdf_type == MICROFACET) {

        float3 reflection = EvaluateParameter(brdfs[index].reflection, brdfs[index].reflection_map_index, uv, texture_array, info_array);
        float roughness = EvaluateParameter(brdfs[index].roughness, brdfs[index].roughness_map_index, uv, texture_array, info_array).x;

        float3 f = Sample_Microfacet_f(roughness, reflection, outgoing_dir, ray_direction, &pdf, shading_normal, RNG_SEED);

//        float cos_factor = max(dot(normal, *ray_direction), 0.f);
        float cos_factor = max(dot(shading_normal, *ray_direction), 0.f) * (dot(normal, *ray_direction) > 0);
//        float cos_factor = max(dot(shading_normal, *ray_direction), 0.f);

        *material = (f * cos_factor) / pdf;
    }
    else if (sampled_brdf_type == MIRROR) {

        float3 f = Sample_Mirror_f(brdfs[index].reflection, outgoing_dir, ray_direction, &pdf, normal, RNG_SEED);

        *material = f / pdf;
    }
    else {
        *material = 0;
        return;
    }

//    return material;
}


float3 EvaluateParameter(float3 scalar, char tex_index, float2 uv, global char* texture_array, global TextureInfo* info_array) {
    if (tex_index == -1)
        return scalar;
    else {
        return Sample_Buffer(texture_array, info_array, tex_index, uv);
    }
}

float3 EvaluateNormalParameter(float3 scalar, const char tex_index, const float3 normal, const float2 uv, const global char* texture_array, const global TextureInfo* info_array) {
    if (tex_index == -1)
        return scalar;
    else {
        float3 shading_normal = Sample_Buffer(texture_array, info_array, tex_index, uv);
        shading_normal = shading_normal * 2 - 1;
        shading_normal.y *= -1;
        shading_normal = TangentToWorld(shading_normal, normal);
        return shading_normal;
    }
}

// Transform this vector from tangent space to world space
// The tangent space is constructed from the worldspace normal passed in argument
float3 TangentToWorld(float3 vec, const float3 normal) {

    float3 orthogonal = (float3)(0.f, -1.f, 0.f);

    float3 t = normalize(cross(normal, orthogonal));
    float3 b = normalize(cross(normal, t));

    return (float3) {
            vec.x * t.x  +  vec.y * b.x  +  vec.z * normal.x,
            vec.x * t.y  +  vec.y * b.y  +  vec.z * normal.y,
            vec.x * t.z  +  vec.y * b.z  +  vec.z * normal.z};

}

