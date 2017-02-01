#ifndef RAYTRACING_SPHERE_H
#define RAYTRACING_SPHERE_H

#include "Intersectable.h"

class Sphere : public Intersectable {

public:

    Vec3 origin {0, 0, 0};

    float radius = 1;

    Sphere();

    Sphere(float x, float y, float z, float radius = 1);

    SurfaceData GetSurfaceData(Vec3 pos, Vec3 ray_direction) const override;

    bool Intersect(const Ray& ray, float& dist_out) const override;

    virtual BoundingBox ComputeBBox() const override;

    virtual Vec3 GetCenter() const;

    void setRadius(float i);
};

#endif //RAYTRACING_SPHERE_H
