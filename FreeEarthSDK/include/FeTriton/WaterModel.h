// Copyright (c) 2011-2014 Sundog Software, LLC. All rights reserved worldwide.

#ifndef TRITON_WATERMODEL_H
#define TRITON_WATERMODEL_H

/** \file WaterModel.h
   \brief An interface for physical models of water waves.
 */

#include <FeTriton/TritonCommon.h>
#include <FeTriton/Vector3.h>
#include <FeTriton/Matrix3.h>

namespace Triton
{
class Environment;
class ProjectedGrid;
class FFT;

/** An interface for physical models of water waves. */
class WaterModel : public MemObject
{
public:
    /** Constructor takes your Environment object, so it knows where to find its
        simulation settings and resources.*/
    WaterModel(const Environment *env);

    /** Virtual destructor. */
    virtual ~WaterModel() {}

    /** Initialize the water model by associating it with a ProjectedGrid class's
        shaders; it will provide the uniform parameters needed to manipulate the
        projected grid. */
    virtual bool Initialize(ShaderHandle pShader, ShaderHandle pPatchShader, bool enableHeightReads, bool noGPU) {
        shader = pShader;
        patchShader = pPatchShader;
        return true;
    }

    /** Updates any underlying physical simulation of the water. */
    virtual bool UpdateSimulation(double time) {
        return true;
    }

    /** Call prior to rendering the projected grid in order to apply the simulation's
        uniform parameters, at the given time sample (in seconds. ) */
    virtual bool PrepShaders(double time) {
        return true;
    }

    /** Call prior to rendering user geometry in order to apply the simulation's uniform
        parameters, at the given time sample (in seconds.) */
    virtual bool PrepPatchShader(double time) {
        return true;
    }

    /** Retrieves the maximum wave amplitude, in world units. This may be an estimate or upper bound
        as opposed to the actual maximum for the previous frame. */
    virtual double GetMaxAmplitude() const {
        return maxAmplitude;
    }

    /** Retrieves the choppiness parameter set by SetChoppiness() */
    float GetChoppiness() const {
        return chopScale;
    }

    /** Set the simulated choppiness of the waves (how pointy the wave peaks are.)
        May range from 0 to around 3.0 before the waves will start intersecting themselves. */
    void SetChoppiness(float chop) {
        chopScale = chop;
    }

    /** Gets the simulated depth of the ocean, in world units, as set by SetDepth(). */
    float GetDepth(Vector3& floorNormal) const;

    /** Set the simulated depth of the water, in world units. */
    void SetDepth(float pDepth, const Vector3& floorNormal) {
        depth = pDepth;
        seaFloorNormal = floorNormal;
    }

    /** Gets the simulated slope of the ocean floor. */
    float GetSlope(float& heading) const {
        heading = slopeHeading;
        return slope;
    }

    void SetWaveHeading(float heading) {
        waveHeading = heading;
    }

    float GetWaveHeading() const {
        return waveHeading;
    }

    void SetLoopingPeriod(float period) {
        loopingPeriod = period;
    }

    float GetLoopingPeriod() const {
        return loopingPeriod;
    }


    /** Sets the simulated slope and direction of the slope of the ocean floor. */
    void SetSlope(float pSlope, float pHeading) {
        slope = pSlope;
        slopeHeading = pHeading;
    }

    /** Called to handle lost devices under DirectX9, prior to a device reset. This will normally
        be called by ProjectedGrid's own method. */
    virtual bool D3D9DeviceLost() {
        return false;
    }

    /** Called to handle a device reset to restore Triton's resources following the reset. This will
        normally be called by ProjectedGrid's own method. */
    virtual bool D3D9DeviceReset() {
        return false;
    }

    /** Retrieves a human-readable description of the water model's underlying FFT method (for example,
        OpenCL, Compute Shader, CUDA) if the water model is FFT based. */
    virtual const char *GetFFTName() const {
        return "Not using FFT Water Model";
    }

    /** Returns the underlying FFT object, if any. */
    virtual const FFT *GetFFT() const {
        return 0;
    }

    /** Retrieves the current height at a given point on the ocean's surface from the water model.
        Does not take ship wakes into account.
        \param position The position on the ocean surface to test
        \param grid The ProjectedGrid to use for the surface
        \param normal Returns the normal vector of the surface at the position given
        \return The height of the ocean surface at the position specified (above sea level)
        */
    virtual float GetHeight(const Vector3& position, const ProjectedGrid *grid, Vector3& normal, bool highResolution) = 0;

    /** Sets the distance at which 3D wave displacements are dampened to prevent aliasing when moving the camera. */
    void SetDisplacementDampingDistance(double distance) {
        displacementDampingDistance = (float)distance;
    }

    /** Retrieves the distance at which 3D wave displacements are dampened to prevent aliasing when moving the camera. */
    float GetDisplacementDampingDistance() const {
        return displacementDampingDistance;
    }

    /** Reloads uniform locations from the Projected Grid shaders when that shader is reloaded. */
    virtual void ReloadGridUniforms(ShaderHandle gridShader, ShaderHandle gridPatchShader) {
        shader = gridShader;
        patchShader = gridPatchShader;
    }

    virtual void EnableSpray(bool enable) {
        sprayEnabled = enable;
    }

    virtual void GetTextures(TextureHandle& slopeFoam, TextureHandle& displacement) {
        slopeFoam = 0;
        displacement = 0;
    }

protected:
    float chopScale;
    float depth, slope, slopeHeading;
    float loopingPeriod;
    Vector3 seaFloorNormal;
    const Environment *environment;
    ShaderHandle shader, patchShader;
    double maxAmplitude;
    float waveHeading;
    float displacementDampingDistance;
    bool sprayEnabled;
};
}

#endif