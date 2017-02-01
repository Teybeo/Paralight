#ifndef PATHTRACER_TRIMESH_H
#define PATHTRACER_TRIMESH_H

#include <atomic>
#include <set>
#include "Object3D.h"
#include "Sphere.h"

class Triangle;
typedef struct aiScene aiScene;

class TriMesh {

    std::vector<Vec3> pos_array;
    std::vector<Vec3> normal_array;
    std::vector<Vec3> uv_array;
    std::vector<Vec3> tangent_array;
    std::vector<Vec3> bitangent_array;
    std::vector<unsigned int> index_array;
    unsigned int vertex_count = 0;

    std::vector<std::unique_ptr<Material>> materials;
    std::vector<Triangle> triangles;
    std::vector<unsigned int> triangle_to_material;

    static std::atomic_int triangle_test_count;
    static std::atomic_int triangle_hit_count;

    Sphere sphere_bounds;

public:

    friend class OpenCLRenderer;
    friend class SceneAdapter;
    friend class Triangle;
    friend CLObject3D GetCLObject3D(const Object3D& object);

    TriMesh() = default;
    TriMesh(const std::string& filename, std::string directory = "");

    void ImportAssimpMesh(const aiScene* ai_scene, std::string basic_string);

    std::vector<Triangle>& GetTriangles() {
        return triangles;
    }

    Material* GetTriangleMaterial(int i) {
        return materials[triangle_to_material[i]].get();
    }

    unsigned int GetVertexCount() const {
        return vertex_count;
    }

    static void ClearCounters() {
        triangle_test_count = 0;
        triangle_hit_count = 0;
    }

    static int GetTestCount() {
        return triangle_test_count;
    }

    static int GetHitCount() {
        return triangle_hit_count;
    }
};


#endif //PATHTRACER_TRIMESH_H
