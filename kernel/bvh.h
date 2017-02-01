#ifndef _BVH_H
#define _BVH_H

#include "objects.h"

typedef struct BoundingBox {
    float4 min;
    float4 max;
} BoundingBox;

typedef struct Node {
    BoundingBox bbox;
    int obj_index;
    int children_indices[8];
} Node;

typedef struct Node2 {
    BoundingBox bbox;
    int obj_index;
} Node2;

typedef struct QueueNode {
    global Node* node;
    float t_near;
} QueueNode;

int BVHFindNearestIntersection(const Ray ray, global Node2* root_node, global Object3D* objects, VERTEX_GEOM_DATA_ARGS, float* t_near_candidate);
bool IntersectBoundingBox(const global BoundingBox* this, const FastRay ray, float* dist_out);

#endif