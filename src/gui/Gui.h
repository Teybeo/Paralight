#ifndef PATHTRACER_GUI_H
#define PATHTRACER_GUI_H

#include "imgui/imgui.h"

#include <memory>
#include <vector>

typedef struct SDL_Window SDL_Window;
typedef struct BaseRenderer BaseRenderer;
typedef struct Object3D Object3D;
typedef struct Scene Scene;

class Options;
class ITexture;

class GUI {
    Scene* scene;
    Options* options;
    SDL_Window* window;
    BaseRenderer*& renderer;
    ImVec2 window_size;
    ImGuiWindowFlags window_flags;
    int selected_platform;
    int selected_device;
    std::vector<std::string> env_map_array;
    const char* env_map_dir = "../../envmaps/";
    int env_map_index = 0;

    void showObjectSettings();
    void showOpenCLSettings();
    void showLightingSettings();

public:

    bool options_has_changed = false;

    GUI() = delete;
    GUI(Options* options, SDL_Window* window, BaseRenderer*& renderer_name, Scene* scene);

    void Draw();
    void Update();

    void showMaterialSettings(Object3D* object);

    void showRendererSelection(bool cpp);

    void showTextureSettings(std::shared_ptr<ITexture> texture, const char* texture_name);

    void showBrdfStackSettings();

};


#endif //PATHTRACER_GUI_H
