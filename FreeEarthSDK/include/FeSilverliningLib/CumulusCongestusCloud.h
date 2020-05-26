// Copyright (c) 2006-2009 Sundog Software. All rights reserved worldwide.

/**
    \file CumulusCongestusCloud.h
    \brief An implementation of CumulusCloud that draws larger, flat-bottomed cumulus clouds.
 */

#ifndef CUMULUS_CONGESTUS_CLOUD_H
#define CUMULUS_CONGESTUS_CLOUD_H

#include <FeSilverliningLib/CumulusCloud.h>

namespace SilverLining
{
/** An implementation of CumulusCloud that draws larger, flat-bottomed cumulus clouds. */
class CumulusCongestusCloud : public CumulusCloud
{
public:
/** Constructor. */
    CumulusCongestusCloud(CloudLayer *layer) : CumulusCloud(layer) {
    }

/** Destructor. */
    virtual ~CumulusCongestusCloud() {
    }

/** Increments the cellular automata that drives cloud creation and growth.
   Cumulus Congestus clouds favor upward growth within a half-ellipse shape. */
    virtual void IncrementTimeStep(unsigned long now);

/** The voxel positions will be "blown" off-center by the current wind vector.
    \param wind The current wind vector, in world units per second.
 */
    virtual void ApplyWindShear(const Vector3& wind);

protected:
/** Seeds the initial conditions of the cellular automata such that it'll
   end up looking like a cumulus congestus cloud. */
    virtual void InitializeVoxelState();
};
}

#endif
