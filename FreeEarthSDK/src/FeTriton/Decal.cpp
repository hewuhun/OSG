// Copyright (c) 2013-2014 Sundog Software LLC. All rights reserved worldwide.

#include <FeTriton/Decal.h>
#include <FeTriton/Configuration.h>
#include <FeTriton/Environment.h>

using namespace Triton;

Decal::Decal(const Environment *env, TextureHandle pTexture, float pSize) :
    environment(env), texture(pTexture), size(pSize), lightColor(1.0, 1.0, 1.0), alpha(1.0f), additive(false),
    scaleX(1.0f), scaleZ(1.0f), culled(false), rotation(0), uOffset(0), vOffset(0)
{
    boxHeight = 15.0;
    Configuration::GetDoubleValue("decal-volume-height", boxHeight);

    volumeScale = 1.0;
    Configuration::GetFloatValue("decal-volume-scale", volumeScale);

    ComputeOBB();
}

void Decal::SetPosition(const Vector3& pos)
{
    position = pos;
    ComputeOBB();
}

void Decal::ComputeOBB()
{
    Vector3 right, up, in;

    switch (environment->GetCoordinateSystem()) {
    case FLAT_ZUP: {
        right = Vector3(1, 0, 0);
        up = Vector3(0, 0, 1);
        in = Vector3(0, 1, 0);
        break;
    }

    case FLAT_YUP: {
        right = Vector3(1, 0, 0);
        up = Vector3(0, 1, 0);
        in = Vector3(0, 0, -1);
        break;
    }

    case WGS84_ZUP:
    case SPHERICAL_ZUP: {
        up = position;
        up.Normalize();
        Vector3 north(0, 0, 1);
        right = north.Cross(up);
        right.Normalize();
        in = up.Cross(right);
        in.Normalize();
        break;
    }

    case WGS84_YUP:
    case SPHERICAL_YUP: {
        up = position;
        up.Normalize();
        Vector3 north(0, 1, 0);
        right = north.Cross(up);
        right.Normalize();
        in = up.Cross(right);
        in.Normalize();
        break;
    }
    }

    obb.Set(position, right * (size * 0.5), up * (boxHeight * 0.5), in * (size * 0.5));
}

void Decal::ComputeDecalMatrix(const Vector3& camPos, const Matrix3& rotate)
{
    Matrix4 view;
    Matrix4 proj;

    Vector3 xa, ya, za;
    xa = rotate * obb.axes[0]; // right
    ya = rotate * obb.axes[1]; // up
    za = rotate * obb.axes[2]; // forward

    // x axis (right)
    view.elem[0][0] = xa.x;
    view.elem[1][0] = xa.y;
    view.elem[2][0] = xa.z;

    // y axis (up) - looking down, so this is forward
    view.elem[0][1] = za.x;
    view.elem[1][1] = za.y;
    view.elem[2][1] = za.z;

    // z axis (forward) - looking down, so this is down
    view.elem[0][2] = -ya.x;
    view.elem[1][2] = -ya.y;
    view.elem[2][2] = -ya.z;


    Vector3 trans = (position - camPos) * -1;
    view.elem[3][0] = view.elem[0][0] * trans.x + view.elem[1][0] * trans.y + view.elem[2][0] * trans.z + view.elem[3][0];
    view.elem[3][1] = view.elem[0][1] * trans.x + view.elem[1][1] * trans.y + view.elem[2][1] * trans.z + view.elem[3][1];
    view.elem[3][2] = view.elem[0][2] * trans.x + view.elem[1][2] * trans.y + view.elem[2][2] * trans.z + view.elem[3][2];
    view.elem[3][3] = view.elem[0][3] * trans.x + view.elem[1][3] * trans.y + view.elem[2][3] * trans.z + view.elem[3][3];


    // Ortho matrix
    proj.elem[0][0] = 1.0 / (scaleX * obb.halfDistances[0]);
    proj.elem[1][1] = 1.0 / (scaleZ * obb.halfDistances[2]);
    proj.elem[2][2] = -1.0 / obb.halfDistances[1];

    decalMatrix = view * proj;

    //decalMatrix.Transpose();
}
