#ifndef PATHTRACER_INTERSECTABLE_H
#define PATHTRACER_INTERSECTABLE_H

#include "core/Ray.h"
#include "SurfaceData.h"

class BoundingBox;
class CLObject3D;

class Intersectable {

public:

    virtual bool Intersect(const Ray& ray, float& dist_out) const = 0;

    virtual SurfaceData GetSurfaceData(Vec3 pos, Vec3 ray_direction) const = 0;

    virtual BoundingBox ComputeBBox() const = 0;

    virtual Vec3 GetCenter() const;
};

#endif //PATHTRACER_INTERSECTABLE_H
