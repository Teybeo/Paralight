#ifndef TEXTURE_H
#define TEXTURE_H

#define UV_MAPPING 0
#define SPHERICAL_MAPPING 1

typedef struct TextureInfo {

    int width;
    int height;
    int byte_offset;
    char mapping;

} TextureInfo;

float3 Sample(image2d_t image, float u, float v);
float3 Sample_Buffer(const global char* image_array, const global TextureInfo* info_array, int id, float2 uv);
float3 Sample_Envmap(image2d_t image, float3 direction);
float3 Sample_Spheremap(image2d_t image, float3 direction);

float2 SphericalToCartesian(float3 direction);

#endif