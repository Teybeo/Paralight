#ifndef RENDERER_H
#define RENDERER_H

#include "BaseRenderer.h"

class CppRenderer : public BaseRenderer {

    std::vector<Vec3> accum_texture;

public:

    CppRenderer() = default;
    CppRenderer(Scene* scene, SDL_Window* pWindow, Film* film, CameraControls* const controls, Options* options);

    ~CppRenderer() override;

    void Render();

    void Update() override;

    void TracePixel(Vec3 pixel, bool picking);

    Vec3 Raytrace(Ray ray, bool debug_pixel = false);

    bool FindNearestObject(const Ray& ray, float& nearest_dist, Object3D*& hit_object, bool is_occlusion_test) const;

    Vec3 Raytrace_Recursive(Ray ray, const int bounce_depth = 0);

};

#endif //RENDERER_H
