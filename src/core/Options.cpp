#include "Options.h"

#include <iostream>

using std::cout;

void Options::Update() {

    options_changed = false;

    if (relevant_key_event_captured)
        options_changed = true;

    relevant_key_event_captured = false;
}

void Options::KeyEvent(SDL_Keysym keysym, SDL_EventType type) {

    if (type == SDL_KEYUP) {

        // Less duplicate code to just deactivate it on irrelevent events
        relevant_key_event_captured = true;

        switch (keysym.sym) {
            case SDLK_a:
                use_emissive_lighting =! use_emissive_lighting;
                cout << "Emissive lighting set to " << use_emissive_lighting << endl;
                break;
            case SDLK_e:
                use_distant_env_lighting =! use_distant_env_lighting;
                cout << "Distant Env lighting set to " << use_distant_env_lighting << endl;
                break;
            case SDLK_l:
                brdf_bitfield ^= LAMBERTIAN;
                cout << "brdf_bitfield set to " << int(brdf_bitfield) << endl;
                break;
            case SDLK_m:
                brdf_bitfield ^= MICROFACET;
                cout << "brdf_bitfield set to " << int(brdf_bitfield) << endl;
                break;
            case SDLK_t:
                use_tonemapping =! use_tonemapping;
                cout << "Tonemapping set to " << use_tonemapping << endl;
                relevant_key_event_captured = false;
                break;
            case SDLK_KP_PLUS:
                sample_count *= 2;
                cout << "Sample count set to " << sample_count << endl;
                break;
            case SDLK_KP_MINUS:
                sample_count /= 2;
                sample_count = std::max(sample_count, short(1));
                cout << "Sample count set to " << sample_count << endl;
                break;
            case SDLK_UP:
                bounce_cout++;
                cout << "Bounce count set to " << bounce_cout << endl;
                break;
            case SDLK_DOWN:
                bounce_cout--;
                bounce_cout = std::max(bounce_cout, short(0));
                cout << "Bounce count set to " << bounce_cout << endl;
                break;
            case SDLK_EQUALS:
                depth_target++;
                break;
            case SDLK_RIGHTPAREN:
                depth_target--;
                depth_target = std::max(depth_target, 0);
                break;
            case SDLK_b:
                use_bvh =! use_bvh;
                cout << "BVH use set to " << use_bvh << endl;
                break;
            case SDLK_g:
                debug = !debug;
                cout << "Debug set to " << debug << endl;
                break;
            default:
                relevant_key_event_captured = false;
                break;
        }

    }

//    CopyOptionsStruct(keysym, type);

}

//void Options::SetBackground(Vec3 color) {
//    background_color = color;
//
//}