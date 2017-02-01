#include "BVH.h"

#include "Scene.h"
#include "objects/TriMesh.h"

int BVH::ray_bbox_test_count;
int BVH::ray_bbox_hit_count;
int BVH::ray_obj_test_count;
int BVH::ray_obj_hit_count;

using std::cout;
using std::endl;
using std::vector;

BVH::BVH(const Scene* scene) {

    BoundingBox bbox = scene->ComputeBBox();

    root = std::unique_ptr<Node>{new Node};
    root->bbox = bbox;
    node_count++;

    for (const auto& object : scene->objects) {
        Insert(object.get(), root);
    }

    ComputeBoundingBoxes(root);

    cout << "BVH max depth: " << max_depth << endl;
    cout << "BVH node count: " << node_count << endl;

}

void BVH::Insert(Object3D* object, const std::unique_ptr<Node>& node, short depth) {

    max_depth = std::max(depth, max_depth);

    if (depth > 100)
    {
        cout << "Depth: " << depth << endl;
        return;
    }
//    {
//        cout << "Center of object to insert: " << object->GetCenter() << endl;
//        cout << "Bbox of current node: min [" << node->bbox.min << "] - max [" << node->bbox.max << "]" << endl;
//        cout << endl;
//    }

    // This node has no children nodes
    if (node->children.empty())
    {
        // This node has no assigned object (only case is the root node)
        if (node->object == nullptr) {
            node->object = object;
        }
        // Node already has object, so we have to partition it into children nodes
        // Important: Not all 8 childs will be created here, only the necessary ones
        else {

            // Insert the object into a child
//            AddChildNode(node, object);
            Vec3 obj_center = object->GetCenter();
//            Vec3 obj_center = object->ComputeBBox().GetCenter();
            auto bbox = GetChildOctreeBoundingBox(node->bbox, obj_center);
            if (bbox.max == bbox.min) {
                cout << "tiny bbox" << endl;
                return;
            }
            node->children.emplace_back(new Node {bbox, object});

            // Reinsert the object into the bvh
            Insert(node->object, node, depth + 1);

            // Detach this node object
            node->object = nullptr;
        }
    }
    else {
//        cout << "Not a leaf, searching for existing enclosing child..." << endl;

        // Find the child which enclose this object center
        Vec3 obj_center = object->GetCenter();
//        Vec3 obj_center = object->ComputeBBox().GetCenter();
//        BoundingBox obj_bbox = object->ComputeBBox();

        auto blabla = GetChildOctreeBoundingBox(node->bbox, obj_center);
        if (blabla.min == blabla.max) {
            cout << "tiny bbox" << endl;
            return;
        }
        for (const auto& child : node->children) {
            if (child->bbox == blabla) {
                Insert(object, child, depth + 1);
                return;
            }
        }
        AddChildNode(node, object);
#if 0

        for (const auto& child : node->children) {
//            if (child->bbox.EnclosesInclusive(obj_bbox)) {
//            if (child->bbox.EnclosesInclusive(obj_center)) {
            if (child->bbox.EnclosesInclusive(obj_center)) {
//                cout << "Found enclosing bbox: min [" << child->bbox.min << "] - max [" << child->bbox.max << "]" << endl << endl;
                Insert(object, child, ++depth);
                return;
            }
        }
//        cout << "Didn't found enclosing bbox, create it now:" << endl;
        if (node->children.size() == 8) {
            std::cout << "Octree fully divided but couldn't fit obj_center: " << obj_center << endl;
            std::cout << "8 child are: " << endl;
            for (const auto& child :node->children) {
                std::cout << child->bbox << " => ";
                if (child->bbox.EnclosesInclusive(obj_center))
                    std::cout << "OK" << endl;
                else
                    std::cout << "FAIL" << endl;
            }
        }

        auto temp_bbox = GetChildOctreeBoundingBox(node->bbox, obj_center);

        for (const auto& child : node->children) {
            if (child->bbox == temp_bbox) {
                std::cout << "WTF we are going to insert a duplicate child to fit obj_center: " << obj_center << endl;
                for (const auto& tmp : node->children) {
                    std::cout << tmp->bbox << " => ";
                    if (tmp->bbox.EnclosesInclusive(obj_center))
                        std::cout << "OK" << endl;
                    else
                        std::cout << "FAIL" << endl;
                }
                auto tmp2 = GetChildOctreeBoundingBox(node->bbox, obj_center);
            }
        }
        // If there's currently no child to enclose this object, then we have to create it
        AddChildNode(node, object);
#endif
    }
}

void BVH::AddChildNode(const std::unique_ptr<Node>& node, Object3D* object) {

    node_count++;
    Node* child = new Node();
    Vec3 obj_center = object->GetCenter();
//    Vec3 obj_center = object->ComputeBBox().GetCenter();
    child->bbox = GetChildOctreeBoundingBox(node->bbox, obj_center);
    child->object = object;
//    node->children.emplace_back(std::unique_ptr<Node>(child));

    node->children.emplace_back(child);
//    cout << "Added child n°" << node->children.size() << endl;
//    cout << "New child is assigned obj: " << obj_center << endl;
//    cout << "New child created with Bbox min [" << child->bbox.min << "] - max [" << child->bbox.max << "]" << endl << endl;

}

BoundingBox BVH::GetChildOctreeBoundingBox(BoundingBox bbox, Vec3 point) {
    Vec3 min, max;
    Vec3 bbox_center = bbox.GetCenter();

    // (P < box center) => [min, max] = [box_min   , box_center]
    // else                [min, max] = [box_center, box_max   ]

    min.x = (point.x < bbox_center.x) ? bbox.min.x : bbox_center.x;
    min.y = (point.y < bbox_center.y) ? bbox.min.y : bbox_center.y;
    min.z = (point.z < bbox_center.z) ? bbox.min.z : bbox_center.z;

    // Point after box center
    max.x = (point.x < bbox_center.x) ? bbox_center.x : bbox.max.x;
    max.y = (point.y < bbox_center.y) ? bbox_center.y : bbox.max.y;
    max.z = (point.z < bbox_center.z) ? bbox_center.z : bbox.max.z;

    return BoundingBox (min, max);
}

void BVH::ComputeBoundingBoxes(const std::unique_ptr<Node>& node) {

    if (node->children.empty())
        node->bbox = node->bbox.ExtendsBy(node->object->ComputeBBox());
//        node->bbox = node->object->ComputeBBox();
    else {
        for (const auto& child : node->children) {
            ComputeBoundingBoxes(child);
            node->bbox = node->bbox.ExtendsBy(child->bbox);
        }
    }
}


bool BVH::FindNearestIntersection(const Ray& ray, float& dist_out, Object3D*& hit_object) {

    FastRay fast_ray {ray};

    return IntersectNode(fast_ray, dist_out, hit_object, root);
//    return IntersectNodeFast(fast_ray, dist_out, hit_object, root);

}

bool BVH::IntersectNode(const FastRay& ray, float& dist_out, Object3D*& hit_object, const std::unique_ptr<Node>& node) {

    float dist;
//    ray_bbox_test_count++;
    // We intersect this node
    if (node->bbox.IntersectBranched(ray, dist) && (dist < dist_out)) {
//        ray_bbox_hit_count++;

        // If no children, this node is a leaf so intersect the object it contains
        if (node->children.empty()) {
//            ray_obj_test_count++;
            if (node->object->Intersect(ray, dist) && (dist < dist_out)) {
//                ray_obj_hit_count++;
                dist_out = dist;
                hit_object = node->object;
            }
        }
        else {
            for (const auto& child : node->children) {
                IntersectNode(ray, dist_out, hit_object, child);
            }
        }

    }

    return (hit_object != nullptr);
}

bool BVH::IntersectNodeFast(const FastRay& ray, float& t_near_candidate, Object3D*& hit_object, const std::unique_ptr<Node>& node) {

//    ray_bbox_test_count++;

    float tmp;
    if (node->bbox.IntersectFast(ray, tmp) == false)
        return false;

//    ray_bbox_hit_count++;

    MyPQueue queue(12);

    float dist = 999999.f;

    queue.emplace(node.get(), dist);

//    char max = 0;

//    while (queue.empty() == false) {
    while (queue.empty() == false && (queue.top().t < t_near_candidate)) {

//        max = (char) fmaxf(max, queue.size());

        const Node* current_node = queue.top().node;
        queue.pop();

        if (current_node->children.empty()) {
//            ray_obj_test_count++;

            if (current_node->object->Intersect(ray, dist) && (dist < t_near_candidate)) {
//                ray_obj_hit_count++;
                t_near_candidate = dist;
                hit_object = current_node->object;
            }
        }
        else {
            for (const auto& child : current_node->children) {
//                ray_bbox_test_count++;

                if (child->bbox.IntersectFast(ray, dist)) {
//                    ray_bbox_hit_count++;
                    queue.emplace(child.get(), dist);
                }
            }
        }
    }

//    if (max > 12)
//    cout << int(max) << endl;

    return (hit_object != nullptr);
}

void BVH::ResetCounters() {
    ray_bbox_test_count = 0;
    ray_bbox_hit_count = 0;
    ray_obj_test_count = 0;
    ray_obj_hit_count = 0;
}

bool BVH::DebugIntersection(const Ray& ray, float& dist_out, Object3D*& hit_object, int depth_target) {
    FastRay fast_ray {ray};

    return DebugIntersectNode(ray, dist_out, hit_object, 0, depth_target, root);
}

bool BVH::DebugIntersectNode(const FastRay& ray, float& dist_out, Object3D*& hit_object, int depth, int debug_depth, const std::unique_ptr<Node>& node) {

    float dist = 99999999.f;
    // We intersect this node
    if (node->bbox.IntersectBranched(ray, dist) && (dist < dist_out)) {

        // If no children, this node is a leaf so intersect the object it contains
        if (depth == debug_depth) {
            dist_out = dist;
//            hit_object = &(node->bbox); //FIXME: if needed make bbox visualisation work again
        }
        else {
            for (const auto& child : node->children) {
                DebugIntersectNode(ray, dist_out, hit_object, depth + 1, debug_depth, child);
            }
        }

    }

    return (hit_object != nullptr);
}
