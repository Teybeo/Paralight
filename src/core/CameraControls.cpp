#include "CameraControls.h"

void CameraControls::KeyEvent(SDL_Keysym keysym, SDL_EventType action) {

    bool state = (action == SDL_KEYDOWN);
    switch (keysym.sym) {
        case SDLK_z:
            move_forward = state;
            break;
        case SDLK_s:
            move_backward = state;
            break;
        case SDLK_q:
            strafe_left = state & (control_mode != ORBIT);
            break;
        case SDLK_d:
            strafe_right = state & (control_mode != ORBIT);
            break;
        case SDLK_ESCAPE:
            if (relative_mode && action == SDL_KEYUP) {
                SDL_SetRelativeMouseMode(SDL_FALSE);
                relative_mode = false;
            }
            else if (!relative_mode && action == SDL_KEYUP) {
                SDL_SetRelativeMouseMode(SDL_TRUE);
                relative_mode = true;
            }
            break;
        case SDLK_c:
            if (action == SDL_KEYUP)
                std::cout << "Cam at: {" << position << "}, xz_angle: " << xz_angle << ", yz_angle: " << yz_angle << std::endl;
            break;
        case SDLK_o:
            if (action == SDL_KEYUP) {
                control_mode = static_cast<ControlMode>(!control_mode); // Bit hacky
            }
            break;
        default:
            break;
    }

}

void CameraControls::MouseEvent(SDL_MouseMotionEvent event) {
    if (relative_mode) {
        // We accumulate because SDL usually reports multiple small mouse motions events over a short time
        x_mouse_motion += event.xrel;
        y_mouse_motion += event.yrel;
    }
}

void CameraControls::Update() {

    last_position = position;
    last_xz_angle = xz_angle;
    last_yz_angle = yz_angle;

    xz_angle += x_mouse_motion * 0.01f;
    yz_angle += y_mouse_motion * 0.01f;

    // Create a rotation matrix from the current angles
    rotation.setRotation(yz_angle, xz_angle, 0);

    Vec3 delta;
    Vec3 forward {0, 0, -1};
    Vec3 strafe {1, 0, 0};

    if (control_mode == FREEFLY) {
        forward = rotation * forward;
        strafe  = rotation * strafe;
//        forward = Vec3 {rotation[0][2], rotation[1][2], rotation[2][2]};
//        strafe  = Vec3 {rotation[0][0], rotation[1][0], rotation[2][0]};
    }

    if (move_forward) {
        delta += forward;
    }
    if (move_backward) {
        delta += -forward;
    }
    if (strafe_left) {
        delta += -strafe;
    }
    if (strafe_right) {
        delta += strafe;
    }

//    delta *= 0.5f; // Speed factor
    delta *= speed_factor; // Speed factor

    if (control_mode == ORBIT) {
        orbit_pos += delta;
        position = rotation * orbit_pos;
    } else {
        position += delta;
    }

    // SDL doesn't send "end of motion" events so we consume the motion here to not replay it infinitely
    x_mouse_motion = 0;
    y_mouse_motion = 0;
}


void CameraControls::SetRotation(float yz_angle, float xz_angle) {

    this->yz_angle = yz_angle;
    this->xz_angle = xz_angle;
    rotation.setRotation(yz_angle, xz_angle, 0);
}

void CameraControls::SetPosition(const Vec3& position) {
    if (control_mode == FREEFLY) {
        this->position = position;
    } else {
        this->orbit_pos = rotation.Transpose() * position;
        this->position = rotation * orbit_pos;
    }
}

bool CameraControls::HasChanged() {

    // Look if the Camera position or orientation has been changed
    return (position != last_position) | (yz_angle != last_yz_angle) | (xz_angle != last_xz_angle);
}

void CameraControls::SetSpeed(float speed) {
    speed_factor = speed / 20;
}







