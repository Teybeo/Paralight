#ifndef PARALIGHT_FILM_H
#define PARALIGHT_FILM_H

#include <cstdint>
#include <vector>
#include <cmath>
#include <math/Vec3.h>

class Film {

private:
    std::vector<uint32_t> pixels;
    int base_width = 500;
    int base_height = 500;
    float render_scale = 0.5f;
    int width = std::lround(base_width * render_scale);
    int height = std::lroundf(base_height * render_scale);
    bool is_dirty = false;
    bool has_changed = false;

public:

    Film() = default;
    Film(int width, int height);

    void Update();

    void* GetPixels() {
        return pixels.data();
    }

    bool HasChanged() const {
        return has_changed;
    }

    void SetFilmRenderScale(float scale) {
        render_scale = scale;
        is_dirty = true;
    }

    void SetBaseFilmSize(int width, int height) {
        base_width = width;
        base_height = height;
        is_dirty = true;
    }

    Vec3 GetSize() const {
        return {float(width), float(height)};
    }

    Vec3 GetBaseSize() const {
        return {float(base_width), float(base_height)};
    }

    int GetWidth() const {
        return width;
    }

    int GetHeight() const {
        return height;
    }

    int GetBaseFilmWidth() const {
        return base_width;
    }

    int GetBaseFilmHeight() const {
        return base_height;
    }

    float GetFilmRenderScale() const {
        return render_scale;
    }
};

#endif //PARALIGHT_FILM_H
