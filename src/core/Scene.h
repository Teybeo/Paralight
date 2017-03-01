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
    std::string prefix = "../../models/";

public:

    BVH2* bvh2;
    BVH bvh;
    std::vector<std::unique_ptr<Object3D>> objects;
    std::set<Material*> material_set;
    std::unique_ptr<TextureFloat> env_map;

    float yz_angle = 0;
    float xz_angle = 0;
    Vec3 cam_pos {0, 0, 10};
    float debug_scale = 1;
    bool material_has_changed = false;
    bool envmap_has_changed = false;
    bool emission_has_changed = false;
    bool model_has_changed = false;

    Scene(std::string file = "");
    ~Scene();

    void LoadObjects(const std::string& file);
//    void Clear();

    BoundingBox ComputeBBox() const;

    std::set<const TriMesh*> GetTriMeshes() const;

    bool HasChanged() const {
        return material_has_changed || envmap_has_changed || emission_has_changed || model_has_changed;
    }

    int GetTriangleCount() const {
        return triangle_count;
    }

    int GetVertexCount() const {
        return vertex_count;
    }

    std::set<Material*> GetMaterialSet() const {
        return material_set;
    }

    void Clear();

private:
    void Load_CornellBox();
    void Load_SphereGrid(int nb);
    void Load_Floor();
    void Load_MirrorRoom();
    void Load_TexturedSphere();
    void LoadSomeLights();
    void LoadModel(const std::string& file);

    void CheckObjectsOrder();

    void PostProcess();
};

#endif //PATHTRACER_SCENE_H
