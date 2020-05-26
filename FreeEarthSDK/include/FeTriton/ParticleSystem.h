// Copyright (c) 2011-2014 Sundog Software LLC. All rights reserved worldwide.

#ifndef TRITON_PARTICLE_SYSTEM_H
#define TRITON_PARTICLE_SYSTEM_H

/** \file ParticleSystem.h
   \brief An individual particle system updated on the GPU.
 */

#include <FeTriton/TritonCommon.h>
#include <FeTriton/Vector3.h>
#include <list>
#include <vector>

namespace Triton
{
class Environment;
class ProjectedGrid;

/** An individual particle system affected by gravity */
class ParticleSystem : public MemObject
{
public:
    /** Constructor.
        \param env The Environment to associate this ParticleSystem with.
    */
    ParticleSystem(const Environment *env);

    /// Destructor
    virtual ~ParticleSystem() {}

    /** Performs any expensive initialization for the particle system. Must be called prior
        to calling Start() and Draw().
        \param texture The texture to draw the particles with.
        \param shader The shader to use to position and draw the particles.
        \param numParticles The number of particles managed by this object.
        \return True if initialization succeeded and this object may be used.
    */
    virtual bool Initialize(TextureHandle texture, ShaderHandle shader, int numParticles, const ProjectedGrid *grid) = 0;

    /** Launches the particles at the given position, direction, size, and velocity with
        specified variances.
        \param pStartTime The timestamp to begin the particle system at, in seconds.
        \param direction The average direction vector (normalized) of the particles.
        \param dirVariation The percent variation of the direction (ie, 0.2 = 20%)
        \param velocity The average velocity of the particles
        \param velVariation The percent variation of the velocity
        \param position The average world position of the particles
        \param posVariation The maximum variance on the position in world units
        \param size The average size of the particles, approximately in world units
        \param sizeVariation The percent variation in the particle sizes.
    */
    virtual void Start(double pStartTime, const Vector3& direction, float dirVariation,
                       float velocity, float velVariation, const Vector3& position, float posVariation,
                       float size, float sizeVariation) = 0;

    /** Adds a vector of particle positions to a larger pool of particles.
        \param pStartTime The start time to associate with the newly created particles.
        \param positions A Vector of initial positions for the new particles.
        \param direction The average normalized direction of the particles.
        \param dirVariation The percent variation in the direction
        \param velocity The average velocity of the particles
        \param velVariation The percent variation in the velocity
        \param posVariation The maximum variance of the position in world units
        \param size The average size of the particles, approximately in world units
        \param sizeVariation The maximum variance of the size
    */
    virtual void StartBatch(double pStartTime, const TRITON_VECTOR(Vector3)&positions,
                            const Vector3& direction, float dirVariation, float velocity, float velVariation,
                            float posVariation, float size, float sizeVariation) = 0;

    /** Sets up shaders, etc common to all particle systems for this frame. Call prior to Draw().
        \param time The time to simulate, in seconds.
    */
    virtual void SetupCommonState(double time) {}

    /** Draws the particle system for the given time sample. Call SetupCommonState() first, then Draw() (perhaps many times),
        then CleanupCommonState().
        \return True if the particle drew successfully.
    */
    virtual bool Draw() = 0;

    /** Cleans up state set with SetupCommonState(). Call after making Draw() calls for a given frame. */
    virtual void CleanupCommonState() {}

    /** Returns the time at which this particle system may be discarded.
    \return The time, in seconds, after which all particles are guaranteed to have falled below their initial position.
    */
    double GetExpirationTime() const {
        return expirationTime;
    }

    /** Sets the initial transparency of the particles.
        \param t The initial transparency; 0 = transparent, 1 = opaque.
    */
    void SetTransparency(float t) {
        transparency = t;
    }

    /** Retrieves the initial transparency of the particles. 0 = transparent, 1 = opaque. */
    float GetTransparency() const {
        return transparency;
    }

    /** Sets the force of gravity on the particles, in meters per second squared. */
    void SetGravity(float g) {
        gravity = g;
    }

    /** Retrieves the force of gravity on the particles, in meters per second squared. */
    float GetGravity() const {
        return gravity;
    }

protected:
    void ComputeExpirationTime(double startTime, double maxP0, double maxV0);

    void GetLocalReference(Vector3& referencePoint, Vector3& offsetFromCamera);

    double startTime, expirationTime;
    const Environment *environment;
    const ProjectedGrid *grid;

    static double referenceTime;
    int offset;
    float transparency, gravity, invSizeFactor;

    bool pureAdditive, alphaMask;
};

/** A class to sort and draw a collection of ParticleSystem objects. */
class ParticleSystemManager
{
public:
    /** Constructor. */
    ParticleSystemManager() {}

    /** Virtual destructor. */
    virtual ~ParticleSystemManager();

    /** Adds a particle system to be managed by this class.
        \param ps The ParticleSystem to add.
    */
    void AddParticleSystem(ParticleSystem *ps);

    /** Updates and draws all particle systems for the given time.
        \param time The time, in seconds, to simulate.
    */
    void Update(double time);

    /** Clears all existing particle systems. */
    void ClearParticleSystems();

protected:
    TRITON_LIST(ParticleSystem *) particleSystems;
};
}

#endif