#ifndef RAYTRACING_CAMERACONTROLS_H
#define RAYTRACING_CAMERACONTROLS_H

#include "math/Matrix.h"
#include "SDL_events.h"

class CameraControls {

    enum ControlMode : bool {
        FREEFLY = false,
        ORBIT   = true,
    };

    ControlMode control_mode = ORBIT;

    bool move_forward = false;
    bool move_backward = false;
    bool strafe_left = false;
    bool strafe_right = false;
    bool relative_mode = false;
    int x_mouse_motion = 0;
    int y_mouse_motion = 0;

    Vec3 position {0, 0, 0};
    float xz_angle = 0;
    float yz_angle = 0;
    Matrix rotation;
    Vec3 orbit_pos {0, 0, 0};

    Vec3 last_position {0, 0, 0};
    float last_yz_angle = 0;
    float last_xz_angle = 0;

    float speed_factor = 0.5f;

public:

    void Update();

    void SetPosition(const Vec3& position);

    void SetRotation(float yz_angle, float xz_angle);

    const Vec3 &GetPosition() const {
        return position;
    }

    const Matrix &GetRotation() const {
        return rotation;
    }

    /**
     * Returns true if the camera position or rotation was changed in the last Update or after
     */
    bool HasChanged();

    void MouseEvent(SDL_MouseMotionEvent event);

    void KeyEvent(SDL_Keysym keysym, SDL_EventType action);

    void SetSpeed(float speed);

};


#endif //RAYTRACING_CAMERACONTROLS_H
