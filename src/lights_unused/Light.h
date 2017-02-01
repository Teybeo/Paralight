#ifndef RAYTRACING_LIGHT_H
#define RAYTRACING_LIGHT_H

#include "math/Vec3.h"

class Light {
public:
    Vec3 color {1, 1, 1};
    float intensity = 10;
public:

    virtual void getLightData(const Vec3& hit_pos, Vec3& direction_out, float& distance_out, Vec3& intensity_out) const = 0;
    virtual Vec3 getOrigin() const = 0;
};


#endif //RAYTRACING_LIGHT_H
