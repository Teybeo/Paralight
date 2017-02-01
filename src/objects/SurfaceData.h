#ifndef PATHTRACER_SURFACEDATA_H
#define PATHTRACER_SURFACEDATA_H

#include "math/Vec3.h"

typedef struct Material Material;

struct SurfaceData {
    Material* material;
    Vec3 normal;
    Vec3 uv;
    Vec3 tangent;
    Vec3 bitangent;
};

#endif //PATHTRACER_SURFACEDATA_H
