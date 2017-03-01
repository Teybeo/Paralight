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
class Texture;

class GUI {
    Scene* scene;
    Options* options;
    SDL_Window* window;
    BaseRenderer*& renderer;
    ImVec2 window_size;
    ImGuiWindowFlags window_flags;
    int selected_platform;
    int selected_device;
    std::vector<std::string> envmap_array;
    std::vector<std::string> model_array;
    const char* env_map_dir = "../../envmaps/";
    const char* model_dir = "../../models/";
    int envmap_index = 0;
    int model_index = 0;
    bool options_has_changed = false;

    void showRendererSettings();
    void showOpenCLSettings();
    void showLightingSettings();
    void showObjectSettings();
    void showMaterialSettings(Object3D* object);
    void showTextureSettings(std::shared_ptr<Texture> texture, const char* texture_name);

public:

    GUI() = delete;
    GUI(Options* options, SDL_Window* window, BaseRenderer*& renderer_name, Scene* scene);

    void Draw();
    void Update();
};


#endif //PATHTRACER_GUI_H
