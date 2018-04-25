#ifndef RENDER
#define RENDER

#include "texture.h"
#include "brdf.h"
#include "objects.h"
#include "bvh.h"

// IMPORTANT: per the spec, float3 == float4 for size and alignement
typedef struct mat4x4 {
    float3 x;
    float3 y;
    float3 z;
    float3 w;
} mat4x4;

// Max type of this struct is float3 (aka float4)
// So aligned on 16 bytes
typedef struct Options {
    float3 origin;                      // [0  - 15]
    mat4x4 rotation;                    // [16 - 79]
    float fov;                          // [80 - 83]
    int triangle_count;                 // [92 - 95]
    short sample_count;                 // [96 - 97]
    short bounce_count;                 // [98 - 99]
    short frame_number;                 // [100 - 101]
    char use_direct_lighting;           // [102]
    char use_distant_env_lighting;      // [103]
    char brdf_bitfield;                 // [104]
    char use_tonemapping;               // [105]
    char accum_clear_bit;               // [106]
    char sphere_count;                  // [107]
    char plane_count;                   // [108]
    char debug;                         // [109]
//    char pad14[2];                    // [110 - 111]
    // 112 bytes = 16 * 7
} Options;

#endif