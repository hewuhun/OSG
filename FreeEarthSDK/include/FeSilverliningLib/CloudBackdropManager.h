// Copyright (c) 2012 Sundog Software, LLC. All rights reserved worldwide.

/**
\file CloudBackdropManager.h
\brief A class to update and draw distant clouds rendered to a group of panels surrounding the viewpoint.
*/

#ifndef CLOUD_BACKDROP_MANAGER_H
#define CLOUD_BACKDROP_MANAGER_H

#include <FeSilverliningLib/MemAlloc.h>
#include <FeSilverliningLib/CloudLayer.h>
#include <FeSilverliningLib/CloudBackdrop.h>

namespace SilverLining
{
    class CloudBackdropManager : public MemObject
    {
    public:
        CloudBackdropManager(int numBackdrops, double distance);

        virtual ~CloudBackdropManager();

        void Enable(bool pEnabled) {
           enabled = pEnabled;
        }

        bool Initialize();

        bool Update(const Frustum& f, const SL_VECTOR(CloudLayer *)& cloudLayers, unsigned long now, const Sky& sky,
            const Vector3& lightDir, const Color& lightColor);

        bool Draw(const Frustum& f);

        bool IsCloudInBackdrop(Cloud *cloud, const Vector3& camPos);

    protected:
        SL_VECTOR(CloudBackdrop*) backdrops;
        SL_VECTOR(CloudLayer *) lastCloudLayers;
        SL_VECTOR(bool) lastCloudLayerEnabled;
        SL_VECTOR(Vector3) lastCloudLayerPositions;
        double distance, cullDistanceSquared, horizontalThreshold, cosBackdropAngularHeight;
        int lastBackdropUpdated;
        ShaderHandle backdropShader;
        Vector3 lastUpdatePosition;
        bool initialFrame, enabled;
        double lastFogDensity;
    };
}

#endif
