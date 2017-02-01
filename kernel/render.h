#ifndef RENDER
#define RENDER

#include "texture.h"
#include "brdf.h"
#include "objects.h"
#include "bvh.h"

// IMPORTANT: per the spec, float3 == float4 for size and alignement
typedef struct float4x4 {
    float3 x;
    float3 y;
    float3 z;
    float3 w;
} float4x4;

// Max type of this struct is float3 (aka float4)
// So aligned on 16 bytes
typedef struct Options {
    float3 origin;                      // [0  - 15]
    float4x4 rotation;                  // [16 - 79]
    float fov;                          // [80 - 83]
    int triangle_count;                 // [84 - 87]
    short sample_count;                 // [88 - 89]
    short bounce_count;                 // [90 - 91]
    short frame_number;                 // [92 - 93]
    char use_direct_lighting;           // [94]
    char use_distant_env_lighting;      // [95]
    char brdf_bitfield;                 // [96]
    char use_tonemapping;               // [97]
    char accum_clear_bit;               // [98]
    char sphere_count;                  // [99]
    char plane_count;                   // [100]
    char debug;                         // [101]
//    char pad14[10];                   // [102 - 111]
    // Options must be 16-aligned so 10 bytes padding
    // 112 bytes = 16 * 7
} Options;

#endif