#include "Gui.h"

#include "renderers/OpenCLRenderer.h"
#include "renderers/CppRenderer.h"
#include "objects/Plane.h"
#include "imgui/imgui_impl_sdl.h"
#include "imgui/imgui_internal.h"

#include <GL/gl.h>
#include <iostream>

static void ShowBVHTree(bool* opened, const BVH& bvh);

using std::shared_ptr;

GUI::GUI(Options* options, SDL_Window* window, BaseRenderer*& renderer) :
    options(options), window{window}, renderer(renderer)
{
    // Setup ImGui binding
    ImGui_ImplSdl_Init(window);
    ImGuiIO& io = ImGui::GetIO();
    io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\Arial.ttf", 14.0f);
    io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\Consola.ttf", 15.0f);

    cout << io.Fonts->Fonts.size() << " fonts" << endl;

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

void GUI::Update() {
    if (has_changed || material_has_changed)
        options->options_changed = true;

}

void GUI::Draw() {

    int w, h;
    SDL_GetWindowSize(window, &w, &h);
    window_size = ImVec2 {w * 0.9f, h * 0.9f};

    has_changed = false;
    material_has_changed = false;

    ImGui_ImplSdl_NewFrame(window);

    ImGui::SetNextWindowPosCenter();
    ImGui::SetNextWindowSize(window_size);

    ImGui::Begin("Settings", nullptr, window_flags);
    {
        if (ImGui::CollapsingHeader("Renderer", nullptr, true, true)) {
            bool cpp = (typeid(*renderer) == typeid(CppRenderer));
            showRendererSelection(cpp);
            if (cpp == false) {
                showOpenCLSettings();
            }
        }
        showLightingSettings();
        showObjectSettings();
//        bool opened = false;
//        ShowExampleAppPropertyEditor(&opened);
        bool cpp = (typeid(*renderer) == typeid(CppRenderer));
        if (cpp) {
//            ShowBVHTree(&opened, static_cast<CppRenderer*>(renderer)->GetScene()->bvh);
        }
        ImGui::Text("Application average %.0f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

        ImGui::Text("BBox Tests: %.2f K", BVH2::ray_bbox_test_count / 1000.f);
        ImGui::Text("Object Tests: %.2f K", BVH2::ray_obj_test_count / 1000.f);

        ImGui::Text("Object Tests Avoided: %.2f K (%g %%)", (BVH2::ray_bbox_test_count - BVH2::ray_obj_test_count) / 1000.f, ((BVH2::ray_bbox_test_count - BVH2::ray_obj_test_count) / float(BVH2::ray_bbox_test_count)) * 100.f);

        ImGui::Text("BBox Tests Success Rate: %.2f K (%g %%)", BVH2::ray_bbox_hit_count / 1000.f, BVH2::ray_bbox_hit_count / float(BVH2::ray_bbox_test_count) * 100);
        ImGui::Text("Object Tests Success Rate: %.2f K (%g %%)", BVH2::ray_obj_hit_count / 1000.f, BVH2::ray_obj_hit_count / float(BVH2::ray_obj_test_count) * 100);

//        ImGui::Text("Triangle Hit/Test percent: %.4f %%", TriMesh::GetHitCount() / float(TriMesh::GetTestCount()) * 100);
//        ImGui::Text("Triangle Tests: %.2f K", TriMesh::GetTestCount() / 1000.f);
//        ImGui::Text("Triangle Hit/Test percent: %.4f %%", TriMesh::GetHitCount() / float(TriMesh::GetTestCount()) * 100);
        ImGui::End();
    }

    // Rendering
    glViewport(0, 0, (int)ImGui::GetIO().DisplaySize.x, (int)ImGui::GetIO().DisplaySize.y);
    const Vec3 clear_color {0.2, 0.2, 0.2};
    glClearColor(clear_color.x, clear_color.y, clear_color.z, 1);
    glClear(GL_COLOR_BUFFER_BIT);
    ImGui::Render();
    SDL_GL_SwapWindow(window);
}

void GUI::showRendererSelection(bool cpp) {
    if (ImGui::RadioButton("C++", cpp)) {
        SDL_Event event;
        event.type = SDL_USEREVENT;
        event.user.code = BaseRenderer::EVENT_CPP_RENDERER;
        SDL_PushEvent(&event);
        has_changed = true;
    }
    if (ImGui::RadioButton("OpenCL", !cpp)) {
        SDL_Event event;
        event.type = SDL_USEREVENT;
        event.user.code = BaseRenderer::EVENT_CL_RENDERER;
        SDL_PushEvent(&event);
        has_changed = true;
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
            has_changed = true;
        }
        ImGui::TreePop();
    }
}

void GUI::showLightingSettings() {
    if (ImGui::CollapsingHeader("Lighting", nullptr, true, true)) {
        bool lambertian = (bool) (options->brdf_bitfield & LAMBERTIAN);
        bool microfacet = (bool) (options->brdf_bitfield & MICROFACET);
        if (ImGui::Checkbox("Lambertian BRDF", &lambertian)) {
            options->brdf_bitfield ^= LAMBERTIAN;
            has_changed = true;
        }
        if (ImGui::Checkbox("Microfacet BRDF", &microfacet)) {
            options->brdf_bitfield ^= MICROFACET;
            has_changed = true;
        }
        has_changed |= ImGui::Checkbox("Tonemapping", &options->use_tonemapping);
        has_changed |= ImGui::Checkbox("Emissive lighting", &options->use_emissive_lighting);
        has_changed |= ImGui::Checkbox("Distant Environnment lighting", &options->use_distant_env_lighting);
        has_changed |= ImGui::Checkbox("Debug", &renderer->debug);
        if (ImGui::SliderInt("Depth target", &options->depth_target, 0, 30, "%.0f")) {
            has_changed = true;
        }
    }
}

//TODO: Maybe have a member or friend function on objects that present and set the state directly ?
void GUI::showObjectSettings() {
    if (ImGui::CollapsingHeader("Object settings", nullptr, true, true)) {

        Object3D* object = renderer->GetSelectedObject();
        if (object == nullptr)
            return;

        showMaterialSettings(object);

//        Vec3 origin = object->origin;
//        if (ImGui::SliderFloat3("Object position", &origin.x, -30.0f, 30.0f)) {
//            object->origin = origin;
//            has_changed = true;
//        }

        Plane* p = dynamic_cast<Plane*>(object->shape);
        if (p != nullptr) {
            Vec3 normal = p->normal;
            if (ImGui::SliderFloat3("Plane normal", &normal.x, -1.0f, 1.0f)) {
                normal = normal.normalize();
                p->normal = normal;
                has_changed = true;
            }
        }
    }
}

void GUI::showMaterialSettings(Object3D* object) {

    if (object->getEmissionIntensity() != -1) {

        Vec3 color = object->getEmissionColor().pow(1.f / 2.2f); // Linear to sRGB
        float intensity = object->getEmissionIntensity();

        if (ImGui::ColorEdit3("Light color", &color.x)) {
            object->setEmissionColor(color.pow(2.2f)); // sRGB to Linear
            material_has_changed = true;
        }
        if (ImGui::SliderFloat("Light intensity", &intensity, 0, 100, "%.3f", 2)) {
            object->setEmissionIntensity(intensity);
            material_has_changed = true;
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

void GUI::showTextureSettings(std::shared_ptr<ITexture> texture, const char* texture_name) {

    ValueTex3f* scalar_tex = dynamic_cast<ValueTex3f*>(texture.get());

    if (scalar_tex != nullptr) {

        // Our renderers consider colors to be in Linear space and convert them to sRGB at the end (gamma correction)
        // ImGui considers colors to be already in sRGB because it doesn't apply gamma correction
        // To keep color consistent, we simply convert them to sRGB before sending them to ImGui and convert them back after

        Vec3 color_value = scalar_tex->value.pow(1.f / 2.2f); // Linear to sRGB

        if (ImGui::ColorEdit3(texture_name, &color_value.x)) {
            scalar_tex->value = color_value.pow(2.2f); // sRGB to Linear
            material_has_changed = true;
        }
        return;
    }

    ValueTex1f* scalar = dynamic_cast<ValueTex1f*>(texture.get());

    if (scalar != nullptr) {

        // Our renderers consider colors to be in Linear space and convert them to sRGB at the end (gamma correction)
        // ImGui considers colors to be already in sRGB because it doesn't apply gamma correction
        // To keep color consistent, we simply convert them to sRGB before sending them to ImGui and convert them back after

        float value = std::pow(scalar->value, 1.f / 2.2f); // Linear to sRGB

        if (ImGui::SliderFloat(texture_name, &value, 0.001f, 1.0f)) {
            scalar->value = std::pow(value, 2.2f); // sRGB to Linear
            material_has_changed = true;
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

/*
 //FIXME Probably dead code that can be removed
void GUI::showBrdfStackSettings() {

    Object3D* object = renderer->GetScene()->objects.back().get();
    BrdfStack* brdf_stack = object->material->GetBrdfStack();
    Brdf* const* brdf_array = brdf_stack->getBrdfArray();

    Lambertian* lambertian = nullptr;
    CookTorrance* cookTorrance = nullptr;
    Mirror* mirror = nullptr;

    for (int i = 0; i < brdf_stack->brdf_count; ++i) {
        if (brdf_array[i]->type == LAMBERTIAN)
            lambertian = static_cast<Lambertian*>(brdf_array[i]);
        else if (brdf_array[i]->type == MICROFACET)
            cookTorrance = static_cast<CookTorrance*>(brdf_array[i]);
        else if (brdf_array[i]->type == MIRROR)
            mirror = static_cast<Mirror*>(brdf_array[i]);

    }
//        Lambertian* lambertian = (Lambertian*) p->brdf_stack->getBrdfArray()[0];
//        CookTorrance* cookTorrance = (CookTorrance*) p->brdf_stack->getBrdfArray()[1];
    if (mirror != nullptr) {
        float reflectance = mirror->getReflectance();
        if (ImGui::SliderFloat("Reflectance", &reflectance, 0, 1)) {
            mirror->setReflectance(reflectance);
            has_changed = true;
        }
    }
    if (lambertian != nullptr) {
        Vec3 plane_color = lambertian->getAlbedo();
        if (ImGui::ColorEdit3("Albedo", &plane_color.x)) {
            lambertian->setAlbedo(plane_color);
            has_changed = true;
        }
    }
    if (cookTorrance != nullptr) {
        float roughness = cookTorrance->getRoughness();
        Vec3 reflectance = cookTorrance->getReflection();
        if (ImGui::SliderFloat("Roughness", &roughness, 0.0f, 1.0f)) {
            cookTorrance->setRawRoughness(roughness);
            has_changed = true;
        }
        if (ImGui::SliderFloat("Reflectance", &reflectance.x, 0.0f, 1.0f)) {
            cookTorrance->setRawReflectance(reflectance.x);
            has_changed = true;
        }
    }
}*/

static void ShowBVHTree(bool* opened, const BVH& bvh)
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


