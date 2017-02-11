#include "CppRenderer.h"
#include "BaseRenderer.h"

#include <SDL_timer.h>
#include <iostream>
#include <objects/TriMesh.h>
#include <core/BVH.h>

#define DUMP_VAR(x) cout << #x ": " << x << '\n';

using std::cout;
using std::endl;
using std::max;
using std::unique_ptr;

BaseRenderer::BaseRenderer(Scene* scene, SDL_Window* window, CameraControls* const controls, Options* options)
        : scene{scene}, window{window}, camera_controls{controls}, options{options} {
    camera_controls->SetSpeed(scene->debug_scale);
}

BaseRenderer::~BaseRenderer() {
}

void BaseRenderer::Update() {

    if (reset_camera) {
        camera_controls->SetRotation(scene->yz_angle, scene->xz_angle);
        camera_controls->SetPosition(scene->cam_pos);
        reset_camera = false;
    }

    if (dump_screenshot == true && frame_number == 50) {
        DumpScreenshot();
        cout << "Dump" << endl;
        dump_screenshot = false;
    }

    // The CLEAR_ACCUM_BIT will be mult/AND with the accumulation buffer and frame number
    // Setting it to 0 will clear them and setting to 1 will do nothing
    // Check if the current rendering config has changed
    CLEAR_ACCUM_BIT = !(options->HasChanged() || camera_controls->HasChanged() || scene->HasChanged());

    frame_number *= CLEAR_ACCUM_BIT;
    frame_number++;

    if (CLEAR_ACCUM_BIT == 0) {
        last_clear_timestamp = SDL_GetTicks() / 1000.f;
    }

    TriMesh::ClearCounters();
    BVH2::ResetCounters();
//TODO: Move this to Scene
//    frame_number = 1;
//
//    static float time = 0;
//    size_t i = 2;
//    for (auto& item: objects) {
//        if (item == objects.front() || i == objects.size() - 1)
//            continue;
//
//        item->pos.y = std::sin(time + 1 * i) * 2.f;
//        item->pos.z = (-1 + std::cos(time + 1 * i)) * 2.f;
//
//        i++;
//    }
//
//    time += .5f;
//    CLEAR_ACCUM_BIT = 0;
}

void BaseRenderer::DrawFrametime() {

    static char title[100] = "";
    static Uint32 last_draw_timestamp = 0;

    // Get CPU time
    static Uint32 debut = 0;
    float duration = (SDL_GetTicks() - debut);
    float now = SDL_GetTicks() / 1000.f - last_clear_timestamp;

    // Limit the title bar update rate to 0.1/s to keep it readable and not spam dwm.exe
    if (fabsf(SDL_GetTicks() - last_draw_timestamp) >= 100) {
        snprintf(title, sizeof(title), "CPU: %.0f ms, frames: %d, time: %.2f s - spp: %d, bounce: %d", duration, frame_number, now, options->sample_count, options->bounce_cout);
        SDL_SetWindowTitle(window, title);
        last_draw_timestamp = SDL_GetTicks();
    }

    debut = SDL_GetTicks();
}

void BaseRenderer::DumpScreenshot() {

    SDL_Surface* surface = SDL_GetWindowSurface(window);
    std::string name = typeid(*this).name();
    SDL_SaveBMP(surface, (std::string("../../dumps/") + name + ".bmp").c_str());
}

void BaseRenderer::KeyEvent(SDL_Keysym keysym, SDL_EventType type) {

    if (type == SDL_KEYUP) {

        switch (keysym.sym) {
            case SDLK_1:
            case SDLK_2:
                SDL_Event event;
                event.type = SDL_USEREVENT;
                event.user.code = keysym.sym == SDLK_1 ? EVENT_CPP_RENDERER : EVENT_CL_RENDERER;
                SDL_PushEvent(&event);
                break;
            case SDLK_r:
                reset_camera = true;
                break;
            case SDLK_p:
                dump_screenshot = true;
                break;
            case SDLK_0:
                debug =! debug;
                break;
            default:
                break;
        }
    }

}
