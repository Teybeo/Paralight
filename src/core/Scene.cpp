#include "Scene.h"

#include "objects/TriMesh.h"
#include "objects/Sphere.h"
#include "objects/Plane.h"
#include "objects/Triangle.h"
#include "objects/BoundingBox.h"

#include <algorithm>
#include <map>
#include <set>

using std::unique_ptr;
using std::shared_ptr;
using std::vector;
using std::set;
using std::string;

Scene::Scene(string model_file) {

    string env_dir = "../../envmaps/";

//    std::string env = "Ditch-River_2k.hdr";
//    std::string env = "pinetree_spheremap.hdr";
//    std::string env = "Sunset at pier.jpg";
//    std::string env = "SnowPano_4k_Ref.hdr";
//    std::string env = "Sky_Clear_1K.hdr";
    std::string env = "panorama_map.hdr";
//    std::string env = "st_nicolaus_church_interior.hdr";
//    std::string env = "Sponza.hdr";

    env_map = std::unique_ptr<TextureFloat>( new TextureFloat {env_dir + env});

//    std::string file = "blender_tests/textured_square.obj";
//    std::string file = "blender_tests/cube.obj";
//    std::string file = "blender_tests/bvh_tests_2.obj";
//    std::string file = "blender_tests/textured_square.obj";
//    std::string file = "blender_tests/multi_mesh.obj";
//    std::string file = "blender_tests/suzanne_low_poly_500.obj";
//    std::string file = "dragon/dragon_low_poly_1k.obj";
//    std::string file = "dragon/dragon.obj";
//    std::string file = "blender_tests/bvh_sponza.obj";
//    std::string file = "blender_tests/crytek_sponza_cl.obj";
//    std::string file = "crytek_sponza/crytek_vase_fixed.obj";
//    std::string file = "marko_sponza/objects/sponza.lwo";
//    std::string file = "sibenik_cathedral/sibenik.obj";
//    std::string file = "blender_tests/sibenik_cl.obj";
//    std::string file = "mechanical-emperor-scorpion/converted/scorpion.obj";
//    std::string file = "blender_tests/carobj.obj";
//    std::string file = "sports_car/sports_car2.obj";
//    std::string file = "sports_car/sports_car.obj";
//    std::string file = "san-miguel 2016 version/sanMiguel/sanMiguel.obj";
//    std::string file = "san_miguel/san-miguel.obj";
//    std::string file = "Shanghai_city/shanghai_city_scene.obj";
    std::string file = "tibet_house/house.obj";
//    std::string file = "ue4 stuff/grux.obj";
//    std::string file = "Array_House_Example/Array House Example_obj.obj";

    if (model_file == "")
        model_file = prefix + file;

    LoadObjects(model_file);
}

void Scene::LoadObjects(const string& file) {

//    Load_CornellBox();
//    Load_SphereGrid(4);
//    Load_MirrorRoom();
//    Load_TexturedSphere();
//    LoadSomeLights();
    LoadModel(file);

    bvh2 = new BVH2 {this};

//    exit(0);

//    bvh = BVH {this};

    PostProcess();
}


void Scene::Clear() {
    delete bvh2;
    set<const TriMesh*> trimeshes = GetTriMeshes();
    for (const auto& trimesh : trimeshes) {
        delete trimesh;
    }
    objects.clear();
    material_set.clear();
}

Scene::~Scene() {
    delete bvh2;
//    GetTriMeshes().clear();
}

void Scene::LoadModel(const string& file) {

    std::vector<std::unique_ptr<Object3D>> triangles = Object3D::CreateTriMesh(file);
    std::move(triangles.begin(), triangles.end(), std::back_inserter(objects));
}

void Scene::LoadSomeLights() {
    cam_pos = {0, 1, 2};

    cam_pos = {1.56117, 1.30991, 1.9523};
    xz_angle = -0.92f;
    yz_angle = 0.24f;

    std::array<Vec3, 5> pos = {
           Vec3 {0, 15, 0},
           Vec3 {7, 15, 0},
           Vec3 {14, 15, 0},
           Vec3 {-7, 15, 0},
           Vec3 {-14, 15, 0},
    };

    for (size_t i = 0; i < pos.size(); ++i) {
        Object3D* light = Object3D::CreateSphere(pos[i].x, pos[i].y, pos[i].z, 2);
        light->material = new Standard {0.4f, 0.1f, 0.1f};
        light->setEmission(10);
        objects.push_back(unique_ptr<Object3D>(light));
    }

}

void Scene::Load_CornellBox() {

    cam_pos = {0, 0, 20};
    xz_angle = 0;
    yz_angle = 0;

    Vec3 yellow {0.9, 0.6, 0.3};
    Vec3 red    {0.8, 0.1, 0};
    Vec3 green  {0.15, 0.4, 0.1};

    Object3D* a = Object3D::CreateSphere(-1, -2, 0);
    Object3D* b = Object3D::CreateSphere(1, -2, 2.5f);
    a->material = new Standard(yellow, 0.7, 0.2);
    b->material = new Standard(yellow, 0.5, 0.02);
    objects.push_back(unique_ptr<Object3D>(a));
    objects.push_back(unique_ptr<Object3D>(b));

    Object3D* light = Object3D::CreateSphere(0, 12.9, 2, 10);
    light->setEmission(10);
    objects.push_back(unique_ptr<Object3D>(light));

    Object3D* left   = Object3D::CreatePlane(Vec3{-3, 0, 0}, Vec3{1, 0, 0});
    Object3D* right  = Object3D::CreatePlane(Vec3{ 3, 0, 0}, Vec3{-1, 0, 0});
    Object3D* top    = Object3D::CreatePlane(Vec3{0, 3, 0}, Vec3{0, -1, 0});
    Object3D* bottom = Object3D::CreatePlane(Vec3{0, -3, 0}, Vec3{0, 1, 0});
    Object3D* back   = Object3D::CreatePlane(Vec3{0, 0, -1}, Vec3{0, 0, 1});
    left->material   = new LambertianMaterial(red);
    left->material   = new LambertianMaterial(red);
    right->material  = new LambertianMaterial(green);
    top->material    = new LambertianMaterial(yellow);
    back->material   = new LambertianMaterial(yellow);
    bottom->material   = new Standard(yellow, 0.4, 0.1);
    objects.push_back(unique_ptr<Object3D>(left));
    objects.push_back(unique_ptr<Object3D>(right));
    objects.push_back(unique_ptr<Object3D>(top));
    objects.push_back(unique_ptr<Object3D>(back));
    objects.push_back(unique_ptr<Object3D>(bottom));

}

void Scene::Load_SphereGrid(int nb) {

//    cam_pos = {0, 5, 8};
    cam_pos = {0, 15, 24};
    yz_angle = 0.63;

    for (int x = 0; x < nb; ++x) {
        for (int y = 0; y < nb; ++y) {
            for (int z = 0; z < nb; ++z) {
                float pos_x = ((-nb / 2) + 1.3f*x) * 2;
                float pos_z = ((-nb / 2) + 1.3f*y) * 2;
                float pos_y = ((-nb / 2) + 1.3f*z) * 2;
//            float pos_x = ( (rand() % 2000) / 1000.f) ;
//            float pos_y = (rand() % 20) ;
//            float pos_z = (rand() % 20) ;
                Object3D* s = Object3D::CreateSphere(pos_x, pos_y, pos_z);
//            s->setRadius(0.5f);
//            s->spec = new CookTorrance {{float(y)/nb}, float(x)/nb};
//            s->spec = new CookTorrance {{float(y)/nb}, float(x)/nb};
                Vec3 color{(std::rand() % 1000) / 1000.f, (std::rand() % 1000) / 1000.f, (std::rand() % 1000) / 1000.f};
//              Vec3 color {1};
//            s->setEmission(color * 50 * (rand() % 20 == 0 ));
//            s->brdf = new Lambertian(color);
//            s->brdf_stack = new StandardStack (color, 1, float(x)/nb);
//            s->brdf_stack = new BrdfStack(new Lambertian(color));
                s->material = new Standard(color, float(y) / nb, float(x) / nb);
//            s->material = new LambertianMaterial {color};
//            s->brdf_stack = new StandardStack (color, {1}, 0);
                objects.push_back(unique_ptr<Object3D>(s));
            }
        }
    }

//    std::random_shuffle(objects.begin(), objects.end());

    Object3D* light = Object3D::CreateSphere(0, 20, 0, 10);
    light->setEmission(5);
//    objects.push_back(unique_ptr<Object3D>(light));

    Object3D* test = Object3D::CreateSphere(0, 20, 0);
    test->material = objects[0]->material;
//    objects.push_back(unique_ptr<Object3D>(test));

    Object3D* egzeg = Object3D::CreateSphere(20, 0, 0);
    egzeg->material = new Standard(0, 0.5, 0.5);
//    objects.push_back(unique_ptr<Object3D>(egzeg));
}

void Scene::Load_MirrorRoom() {

    Object3D* light = Object3D::CreateSphere(0, 100, 0, 85);
    light->setEmission(10);
    objects.push_back(unique_ptr<Object3D>(light));

    int nb = 4;
    for (int x = 0; x < nb; ++x) {
        for (int y = 0; y < nb; ++y) {
            float pos_x = ((-nb / 2) + x) * 8;
            float pos_y = (rand() % 10) ;
            float pos_z = ((-nb/2) + y) * 8;

            Object3D* s = Object3D::CreateSphere(pos_x, pos_y, pos_z, 1 + rand() % 5);
            Vec3 color {(std::rand() % 1000) / 1000.f, (std::rand() % 1000) / 1000.f, (std::rand() % 1000) / 1000.f};
//
//            s->setEmission(color * 50 * (rand() % 20 == 0 ));
//            s->brdf_stack = new StandardStack (color, 1, float(x)/nb);
//            s->brdf_stack = new BrdfStack(new Lambertian(color));
            s->material = new Standard (color, float(y)/nb, float(x)/nb);
//            s->brdf_stack = new StandardStack (color, {1}, 0);
            objects.push_back(unique_ptr<Object3D>(s));
        }
    }
    Object3D* left   = Object3D::CreatePlane(Vec3{-40, 0, 0}, Vec3{1, 0, 0});
    Object3D* right  = Object3D::CreatePlane(Vec3{ 40, 0, 0}, Vec3{-1, 0, 0});
    Object3D* top    = Object3D::CreatePlane(Vec3{0, 20, 0}, Vec3{0, -1, 0});
    Object3D* bottom = Object3D::CreatePlane(Vec3{0, -20, 0}, Vec3{0, 1, 0});
    Object3D* back   = Object3D::CreatePlane(Vec3{0, 0, -40}, Vec3{0, 0, 1});
    Object3D* front   = Object3D::CreatePlane(Vec3{0, 0, 40}, Vec3{0, 0, 1});
//    BrdfStack* mat = new StandardStack (0.4, 0.4, 0.1);
    Material* mat = new Standard({0.5}, 0.2, 0.2);
    left->material   = mat;
    left->material   = mat;
    right->material  = mat;
    top->material    = mat;
    back->material   = mat;
    front->material   = mat;
    bottom->material = new LambertianMaterial(Vec3(0.8f, 0.1, .4));
//    objects.push_back(unique_ptr<Object3D>(left));
//    objects.push_back(unique_ptr<Object3D>(right));
//    objects.push_back(unique_ptr<Object3D>(top));
//    objects.push_back(unique_ptr<Object3D>(back));
//    objects.push_back(unique_ptr<Object3D>(bottom));
//    objects.push_back(unique_ptr<Object3D>(front));
}

void Scene::Load_Floor() {
    Object3D* plane = Object3D::CreatePlane(Vec3(0, 0, 0), Vec3(0, 1, 0));
//    Object3D* plane = Object3D::CreatePlane(Vec3(0.66, 0.66, 0), Vec3(0, 0, 1));
//    plane->brdf = new Mirror;
//    plane->brdf = new Lambertian(1);
//    plane->spec = new CookTorrance({0.2f}, 0.4);
    plane->material = new Standard (1, 0.1f, 0.4);
//    plane->brdf_stack = new BrdfStack(new Lambertian(1));
//    plane->brdf_stack = new MicrofacetMaterial {0.2f, 0.4f};
    objects.push_back(unique_ptr<Object3D>(plane));
}

void Scene::Load_TexturedSphere() {
    cam_pos = {0, 0, 3};
    xz_angle = 0;
    yz_angle = 0;

    Object3D* light = Object3D::CreateSphere(0, 20, 0, 10);
    light->setEmission(5);
    objects.push_back(unique_ptr<Object3D>(light));

    string prefix = "D:/Media/Textures/";
    string rel_prefix = "../../images/";

    std::string albedo_file = "../../imagesold/T_Brick_Beige_D_higher_freq.jpg";
//    std::string albedo_file = "Dark Stone Tiles/Dark_Stone_Tiles_Normal.jpg";
//    std::string albedo_file = "Dark Stone Tiles/Dark_Stone_Tiles_Base_Color.jpg";
//    std::string albedo_file = rel_prefix + "uv_checker.png";
    shared_ptr<Texture> albedo_tex = std::make_shared<TextureUbyte>(prefix + albedo_file);
//    shared_ptr<Texture> albedo_tex = std::make_shared<ValueTex3f>(0.7f);

//    std::string roughness_file = "T_Brick_Beige_R.jpg";
    std::string roughness_file = "Dark Stone Tiles/Dark_Stone_Tiles_Roughness.jpg";
//    std::string roughness_file = "../../imagesold/T_Brick_Beige_R_boosted_higher_freq_blured.jpg";
    shared_ptr<Texture> roughness_tex = std::make_shared<TextureUbyte>(prefix + roughness_file);
//    shared_ptr<Texture> roughness_tex = std::make_shared<ValueTex1f>(0.2f);

//    std::string reflectance_file = "../../imagesold/127_pixel.bmp";
    std::string reflectance_file = "Dark Stone Tiles/Dark_Stone_Tiles_Metallic.jpg";
    shared_ptr<Texture> reflectance_tex = std::make_shared<TextureUbyte>(prefix + reflectance_file);
//    shared_ptr<Texture> reflectance_tex = std::make_shared<ValueTex3f>(0.01f);

//    std::string normal_file = "normal_up_1k_2x.jpg";
    std::string normal_file = "Dark Stone Tiles/Dark_Stone_Tiles_Normal.jpg";
    std::shared_ptr<Texture> normal_tex = std::make_shared<TextureUbyte>(prefix + normal_file, false);

    //TODO: Ideal api_low_poly

    Object3D* a = Object3D::CreateSphere(0, 0, 0);
//    Object3D* b = Object3D::CreateSphere(0, 0, -2);
//    Object3D* c = Object3D::CreateSphere(0, 0, -4);
//    Object3D* d = Object3D::CreateSphere(0, 0, -6);
//    Object3D* e = Object3D::CreateSphere(0, 0, -8);
//    Object3D* f = Object3D::CreateSphere(0, 0, -10);
//    Object3D* g = Object3D::CreateSphere(0, 0, -12);
    a->material = new Standard {albedo_tex, roughness_tex, reflectance_tex, normal_tex};
//    b->material = new Standard {albedo_tex, roughness_tex, reflectance_tex};

//    sphere->material = new Standard {{0.45, 0.03, 0.01}, 0.1f, 0.04f, normal_tex};
//    sphere->material = new Standard {1, 0.1f, 0.04f};
//    sphere->material = new LambertianMaterial {0.7f};
//    sphere->material = new LambertMaterial(albedo_map);
//    sphere->material = new MicrofacetMaterial(reflectance_map, roughness_map);
//    sphere->material = new OrenNayarMaterial(albedo_map, roughness_map);

//    objects.push_back(unique_ptr<Object3D>(g));
//    objects.push_back(unique_ptr<Object3D>(f));
//    objects.push_back(unique_ptr<Object3D>(e));
//    objects.push_back(unique_ptr<Object3D>(d));
//    objects.push_back(unique_ptr<Object3D>(c));
//    objects.push_back(unique_ptr<Object3D>(b));
    objects.push_back(unique_ptr<Object3D>(a));
//
//    objects.push_back(unique_ptr<Object3D>(a));
//    objects.push_back(unique_ptr<Object3D>(b));
//    objects.push_back(unique_ptr<Object3D>(c));
//    objects.push_back(unique_ptr<Object3D>(d));
//    objects.push_back(unique_ptr<Object3D>(e));
//    objects.push_back(unique_ptr<Object3D>(f));
//    objects.push_back(unique_ptr<Object3D>(g));

//    Load_Floor();
}

BoundingBox Scene::ComputeBBox() const {

    BoundingBox bbox;

    for (const auto& object : objects) {
        bbox.ExtendsBy(object->ComputeBBox());
    }

    return bbox;
}

set<const TriMesh*> Scene::GetTriMeshes() const {

    set<const TriMesh*> trimeshes;

    for (const auto& object : objects) {

        if (typeid(*object->shape) == typeid(Triangle)) {
            trimeshes.insert((static_cast<Triangle*>(object->shape))->trimesh_ptr);
        }
    }

    return trimeshes;
}

void Scene::CheckObjectsOrder() {

    if (objects.size() == 0)
        return;

    bool sphere_appeared = false;
    bool plane_appeared = false;
    bool triangle_appeared = false;

    std::array<int, 3> order = {-1, -1, -1};

    for (const auto& object : objects) {
        if (typeid(*object->shape) == typeid(Sphere)) {
            // First sphere
            if (sphere_appeared == false) {
                sphere_appeared = true;
                if (plane_appeared || triangle_appeared)
                    order[0] = 0;
                else
                    order[0] = 1;
            }
            else {
                if (plane_appeared || triangle_appeared)
                    order[0] = 0;
            }
        }
        else if (typeid(*object->shape) == typeid(Plane)) {
            if (plane_appeared == false) {
                plane_appeared = true;
                if (triangle_appeared)
                    order[1] = 0;
                else
                    order[1] = 1;
            }
            else {
                if (triangle_appeared)
                    order[1] = 0;
            }
        }
        else if (typeid(*object->shape) == typeid(Triangle)) {
            if (triangle_appeared == false) {
                triangle_appeared = true;
                order[2] = 1;
            }
        }
    }

    for (const auto& item : order) {
        if (item == 0) {
            std::cerr << "Incorrect object order for OpenCL !!" << endl;
        }
    }

}

void Scene::PostProcess() {

    Vec3 min = bvh2->GetRoot()->bbox.min;
    Vec3 max = bvh2->GetRoot()->bbox.max;
    debug_scale = std::sqrt((max * max).max() + (min * min).max());
    debug_scale = ((max - min) / 2.f).max();
    cout << "BVH scale " << debug_scale << endl;

    triangle_count = 0;
    for (const auto& object : objects) {
        if (typeid(*object->shape) == typeid(Triangle))
            triangle_count++;
    }

    vertex_count = 0;
    const auto& trimeshes = GetTriMeshes();
    for (const auto& trimesh : trimeshes) {
        vertex_count += trimesh->GetVertexCount();
    }

    for (const auto& object : objects) {
        material_set.insert(object->material);
    }

    cout << triangle_count << " triangles" << endl;
    cout << vertex_count << " vertices" << endl;
    cout << material_set.size() << " materials" << endl;

    CheckObjectsOrder();
}

