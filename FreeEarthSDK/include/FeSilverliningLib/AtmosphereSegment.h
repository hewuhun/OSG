﻿// Copyright (c) 2007-2009 Sundog Software, LLC. All rights reserved worldwide.

/**
    \file AtmosphereSegment.h
    \brief A portion of the visible ring surrounding the planet.
 */

#ifndef ATMOSPHERE_SEGMENT_H
#define ATMOSPHERE_SEGMENT_H

#include <FeSilverliningLib/Renderable.h>
#include <FeSilverliningLib/Matrix3.h>

namespace SilverLining
{
class IndexBuffer;
class VertexBuffer;

class AtmosphereSegment : public Renderable
{
public:
/** Default constructor. */
    AtmosphereSegment();

/** Virtual destructor. */
    virtual ~AtmosphereSegment();

    void BuildGeometry(float startRadians, float endRadians, float r, float H0,
                       float atmosphereHeight, int xResolution, int yResolution,
                       float atmosphereBrightness);

    void ComputeVertices(const Vector3& sunPosNorm, const Matrix4& world, float atmosphereFadeDistance, float alpha, float radius);

/** Draws the object, or schedules it for later rendering in the
   translucency pass.
   \param pass Set to 0 for the lighting pass, or 1 for the rendering pass.
   \return True if the operation completed successfully.
 */
    virtual bool Draw(int pass);

/** Tests for visibility of the object within a view frustum.
   \param f The Frustum to test visibility within.
   \return True if the object is not inside the frustum and should be culled;
   false otherwise.
 */
    virtual bool Cull(const Frustum& f);

/** Set the position of the object in world coordinates.
   \sa GetWorldPosition()
 */
    virtual void SetWorldPosition(const Vector3& pos) {
        worldPos = pos;
    }

/** Retrieves the position of the object in world coordinates.
   \sa SetWorldPosition()
 */
    virtual const Vector3& GetWorldPosition() const {
        return worldPos;
    }

/** Translucent objects may submit themselves to the Renderer for later drawing
   once all translucent objects in the scene have been sorted from back to front.
   The Renderer will call the object's DrawBlendedObject() method to actually
   draw the object in this case. */
    virtual void DrawBlendedObject() {
    }

private:
    Vector3 worldPos;

    IndexBuffer *ib;
    VertexBuffer *vb;

    bool doShadow;
};
}

#endif
