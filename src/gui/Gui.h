#ifndef PATHTRACER_GUI_H
#define PATHTRACER_GUI_H

#include <memory>
#include "imgui/imgui.h"

typedef struct SDL_Window SDL_Window;
typedef struct BaseRenderer BaseRenderer;
typedef struct Object3D Object3D;

class Options;
class ITexture;

class GUI {
    Options* options;
    SDL_Window* window;
    BaseRenderer*& renderer;
    ImVec2 window_size;
    ImGuiWindowFlags window_flags;
    int selected_platform;
    int selected_device;

    void showObjectSettings();
    void showOpenCLSettings();
    void showLightingSettings();

public:

    bool has_changed = false;
    bool material_has_changed = false;

    GUI() = delete;
    GUI(Options* options, SDL_Window* window, BaseRenderer*& renderer_name);

    void Draw();
    void Update();

    void showMaterialSettings(Object3D* object);

    void showRendererSelection(bool cpp);

    void showTextureSettings(std::shared_ptr<ITexture> texture, const char* texture_name);

    void showBrdfStackSettings();
};


#endif //PATHTRACER_GUI_H
