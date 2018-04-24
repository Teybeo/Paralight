#ifndef PATHTRACER_BOUNDINGBOX_H
#define PATHTRACER_BOUNDINGBOX_H

#include "Intersectable.h"

#define HUGE_NUMBER 10000000.f

struct alignas(16) CLBoundingBox {
    Vec3 min;
    float pad;
    Vec3 max;
    float pad2;
};

class BoundingBox : public Intersectable {

public:

    Vec3 min = HUGE_NUMBER;
    Vec3 max = -HUGE_NUMBER;

    BoundingBox() = default;
    explicit BoundingBox(Vec3 min, Vec3 max) : min{min}, max{max}
    { }

    bool Intersect(const Ray& ray, float& dist_out) const override;

    SurfaceData GetSurfaceData(Vec3 pos, Vec3 ray_direction) const override;

	BoundingBox ComputeBBox() const override;

    BoundingBox& ExtendsBy(const Vec3& point) {

        min.x = std::min(min.x, point.x);
        min.y = std::min(min.y, point.y);
        min.z = std::min(min.z, point.z);

        max.x = std::max(max.x, point.x);
        max.y = std::max(max.y, point.y);
        max.z = std::max(max.z, point.z);
        return *this;
    }

    BoundingBox& ExtendsBy(const BoundingBox& other) {
        return ExtendsBy(other.min).ExtendsBy(other.max);
    }

	Vec3 GetCenter() const override {
        return (min + max) / 2.f;
    }

    bool Encloses(Vec3 point) const;
    bool EnclosesInclusive(Vec3 point) const;

    bool IntersectBranched(const FastRay& ray, float& distance) const;
    bool IntersectFast(const FastRay& ray, float& distance) const;

    bool EnclosesInclusive(BoundingBox bbox) const;

    bool operator==(const BoundingBox& other);

    friend std::ostream& operator<<(std::ostream& out, const BoundingBox& bbox);

    char GetLargestAxis();

    float GetSurfaceArea() const;
};




#endif //PATHTRACER_BOUNDINGBOX_H
