// Copyright (c) 2012 Sundog Software LLC. All rights reserved worldwide.

/**
\file CloudBackdrop.h
\brief An individual quad that renders a flattened representation of clouds behind it.
*/

#ifndef CLOUDBACKDROP_H
#define CLOUDBACKDROP_H

#include <FeSilverliningLib/MemAlloc.h>
#include <FeSilverliningLib/Renderable.h>

namespace SilverLining
{
    class Cloud;

    /** An individual quad that renders a dynamically generated imposter of the clouds behind it. */
    class CloudBackdrop : public Renderable
    {
    public:
        CloudBackdrop(double direction, double angularSize, double distance);
        
        virtual ~CloudBackdrop();

        bool Initialize(ShaderHandle shader);

        bool Update(const Frustum& f, const SL_VECTOR(CloudLayer*)& clouds, unsigned long now, const Sky& sky,
            const Vector3& lightDir, const Color& lightColor, bool forceUpdate);

        virtual bool Draw(int pass);

        virtual bool Cull(const Frustum& f);

        virtual const Vector3& GetWorldPosition() const;

        virtual void DrawBlendedObject();

    protected:
        bool SetupGeometry();
        bool CullCloudInside(const Frustum& f, const Cloud *cloud);
        bool DrawClouds(int texNum);

        double width, height, direction, angularSize, distance, backdropAngularHeight;
        VertexBuffer *vb;
        IndexBuffer *ib;
        RenderTextureHandle renderTexture[2];
        int currentTexture;
        unsigned long transitionStartTime;
        float transitionLength;
        bool initialized, rendered;
        Vector3 worldPosition, relativePosition;
        ShaderHandle shader;
        double radius;
        Vector3 lastUpdatePosition;
        Vector3 lastLightDir;
        Color lastLightColor;
        double verticalThreshold, horizontalThreshold;
        int numClouds;
        float blend;
        bool firstFrame;
        int imposterDimX, imposterDimY;
    };
}

#endif

