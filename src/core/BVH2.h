#ifndef PATHTRACER_BVH2_H
#define PATHTRACER_BVH2_H

#include "math/Vec3.h"
#include "objects/BoundingBox.h"
#include "Ray.h"
#include "BVHCommons.h"

#include <queue>
#include <vector>
#include <memory>

typedef struct Object3D Object3D;
typedef struct Scene Scene;

class BVH2 {

    std::unique_ptr<Node2> root;
    std::vector<std::unique_ptr<Object3D>> _objects;

    struct QueueElement
    {
        const Node2 *node; // octree node held by this node in the tree
        float t; // used as key
        QueueElement() = default;
        QueueElement(const Node2 *node, float t) : node(node), t(t) {}
        // comparator is > instead of < so priority_queue behaves like a min-heap
        friend bool operator < (const QueueElement &a, const QueueElement &b) { return a.t > b.t; }
    };

    // To avoid high de/alloc counts, we subclass the queue to access its container and reserve it
    class MyPQueue : public std::priority_queue<BVH2::QueueElement>
    {
    public:
        MyPQueue(size_t reserve_size)
        {
            this->c.reserve(reserve_size);
        }
    };

public:

    BVH2() = default;

    BVH2(const Scene* scene);

    bool FindNearestIntersection(const Ray& ray, float& dist_out, Object3D*& hit_object);

    bool FindNearestIntersectionOpti(const Ray& ray, float& dist_out, Object3D*& hit_object);

    static int ray_bbox_test_count;
    static int ray_bbox_hit_count;
    static int ray_obj_test_count;
    static int ray_obj_hit_count;

    static void ResetCounters();

    bool DebugIntersection(const Ray& ray, float& dist_out, Object3D*& hit_object, int depth_target);

    Node2* GetRoot() const {
        return root.get();
    }

    int GetNodeCount() const {
        return node_count;
    }

private:

    bool IntersectNode(const FastRay& ray, float& dist_out, Object3D*& hit_object, const std::unique_ptr<Node2>& node);

    bool IntersectNodeFast(const FastRay& ray, float& t_near_candidate, Object3D*& hit_object, const std::unique_ptr<Node2>& node);

    bool DebugIntersectNode(const FastRay& ray, float& dist_out, Object3D*& hit_object, int depth, int debug_depth, const std::unique_ptr<Node2>& node);

    short max_depth = 0;
    int node_count = 0;

    Node2* RecursiveBuild(int first, int last, int depth = 0);

    bool IntersectNodeOpti(const FastRay& ray, float& dist_out, Object3D*& hit_object, const std::unique_ptr<Node2>& node, const char direction_sign);
};

#endif //PATHTRACER_BVH2_H
