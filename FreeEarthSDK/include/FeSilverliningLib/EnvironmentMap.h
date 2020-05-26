// Copyright (c) 2013-2015 Sundog Software LLC. All rights reserved worldwide.

#ifndef ENVIRONMENT_MAP_H
#define ENVIRONMENT_MAP_H

#include <FeSilverliningLib/MemAlloc.h>
#include <FeSilverliningLib/SilverLiningTypes.h>

namespace SilverLining
{
    class Atmosphere;

    class EnvironmentMap : public MemObject
    {
    public:
        EnvironmentMap(Atmosphere *atm);

        virtual ~EnvironmentMap();

        bool Create(int facesToRender = 6, bool floatingPoint = false, bool drawClouds = false, bool drawSunAndMoon = true);

        void *GetNativeTexture() const;

        void SetGeocentricMode(bool on) {geocentric = on;}

        static bool GetIsDrawingEnvMap() {return drawingEnvMap;}

    protected:
        Atmosphere *atmosphere;
        RenderTextureHandle cubeMap;
        int cubeMapDim;
        bool geocentric;
        int lastFaceRendered;
        bool enableStars;
        bool floatingPoint;
        bool neverDrawClouds;
        static bool drawingEnvMap;
    };
}
#endif
