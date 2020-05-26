// Copyright (c) 2004-2011  Sundog Software, LLC All rights reserved worldwide.

/**
    \file Frustum.h
    \brief A viewing frustum, modeled as a collection of six planes.
 */
#ifndef TRITON_FRUSTUM_H
#define TRITON_FRUSTUM_H

#include <FeTriton/Plane.h>

namespace Triton
{
/** A simple class that just collects six planes together and calls
   it a frustum. */
class Frustum : public MemObject
{
public:

    /** Default constructor. */
    Frustum() : nCullingPlanes(6) {
    }

    /** Destructor; does nothing. */
    ~Frustum() {
    }

    /** Identifiers for each of the six planes that make up the viewing
       frustum. */
    enum Planes {

        PNEAR = 0,
        PLEFT,
        PRIGHT,
        PTOP,
        PBOTTOM,
        PBACK,
        NUM_PLANES
    };

    /** Assigns a plane to one of the enumerated planes of the frustum.

       \param planeNum A member of the Planes enumeration, specifying if this plane
       represents the near, left, right, top, bottom, or back frustum plane.
       \param plane A Plane object representing the Frustum plane specified.
     */
    void SetPlane(int planeNum, const Plane& plane) {
        planes[planeNum] = plane;
    }

    /** Retrieves a specific Plane of the frustum.

       \param planeNum A member of the Planes enumeration, specifying if you
       wish to retrieve the near, left, right, top, bottom, or back frustum plane.
       \return A const reference to the Plane object requested.
     */
    const Plane& GetPlane(int planeNum) const {
        return planes[planeNum];
    }

    /** Enables culling against the far clip plane from this frustum (enabled by default.)
     */
    void EnableFarClipCulling(bool bEnable) {
        bEnable ? nCullingPlanes = 6 : nCullingPlanes = 5;
    }

    int GetNumCullingPlanes() const {
        return nCullingPlanes;
    }

private:
    Plane planes[6];
    int nCullingPlanes;
};
}

#endif
