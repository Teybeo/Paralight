#include "Texture.h"

#include "math/TrigoLut.h"
#include "hdrloader/hdrloader.h"
#include "math/Vec3.h"
#include <SDL_image.h>
#include <iostream>
#include <cstdio>
#include <cassert>

using std::string;
using std::cout;
using std::endl;

template class Texture<float>;
template class Texture<uint8_t>;

template <typename T>
Texture<T>::Texture(const std::string& path, bool store_in_linear) : path(path) {

    cout << "Loading [" + path + "] ..." << endl;

    Uint32 start = SDL_GetTicks();

    string ext = path.substr(path.find_last_of(".") + 1);


    if (ext == "hdr") {
        Load_HDR();
    }
    else if (ext == "fpm") {
        Load_FPM();
    }
    else {
        Load_Generic(store_in_linear);
    }

    cout << (width * height * sizeof(T) * 4) / 1024 << "Ko,  " << width << " x " << height << endl;
    cout << "Loaded in " << (SDL_GetTicks() - start) / 1000.f << "s" << endl;

}

template <typename T>
Texture<T>::Texture(const Texture& other) {
    cout << "Texture [" << path << "] copied" << endl;
}

template <typename T>
Texture<T>::~Texture() {
    cout << "Texture [" << path << "] destroyed" << endl;
    delete[] data;
}

/**
 * @return The size in bytes of this texture
 */
template<typename T>
size_t Texture<T>::GetSize() const {
    return sizeof(T) * 4 * width * height;
}

template <typename T>
void Texture<T>::Load_Generic(bool store_in_linear) {

    SDL_Surface* surface = IMG_Load(path.c_str());

    if (surface == nullptr) {
        std::cerr << "Error loading [" << path << "]" << endl;
        std::cerr << SDL_GetError() << endl;
        throw std::bad_exception();
    }
    width = (size_t) surface->w;
    height = (size_t) surface->h;

    Uint8* pixels = reinterpret_cast<Uint8*>(surface->pixels);
    int width = surface->w;
    int height = surface->h;

    data = new T[4 * width * height];

    for (int y = 0; y < height; ++y) {

        for (int x = 0; x < width; ++x) {

            int pixel_id = (y * surface->pitch + x * surface->format->BytesPerPixel);
            Uint32* pixel = reinterpret_cast<Uint32*>(pixels + pixel_id);

            data[(y * width + x) * 4 + 0] = (*pixel & surface->format->Rmask) >> surface->format->Rshift;
            data[(y * width + x) * 4 + 1] = (*pixel & surface->format->Gmask) >> surface->format->Gshift;
            data[(y * width + x) * 4 + 2] = (*pixel & surface->format->Bmask) >> surface->format->Bshift;
//            data[(y * width + x) * 4 + 3] = *pixel & surface->format->Amask;
            data[(y * width + x) * 4 + 3] = 0;
        }
    }

    if (store_in_linear) {
        ConvertToLinear();
    }
}

template <typename T>
void Texture<T>::Load_HDR() {

    HDRLoaderResult result;
    bool ret = HDRLoader::load(path.c_str(), result);
    if (ret == false) {
        cout << "Error loading [" << path << "]\n";
        throw std::bad_exception();
    }

    width = (size_t) result.width;
    height = (size_t) result.height;

    data = new T[4 * width * height];

    for (size_t i = 0; i < width * height; ++i) {
        data[4 * i + 0] = result.cols[3 * i];
        data[4 * i + 1] = result.cols[3 * i + 1];
        data[4 * i + 2] = result.cols[3 * i + 2];
        data[4 * i + 3] = 0;
    }
}

template <typename T>
void Texture<T>::Load_FPM() {

    FILE* file = fopen(path.c_str(), "rb");
    if (file == nullptr) {
        cout << "Error loading [" << path << "]\n";
        throw std::bad_exception();
    }
    char type[16];
    float dummy;
    fscanf(file, "%s\n%zu %zu\n", type, &width, &height);
    fgetc(file);
    fscanf(file, "\n%f\n", &dummy);

    //DEBUG Ausgabe
    printf("Keyword: %s\n",type);
    printf("Size X: %zu\n", width);
    printf("Size Y: %zu\n", height);
    printf("dummy: %f\n", dummy);
    // ENDE Debug Ausgabe

    data = new T[4 * width * height];

    int result;
    int lSize;
    lSize = width * 3;
    for (int y = height - 1; y >= 0; y--) {
        result = fread(data + 1 + width * y, sizeof(float), lSize, file);
        if (result != lSize) {
            printf("Error reading PFM-File. %d Bytes read.\n", result);
        }
    }

    fclose(file);
}

template <typename T>
Vec3 Texture<T>::Evaluate(const Vec3& uv) {
    return Sample(uv.x, uv.y);
}

/**
 * direction must be normalized
 */
template <typename T>
Vec3 Texture<T>::Sample_Spheremap(const Vec3& direction) {

#ifdef USE_TRIGO_LOOKUP
    float polar = ACOS_LOOKUP(direction.y);      // acos() => [0, PI]
#else
    float polar = acosf(direction.y);            // acos() => [0, PI]
#endif

    float azimuth = atan2f(direction.x, direction.z);// atan() => [-PI, PI]

    float u = (azimuth + M_PI_F) / (2 * M_PI_F); // [-PI, PI] => [0, 1]
    float v = (polar / M_PI_F);                  // [0, PI]   => [0, 1]

    return Sample(u, v);
}

/**
 * UV coord must be normalized [0, 1]
 */
template <typename T>
inline Vec3 Texture<T>::Sample(float u, float v) {

    int x = (int) round(u * (width - 1));
    int y = (int) round(v * (height - 1));

    return Vec3 (data[(y * width + x) * 4 + 0],
                 data[(y * width + x) * 4 + 1],
                 data[(y * width + x) * 4 + 2]);
}
template <>
inline Vec3 Texture<uint8_t>::Sample(float u, float v) {

    //TODO: Need better handling of out of range coordinates
//    u = std::min(1.f, std::max(0.f, u));
//    v = std::min(1.f, std::max(0.f, v));

    int x = (int) round(u * (width - 1));
    int y = (int) round(v * (height - 1));

    x = x % width;
    y = y % height;

    x = (x >= 0) ? x : width + x;
    y = (y >= 0) ? y : height + y;

//    if (x > w)

    return Vec3 (data[(y * width + x) * 4 + 0] / 255.f,
                 data[(y * width + x) * 4 + 1] / 255.f,
                 data[(y * width + x) * 4 + 2] / 255.f);
}

#define RIGHT   0
#define LEFT    1
#define TOP     2
#define BOTTOM  3
#define FRONT   4
#define BACK    5
#define EPSILON 0.001f

Vec3 projectDirectionOnFace(const Vec3& direction) {
    Vec3 projected = direction / direction.abs().max();
    Vec3 uv;
    if (fabs(projected.x) >= (1 - EPSILON)) {
        uv.x = projected.x > 0 ? projected.z : -projected.z;
        uv.y = projected.y;
        uv.z = projected.x > 0 ? RIGHT : LEFT;
    }
    else if (fabs(projected.y) >= (1 - EPSILON)) {
        uv.x = projected.x;
        uv.y = projected.y > 0 ? projected.z : -projected.z;
        uv.z = projected.y > 0 ? TOP : BOTTOM;
    }
    else if (fabs(projected.z) >= (1 - EPSILON)) {
        uv.x = projected.z > 0 ? -projected.x : projected.x;
        uv.y = projected.y;
        uv.z = projected.z > 0 ? FRONT : BACK;
    }
    return Vec3 {(uv.x+1.f)/2.f, (uv.y+1.f)/2.f, uv.z};
}

float mapToRange(float x, float min, float max) {
    return min + (x * (max - min));
}

template <typename T>
Vec3 Texture<T>::Sample_Cubemap(const Vec3& direction) {

    // Get normalized UV coordinates
    // uv.z indicate which face to sample
    Vec3 uv = projectDirectionOnFace(direction);

    uv.y = 1 - uv.y;
    if (is_pfm_cross == true) {
        switch (int(uv.z)) {
            case RIGHT:
                uv.x = mapToRange(uv.x, 2/3.f, 1);
                uv.y = mapToRange(uv.y, 0.25f, 0.5f);
                break;
            case LEFT:
                uv.x = mapToRange(uv.x, 0, 1/3.f);
                uv.y = mapToRange(uv.y, 0.25f, 0.5f);
                break;
            case TOP:
                uv.x = mapToRange(uv.x, 1/3.f, 2/3.f);
                uv.y = mapToRange(uv.y, 0, 0.25f);
                break;
            case BOTTOM:
                uv.x = mapToRange(uv.x, 1/3.f, 2/3.f);
                uv.y = mapToRange(uv.y, 0.5, 0.75f);
                break;
            case FRONT:
                uv.x = mapToRange(uv.x, 1/3.f, 2/3.f);
                uv.y = mapToRange(1 - uv.y, 0.75f, 1);
                break;
            case BACK:
                uv.x = mapToRange(uv.x, 1/3.f, 2/3.f);
                uv.y = mapToRange(uv.y, 0.25f, 0.5f);
                break;
            default:
                break;
        }
    }
    else {
        switch (int(uv.z)) {
            case RIGHT:
                uv.x = mapToRange(uv.x, 0.5f, 0.75f);
                uv.y = mapToRange(uv.y, 0.5f, 1);
                break;
            case LEFT:
                uv.x = mapToRange(uv.x, 0, 0.25f);
                uv.y = mapToRange(uv.y, 0.5f, 1);
                break;
            case TOP:
                uv.x = mapToRange(uv.x, 0.25f, 0.5f);
                uv.y = mapToRange(uv.y, 0, 0.5f);
                break;
            case BOTTOM:
                uv.x = mapToRange(uv.x, 0.5f, 0.75f);
                uv.y = mapToRange(uv.y, 0, 0.5f);
                break;
            case FRONT:
                uv.x = mapToRange(uv.x, 0.75f, 1);
                uv.y = mapToRange(uv.y, 0.5f, 1);
                break;
            case BACK:
                uv.x = mapToRange(uv.x, 0.25f, 0.5f);
                uv.y = mapToRange(uv.y, 0.5f, 1);
                break;
            default:
                break;
        }
    }
    return Sample(uv.x, uv.y);
}

template <typename T>
void Texture<T>::ConvertToLinear() {
    cout << "ConvertToLinear generic template" << endl;
}

template <typename T>
string Texture<T>::GetName() {
    return path;
}

template<>
void Texture<float>::ConvertToLinear() {
}

template<>
void Texture<uint8_t>::ConvertToLinear() {
    for (size_t i = 0; i < 4 * width * height; ++i) {
        data[i] = static_cast<uint8_t>(std::pow(data[i] / 255.f, 2.2f) * 255);
    }
}



