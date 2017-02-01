#ifndef TEST3D_VEC3_H
#define TEST3D_VEC3_H

#include <iosfwd>
#include <cmath>
#include <iostream>
#include "TrigoLut.h"

#define CHECK_NAN(X, x, y)\
        if (isnan((X).x) || isnan((X).y) || isnan((X).x)) \
            printf("%f, %f, %f at (%d, %d)\n", (X).x, (X).y, (X).z, x, y);


class Vec3 {
public:
    float x = 0;
    float y = 0;
    float z = 0;

    Vec3() = default;
    Vec3(const Vec3& other) = default;
    Vec3(float x, float y, float z) : x(x), y(y), z(z) { }
    Vec3(float value) : x(value), y(value), z(value) { }

    friend std::ostream& operator<<(std::ostream& out, const Vec3& vec);

    bool operator==(const Vec3& rhs) const {
        return x == rhs.x && y == rhs.y && z == rhs.z;
    }

    bool operator!=(const Vec3& rhs) const {
        return x != rhs.x || y != rhs.y || z != rhs.z;
    }

    Vec3 operator-(const Vec3& rhs) const {
        return Vec3{x - rhs.x,
                    y - rhs.y,
                    z - rhs.z};
    }

    void operator-=(const Vec3& rhs) {
        x -= rhs.x;
        y -= rhs.y;
        z -= rhs.z;
    }

    Vec3 operator-() const {
        return Vec3{-x, -y, -z};
    }

    Vec3 operator+(const Vec3& rhs) const {
        return Vec3{x + rhs.x,
                    y + rhs.y,
                    z + rhs.z};
    }

    void operator+=(const Vec3& rhs) {
        x += rhs.x;
        y += rhs.y;
        z += rhs.z;
    }

    Vec3 operator*(const Vec3& rhs) const {
        return Vec3{x * rhs.x,
                    y * rhs.y,
                    z * rhs.z};
    }
    void operator*=(const Vec3& rhs) {
        x *= rhs.x;
        y *= rhs.y;
        z *= rhs.z;
    }

    Vec3 friend operator*(Vec3 vec, float scalar);
    Vec3 friend operator*(float scalar, Vec3 vec);

    void operator*=(const float scalar) {
        x *= scalar;
        y *= scalar;
        z *= scalar;
    }

    Vec3 operator/(const Vec3& rhs) const {
        return Vec3{x / rhs.x,
                    y / rhs.y,
                    z / rhs.z};
    }

    Vec3 operator/(const float scalar) const {
        return Vec3{x / scalar,
                    y / scalar,
                    z / scalar};
    }

    void operator/=(const float scalar) {
        x /= scalar;
        y /= scalar;
        z /= scalar;
    }

    bool operator>(const Vec3& rhs) const {
        return x > rhs.x && y > rhs.y && z > rhs.z;
    }

    bool operator<(const Vec3& rhs) const {
        return x < rhs.x && y < rhs.y && z < rhs.z;
    }

    bool operator>=(const Vec3& rhs) const {
        return x >= rhs.x && y >= rhs.y && z >= rhs.z;
    }

    bool operator<=(const Vec3& rhs) const {
        return x <= rhs.x && y <= rhs.y && z <= rhs.z;
    }

    bool operator>(const float scalar) const {
        return x > scalar && y > scalar && z > scalar;
    }

    bool operator<(const float scalar) const {
        return x < scalar && y < scalar && z < scalar;
    }

    const float& operator[](int dimension) const {
        return (&x)[dimension];
    }

    float dot(const Vec3& rhs) const {
        return (this->x * rhs.x) +
               (this->y * rhs.y) +
               (this->z * rhs.z);
    }

   Vec3 cross(const Vec3& rhs) const {
        return Vec3(y * rhs.z - z * rhs.y,
                    z * rhs.x - x * rhs.z,
                    x * rhs.y - y * rhs.x);
   }

    Vec3& normalize() {
        float inv_length = 1 / this->length();
        this->x *= inv_length;
        this->y *= inv_length;
        this->z *= inv_length;
        return *this;
    }

    float length() const {
        return std::sqrt(this->dot(*this));
    }

    float lengthSquared() const {
        return this->dot(*this);
    }

    /**
     * Reflect vec around this vector
     * Both vectors must be normalized
     * Result is normalized if input vectors are
     */
    Vec3 reflect(const Vec3& vec) const {
        return (this->dot(vec) * 2 * (*this)) - vec;
    }
    /**
     * [u projected on v] = (u.v / v.v) * v
     * Project this vector on the other vector passed in argument
     * @param v The vector on which to project
     */
    Vec3 projectOn(Vec3 v) {

        return (this->dot(v) / v.dot(v)) * v;
    }

    Vec3 clamp(float min, float max) {
        return Vec3{
                std::max(min, std::min(max, x)),
                std::max(min, std::min(max, y)),
                std::max(min, std::min(max, z))
        };
    }
    /**
     * Treat this vector as the normal and create associated bitangent and tangent vectors
     */
    void createBitangentAndTangent(Vec3& b, Vec3& t) {
        if (std::fabs(x) > std::fabs(y))
            t = Vec3(z, 0, -x) / sqrtf(x * x + z * z);
        else
            t = Vec3(0, -z, y) / sqrtf(y * y + z * z);
        b = this->cross(t);
        b.normalize();
//        t = Vec3{-z, 0, x};
//        b = this->cross(t);
    }

    /**
     * Transform this vector from world space to the tangent space defined by a normal
     */
    Vec3 WorldToTangent(Vec3 normal) {

        Vec3 b, t;
        normal.createBitangentAndTangent(b, t);

        return Vec3 {
                x * b.x  +  y * normal.x  +  z * t.x,
                x * b.y  +  y * normal.y  +  z * t.y,
                x * b.z  +  y * normal.z  +  z * t.z};
    }

#define UP Vec3(0, 1, 0)

    //TODO: Transform this vector from tangent space to world space
    // The tangent space is constructed from the worldspace normal passed in argument
    Vec3 TangentToWorld2(Vec3 normal) {

        Vec3 orthogonal = -UP.normalize();

        Vec3 t = normal.cross(orthogonal).normalize();
        Vec3 b = normal.cross(t).normalize();

        return Vec3 {
                x * t.x  +  y * b.x  +  z * normal.x,
                x * t.y  +  y * b.y  +  z * normal.y,
                x * t.z  +  y * b.z  +  z * normal.z}.normalize();

    }

    Vec3 TangentToWorld(Vec3 normal, Vec3 tangent, Vec3 bitangent) {

//        Vec3 b, t;
//        normal.createBitangentAndTangent(b, t);
//        return Vec3 {
//                this->dot(tangent),
//                this->dot(bitangent),
//                this->dot(normal)}.normalize();

        return Vec3 {
                x * tangent.x  +  y * bitangent.x  +  z * normal.x,
                x * tangent.y  +  y * bitangent.y  +  z * normal.y,
                x * tangent.z  +  y * bitangent.z  +  z * normal.z}.normalize();

    }

    friend Vec3 linear_to_sRGB(Vec3 vec);

    float max() const {
        return std::max(x, std::max(y, z));
    }
    float min() const {
        return std::min(x, std::min(y, z));
    }

    Vec3 abs() const {
        return Vec3{fabsf(x), fabsf(y), fabsf(z)};
    }

    Vec3 pow(float exponent) const {
        return Vec3{
                powf(x, exponent),
                powf(y, exponent),
                powf(z, exponent)};
    }

    void checkNormal() const {
        if (length() != 1.0e-2) {
            std::cout.setf( std::ios::fixed, std:: ios::floatfield ); // floatfield set to fixed
            std::cout.precision(26);
            std::cout << " length: " << length() << std::endl;
        }
    }

    Vec3 SphericalToCartesian() const {
#ifdef USE_TRIGO_LOOKUP
        float polar = ACOS_LOOKUP(y);      // acos() => [0, PI]
#else
        float polar = acosf(y);            // acos() => [0, PI]
#endif

        float azimuth = atan2f(x, z);// atan() => [-PI, PI]

        float u = (azimuth + M_PI_F) / (2 * M_PI_F); // [-PI, PI] => [0, 1]
        float v = (polar / M_PI_F);                  // [0, PI]   => [0, 1]

        return Vec3{u, v, 0};
    }

    Vec3 Abs();
};

inline Vec3 operator*(Vec3 vec, float scalar) {
    vec *= scalar;
    return vec;
}

inline Vec3 operator*(float scalar, Vec3 vec) {
    vec *= scalar;
    return vec;
}

struct alignas(16) CLVec4 {
    float r;
    float g;
    float b;
    float a;
};

struct alignas(8) CLVec2 {
    float u;
    float v;
};

#endif //TEST3D_VEC3_H
