#ifndef OPENCL_OPENCLRENDERER_H
#define OPENCL_OPENCLRENDERER_H

#include <map>
#include <core/BVH.h>
#include "BaseRenderer.h"
#include "opencl/Program.h"
#include "opencl/OpenCLPlatformList.h"
#include "CL/cl.hpp"
#include "opencl/SceneAdapter.h"

// See render.h for layout/alignement
struct CLOptions {
    Vec3 origin;
    char pad4[4];
    Matrix rotation;
    float fov;
    int triangle_count;
    short sample_count;
    short bounce_count;
    short frame_number;
    char use_emissive_lighting;
    char use_distant_env_lighting;
    char brdf_bitfield;
    char use_tonemapping;
    char accum_clear_bit;
    char sphere_count;
    char plane_count;
    char debug;
};

class OpenCLRenderer : public BaseRenderer {

private:
    OpenCLPlatformList platform_list;
    cl::Platform platform;
    cl::Device device;
    cl::Context context;
    cl::Kernel render_kernel;
    Program program;
    cl::CommandQueue queue;
    cl::Buffer texture;
    cl::Buffer accum_buffer;
    cl::Buffer object_buffer;
    cl::Buffer bvh_node_buffer;
    cl::Buffer pos_buffer;
    cl::Buffer normal_buffer;
    cl::Buffer uv_buffer;
    cl::Buffer brdfs_buffer;
    cl::Buffer options_buffer;
    cl::Buffer image_buffer;
    cl::Buffer image_info_buffer;
    cl::Image2D env_map_image;
    std::map<std::string, char> image_index_cache;
    CLOptions clOptions;
    SceneAdapter adapter;

    bool reload_kernel = false;
    bool update_option = false;
    bool use_fast_math = true;
    const int MAX_IMAGE_COUNT = 200;
    unsigned char image_count = 0;

public:
    OpenCLRenderer(Scene* scene, SDL_Window* pWindow, CameraControls* pControls, Options* options,
                   int platform_index = -1, int device_index = -1);

    ~OpenCLRenderer() override;

    void Draw();

    void TracePixel(int x, int y, bool picking);

    const OpenCLPlatformList& getPlatformList() const {
        return platform_list;
    }

    int getCurrentPlatformIndex() {
        return platform_list.FindPlatformIndex(platform);
    }

    int getCurrentDeviceIndex() {
        return platform_list.FindDeviceIndex(device, platform);
    }

    static const char* GetCLErrorString(int code);

    void Update() override;

private:

    void KeyEvent(SDL_Keysym keysym, SDL_EventType type);
    void UpdateCLScene();
    void UpdateCLOptions();


    cl::Buffer CreateBuffer(cl_mem_flags flags, size_t size, const void* data = nullptr);

    void dump_program(std::string filename);

    static void CL_CALLBACK debugCallback (const char *errinfo, const void *private_info, size_t cb, void *user_data);

    cl::Kernel CreateRenderKernel() const;
};

#endif //OPENCL_OPENCLRENDERER_H