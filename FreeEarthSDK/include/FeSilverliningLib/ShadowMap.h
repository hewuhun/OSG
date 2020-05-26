// Copyright (c) 2013 Sundog Software, LLC. All rights reserved worldwide.

#ifndef SHADOW_MAP_H
#define SHADOW_MAP_H

#include <FeSilverliningLib/SilverLiningTypes.h>
#include <FeSilverliningLib/Color.h>
#include <FeSilverliningLib/MemAlloc.h>
#include <FeSilverliningLib/Matrix4.h>
#include <vector>

namespace SilverLining
{
    class Atmosphere;
    class CloudLayer;

    class ShadowMap : public MemObject
    {
    public:
        ShadowMap(Atmosphere *atm, int texDim);

        virtual ~ShadowMap();

        bool SetupShadowMap(const Vector3& lightDirection, const SL_VECTOR(CloudLayer *)& cloudLayers, bool wholeLayers, double maxShadowDistance);

        void SetLightMatrices();

        bool StartDrawing(const Color& shadowColor, bool moonShadows);

        bool DrawToRenderTexture();

        bool EndDrawing();

        bool IsRendering() const {return isRendering;}

        TextureHandle GetTextureHandle();

        Matrix4 GetLightModelviewProjectionMatrix();

        Matrix4 GetWorldToShadowTexCoordMatrix();

    private:

        void FindFrustumWorldSpace(const SL_VECTOR(CloudLayer *)& cloudLayers, bool wholeLayers, double maxShadowDistance);

        void ComputeLightMatrix(const Vector3& lightDir, bool wholeLayers);

        void GetFrustumBounds(const Matrix4& xform, Vector3& min, Vector3& max);

        Matrix4 MakeOrthoMatrix(double left, double right, double top, double bottom, double pnear, double pfar);

        Matrix4 lightMVP, lightModelView, lightProjection, billboard;
        Vector3 frustumCornersWorld[8];
        Vector3 frustumCentroidWorld;
        double frustumBoundRadius;
        Vector3 lightWorldPos, savedCamPos;
        Atmosphere *atmosphere;
        RenderTextureHandle texture;
        int texSize;
        bool cleared;
        bool hasShadows;
        bool isRendering;
        int savedX, savedY, savedW, savedH;
        double savedDensity;
        float shadowBlend;
    };
}

#endif

