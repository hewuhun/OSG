// Copyright (c) 2005-2009 Sundog Software, LLC, All rights reserved worldwide.

/**
    \file CloudImposter.h
    \brief A class that implements dynamically-generated imposters for Clouds.
 */

#ifndef CLOUD_IMPOSTER_H
#define CLOUD_IMPOSTER_H

#include <FeSilverliningLib/MemAlloc.h>
#include <FeSilverliningLib/Cloud.h>
#include <FeSilverliningLib/Renderer.h>
#include <FeSilverliningLib/Billboard.h>

namespace SilverLining
{
/** A class that renders a cloud to a texture map that is then applied to a billboard,
   in order to draw distant clouds as efficiently as possible. This technique is known as
   dynamically generated imposters. See G. Schaufler, 褼ynamically Generated Impostors?
   GI Workshop <i>Modeling - Virtual Worlds - Distributed Graphics</i>,  1995, pp 129-136
   for an overview of the technique. The imposters redraw themselves in response to changes
   in the viewing angle or lighting conditions. */
class CloudImposter : public MemObject
{
public:
/** Constructor; creates a CloudImposter that represents the Cloud passed in. */
    CloudImposter(Cloud *cloud);

/** Virtual destructor. */
    virtual ~CloudImposter();

/** Before drawing a cloud, call SetContext() to give the imposter a chance to
   take over, and set the rendering context to its own texture map if it needs to
   be updated. Be sure to call RestoreContext() when drawing is completed, to give
   the class an opportunity to set the rendering context back to what it was
   originally.

   \param useImposter - Output; will be set to true if you may draw the imposter
   instead of the cloud itself.
   \param wantsUpdate - Output; will be set to true if the imposter has set the
   rendering context to its own texture map for updating.
 */
    void SetContext(bool *useImposter, bool *wantsUpdate);

/** If something changed with the attached cloud such that you must regenerate
   the imposter, call Invalidate() to force it to update on the next SetContext()
   call. */
    void Invalidate() {
        invalid = true;
    }

/** After drawing a cloud, balance the call to SetContext() prior to drawing with
   a call to RestoreContext(). This will allow the class to reset the render context
   and state if it changed it. */
    bool RestoreContext();

/** Renders the 2D imposter at the given world position. Only call this if you're
   not drawing the 3D cloud; ie, if SetContext sets the useImposter parameter to true.
   Fade ranges from 0 (transparent) to 1 (fully visible) */
    bool Render(const Vector3& worldPos, double fade);

/** Call at the beginning of the frame to reset the count of updates per frame. */
    static void FrameStarted() {
        updatesThisFrame = 0;
    }

private:
    Cloud *cloud;
    double imposterRadius;
    int imposterDim;
    bool contextSet;
    Vector3 lastNearPt, lastFarPt;
    bool invalid, disabled;
    Vector3 lastWorldPos;

    RenderTextureHandle renderTexture;

    Billboard *billboard;

    static int updatesThisFrame;
    static int maxUpdatesPerFrame;
};
}

#endif
