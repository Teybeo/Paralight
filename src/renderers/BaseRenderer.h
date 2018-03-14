#ifndef BASERENDERER_H
#define BASERENDERER_H

#include <chrono>
#include <app/Chronometer.h>
#include "core/Scene.h"
#include "core/CameraControls.h"
#include "core/Options.h"

//#define INNER_LOOP

class BaseRenderer {

//    std::vector<std::unique_ptr<Light>> lights;

public:
    static const int EVENT_CPP_RENDERER = 0;
    static const int EVENT_CL_RENDERER  = 1;
    static const int EVENT_CL_PLATFORM_DEVICE_CHANGE = 2;

    bool debug = false;

protected:
    Scene* scene;
    SDL_Window* window;
    CameraControls* camera_controls;
    Options* options;
    std::vector<uint32_t> pixels;
    unsigned int texture;
    int film_width = 512;
    int film_height = film_width;
    Vec3 film_display_size = 1.0f;
    float film_render_scale = 1.0f;

protected:
    bool CLEAR_ACCUM_BIT = false;
    short frame_number = 0;
    Chronometer render_chrono;
    Object3D* selected_object = nullptr;
    bool reset_camera = false;
    bool dump_screenshot = false;

public:

    BaseRenderer() = delete;
    virtual ~BaseRenderer();

    BaseRenderer(Scene* scene, SDL_Window* pWindow, CameraControls* pControls, Options* pOptions);

    virtual void Render() = 0;

    virtual void TracePixel(Sint32 x, Sint32 y, bool picking) = 0;

    virtual void KeyEvent(SDL_Keysym keysym, SDL_EventType param);

    virtual void Update();
    
    Object3D* GetSelectedObject() {
        return selected_object;
    }

    unsigned int GetFilmTexture() const {
        return texture;
    }

    Vec3 GetFilmSize() {
        return {float(film_width), float(film_height)};
    }

    Vec3 GetFilmDisplaySize() const {
        return film_display_size;
    }

    void SetFilmDisplaySize(Vec3 film_scale) {
        BaseRenderer::film_display_size = film_scale;
    }

    float GetFilmRenderScale() const {
        return film_render_scale;
    }

    void SetFilmRenderScale(float film_render_scale) {

        BaseRenderer::film_render_scale = film_render_scale;

        int new_film_width  = static_cast<int>(film_width * film_render_scale);
        int new_film_height = static_cast<int>(film_height * film_render_scale);

        printf("texture: %i %i\n", new_film_width, new_film_height);

        pixels.resize(new_film_width * new_film_height);
    }

    short GetFrameNumber() const {
        return frame_number;
    }
    
    float GetRenderTime() const {
        return render_chrono.GetSeconds();
    }
    
    void DumpScreenshot();
    
    void DrawTexture();
    
};


#endif //BASERENDERER_H
