#include "OpenCLRenderer.h"

#include "objects/Sphere.h"
#include "objects/Plane.h"
#include "objects/Triangle.h"
#include "opencl/SceneAdapter.h"

#include <fstream>
#include <SDL_timer.h>
#include <chrono>

#define STRINGIFY2( x) #x
#define STRINGIFY(x) STRINGIFY2(x)

#define DUMP_SIZE(type) cout << "sizeof " STRINGIFY(type) " " << sizeof(type) << "  alignof " STRINGIFY(type) " " << alignof(type) << endl;

using std::cout;
using std::cerr;
using std::endl;
using std::string;
using std::shared_ptr;
using std::unique_ptr;
using std::vector;
using std::set;
using std::map;

Program CreateProgram(cl::Context& context, cl::CommandQueue& queue, cl::Device& device, const Options* options) ;

OpenCLRenderer::OpenCLRenderer(Scene* scene, SDL_Window* pWindow, CameraControls* pControls, Options* options,
                               int platform_index, int device_index)
        : BaseRenderer(scene, pWindow, pControls, options) {

    if (platform_index != -1 && device_index != -1) {
        platform = platform_list.getPlatformByIndex(platform_index);
        device   = platform_list.getDeviceByIndex(device_index, platform_index);
    }
    else {

//        platform_list.printPlatformAndDevices();

        // TODO: clean this in initPlatform/Device functions ?
        string vendor = "NVIDIA";
//        string vendor = "Intel";
        char* vendor_env = SDL_getenv("CL_PLATFORM_VENDOR");
        if (vendor_env != nullptr)
            vendor = vendor_env;

//        cl_device_type device_type = CL_DEVICE_TYPE_CPU;
        cl_device_type device_type = CL_DEVICE_TYPE_GPU;
        char* gpu_env = SDL_getenv("USE_CPU");
        if (gpu_env != nullptr) {
            device_type = (gpu_env == string("\"CPU\"")) ? CL_DEVICE_TYPE_CPU : CL_DEVICE_TYPE_GPU;
        }

        platform = platform_list.getPlatformByVendor(vendor);
        device = platform_list.getDeviceByType(device_type, platform);
        cout << "Test: " << device.getInfo<CL_DEVICE_NAME>().c_str() << endl;
        cout << "CL_DEVICE_MAX_CONSTANT_ARGS: " << device.getInfo<CL_DEVICE_MAX_CONSTANT_ARGS>() << endl;
        cout << "CL_DEVICE_MAX_CONSTANT_BUFFER_SIZE: " << device.getInfo<CL_DEVICE_MAX_CONSTANT_BUFFER_SIZE>() << endl;
        cout << "CL_DEVICE_MAX_MEM_ALLOC_SIZE: " << device.getInfo<CL_DEVICE_MAX_MEM_ALLOC_SIZE>() << endl;
        cout << "CL_DEVICE_MAX_PARAMETER_SIZE: " << device.getInfo<CL_DEVICE_MAX_PARAMETER_SIZE>() << endl;
    }

    context = cl::Context {device, nullptr, debugCallback};
    queue = cl::CommandQueue {context, device};

    program = CreateProgram(context, queue, device, options);

    size_t object_count = scene->objects.size();

    cout << "Objects: " << object_count << endl;

    adapter = SceneAdapter {scene};

    const vector<CLObject3D>& object_vector = adapter.GetObjectArray();
    const vector<CLBrdf>& brdf_vector = adapter.GetBrdfArray();

    cl_mem_flags COPY_TO_DEVICE_FLAGS = CL_MEM_READ_ONLY | CL_MEM_HOST_WRITE_ONLY | CL_MEM_COPY_HOST_PTR;

    texture           = CreateBuffer( CL_MEM_WRITE_ONLY | CL_MEM_HOST_READ_ONLY, sizeof(Uint8) * 4 * 1920 * 1080);
    accum_buffer      = CreateBuffer( CL_MEM_READ_WRITE | CL_MEM_HOST_NO_ACCESS, sizeof(float) * 4 * 1920 * 1080);

    object_buffer     = CreateBuffer(COPY_TO_DEVICE_FLAGS, sizeof(CLObject3D) * object_vector.size(), object_vector.data());

    cout << object_vector.size() * sizeof(CLObject3D) / 1024 << " Ko written to CL Device for " << object_vector.size() << " objects " << endl;

    bvh_node_buffer   = CreateBuffer(COPY_TO_DEVICE_FLAGS, sizeof(CLNode2) * adapter.GetBvhNodeArray().size(), adapter.GetBvhNodeArray().data());

    if (scene->GetVertexCount() > 0) {
        pos_buffer        = CreateBuffer (COPY_TO_DEVICE_FLAGS, sizeof(CLVec4) * scene->GetVertexCount(), adapter.GetPosArray().data());
        normal_buffer     = CreateBuffer (COPY_TO_DEVICE_FLAGS, sizeof(CLVec4) * scene->GetVertexCount(), adapter.GetNormalArray().data());
    }

    if (adapter.GetUvArray().size() > 0) {
        uv_buffer         = CreateBuffer (COPY_TO_DEVICE_FLAGS, sizeof(CLVec2) * adapter.GetUvArray().size(), adapter.GetUvArray().data());
    }

    brdfs_buffer      = CreateBuffer( COPY_TO_DEVICE_FLAGS, sizeof(CLBrdf) * brdf_vector.size(), brdf_vector.data());
    cout << brdf_vector.size() * sizeof(CLBrdf) / 1024 << " Ko written to CL Device for " << brdf_vector.size() << " materials" << endl;

    options_buffer    = CreateBuffer( CL_MEM_READ_ONLY | CL_MEM_HOST_WRITE_ONLY, sizeof(CLOptions));

    const vector<TextureUbyte*>& texture_array = adapter.GetTextureArray();

    if (texture_array.size() > 0) {

        const vector<CLTextureInfo>& info_array = adapter.GetTextureInfoArray();
        image_buffer      = CreateBuffer(CL_MEM_READ_ONLY | CL_MEM_HOST_WRITE_ONLY, (size_t) adapter.GetTextureArraySize());
        for (size_t i = 0; i < texture_array.size(); ++i) {
            queue.enqueueWriteBuffer(image_buffer, CL_TRUE, (size_t) info_array[i].byte_offset, texture_array[i]->GetSize(), texture_array[i]->data);
        }
        image_info_buffer = CreateBuffer( COPY_TO_DEVICE_FLAGS, sizeof(CLTextureInfo) * info_array.size(), info_array.data());
        cout << info_array.size() * sizeof(CLTextureInfo) / 1024 << " Ko written to CL Device for " << info_array.size() << " tex infos" << endl;
    }

    env_map_image   = cl::Image2D{context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, cl::ImageFormat{CL_RGBA, CL_FLOAT}, scene->env_map->width, scene->env_map->height, 0, scene->env_map->data};
    cout <<  scene->env_map->GetSize() / 1024 << " Ko written to CL Device for the environnment map" << endl;

    image_index_cache = std::map<string, char>();

    clOptions.fov = tanf((float) ((fov / 2.f) * (M_PI / 180)));

    DUMP_SIZE(CLNode2)

    render_kernel = CreateRenderKernel();

    string name = "non_native";

//    program.DumpBinaries(name);
//    system(("ptx_to_cubin.bat " + name).c_str());
//    exit(0);

    cout << "OpenCL Renderer ready" << endl;
}

cl::Kernel OpenCLRenderer::CreateRenderKernel() const {

    cl::Kernel kernel {program.prog, "render"};

    kernel.setArg(0, texture);
    kernel.setArg(1, accum_buffer);
    kernel.setArg(2, bvh_node_buffer);
    kernel.setArg(3, object_buffer);
    kernel.setArg(4, pos_buffer);
    kernel.setArg(5, normal_buffer);
    kernel.setArg(6, uv_buffer);
    kernel.setArg(7, brdfs_buffer);
    kernel.setArg(8, options_buffer);
    kernel.setArg(9, env_map_image);
    kernel.setArg(10, image_buffer);
    kernel.setArg(11, image_info_buffer);

    return kernel;
}

Program CreateProgram(cl::Context& context, cl::CommandQueue& queue, cl::Device& device, const Options* options) {

    vector<string> build_options = {
            " -cl-std=CL1.2",
            " -cl-fast-relaxed-math",
//            " -cl-unsafe-math-optimizations",
//            " -cl-single-precision-constant",
//            " -cl-no-signed-zeros -cl-mad-enable -Werror",
//            " -save-temps=binary", // AMD specific
    };

    if (options->use_bvh)
        build_options.push_back(" -D USE_BVH");
//    build_options.push_back(" -cl-opt-disable");
//    build_options.push_back(" -cl-nv-opt-level=0");
//    build_options.push_back(" -cl-nv-maxrregcount=30");

    vector<string> source_array = {
            "tonemap.cl",
            "texture.cl",
            "brdf.cl",
            "objects.cl",
            "bvh.cl",
            "material.cl",
            "render.cl",
    };

    Uint32 start = SDL_GetTicks();

    Program program {source_array, build_options};
    program.Build(context, device);

    cout << "Program compiled in " << (SDL_GetTicks() - start) / 1000.f << "s" << endl;

    return program;
}

OpenCLRenderer::~OpenCLRenderer() {
    cout << "OpenCL Renderer destructor called" << endl;
}

void OpenCLRenderer::Draw() {

    SDL_Surface* surface = SDL_GetWindowSurface(window);
    Uint32* pixels = (Uint32*) surface->pixels;
    size_t width = (size_t) surface->w;
    size_t height = (size_t) surface->h;

//    queue.enqueueNDRangeKernel(render_kernel, cl::NullRange, cl::NDRange(width, height));
    queue.enqueueNDRangeKernel(render_kernel, cl::NullRange, cl::NDRange(width, height), cl::NDRange(8, 4));
//    queue.enqueueNDRangeKernel(render_kernel, cl::NullRange, cl::NDRange(width, height), cl::NDRange(8, 8));
    queue.enqueueReadBuffer(texture, CL_TRUE, 0, sizeof(Uint32) * width * height, pixels);
}

void OpenCLRenderer::TracePixel(int x, int y, bool picking) {

    SDL_Surface* surface = SDL_GetWindowSurface(window);
    size_t width = (size_t) surface->w;
    size_t height = (size_t) surface->h;

    float coord[2] = {float(x), float(y)};
    int hit_object_index = -1;

    cl::Buffer hit_object_output_buffer  = cl::Buffer {context, CL_MEM_WRITE_ONLY | CL_MEM_HOST_READ_ONLY, sizeof(int)};
    cl::Buffer coord_input_buffer        = cl::Buffer {context, CL_MEM_READ_ONLY  | CL_MEM_HOST_WRITE_ONLY, sizeof(float) * 2};

    queue.enqueueWriteBuffer(coord_input_buffer, CL_TRUE, 0, sizeof(float) * 2, coord);

    cl::make_kernel<cl::Buffer&, cl::Buffer&, cl::Buffer&, cl::Buffer&, cl::Buffer&, cl::Buffer&, cl::Buffer&> kernel(program.prog, "Intersect");
    cl::EnqueueArgs enqueueArgs(queue, cl::NDRange(width, height));
    kernel(enqueueArgs, hit_object_output_buffer, coord_input_buffer, bvh_node_buffer, object_buffer, pos_buffer, normal_buffer, options_buffer).wait();

    queue.enqueueReadBuffer(hit_object_output_buffer, CL_TRUE, 0, sizeof(int), &hit_object_index);

    if (hit_object_index == -1) {
        selected_object = nullptr;
    } else if (hit_object_index < int(scene->objects.size())) {
        selected_object = scene->objects[hit_object_index].get();
    }
}

void OpenCLRenderer::Update() {

    BaseRenderer::Update();

    if (update_option) {

        if (options->use_bvh)
            program.AddBuildOption(" -D USE_BVH");
        else
            program.RemoveBuildOption(" -D USE_BVH");

        if (use_fast_math)
            program.AddBuildOption(" -cl-fast-relaxed-math");
        else
            program.RemoveBuildOption(" -cl-fast-relaxed-math");

        reload_kernel = true;
        update_option = false;
    }

    if (program.Refresh(context, device, reload_kernel)) {
        CLEAR_ACCUM_BIT = 0;
        frame_number = 0;
        render_kernel = CreateRenderKernel();
        reload_kernel = false;
    }

    //TODO: Only update these data when they have changed ?
    auto start = std::chrono::high_resolution_clock::now();
    UpdateCLScene();
    auto end = std::chrono::high_resolution_clock::now();

    if ((start - end).count() > 1.f)
        cout << "UpdateCLScene: " << std::chrono::duration<float>(end - start).count() * 1000.f << "ms" << endl;

    UpdateCLOptions();
}

void OpenCLRenderer::UpdateCLScene() {

    clOptions.sphere_count = 0;
    clOptions.plane_count = 0;
}

void OpenCLRenderer::KeyEvent(SDL_Keysym keysym, SDL_EventType type) {

    BaseRenderer::KeyEvent(keysym, type);

    if (type == SDL_KEYUP) {

        switch (keysym.sym) {
            case SDLK_k:
                reload_kernel = true;
                break;
            case SDLK_b:
                update_option = true;
                break;
            case SDLK_f:
                use_fast_math =! use_fast_math;
                update_option = true;
                cout << "OpenCL fast math set to " << use_fast_math << endl;
                break;
            default:
                break;
        }
    }
}
// Update the clOptions struct to be sent to the opencl device
void OpenCLRenderer::UpdateCLOptions() {

    clOptions.use_emissive_lighting    = options->use_emissive_lighting;
    clOptions.use_distant_env_lighting = options->use_distant_env_lighting;
    clOptions.brdf_bitfield            = options->brdf_bitfield;
    clOptions.use_tonemapping          = options->use_tonemapping;
    clOptions.triangle_count           = std::min(100, scene->GetTriangleCount());
    clOptions.sample_count             = options->sample_count;
    clOptions.bounce_count             = options->bounce_cout;
    clOptions.debug                    = debug;
    clOptions.accum_clear_bit          = CLEAR_ACCUM_BIT;
    clOptions.frame_number             = frame_number;
    clOptions.origin                   = camera_controls->GetPosition();
    clOptions.rotation                 = camera_controls->GetRotation();

    queue.enqueueWriteBuffer(options_buffer, CL_TRUE, 0, sizeof(CLOptions), &clOptions);
}

cl::Buffer OpenCLRenderer::CreateBuffer(cl_mem_flags flags, size_t size, const void* data) {

    auto max_size = device.getInfo<CL_DEVICE_MAX_CONSTANT_BUFFER_SIZE>();

    if (flags == (CL_MEM_READ_ONLY | CL_MEM_HOST_WRITE_ONLY) && (size >= max_size)) {
        auto device_type = device.getInfo<CL_DEVICE_TYPE>();
        auto vendor = device.getInfo<CL_DEVICE_VENDOR>();
        // Intel Gpu
        if (vendor.find("Intel") != string::npos && device_type == CL_DEVICE_TYPE_GPU)
            cerr << "WARNING: Requested " << size / 1024 << " Ko of constant buffer, max is: " << max_size / 1024 << " Ko" << endl;
        else
            cout << "WARNING: Requested " << size / 1024 << " Ko of constant buffer, max is: " << max_size / 1024 << " Ko" << endl;
    }

    return cl::Buffer{context, flags, size, (void*)data};
}


void CL_CALLBACK OpenCLRenderer::debugCallback(const char* errinfo, const void* private_info, size_t cb, void* user_data) {
    cout << "Debug Callback: " << errinfo << '\n';
    cout << "Debug Callback: [private_info] : "<< private_info << '\n';
    cout << "Debug Callback: [cb] : "<< cb << endl;
}

#define CASE_MACRO(name) case name: return #name;
const char* OpenCLRenderer::GetCLErrorString(int code) {
    switch (code) {
        CASE_MACRO(CL_SUCCESS)
        CASE_MACRO(CL_DEVICE_NOT_FOUND)
        CASE_MACRO(CL_DEVICE_NOT_AVAILABLE)
        CASE_MACRO(CL_COMPILER_NOT_AVAILABLE)
        CASE_MACRO(CL_MEM_OBJECT_ALLOCATION_FAILURE)
        CASE_MACRO(CL_OUT_OF_RESOURCES)
        CASE_MACRO(CL_OUT_OF_HOST_MEMORY)
        CASE_MACRO(CL_PROFILING_INFO_NOT_AVAILABLE)
        CASE_MACRO(CL_MEM_COPY_OVERLAP)
        CASE_MACRO(CL_IMAGE_FORMAT_MISMATCH)
        CASE_MACRO(CL_IMAGE_FORMAT_NOT_SUPPORTED)
        CASE_MACRO(CL_BUILD_PROGRAM_FAILURE)
        CASE_MACRO(CL_MAP_FAILURE)
        CASE_MACRO(CL_MISALIGNED_SUB_BUFFER_OFFSET)
        CASE_MACRO(CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST)
        CASE_MACRO(CL_COMPILE_PROGRAM_FAILURE)
        CASE_MACRO(CL_LINKER_NOT_AVAILABLE)
        CASE_MACRO(CL_LINK_PROGRAM_FAILURE)
        CASE_MACRO(CL_DEVICE_PARTITION_FAILED)
        CASE_MACRO(CL_KERNEL_ARG_INFO_NOT_AVAILABLE)
        CASE_MACRO(CL_INVALID_VALUE)
        CASE_MACRO(CL_INVALID_DEVICE_TYPE)
        CASE_MACRO(CL_INVALID_PLATFORM)
        CASE_MACRO(CL_INVALID_DEVICE)
        CASE_MACRO(CL_INVALID_CONTEXT)
        CASE_MACRO(CL_INVALID_QUEUE_PROPERTIES)
        CASE_MACRO(CL_INVALID_COMMAND_QUEUE)
        CASE_MACRO(CL_INVALID_HOST_PTR)
        CASE_MACRO(CL_INVALID_MEM_OBJECT)
        CASE_MACRO(CL_INVALID_IMAGE_FORMAT_DESCRIPTOR)
        CASE_MACRO(CL_INVALID_IMAGE_SIZE)
        CASE_MACRO(CL_INVALID_SAMPLER)
        CASE_MACRO(CL_INVALID_BINARY)
        CASE_MACRO(CL_INVALID_BUILD_OPTIONS)
        CASE_MACRO(CL_INVALID_PROGRAM)
        CASE_MACRO(CL_INVALID_PROGRAM_EXECUTABLE)
        CASE_MACRO(CL_INVALID_KERNEL_NAME)
        CASE_MACRO(CL_INVALID_KERNEL_DEFINITION)
        CASE_MACRO(CL_INVALID_KERNEL)
        CASE_MACRO(CL_INVALID_ARG_INDEX)
        CASE_MACRO(CL_INVALID_ARG_VALUE)
        CASE_MACRO(CL_INVALID_ARG_SIZE)
        CASE_MACRO(CL_INVALID_KERNEL_ARGS)
        CASE_MACRO(CL_INVALID_WORK_DIMENSION)
        CASE_MACRO(CL_INVALID_WORK_GROUP_SIZE)
        CASE_MACRO(CL_INVALID_WORK_ITEM_SIZE)
        CASE_MACRO(CL_INVALID_GLOBAL_OFFSET)
        CASE_MACRO(CL_INVALID_EVENT_WAIT_LIST)
        CASE_MACRO(CL_INVALID_EVENT)
        CASE_MACRO(CL_INVALID_OPERATION)
        CASE_MACRO(CL_INVALID_GL_OBJECT)
        CASE_MACRO(CL_INVALID_BUFFER_SIZE)
        CASE_MACRO(CL_INVALID_MIP_LEVEL)
        CASE_MACRO(CL_INVALID_GLOBAL_WORK_SIZE)
        CASE_MACRO(CL_INVALID_PROPERTY)
        CASE_MACRO(CL_INVALID_IMAGE_DESCRIPTOR)
        CASE_MACRO(CL_INVALID_COMPILER_OPTIONS)
        CASE_MACRO(CL_INVALID_LINKER_OPTIONS)
        CASE_MACRO(CL_INVALID_DEVICE_PARTITION_COUNT)
        default:
            return ("Unknown CL error code: " + std::to_string(code)).c_str();
    }
}

