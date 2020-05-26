// Copyright (c) 2014 Sundog Software. All rights reserved worldwide.

/**
    \file ToweringCumulusCloud.h
    \brief An implementation of CumulusCloud that draws very large cumulus clouds in the process of becoming a thunderstorm.
 */

#ifndef TOWERING_CUMULUS_CLOUD_H
#define TOWERING_CUMULUS_CLOUD_H

#include <FeSilverliningLib/CumulusCloud.h>

namespace SilverLining
{
/** An implementation of CumulusCloud that draws very large cumulus clouds in the process of becoming a thunderstorm. */
    class ToweringCumulusCloud : public CumulusCloud
{
public:
/** Constructor. */
    ToweringCumulusCloud(CloudLayer *layer) : CumulusCloud(layer) {
    }

/** Destructor. */
    virtual ~ToweringCumulusCloud() {
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
