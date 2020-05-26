// Copyright (c) 2009 Sundog Software LLC, All rights reserved worldwide

/**
    \file Precipitation.h
    \brief Interface for snow and rain effects.
 */

#ifndef PRECIPITATION_H
#define PRECIPITATION_H

#if defined(WIN32) || defined(WIN64)
#pragma warning (disable: 4786)
#endif

#include <FeSilverliningLib/Frustum.h>
#include <FeSilverliningLib/Color.h>
#include <FeSilverliningLib/SilverLiningTypes.h>

namespace SilverLining
{
/** An interface for rain and snow effects overlaid on the scene. */
class Precipitation : public MemObject
{
public:
/** Default constructor. */
    Precipitation() : visibility(0), fogDensity(0), intensity(-1), nearClip(0.5), farClip(8.0), useDepthBuffer(false) {
    }

/** Virtual destructor. */
    virtual ~Precipitation() {
    }

/** Sets the intensity of the precipitation in millimeters per hour.
   In the case of snow, this is the liquid equivalent, not snow
   accumulation rates. Reasonable ranges would be 1.0 - 30.0.
   This value will affect visibility, the distribution of particle sizes,
   the velocities of the particles, and the number of particles.

   \sa GetIntensity()

   \param R Liquid equivalent precipitation rate, in millimeters per hour.
 */
    virtual void SetIntensity(double R) = 0; // millimeters of rainfall per hour

/** Retrieves the current simulated precipitation rate, in millimeters per hour
   (liquid equivalent.)

   \sa SetIntensity()
 */
    double GetIntensity() const {
        return intensity;
    }

/** Renders the precipitation effect. Should be one of the last things rendered
   in the scene.

   \param dt The time for this frame, in seconds. This affects the length of
     precipitation streaks and particle motion.
    \param f  The current view frustum, used for culling particles.
   \param lightColor The dominant light color that will modulate the particle colors.
 */
    virtual void Render(double dt, const Frustum& f, const Color& lightColor) = 0;

/** Returns the simulated visibility, in meters. Depends on SetIntensity() being
   called. */
    double GetVisibility() const {
        return visibility;
    }

/** Returns the current density term for exponential fog, based on the simulated visibility.
   Dependson SetIntensity() being called.

   \sa GetVisibility()
   \sa SetIntensity();
 */
    double GetFogDensity() const {
        return fogDensity;
    }

/** Set the extents of the precipitation effect in terms of distance from the camera. The near clipping
    plane will be adjusted to encompass fNear while rendering the precipitation. */
    virtual void SetEffectRange(double fNear, double fFar, bool bUseDepthBuffer) {
        nearClip = fNear;
        farClip = fFar;
        useDepthBuffer = bUseDepthBuffer;
    }

/** Gets the extents of the precipitation effect in terms of distances from the camera. */
    void GetEffectRange(double& fNear, double& fFar, bool &bUseDepthBuffer) const {
        fNear = nearClip;
        fFar = farClip;
        bUseDepthBuffer = useDepthBuffer;
    }

    static ShaderHandle precipShader;

protected:
    double visibility, fogDensity, intensity;
    double nearClip, farClip;
    bool useDepthBuffer;
};
}

#endif
