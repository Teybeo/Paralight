#include "BoundingBox.h"

bool BoundingBox::Intersect(const Ray& ray, float& dist_out) const {

    return IntersectBranched(ray, dist_out);
//    return IntersectFast(ray, dist_out);
}

bool BoundingBox::IntersectBranched(const FastRay& ray, float& distance) const {

    float t_nearest_x = (min.x - ray.origin.x) * ray.direction_inv.x;
    float t_farthest_x = (max.x - ray.origin.x) * ray.direction_inv.x;

    // Swap the intersections distances if needed to handle all directions
    if (t_nearest_x > t_farthest_x)
        std::swap(t_nearest_x, t_farthest_x);

    float t_nearest_y = (min.y - ray.origin.y) * ray.direction_inv.y;
    float t_farthest_y = (max.y - ray.origin.y) * ray.direction_inv.y;

    if (t_nearest_y > t_farthest_y)
        std::swap(t_nearest_y, t_farthest_y);

    if ((t_nearest_x > t_farthest_y) || (t_nearest_y > t_farthest_x))
        return false;

    t_nearest_x = std::max(t_nearest_y, t_nearest_x);
    t_farthest_x = std::min(t_farthest_y, t_farthest_x);

    float tzmin = (min.z - ray.origin.z) * ray.direction_inv.z;
    float tzmax = (max.z - ray.origin.z) * ray.direction_inv.z;

    if (tzmin > tzmax) std::swap(tzmin, tzmax);

    if ((t_nearest_x > tzmax) || (tzmin > t_farthest_x))
        return false;

    t_nearest_x = std::max(tzmin, t_nearest_x);
    t_farthest_x = std::min(tzmax, t_farthest_x);

//    distance = (t_nearest_x > 0) ? t_nearest_x : t_farthest_x;
//
//    return distance > 0;

    distance = t_nearest_x;

    return (t_nearest_x > 0) || (t_farthest_x > 0);
}
#if 1
bool BoundingBox::IntersectFast(const FastRay& ray, float& distance) const {

    float t_nearest;
    float t_farthest;

    float t_nearest_axis = (min.x - ray.origin.x) * ray.direction_inv.x;
    float t_farthest_axis = (max.x - ray.origin.x) * ray.direction_inv.x;

    t_nearest = std::min(t_nearest_axis, t_farthest_axis);
    t_farthest = std::max(t_nearest_axis, t_farthest_axis);

    t_nearest_axis = (min.y - ray.origin.y) * ray.direction_inv.y;
    t_farthest_axis = (max.y - ray.origin.y) * ray.direction_inv.y;

    t_nearest = std::max(t_nearest, std::min(t_nearest_axis, t_farthest_axis));
    t_farthest = std::min(t_farthest, std::max(t_nearest_axis, t_farthest_axis));

    t_nearest_axis = (min.z - ray.origin.z) * ray.direction_inv.z;
    t_farthest_axis = (max.z - ray.origin.z) * ray.direction_inv.z;

    t_nearest = std::max(t_nearest, std::min(t_nearest_axis, t_farthest_axis));
    t_farthest = std::min(t_farthest, std::max(t_nearest_axis, t_farthest_axis));

//    if (t_farthest < t_nearest)
//        return false;

//    distance = (t_nearest > 0) ? t_nearest : t_farthest;
//
//    return distance > 0;
    distance = t_nearest;

    return (t_nearest <= t_farthest) && ((t_nearest > 0) || (t_farthest > 0));
}
#else
bool BoundingBox::IntersectFast(const FastRay& ray, float& distance) const {

    float t_nearest;
    float t_farthest;

    float t_nearest_x = (min.x - ray.origin.x) * ray.direction_inv.x;
    float t_farthest_x = (max.x - ray.origin.x) * ray.direction_inv.x;

    t_nearest = std::min(t_nearest_x, t_farthest_x);
    t_farthest = std::max(t_nearest_x, t_farthest_x);

    float t_nearest_y = (min.y - ray.origin.y) * ray.direction_inv.y;
    float t_farthest_y = (max.y - ray.origin.y) * ray.direction_inv.y;

    t_nearest = std::max(t_nearest, std::min(t_nearest_y, t_farthest_y));
    t_farthest = std::min(t_farthest, std::max(t_nearest_y, t_farthest_y));

    float t_nearest_z = (min.z - ray.origin.z) * ray.direction_inv.z;
    float t_farthest_z = (max.z - ray.origin.z) * ray.direction_inv.z;

    t_nearest = std::max(t_nearest, std::min(t_nearest_z, t_farthest_z));
    t_farthest = std::min(t_farthest, std::max(t_nearest_z, t_farthest_z));

//    if (t_farthest < t_nearest)
//        return false;

//    distance = (t_nearest > 0) ? t_nearest : t_farthest;
//
//    return distance > 0;
    distance = t_nearest;

    return (t_nearest < t_farthest) && ((t_nearest > 0) || (t_farthest > 0));
}
#endif
SurfaceData BoundingBox::GetSurfaceData(Vec3 pos, Vec3 ray_direction) const {

    Vec3 normal;
    if (pos.x == min.x)
        normal = {-1, 0, 0};
    else if (pos.x == max.x)
        normal = {1, 0, 0};
    else if (pos.y == min.y)
        normal = {0, -1, 0};
    else if (pos.y == max.y)
        normal = {0, 1, 0};
    else if (pos.z == min.z)
        normal = {0, 0, -1};
    else if (pos.z == min.z)
        normal = {0,  0, 1};

    SurfaceData surface_data;
    surface_data.normal = normal;
    return surface_data;
}

bool BoundingBox::Encloses(Vec3 point) const {
    return point > min && point < max;
}

bool BoundingBox::EnclosesInclusive(Vec3 point) const {
    return min < point && point <= max;
}

bool BoundingBox::EnclosesInclusive(BoundingBox bbox) const {
    return EnclosesInclusive(bbox.min) && EnclosesInclusive(bbox.max);
}

BoundingBox BoundingBox::ComputeBBox() const {
    return BoundingBox(*this);
}

std::ostream& operator<<(std::ostream& out, const BoundingBox& bbox) {
    out.unsetf ( std::ios::floatfield );
    out.precision(9);
    out << "X: [" << bbox.min.x << ", " << bbox.max.x << "]  ";
    out << "Y: [" << bbox.min.y << ", " << bbox.max.y << "]  ";
    out << "Z: [" << bbox.min.z << ", " << bbox.max.z << "]  ";

    return out;
}

bool BoundingBox::operator==(const BoundingBox& other) {
    return min == other.min && max == other.max;
}

char BoundingBox::GetLargestAxis() {

    Vec3 diag = max - min;
    float largest_length = diag.max();

    for (char i = 0; i < 3; ++i) {
        if (diag[i] == largest_length)
            return i;
    }

    return 0;
}

float BoundingBox::GetSurfaceArea() const {

    float width  = std::abs(max.x - min.x);
    float height = std::abs(max.y - min.y);
    float depth  = std::abs(max.z - min.z);

    return (width * height * 2.f) + (width * depth * 2.f) + (height * depth * 2.f);
}