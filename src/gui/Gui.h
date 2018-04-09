#ifndef PATHTRACER_GUI_H
#define PATHTRACER_GUI_H

#include <math/Vec3.h>
#include <memory>
#include <vector>
#include <core/CameraControls.h>
#include <core/Film.h>

typedef struct SDL_Window SDL_Window;
typedef struct BaseRenderer BaseRenderer;
typedef struct Object3D Object3D;
typedef struct Scene Scene;
typedef int ImGuiWindowFlags;

class Options;
class Texture;

class GUI {
    Scene* scene;
    Options* options;
    SDL_Window* window;
    BaseRenderer*& renderer;
    CameraControls* controls;
    Film* film;
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
    bool is_settings_opened = true;
    bool is_stats_opened = true;
    Vec3 render_image_size;
    Vec3 render_image_pos;

    void ShowRendererSettings();
    void ShowOpenCLSettings();
    void ShowLightingSettings();
    void ShowObjectSettings();
    void ShowMaterialSettings(Object3D* object);
    void ShowTextureSettings(std::shared_ptr<Texture> texture, const char* texture_name);
    void ShowAppMenuBar();

public:

    GUI() = delete;
    GUI(Options* options, SDL_Window* window, BaseRenderer*& renderer, Scene* scene, Film* film, CameraControls* controls);

    void Draw();
    void Update();
    bool ProcessEvent(SDL_Event* event);

    bool MouseClickedOnFilm(SDL_MouseButtonEvent event);
    Vec3 ScreenToFilmCoordinates(SDL_MouseButtonEvent mouse);
};


#endif //PATHTRACER_GUI_H
