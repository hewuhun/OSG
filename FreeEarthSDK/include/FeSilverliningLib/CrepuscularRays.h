// Copyright (c) 2013-2015 Sundog Software LLC. All rights reserved worldwide.

/**
\file CrepuscularRays.h
\brief A class that performs a postprocessing effect for shafts of light from clouds.
*/

#ifndef CREPUSCULAR_RAYS_H
#define CREPUSCULAR_RAYS_H

#include <FeSilverliningLib/MemAlloc.h>
#include <FeSilverliningLib/SilverLiningTypes.h>
#include <FeSilverliningLib/Sky.h>
#include <FeSilverliningLib/Vector3.h>
#include <FeSilverliningLib/Color.h>
#include <vector>

namespace SilverLining
{
    class Atmosphere;
    class VertexBuffer;
    class IndexBuffer;
    class CloudLayer;

    /** A class to render shafts of light from clouds. */
    class CrepuscularRays : public MemObject
    {
    public:
        /// Constructor; takes the Atmosphere to associate the effect with.
        CrepuscularRays(Atmosphere *atm);

        /// Virtual destructor
        virtual ~CrepuscularRays();

        /** Computes the position of the sun in screen space, and scales the intensity of the effect
            based on whether the sun is near the current view and the amount of cloud coverage present
            at the camera position.
            \param ephemeris the Ephemeris used to get the sun position.
            \param sunColor The color of the directional sunlight, used to color the crepuscular rays
            \param cloudCoverage The amount of sky covered by clouds (0-1) above the current camera position
        */
        void UpdateLightSource(const Ephemeris *ephemeris, const Color& sunColor, double cloudCoverage);

        /** Call to render the current blended object list into our offscreen texture that collects things
            that occlude the sun. */
        bool DrawToRenderTexture();

        /** Draws the actual 2D effect; UpdateLightSource() and DrawToRenderTexture() must have been called previously. */
        bool DrawRays();

        /** Set whether we are in a geocentric coordinate system. */
        void SetGeocentricMode(bool pGeocentric) {geocentric = pGeocentric;}

        /** Flag to indicate DrawToRenderTexture() is currently drawing objects into its offscreen texture.  Can be
            used to prevent drawing of objects that shouldn't contribute to rays (such as cirrus clouds.) */
        bool IsRendering() const {return isRendering;}

        /** Set the overall intensity of the effect at runtime; 1.0 maps to the default value of crepuscular-rays-exposure
            in SilverLining.config, while 0 results in no intensity. */
        void SetExposure(float exp) {exposureScale = exp;}

        /** Retrieves the overall intensity of the effect, as set by CrepuscularRays::SetExposure(). */
        float GetExposure() const {return exposureScale;}

        /** Returns the native texture pointer for the cloud texture against a neutral background. */
        void *GetNativeCloudTexture();

    protected:
        Vector3 ComputeSunPosClip(const Vector3& sunWorld, bool& clipped);

        Atmosphere *atmosphere;
        RenderTextureHandle renderTexture;
        ShaderHandle shader;
        VertexBuffer *vertexBuffer;
        IndexBuffer *indexBuffer;
        bool cleared, geocentric;
        Vector3 sunPosClip, sunPosTex, lightColor;
        float effectScale;
        float exposure, decay, density, weight, exposureScale;
        int texSize;
        bool isRendering;
        double sunDistance;
    };
}

#endif
