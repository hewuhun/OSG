// Copyright (c) 2004-2012  Sundog Software, LLC. All rights reserved worldwide.

#include <FeSilverliningLib/Voxel.h>
#include <FeSilverliningLib/Metaball.h>
#include <FeSilverliningLib/Utils.h>
#if defined(__INTEL_COMPILER)
#include <mathimf.h>
#else
#include <math.h>
#endif
#include <stdio.h>

using namespace SilverLining;

Vector3 Voxel::sortPos;
Vector3 Voxel::offsetPos;
Voxel::SortingModes Voxel::sortMode = BACK_TO_FRONT;

Voxel::Voxel(double dimension, double particleSize, double maxDensity, double spinRate, int atlasIdx, bool rotate)
{
    float ballRadius = (float)ComputeVoxelRadius(dimension);

    InitializeMetaball(ballRadius, (float)spinRate, atlasIdx, rotate);

    double T = dimension;
    opticalDepth = (float)(PI * (particleSize * particleSize) * T * maxDensity);

    double extinction = exp(-opticalDepth);
    SetAlpha(1.0 - extinction);

    SetHasLightning(false);

#ifndef HAS_BITSET
    flags = 0;
#endif
}

Voxel::Voxel(std::istream& s)
{
#ifndef HAS_BITSET
    flags = 0;
#endif

    s.read((char *)&opticalDepth, sizeof(float));
    bool flag;

    s.read((char *)&flag, sizeof(bool));
    SetHasCloud(flag);
    s.read((char *)&flag, sizeof(bool));
    SetVapor(flag);
    s.read((char *)&flag, sizeof(bool));
    SetPhaseTransition(flag);
    s.read((char *)&flag, sizeof(bool));
    SetHasLightning(flag);

    float ballRadius;
    s.read((char *)&ballRadius, sizeof(float));

    float spinRate;
    s.read((char *)&spinRate, sizeof(float));

    Vector3 pos;
    pos.Unserialize(s);

    Color col;
    col.Unserialize(s);

    Color billboardCol;
    billboardCol.Unserialize(s);

    s.read((char *)&atlasIndex, sizeof(unsigned char));

    InitializeMetaball(ballRadius, spinRate, atlasIndex, atlasIndex == 9 || atlasIndex == 13 || atlasIndex == 0);
    SetWorldPosition(pos);
    SetMetaballColor(col);
    SetColor(billboardCol);

    SetCreated(true);
}

bool Voxel::Serialize(std::ostream& s)
{
    s.write((char *)&opticalDepth, sizeof(float));

    bool flag;

    flag = GetHasCloud();
    s.write((char *)&flag, sizeof(bool));
    flag = GetVapor();
    s.write((char *)&flag, sizeof(bool));
    flag = GetPhaseTransition();
    s.write((char *)&flag, sizeof(bool));
    flag = GetHasLightning();
    s.write((char *)&flag, sizeof(bool));


    float ballRadius = (float)GetRadius();
    s.write((char *)&ballRadius, sizeof(float));

    float spinRate = (float)GetSpinRate();
    s.write((char *)&spinRate, sizeof(float));

    const Vector3f& fPos = GetWorldPosition();
    Vector3 pos(fPos.x, fPos.y, fPos.z);
    pos.Serialize(s);

    Color col = GetMetaballColor();
    col.Serialize(s);

    col = GetColor();
    col.Serialize(s);

    s.write((char *)&atlasIndex, sizeof(unsigned char));

    return true;
}

Voxel::~Voxel()
{
}


bool Voxel::operator < (const Voxel& v)
{
    const Vector3f& myWorldPos = GetWorldPosition();
    const Vector3f& theirWorldPos = v.GetWorldPosition();

    double myDist = (sortPos - (Vector3(myWorldPos.x, myWorldPos.y, myWorldPos.z) + offsetPos)).SquaredLength();
    double itsDist = (sortPos - (Vector3(theirWorldPos.x, theirWorldPos.y, theirWorldPos.z) + offsetPos)).SquaredLength();

    if (sortMode == FRONT_TO_BACK) {
        return myDist < itsDist;
    } else {
        return itsDist < myDist;
    }
}
