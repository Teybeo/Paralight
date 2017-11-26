#include "BVH2.h"

#include "Scene.h"
#include "objects/TriMesh.h"

#include <SDL_timer.h>
#include <algorithm>
#include "app/Chronometer.h"

int BVH2::ray_bbox_test_count;
int BVH2::ray_bbox_hit_count;
int BVH2::ray_obj_test_count;
int BVH2::ray_obj_hit_count;

using std::cout;
using std::endl;
using std::vector;
using std::unique_ptr;

struct BuildInfo {
    BoundingBox bbox;
    Object3D* object;
    Vec3 bbox_center;
};

int SplitSAH      (vector<BuildInfo>& objects, int first, int last, BoundingBox box);
int SplitAtPoint  (vector<BuildInfo>& objects, int first, int last, char axis, float split_point);
int SplitEqualSets(vector<BuildInfo>& objects, int first, int last, char axis);

BoundingBox GetBoundingBoxForObjects(const vector<BuildInfo>& objects, int i, int i1);

void ComputeBoundingBoxes(const unique_ptr<Node2>& node);

#define SAH
//#define MID_POINT

BVH2::BVH2(const Scene* scene) {

    std::vector<BuildInfo> build_info_array;

    build_info_array.reserve(scene->objects.size());

    for (const auto& object : scene->objects) {
        build_info_array.push_back( BuildInfo { object->ComputeBBox(), object.get(), object->ComputeBBox().GetCenter() });
    }
    
    Chronometer chrono;

    cout << "Building BVH..." << endl;

    root = unique_ptr<Node2>(RecursiveBuild(build_info_array, 0, (int) scene->objects.size()));

    cout << "BVH built in " << chrono.GetSeconds() << " s" << endl;
    
//    ComputeBoundingBoxes(root);

    cout << "BVH2 max depth: " << max_depth << endl;
    cout << "BVH2 node count: " << node_count << endl;
}

/**
 * Split all objects in 2 sets separated by the provided split_point along the axis
 */
int SplitAtPoint(vector<unique_ptr<Object3D>>& objects, int first, int last, char axis, float split_point) ;

Node2* BVH2::RecursiveBuild(vector<BuildInfo>& objects, int first, int last, int depth) {

    max_depth = std::max(max_depth, short(depth));

    Node2* node = new Node2;
    node_count++;

    int obj_count = last - first;
    if (obj_count == 1) {
        node->object = objects[first].object;
        node->bbox = objects[first].bbox;
    }
    else {

        BoundingBox bbox = GetBoundingBoxForObjects(objects, first, last);
        if (bbox == BoundingBox()) {
            node->object = objects[first].object;
            node->bbox = objects[first].bbox;
            return node;
        }

        char axis = bbox.GetLargestAxis();
        int middle;

#if MID_POINT
        middle = SplitAtPoint(objects, first, last, axis, bbox.GetCenter()[axis]);
#elif defined EQUAL
        middle = SplitEqualSets(objects, first, last, axis);
#elif defined SAH
        middle = SplitSAH(objects, first, last, bbox);
#endif

        node->bbox = bbox;
        node->split_axis = axis;
        node->left_child = std::unique_ptr<Node2>(RecursiveBuild(objects, first, middle, depth + 1));
        node->right_child = std::unique_ptr<Node2>(RecursiveBuild(objects, middle, last, depth + 1));
    }

    return node;
}
/*
 * Compute the surface area of the bbox
 * Subdivide the largest axis at N split points, creating N + 1 sub-bboxes
 * For each side of each split,
 *      get every object whose bbox center falls inside the sub-bbox and compute their enclosing bbox
 *      compute the ratio of the parent bbox and this sub-bbox SA
 * Find the split for which the sum of each side SA ratio is minimal
 */
int SplitSAH(vector<BuildInfo>& objects, int first, int last, BoundingBox box) {

    char axis = box.GetLargestAxis();

    const int split_count = 20;
    std::array<float, split_count> split_point_array;

    float start = box.min[axis];
    float end = box.max[axis];

    float step = std::abs(start - end) / (split_count + 1);

    for (int i = 0; i < split_count; ++i) {
        split_point_array[i] = start + step * (i + 1);
    }

    float parent_surface_area = box.GetSurfaceArea();

    std::array<float, split_count> sah_costs;

    for (int i = 0; i < split_count; ++i) {
        BoundingBox left;
        BoundingBox right;
        int left_count = 0;
        int right_count = 0;
        for (int j = first; j < last; ++j) {
            const BuildInfo& object = objects[j];
            float obj_center = object.bbox_center[axis];
//            float obj_center = object->GetCenter()[axis];
            if (obj_center <= split_point_array[i]) {
                left.ExtendsBy(object.bbox);
                left_count++;
            }
            else {
                right.ExtendsBy(object.bbox);
                right_count++;
            }
        }
        sah_costs[i] = ((left_count * left.GetSurfaceArea()) + (right_count * right.GetSurfaceArea())) / parent_surface_area;
    }

    int min_cost_index = 0;

    for (int i = 0; i < split_count; ++i) {
        if (sah_costs[i] < sah_costs[min_cost_index]) {
            min_cost_index = i;
        }
    }

    float split_point = split_point_array[min_cost_index];

    return SplitAtPoint(objects, first, last, axis, split_point);
}

/**
 * Split all objects in 2 sets separated by the provided split_point along the axis
 */
int SplitAtPoint(vector<BuildInfo>& objects, int first, int last, char axis, float split_point) {

    // Split all objects in 2 sets separated by the middle point of the bbox largest axis
    const auto& middle_it = std::stable_partition(&objects[first], &objects[last], [=] (BuildInfo& i) {
//        return i->GetCenter()[axis] < split_point;
        return i.bbox_center[axis] < split_point;
    });

    int middle = static_cast<int>(middle_it - &objects.front());

    if (middle == first || middle == last) {
        middle = SplitEqualSets(objects, first, last, axis);
    }

    return middle;
}
/**
 * Slit all objects in 2 sets of equal sizes
 */
int SplitEqualSets(vector<BuildInfo>& objects, int first, int last, char axis) {
    // Slit all objects in 2 sets of equal sizes
    int middle = (last + first) / 2;
    std::nth_element(&objects[first], &objects[middle], &objects[last], [=] (BuildInfo& a, BuildInfo& b) {
        return a.object->GetCenter()[axis] < b.object->GetCenter()[axis];
    });

    return middle;
}

BoundingBox GetBoundingBoxForObjects(const vector<BuildInfo>& objects, int start, int end) {

    BoundingBox bbox;
    for (int i = start; i < end; ++i) {
        bbox.ExtendsBy(objects[i].bbox);
    }
    return bbox;
}

bool BVH2::FindNearestIntersection(const Ray& ray, float& dist_out, Object3D*& hit_object) {

    FastRay fast_ray {ray};

    return IntersectNode(fast_ray, dist_out, hit_object, root);
//    return IntersectNodeFast(fast_ray, dist_out, hit_object, root);

}

bool BVH2::FindNearestIntersectionOpti(const Ray& ray, float& dist_out, Object3D*& hit_object) {

    const FastRay fast_ray {ray};

    char direction_sign = static_cast<char>((ray.direction.x > 0) + 2 * (ray.direction.y > 0) + 4 * (ray.direction.z > 0));

    return IntersectNodeOpti(fast_ray, dist_out, hit_object, root, direction_sign);
//    return IntersectNodeFast(fast_ray, dist_out, hit_object, root);

}

bool BVH2::IntersectNodeOpti(const FastRay& ray, float& dist_out, Object3D*& hit_object, const unique_ptr<Node2>& node, char direction_sign) {

    float dist;
//    ray_bbox_test_count++;
    // We intersect this node
//    if (node->bbox.IntersectBranched(ray, dist) && (dist < dist_out)) {
    if (node->bbox.IntersectFast(ray, dist) && (dist < dist_out)) {
//        ray_bbox_hit_count++;

        // If no children, this node is a leaf so intersect the object it contains
        if (node->object != nullptr) {
//            ray_obj_test_count++;
            if (node->object->Intersect(ray, dist) && (dist < dist_out)) {
//                ray_obj_hit_count++;
                dist_out = dist;
                hit_object = node->object;
            }
        }
        else {
            if (direction_sign & (1 << node->split_axis)) {
                IntersectNodeOpti(ray, dist_out, hit_object, node->left_child, direction_sign);
                IntersectNodeOpti(ray, dist_out, hit_object, node->right_child, direction_sign);
            }
            else {
                IntersectNodeOpti(ray, dist_out, hit_object, node->right_child, direction_sign);
                IntersectNodeOpti(ray, dist_out, hit_object, node->left_child, direction_sign);
            }
        }

    }

    return (hit_object != nullptr);
}

bool BVH2::IntersectNode(const FastRay& ray, float& dist_out, Object3D*& hit_object, const unique_ptr<Node2>& node) {

    float dist;
//    ray_bbox_test_count++;
    // We intersect this node
    if (node->bbox.IntersectBranched(ray, dist) && (dist < dist_out)) {
//        ray_bbox_hit_count++;

        // If no children, this node is a leaf so intersect the object it contains
        if (node->object != nullptr) {
//            ray_obj_test_count++;
            if (node->object->Intersect(ray, dist) && (dist < dist_out)) {
//                ray_obj_hit_count++;
                dist_out = dist;
                hit_object = node->object;
            }
        }
        else {
            IntersectNode(ray, dist_out, hit_object, node->left_child);
            IntersectNode(ray, dist_out, hit_object, node->right_child);
        }

    }

    return (hit_object != nullptr);
}

void BVH2::ResetCounters() {
    ray_bbox_test_count = 0;
    ray_bbox_hit_count = 0;
    ray_obj_test_count = 0;
    ray_obj_hit_count = 0;
}

bool BVH2::DebugIntersection(const Ray& ray, float& dist_out, Object3D*& hit_object, int depth_target) {
    FastRay fast_ray {ray};

    return DebugIntersectNode(ray, dist_out, hit_object, 0, depth_target, root);
}

bool BVH2::DebugIntersectNode(const FastRay& ray, float& dist_out, Object3D*& hit_object, int depth, int debug_depth, const unique_ptr<Node2>& node) {

    float dist = 99999999.f;
    // We intersect this node
    if (node->bbox.IntersectBranched(ray, dist) && (dist < dist_out)) {

        // If no children, this node is a leaf so intersect the object it contains
        if (depth == debug_depth) {
            dist_out = dist;
//            hit_object = &(node->bbox); //FIXME: if needed make bbox visualisation work again
        }
        else {
            DebugIntersectNode(ray, dist_out, hit_object, depth + 1, debug_depth, node->left_child);
            DebugIntersectNode(ray, dist_out, hit_object, depth + 1, debug_depth, node->right_child);
        }

    }

    return (hit_object != nullptr);
}

void ComputeBoundingBoxes(const unique_ptr<Node2>& node) {

    if (node->object != nullptr)
//        node->bbox = node->bbox.ExtendsBy(node->object->ComputeBBox());
        node->bbox = node->object->ComputeBBox();
    else {
        ComputeBoundingBoxes(node->left_child);
        ComputeBoundingBoxes(node->right_child);
        node->bbox = node->bbox.ExtendsBy(node->left_child->bbox);
        node->bbox = node->bbox.ExtendsBy(node->right_child->bbox);
    }
}