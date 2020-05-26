// Copyright (c) 2013-2015 Sundog Software LLC. All rights reserved worldwide.

#include <FeTriton/DecalManager.h>
#include <FeTriton/Environment.h>
#include <FeTriton/Configuration.h>
#include <FeTriton/Plane.h>

using namespace Triton;

DecalManager::DecalManager(const Environment *env) : environment(env)
{
    depthOffset = -0.001f;
    Configuration::GetFloatValue("decal-depth-offset", depthOffset);

    getDepthRange = false;
    Configuration::GetBoolValue("decal-get-depth-range", getDepthRange);
}

DecalManager::~DecalManager()
{
    decals.clear();
}

void DecalManager::AddDecal(Decal *decal)
{
    decals.push_back(decal);
}

void DecalManager::RemoveDecal(Decal *decal)
{
    decals.remove(decal);
}

bool DecalManager::HasDecals() const
{
    TRITON_LIST(Decal*)::const_iterator it;
    for (it = decals.begin(); it != decals.end(); it++) {
        if ((*it)->GetActive()) {
            return true;
        }
    }

    return false;
}

void DecalManager::ComputeBasis()
{
    if (environment) {
        Vector3 up = environment->GetUpVector();
        Vector3 right = environment->GetRightVector();
        Vector3 in = up.Cross(right);

        basis.elem[0][0] = right.x;
        basis.elem[0][1] = right.y;
        basis.elem[0][2] = right.z;
        basis.elem[1][0] = up.x;
        basis.elem[1][1] = up.y;
        basis.elem[1][2] = up.z;
        basis.elem[2][0] = -in.x;
        basis.elem[2][1] = -in.y;
        basis.elem[2][2] = -in.z;

        invBasis = basis.InverseCramers();
    }
}

double DecalManager::GetNearClipDistance(const Matrix4& m)
{
    if (environment->IsDirectX()) {
        return m(2, 3) * -1.0;
    } else {
        Vector3 m2 = m.GetRow(2);
        Vector3 m3 = m.GetRow(3);

        Vector3 n;
        double d;

        n = m3 + m2;
        d = m(3, 3) + m(2, 3);
        Plane pnear(n, d);

        pnear.Normalize();

        return pnear.GetDistance() * -1.0;
    }
}