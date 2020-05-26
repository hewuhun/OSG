// Copyright (c) 2004-2012  Sundog Software, LLC. All rights reserved worldwide.

/**
    \file Voxel.h
    \brief A single cell in the cellular automata that comprises a CumulusCloud.
 */

#ifndef VOXEL_H
#define VOXEL_H

#include <FeSilverliningLib/Metaball.h>
#include <FeSilverliningLib/Color.h>
#include <FeSilverliningLib/Vector3.h>
#include <FeSilverliningLib/Frustum.h>
#include <bitset>
#include <stdio.h>
#include <iostream>

//#define USE_BITSET

namespace SilverLining
{
enum VoxelFlags
{
    hasCloud = 0,
    vapor,
    phaseTransition,
    lightning,
    created,
    destroyed,
    NUM_FLAGS
};

/** A single cell in the cellular automata that makes up a CumulusCloud. Extends a
   Metaball which models the actual lighting of this cell, which in turn contains a
   Billboard that performs the actual rendering.
 */
class Voxel : public Metaball
{
public:
/** Constructs a voxel of a specified size and density.
   \param dimension The CumulusCloud is broken into a 3D array of Voxels, which
   may be thought of as little cubes stacked against each other. The dimension
   is the length of any side of an individual cube, in world units.
   \param particleSize The diameter of the simulated particles that make up
   the cloud puff within this voxel, specifically the water droplets. This is
   used to compute the optical depth of this voxel.
   \param maxDensity The simulated density of the cloud puff, in particles per
   cubic world unit. This is also used to compute the optical depth of this
   voxel.
   \param spinRate The maximum rate of the voxel spin, in radians per second
 */
    Voxel(double dimension, double particleSize, double maxDensity, double spinRate, int atlasIdx = 0, bool rotate = true);

/** Restores a voxel from disk. */
    Voxel(std::istream& s);

/** Destructor; disposes of the Metaball contained by this class. */
    virtual ~Voxel();

/** Saves a voxel to disk. */
    bool Serialize(std::ostream& s);

/** Sets the position to use for sorting this voxel against other objects
   in the scene. This may be modified by SetOffsetPosition(). */
    static void SetSortFromPosition(const Vector3& pos) {
        sortPos = pos;
    }

/** Retrieves the position to use for sorting this voxel against other
   objects in the scene, as previously set by SetSortFromPosition(). It does
   not include an offset specified by SetOffsetPosition(). */
    static Vector3 GetSortFromPosition() {
        return sortPos;
    }

/** Sets an offset to be added to the sort position previously set by
   SetSortFromPosition(). */
    static void SetOffsetPosition(const Vector3& pos) {
        offsetPos = pos;
    }

/** Retrieves the offset to the sort position previously set by
   SetOffsetPosition(). */
    static Vector3 GetOffsetPosition() {
        return offsetPos;
    }

/** Enumeration of ways to sort voxels; either front-to-back or
   back-to-front. */
    enum SortingModes
    {
        FRONT_TO_BACK,
        BACK_TO_FRONT
    };

/** Specifies whether Voxel objects should be sorted by STL methods in front to back
   or back to front order.
   \param mode One of the SortingModes enumeration; either Voxel::FRONT_TO_BACK or
   Voxel::BACK_TO_FRONT.
 */
    static void SetSortingMode(SortingModes mode) {
        sortMode = mode;
    }

/** Returns the simulated radius of the cloud puff within this voxel, in world units. */
    static double ComputeVoxelRadius(double dimension) {
        return sqrt(2.0) * dimension;
    }

/** The less-than operator is overloaded to allow sorting of Voxel objects using STL
   methods. Sorting will be influenced by SetSortingMode(). */
    bool operator < (const Voxel& v);

/** Retreives whether this voxel contains a simulated cloud puff or not. */
    bool GetHasCloud() const {
#ifdef USE_BITSET
        return flags[hasCloud];
#else
        return (flags & (1 << hasCloud)) != 0;
#endif
    }

/** Sets whether this voxel contains a simulated cloud puff or not. */
    void SetHasCloud(bool b) {
#ifdef USE_BITSET
        flags.set(hasCloud, b);
#else
        if (b)
            flags |= (1 << hasCloud);
        else
            flags &= ~(1 << hasCloud);
#endif
    }

/** Gets whether this voxel is modeled to contain a high amount of water vapor. */
    bool GetVapor() const {
#ifdef USE_BITSET
        return flags[vapor];
#else
        return (flags & (1 << vapor)) != 0;
#endif
    }

/** Sets whether this voxel is modeled to contain a high amount of water vapor. */
    void SetVapor(bool b) {
#ifdef USE_BITSET
        flags.set(vapor, b);
#else
        if (b)
            flags |= (1 << vapor);
        else
            flags &= ~(1 << vapor);
#endif
    }

/** Gets whether this voxel is currently illuminated by lightning */
    bool GetHasLightning() const {
#ifdef USE_BITSET
        return flags[lightning];
#else
        return (flags & (1 << lightning)) != 0;
#endif
    }

/** Sets whether this voxel is currently illuminated by lightning */
    void SetHasLightning(bool b) {
#ifdef USE_BITSET
        flags.set(lightning, b);
#else
        if (b)
            flags |= (1 << lightning);
        else
            flags &= ~(1 << lightning);
#endif
    }

/** Gets whether this voxel is undergoing a "phase transition" from water vapor. */
    bool GetPhaseTransition() const {
#ifdef USE_BITSET
        return flags[phaseTransition];
#else
        return (flags & (1 << phaseTransition)) != 0;
#endif
    }

/** Sets whether this voxel is undergoing a "phase transition" from water vapor. */
    void SetPhaseTransition(bool b) {
#ifdef USE_BITSET
        flags.set(phaseTransition, b);
#else
        if (b)
            flags |= (1 << phaseTransition);
        else
            flags &= ~(1 << phaseTransition);
#endif
    }

/** Gets whether this voxel was created since the last iteration and should be fading in. */
    bool GetCreated() const {
#ifdef USE_BITSET
        return flags[created];
#else
        return (flags & (1 << created)) != 0;
#endif
    }

/** Sets whether this voxel was created since the last iteration. */
    void SetCreated(bool b) {
#ifdef USE_BITSET
        flags.set(created, b);
#else
        if (b)
            flags |= (1 << created);
        else
            flags &= ~(1 << created);
#endif
    }

    /** Gets whether this voxel was destroyed since the previous iteration and should be fading out. */
    bool GetDestroyed() const {
#ifdef USE_BITSET
        return flags[destroyed];
#else
        return (flags & (1 << destroyed)) != 0;
#endif
    }

/** Sets whether this voxel was destroyed since the previous iteration. */
    void SetDestroyed(bool b) {
#ifdef USE_BITSET
        flags.set(destroyed, b);
#else
        if (b)
            flags |= (1 << destroyed);
        else
            flags &= ~(1 << destroyed);
#endif
    }

/** Returns the optical depth of this voxel. */
    float GetOpticalDepth() const {
        return opticalDepth;
    }

private:

#ifdef USE_BITSET
    std::bitset<6> flags;
#else
    char flags;
#endif

    float opticalDepth;

    static Vector3 sortPos;
    static Vector3 offsetPos;
    static SortingModes sortMode;
};
}

#endif
