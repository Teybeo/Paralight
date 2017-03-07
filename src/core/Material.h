#ifndef PATHTRACER_MATERIAL_H
#define PATHTRACER_MATERIAL_H

#include <memory>
#include <iostream>
#include <map>
#include <math/Vec3.h>
#include <material/BrdfStack.h>
#include <assimp/material.h>
#include <objects/SurfaceData.h>

#include "Texture.h"

class Material {

protected:
    std::unique_ptr<BrdfStack> brdf_stack;

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

class Standard : public Material {

    std::shared_ptr<Texture> albedo_map;
    std::shared_ptr<Texture> roughness_map;
    std::shared_ptr<Texture> reflectance_map;
    std::shared_ptr<Texture> normal_map;

public:

    Standard() = default;

    Standard(const std::shared_ptr<Texture>& albedo_map, const std::shared_ptr<Texture>& roughness_map, const std::shared_ptr<Texture>& reflectance_map, const std::shared_ptr<Texture> normal_map = nullptr)
            :  Material{new StandardStack(0, 0, 0)}, albedo_map(albedo_map), roughness_map(roughness_map), reflectance_map(reflectance_map), normal_map{normal_map}
    {
    }

    Standard(Vec3 albedo, float roughness, Vec3 reflectance, const std::shared_ptr<Texture> normal_map = nullptr)
            : Material{new StandardStack(0, 0, 0)}
    {
        albedo_map      = std::make_shared<ValueTex3f>(albedo);
        roughness_map   = std::make_shared<ValueTex1f>(roughness);
        reflectance_map = std::make_shared<ValueTex3f>(reflectance);
        this->normal_map = normal_map;
    }

    Standard(aiMaterial* ai_material, const std::string& directory)
            : Material{new StandardStack(0, 0, 0)}
    {
        aiString name;
        ai_material->Get(AI_MATKEY_NAME, name);
        cout << "Material: [" << name.C_Str() << "]" << endl;

        auto ImportTexture = [&] (std::shared_ptr<Texture>& texture_map, aiTextureType type, bool store_in_linear = true) {

            if (ai_material->GetTextureCount(type) > 0) {

                aiString ai_filename;
                std::string filename;

                if (ai_material->GetTexture(type, 0, &ai_filename) == AI_SUCCESS) {
                    filename = ai_filename.C_Str();
                    if (filename.find(':') == std::string::npos)
                        filename = directory + filename;

                    texture_map = std::make_shared<TextureUbyte>(filename, store_in_linear);
                }
            }
        };

        auto ImportValue = [&] (std::shared_ptr<Texture>& texture_map, const char* matkey, Vec3 default_value, bool store_in_linear = true) {

            aiColor3D ai_color;

            std::map<const char *, const char *> stupid_map {{"diffuse", "$clr.diffuse"}, {"specular", "$clr.specular"}};

            if (texture_map == nullptr) {
                if (ai_material->Get(stupid_map[matkey], 0, 0, ai_color) != AI_SUCCESS) {
                    std::cout << "No textures and no colors found for " << matkey << std::endl;
                }

                Vec3 color = Vec3{ai_color.r, ai_color.g, ai_color.b};
                if (color == 0 && default_value != 0)
                    color = default_value;

                texture_map = std::make_shared<ValueTex3f>(color);
                std::cout << color << " for " << matkey << endl;
            }
        };

        ImportTexture(albedo_map, aiTextureType_DIFFUSE);
        ImportTexture(reflectance_map, aiTextureType_SPECULAR);
//        ImportTexture(roughness_map, aiTextureType_SPECULAR);
        ImportTexture(normal_map, aiTextureType_HEIGHT, false);

        ImportValue(albedo_map, "diffuse", 0);
        ImportValue(reflectance_map, "specular", 0.04f);

        roughness_map = std::make_shared<ValueTex1f>(0.1f);
    }

    Standard(const Standard& other) : albedo_map{other.albedo_map}, roughness_map{other.roughness_map}, reflectance_map{other.reflectance_map} {
        std::cout << "Standard Material copy ctor";
    }
    ~Standard() {
//        std::cout << "Standard Material dtor" << std::endl;
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
