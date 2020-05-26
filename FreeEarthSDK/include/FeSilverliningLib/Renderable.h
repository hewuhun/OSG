// Copyright (c) 2004-2008  Sundog Software, LLC All rights reserved worldwide.

/**
    \file Renderable.h
    \brief An interface for objects that are drawable.
 */

#ifndef RENDERABLE_H
#define RENDERABLE_H

#if defined(WIN32) || defined(WIN64)
#pragma warning (disable:4786)
#endif

#include <FeSilverliningLib/MemAlloc.h>
#include <FeSilverliningLib/Frustum.h>
#include <FeSilverliningLib/Vector3.h>
#include <FeSilverliningLib/Renderer.h>

namespace SilverLining
{
/** An interface for objects that are drawable. */
class Renderable : public MemObject
{
public:
/** Default constructor. */
    Renderable() : depthOffset(0) {
    }

/** Virtual destructor. */
    virtual ~Renderable() {
        Renderer::GetInstance()->RemoveBlendedObject(this);
    }

/** Set the position of the object in world coordinates.
   \sa GetWorldPosition()
 */
    virtual void SetWorldPosition(const Vector3& pos) {
    }

/** Retrieves the position of the object in world coordinates.
   \sa SetWorldPosition()
 */
    virtual const Vector3& GetWorldPosition() const = 0;

    virtual Vector3 GetSortPosition() const {
        return GetWorldPosition();
    }

/** Translucent objects may submit themselves to the Renderer for later drawing
   once all translucent objects in the scene have been sorted from back to front.
   The Renderer will call the object's DrawBlendedObject() method to actually
   draw the object in this case. */
    virtual void DrawBlendedObject() {
    }

/** Adds an offset to the distance from the camera computed when sorting objects,
   in world units.
   \sa GetDepthOffset()
 */
    void SetDepthOffset(double offset) {
        depthOffset = offset;
    }

/** Retrieves the offset to the distance from the camera used when sorting objects,
   in world units.
   \sa SetDepthOffset()
 */
    double GetDepthOffset() const {
        return depthOffset;
    }

/** Computes the screen depth of this object, along the vector specified, for use in depth sorting.
   \param from The point from which to measure distance from
   \param to The point to measure the distance to
   \param mv The modelview matrix
   \return The eye space depth of the point of intersection with the offset.
 */
    virtual double GetDistance(const Vector3& from, const Vector3& to, const Matrix4& mv, bool rightHanded) const
    {
        Vector3 eyeSpace = mv * GetWorldPosition();
        return rightHanded ? -eyeSpace.z : eyeSpace.z;
    }

private:
    double depthOffset;
};
}

#endif
