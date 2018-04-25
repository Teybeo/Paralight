#include "Options.h"

#include <iostream>

using std::cout;

void Options::Update() {

    has_changed = false;

    if (relevant_key_event_captured)
        has_changed = true;

    relevant_key_event_captured = false;
}

void Options::KeyEvent(SDL_Keysym keysym, SDL_EventType type) {

    if (type == SDL_KEYUP) {

        // Less duplicate code to just deactivate it on irrelevent events
        relevant_key_event_captured = true;

        switch (keysym.scancode) {
            case SDL_SCANCODE_Q:
                use_emissive_lighting =! use_emissive_lighting;
                cout << "Emissive lighting set to " << use_emissive_lighting << endl;
                break;
            case SDL_SCANCODE_E:
                use_distant_env_lighting =! use_distant_env_lighting;
                cout << "Distant Env lighting set to " << use_distant_env_lighting << endl;
                break;
            case SDL_SCANCODE_L:
                brdf_bitfield ^= LAMBERTIAN;
                cout << "brdf_bitfield set to " << int(brdf_bitfield) << endl;
                break;
            case SDL_SCANCODE_M:
                brdf_bitfield ^= MICROFACET;
                cout << "brdf_bitfield set to " << int(brdf_bitfield) << endl;
                break;
            case SDL_SCANCODE_T:
                use_tonemapping =! use_tonemapping;
                cout << "Tonemapping set to " << use_tonemapping << endl;
                relevant_key_event_captured = false;
                break;
            case SDL_SCANCODE_KP_PLUS:
                sample_count *= 2;
                cout << "Sample count set to " << sample_count << endl;
                break;
            case SDL_SCANCODE_KP_MINUS:
                sample_count /= 2;
                sample_count = std::max(sample_count, short(1));
                cout << "Sample count set to " << sample_count << endl;
                break;
            case SDL_SCANCODE_UP:
                bounce_cout++;
                cout << "Bounce count set to " << bounce_cout << endl;
                break;
            case SDL_SCANCODE_DOWN:
                bounce_cout--;
                bounce_cout = std::max(bounce_cout, short(0));
                cout << "Bounce count set to " << bounce_cout << endl;
                break;
            case SDL_SCANCODE_EQUALS:
                depth_target++;
                break;
            case SDL_SCANCODE_9:
                depth_target--;
                depth_target = std::max(depth_target, 0);
                break;
            case SDL_SCANCODE_B:
                use_bvh =! use_bvh;
                cout << "BVH use set to " << use_bvh << endl;
                break;
            case SDL_SCANCODE_G:
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