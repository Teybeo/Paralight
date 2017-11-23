#include "Material.h"

#include <assimp/material.h>

#define AI_MATKEY_GLTF_PBRMETALLICROUGHNESS_METALLICROUGHNESS_TEXTURE "$tex.file", aiTextureType_UNKNOWN
#define AI_MATKEY_GLTF_PBRMETALLICROUGHNESS_METALLIC_FACTOR "$mat.gltf.pbrMetallicRoughness.metallicFactor", 0, 0
#define AI_MATKEY_GLTF_PBRMETALLICROUGHNESS_ROUGHNESS_FACTOR "$mat.gltf.pbrMetallicRoughness.roughnessFactor", 0, 0

using std::string;

void MakePathAbsolute(string& path, const string& directory);

string GetPathAssimp(aiTextureType tex_type, aiMaterial* ai_material);
string GetPathAssimp(string key, aiTextureType type, aiMaterial* ai_material);

float GetNumberFactorAssimp(string key, unsigned int type, unsigned int slot, float default_value, aiMaterial* ai_material);
Vec3 GetVecFactorAssimp(string key, unsigned int type, unsigned int slot, Vec3 default_value, aiMaterial* ai_material);

OldMaterial::OldMaterial(aiMaterial* ai_material, const std::string& directory)
    : Material{new StandardStack(0, 0, 0)}
{
    aiString name;
    ai_material->Get(AI_MATKEY_NAME, name);
    cout << "Material: [" << name.C_Str() << "]" << endl;
    
    auto albedo_path = GetPathAssimp(aiTextureType_DIFFUSE, ai_material);
    auto reflectance_path = GetPathAssimp(aiTextureType_SPECULAR, ai_material);
    auto normal_path = GetPathAssimp(aiTextureType_HEIGHT, ai_material);
    
    Vec3 albedo_value = GetVecFactorAssimp(AI_MATKEY_COLOR_DIFFUSE, 1, ai_material);
    Vec3 reflectance_value = GetVecFactorAssimp(AI_MATKEY_COLOR_SPECULAR, 0.04, ai_material);
    
    MakePathAbsolute(albedo_path, directory);
    MakePathAbsolute(reflectance_path, directory);
    MakePathAbsolute(normal_path, directory);
    
    LoadLegacyMaterial(albedo_path, reflectance_path, normal_path, albedo_value, reflectance_value);
    
    roughness_map = std::make_shared<ValueTex1f>(0.1f);
    
}

void OldMaterial::LoadLegacyMaterial(string albedo_path, string reflectance_path, string normal_path, Vec3 albedo_value, Vec3 reflectance_value) {
    
    if (albedo_path.empty() == false)
        albedo_map = std::make_shared<TextureUbyte>(albedo_path);
    else
        albedo_map = std::make_shared<ValueTex3f>(albedo_value);
    
    if (reflectance_path.empty() == false)
        reflectance_map = std::make_shared<TextureUbyte>(reflectance_path);
    else
        reflectance_map = std::make_shared<ValueTex3f>(reflectance_value);
    
    if (normal_path.empty() == false) {
        normal_map = std::make_shared<TextureUbyte>(normal_path, false);
    }
}

MetallicWorkflow::MetallicWorkflow(aiMaterial* ai_material, const std::string& directory)
    : Material{new StandardStack(0, 0, 0)}
{
    aiString name;
    ai_material->Get(AI_MATKEY_NAME, name);
    cout << "Material: [" << name.C_Str() << "]" << endl;
    
    packed_metal_roughness = true;
    
    auto albedo_path = GetPathAssimp(aiTextureType_DIFFUSE, ai_material);
    auto metallic_path = GetPathAssimp(AI_MATKEY_GLTF_PBRMETALLICROUGHNESS_METALLICROUGHNESS_TEXTURE, ai_material);
    auto normal_path = GetPathAssimp(aiTextureType_NORMALS, ai_material);
    
    Vec3 base_color_factor = GetVecFactorAssimp(AI_MATKEY_COLOR_DIFFUSE, 1, ai_material);
    float metallic_factor = GetNumberFactorAssimp(AI_MATKEY_GLTF_PBRMETALLICROUGHNESS_METALLIC_FACTOR, 1, ai_material);
    float roughness_factor = GetNumberFactorAssimp(AI_MATKEY_GLTF_PBRMETALLICROUGHNESS_ROUGHNESS_FACTOR, 1, ai_material);
    
    MakePathAbsolute(albedo_path, directory);
    MakePathAbsolute(metallic_path, directory);
    MakePathAbsolute(normal_path, directory);
    
    LoadMaterial(albedo_path, metallic_path, normal_path, base_color_factor, metallic_factor, roughness_factor);
    
}

void MetallicWorkflow::LoadMaterial(string albedo_path, string metallic_path, string normal_path, Vec3 base_color_factor, float metallic_factor, float roughness_factor) {
    
    if (albedo_path.empty() == false) {
        
        albedo_map = std::make_shared<TextureUbyte>(albedo_path);
        if (base_color_factor != 1) {
            cout << "Multiplication of base_color texture not implemented" << endl;
        }
    }
    else {
        albedo_map = std::make_shared<ValueTex3f>(base_color_factor);
    }
    
    if (metallic_path.empty() == false) {
        
        metallic_map = std::make_shared<TextureUbyte>(metallic_path);
        if (metallic_factor != 1) {
            cout << "Multiplication of metallic texture not implemented" << endl;
        }
        if (roughness_factor != 1) {
            cout << "Multiplication of roughness texture not implemented" << endl;
        }
    }
    else {
        Vec3 metallic_roughness {0, roughness_factor, metallic_factor};
        metallic_map = std::make_shared<ValueTex3f>(metallic_roughness);
    }
    
    if (normal_path.empty() == false) {
        normal_map = std::make_shared<TextureUbyte>(normal_path, false);
    }
}

Vec3 GetVecFactorAssimp(string key, unsigned int type, unsigned int slot, Vec3 default_value, aiMaterial* ai_material) {
    
    aiColor4D ai_color;
    
    if (ai_material->Get(key.c_str(), type, slot, ai_color) == AI_SUCCESS)
        return Vec3{ai_color.r, ai_color.g, ai_color.b};
    else
        return default_value;
}

float GetNumberFactorAssimp(string key, unsigned int type, unsigned int slot, float default_value, aiMaterial* ai_material) {
    
    float value;
    
    if (ai_material->Get(key.c_str(), type, slot, value) == AI_SUCCESS)
        return value;
    else
        return default_value;
}

string GetPathAssimp(aiTextureType tex_type, aiMaterial* ai_material) {
    
    if (ai_material->GetTextureCount(tex_type) > 0) {
        
        aiString ai_filename;
        string filename;
        
        if (ai_material->GetTexture(tex_type, 0, &ai_filename) == AI_SUCCESS) {
            return ai_filename.C_Str();
        }
    }
    
    return "";
}

string GetPathAssimp(string key, aiTextureType type, aiMaterial* ai_material) {
    
    aiString ai_filename;
    
    if (ai_material->Get(key.c_str(), type, 0, ai_filename) == AI_SUCCESS)
        return ai_filename.C_Str();
    else
        return "";
}

#ifdef USE_GLTF_LIB

#include <tinygltf/tiny_gltf.h>

string GetPathGLTF(const string tex_name, tinygltf::ParameterMap &map, tinygltf::Model &model);
Vec3 GetVecFactorGLTF(string factor_name, Vec3 _default, tinygltf::ParameterMap &map);
float GetNumberFactorGLTF(string factor_name, float _default, tinygltf::ParameterMap &map);

MetallicWorkflow::MetallicWorkflow(int index, tinygltf::Model &model, const std::string &directory)
    : Material{new StandardStack(0, 0, 0)}
{
    
    auto& material = model.materials[index];
    
    cout << material.name << endl;
    
    auto& pbr_parameter_map = material.values;
    auto& parameter_map = material.additionalValues;
    
    for (const auto &parameter : pbr_parameter_map) {
        cout << parameter.first << endl;
    }
    
    packed_metal_roughness = true;
    
    auto albedo_path = GetPathGLTF("baseColorTexture", pbr_parameter_map, model);
    auto metallic_path = GetPathGLTF("metallicRoughnessTexture", pbr_parameter_map, model);
    auto normal_path = GetPathGLTF("normalTexture", parameter_map, model);
    
    Vec3 base_color_factor = GetVecFactorGLTF("baseColorFactor", 1, pbr_parameter_map);
    float metallic_factor = GetNumberFactorGLTF("metallicFactor", 1, pbr_parameter_map);
    float roughness_factor = GetNumberFactorGLTF("roughnessFactor", 1, pbr_parameter_map);

    MakePathAbsolute(albedo_path, directory);
    MakePathAbsolute(metallic_path, directory);
    MakePathAbsolute(normal_path, directory);
    
    LoadMaterial(albedo_path, metallic_path, normal_path, base_color_factor, metallic_factor, roughness_factor);
}

string GetPathGLTF(const string tex_name, tinygltf::ParameterMap &map, tinygltf::Model &model) {
    
    auto iter = map.find(tex_name);
    if (iter != map.end()) {
        int tex_index = static_cast<int>(iter->second.json_double_value["index"]);
        int source_index = model.textures[tex_index].source;
        
        return model.images[source_index].uri;
    }
    
    return "";
};

Vec3 GetVecFactorGLTF(string factor_name, Vec3 _default, tinygltf::ParameterMap &map) {
    
    auto iter = map.find(factor_name);
    
    if (iter != map.end()) {
        const auto& data = iter->second.number_array;
        Vec3 vec;
        vec.x = (float) data[0];
        vec.y = (float) data[1];
        vec.z = (float) data[2];
        return vec;
    }
    else
        return _default;
};

float GetNumberFactorGLTF(string factor_name, float _default, tinygltf::ParameterMap &map) {
    
    auto iter = map.find(factor_name);
    
    if (iter != map.end()) {
        const auto& data = iter->second.number_array;
        return float(data[0]);
    }
    else
        return _default;
}

#endif

void MakePathAbsolute(string& path, const string& directory) {
    
    if (path.empty())
        return;
    
    if (path.find(':') == std::string::npos)
        path = directory + path;
}