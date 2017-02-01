#include "Plane.h"

#include "BoundingBox.h"
#include "Object3D.h"

Plane::Plane(const Vec3& origin, const Vec3& normal) : origin{origin}, normal{normal} {
}
/**
 * for a plane defined by the normal N and position PlaneO, a point P is lying on this plane if
 * the vector P_P0 is perpendicular to the plane normal N, that is (P - PlaneO).N = 0
 * The point P is expressed with the ray parametric equation is: Ro + R * t
 * The equation is thus rewritten to: (Ro + R * t - PlaneO).N = 0
 * Solving for t gives: t = ((PlaneO - Ro).N) / (R.N)
*/
#define DISK_RADIUS (4.f)
#define DISK_RADIUS_SQUARED (DISK_RADIUS * DISK_RADIUS)

#if 1
bool Plane::Intersect(const Ray& ray, float& depth_out) const {

    // Using the opposite of the plane normal
    float denominator = 1 / -normal.dot(ray.direction);

    // Single-sided intersection: don't intersect if normal is pointing away from the ray
//    if (denominator <= 0)
//        return false;

    depth_out = -normal.dot(origin - ray.origin) * denominator;

    // Defines infinite planes
//    return depth_out > 0.00001f;

    // Or disks defined by the dist_to_plane_origin (radius)
    if (depth_out <= 0.00001f)
        return false;

    float dist_to_plane_origin = ((ray.origin + ray.direction * depth_out) - origin).lengthSquared();

    return dist_to_plane_origin <= DISK_RADIUS_SQUARED;
}
#else
bool Plane::intersect(const Ray &ray, float &depth_out) const {

    // Using the opposite of the plane normal
    float denominator = -normal.dot(ray.direction);

    // Single-sided intersection: don't intersect if normal is pointing away from the ray
//    if (denominator <= 0)
//        return false;

    depth_out = -normal.dot(position - ray.position) / denominator;

    // Defines infinite planes
//    return depth_out > 0.00001f;

    // Or disks defined by the dist_to_plane_origin (radius)
    if (depth_out <= 0.00001f)
        return false;

    float dist_to_plane_origin = ((ray.position + ray.direction * depth_out) - position).lengthSquared();

    return dist_to_plane_origin <= DISK_RADIUS_SQUARED;
}
#endif

BoundingBox Plane::ComputeBBox() const {

    return BoundingBox(origin - DISK_RADIUS, origin + DISK_RADIUS);
}

SurfaceData Plane::GetSurfaceData(Vec3 pos, Vec3 ray_direction) const {

    SurfaceData surface_data;

    // Double-Sided plane normal
    surface_data.normal = (normal.dot(-ray_direction) < 0) ? -normal : normal;
    surface_data.uv = 0;

    return surface_data;
}

