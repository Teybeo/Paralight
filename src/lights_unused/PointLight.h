#ifndef RAYTRACING_POINTLIGHT_H
#define RAYTRACING_POINTLIGHT_H

#include "Light.h"

class PointLight : public Light {

    Vec3 position;
public:

    virtual Vec3 getOrigin() const;

    virtual void getLightData(const Vec3& hit_pos, Vec3& direction_out, float& distance_out, Vec3& intensity_out) const override;

    PointLight() = default;
    PointLight(const Vec3& position) : position(position) { };
    PointLight(const Vec3& position, const float& intensity) : position(position) {
        this->intensity = intensity;
    };

};


#endif //RAYTRACING_POINTLIGHT_H
