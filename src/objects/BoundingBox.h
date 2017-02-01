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

    virtual BoundingBox ComputeBBox() const override;

    BoundingBox ExtendsBy(Vec3 point) const {

        return BoundingBox {
                {   std::min(min.x, point.x),
                    std::min(min.y, point.y),
                    std::min(min.z, point.z)
                },
                {   std::max(max.x, point.x),
                    std::max(max.y, point.y),
                    std::max(max.z, point.z)
                }
        };
    }

    BoundingBox ExtendsBy(const BoundingBox& other) const {
        return ExtendsBy(other.min, other.max);
    }

    Vec3 GetCenter() const {
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

private:

    BoundingBox ExtendsBy(Vec3 min, Vec3 max) const {

        return ExtendsBy(min).ExtendsBy(max);

        Vec3 new_min {std::min(this->min.x, min.x),
                      std::min(this->min.y, min.y),
                      std::min(this->min.z, min.z)};

        Vec3 new_max {std::max(this->max.x, max.x),
                      std::max(this->max.y, max.y),
                      std::max(this->max.z, max.z)};

        return BoundingBox {new_min, new_max};
    }


};




#endif //PATHTRACER_BOUNDINGBOX_H
