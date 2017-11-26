#include "Texture.h"

#include "math/TrigoLut.h"
#include "hdrloader/hdrloader.h"
#include "math/Vec3.h"
#include <SDL_image.h>
#include <iostream>
#include <cstdio>
#include <cassert>
#include "app/Chronometer.h"

using std::string;
using std::cout;
using std::endl;

template class ImageTexture<float>;
template class ImageTexture<uint8_t>;

Vec3 SphericalToCartesian(const Vec3& direction) ;

template <typename T>
ImageTexture<T>::ImageTexture(const std::string& path, bool store_in_linear) : path(path) {

    cout << "Loading [" + path + "] ..." << endl;
    
    Chronometer chrono;

    string ext = path.substr(path.find_last_of(".") + 1);

    if (ext == "hdr") {
        Load_HDR();
    }
    else {
        Load_Generic(store_in_linear);
    }

    cout << width << " x " << height << " x " << int(channel_count) << " = " << (width * height * sizeof(T) * channel_count) / 1024 << " Ko";
    cout << ", " << chrono.GetSeconds() << " s" << endl;
}

template <typename T>
ImageTexture<T>::~ImageTexture() {
    cout << "ImageTexture [" << path << "] destroyed" << endl;
    delete[] data;
    data = nullptr;
}

/**
 * @return The size in bytes of this texture
 */
template<typename T>
size_t ImageTexture<T>::GetSize() const {
    return sizeof(T) * channel_count * width * height;
}

template <typename T>
void ImageTexture<T>::Load_Generic(bool store_in_linear) {

    SDL_Surface* surface = IMG_Load(path.c_str());

    if (surface == nullptr) {
        std::cerr << "Error loading [" << path << "]" << endl;
        std::cerr << SDL_GetError() << endl;
        throw std::bad_exception();
    }
    
    width = (size_t) surface->w;
    height = (size_t) surface->h;

    SDL_PixelFormat* format = SDL_AllocFormat(SDL_PIXELFORMAT_RGB24); // 3 bpp
//    SDL_PixelFormat* format = SDL_AllocFormat(SDL_PIXELFORMAT_BGR888);  // 4 bpp

    char Bpp = format->BytesPerPixel;
    channel_count = Bpp; // Meh
    data = new T[Bpp * width * height];

    auto error = SDL_ConvertPixels(surface->w, surface->h, surface->format->format, surface->pixels, surface->pitch, format->format, data, int(width * Bpp));
    if (error) {
        std::cerr << "Error Loading [" << path << "]" << endl;
        std::cerr << SDL_GetError();
    
        SDL_Surface* converted_surface = SDL_ConvertSurface(surface, format, 0);
        if (converted_surface == nullptr) {
            std::cerr << "Error loading [" << path << "]" << endl;
            std::cerr << "Failed to convert surface" << endl;
            std::cerr << SDL_GetError() << endl;
            throw std::bad_exception();
        }
        
        auto converted_pixels = static_cast<T*>(converted_surface->pixels);
        
        std::copy(converted_pixels, converted_pixels + (channel_count * width * height), data);
        SDL_FreeSurface(converted_surface);
    }

    SDL_FreeSurface(surface);

    if (store_in_linear) {
        ConvertToLinear();
    }
}

template <typename T>
void ImageTexture<T>::Load_HDR() {

    Chronometer chrono;

    HDRLoaderResult result;
    bool ret = HDRLoader::load(path.c_str(), result);
    if (ret == false) {
        cout << "Error loading HDR envmap [" << path << "]\n";
        throw std::bad_exception();
    }
    cout << "HDRLoader in " << chrono.GetSeconds() << " s" << endl;

    chrono.Restart();
    
    width = (size_t) result.width;
    height = (size_t) result.height;
    channel_count = 4;
//    channel_count = 3;
    data = new T[channel_count * width * height];

    for (size_t i = 0; i < width * height; ++i) {
        data[channel_count * i + 0] = result.cols[3 * i + 0];
        data[channel_count * i + 1] = result.cols[3 * i + 1];
        data[channel_count * i + 2] = result.cols[3 * i + 2];
        data[channel_count * i + 3] = 0;
    }

    cout << "HDR copied in " << chrono.GetSeconds() << " s" << endl;

    free(result.cols);
}


template <typename T>
Vec3 ImageTexture<T>::Evaluate(const Vec3& uv) {
    return Sample(uv.x, uv.y);
}

/**
 * direction must be normalized
 */
template <typename T>
Vec3 ImageTexture<T>::SampleEnvmap(const Vec3& direction) {

    Vec3 uv = SphericalToCartesian(direction);

    return Sample(1 - uv.x, uv.y);
}

template <typename T>
Vec3 ImageTexture<T>::SampleSpheremap(const Vec3& direction) {

    Vec3 uv = SphericalToCartesian(direction);

    // The u coordinate is reversed because we are sampling the outside of a sphere
    return Sample(-uv.x, uv.y);
}

/**
 * UV coord must be normalized [0, 1]
 */
template <typename T>
inline Vec3 ImageTexture<T>::Sample(float u, float v) {

    int x = (int) round(u * (width - 1));
    int y = (int) round(v * (height - 1));

    int offset = channel_count * (y * width + x);

    return Vec3 (data[offset + 0],
                 data[offset + 1],
                 data[offset + 2]);
}

template <>
inline Vec3 ImageTexture<uint8_t>::Sample(float u, float v) {

    u -= std::floor(u);
    v -= std::floor(v);

    int x = (int) round(u * (width - 1));
    int y = (int) round(v * (height - 1));

    int offset = channel_count * (y * width + x);

    return Vec3 (data[offset + 0] / 255.f,
                 data[offset + 1] / 255.f,
                 data[offset + 2] / 255.f);
}

Vec3 SphericalToCartesian(const Vec3& direction) {

#ifdef USE_TRIGO_LOOKUP
    float polar = ACOS_LOOKUP(direction.y);      // acos() => [0, PI]
#else
    float polar = acosf(direction.y);            // acos() => [0, PI]
#endif

    float azimuth = atan2f(direction.x, direction.z);// atan() => [-PI, PI]

    float u = (azimuth + M_PI_F) / (2 * M_PI_F); // [-PI, PI] => [0, 1]
    float v = (polar / M_PI_F);                  // [0, PI]   => [0, 1]

    return {u, v, 0};
}

template <typename T>
void ImageTexture<T>::ConvertToLinear() {
    cout << "ConvertToLinear generic template" << endl;
}

template<>
void ImageTexture<uint8_t>::ConvertToLinear() {
#pragma omp parallel for
    for (size_t i = 0; i < channel_count * width * height; ++i) {
        data[i] = static_cast<uint8_t>(std::pow(data[i] / 255.f, 2.2f) * 255);
    }
}

template<>
void ImageTexture<float>::ConvertToLinear() {
}

template <typename T>
string ImageTexture<T>::GetName() {
    return path;
}

