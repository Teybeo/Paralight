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
    
    Triangle(const int* index_pointer, const TriMesh* triMesh)
            : trimesh_ptr{triMesh}
    {
        A_index = (unsigned int) index_pointer[0];
        B_index = (unsigned int) index_pointer[1];
        C_index = (unsigned int) index_pointer[2];
    }

    bool Intersect(const Ray& ray, float& dist_out) const override;

    SurfaceData GetSurfaceData(Vec3 pos, Vec3 ray_direction) const override;

	BoundingBox ComputeBBox() const override;

	Vec3 GetCenter() const override;

    friend class OpenCLRenderer;

    friend CLObject3D GetCLObject3D(const Object3D& object);

    friend std::ostream& operator<< (std::ostream& out, const Triangle& tri);
};

#endif //PATHTRACER_TRIANGLE_H
