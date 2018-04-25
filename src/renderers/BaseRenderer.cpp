#include "BaseRenderer.h"

#include "app/Chronometer.h"
#include "objects/TriMesh.h"
#include "core/BVH.h"

#include <SDL_timer.h>
#include <iostream>
#include <ctime>
#include <SDL_opengl.h>

#define DUMP_VAR(x) cout << #x ": " << x << '\n';

using std::cout;
using std::endl;
using std::max;
using std::string;
using std::unique_ptr;

BaseRenderer::BaseRenderer(Scene* scene, SDL_Window* window, Film* film, CameraControls* const controls, Options* options)
        : scene{scene}, window{window}, film{film}, camera_controls{controls}, options{options} {
    camera_controls->SetSpeed(scene->debug_scale);

    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glBindTexture(GL_TEXTURE_2D, 0);
}

BaseRenderer::~BaseRenderer() {
    glDeleteTextures(1, &texture);
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
    CLEAR_ACCUM_BIT = !(options->HasChanged() || camera_controls->HasChanged() || scene->HasChanged() || film->HasChanged());

    frame_number *= CLEAR_ACCUM_BIT;
    frame_number++;

    TriMesh::ClearCounters();
    BVH2::ResetCounters();
}

void BaseRenderer::Render() {

    if (frame_number == 1) {
        render_chrono.Restart();
    }
}

void BaseRenderer::UpdateGLTexture() {

    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, film->GetWidth(), film->GetHeight(), 0, GL_BGRA, GL_UNSIGNED_BYTE, film->GetPixels());
    glBindTexture(GL_TEXTURE_2D, 0);
}

void BaseRenderer::DumpScreenshot() {

    SDL_Surface* surface = SDL_GetWindowSurface(window);
    string name = string(typeid(*this).name()).find("CPP") ? "C++" : "CL" ;
    time_t now_t = std::time(nullptr);
    tm* now = std::localtime(&now_t);
    string time = std::to_string(now->tm_hour) + "h_" + std::to_string(now->tm_min);

    SDL_SaveBMP(surface, (string("../../dumps/") + time + "_" + name + ".bmp").c_str());
}

void BaseRenderer::KeyEvent(SDL_Keysym keysym, SDL_EventType type) {

    if (type == SDL_KEYUP) {
        SDL_Event event {};
        switch (keysym.scancode) {
            case SDL_SCANCODE_1:
                event.type = SDL_USEREVENT;
                event.user.code = EVENT_CPP_RENDERER;
                SDL_PushEvent(&event);
                break;
            case SDL_SCANCODE_2:
                event.type = SDL_USEREVENT;
                event.user.code = EVENT_CL_RENDERER;
                SDL_PushEvent(&event);
                break;
            case SDL_SCANCODE_R:
                reset_camera = true;
                break;
            case SDL_SCANCODE_P:
                dump_screenshot = true;
                break;
            case SDL_SCANCODE_0:
                debug =! debug;
                break;
            default:
                break;
        }
    }

}
