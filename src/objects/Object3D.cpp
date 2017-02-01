#include "Object3D.h"

#include "Triangle.h"

Object3D::Object3D() {
//    Vec3 albedo = Vec3{(std::rand() % 1000) / 1000.f, (std::rand() % 1000) / 1000.f, (std::rand() % 1000) / 1000.f};

//    material = new LambertianMaterial(albedo);
    brdf = nullptr;
    spec = nullptr;
}

Object3D::Object3D(const Vec3& origin) {
//    Vec3 albedo = Vec3{(std::rand() % 1000) / 1000.f, (std::rand() % 1000) / 1000.f, (std::rand() % 1000) / 1000.f};

//    material = new LambertianMaterial(albedo);
}

std::vector<std::unique_ptr<Object3D>> Object3D::CreateTriMesh(std::string filename, std::string directory) {

    TriMesh* mesh = new TriMesh{filename, directory};
    std::vector<Triangle>& triangles = mesh->GetTriangles();

    std::vector<std::unique_ptr<Object3D>> objects;

    for (size_t i = 0; i < triangles.size(); ++i) {
        Object3D* obj = new Object3D;
        obj->shape = &triangles[i];
        obj->material = mesh->GetTriangleMaterial(i);
        objects.push_back(std::unique_ptr<Object3D>(obj));
    }

    return objects;
}
