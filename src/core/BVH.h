#ifndef PATHTRACER_BVH_H
#define PATHTRACER_BVH_H

#include "math/Vec3.h"
#include "objects/BoundingBox.h"
#include "Ray.h"
#include "BVHCommons.h"

#include <queue>

#include <vector>
#include <memory>

typedef struct Object3D Object3D;
typedef struct Scene Scene;

class BVH {

    std::unique_ptr<Node> root;

    struct QueueElement
    {
        const Node *node; // octree node held by this node in the tree
        float t; // used as key
        QueueElement() = default;
        QueueElement(const Node *node, float t) : node(node), t(t) {}
        // comparator is > instead of < so priority_queue behaves like a min-heap
        friend bool operator < (const QueueElement &a, const QueueElement &b) { return a.t > b.t; }
    };

    // To avoid high de/alloc counts, we subclass the queue to access its container and reserve it
    class MyPQueue : public std::priority_queue<BVH::QueueElement>
    {
    public:
        MyPQueue(size_t reserve_size)
        {
            this->c.reserve(reserve_size);
        }
    };

public:

    BVH() = default;

    BVH(const Scene* scene);

    void Insert(Object3D* object, const std::unique_ptr<Node>& node, short depth = 0);

    bool FindNearestIntersection(const Ray& ray, float& dist_out, Object3D*& hit_object);

    static int ray_bbox_test_count;
    static int ray_bbox_hit_count;
    static int ray_obj_test_count;
    static int ray_obj_hit_count;

    static void ResetCounters();

    bool DebugIntersection(const Ray& ray, float& dist_out, Object3D*& hit_object, int depth_target);

    Node* GetRoot() const {
        return root.get();
    }

    int GetNodeCount() const {
        return node_count;
    }

    static BoundingBox GetChildOctreeBoundingBox(BoundingBox bbox, Vec3 point);

private:

    void AddChildNode(const std::unique_ptr<Node>& node, Object3D* object);

    void ComputeBoundingBoxes(const std::unique_ptr<Node>& node);

    bool IntersectNode(const FastRay& ray, float& dist_out, Object3D*& hit_object, const std::unique_ptr<Node>& node);

    bool IntersectNodeFast(const FastRay& ray, float& t_near_candidate, Object3D*& hit_object, const std::unique_ptr<Node>& node);

    bool DebugIntersectNode(const FastRay& ray, float& dist_out, Object3D*& hit_object, int depth, int debug_depth, const std::unique_ptr<Node>& node);

    short max_depth = 0;
    int node_count = 0;

};

#endif //PATHTRACER_BVH_H
