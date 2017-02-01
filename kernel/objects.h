#ifndef _OBJECTS_H
#define _OBJECTS_H

typedef struct Object3D {
    float3 pos;           // Object3D
    float3 emission;      // Object3D
    float3 normal;        // Plane
    float radius;         // Sphere
    unsigned int A_index; // Triangle
    unsigned int B_index; // Triangle
    unsigned int C_index; // Triangle
    char type;            // Object3D
    short material_index;
    char has_uv;          // Triangle
//    char pad1[2];
} Object3D;

typedef struct Ray {
    float3 origin;
    float3 direction;
} Ray;

typedef struct FastRay {
    float3 origin;
    float3 direction_inv;
} FastRay;

#define VERTEX_DATA_ARGS const global float3* pos_array, const global float4* normal_array, const global float2* uv_array
#define VERTEX_DATA pos_array, normal_array, uv_array

#define VERTEX_GEOM_DATA_ARGS const global float3* pos_array, const global float4* normal_array
#define VERTEX_GEOM_DATA pos_array, normal_array

bool IntersectSphere(const Object3D obj, const Ray ray, float* t_near);
bool IntersectPlane(const Object3D obj, const Ray ray, float* t_near);
bool IntersectTriangle(const Object3D obj, VERTEX_GEOM_DATA_ARGS, const Ray ray, float* t_near);
bool IntersectObj(const Object3D obj, VERTEX_GEOM_DATA_ARGS, const Ray ray, float* t_near);

void GetSurfaceData(float3* normal_out, float2* uv_out, float3 hit_pos, const Ray ray, int index, global Object3D* objects, VERTEX_DATA_ARGS);
void GetTriangleData(const Object3D obj, const float3 hit_pos, float3* normal_out, float2* uv_out, VERTEX_DATA_ARGS);

#endif