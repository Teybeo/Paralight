#include "render.h"

#include "tonemap.h"
#include "bvh.h"
#include "texture.h"
#include "material.h"

#include "macros.h"

Ray PrimaryRay(float x, float y, int width, int height, constant Options* options);
float3 Trace(Ray ray, global Node2* bvh_root, global Object3D* objects, VERTEX_DATA_ARGS, global Brdf* brdfs, constant Options* options, read_only image2d_t env_map, global char* texture_array, global TextureInfo* info_array, RNG_SEED_ARGS);
int FindNearestObject(const Ray ray, global Object3D* objects, VERTEX_GEOM_DATA_ARGS, float* nearest_dist, constant Options* options);

//kernel __attribute__((reqd_work_group_size(8, 4, 1)))
kernel __attribute__((work_group_size_hint(8, 4, 1)))
//kernel __attribute__((work_group_size_hint(8, 8, 1)))
//kernel
void render(global uchar4* framebuffer, global float4* accum_buffer, global Node2* bvh_root, global Object3D* objects, VERTEX_DATA_ARGS, global Brdf* brdfs, constant Options* options, read_only image2d_t env_map, global char* texture_array, global TextureInfo* info_array) {

    int x = get_global_id(0);
    int y = get_global_id(1);
    int w = get_global_size(0);
    int h = get_global_size(1);

    DEBUG_PIXEL(200, 200) {
//        printf("%d %d\n", get_local_size(0), get_local_size(1));
//        printf("%d \n", get_local_size(0));
//        DUMP_SIZE(Node)
//        DUMP_SIZE(Node2)
//        DUMP_SIZE(BoundingBox)
//        printf("group_size: %d, %d\n", get_local_size(0), get_local_size(1));
//        printf("num_groups: %d, %d\n", get_num_groups(0), get_num_groups(1));
    }

    // The PRNG needs a unique seed per pixel and per frame
    uint seed_x = x * (options->frame_number);
    uint seed_y = y * (options->frame_number);
//    uint seed_x = x;
//    uint seed_y = y;
    // Randomize the seeds to avoid locality artefacts
    seed_x *= getRandom(&seed_x, &seed_y);
    seed_y *= getRandom(&seed_x, &seed_y);

    // A touch of antialiasing (Center at 0.5f + random in [-0.5f, 0.5f])
    float jitter_x = 0.5f + (getRandom(&seed_x, &seed_y) - 0.5f);
    float jitter_y = 0.5f + (getRandom(&seed_x, &seed_y) - 0.5f);

    Ray ray = PrimaryRay(x + jitter_x, y + jitter_y, w, h, options);
//    Ray ray = PrimaryRay(x, y, w, h, options);

    float3 pixel = 0;

    for (int i = 0; i < options->sample_count; ++i) {
        pixel += Trace(ray, bvh_root, objects, VERTEX_DATA, brdfs, options, env_map, texture_array, info_array, &seed_x, &seed_y) * (1.f / options->sample_count);
    }

    accum_buffer[x + y * w] *= options->accum_clear_bit;

//    if (options->accum_clear_bit == 0)
//        accum_buffer[x + y * w] = 0;

    accum_buffer[x + y * w].xyz += pixel;

    pixel = accum_buffer[x + y * w].xyz / options->frame_number;

    if (options->use_tonemapping)
        tonemap(&pixel);

//    pixel = Sample(env_map, x / (float)w, y / (float)h);
//    pixel = Sample_Buffer(texture_array, info_array, 2, (float2)(x / (float)w, y / (float)h));
    pixel = powr(pixel, 1.f/2.2f);
    pixel *= 255;

    framebuffer[x + y * w].zyx = convert_uchar3_sat(pixel);
    framebuffer[x + y * w].w = 255;
}

float3 Trace(Ray ray, global Node2* bvh_root, global Object3D* objects, VERTEX_DATA_ARGS, global Brdf* brdfs, constant Options* options, read_only image2d_t env_map, global char* texture_array, global TextureInfo* info_array, RNG_SEED_ARGS) {

    float3 material = 1;
//    for (int i = 0; i < options->bounce_count + 1; i++) {
//    for (int i = 0; i < 1; i++) {
    for (int i = 0; i < 8; i++) {

        float dist = 999999.9f;

#ifdef USE_BVH
        int index = BVHFindNearestIntersection(ray, bvh_root, objects, VERTEX_GEOM_DATA, &dist);
#else
        int index = FindNearestObject(ray, objects, VERTEX_GEOM_DATA, &dist, options);
#endif
        // The current ray didn't hit any objects, return a "sky" color
        if (index == -1) {
            return 1.f * material * Sample_Envmap(env_map, ray.direction) * options->use_distant_env_lighting;
        }

        // The current ray hit an emissive object, return the emitted light
        if (objects[index].emission.x != -1) {
            return material * objects[index].emission * options->use_direct_lighting;
        }

        // Get information about the surface hit by the current ray
        float3 hit_pos = ray.origin + ray.direction * dist;

        float3 normal;
        {
        float2 uv;

        GetSurfaceData(&normal, &uv, hit_pos, ray, index, objects, VERTEX_DATA);
        short material_index = objects[index].material_index;

        float3 shading_normal = EvaluateNormalParameter(normal, brdfs[material_index].normal_map_index, normal, uv, texture_array, info_array);
        shading_normal = normalize(shading_normal);
//        shading_normal = normal;
//        return shading_normal;

        EvaluateMaterial(&ray.direction, &material, material_index, normal, shading_normal, uv, brdfs, options->brdf_bitfield, texture_array, info_array, RNG_SEED);
        }
//        return material;

        // Slightly displace the bounce point to avoid self-intersection
        ray.origin = hit_pos + 0.0001f * normal;

        if (all(material == 0))
            return 0;

        #if 1
        float test = max(material.x, max(material.y, material.z));
        if (test < 0.1f) {
            float rand = getRandom(RNG_SEED);
            if (rand > test)
                return (float3)(0); // Absorption
            else
                material *= 1.f / (test);
        }
        #endif
    }


    return (float3)(0);
}

void kernel Intersect(global int* hit_object_index, constant float2* coord, global Node2* bvh_root, global Object3D* objects, VERTEX_GEOM_DATA_ARGS, constant Options* options) {

    int w = get_global_size(0);
    int h = get_global_size(1);

    Ray ray = PrimaryRay(coord[0].x, coord[0].y, w, h, options);

    float dist = 9999999.f;
    int index;

#ifdef USE_BVH
    index = BVHFindNearestIntersection(ray, bvh_root, objects, VERTEX_GEOM_DATA, &dist);
#else
    index = FindNearestObject(ray, objects, VERTEX_GEOM_DATA, &dist, options);
#endif

//    DEBUG_PIXEL(coord[0].x, coord[0].y)
//        PRINT_I("index: ", index);

    *hit_object_index = index;
}

int FindNearestObject(const Ray ray, global Object3D* objects, VERTEX_GEOM_DATA_ARGS, float* nearest_dist, constant Options* options)
{
    int index = -1;
    float dist;

    int i;
    for (i = 0; i < options->object_count; i++)
    {
        if (IntersectObj(objects[i], VERTEX_GEOM_DATA, ray, &dist) && (dist < *nearest_dist)) {
            index = i;
            *nearest_dist = dist;
        }
    }
    return index;
}

/*
//TODO: The objects are expected to be sorted by type, spheres first, planes second
int FindNearestObjectOLD(const Ray ray, global Object3D* objects, VERTEX_GEOM_DATA_ARGS, float* nearest_dist, constant Options* options) {

    int index = -1;
    float dist;

    // Spheres
    int i;
    for (i = 0; i < options->sphere_count; i++) {
        if (IntersectSphere(objects[i], ray, &dist) && (dist < *nearest_dist)) {
            index = i;
            *nearest_dist = dist;
        }
    }

    // Planes
    for (; i < options->plane_count + options->sphere_count; i++) {
        if (IntersectPlane(objects[i], ray, &dist) && (dist < *nearest_dist)) {
            index = i;
            *nearest_dist = dist;
        }
    }

    // Trimesh
    for (; i < options->plane_count + options->sphere_count + options->triangle_count; i++) {
//        DEBUG_PIXEL(200, 200)
//            PRINT_I("i: ", i)
        if (IntersectTriangle(objects[i], VERTEX_GEOM_DATA, ray, &dist) && (dist < *nearest_dist)) {
            index = i;
            *nearest_dist = dist;
        }
    }

    return index;
}
*/
float3 mul(float3 vec, constant mat4x4* mat);

Ray PrimaryRay(float x, float y, int width, int height, constant Options* options) {
    Ray ray;
    ray.origin = options->origin;
    ray.direction.x =  (2 * (x + 0.5f) / width) - 1;
    ray.direction.y = -(2 * (y + 0.5f) / height) + 1;
    ray.direction.x *= (float)width / height * options->fov;
    ray.direction.y *= options->fov;
    ray.direction.z = -1;
    ray.direction = normalize(ray.direction);
    ray.direction = mul(ray.direction, &options->rotation);
    return ray;
}

float3 mul(float3 vec, constant mat4x4* mat)  {
    float3 new_vec = vec;
    new_vec.x = dot(mat->x, vec);
    new_vec.y = dot(mat->y, vec);
    new_vec.z = dot(mat->z, vec);
    return new_vec;
}

/**
 * Problem is: recursive version did:
 *   Sample an incoming direction from a pdf and its pdf value for a brdf
 *   Evaluate the brdf for this direction
 *   Immediately call Pathtrace to get light in this direction at isect point
 *   Add specular + diffuse if both were used
 * But in an iterative version, we can't just immediately ask what the light is in this direction !
 * No problem for 1 brdf but if you use 2 brdf (like lambertian + microfacet), then how do you sample both
 * with only a single direction ???
 */
/*
 * Pbrt seems to use RR for splitting and path termination
 * Here what the BSDF->sample_f() do:
 *   Randomly decide which brdf to sample // Aka splitting
 *   Sample the chosen brdf to get an incoming light direction
 *   then evaluate ALL the brdfs for this particular direction
 *   sum their values and their respective pdfs values for this direction
 */

/*
 * About the fresnel ratio computed by Microfacet brdf but used to weigh diffuse for
 * energy conservation:
 * Pbrt has a "Plastic" material which does Lambertian + Microfacet
 * The diffuse from the lambertian doesn't take into account the fresnel effect
 * But it also has a "Substrate" material using the FresnelBlend BSDF which
 * does diffuse + specular. The diffuse is weighted by the fresnel effect and is
 * approximated by the weird 28Rd/23....
 */
