#ifndef OPENCL_OPTIONS_H
#define OPENCL_OPTIONS_H

#include <SDL_keyboard.h>
#include <SDL_events.h>

#include "material/Brdf.h"
#include "math/Vec3.h"

class Options {

public:
    bool use_emissive_lighting = true;
    bool use_distant_env_lighting = true;
    char brdf_bitfield = LAMBERTIAN | MICROFACET;
    bool use_tonemapping = false;
    short sample_count = 1;
    short bounce_cout = 1;
    bool relevant_key_event_captured = false;
    Vec3 background_color {0.18, 0.18, 0.36};
    int depth_target = 0;
    int fov = 70;
    bool debug = false;
    bool use_bvh = true;

    void KeyEvent(SDL_Keysym keysym, SDL_EventType param);

    void Update();

    bool HasChanged() const {
        return has_changed;
    }

//    void SetBackground(Vec3 color);
    bool has_changed = false;
};


#endif //OPENCL_OPTIONS_H
