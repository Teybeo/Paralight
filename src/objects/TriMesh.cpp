#include "TriMesh.h"

#include "Triangle.h"

#include "assimp/scene.h"
#include "assimp/Importer.hpp"
#include "assimp/postprocess.h"

#include <chrono>
#include "app/Chronometer.h"

using std::string;
using std::vector;
using std::unique_ptr;

std::atomic_int TriMesh::triangle_test_count;
std::atomic_int TriMesh::triangle_hit_count;

float FindSphereBoundRadius(const vector<Vec3>& pos_array);

// Assimp separate the indices of each triangle in their own Face structure
// We flatten it to an array of unsigned int for OpenGL consumption
vector<unsigned int> CreateFlattenIndexArray(const aiFace* face_array, const unsigned int face_count) ;

TriMesh::TriMesh(const string& filename, string directory) {

    Assimp::Importer Importer;
    cout << "Loading [" + filename + "] mesh..." << endl;

    unsigned int flags = 0
                         | aiProcess_Triangulate
                         | aiProcess_FindInvalidData
                         | aiProcess_GenSmoothNormals
                         | aiProcess_FlipUVs
                         | aiProcess_CalcTangentSpace
                         | aiProcess_RemoveRedundantMaterials
                         | aiProcess_FindInstances
                         | aiProcess_OptimizeMeshes
                         | aiProcess_OptimizeGraph
                         | aiProcess_JoinIdenticalVertices
                         | aiProcess_PreTransformVertices

    ;
    
    // Use the file directory to search for materials if no directory was passed in argument
    if (directory.empty()) {
        directory = filename.substr(0, filename.find_last_of('/')) + "/";
    }
    
    string ext = filename.substr(filename.find_last_of('.'));
    
    
    Chronometer chrono;

    const aiScene* pScene = Importer.ReadFile(filename.c_str(), flags);
    if (pScene == nullptr) {
        cout << "Error reading mesh file" << endl;
        cout << Importer.GetErrorString() << endl;
        throw std::bad_exception();
    }

    cout << "File reading: " << chrono.GetSeconds() << " s" << endl;

    
    chrono.Restart();

    ImportAssimpMesh(pScene, directory, ext);

    cout << "Import: " << chrono.GetSeconds() << " s" << endl;
    
    
    chrono.Restart();

    float radius = FindSphereBoundRadius(pos_array);

    cout << "Bounds scan: " << chrono.GetSeconds() << " s" << endl;


    sphere_bounds = Sphere {0, 0, 0, radius};
}

void TriMesh::ImportAssimpMesh(const aiScene *ai_scene, std::string directory, std::string ext) {

    unsigned int vertex_total = 0;

    for (size_t i = 0; i < ai_scene->mNumMeshes; ++i) {

        const aiMesh* ai_mesh = ai_scene->mMeshes[i];

        int vertex_count = ai_mesh->mNumVertices;

        // Looks unsafe but it works because Vec3 and aiVector3t both are simple 3-float vec classes
        Vec3* vertices   = reinterpret_cast<Vec3*>(ai_mesh->mVertices);
        Vec3* normals    = reinterpret_cast<Vec3*>(ai_mesh->mNormals);
        Vec3* tex_coords = reinterpret_cast<Vec3*>(ai_mesh->mTextureCoords[0]);
        Vec3* tangents   = reinterpret_cast<Vec3*>(ai_mesh->mTangents);
        Vec3* bitangents = reinterpret_cast<Vec3*>(ai_mesh->mBitangents);

//        std::copy_n(vertices, vertex_count, pos_array.begin() + vertex_total);
//        std::copy_n(normals, vertex_count, normal_array.begin() + vertex_total);
//        std::copy_n(tex_coords, vertex_count, uv_array.begin() + vertex_total);
//        std::copy_n(tangents, vertex_count, tangent_array.begin() + vertex_total);
//        std::copy_n(bitangents, vertex_count, bitangent_array.begin() + vertex_total);

        pos_array.insert(pos_array.end(), vertices, vertices + ai_mesh->mNumVertices);
        normal_array.insert(normal_array.end(), normals, normals + ai_mesh->mNumVertices);
        if (ai_mesh->HasTextureCoords(0))
            uv_array.insert(uv_array.end(), tex_coords, tex_coords + ai_mesh->mNumVertices);
        if (ai_mesh->HasTangentsAndBitangents()) {
            tangent_array.insert(tangent_array.end(), tangents, tangents + ai_mesh->mNumVertices);
            bitangent_array.insert(bitangent_array.end(), bitangents, bitangents + ai_mesh->mNumVertices);
        }

        vector<unsigned int> mesh_index_array = CreateFlattenIndexArray(ai_mesh->mFaces, ai_mesh->mNumFaces);
//        std::copy_n(index_array.begin(), mesh_index_array.size(), mesh_index_array.begin() + vertex_total);

        for (auto& index : mesh_index_array) {
            index += vertex_total;
        }

        index_array.insert(index_array.end(), mesh_index_array.begin(), mesh_index_array.end());

        for (size_t tri = 0; tri < mesh_index_array.size(); tri += 3) {
            triangle_to_material.push_back(ai_mesh->mMaterialIndex);
        }

        vertex_total += vertex_count;
    }

    vertex_count = vertex_total;

    for (size_t tri = 0; tri < index_array.size(); tri += 3) {
        triangles.emplace_back(Triangle {&index_array[tri], this});
    }

    materials = vector<unique_ptr<Material>>(ai_scene->mNumMaterials);

    for (size_t i = 0; i < materials.size(); ++i) {
        if (ext == ".gltf")
            materials[i] = unique_ptr<Material>(new MetallicWorkflow(ai_scene->mMaterials[i], directory));
        else
            materials[i] = unique_ptr<Material>(new OldMaterial(ai_scene->mMaterials[i], directory));
    }

}

/**
 * Assimp separate the indices of each triangle in their own Face structure
 * We flatten it to an array of unsigned int for simpler triangle processing
 */
vector<unsigned int> CreateFlattenIndexArray(const aiFace* face_array, const unsigned int face_count) {

    vector<unsigned int> index_array = vector<unsigned int>(face_count * 3);

    auto index_ptr = index_array.begin();
    for (unsigned int f = 0; f < face_count; ++f) {
        std::copy_n(face_array[f].mIndices, 3, index_ptr);
        index_ptr += 3;
    }

    return index_array;
}

float FindSphereBoundRadius(const vector<Vec3>& pos_array) {

    float radius_squared = 0;

    for (const auto& position : pos_array)
    {
        if (position.lengthSquared() > radius_squared)
            radius_squared = position.lengthSquared();
    }


    if (radius_squared > 0)
        radius_squared = sqrtf(radius_squared);

    return radius_squared;
}

