#ifndef TEST3D_OBJECT3D_H
#define TEST3D_OBJECT3D_H

#include <assert.h>
#include "core/Material.h"
#include "math/Matrix.h"
#include "material/BrdfStack.h"
#include "core/Ray.h"
#include "Intersectable.h"
#include "Sphere.h"
#include "Plane.h"
#include "BoundingBox.h"

class Object3D {

protected:
    Object3D(const Vec3& origin);

    Vec3 emission_color{0, 0, 0};
    float emission_intensity = -1;

public:

    Intersectable* shape = nullptr;
    Material* material = nullptr;
    Brdf* brdf;
    Brdf* spec;

    Object3D();

    static Object3D* CreateSphere(float x, float y, float z, float radius = 1) {
        Object3D* obj = new Object3D{};
        obj->shape = new Sphere{x, y, z, radius};
        return obj;
    }

    static Object3D* CreatePlane(Vec3 origin, Vec3 normal) {
        Object3D* obj = new Object3D {};
        obj->shape = new Plane{origin, normal};
        return obj;
    }

    static std::vector<std::unique_ptr<Object3D>> CreateTriMesh(std::string filename, std::string directory = "");

    bool Intersect(const Ray& ray, float& dist_out) {
        return shape->Intersect(ray, dist_out);
    }

    SurfaceData GetSurfaceData(Vec3 pos, Vec3 ray_direction) {
        SurfaceData surface_data = shape->GetSurfaceData(pos, ray_direction);
        surface_data.material = material;
        return surface_data;
    }

    virtual BoundingBox ComputeBBox() const {
        return shape->ComputeBBox();
    }

    virtual Vec3 GetCenter() const {
//        return shape->ComputeBBox().GetCenter();
        return shape->GetCenter();
    }

    //TODO: Clean this in an EmissiveMaterial ?
    void setEmission(const Vec3& color, float intensity = 1) {
        float max = color.max();
        if (max > 1) {
            emission_color = color / max;
            emission_intensity = max;
        } else {
            emission_color = color;
            emission_intensity = intensity;
        }
    }

    Vec3 getEmission() const {
        return emission_color * emission_intensity;
    }

    void setEmissionColor(const Vec3& color) {
        this->emission_color = color;
    }

    const Vec3& getEmissionColor() const {
        return emission_color;
    }

    void setEmissionIntensity(float intensity) {
        this->emission_intensity = intensity;
    }

    float getEmissionIntensity() const {
        return emission_intensity;
    }

};

struct alignas(16) CLObject3D {
    Vec3 pos;               // Object3D
    float pad;
    Vec3 emission;          // Object3D
    float pad2;
    Vec3 normal;            // Plane
    float pad4;
    float radius;           // Sphere
    unsigned int A_index;   // Triangle
    unsigned int B_index;   // Triangle
    unsigned int C_index;   // Triangle
    char type;              // Object3D
    short material_index;
    char has_uv;            // Triangle
    char pad5[2];
};

#endif //TEST3D_OBJECT3D_H