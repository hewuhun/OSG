// Copyright (c) 2009-2015 Sundog Software, LLC. All rights reserved worldwide.

/** \file Rain.h
   \brief Renders rain effects.
 */

#ifndef RAIN_H
#define RAIN_H

#if defined(WIN32) || defined(WIN64)
#pragma warning (disable: 4786)
#endif

#include <FeSilverliningLib/Vector3.h>
#include <FeSilverliningLib/Frustum.h>
#include <FeSilverliningLib/Renderer.h>
#include <FeSilverliningLib/Atmosphere.h>
#include <FeSilverliningLib/Color.h>
#include <FeSilverliningLib/Precipitation.h>
#include <vector>

// Raindrop sizes sampled from 1-5mm at 0.5mm increments
#define NUM_RAINDROP_DIAMETERS 9
#define MIN_RAINDROP_DIAMETER 1.0
#define RAINDROP_DIAMETER_INCREMENT 0.5

#define NUM_RAIN_TEXTURE_ANGLES 5
#define RAIN_TEXTURE_ANGLE_INCREMENT 20
#define NUM_RAIN_TEXTURE_OSCILLATIONS 10

namespace SilverLining
{
class VertexBuffer;
class IndexBuffer;

/** An individual rain streak. */
class RainParticle : public MemObject
{
public:
/** Default constructor; does nothing. */
    RainParticle() {
    }

/** Randomly positions the particle within a radius surrounding the position
   passed in. */
    inline void InitializeParticlePosition(const Vector3& camPos, int idxOffset);

/** The current location of this raindrop (center of the streak). */
    Vector3 pos;
};

/** Manages the physical simulation of rain and its rendering. */
class Rain : public Precipitation
{
public:
/** Constructor, takes in the scene's Atmosphere object. */
    Rain(const Atmosphere *atm);

/** Virtual destructor. */
    virtual ~Rain();

/** Set the simulated rainfall rate, in millimeters per hour. Reasonable values
   would range from 1.0 to 30.0. The intensity will affect visibility, particle
   size distribution, number of particles, and particle velocity.

   \param R rainfall rate, in mm/hr.
 */
    virtual void SetIntensity(double R);

/** Set the extents of the precipitation effect in terms of distance from the camera. The near clipping
    plane will be adjusted to encompass fNear while rendering the precipitation. */
    virtual void SetEffectRange(double fNear, double fFar, bool bUseDepthBuffer);

/** Renders the rain effect.

   \param dt The frame time, in seconds
   \param f  The view frustum, used for culling.
   \param lightColor The dominant light color, used for lighting the rain particles.
 */
    virtual void Render(double dt, const Frustum& f, const Color& lightColor);

private:
    void UpdateParticles(const Vector3& motion, const Vector3& dp, int start, int end);
    void InitializeBuffers();
    void InitializePositions();
    void LoadStreakTextures();
    void LoadStreakColors();
    void ComputeOscillationTimes();

    int maxParticles;
    double maxIntensity;
    double rainConstantAlpha;
    double rainStreakFrameTime;
    bool rainStreakCameraCoords;

    VertexBuffer *vertexBuffer;
    IndexBuffer *indexBuffer;
    SL_VECTOR(RainParticle) particles;

    Vector3 previousCamPos;

    int nDrops[NUM_RAINDROP_DIAMETERS];
    double dropDiameters[NUM_RAINDROP_DIAMETERS]; // millimeters
    double terminalVelocities[NUM_RAINDROP_DIAMETERS]; // m/s
    double oscillationTimes[NUM_RAINDROP_DIAMETERS]; // s

    TextureHandle rainTextures[NUM_RAIN_TEXTURE_ANGLES][NUM_RAIN_TEXTURE_OSCILLATIONS];
    Color rainColors[NUM_RAIN_TEXTURE_ANGLES][NUM_RAIN_TEXTURE_OSCILLATIONS];
    double angleThresholds[NUM_RAIN_TEXTURE_ANGLES];
    const Atmosphere *atmosphere;

    double visibilityMultiplier, minParticlePixels;

    int maxRenderedParticles;

    float rainAlphaThreshold, rainWidthMultiplier;

    double lastComputedIntensity, lastComputedFogDensity;

    double rainVelocityFactor;

    Color rainColor;
};
}

#endif
