#ifndef TEST3D_MATRIX_H
#define TEST3D_MATRIX_H

#include "Vec3.h"

class Matrix {

    void identity();

public:

    float values[4][4];
    Matrix();

    Matrix(const Matrix& other);

    Matrix& translateBy(const Vec3& vec);

    friend std::ostream& operator<<(std::ostream& out, Matrix& matrix);

    Matrix &translateBy(const float &x, const float &y, const float &z);

    float *operator[](int i);

    const float *operator[](int i) const;

    Matrix operator*(const Matrix &rhs);

    void setRotation(float yz_angle, float xz_angle, float xy_angle);

    void setXZRotation(float angle);

    Vec3 operator*(const Vec3 &vec) const;

    void setYZRotation(float angle);

    Matrix Transpose() const;
};


#endif //TEST3D_MATRIX_H
