#ifndef RAYTRACING_RAY_H
#define RAYTRACING_RAY_H

#include "math/Vec3.h"

class Ray {
public:

    Vec3 origin;
    Vec3 direction;

    Ray() = default;
    Ray(const Ray& ray) : origin{ray.origin}, direction{ray.direction} { }
    Ray(const Vec3& origin, const Vec3& direction) : origin{origin}, direction{direction} { }

    Ray(const Vec3& _origin, int x, int y, int width, int height, float aspect_ratio, float fov_factor) {
        direction.x =  (2 * (x + 0.5f) / width) - 1;
        direction.y = -(2 * (y + 0.5f) / height) + 1;
        direction.x *= aspect_ratio * fov_factor;
        direction.y *= fov_factor;
        direction.z = -1;
        direction.normalize();
        origin = _origin;
    }

    Ray(int x, int y, int width, int height, float aspect_ratio, float fov_factor) {
        direction.x =  (2 * (x + 0.5f) / width) - 1;
        direction.y = -(2 * (y + 0.5f) / height) + 1;
        direction.x *= aspect_ratio * fov_factor;
        direction.y *= fov_factor;
        direction.z = -1;
        direction.normalize();
    }
};
struct alignas(16) CLRay {
    Vec3 origin;
    char pad;
    Vec3 direction;
};

class FastRay : public Ray {
public:
    FastRay(const Ray& ray) : Ray{ray} {
        direction_inv = Vec3{1.f} / ray.direction;
    }

    Vec3 direction_inv;
};

#endif //RAYTRACING_RAY_H


