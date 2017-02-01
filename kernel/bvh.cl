#include "bvh.h"

#include "macros.h"
#include "objects.h"

int BVHFindNearestIntersection(const Ray ray, global Node2* node, global Object3D* objects, VERTEX_GEOM_DATA_ARGS, float* t_near_candidate) {

    FastRay fast_ray;
    fast_ray.origin = ray.origin;
    fast_ray.direction_inv = normalize(1.f / ray.direction);


    int node_idx = 0;
    int obj_index_candidate = -1;

//    int count = 0;

    while (node_idx != -1) {
//        count++;
//        if (count > 6900) {
//            DEBUG_PIXEL(2000, 200)
//                printf("Avoiding infinite loop, iteration: %d\n", count);
//            break;
//        }

        float t_near;
        // If we hit this node bbox
        if (IntersectBoundingBox(&node[node_idx].bbox, fast_ray, &t_near)) {

            // If node is a leaf
            if (node[node_idx].obj_index != -1) {
                // Intersect the contained object
                if (IntersectObj(objects[node[node_idx].obj_index], VERTEX_GEOM_DATA, ray, &t_near) && (t_near < *t_near_candidate)) {
                    *t_near_candidate = t_near;
                    obj_index_candidate = node[node_idx].obj_index;
                }
                // Whatever the result, go to this node, can be sibling or uncle
                node_idx = node[node_idx].bbox.max.w;
            }
            // Else advance to next node in memory, guaranteed to be the first child
            else {
                node_idx++;
            }
        }
        // We didn't hit this node, so go to this node, can be sibling or uncle
        else {
            node_idx = node[node_idx].bbox.max.w;
        }
    }

    return obj_index_candidate;
}

#if 1
bool IntersectBoundingBox(const global BoundingBox* this, const FastRay ray, float* dist_out) {

//    float3 _min = this->min.xyz;
//    float3 _max = this->max.xyz;

    float t_nearest;
    float t_farthest;

    float t_nearest_axis = (this->min.x - ray.origin.x) * ray.direction_inv.x;
    float t_farthest_axis = (this->max.x - ray.origin.x) * ray.direction_inv.x;

    t_nearest = min(t_nearest_axis, t_farthest_axis);
    t_farthest = max(t_nearest_axis, t_farthest_axis);

    t_nearest_axis = (this->min.y - ray.origin.y) * ray.direction_inv.y;
    t_farthest_axis = (this->max.y - ray.origin.y) * ray.direction_inv.y;

    t_nearest = max(t_nearest, min(t_nearest_axis, t_farthest_axis));
    t_farthest = min(t_farthest, max(t_nearest_axis, t_farthest_axis));

    t_nearest_axis = (this->min.z - ray.origin.z) * ray.direction_inv.z;
    t_farthest_axis = (this->max.z - ray.origin.z) * ray.direction_inv.z;

    t_nearest = max(t_nearest, min(t_nearest_axis, t_farthest_axis));
    t_farthest = min(t_farthest, max(t_nearest_axis, t_farthest_axis));

//    if (t_farthest < t_nearest)
//        return false;

//    distance = (t_nearest > 0) ? t_nearest : t_farthest;
//
//    return distance > 0;
    *dist_out = t_nearest;

    return (t_nearest <= t_farthest) && ((t_nearest > 0) || (t_farthest > 0));
}
#else
bool IntersectBoundingBox(const global BoundingBox* this, const FastRay ray, float* dist_out) {

    float3 _min = this->min.xyz;
    float3 _max = this->max.xyz;

    float t_nearest = 999999999.f;
    float t_farthest = -999999999.f;

    float t_nearest_x = (_min.x - ray.origin.x) * ray.direction_inv.x;
    float t_farthest_x = (_max.x - ray.origin.x) * ray.direction_inv.x;

    t_nearest = min(t_nearest_x, t_farthest_x);
    t_farthest = max(t_nearest_x, t_farthest_x);

    float t_nearest_y = (_min.y - ray.origin.y) * ray.direction_inv.y;
    float t_farthest_y = (_max.y - ray.origin.y) * ray.direction_inv.y;

    t_nearest = max(t_nearest, min(t_nearest_y, t_farthest_y));
    t_farthest = min(t_farthest, max(t_nearest_y, t_farthest_y));

    float t_nearest_z = (_min.z - ray.origin.z) * ray.direction_inv.z;
    float t_farthest_z = (_max.z - ray.origin.z) * ray.direction_inv.z;

    t_nearest = max(t_nearest, min(t_nearest_z, t_farthest_z));
    t_farthest = min(t_farthest, max(t_nearest_z, t_farthest_z));

//    if (t_farthest < t_nearest)
//        return false;

//    distance = (t_nearest > 0) ? t_nearest : t_farthest;
//
//    return distance > 0;
    *dist_out = t_nearest;

    return (t_nearest <= t_farthest) && ((t_nearest > 0) || (t_farthest > 0));
}
#endif

#if 0
void SortedInsert(QueueNode* queue, int* queue_size, global Node* node, float t_near);
global Node* SortedPop(QueueNode* queue, int* queue_size);

#define TOP (queue_size-1)

int BVHFindNearestIntersection2(const Ray ray, global Node* root_node, global Object3D* objects, VERTEX_GEOM_DATA_ARGS, float* t_near_candidate) {

    FastRay fast_ray;
    fast_ray.origin = ray.origin;
    fast_ray.direction_inv = normalize(1.f / ray.direction);

    float tmp = 9999999.f;
    if (IntersectBoundingBox(&root_node->bbox, fast_ray, &tmp) == false)
        return -1;

    float t_near = 999999.f;

    QueueNode queue[20];

    int queue_size = 1;

    queue[TOP].node = root_node;
    queue[TOP].t_near = t_near;

    int obj_index_candidate = -1;

    while (queue_size != 0 && (queue[TOP].t_near < *t_near_candidate)) {

        global Node* current_node = SortedPop(queue, &queue_size);

        if (current_node->children_indices[0] == -1) {

            if (IntersectObj(objects[current_node->obj_index], VERTEX_GEOM_DATA, ray, &t_near) && (t_near < *t_near_candidate)) {
                *t_near_candidate = t_near;
                obj_index_candidate = current_node->obj_index;
            }
        }
        else {

            for (int i = 0; i < 8; ++i) {

                int child_index = current_node->children_indices[i];

                if (child_index == -1)
                   break;

                if (IntersectBoundingBox(&root_node[child_index].bbox, fast_ray, &t_near)) {
                    SortedInsert(queue, &queue_size, &root_node[child_index], t_near);
                }
            }

        }

    }

    return obj_index_candidate;
}
void swap(QueueNode* a, QueueNode* b) {
    QueueNode tmp = *a;
    *a = *b;
    *b = tmp;
}

void SortedInsert(QueueNode* queue, int* queue_size, global Node* node, float t_near) {

    int i;

    for (i = (*queue_size) - 1; i >= 0; i--) {

        if (queue[i].t_near >= t_near) {
            break;
        }

    }

    i++;
    (*queue_size)++;

    if (i != (*queue_size - 1)) {
        for (int j = (*queue_size) - 1; j > i; j--) {
            swap(&queue[j], &queue[j - 1]);
        }
    }

    queue[i].node = node;
    queue[i].t_near = t_near;

}

global Node* SortedPop(QueueNode* queue, int* queue_size) {
    (*queue_size)--;

    return queue[*queue_size].node;
}
#endif