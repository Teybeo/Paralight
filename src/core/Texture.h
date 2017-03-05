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

class Texture {
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
class ImageTexture : public Texture {
public:
    //TODO: unique_ptr here
    T* data = nullptr;
    char channel_count = 0;
    Vec3 constant = 0;
    const std::string path;
    size_t width = 0;
    size_t height = 0;

    ImageTexture(const std::string& path, bool store_in_linear = true);
    ~ImageTexture();

    Vec3 Evaluate(const Vec3& uv) override;

    Vec3 SampleEnvmap(const Vec3& direction);
    Vec3 SampleSpheremap(const Vec3& direction);

    size_t GetSize() const override ;

    std::string GetName() override;

private:
    inline Vec3 Sample(float u, float v);

    void Load_Generic(bool store_in_linear);
    void Load_HDR();

    void ConvertToLinear();

};

template <typename T>
class ValueTexture : public Texture {

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

typedef ImageTexture<float> TextureFloat;
typedef ImageTexture<uint8_t> TextureUbyte;


#endif //OPENCL_TEXTURE_H
