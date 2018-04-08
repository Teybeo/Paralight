#ifndef BASERENDERER_H
#define BASERENDERER_H

#include <chrono>
#include <app/Chronometer.h>
#include <core/Film.h>
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
    Film* film;
    unsigned int texture;

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

    BaseRenderer(Scene* scene, SDL_Window* pWindow, Film* film, CameraControls* pControls, Options* pOptions);

    virtual void Render() = 0;

    virtual void TracePixel(Vec3 pixel, bool picking) = 0;

    virtual void KeyEvent(SDL_Keysym keysym, SDL_EventType param);

    virtual void Update();

    Object3D* GetSelectedObject() {
        return selected_object;
    }

    unsigned int GetFilmTexture() const {
        return texture;
    }

    short GetFrameNumber() const {
        return frame_number;
    }

    float GetRenderTime() const {
        return render_chrono.GetSeconds();
    }

    void DumpScreenshot();

    void UpdateGLTexture();
};


#endif //BASERENDERER_H
