#ifndef PATHTRACER_BVHCOMMONS_H
#define PATHTRACER_BVHCOMMONS_H

#include <objects/BoundingBox.h>
#include <bits/unique_ptr.h>
#include <vector>

typedef struct Object3D Object3D;

struct CLNode {

    CLBoundingBox bbox;
    int obj_index;
    int children_indices[8];
};
struct CLNode2 {

    CLBoundingBox bbox;
    int obj_index;
    int left_child;
    int right_child;
};

struct Node {

    BoundingBox bbox;
    Object3D* object = nullptr;
    std::vector<std::unique_ptr<Node>> children;
    char split_axis = -1;

    Node() = default;
    Node(const BoundingBox& bbox, Object3D* object)
            : bbox(bbox), object(object)
    { }
};

struct Node2 {

    BoundingBox bbox;
    Object3D* object = nullptr;
    std::unique_ptr<Node2> left_child;
    std::unique_ptr<Node2> right_child;
    char split_axis = -1;

    Node2() = default;
    Node2(const BoundingBox& bbox, Object3D* object)
            : bbox(bbox), object(object)
    { }
};

#endif //PATHTRACER_BVHCOMMONS_H
