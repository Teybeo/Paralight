#include "Intersectable.h"

#include "BoundingBox.h"

Vec3 Intersectable::GetCenter() const {
    return ComputeBBox().GetCenter();
}

