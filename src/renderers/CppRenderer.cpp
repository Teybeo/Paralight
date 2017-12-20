#include "CppRenderer.h"

#include <chrono>
#include <objects/Triangle.h>

#include "objects/Plane.h"
#include "objects/Sphere.h"
#include "BaseRenderer.h"

using std::cout;
using std::endl;

#define sRGB_LUT_LENGTH (2 << 15)
unsigned char sRGB_table[sRGB_LUT_LENGTH];

void initializeSRGBTable();

CppRenderer::CppRenderer(Scene* scene, SDL_Window* window, CameraControls* const controls, Options* options)
        : BaseRenderer{scene, window, controls, options} {

#ifdef USE_TRIGO_LOOKUP
    initializeCosTable();
    initializeSinTable();
    initializeAcosTable();
    initializeTanTable();
#endif

    initializeSRGBTable();
    accum_texture = new Vec3[1920 * 1080];

    cout << "C++ Renderer ready" << endl;
}

CppRenderer::~CppRenderer() {
    cout << "C++ Renderer dtor called" << endl;

    delete[] accum_texture;
};

void CppRenderer::Render() {

    BaseRenderer::Render();

    float ratio = (float) film_width / film_height;
    float fov_factor = tanf((float) ((fov / 2.f) * (M_PI / 180)));

    bool debug_pixel = false;
    
    #if defined(USE_OPENMP) && not defined(DEBUG_BUILD)
        #pragma message "OpenMP Enabled for C++"
    #pragma omp parallel for schedule(dynamic, 1)
    #else
        #pragma message "OpenMP Disabled for C++"
    #endif
    for (int y = 0; y < film_height; ++y) {

        for (int x = 0; x < film_width; ++x) {

            Ray ray{camera_controls->GetPosition(), x, y, film_width, film_height, ratio, fov_factor};
            ray.direction = camera_controls->GetRotation() * ray.direction;
            Vec3 pixel;
#ifndef INNER_LOOP
            for (int i = 0; i < options->sample_count; ++i) {
                pixel += Raytrace(ray, debug_pixel) * (1.f / options->sample_count);
//                pixel += Raytrace_Recursive(ray) * (1.f / options->sample_count);
            }
#else
            pixel += Raytrace(ray);
#endif
            accum_texture[y * film_width + x] *= CLEAR_ACCUM_BIT;
            accum_texture[y * film_width + x] += pixel;

            pixel = accum_texture[y * film_width + x] / frame_number;

//            pixel = env_map->Sample(float(x) / width, float(y) / height);

            pixel = pixel.clamp(0, 1);
//            pixel = linear_to_sRGB(pixel); // CL uses 1/2.2
            pixel = pixel.pow(1.f / 2.2f);
            pixel *= 255;

//            pixels[y * width + x] = SDL_MapRGB(surface->format, (Uint8) pixel.x, (Uint8) pixel.y, (Uint8) pixel.z);
//
            pixels[y * film_width + x] = 0;
            pixels[y * film_width + x] |= (Uint32) (((Uint8)pixel.x << 16) | ((Uint8)pixel.y << 8) | ((Uint8)pixel.z << 0));
        }
    }

}

Vec3 CppRenderer::Raytrace(Ray ray, bool debug_pixel) {

    Vec3 material {1};

//    for (int i = 0; i < 4; ++i) {
    for (int i = 0; i < 8; ++i) {
//    for (int i = 0; i < options->bounce_cout + 1; ++i) {

        float dist = 99999999.f;
        Object3D* hit_object = nullptr;

//        FindNearestObject(ray, dist, hit_object, false);
//        scene->bvh.FindNearestIntersection(ray, dist, hit_object);
//        if (options->debug)
            scene->bvh2->FindNearestIntersectionOpti(ray, dist, hit_object);
//        else
//            scene->bvh2->FindNearestIntersection(ray, dist, hit_object);

//        bvh.DebugIntersection(ray, dist, hit_object, options->depth_target);

        // The current ray didn't hit any objects, return a "sky" color
        if (hit_object == nullptr) {

            if (options->use_distant_env_lighting) {
//                return material * Vec3{0.18, 0.18, 0.18};
//                return material * options->background_color;
                return material * scene->env_map->SampleEnvmap(ray.direction);
            }
            else
                return {0, 0, 0};
        }

        // The current ray hit an emissive material, return its emitted light
        if (hit_object->getEmissionIntensity() != -1) {
            return material * hit_object->getEmission() * options->use_emissive_lighting;
        }

        // Optim if no shading
        if (options->brdf_bitfield == 0)
            return 0;


        // Get information about the surface hit by the current ray
        Vec3 pos = ray.origin + ray.direction * dist;

        SurfaceData surface_data = hit_object->GetSurfaceData(pos, ray.direction);

        if (options->depth_target) {
            return ((hit_object->GetCenter() + scene->debug_scale) / (scene->debug_scale * 2));
        }
//        return ((hit_object->GetCenter() + 7) / 14) * cos(normal.dot(ray.direction));

        Vec3 outgoing_dir = -ray.direction;
        float pdf = 1;

        Vec3 shading_normal = surface_data.normal;

        BrdfStack* stack = hit_object->material->CreateBSDF(surface_data, shading_normal);
        Vec3 f = stack->Sample_f(outgoing_dir, shading_normal, ray.direction, pdf, options->brdf_bitfield);
        delete stack;

//        if (options->debug)
//            return normal;
//        else
//            return shading_normal;

        if (f == 0) {
            return 0;
        }

        float cos_factor = shading_normal.dot(ray.direction) * ((surface_data.normal.dot(ray.direction) > 0) || debug);

        material *= (f * cos_factor) / pdf;

        // Slightly displace the bounce point to avoid self-intersection
        ray.origin = pos + 0.0001f * surface_data.normal;

        #if 1
        float SEUIL = material.max();
        if (SEUIL < 0.2f ){
            float rand = Random::GetInstance().GetUniformRandom();
            if (rand > SEUIL)
                break; // Absorption
            else
                material *= 1 / (SEUIL);
        }
        #endif
    }

    return 0;
}

//region Recursive Path-Tracing

Vec3 CppRenderer::Raytrace_Recursive(Ray ray, const int bounce_depth) {

    float dist = 99999999.f;
    Object3D* hit_object = nullptr;

    FindNearestObject(ray, dist, hit_object, false);

    // No intersection
    if (hit_object == nullptr) {
        if (options->use_distant_env_lighting)
//            return {0.18, 0.18, 0.36};
//            return env_map->Sample_Cubemap(ray.direction);
            return scene->env_map->SampleEnvmap(ray.direction);
        else
            return {0, 0, 0};
    }

    // We hit an emissive material, stop the tracing and propagate the energy up the recursion stack
    if (hit_object->getEmissionIntensity() != -1) {
        return hit_object->getEmission() * options->use_emissive_lighting;
    }

    // Get information about the surface hit
    Vec3 pos = ray.origin + ray.direction * dist;
    SurfaceData surface_data;
//    Vec3 normal, uv, tangent, bitangent;
    surface_data = hit_object->GetSurfaceData(pos, ray.direction);

    //region [DEACTIVATED] Explicit light sampling
#if 0
    for (auto& light: lights) {

        Vec3 hit_to_light;
        float light_dist;
        Vec3 intensity;
        light->getLightData(pos, hit_to_light, light_dist, intensity);

        Ray shadow_ray {pos, hit_to_light};

        bool visible = !FindNearestObject(shadow_ray, light_dist, hit_object, true);
        direct_light += visible * intensity * std::max(normal.dot(hit_to_light), 0.f);
    }
#endif
    //endregion

    if (bounce_depth != options->bounce_cout) {

        Vec3 outgoing_dir = -ray.direction;
        Vec3 incoming_dir;

        Vec3 specular{0,0,0};
        Vec3 indirect_diffuse;
//        float ratio = 1;
        float pdf;

        if (options->brdf_bitfield & LAMBERTIAN) {

            Brdf* brdf = hit_object->brdf;
//            incoming_dir = brdf->Sample_direction(outgoing_dir, pdf, normal);
//            Vec3 f = brdf->Evaluate_f(outgoing_dir, incoming_dir, normal);

            Vec3 f = brdf->Sample_f(outgoing_dir, incoming_dir, pdf, surface_data.normal);
            Vec3 Li = Raytrace_Recursive(Ray{pos, incoming_dir}, bounce_depth + 1);
            float cos_factor = surface_data.normal.dot(incoming_dir);
            // Don't apply N.L light attenuation for perfect mirror
//            if (typeid(*brdf) == typeid(Mirror)) {
//                cos_factor =  1;
//            }
            // For lambertian + microfacet weighting, lambertian need to be scaled by 1 - fresnel
            // Fresnel already included in microfacet but not in lambertian, we're doing it here
            Vec3 ratio = 1;
            if (hit_object->spec != nullptr) {
                Vec3 specular_ray = surface_data.normal.reflect(outgoing_dir);
                ratio = Vec3{1} - Fresnel(((CookTorrance*)hit_object->spec)->getReflection().x, specular_ray, surface_data.normal);
            }
            indirect_diffuse = ratio * (Li * f * cos_factor) / pdf;
        }
        if ((options->brdf_bitfield & MICROFACET) && hit_object->spec != nullptr) {

            CookTorrance* spec = (CookTorrance*) hit_object->spec;
            Vec3 f = spec->Sample_f(outgoing_dir, incoming_dir, pdf, surface_data.normal);
//            incoming_dir = spec->Sample_direction(outgoing_dir, pdf, normal);
//            Vec3 f = spec->Evaluate_f(outgoing_dir, incoming_dir, normal);

            Vec3 Li = Raytrace_Recursive(Ray{pos, incoming_dir}, bounce_depth + 1);
            float cos_factor = surface_data.normal.dot(incoming_dir);

            specular = (Li * f * cos_factor) / pdf;
        }

        return indirect_diffuse + specular;
    }

    return 0;
}
//endregion

bool CppRenderer::FindNearestObject(const Ray& ray, float& nearest_dist, Object3D*& hit_object, bool is_occlusion_test) const {

    hit_object = nullptr;

    for (const auto& item: scene->objects) {
        float dist = 99999999;

        if (item->Intersect(ray, dist) && (dist < nearest_dist)) {
            hit_object = item.get();
            nearest_dist = dist;

            if (is_occlusion_test)
                return true;
        }
    }

    return (hit_object != nullptr);
}

void CppRenderer::TracePixel(int x, int y, bool picking) {

    SDL_Surface* surface = SDL_GetWindowSurface(window);

    int width = surface->w;
    int height = surface->h;

    float ratio = (float) width / height;
    float fov_factor = tanf((float) ((fov / 2.f) * (M_PI / 180)));

    Ray ray{camera_controls->GetPosition(), x, y, width, height, ratio, fov_factor};
    ray.direction = camera_controls->GetRotation() * ray.direction;

    if (picking) {
        Object3D* hit_object = nullptr;
        float dist = 999999999.f;
        int triangle_index = -1;
        int submesh_index = -1;
        cout << "Ray origin: " << ray.origin << " direction: " << ray.direction << endl;
//        if (FindNearestObject(ray, dist, hit_object, triangle_index, submesh_index) == true) {
//        if (scene->bvh.FindNearestIntersection(ray, dist, hit_object) == true && hit_object) {
        if (scene->bvh2->FindNearestIntersectionOpti(ray, dist, hit_object) == true && hit_object) {
            selected_object = hit_object;
            cout << "Hit object: " << typeid(*hit_object->shape).name() << endl;
            cout << "Hitpos: " << (ray.origin + ray.direction * dist) << endl;
            if (typeid(*hit_object->shape) == typeid(Triangle)) {
                cout << "Triangle center: " << hit_object->GetCenter() << endl;
            }
        }
    }
    else {
        Raytrace(ray, true);
    }
}

void initializeSRGBTable() {
//    sRGB_table = new char[sRGB_LUT_LENGTH];
    for (int i = 0; i < sRGB_LUT_LENGTH; i++) {
        float Lin = i / (float) (sRGB_LUT_LENGTH - 1);
        float srgb_normalized;
        if (Lin <= 0.0031308f)
            srgb_normalized = (Lin * 12.92f);
        else
            srgb_normalized = ((1 + 0.055f) * powf(Lin, 1 / 2.4f) - 0.055f);

        sRGB_table[i] = (unsigned char) (255 * srgb_normalized);
    }
}

/*
 * Input is linear float [0, 1]
 * Output is srgb float [0, 255]
 */
Vec3 linear_to_sRGB(Vec3 vec) {
    return Vec3{
//            (float)(unsigned char) (255 & sRGB_table[(int)(vec.x * (sRGB_LUT_LENGTH - 1) )]),
//            (float)(unsigned char) (255 & sRGB_table[(int)(vec.y * (sRGB_LUT_LENGTH - 1) )]),
//            (float)(unsigned char) (255 & sRGB_table[(int)(vec.z * (sRGB_LUT_LENGTH - 1) )])
            float(sRGB_table[(int)(vec.x * (sRGB_LUT_LENGTH - 1) )]),
            float(sRGB_table[(int)(vec.y * (sRGB_LUT_LENGTH - 1) )]),
            float(sRGB_table[(int)(vec.z * (sRGB_LUT_LENGTH - 1) )])
    };
}

