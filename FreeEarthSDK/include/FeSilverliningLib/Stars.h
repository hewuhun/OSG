// Copyright (c) 2004-2012  Sundog Software, LLC. All rights reserved worldwide.

/**
    \file Stars.h
    \brief Renders the stars, planets, and any associated glares.
 */

#ifndef STARS_H
#define STARS_H

#include <FeSilverliningLib/Renderer.h>
#include <vector>

namespace SilverLining
{
class VertexBuffer;
class Ephemeris;
class Glare;
class GlareManager;
class Sky;

/** Efficiently renders the stars, planets, and their glares - employing vertex
   and fragment shaders when available. */
class Stars : public MemObject
{
public:
/** Constructor.
   \param eph A pointer to an Ephemeris object you created, which represents the
   current time and location, and provides methods for determining astronomical
   positions based on it. This object will not be deleted by this class.
   \param gmgr A pointer to a GlareManager object you created, which creates
   Glare objects to draw bright planets or stars on dark nights with glare
   effects. This object will not be deleted by this class.
   \param turbidity The modeled atmospheric turbidity, which affects the color
   of the stars when using vertex shaders. For a description of how to
   interpret this value, see AtmosphericConditions::SetTurbidity().
 */
    Stars(Ephemeris *eph, GlareManager *gmgr, double turbidity);

/** Destructor; destroys any objects created internally within the class. */
    ~Stars();

/** Draws the stars; assumes the modelview matrix has already been set up by
   the Sky class's Sky::Draw() method. If available, vertex shaders will be used
   to simulate the visibile stars' brightness and colors. */
    bool Draw(const Sky *sky);

/** Getter for the underlying shader. */
    ShaderHandle GetShader() const {return starShader;}

/** Reload the underlying shader. */
    void ReloadShaders();

private:
    void UpdatePlanets();
    void DrawGlares(const Matrix4& eqToHorizon);

    VertexBuffer *starVerts, *planetVerts;
    Glare **planetGlares;
    Ephemeris *ephemeris;
    ShaderHandle starShader;
    int nStars;
    double turbidity;
    double starBrightness;
    bool noGlare;
    SL_VECTOR(Glare *) glares;
    GlareManager *glareManager;
    double glareThreshold;
    float fStarDistance;
    bool geoZUp;
    float maxLightPollution;
};
}

#endif
