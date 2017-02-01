#include "Matrix.h"

#include <string.h>
#include <iostream>

Matrix::Matrix() {
    //std::cout << "Matrix empty ctor\n";
    identity();
}

Matrix::Matrix(const Matrix& other) {
    std::cout << "Matrix copy ctor\n";
    memcpy(values, other.values, 16 * sizeof(float));
}

void Matrix::identity() {

    memset(values, 0, 16 * sizeof(float));

    values[0][0] = 1;
    values[1][1] = 1;
    values[2][2] = 1;
    values[3][3] = 1;
}

/* ?,   ?,   ?,   += x,
   ?,   ?,   ?,   += y,
   ?,   ?,   ?,   += z,
   ?,   ?,   ?,   ? */
Matrix& Matrix::translateBy(const Vec3& vec) {

    values[0][3] += vec.x;
    values[1][3] += vec.y;
    values[2][3] += vec.z;

    return *this;
}

Matrix& Matrix::translateBy(const float& x, const float& y, const float& z) {

    values[0][3] += x;
    values[1][3] += y;
    values[2][3] += z;

    return *this;
}

std::ostream& operator<< (std::ostream& out, Matrix& matrix) {

    out << matrix[0][0] << ", " << matrix[0][1] << ", " << matrix[0][2] << ", " << matrix[0][3] << '\n';
    out << matrix[1][0] << ", " << matrix[1][1] << ", " << matrix[1][2] << ", " << matrix[1][3] << '\n';
    out << matrix[2][0] << ", " << matrix[2][1] << ", " << matrix[2][2] << ", " << matrix[2][3] << '\n';
    out << matrix[3][0] << ", " << matrix[3][1] << ", " << matrix[3][2] << ", " << matrix[3][3] << '\n';
    return out;
}

float* Matrix::operator[] (const int i) {
    return values[i];
}
const float* Matrix::operator[] (const int i) const {
    return values[i];
}

Matrix Matrix::operator* (const Matrix& rhs) {

    Matrix res;

    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            res[i][j] =
                    values[i][0] * rhs.values[0][j] +
                    values[i][1] * rhs.values[1][j] +
                    values[i][2] * rhs.values[2][j] +
                    values[i][3] * rhs.values[3][j];
        }
    }
    return res;
}

void Matrix::setRotation(float yz_angle, float xz_angle, float xy_angle) {

    Matrix yz_rotation;
    Matrix xz_rotation;

    yz_rotation.setYZRotation(yz_angle);
    xz_rotation.setXZRotation(xz_angle);

    *this = xz_rotation * yz_rotation;
}

void Matrix::setXZRotation(float angle) {
    values[0][0] =  cosf(angle);
    values[2][0] =  sinf(angle);
    values[0][2] = -sinf(angle);
    values[2][2] = cosf(angle);
}

void Matrix::setYZRotation(float angle) {
    values[1][1] =  cosf(angle);
    values[2][1] = -sinf(angle);
    values[1][2] =  sinf(angle);
    values[2][2] =  cosf(angle);
}

Vec3 Matrix::operator*(const Vec3& vec) const {
    Vec3 result(vec);
    result.x = vec.x * values[0][0] +
               vec.y * values[0][1] +
               vec.z * values[0][2];

    result.y = vec.x * values[1][0] +
               vec.y * values[1][1] +
               vec.z * values[1][2];

    result.z = vec.x * values[2][0] +
               vec.y * values[2][1] +
               vec.z * values[2][2];

    return result;
}

Matrix Matrix::Transpose() const {
    Matrix res;

    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            res[i][j] = values[j][i];
        }
    }
    return res;
}