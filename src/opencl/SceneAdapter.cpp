#include "SceneAdapter.h"

#include "core/Scene.h"
#include "objects/Triangle.h"

using std::vector;
using std::map;
using std::set;
using std::unique_ptr;
using std::shared_ptr;

static int SerializeBVH2(const Node2* node, vector<CLNode2>& bvh_node_array, vector<CLObject3D>& object_array, map<Object3D*, int>& obj_map);
static int FindObject(Object3D* obj, map<Object3D*, int>& obj_map);
static void SetSkipPointers2(vector<CLNode2>& bvh_node_array);
static int SetLastChildPointers2(CLNode2& node, vector<CLNode2>& node_array, int& node_count) ;

static char RegisterTexture(TextureUbyte* texture, map<TextureUbyte*, char>& texture_index_map);

template <typename T = float>
static void SetTextureParameter(map<TextureUbyte*, char>& texture_index_map, const shared_ptr<Texture>& tex, char& tex_index, T* scalar = nullptr);

CLBrdf GetCLBrdf(const Material* material, map<TextureUbyte*, char>& texture_index_map) ;
CLObject3D GetCLObject3D(const Object3D& object);

SceneAdapter::SceneAdapter(const Scene* scene) {

    map<Object3D*, int> obj_map = CreateCLObjectArray(object_array, scene->objects, scene->GetMaterialSet());
    CreateTriangleDataArrays(scene->GetTriMeshes());
    CreateBvhNodeArray(scene->bvh2, obj_map);

    map<TextureUbyte*, char> texture_index_map = CreateBrdfArray(brdf_array, scene->GetMaterialSet());
    CreateTextureArray(texture_index_map);
    CreateTextureInfoArray(texture_index_map);
}

SceneAdapter::SceneAdapter(const std::set<Material*>& material_set) {

    CreateBrdfArray(brdf_array, material_set);
}

SceneAdapter::SceneAdapter(vector<unique_ptr<Object3D>>& objects, const set<Material*>& material_set) {

    CreateCLObjectArray(object_array, objects, material_set);
}

map<Object3D*, int> SceneAdapter::CreateCLObjectArray(vector<CLObject3D>& object_array, const vector<unique_ptr<Object3D>>& objects, const set<Material*>& material_set) {

    map<Object3D*, int> obj_map;

    for (int i = 0; i < int(objects.size()); ++i) {
        
        Object3D* object = objects[i].get();
        
        CLObject3D cl_obj = GetCLObject3D(*object);

        cl_obj.emission = object->getEmissionIntensity() != -1 ? object->getEmission() : -1;

        // The index of the Material* in the MaterialSet should be the same as the index of
        // its CL counterpart in the cl_brdf array (because std::set is ordered)
        const auto& it = material_set.find(object->material);
        cl_obj.material_index = short(distance(material_set.begin(), it));

        object_array.push_back(cl_obj);
        obj_map.emplace(object, i);
    }

    return obj_map;
}

CLObject3D GetCLObject3D(const Object3D& object) {

    CLObject3D cl_obj {};

    Intersectable* shape = object.shape;

    if (typeid(*shape) == typeid(Sphere)) {
        Sphere* sphere = static_cast<Sphere*>(shape);
        cl_obj.type = 1;
        cl_obj.pos = sphere->origin;
        cl_obj.radius = sphere->radius * sphere->radius; // Precompute radius²
    }
    else if (typeid(*shape) == typeid(Plane)) {
        Plane* plane = static_cast<Plane*>(shape);
        cl_obj.type = 2;
        cl_obj.pos = plane->origin;
        cl_obj.normal = plane->normal;
    }
    else if (typeid(*shape) == typeid(Triangle)) {
        Triangle* triangle = static_cast<Triangle*>(shape);
        cl_obj.type = 3;
        cl_obj.A_index = triangle->A_index;
        cl_obj.B_index = triangle->B_index;
        cl_obj.C_index = triangle->C_index;
        cl_obj.has_uv  = triangle->trimesh_ptr->uv_array.empty() == false;
    }

    return cl_obj;
}

map<TextureUbyte*, char> SceneAdapter::CreateBrdfArray(vector<CLBrdf>& brdf_array, const set<Material*>& material_set) {

    // This map isn't used because we don't support runtime texture changes (yet ?)
    map<TextureUbyte*, char> texture_index_map;

    for (const auto& material : material_set) {

        CLBrdf cl_brdf = GetCLBrdf(material, texture_index_map);

        brdf_array.push_back(cl_brdf);
    }

    return texture_index_map;
}

CLBrdf GetCLBrdf(const Material* material, map<TextureUbyte*, char>& texture_index_map) {

    CLBrdf cl_brdf = {};
    cl_brdf.albedo = 0.7f;
    cl_brdf.reflection = 0.4f;

    const OldMaterial* standard = dynamic_cast<const OldMaterial*>(material);

    if (standard != nullptr) {
        cl_brdf.type = LAMBERTIAN | MICROFACET;
        cl_brdf.use_metalness = false;
        cl_brdf.packed_metal_rough = false;
        
        SetTextureParameter(texture_index_map, standard->GetAlbedo(), cl_brdf.albedo_map_index, &cl_brdf.albedo);
        SetTextureParameter(texture_index_map, standard->GetReflectance(), cl_brdf.reflection_map_index, &cl_brdf.reflection);
        SetTextureParameter(texture_index_map, standard->GetRoughness(), cl_brdf.roughness_map_index, &cl_brdf.roughness);
        SetTextureParameter(texture_index_map, standard->GetNormal(), cl_brdf.normal_map_index);
    }

    const MetallicWorkflow* metallic_workflow = dynamic_cast<const MetallicWorkflow*>(material);

    if (metallic_workflow != nullptr) {
        cl_brdf.type = LAMBERTIAN | MICROFACET;
        cl_brdf.use_metalness = true;
        cl_brdf.packed_metal_rough = metallic_workflow->IsPacked();

        SetTextureParameter(texture_index_map, metallic_workflow->GetAlbedo(), cl_brdf.albedo_map_index, &cl_brdf.albedo);
        SetTextureParameter(texture_index_map, metallic_workflow->GetMetallic(), cl_brdf.metalness_map_index, &cl_brdf.metalness);
        SetTextureParameter(texture_index_map, metallic_workflow->GetRoughness(), cl_brdf.roughness_map_index, &cl_brdf.roughness);
        SetTextureParameter(texture_index_map, metallic_workflow->GetNormal(), cl_brdf.normal_map_index);
    }

    const LambertianMaterial* lambertian = dynamic_cast<const LambertianMaterial*>(material);

    if (lambertian != nullptr) {
        cl_brdf.type = LAMBERTIAN;

        SetTextureParameter(texture_index_map, lambertian->GetAlbedo(), cl_brdf.albedo_map_index, &cl_brdf.albedo);
        cl_brdf.normal_map_index = -1;
    }

    return cl_brdf;
}

template <typename T = float>
void SetTextureParameter(map<TextureUbyte*, char>& texture_index_map, const shared_ptr<Texture>& tex, char& tex_index, T* scalar) {

    ValueTexture<T>* value_texture = dynamic_cast<ValueTexture<T>*>(tex.get());
    TextureUbyte* texture = dynamic_cast<TextureUbyte*>(tex.get());

    if (texture != nullptr)
    {
        tex_index = RegisterTexture(texture, texture_index_map);
    }
    else if (value_texture != nullptr) {
        *scalar = value_texture->value;
        tex_index = -1;
    }
    else {
        tex_index = -1;
    }
}
/**
 * Register this texture name along with an unique index but doesn't actually copy it to the cl device
 * @return The index of the texture to access it from the cl device
 */
char RegisterTexture(TextureUbyte* texture, map<TextureUbyte*, char>& texture_index_map) {

    auto it = texture_index_map.find(texture);
    if (it != texture_index_map.end()) {
        return (*it).second;
    }

    char index = (char) texture_index_map.size();

    texture_index_map.emplace(texture, index);

    return index;
}

void SceneAdapter::CreateTextureArray(std::map<TextureUbyte*, char> texture_index_map) {

    texture_array.resize(texture_index_map.size());

    for (const auto& item : texture_index_map) {
        texture_array[item.second] = item.first;
    }
}

void SceneAdapter::CreateTextureInfoArray(map<TextureUbyte*, char>& texture_index_map) {

    int total_texture_size_bytes = 0;

    info_array.resize(texture_index_map.size());

    for (const auto& item : texture_index_map) {
        CLTextureInfo info {};
        info.width = (int) item.first->width;
        info.height = (int) item.first->height;
        info.mapping = 0;
        info.byte_offset = total_texture_size_bytes;
        total_texture_size_bytes += item.first->GetSize();

        info_array[item.second] = info;
    }
    texture_array_size = total_texture_size_bytes;
}


void SceneAdapter::CreateTriangleDataArrays(const set<const TriMesh*> trimeshes) {

    for (const auto& trimesh : trimeshes) {

        const auto& tri_pos_array = trimesh->pos_array;
        const auto& tri_normal_array = trimesh->normal_array;
        const auto& tri_uv_array = trimesh->uv_array;

        for (size_t i = 0; i < tri_pos_array.size(); ++i) {
            pos_array.   push_back(CLVec4{tri_pos_array[i].x   , tri_pos_array[i].y   , tri_pos_array[i].z   , 0});
            normal_array.push_back(CLVec4{tri_normal_array[i].x, tri_normal_array[i].y, tri_normal_array[i].z, 1});
        }
        for (const auto& uv : tri_uv_array) {
            uv_array.push_back(CLVec2{uv.x, uv.y});
        }
    }
}

void SceneAdapter::CreateBvhNodeArray(BVH2* bvh_root, map<Object3D*, int>& obj_map) {

    bvh_node_array.reserve((size_t)bvh_root->GetNodeCount());

    SerializeBVH2(bvh_root->GetRoot(), bvh_node_array, object_array, obj_map);

    SetSkipPointers2(bvh_node_array);
}

int SerializeBVH2(const Node2* node, vector<CLNode2>& bvh_node_array, vector<CLObject3D>& object_array, map<Object3D*, int>& obj_map) {

    CLNode2 cl_node;
    cl_node.bbox.max = node->bbox.max;
    cl_node.bbox.min = node->bbox.min;
    cl_node.obj_index = FindObject(node->object, obj_map);
    cl_node.left_child = -1;
    cl_node.right_child = -1;

    bvh_node_array.push_back(cl_node);

    int index = (int) (bvh_node_array.size() - 1);

    if (node->left_child)
        bvh_node_array[index].left_child = SerializeBVH2(node->left_child.get(), bvh_node_array, object_array, obj_map);

    if (node->right_child)
        bvh_node_array[index].right_child = SerializeBVH2(node->right_child.get(), bvh_node_array, object_array, obj_map);

    return index;
}

int FindObject(Object3D* obj, map<Object3D*, int>& obj_map) {

    if (obj == nullptr)
        return -1;

    auto it = obj_map.find(obj);
    if (it != obj_map.end()) {
        return it->second;
    }

    return -1;
}

void SetSkipPointers2(vector<CLNode2>& bvh_node_array) {

    int tmp = 0;
    SetLastChildPointers2(bvh_node_array[0], bvh_node_array, tmp);

    bvh_node_array[0].bbox.pad2 = -1;

    for (size_t i = 0; i < bvh_node_array.size(); ++i) {

        CLNode2& node = bvh_node_array[i];

        // Only internal nodes
        if (node.bbox.pad != -1.f)
        {
            // Keep last child index of this node
            int temp = (int) node.bbox.pad;

            if (node.left_child >= 0 && node.right_child >= 0)
                bvh_node_array[node.left_child].bbox.pad2 = node.right_child;

            // Make the last child of this node point to this node sucessor (ths sibling of this node or -1 if root)
            bvh_node_array[temp].bbox.pad2 = node.bbox.pad2;
        }
    }
}

int SetLastChildPointers2(CLNode2& node, vector<CLNode2>& node_array, int& node_count) {

    int node_index = node_count;
    node_count++;

    // Leaf node
    if (node.obj_index != -1) {
        node.bbox.pad = -1;
    }
    else {

        int last_child_index = -1;
        if (node.left_child != -1)
            last_child_index = SetLastChildPointers2(node_array[node.left_child], node_array, node_count);

        if (node.right_child != -1)
            last_child_index = SetLastChildPointers2(node_array[node.right_child], node_array, node_count);

        // This internal node point to its last child
        node.bbox.pad = last_child_index;
    }

    return node_index;

}