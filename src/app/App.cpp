#include "App.h"

#include "renderers/CppRenderer.h"
#include "renderers/OpenCLRenderer.h"
#include "gui/imgui/imgui.h"
#include "gui/imgui/imgui_impl_sdl.h"

#include <SDL.h>
#include <SDL_image.h>
#include <ctime>
#include <chrono>

App::App() {

    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        std::cout << "Erreur au chargement de SDL2\n" << SDL_GetError();
        throw std::bad_exception();
    }

    if (IMG_Init(IMG_INIT_JPG) != IMG_INIT_JPG) {
        std::cout << "Erreur au chargement de SDL2_image\n" << SDL_GetError();
        throw std::bad_exception();
    }

    std::srand((unsigned int) std::time(0));
//    std::srand((unsigned int) 3);

    window = Window {"Paralight", 1280, 720};

    camera_controls.SetPosition(scene.cam_pos);
    camera_controls.SetRotation(scene.yz_angle, scene.xz_angle);

//    renderer = new CppRenderer(&scene, window.GetSDL_window(), &camera_controls, &options);
    renderer = new OpenCLRenderer(&scene, window.GetSDL_window(), &camera_controls, &options);

    overlay = new GUI {&options, window.GetSDL_window(), renderer, &scene};

    is_running = true;
}

void App::Run() {
    is_running = true;
    do {
        Event();
        Update();
        Draw();
    } while (is_running);
}

void App::Draw() {
    
    renderer->Render();
    renderer->DrawTexture();
    renderer->DrawFrametime();
    overlay->Draw();
    
    SDL_GL_SwapWindow(window.GetSDL_window());
}

void App::Update() {
    options.Update();
    overlay->Update();
    camera_controls.Update();
    renderer->Update();
}

const char* GetWindowEventString(int code);

void App::Event() {

    SDL_Event ev;
    while (SDL_PollEvent(&ev)) {
        ImGui_ImplSdl_ProcessEvent(&ev);

        switch (ev.type) {
            case SDL_QUIT:
                is_running = false;
                break;
            case SDL_WINDOWEVENT:
//                std::cout << GetWindowEventString(ev.window.event) << '\n';

                if (ev.window.event == SDL_WINDOWEVENT_CLOSE) {
                    is_running = false;
                }
                break;
            case SDL_MOUSEBUTTONUP:
                if (ev.button.windowID == SDL_GetWindowID(window.GetSDL_window())) {
                    renderer->TracePixel(ev.button.x, ev.button.y, ev.button.button == SDL_BUTTON_LEFT);
                }
                break;
            case SDL_MOUSEMOTION:
                camera_controls.MouseEvent(ev.motion);
                break;
            case SDL_KEYDOWN:
            case SDL_KEYUP:
                camera_controls.KeyEvent(ev.key.keysym, (SDL_EventType) ev.type);
                options.KeyEvent(ev.key.keysym, (SDL_EventType) ev.type);
                renderer->KeyEvent(ev.key.keysym, (SDL_EventType) ev.type);
                break;
            case SDL_USEREVENT:
                int platform_index;
                int device_index;
                delete renderer;

                switch (ev.user.code) {
                case BaseRenderer::EVENT_CPP_RENDERER:
                    renderer = new CppRenderer(&scene, window.GetSDL_window(), &camera_controls, &options);
                    break;
                case BaseRenderer::EVENT_CL_RENDERER:
                    renderer = new OpenCLRenderer(&scene, window.GetSDL_window(), &camera_controls, &options);
                    break;
                case BaseRenderer::EVENT_CL_PLATFORM_DEVICE_CHANGE:
                    platform_index = *static_cast<int*>(ev.user.data1);
                    device_index   = *static_cast<int*>(ev.user.data2);
                    renderer = new OpenCLRenderer(&scene, window.GetSDL_window(), &camera_controls, &options, platform_index, device_index);
                    break;
                default:
                    break;
                }
                break;
            default:
                break;
        }
    }

}
#define CASE_MACRO(name) case name: return #name;
const char* GetWindowEventString(int code) {
    switch (code) {
        CASE_MACRO(SDL_WINDOWEVENT_NONE)
        CASE_MACRO(SDL_WINDOWEVENT_SHOWN)
        CASE_MACRO(SDL_WINDOWEVENT_HIDDEN)
        CASE_MACRO(SDL_WINDOWEVENT_EXPOSED)
        CASE_MACRO(SDL_WINDOWEVENT_MOVED)
        CASE_MACRO(SDL_WINDOWEVENT_RESIZED)
        CASE_MACRO(SDL_WINDOWEVENT_SIZE_CHANGED)
        CASE_MACRO(SDL_WINDOWEVENT_MINIMIZED)
        CASE_MACRO(SDL_WINDOWEVENT_MAXIMIZED)
        CASE_MACRO(SDL_WINDOWEVENT_RESTORED)
        CASE_MACRO(SDL_WINDOWEVENT_ENTER)
        CASE_MACRO(SDL_WINDOWEVENT_LEAVE)
        CASE_MACRO(SDL_WINDOWEVENT_FOCUS_GAINED)
        CASE_MACRO(SDL_WINDOWEVENT_FOCUS_LOST)
        CASE_MACRO(SDL_WINDOWEVENT_CLOSE)
        default:
            return "Unknown build status";
    }
}
