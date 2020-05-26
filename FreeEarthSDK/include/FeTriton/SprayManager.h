// Copyright (c) 2011-2014 Sundog Software LLC. All rights reserved worldwide.

/**
    \file SprayManager.h
    \brief Manages spray particle effects from breaking waves.
*/

#ifndef TRITON_SPRAYMANAGER_H
#define TRITON_SPRAYMANAGER_H

#include <FeTriton/TritonCommon.h>
#include <FeTriton/Vector3.h>
#include <vector>

namespace Triton
{
class Environment;
class ProjectedGrid;
class ParticleSystemManager;
class ParticleSystem;
class TBoundingBox;

/** Manages the spray particle effects resulting from breaking waves. */
class SprayManager : public MemObject
{
public:
    /** Constructor, must be associated with an Environment.
        \param env The Environment this SprayManager is associated with. */
    SprayManager(const Environment *env);

    /** Virtual destructor. */
    virtual ~SprayManager();

    /** Initializes the underlying particle systems and associates the SprayManager with a ProjectedGrid.
        \param grid The ProjectedGrid that represents the ocean surface this class will add spray effects to.
        \return True if the SprayManager initialized successfully.
    */
    virtual bool Initialize(const ProjectedGrid *grid);

    /** Updates and draws the spray particles.
        \param time The simulated time, in seconds.
        \param waveHeading The direction of the waves in which the spray will head, in radians.
        \return True if the particles drew successfully.
    */
    //JWH - added override for patch bounds
    virtual bool Draw(double time, float waveHeading, const TBoundingBox* patchBounds = 0);

    /** Must be called when a Direct3D9 device is lost. */
    virtual void D3D9DeviceLost() {}

    /** Must be called when a Direct3D9 device is reset. */
    virtual void D3D9DeviceReset() {}

    /** Retrieve the underlying shader object for the spray particles. */
    virtual ShaderHandle GetShader() const {
        return 0;
    }

    /** OpenGL only: reload shader programs linking in a new user list of shader objects. */
    virtual bool ReloadShaders(const TRITON_VECTOR(unsigned int)& shaders) {
        return true;
    }

protected:
    bool ReadConfigSettings();

    const Environment *environment;
    const ProjectedGrid *grid;
    ParticleSystem *system;

    bool sprayEnabled;
    int numSprays;
    float sprayDirectionVariance, sprayVelocity;
    float sprayVelocityVariance, sprayPositionVariance;
    float sprayParticleSize, sprayParticleSizeVariance;
    float sprayTransparency, sprayOriginDepth;
    float sprayGravity, sprayCullDistance;
    float sprayPositionLead;
    int spraySparsity, sprayBudget;
    TRITON_VECTOR(Vector3) sprayPositions;
};
}

#endif