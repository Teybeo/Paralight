#include "Sphere.h"

#include <iostream>

#include "BoundingBox.h"
#include "Object3D.h"

Sphere::Sphere() {
//    radius = 0.2f + (std::rand() % 100) / 90.f;
    radius = 1;
}

Sphere::Sphere(float x, float y, float z, float radius) {
//    radius = 0.2f + (std::rand() % 100) / 90.f;
    origin = Vec3{x, y, z};
    this->radius = radius;
}

bool Sphere::Intersect(const Ray& ray, float& dist_out) const {

    Vec3 origin_to_center = origin - ray.origin;

    float proj = origin_to_center.dot(ray.direction);
    if (proj < 0)
        return false;

    float dxd = origin_to_center.dot(origin_to_center) - (proj * proj);
    if (dxd > (radius * radius))
        return false;

    float half_inner_dist = std::sqrt(radius * radius - dxd);

    float t0 = proj - half_inner_dist;
    float t1 = proj + half_inner_dist;

    if (t0 < 0 && t1 < 0)
        return false;

    dist_out = (t0 > 0) ? t0 : t1;

    return true;
//    return t0 > 0;
}

SurfaceData Sphere::GetSurfaceData(Vec3 pos, Vec3 ray_direction) const {

    SurfaceData surface_data;
    surface_data.normal = (pos - origin).normalize();
    surface_data.uv = surface_data.normal.SphericalToCartesian();
    return surface_data;
}

BoundingBox Sphere::ComputeBBox() const {

    return BoundingBox{origin - radius, origin + radius};
}

void Sphere::setRadius(float _radius) {
    radius = _radius;
}

Vec3 Sphere::GetCenter() const {
    return origin;
}
