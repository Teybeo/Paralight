#include "texture.h"

#include "macros.h"

int2 NormalizedToImageBounds(float2 uv, int width, int height);

float3 Sample_Buffer(const global char* image_array, const global TextureInfo* info_array, int id, float2 uv) {

    uchar3 pixel;

    const global TextureInfo* info = info_array + id;

    uv -= floor(uv);

    int2 xy = NormalizedToImageBounds(uv, info->width, info->height);

    int pixel_offset = (info->width * xy.y + xy.x);

    const global uchar* texture = (const global uchar*) (image_array + info->byte_offset); // * 4 = Get Psyched Novidia

    const global uchar* pixel_byte_ptr = (const global uchar*) (texture + pixel_offset * 3); // RGB
//    const global uchar* pixel_byte_ptr = (const global uchar*) (texture + pixel_offset * 4); // RGBA

    pixel = (uchar3)(pixel_byte_ptr[0], pixel_byte_ptr[1], pixel_byte_ptr[2]);

    return (float3)(pixel.x, pixel.y, pixel.z) / 255.f;
}

/*
 * direction must be normalized
 */
float3 Sample_Envmap(image2d_t image, float3 direction) {

    float2 uv = SphericalToCartesian(direction);

    return Sample(image, 1-uv.x, uv.y);
}

/*
 * direction must be normalized
 */
float3 Sample_Spheremap(image2d_t image, float3 direction) {

    float2 uv = SphericalToCartesian(direction);

    // The u coordinate is reversed because we are sampling the outside of a sphere
    return Sample(image, -uv.x, uv.y);
}

/**
 * UV coord in arguments must be normalized [0, 1]
 * read_image without sampler default to non-normalized coord
 */
float3 Sample(image2d_t image, float u, float v) {
    int width = get_image_width(image);
    int height = get_image_height(image);
    int x = (int) round(u * (width - 1));
    int y = (int) round(v * (height - 1));
    return read_imagef(image, (int2)(x, y)).xyz;
}

// [0, 1] => [0, size-1]
int2 NormalizedToImageBounds(float2 uv, int width, int height) {
    int x = (int) round(uv.x * (width - 1));
    int y = (int) round(uv.y * (height - 1));
    return (int2)(x, y);
}

float2 SphericalToCartesian(float3 direction) {

    float polar = acos(direction.y);                 // acos() => [0, PI]
    float azimuth = atan2(direction.x, direction.z); // atan() => [-PI, PI]

    float u = (azimuth + M_PI_F) / (2 * M_PI_F); // [-PI, PI] => [0, 1]
    float v = polar / M_PI_F;                    // [0, PI]   => [0, 1]

    return (float2)(u, v);
}
