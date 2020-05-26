// Copyright (c) 2004-2011  Sundog Software, LLC. All rights reserved worldwide.

#include <FeTriton/Matrix3.h>

using namespace Triton;

Matrix3 Matrix3::Identity = Matrix3(1, 0, 0, 0, 1, 0, 0, 0, 1);

void Matrix3::FromRx(double rad)
{
    double sinr = sin(rad);
    double cosr = cos(rad);

    elem[0][0] = 1.0;
    elem[0][1] = 0;
    elem[0][2] = 0;
    elem[1][0] = 0;
    elem[1][1] = cosr;
    elem[1][2] = sinr;
    elem[2][0] = 0;
    elem[2][1] = -sinr;
    elem[2][2] = cosr;
}

void Matrix3::FromRy(double rad)
{
    double sinr = sin(rad);
    double cosr = cos(rad);

    elem[0][0] = cosr;
    elem[0][1] = 0;
    elem[0][2] = -sinr;
    elem[1][0] = 0;
    elem[1][1] = 1.0;
    elem[1][2] = 0;
    elem[2][0] = sinr;
    elem[2][1] = 0;
    elem[2][2] = cosr;
}

void Matrix3::FromRz(double rad)
{
    double sinr = sin(rad);
    double cosr = cos(rad);

    elem[0][0] = cosr;
    elem[0][1] = sinr;
    elem[0][2] = 0;
    elem[1][0] = -sinr;
    elem[1][1] = cosr;
    elem[1][2] = 0;
    elem[2][0] = 0;
    elem[2][1] = 0;
    elem[2][2] = 1;
}

void Matrix3::FromXYZ(double Rx, double Ry, double Rz)
{
    Matrix3 rx, ry, rz;
    rx.FromRx(Rx);
    ry.FromRy(Ry);
    rz.FromRz(Rz);

    *this = rx * (ry * rz);
}

Vector3 Matrix3::operator* (const Vector3& rkPoint) const
{
    Vector3 kProd;
    kProd.x =
        elem[0][0]*rkPoint.x +
        elem[0][1]*rkPoint.y +
        elem[0][2]*rkPoint.z;

    kProd.y =
        elem[1][0]*rkPoint.x +
        elem[1][1]*rkPoint.y +
        elem[1][2]*rkPoint.z;

    kProd.z =
        elem[2][0]*rkPoint.x +
        elem[2][1]*rkPoint.y +
        elem[2][2]*rkPoint.z;

    return kProd;
}

Matrix3 Matrix3::operator * (const Matrix3 &mat)
{
    Matrix3 out;
    for (int row = 0; row < 3; row++) {
        for (int col = 0; col < 3; col++) {
            out.elem[row][col] =
                elem[row][0] * mat.elem[0][col] +
                elem[row][1] * mat.elem[1][col] +
                elem[row][2] * mat.elem[2][col];
        }
    }

    return out;
}

Matrix3 Matrix3::Transpose() const
{
    Matrix3 out;
    for (int row = 0; row < 3; row++) {
        for (int col = 0; col < 3; col++) {
            out.elem[row][col] = elem[col][row];
        }
    }

    return out;
}

namespace Triton
{
Vector3 operator* (const Vector3& rkPoint, const Matrix3& rkMatrix)
{
    Vector3 kProd;

    kProd.x = rkPoint.x * rkMatrix.elem[0][0] +
              rkPoint.y * rkMatrix.elem[1][0] +
              rkPoint.z * rkMatrix.elem[2][0];

    kProd.y = rkPoint.x * rkMatrix.elem[0][1] +
              rkPoint.y * rkMatrix.elem[1][1] +
              rkPoint.z * rkMatrix.elem[2][1];

    kProd.z = rkPoint.x * rkMatrix.elem[0][2] +
              rkPoint.y * rkMatrix.elem[1][2] +
              rkPoint.z * rkMatrix.elem[2][2];

    return kProd;
}
}
