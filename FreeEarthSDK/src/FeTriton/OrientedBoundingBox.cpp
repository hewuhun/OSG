// Copyright 2013-2014 Sundog Software LLC. All rights reserved worldwide.

#include <FeTriton/OrientedBoundingBox.h>

using namespace Triton;

OrientedBoundingBox::OrientedBoundingBox()
{
    for (int i = 0; i < 3; i++) {
        halfDistances[i] = 0;
    }
}

void OrientedBoundingBox::Set(const Vector3& pCenter, const Vector3& xExtent, const Vector3& yExtent, const Vector3& zExtent)
{
    center = pCenter;

    halfDistances[0] = xExtent.Length();
    halfDistances[1] = yExtent.Length();
    halfDistances[2] = zExtent.Length();

    axes[0] = xExtent;
    axes[1] = yExtent;
    axes[2] = zExtent;

    axes[0].Normalize();
    axes[1].Normalize();
    axes[2].Normalize();

    RecomputeBasis();
}

void OrientedBoundingBox::RecomputeBasis()
{
    for (int row = 0; row < 3; row++) {
        invBasis.elem[row][0] = axes[row].x;
        invBasis.elem[row][1] = axes[row].y;
        invBasis.elem[row][2] = axes[row].z;
    }
    invBasis.Transpose();
}

bool OrientedBoundingBox::PointInBox(const Vector3& point, double slop) const
{
    Vector3 relativePoint = point - center;
    Vector3 rotatedPoint = relativePoint * invBasis;

    if (fabs(rotatedPoint.x) > halfDistances[0] + slop || fabs(rotatedPoint.y) > halfDistances[1] + slop || fabs(rotatedPoint.z) > halfDistances[2] + slop) {
        return false;
    }

    return true;
}

