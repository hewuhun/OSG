// Copyright (c) 2011-2015 Sundog Software, LLC. All rights reserved worldwide.

#ifndef TRITON_WAKE_GENERATOR_H
#define TRITON_WAKE_GENERATOR_H

#ifdef SWIG
%module TritonWakeGenerator
%import "Ocean.h"
#define TRITONAPI
%{
#include <FeTriton/WakeGenerator.h>
using namespace Triton;
%}
#endif

#include <FeTriton/TritonCommon.h>
#include <FeTriton/Vector3.h>
#include <FeTriton/Export.h>

/** \file WakeGenerator.h
    \brief An object that generates a ship Kelvin wake as it moves.
*/

#pragma pack(push)
#pragma pack(8)

namespace Triton
{
class Ocean;

/** WakeGeneratorParameters contains the parameters required to construct a Triton::WakeGenerator object. */
class FETRITON_EXPORT WakeGeneratorParameters : public MemObject
{
public:
    /** The constructor populates all data members with reasonable default values, although you will at a minimum
        probably want to specify the sprayEffects, length, beamWidth, and bowOffset. */
    WakeGeneratorParameters();

    // Bow spray settings:
    bool   sprayEffects;            ///< Whether you wish this wake to emit spray particles originating from this wake generator.

    double bowSprayOffset;          ///< Use this to have spray particles emitted from a point different
                                    ///< from  the ship position. This distance will be added
                                    ///< to the current wake generator's position along the direction of travel.
                                    ///< Unused if sprayEffects is false.

    double sprayVelocityScale;      ///< A scaling factor for spray effects at the bow of the ship; this is applied to the 
                                    ///< initial velocity of the spray particles.

    double spraySizeScale;          ///< A scaling factor applied to the size of the bow spray particles and their random spread.

    // Kelvin wake settings:
    bool bowWave;                   ///< Whether the bow wave effect should exist on this object.

    double bowWaveOffset;           ///< Offset from the ship position, along the direction of travel, at which bow waves will
                                    ///< originate.

    double bowWaveScale;            ///< A scaling factor for the bow wave's amplitude at the bow of the ship.

    double bowWaveMax;              ///< The maximum amplitude of the bow wave, or -1.0 for unbounded.

    double bowSize;                 ///< The size of the bow in world units; affects the wavelength of the bow wave, and the
                                    ///< initial spread of spray particles at the bow. For a pointy bow, use a value of 0.
                                    ///< Generally you'd only use this for things like ferries or LCACs.

    bool sternWaves;                ///< Whether stern wakes should be generated from this object.

    double sternWaveOffset;         ///< Offset for the stern wakes.

    // Ship geometry:
    double length;                  ///< The length of the object generating the wake.

    double beamWidth;               ///< The width of the object generating the wake.

    double draft;                   ///< The "draft" of the ship, or the depth the hull extends to underwater. This affects
					                ///< the size of the bow wake.

    // Prop wash / tubulent wake settings:
    bool   propWash;                ///< Whether you want propeller backwash effects generated from this wake generator.

    double propWashOffset;          ///< Use this to have propeller backwash effects generated from a point 
                                    ///< different from the ship position.

    double propWashWidthMultiplier; ///< Multiplies the beam width by this value to arrive at the width of the prop wash effect.

    double propWashFadeTime;        ///< The amount of time, in seconds, over which the prop wash will dissipate.

    // Hull spray settings:
    int    numHullSprays;           ///< How many spray particle systems are emitted periodically along the hull of the ship. Set 
                                    ///< to 0 to disable hull sprays.

    double hullSprayStartOffset;    ///< The offset at which hull spray effects begin.

    double hullSprayEndOffset;      ///< The offset at which hull spray effects end.

    double hullSprayScale;          ///< The initial velocity of hull spray particles as a percent of the ship velocity (0-1)

    double hullSpraySizeScale;      ///< A scaling factor applied to the hull spray particles.

    double hullSprayVerticalOffset;///< A vertical offset to the starting point of new hull spray particles.

	double bowWakeMultiplier;		///< A scaling factor applied to the bow Kelvin wake effect.
	
	double sternWakeMultiplier;		///< A scaling factor applied to the stern Kelvin wake effects.
};

/** A WakeGenerator represents an object on the water that generates a wake as it moves, such as a
    ship. Simply call Triton::WakeGenerator::Update() to move the object and generate a realistic wake behind it.
    Any WakeGenerator moving at constant velocity will generate a wake of 19.46 degrees behind it,
    but acceleration, deceleration, and curved paths are all handled properly as well. */
class FETRITON_EXPORT WakeGenerator : public MemObject
{
public:
    /** Construct a WakeGenerator with the same Triton::Ocean it will be associated with. This form of the 
        constructor takes a WakeGeneratorParameters class instead of a long list of parameters, and is
        the preferred constructor to ensure code readability and prevent errors from misplaced parameters.

        \param pOcean The Triton::Ocean object you will associate this WakeGenerator with. A common error is to
                      create a WakeGenerator before the Ocean has been created, so make sure this is a valid,
                      non-null pointer.
        \param parameters A Triton::WakeGeneratorParameters object containing the properties of this WakeGenerator.
    */
    WakeGenerator(Ocean *pOcean, const WakeGeneratorParameters& parameters);

	virtual ~WakeGenerator();

    /** For any active WakeGenerator, this should be called every frame to update its
        position and velocity. No wake will be generated until this is called.
        \param pPosition The position of the object generating the wake, such as the stern of a ship,
                         in world coordinates.
        \param pDirection A normalized direction vector indicating the direction this object is moving in.
        \param pVelocity The velocity of the object generating the wake, in world units per second.
        \param pTime     The current simulated time sample, in seconds. This should be realtive to the same
		                 start time used for the time stamps sent into Triton::Ocean::Draw().
    */
    void TRITONAPI Update(const Vector3& pPosition, const Vector3& pDirection, double pVelocity, double pTime);

    /** Clears all previously emitted wakes from the Ocean. */
    void ClearWakes();

    /** Retrieves the world position of the WakeGenerator.
        \return The world position of the WakeGenerator, as last specified by Triton::WakeGenerator::Update().
    */
    Vector3 TRITONAPI GetPosition() const {
        return position;
    }

    /** Retrieves the world position of the point where stern wakes originate from. */
    Vector3 TRITONAPI GetSternPosition() const {
        return sternPosition;
    }

    /** Retrieves the velocity of the WakeGenerator.
        \return The velocity of the WakeGenerator in world units per second, as last specified by
                Triton::WakeGenerator::Update().
    */
    double TRITONAPI GetVelocity() const {
        return velocity;
    }

    /** Retrieves whether propeller backwash was enabled for this wake generator. */
    bool TRITONAPI HasPropWash() const {
        return params.propWash;
    }

    /** Sets the distance at which the number of visible wake waves will be halved. For example,
        if a WakeGenerator has built up 8 wake waves and propeller wash segments and you pass a
        distance of 1000 meters, all 8 will be visible when the camera is closer than 1km to the
        WakeGenerator's position. Between 1 and 2km, every other wave will be skipped, resulting in
        4 waves. From 3 to 4km, you'll get 2, which is as low as it will go. 

        Set this to 0 to disable LOD, which is the default. */
    void TRITONAPI SetLODDistance(double distance) {
        lodDistance = distance;
    }

    /** Retrieves the LOD distance set in Triton::WakeGenerator::SetLODDistance(), or 0 if LOD is
        disabled. */
    double TRITONAPI GetLODDistance() const {
        return lodDistance;
    }

    /** Returns the current parameters for this WakeGenerator. */
    const WakeGeneratorParameters& TRITONAPI GetParameters() const;

    /** Set this WakeGenerator's parameters using the WakeGeneratorParameters provided. */
    void TRITONAPI SetParameters(const WakeGeneratorParameters& parameters);

    // Used internally to clear out the ocean reference when it is deleted.
    void OceanDestroyed() {
        ocean = 0;
        registered = false;
    }

    // Used internally to clear out the wake manager reference when it is deleted.
    void WakeManagerDestroyed() {
        registeredWakeManager = false;
    }

    // Used internally to keep track of the current max wake number.
    int GetMaxWakeNumber() const {
        return wakeNumber - 1;
    }

    // Used internally for fading prop wash effects.
    double GetDistanceTravelled() const {
        return distanceTravelled;
    }

    // Return the Ocean object this wake is attached to (if any)
    // Used internally.
    const Ocean *GetOcean() const {
       return ocean;
    }

protected:
    Vector3 position, sternPosition, lastPosition, washPosition, lastEmitPosition, lastLastEmitPosition;
    Vector3 lastEmitSourcePosition, lastWakePosition;
    double velocity;
    double distanceTravelled;
    Ocean *ocean;
    double lastSprayEmitTime;
    double lodDistance, hullSpeed;
    double sprayCullDist;
    bool firstEmit, registered, registeredWakeManager;
    int wakeNumber, lastWakeNumber;
    double minPropSegmentLength, decayRate;
    double sprayPositionVariation, curveGenerationFactor;
    double gravity, maxDistanceSquared;
    int maxWakes;

    WakeGeneratorParameters params;
};
}

#pragma pack(pop)

#endif