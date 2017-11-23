#ifndef PATHTRACER_MATERIAL_H
#define PATHTRACER_MATERIAL_H

#include <memory>
#include <iostream>
#include <map>
#include <math/Vec3.h>
#include <material/BrdfStack.h>
#include <objects/SurfaceData.h>

#include "Texture.h"

//#define USE_GLTF_LIB

// Forward declaration to avoid big include files
typedef struct aiMaterial aiMaterial;

#ifdef USE_GLTF_LIB
namespace tinygltf {
    struct Model;
}
#endif

class Material {

protected:
    std::unique_ptr<BrdfStack> brdf_stack = nullptr;

public:

    Material() {
        std::cout << "Material ctor" << std::endl;
    }

    Material(BrdfStack* brdf_stack) : brdf_stack(brdf_stack) {
    }

    Material(const Material& other) {
        std::cout << "Material copy ctor" << std::endl;
    }
    
    virtual ~Material() {
//        std::cout << "Material dtor" << std::endl;
    };

    virtual BrdfStack* CreateBSDF(const SurfaceData& surface_data, Vec3& shading_normal) {
        return nullptr;
    }

    std::unique_ptr<BrdfStack>::pointer GetBrdfStack() {
        return brdf_stack.get();
    }
};

// TODO: Decoupling of asset workflow and material representation

class OldMaterial : public Material {

    std::shared_ptr<Texture> albedo_map;
    std::shared_ptr<Texture> roughness_map;
    std::shared_ptr<Texture> reflectance_map;
    std::shared_ptr<Texture> normal_map;

public:

    OldMaterial() = default;
    
    OldMaterial(aiMaterial* ai_material, const std::string& directory);

    OldMaterial(const std::shared_ptr<Texture>& albedo_map, const std::shared_ptr<Texture>& roughness_map, const std::shared_ptr<Texture>& reflectance_map, const std::shared_ptr<Texture> normal_map = nullptr)
            : Material{new StandardStack(0, 0, 0)}, albedo_map(albedo_map), roughness_map(roughness_map), reflectance_map(reflectance_map), normal_map{normal_map}
    { }

    OldMaterial(Vec3 albedo, float roughness, Vec3 reflectance, const std::shared_ptr<Texture> normal_map = nullptr)
            : Material{new StandardStack(0, 0, 0)}
    {
        albedo_map      = std::make_shared<ValueTex3f>(albedo);
        roughness_map   = std::make_shared<ValueTex1f>(roughness);
        reflectance_map = std::make_shared<ValueTex3f>(reflectance);
        this->normal_map = normal_map;
    }
    
    OldMaterial(const OldMaterial& other) : albedo_map{other.albedo_map}, roughness_map{other.roughness_map}, reflectance_map{other.reflectance_map} {
        std::cout << "OldMaterial Material copy ctor";
    }
    
    ~OldMaterial() {
//        std::cout << "OldMaterial Material dtor" << std::endl;
    }

    BrdfStack* CreateBSDF(const SurfaceData& surface_data, Vec3& shading_normal) {

        Vec3 albedo = albedo_map->Evaluate(surface_data.uv);
        Vec3 roughness = roughness_map->Evaluate(surface_data.uv);
        Vec3 reflectance = reflectance_map->Evaluate(surface_data.uv);

        BrdfStack* stack = brdf_stack->copy();

        Brdf* const* brdf_array = stack->getBrdfArray();

        static_cast<Lambertian*>(brdf_array[0])->setAlbedo(albedo);
        static_cast<CookTorrance*>(brdf_array[1])->setRawRoughness(roughness.x);
        static_cast<CookTorrance*>(brdf_array[1])->setRawReflectance(reflectance);

        if (normal_map != nullptr) {

            //TODO: The normal_map is not normalized so check if a immediate normalize() change the output
            //TODO: Even if TangentToWorld return is normalized
            shading_normal = normal_map->Evaluate(surface_data.uv); // Normal already in Linear space [0, 1]
//            shading_normal.y = 1 - shading_normal.y;
            shading_normal = shading_normal * 2 - 1; // => [-1, 1]
            shading_normal.normalize();
            shading_normal = shading_normal.TangentToWorld(surface_data.normal, surface_data.tangent, surface_data.bitangent);

//            shading_normal = (shading_normal + 1) / 2; // => [-1, 1]
//            shading_normal = shading_normal.pow(2.2f);
        }

        return stack;
    }
    

    std::shared_ptr<Texture> GetAlbedo() const {
        return albedo_map;
    }

    std::shared_ptr<Texture> GetRoughness() const {
        return roughness_map;
    }

    std::shared_ptr<Texture> GetReflectance() const {
        return reflectance_map;
    }

    std::shared_ptr<Texture> GetNormal() const {
        return normal_map;
    }
    
private:
    void LoadLegacyMaterial(std::string albedo_path, std::string reflectance_path, std::string normal_path, Vec3 albedo_value, Vec3 reflectance_value);
};


class MetallicWorkflow : public Material {

    std::shared_ptr<Texture> albedo_map;
    std::shared_ptr<Texture> roughness_map;
    std::shared_ptr<Texture> metallic_map;
    std::shared_ptr<Texture> normal_map;
    bool packed_metal_roughness = false;

public:

    MetallicWorkflow() = default;
    
    MetallicWorkflow(aiMaterial* ai_material, const std::string& directory);

#ifdef USE_GLTF_LIB
    MetallicWorkflow(int index, tinygltf::Model& model, const std::string& directory);
#endif
    
    MetallicWorkflow(const std::shared_ptr<Texture>& albedo_map, const std::shared_ptr<Texture>& roughness_map, const std::shared_ptr<Texture>& metalness_map, const std::shared_ptr<Texture> normal_map = nullptr)
            :  Material{new StandardStack(0, 0, 0)}, albedo_map(albedo_map), roughness_map(roughness_map), metallic_map(metalness_map), normal_map{normal_map}
    { }

    MetallicWorkflow(Vec3 albedo, float roughness, Vec3 metalness, const std::shared_ptr<Texture> normal_map = nullptr)
            : Material{new StandardStack(0, 0, 0)}
    {
        albedo_map      = std::make_shared<ValueTex3f>(albedo);
        roughness_map   = std::make_shared<ValueTex1f>(roughness);
        metallic_map   = std::make_shared<ValueTex3f>(metalness);
        this->normal_map = normal_map;
    }
    
    MetallicWorkflow(const MetallicWorkflow& other) : albedo_map{other.albedo_map}, roughness_map{other.roughness_map}, metallic_map{other.metallic_map} {
        std::cout << "MetallicWorkflow Material copy ctor";
    }
    
    ~MetallicWorkflow() {
//        std::cout << "MetallicWorkflow Material dtor" << std::endl;
    }

    BrdfStack* CreateBSDF(const SurfaceData& surface_data, Vec3& shading_normal) {

        Vec3 base_color = albedo_map->Evaluate(surface_data.uv);
        float metallic;
        float roughness;
        if (packed_metal_roughness) {
            Vec3 packed = metallic_map->Evaluate(surface_data.uv);
            metallic = packed.z;
            roughness = packed.y;
        } else {
            metallic = metallic_map->Evaluate(surface_data.uv).x;
            roughness = roughness_map->Evaluate(surface_data.uv).x;
        }

        /*
         *  cdiff = lerp(base_color.rgb * (1 - 0.04), black, metallic)
         *  F0    = lerp(0.04, base_color.rgb              , metallic)
         */

        Vec3 albedo = Vec3::mix(base_color * (1 - 0.04), 0, metallic);
        Vec3 reflectance = Vec3::mix(0.04, base_color, metallic);

        BrdfStack* stack = brdf_stack->copy();

        Brdf* const* brdf_array = stack->getBrdfArray();

        static_cast<Lambertian*>(brdf_array[0])->setAlbedo(albedo);
        static_cast<CookTorrance*>(brdf_array[1])->setRoughness(roughness);
        static_cast<CookTorrance*>(brdf_array[1])->setRawReflectance(reflectance);

        if (normal_map != nullptr) {

            //TODO: The normal_map is not normalized so check if a immediate normalize() change the output
            //TODO: Even if TangentToWorld return is normalized
            shading_normal = normal_map->Evaluate(surface_data.uv); // Normal already in Linear space [0, 1]
//            shading_normal.y = 1 - shading_normal.y;
            shading_normal = shading_normal * 2 - 1; // => [-1, 1]
            shading_normal.normalize();
//            shading_normal = shading_normal.TangentToWorld(surface_data.normal, surface_data.tangent, surface_data.bitangent);
            shading_normal = shading_normal.TangentToWorld2(surface_data.normal) ;
//
//            shading_normal = (shading_normal + 1) / 2; // => [-1, 1]
//            shading_normal = shading_normal.pow(2.2f);
        }

        return stack;
    }

    std::shared_ptr<Texture> GetAlbedo() const {
        return albedo_map;
    }

    std::shared_ptr<Texture> GetRoughness() const {
        return roughness_map;
    }

    std::shared_ptr<Texture> GetMetallic() const {
        return metallic_map;
    }

    std::shared_ptr<Texture> GetNormal() const {
        return normal_map;
    }
    
    bool IsPacked()const {
        return packed_metal_roughness;
    }
    
private:
    void LoadMaterial(std::string albedo_path, std::string metallic_path, std::string normal_path,
                      Vec3 base_color_factor, float metallic_factor, float roughness_factor);
};



class LambertianMaterial : public Material {

    std::shared_ptr<ValueTex3f> albedo_map;

public:

    LambertianMaterial(Vec3 albedo) : Material{new SingleBrdf{new Lambertian{albedo}}}
    {
        albedo_map = std::make_shared<ValueTex3f>(albedo);
    }

    virtual BrdfStack* CreateBSDF(const SurfaceData& surface_data, Vec3& shading_normal) override {
        BrdfStack* stack = brdf_stack->copy();

        Brdf* const* brdf_array = stack->getBrdfArray();

        Vec3 albedo = albedo_map->value;

        static_cast<Lambertian*>(brdf_array[0])->setAlbedo(albedo);

        return stack;
    }

    std::shared_ptr<Texture> GetAlbedo() const {
        return albedo_map;
    }

};


class MirrorMaterial : public Material {
public:
    MirrorMaterial() : Material(new SingleBrdf(new Mirror())) {}
};

#endif //PATHTRACER_MATERIAL_H
