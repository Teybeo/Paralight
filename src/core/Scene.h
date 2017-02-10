#ifndef PATHTRACER_SCENE_H
#define PATHTRACER_SCENE_H

#include "Texture.h"
//#include "BVH.h"
#include "BVH2.h"
#include "BVH.h"

#include <memory>
#include <set>

typedef struct TriMesh TriMesh;
typedef struct Object3D Object3D;

class Scene {

private:

    int triangle_count = 0;
    int vertex_count = 0;

    void Load_CornellBox();
    void Load_SphereGrid(int nb);
    void Load_Floor();
    void Load_MirrorRoom();
    void Load_TexturedSphere();
    void Load_TriMesh();

    void CheckObjectsOrder();

public:
    Scene();

    BVH2* bvh2;
    BVH bvh;
    std::vector<std::unique_ptr<Object3D>> objects;
    std::set<Material*> material_set;
    std::unique_ptr<TextureFloat> env_map;

    float yz_angle = 0;
    float xz_angle = 0;
    Vec3 cam_pos {0, 0, 0};
    float debug_scale = 1;
    bool has_changed = false;
    bool env_map_has_changed = false;

    BoundingBox ComputeBBox() const;

    std::set<const TriMesh*> GetTriMeshes() const;

    int GetTriangleCount() const {
        return triangle_count;
    }

    int GetVertexCount() const {
        return vertex_count;
    }

    std::set<Material*> GetMaterialSet() const {
        return material_set;
    }
};

#endif //PATHTRACER_SCENE_H
