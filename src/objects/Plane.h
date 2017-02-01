#ifndef RAYTRACING_PLANE_H
#define RAYTRACING_PLANE_H

#include "Intersectable.h"

class Plane : public Intersectable {

public:

    Vec3 origin {0, 0, 0};

    Vec3 normal {0, 1, 0};

    bool Intersect(const Ray& ray, float& dist_out) const override;

    SurfaceData GetSurfaceData(Vec3 pos, Vec3 ray_direction) const override;

    virtual BoundingBox ComputeBBox() const override;

    Plane(const Vec3 &origin, const Vec3 &normal);

    Plane() = default;
};

#endif //RAYTRACING_PLANE_H
