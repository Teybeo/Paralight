#ifndef PATHTRACER_TRIANGLE_H
#define PATHTRACER_TRIANGLE_H

#include "Intersectable.h"
#include "Object3D.h"
#include "TriMesh.h"

typedef struct SubMesh SubMesh;

class Triangle : public Intersectable {

    unsigned int A_index;
    unsigned int B_index;
    unsigned int C_index;

public:

    const TriMesh* trimesh_ptr;

    Triangle(const unsigned int* index_pointer, const TriMesh* triMesh)
            : trimesh_ptr{triMesh}
    {
        A_index = index_pointer[0];
        B_index = index_pointer[1];
        C_index = index_pointer[2];
    }

    bool Intersect(const Ray& ray, float& dist_out) const override;

    SurfaceData GetSurfaceData(Vec3 pos, Vec3 ray_direction) const override;

    virtual BoundingBox ComputeBBox() const;

    virtual Vec3 GetCenter() const;

    friend class OpenCLRenderer;

    friend CLObject3D GetCLObject3D(const Object3D& object);
};

#endif //PATHTRACER_TRIANGLE_H
