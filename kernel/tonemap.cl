#include "tonemap.h"

#include "macros.h"

// TFW no matrix in OpenCL :'(
typedef struct mat3 {
    float4 x, y, z;
} mat3;

float3 RGB2Yxy(float3 rgb);
float3 Yxy2RGB(float3 Yxy);
float3 mat3_mul_float3(mat3 mat, float3 vec);

#define EXPOSURE 1.0f
#define WHITEPOINT 1.0f

void tonemap(float3* color) {

    if (all(isequal(*color, (float3)(0))))
        return;

    float3 Yxy = RGB2Yxy(*color);
    float lum = Yxy.x;
    lum *= EXPOSURE;
    lum = (lum + ((lum * lum) / (WHITEPOINT * WHITEPOINT))) / (lum + 1.0f);
    Yxy.x = lum;
    *color = Yxy2RGB(Yxy);
}

float3 mat3_mul_float3(mat3 mat, float3 vec) {
    float3 res;
    float4 v = (float4)(vec, 1);
    float4 a, b, c;
    a = (float4){mat.x.x, mat.y.x, mat.z.x, 0};
    b = (float4){mat.x.y, mat.y.y, mat.z.y, 0};
    c = (float4){mat.x.z, mat.y.z, mat.z.z, 0};
    res.x = dot(a, v);
    res.y = dot(b, v);
    res.z = dot(c, v);
    return res;
}

float3 RGB2Yxy(float3 rgb) {

    // These doesn't seem to be "official" CIE RGB to XYZ conversion matrices...
    const mat3 RGB2XYZ = {
            {0.5141364f, 0.3238786f,  0.16036376f, 0},
            {0.265068f,  0.67023428f, 0.06409157f, 0},
            {0.0241188f, 0.1228178f,  0.84442666f, 0},
    };
    float3 XYZ = mat3_mul_float3(RGB2XYZ, rgb);
    float3 Yxy = XYZ;
    Yxy.yz = XYZ.xy / dot((float3)(1), XYZ.xyz);
    return Yxy;
}

float3 Yxy2RGB(float3 Yxy) {

    // These doesn't seem to be "official" CIE RGB to XYZ conversion matrices...
    const mat3 XYZ2RGB = {
            {2.5651f, -1.1665f, -0.3986f, 0},
            {-1.0217f, 1.9777f,  0.0439f, 0},
            {0.0753f, -0.2543f,  1.1892f, 0},
    };

    float3 XYZ;
    XYZ.x = Yxy.x * Yxy.y / Yxy.z;
//    DEBUG_PIXEL(10, 10)
    XYZ.y = Yxy.x;
    XYZ.z = Yxy.x * (1.0f - Yxy.y - Yxy.z) / Yxy.z ;
    return mat3_mul_float3(XYZ2RGB, XYZ);
}