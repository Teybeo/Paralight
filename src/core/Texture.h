#ifndef OPENCL_TEXTURE_H
#define OPENCL_TEXTURE_H

#include <string>
#include <math/Vec3.h>
#include <SDL_quit.h>

struct alignas(4) CLTextureInfo {
    int width;
    int height;
    int byte_offset;
    char mapping;
};

class ITexture {
public:
    virtual Vec3 Evaluate(const Vec3& direction) = 0;

    virtual size_t GetSize() const {
        std::cerr << "WUUUUUUUUUUUT" << std::endl;
        return 0;
    };

    virtual std::string GetName() {
        return "Undefined";
    }
};

template <typename T>
class Texture : public ITexture {
public:
    //TODO: unique_ptr here
    T* data = nullptr;
    Vec3 constant = 0;
    const bool is_pfm_cross = false;
    const std::string path;
    size_t width = 0;
    size_t height = 0;

    Texture(const std::string& path, bool store_in_linear = true);
    ~Texture();

    Vec3 Evaluate(const Vec3& uv) override;

    Vec3 Sample_Spheremap(const Vec3& direction);

    size_t GetSize() const override ;

    std::string GetName() override;

private:
    inline Vec3 Sample(float u, float v);
    inline Vec3 Sample_Cubemap(const Vec3& direction);

    void Load_Generic(bool store_in_linear);
    void Load_FPM();
    void Load_HDR();

    void ConvertToLinear();
};

template <typename T>
class ValueTexture : public ITexture {

public:

    T value;

    ValueTexture(const T& value) : value(value) {
    }

    Vec3 Evaluate(const Vec3& direction) override {
        return Vec3{value};
    }
};

typedef ValueTexture<Vec3> ValueTex3f;
typedef ValueTexture<float> ValueTex1f;

typedef Texture<float> TextureFloat;
typedef Texture<uint8_t> TextureUbyte;


#endif //OPENCL_TEXTURE_H
