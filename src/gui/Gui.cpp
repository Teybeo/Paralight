#include "Gui.h"

#include "renderers/OpenCLRenderer.h"
#include "renderers/CppRenderer.h"
#include "objects/Plane.h"
#include "imgui/imgui_impl_sdl.h"
#include "imgui/imgui_internal.h"

#include <GL/gl.h>
#include <iostream>
#include "tinydir/tinydir.h"

using std::string;
using std::vector;

int GetEnvMapIndex(const string& env_map, vector<string> env_map_array);
vector<string> GetEnvMapArray(const char* env_map_dir);

std::vector<std::string> GetModelArray(const string& model_dir_path) ;

static void showBVHStatistics() ;

using std::shared_ptr;

GUI::GUI(Options* options, SDL_Window* window, BaseRenderer*& renderer, Scene* scene) :
    scene{scene}, options(options), window{window}, renderer(renderer)
{
    // Setup ImGui binding
    ImGui_ImplSdl_Init(window);
    ImGuiIO& io = ImGui::GetIO();
    io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\Arial.ttf", 14.0f);
    io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\Consola.ttf", 15.0f);

    cout << io.Fonts->Fonts.size() << " fonts" << endl;

    envmap_array = GetEnvMapArray(env_map_dir);
    envmap_index = GetEnvMapIndex(scene->env_map->path, envmap_array);

    model_array = GetModelArray(model_dir);

    int w, h;
    SDL_GetWindowSize(window, &w, &h);
    window_size = ImVec2 {w * 0.9f, h * 0.9f};

    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(8, 5));
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(5, 5));
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 3.0f);

    window_flags = 0;
    window_flags |= ImGuiWindowFlags_NoTitleBar;
    window_flags |= ImGuiWindowFlags_NoResize;
    window_flags |= ImGuiWindowFlags_NoSavedSettings;
    window_flags |= ImGuiWindowFlags_NoMove;

    OpenCLRenderer* cl_renderer = dynamic_cast<OpenCLRenderer*>(renderer);
    if (cl_renderer != nullptr) {
        selected_device   = cl_renderer->getCurrentDeviceIndex();
        selected_platform = cl_renderer->getCurrentPlatformIndex();
    } else {
        selected_device   = -1;
        selected_platform = -1;
    }
}

void GUI::Draw() {

    int w, h;
    SDL_GetWindowSize(window, &w, &h);
    window_size = ImVec2 {w * 0.9f, h * 0.9f};

    ImGui_ImplSdl_NewFrame(window);

    ImGui::SetNextWindowPosCenter();
    ImGui::SetNextWindowSize(window_size);

    ImGui::Begin("Settings", nullptr, window_flags);
    {
        showRendererSettings();
        showLightingSettings();
        showObjectSettings();
//        ShowBVHTree(scene->bvh);
        ImGui::Text("Avg %.0f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
//        showBVHStatistics();
    }
    ImGui::End();

    // Rendering
    glViewport(0, 0, (int)ImGui::GetIO().DisplaySize.x, (int)ImGui::GetIO().DisplaySize.y);
    const Vec3 clear_color {0.2, 0.2, 0.2};
    glClearColor(clear_color.x, clear_color.y, clear_color.z, 1);
    glClear(GL_COLOR_BUFFER_BIT);
    ImGui::Render();
    SDL_GL_SwapWindow(window);
}

void GUI::Update() {

    // Can't set it directly in Draw because Gui is drawn after Renderer
    // so at next App::Update, Options::Update is called and it resets this flag
    // Instead we set it here as Gui::Update is called after Options::Update
    if (options_has_changed)
        options->has_changed = true;

    options_has_changed = false;
}

void GUI::showRendererSettings() {

    if (ImGui::CollapsingHeader("Renderer", nullptr, true, true)) {
        bool is_opencl = (typeid(*renderer) == typeid(OpenCLRenderer));
        if (ImGui::RadioButton("C++", !is_opencl)) {
            SDL_Event event;
            event.type = SDL_USEREVENT;
            event.user.code = BaseRenderer::EVENT_CPP_RENDERER;
            SDL_PushEvent(&event);
        }
        if (ImGui::RadioButton("OpenCL", is_opencl)) {
            SDL_Event event;
            event.type = SDL_USEREVENT;
            event.user.code = BaseRenderer::EVENT_CL_RENDERER;
            SDL_PushEvent(&event);
        }
        if (is_opencl == false) {
            showOpenCLSettings();
        }
    }
}

void GUI::showOpenCLSettings() {

    if (ImGui::TreeNode("Platform/Device selection")) {

        OpenCLRenderer* cl_renderer = static_cast<OpenCLRenderer*>(renderer);
        OpenCLPlatformList platform_list = cl_renderer->getPlatformList();

        // If the ctor was called with a CPP renderer, these values were not set, so do it now
        if (selected_device == -1)
            selected_device = cl_renderer->getCurrentDeviceIndex();
        if (selected_platform == -1)
            selected_platform = cl_renderer->getCurrentPlatformIndex();

        if (ImGui::Combo("Platform", &selected_platform, platform_list.getPlatformNames(), platform_list.getPlatformCount())) {
            // If selecting the current platform, preselect the current device
            // Else the first device of the new platform
            if (selected_platform == cl_renderer->getCurrentPlatformIndex()) {
                selected_device = cl_renderer->getCurrentDeviceIndex();
            } else {
                selected_device = 0;
            }

        }
        ImGui::Combo("Device", &selected_device, platform_list.getDevicesNames(selected_platform), platform_list.getDeviceCount(selected_platform));

        bool can_apply = selected_platform != cl_renderer->getCurrentPlatformIndex() ||
                            selected_device   != cl_renderer->getCurrentDeviceIndex();

        int button_flags = can_apply ? 0 : ImGuiButtonFlags_Disabled;
        if (can_apply && ImGui::ButtonEx("Apply", ImVec2(0, 0), button_flags)) {

            std::cout << "Set platform: " << selected_platform << "\n";
            std::cout << "Set device: " << selected_device << "\n";
            SDL_Event event;
            event.type = SDL_USEREVENT;
            event.user.code = BaseRenderer::EVENT_CL_PLATFORM_DEVICE_CHANGE;
            event.user.data1 = &selected_platform;
            event.user.data2 = &selected_device;
            SDL_PushEvent(&event);
        }
        ImGui::TreePop();
    }
}

bool item_getter(void* data, int current_index, const char** name) {
    vector<string>& envmap_array = *(vector<string>*)data;
    string& env_map = envmap_array[current_index];
    // Remove the folder subpaths for display
    size_t offset = env_map.find_last_of("/\\") + 1;
    *name = env_map.c_str() + offset;
    return true;
}

void GUI::showLightingSettings() {

    scene->envmap_has_changed = false;
    scene->model_has_changed = false;

    if (ImGui::CollapsingHeader("Render settings", nullptr, true, true)) {

        ImGui::PushItemWidth(ImGui::GetWindowContentRegionWidth() / 2.f);
            int temp_envmap_index = envmap_index;
            ImGui::Combo("Environnement Map", &temp_envmap_index, item_getter, &envmap_array, (int) envmap_array.size());

            if (temp_envmap_index != envmap_index) {
                envmap_index = temp_envmap_index;
                scene->env_map.reset(new TextureFloat {envmap_array[envmap_index]});
                scene->envmap_has_changed = true;
            }
        ImGui::PopItemWidth();

        ImGui::PushItemWidth(ImGui::GetWindowContentRegionWidth() / 2.f);
            int temp_model_index = model_index;
            bool changed = ImGui::Combo("Model", &temp_model_index, item_getter, &model_array, (int) model_array.size());

            if (changed || temp_model_index != model_index) {
                model_index = temp_model_index;
                scene->Clear();
                scene->LoadObjects(model_array[model_index]);
                scene->model_has_changed = true;
            }
        ImGui::PopItemWidth();

        bool lambertian = (bool) (options->brdf_bitfield & LAMBERTIAN);
        bool microfacet = (bool) (options->brdf_bitfield & MICROFACET);
        if (ImGui::Checkbox("Lambertian BRDF", &lambertian)) {
            options->brdf_bitfield ^= LAMBERTIAN;
            options_has_changed = true;
        }
        if (ImGui::Checkbox("Microfacet BRDF", &microfacet)) {
            options->brdf_bitfield ^= MICROFACET;
            options_has_changed = true;
        }
        options_has_changed |= ImGui::Checkbox("Tonemapping", &options->use_tonemapping);
        options_has_changed |= ImGui::Checkbox("Emissive lighting", &options->use_emissive_lighting);
        options_has_changed |= ImGui::Checkbox("Distant Environnment lighting", &options->use_distant_env_lighting);
        options_has_changed |= ImGui::Checkbox("Debug", &renderer->debug);
//        if (ImGui::SliderInt("Depth target", &options->depth_target, 0, 30, "%.0f")) {
//            options_has_changed = true;
//        }
    }

}

void GUI::showObjectSettings() {

    if (ImGui::CollapsingHeader("Object settings", nullptr, true, true)) {

        Object3D* object = renderer->GetSelectedObject();
        if (object == nullptr)
            return;

        showMaterialSettings(object);
    }
}

void GUI::showMaterialSettings(Object3D* object) {

    scene->material_has_changed = false;
    scene->emission_has_changed = false;

    if (object->getEmissionIntensity() != -1) {

        Vec3 color = object->getEmissionColor().pow(1.f / 2.2f); // Linear to sRGB
        float intensity = object->getEmissionIntensity();

        if (ImGui::ColorEdit3("Light color", &color.x)) {
            object->setEmissionColor(color.pow(2.2f)); // sRGB to Linear
            scene->emission_has_changed = true;
        }
        if (ImGui::SliderFloat("Light intensity", &intensity, 0, 100, "%.3f", 2)) {
            object->setEmissionIntensity(intensity);
            scene->emission_has_changed = true;
        }

        return;
    }

    Standard* standard = dynamic_cast<Standard*>(object->material);
    if (standard != nullptr) {
        showTextureSettings(standard->GetAlbedo(), "Albedo");
        showTextureSettings(standard->GetRoughness(), "Roughness");
        showTextureSettings(standard->GetReflectance(), "Reflectance");
        showTextureSettings(standard->GetNormal(), "Normal");
    }

    LambertianMaterial* lambertian = dynamic_cast<LambertianMaterial*>(object->material);
    if (lambertian != nullptr) {
        showTextureSettings(lambertian->GetAlbedo(), "Albedo");
    }
}

void GUI::showTextureSettings(std::shared_ptr<Texture> texture, const char* texture_name) {

    ValueTex3f* scalar_tex = dynamic_cast<ValueTex3f*>(texture.get());

    if (scalar_tex != nullptr) {

        // The renderers consider colors to be in Linear space and convert them to sRGB at the end (gamma correction)
        // ImGui considers colors to be already in sRGB because it doesn't apply gamma correction
        // To keep color consistent, we simply convert them to sRGB before sending them to ImGui and convert them back after

        Vec3 color_value = scalar_tex->value.pow(1.f / 2.2f); // Linear to sRGB

        if (ImGui::ColorEdit3(texture_name, &color_value.x)) {
            scalar_tex->value = color_value.pow(2.2f); // sRGB to Linear
            scene->material_has_changed = true;
        }
        return;
    }

    ValueTex1f* scalar = dynamic_cast<ValueTex1f*>(texture.get());

    if (scalar != nullptr) {

        // The renderers consider colors to be in Linear space and convert them to sRGB at the end (gamma correction)
        // ImGui considers colors to be already in sRGB because it doesn't apply gamma correction
        // To keep color consistent, we simply convert them to sRGB before sending them to ImGui and convert them back after

        float value = std::pow(scalar->value, 1.f / 2.2f); // Linear to sRGB

        if (ImGui::SliderFloat(texture_name, &value, 0.001f, 1.0f)) {
            scalar->value = std::pow(value, 2.2f); // sRGB to Linear
            scene->material_has_changed = true;
        }
        return;
    }

    TextureUbyte* tex_ubyte = dynamic_cast<TextureUbyte*>(texture.get());
    if (tex_ubyte != nullptr) {
        ImGui::Text(tex_ubyte->path.c_str());
    }

    TextureFloat* tex_float = dynamic_cast<TextureFloat*>(texture.get());
    if (tex_float != nullptr) {
        ImGui::Text(tex_float->path.c_str());
    }
}

// tinydir_open_sorted read files/directories inside specified path

std::vector<std::string> GetModelArray(const string& model_dir_path) {

    tinydir_dir dir;
    tinydir_open_sorted(&dir, model_dir_path.c_str());

    vector<string> model_array;

    for (size_t i = 0; i < dir.n_files; i++) {

        tinydir_file file;
        tinydir_readfile_n(&dir, &file, i);

        if (file.is_dir && file.name != string(".") && file.name != string("..")) {
            const auto& sub_array = GetModelArray(string(file.path) + "/");
            model_array.insert(model_array.end(), sub_array.begin(), sub_array.end());
        }
        else if (strcmp(file.extension, "obj") == 0) {
            model_array.push_back(model_dir_path + file.name);
        }

    }

    tinydir_close(&dir);

    return model_array;
}

vector<string> GetEnvMapArray(const char* env_map_dir) {

    tinydir_dir dir;
    tinydir_open_sorted(&dir, env_map_dir);

    vector<string> env_map_array;

    for (size_t i = 0; i < dir.n_files; i++) {

        tinydir_file file;
        tinydir_readfile_n(&dir, &file, i);

        if (strcmp(file.extension, "hdr") == 0) {
            env_map_array.push_back(string(env_map_dir) + file.name);
        }
    }

    tinydir_close(&dir);

    return env_map_array;
}

int GetEnvMapIndex(const string& env_map, vector<string> env_map_array) {
    for (int i = 0; i < (int) env_map_array.size(); ++i) {
        if (env_map_array[i] == env_map)
            return i;
    }
    return 0;
}

static void ShowBVHTree(const BVH& bvh)
{
//    ImGui::SetNextWindowSize(ImVec2(430,450), ImGuiSetCond_FirstUseEver);
//    if (!ImGui::Begin("Example: Property editor", opened))
//    {
//        ImGui::End();
//        return;
//    }
    if (ImGui::CollapsingHeader("BVH", nullptr, true, true)) {

    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(2,2));
    ImGui::Columns(4);
    ImGui::Separator();
    ImFontAtlas* atlas = ImGui::GetIO().Fonts;

    if (atlas->Fonts.size() >= 2)
        ImGui::PushFont(atlas->Fonts[1]);

    struct funcs
    {
        static void ShowBVHNode(Node* node, const char* prefix, ImU32 uid)
        {
            ImGui::PushID(uid);                      // Use object uid as identifier. Most commonly you could also use the object pointer as a base ID.
            ImGui::AlignFirstTextHeightToWidgets();  // Text and Tree nodes are less high than regular widgets, here we add vertical spacing to make the tree lines equal high.
            bool opened = ImGui::TreeNode("Node", "%s %u", prefix, uid);
            ImGui::NextColumn();

            Vec3& min = node->bbox.min;
            Vec3& max = node->bbox.max;

            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1, 0.5, 0.5, 1));
            ImGui::AlignFirstTextHeightToWidgets();
            ImGui::Text("X:[%g, %g]", min.x, max.x);
            ImGui::PopStyleColor();
            ImGui::NextColumn();

            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.5, 1, 0.5, 1));
            ImGui::AlignFirstTextHeightToWidgets();
            ImGui::Text("Y:[%g, %g]", min.y, max.y);
            ImGui::PopStyleColor();
            ImGui::NextColumn();

            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.5, 0.5, 1, 1));
            ImGui::AlignFirstTextHeightToWidgets();
            ImGui::Text("Z:[%g, %g]", min.z, max.z);
            ImGui::PopStyleColor();
            ImGui::NextColumn();

            if (opened)
            {
                ImGui::Separator();
                if (node->children.empty() == true) {

                    char label[32];
                    sprintf(label, "Obj*: %p", node->object);
                    const auto& bbox = node->object->ComputeBBox();

                    ImGui::AlignFirstTextHeightToWidgets();
                    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1, 1, 1, 1));
                    ImGui::Bullet();
                    ImGui::Text(label);
                    ImGui::PopStyleColor();
                    ImGui::NextColumn();

                    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1, 0.25, 0.25, 1));
                    ImGui::Text("%g", node->object->GetCenter().x);
                    ImGui::Text("[%g, %g]", bbox.min.x, bbox.max.x);
                    ImGui::PopStyleColor();
                    ImGui::NextColumn();

                    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.25, 1, 0.25, 1));
                    ImGui::Text("%g", node->object->GetCenter().y);
                    ImGui::Text("[%g, %g]", bbox.min.y, bbox.max.y);
                    ImGui::PopStyleColor();
                    ImGui::NextColumn();

                    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.25, 0.25, 1, 1));
                    ImGui::Text("%g", node->object->GetCenter().z);
                    ImGui::Text("[%g, %g]", bbox.min.z, bbox.max.z);
                    ImGui::PopStyleColor();
                    ImGui::NextColumn();
                }
                else {

                    ImU32 i = 0;
                    for (const auto& child : node->children)
                    {
                        ImGui::PushID(&child); // Use field index as identifier.
                        ShowBVHNode(child.get(), "Child", i++);
                        ImGui::PopID();
                    }
                }
                ImGui::TreePop();
                ImGui::Separator();
            }
            ImGui::PopID();
        }
    };

    // Iterate dummy objects with dummy members (all the same data)
    funcs::ShowBVHNode(bvh.GetRoot(), "Node", 0);

    ImGui::Columns(1);
    ImGui::Separator();
    ImGui::PopStyleVar();
    ImGui::PopFont();
    }
}

static void showBVHStatistics() {
    ImGui::Text("BBox Tests: %.2f K", BVH2::ray_bbox_test_count / 1000.f);
    ImGui::Text("Object Tests: %.2f K", BVH2::ray_obj_test_count / 1000.f);

    ImGui::Text("Object Tests Avoided: %.2f K (%g %%)", (BVH2::ray_bbox_test_count - BVH2::ray_obj_test_count) / 1000.f, ((BVH2::ray_bbox_test_count - BVH2::ray_obj_test_count) / float(BVH2::ray_bbox_test_count)) * 100.f);

    ImGui::Text("BBox Tests Success Rate: %.2f K (%g %%)", BVH2::ray_bbox_hit_count / 1000.f, BVH2::ray_bbox_hit_count / float(BVH2::ray_bbox_test_count) * 100);
    ImGui::Text("Object Tests Success Rate: %.2f K (%g %%)", BVH2::ray_obj_hit_count / 1000.f, BVH2::ray_obj_hit_count / float(BVH2::ray_obj_test_count) * 100);
}

