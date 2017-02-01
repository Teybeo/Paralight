#include "Triangle.h"

#include "TriMesh.h"
#include "BoundingBox.h"

bool Triangle::Intersect(const Ray& ray, float& dist_out) const {

    Vec3 A = trimesh_ptr->pos_array[A_index];
    Vec3 B = trimesh_ptr->pos_array[B_index];
    Vec3 C = trimesh_ptr->pos_array[C_index];

    Vec3 normal = (B - A).cross(C - A).normalize();
    Vec3 _origin = A;

    // Using the opposite of the plane normal
    float denominator = 1 / -normal.dot(ray.direction);

    // Single-sided intersection: don't intersect if normal is pointing away from the ray
//    if (denominator <= 0)
//        return false;

    dist_out = -normal.dot(_origin - ray.origin) * denominator;

    if (dist_out <= 0.000001f)
        return false;

    Vec3 hit_pos = ray.origin + ray.direction * dist_out;

    bool is_left_of_AB = (B - A).cross(hit_pos - A).dot(normal) > 0;
    bool is_left_of_BC = (C - B).cross(hit_pos - B).dot(normal) > 0;
    bool is_left_of_CA = (A - C).cross(hit_pos - C).dot(normal) > 0;

    bool is_inside = is_left_of_AB && is_left_of_BC && is_left_of_CA;

    return is_inside;
}


/*
 * Compute U, V, W, which are the barycentric coordinates of the point P in the triangle
 * U = Aire[ACP] / Aire[ABC]    U * B
 * V = Aire[ABP] / Aire[ABC]    V * C
 * W = Aire[BCP] / Aire[ABC]    W * A
 *
 * P = (U * B) + (V * C) + (W * A)
 */

SurfaceData Triangle::GetSurfaceData(Vec3 pos, Vec3 ray_direction) const {

    SurfaceData surface_data;

    Vec3 A = trimesh_ptr->pos_array[A_index];
    Vec3 B = trimesh_ptr->pos_array[B_index];
    Vec3 C = trimesh_ptr->pos_array[C_index];

    Vec3 AB = B - A;
    Vec3 BC = C - B;
    Vec3 CA = A - C;

    float ABC_area = AB.cross(-CA).length() / 2;
    float ABP_area = AB.cross(pos - A).length() / 2;
    float BCP_area = BC.cross(pos - B).length() / 2;
    float CAP_area = CA.cross(pos - C).length() / 2;

    float U = CAP_area / ABC_area; // B
    float V = ABP_area / ABC_area; // C
    float W = BCP_area / ABC_area; // A

//    if ((U + V + W) - 1 > 0.001) {
//        cout << "U+V+W = " << U + V + W << " (> 1 !)   U: " << U << " V: " << V << " W: " << W << endl;
//    }

    if (trimesh_ptr->uv_array.empty())
        surface_data.uv = 0;
    else
        surface_data.uv =  (U * trimesh_ptr->uv_array[B_index]) + (V * trimesh_ptr->uv_array[C_index]) + (W * trimesh_ptr->uv_array[A_index]);

    surface_data.normal = (U * trimesh_ptr->normal_array[B_index]) + (V * trimesh_ptr->normal_array[C_index]) + (W * trimesh_ptr->normal_array[A_index]);

    //FIXME Take a look at the handling of models without tangents
    if (trimesh_ptr->tangent_array.size() > 0)
        surface_data.tangent = (U * trimesh_ptr->tangent_array[B_index]) + (V * trimesh_ptr->tangent_array[C_index]) + (W * trimesh_ptr->tangent_array[A_index]);

    if (trimesh_ptr->bitangent_array.size() > 0)
        surface_data.bitangent = (U * trimesh_ptr->bitangent_array[B_index]) + (V * trimesh_ptr->bitangent_array[C_index]) + (W * trimesh_ptr->bitangent_array[A_index]);

    surface_data.normal.normalize();
    surface_data.tangent.normalize();
    surface_data.bitangent.normalize();

    return surface_data;
}

BoundingBox Triangle::ComputeBBox() const {
    BoundingBox bbox;

    Vec3 A = trimesh_ptr->pos_array[A_index];
    Vec3 B = trimesh_ptr->pos_array[B_index];
    Vec3 C = trimesh_ptr->pos_array[C_index];

    return bbox.ExtendsBy(A).ExtendsBy(B).ExtendsBy(C);
}

Vec3 Triangle::GetCenter() const {

    Vec3 A = trimesh_ptr->pos_array[A_index];
    Vec3 B = trimesh_ptr->pos_array[B_index];
    Vec3 C = trimesh_ptr->pos_array[C_index];

    Vec3 center = A + (((B-A) + (C-A)) / 3.f);

    return center;
}

