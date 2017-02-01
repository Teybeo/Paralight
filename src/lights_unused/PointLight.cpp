#include "PointLight.h"

Vec3 PointLight::getOrigin() const {
    return position;
}

void PointLight::getLightData(const Vec3& hit_pos, Vec3& direction_out, float& distance_out, Vec3& intensity_out) const {
    direction_out = (position - hit_pos);
    distance_out = direction_out.length();
//    intensity_out = (color * intensity) / powf(1 + distance_out, 2);
    intensity_out = (color * intensity) / (1 + powf(distance_out, 2));
    direction_out.normalize();
    distance_out +=  0.0001f;

}
