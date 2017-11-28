#ifndef BASERENDERER_H
#define BASERENDERER_H

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
    int fov = 70;
    SDL_Window* window;
    CameraControls* camera_controls;
    Options* options;
    std::vector<uint32_t> pixels;
    unsigned int texture;
    int film_width = 512;
    int film_height = film_width;
    bool CLEAR_ACCUM_BIT = 0;
    short frame_number = 0;
    float last_clear_timestamp = 0;
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

    void DrawFrametime();

    Object3D* GetSelectedObject() {
        return selected_object;
    }

    void DumpScreenshot();
    
    void DrawTexture();
    
};


#endif //BASERENDERER_H
