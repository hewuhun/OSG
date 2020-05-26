// Copyright (c) Sundog Software LLC 2008-2014, All rights reserved worldwide.

/**
    \file PrecipitationManager.h
    \brief Set and retrieve aggregate precipitation effects.
 */

#ifndef PRECIPITATION_MANAGER_H
#define PRECIPITATION_MANAGER_H

#include <FeSilverliningLib/Color.h>
#include <FeSilverliningLib/Frustum.h>
#include <map>

namespace SilverLining
{
class Atmosphere;
class Rain;
class Snow;
class Sleet;

/** Provides a common point of control for rain and snow effects. This is a singleton. */
class PrecipitationManager : public MemObject
{
public:
/** Retrieves the single, static instance of the precipitation manager. */
    static PrecipitationManager *GetInstance(Atmosphere *atm);

/** Releases all resources and destroys the instance. */
    static void Release(Atmosphere *atm);

/** Reloads the precipitation shader, re-linking in any user shaders. */
    static void ReloadShader();

/** Retrieves the precipitation shader program. */
    static ShaderHandle GetShaderProgram();

/** Initializes rain and snow effects.

   \param atm A pointer to the Atmosphere object for the scene.
 */
    void Initialize(Atmosphere *atm);

/** Destructor. */
    ~PrecipitationManager();

/** Stops all precipitation effects. */
    void ClearPrecipitation();

/** Returns the total density term for exponential fog, based on the visibility reduction
   from precipitation. */
    double GetFogDensity();

/** Sets the intensity, in millimeters per hour (liquid equivalent) of precipitation
   of a given type.

   \param precipType Either CloudLayer::RAIN, CloudLayer::DRY_SNOW, CloudLayer::WET_SNOW,
                      or CloudLayer::SLEET.
   \param mmPerHour Liquid equivalent precipitation rate. Reasonable values
   range from 1.0 to 30.0.
 */
    void SetIntensity(int precipType, double mmPerHour);

/** Sets the near and far extents of the precipitation effect as distances from the camera. */
    void SetClipPlanes(int precipType, double nearClip, double farClip, bool useDepthBuffer);

/** Renders all precipitation effects that have a non-zero intensity.
   \sa SetIntensity()

   \param dt The frame time, in seconds.
   \param f  The view frustum.
   \parma lightColor The dominant light color
 */
    void Render(double dt, const Frustum& f, const Color& lightColor);

/** Sets the current camera in use, so we can have separate particle systems for each. */
    void SetCurrentCamera(CameraHandle cam);

private:
    PrecipitationManager(Atmosphere *atm) : atmosphere(atm), currentCamera(0) {
    }

    Rain *GetRain(CameraHandle cam);
    Snow *GetSnow(CameraHandle cam);
    Sleet *GetSleet(CameraHandle cam);

    SL_MAP(CameraHandle, Rain *) rainSystems;
    SL_MAP(CameraHandle, Snow *) snowSystems;
    SL_MAP(CameraHandle, Sleet *) sleetSystems;

    static SL_MAP(Atmosphere *, PrecipitationManager *) instances;

    CameraHandle currentCamera;
    Atmosphere *atmosphere;
};
}

#endif
