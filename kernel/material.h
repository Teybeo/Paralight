#ifndef _MATERIAL_H
#define _MATERIAL_H

#include "brdf.h"
#include "objects.h"
#include "texture.h"

void EvaluateMaterial(float3* ray_direction, float3* material, int index, float3 normal, float3 shading_normal, float2 uv, global Brdf* brdfs, char brdf_bitfield, global char* texture_array, global TextureInfo* info_array, RNG_SEED_ARGS);
float3 EvaluateParameter(float3 scalar, char tex_index, float2 uv, global char* texture_array, global TextureInfo* info_array);
float3 EvaluateNormalParameter(float3 scalar, const char tex_index, const float3 normal, const float2 uv, const global char* texture_array, const global TextureInfo* info_array);
float3 TangentToWorld(float3 vec, float3 normal);

#endif