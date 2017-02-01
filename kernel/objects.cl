#include "objects.h"

#include "macros.h"
#include "texture.h"

bool IntersectSphere(const Object3D obj, const Ray ray, float* dist_out) {

    float3 origin_to_center = obj.pos - ray.origin;

    float proj = dot(origin_to_center, ray.direction);
//    if (proj < 0)
//        return false;

    float dxd = dot(origin_to_center, origin_to_center) - (proj * proj);
    if (dxd > (obj.radius))
        return false;

    float half_inner_dist = sqrt(obj.radius - dxd);

//    float t0 = proj - half_inner_dist;
//    float t1 = proj + half_inner_dist;
//
//    if (t0 < 0 || t1 < 0)
//        return false;
//    *dist_out = t0;
//
//    return true;
    float t = min(proj - half_inner_dist, proj + half_inner_dist);

    return ((*dist_out = t) > 0);
}

bool IntersectSphere1(const Object3D obj, const Ray ray, float* dist_out) {

    float3 origin_to_center = obj.pos - ray.origin;
    float epsilon = 0.01f;
    float proj = dot(origin_to_center, ray.direction);
    float disc = proj*proj - dot(origin_to_center, origin_to_center) + obj.radius; // discriminant
    if (disc < 0)
        return false;
    else
        disc = sqrt(disc);
    return (*dist_out = proj - disc) > epsilon ?
           true
                                    :
           ((*dist_out = proj + disc) >epsilon ?
            true
                                               :
            false);
}

bool IntersectPlane(const Object3D obj, const Ray ray, float* dist_out) {

    // Using the opposite of the plane normal
    float denominator = (dot(-obj.normal, ray.direction));

    // In Single-sided mode: don't intersect if normal isn't facing ray
//    if (denominator <= 0)
//        return false;

    *dist_out = (dot(-obj.normal, obj.pos - ray.origin)) / denominator;

    // Infinite plane
//    return *dist_out > 0.00001f;

    // Or disk defined by the dist_to_plane_origin (radius)
    if (*dist_out <= 0.00001f)
        return false;

    float dist_to_plane_origin = length((ray.origin + ray.direction * *dist_out) - obj.pos);

    return dist_to_plane_origin <= 4;
}

#define _origin A
bool IntersectTriangle(const Object3D obj, VERTEX_GEOM_DATA_ARGS, const Ray ray, float* dist_out) {

    float3 A = pos_array[obj.A_index];
    float3 B = pos_array[obj.B_index];
    float3 C = pos_array[obj.C_index];

    float3 normal = normalize(cross(B - A, C - A));
//    float3 _origin = A;
    // Using the opposite of the plane normal
    float denominator = 1.f / dot(-normal, ray.direction);

    // For single-sided intersection: don't intersect if normal is pointing away from the ray
//    if (denominator <= 0)
//        return false;

    *dist_out = dot(-normal, _origin - ray.origin) * denominator;

    if (*dist_out <= 0.000001f)
        return false;

    float3 hit_pos = ray.origin + ray.direction * *dist_out;

    bool is_left_of_AB = dot(cross(B - A, hit_pos - A), normal) > 0;
    bool is_left_of_BC = dot(cross(C - B, hit_pos - B), normal) > 0;
    bool is_left_of_CA = dot(cross(A - C, hit_pos - C), normal) > 0;

    return is_left_of_AB && is_left_of_BC && is_left_of_CA;
}

void GetTriangleData(const Object3D obj, const float3 hit_pos, float3* normal_out, float2* uv_out, VERTEX_DATA_ARGS) {

    float3 A = pos_array[obj.A_index];
    float3 B = pos_array[obj.B_index];
    float3 C = pos_array[obj.C_index];

    float3 AB = B - A;
    float3 BC = C - B;
    float3 CA = A - C;

    float ABC_area = length(cross(AB, -CA)        ) / 2;
    float ABP_area = length(cross(AB, hit_pos - A)) / 2;
    float BCP_area = length(cross(BC, hit_pos - B)) / 2;
    float CAP_area = length(cross(CA, hit_pos - C)) / 2;

    float U = CAP_area / ABC_area; // B
    float V = ABP_area / ABC_area; // C
    float W = BCP_area / ABC_area; // A

    *normal_out = ((U * normal_array[obj.B_index]) + (V * normal_array[obj.C_index]) + (W * normal_array[obj.A_index])).xyz;
    *normal_out = normalize(*normal_out);

    if (obj.has_uv) {
        *uv_out = ((U * uv_array[obj.B_index]) + (V * uv_array[obj.C_index]) + (W * uv_array[obj.A_index]));
    }
}

bool IntersectObj(const Object3D obj, VERTEX_GEOM_DATA_ARGS, const Ray ray, float* t_near) {

    switch (obj.type) {
    case 1:
        return IntersectSphere(obj, ray, t_near);
    case 2:
        return IntersectPlane(obj, ray, t_near);
    case 3:
        return IntersectTriangle(obj, VERTEX_GEOM_DATA, ray, t_near);
    default:
        return false;
    }
}

void GetSurfaceData(float3* normal_out, float2* uv_out, float3 hit_pos, const Ray ray, int index, global Object3D* objects, VERTEX_DATA_ARGS) {

    switch (objects[index].type) {
    case 1:
        *uv_out = SphericalToCartesian(*normal_out);
        *normal_out = normalize(hit_pos - objects[index].pos);
        break;
    case 3:
        GetTriangleData(objects[index], hit_pos, normal_out, uv_out, VERTEX_DATA);
        break;
    case 2:
        *uv_out = 0;
        *normal_out = objects[index].normal;
        // Double-side normal for planes
        *normal_out *= sign(dot(-(*normal_out), ray.direction));
        break;
    default:
        *uv_out = 0;
        *normal_out = objects[index].normal;
        break;
    }
}