#ifndef PATHTRACER_SCENECONVERTER_H
#define PATHTRACER_SCENECONVERTER_H

#include <map>
#include <vector>
#include <set>
#include "math/Vec3.h"
#include "objects/Object3D.h"
#include "core/BVHCommons.h"

typedef struct Scene Scene;
typedef struct TriMesh TriMesh;
typedef struct BVH2 BVH2;

class SceneAdapter {


    std::vector<CLObject3D> object_array;
    std::vector<CLVec4> pos_array;
    std::vector<CLVec4> normal_array;
    std::vector<CLVec2> uv_array;
    std::vector<CLBrdf> brdf_array;
    std::vector<CLTextureInfo> info_array;
    std::vector<CLNode2> bvh_node_array;
    std::vector<TextureUbyte*> texture_array;
    int texture_array_size = 0;

public:

    SceneAdapter() = default;
    SceneAdapter(const Scene* scene);
    SceneAdapter(const std::set<Material*>& material_set);
    SceneAdapter(std::vector<std::unique_ptr<Object3D>>& objects, const std::set<Material*>& material_set);

    static std::map<TextureUbyte*, char> CreateBrdfArray(std::vector<CLBrdf>& brdf_array, const std::set<Material*>& material_set);
    static std::map<Object3D*, int> CreateCLObjectArray(std::vector<CLObject3D>& object_array, const std::vector<std::unique_ptr<Object3D>>& objects, const std::set<Material*>& material_set);

    const std::vector<CLObject3D>& GetObjectArray() const {
        return object_array;
    }

    const std::vector<CLBrdf>& GetBrdfArray() const {
        return brdf_array;
    }

    const std::vector<CLVec4>& GetPosArray() const {
        return pos_array;
    }

    const std::vector<CLVec4>& GetNormalArray() const {
        return normal_array;
    }

    const std::vector<CLVec2>& GetUvArray() const {
        return uv_array;
    }

    const std::vector<CLTextureInfo>& GetTextureInfoArray() const {
        return info_array;
    }

    const std::vector<CLNode2>& GetBvhNodeArray() const {
        return bvh_node_array;
    }

    const std::vector<TextureUbyte*>& GetTextureArray() const {
        return texture_array;
    }

    int GetTextureArraySize() const {
        return texture_array_size;
    }

private:

    void CreateTriangleDataArrays(const std::set<const TriMesh*> trimeshes);

    void CreateBvhNodeArray(BVH2* bvh_root, std::map<Object3D*, int>& obj_map);

    void CreateTextureInfoArray(std::map<TextureUbyte*, char>& texture_index_map);

    void CreateTextureArray(std::map<TextureUbyte*, char> map);
};

#endif //PATHTRACER_SCENECONVERTER_H
