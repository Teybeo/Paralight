#include "Vec3.h"

#include <ostream>

std::ostream& operator<<(std::ostream &out, const Vec3 &vec) {
    out << vec.x << ", " << vec.y << ", " << vec.z;
    return out;
}

