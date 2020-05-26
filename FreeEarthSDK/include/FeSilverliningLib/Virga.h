// Copyright (c) 2006-2009 Sundog Software, LLC. All rights reserved worldwide.

/**
    \file Virga.h
    \brief Renders rain hanging underneath a cloud.
 */

#ifndef VIRGA_H
#define VIRGA_H

#include <FeSilverliningLib/Atmosphere.h>
#include <FeSilverliningLib/Renderable.h>
#include <FeSilverliningLib/Renderer.h>

namespace SilverLining
{
class Cloud;
class Billboard;

/** Renders rain hanging underneath a cloud, which may or may not actually reach the ground. This
   is done using a billboarding effect.*/
class Virga : public Renderable
{
public:
/** Constructs a Virga object of a given size and position, attached to a given Cloud.
   \param parentCloud The Cloud to associate this Virga with.
   \param diameter The diameter of the area of rain underneath the cloud.
   \param height The distance from the base of the cloud, plus the offsetPos provided, to the
   bottom of the Virga. For rain, this must be greater than the distance to the ground.
   \param offsetPos An offset from the center of the cloud from which to start drawing the virga
   from.
   \param atm The Atmosphere object used to render the Cloud.
 */
    Virga(Cloud *parentCloud, double diameter, double height, const Vector3& offsetPos,
          const Atmosphere& atm);

/** Virtual destructor; releases any resources allocated internally. */
    virtual ~Virga();

/** Sets the color of the virga, which modulates the virga texture map. */
    void SetColor(const Color& c);

/** Draws the virga, or rather schedules it for drawing once all translucent objects in the
   scene have been sorted via DrawBlendedObject().
   \param pass Set to 0 for the lighting pass, or 1 for the rendering pass.
   \return True if the operation succeeded.
 */
    virtual bool Draw(int pass);

/** Returns the position of the virga in world coordinates. */
    virtual const Vector3& GetWorldPosition() const;

/** Does the actual drawing of the Virga, once the framework has sorted it against all other
   translucent objects in the scene. */
    virtual void DrawBlendedObject();

protected:
    bool LoadTexture();
    void UpdateDrift();

    Vector3 offsetPos;
    static TextureHandle texture;
    Cloud *cloud;
    double yScale;
    Billboard *billboard;
    const Atmosphere& atmosphere;
    float virgaAlpha;
    Vector3 drift;
    float driftOffset, driftSpeed, driftRadius;
    float fadeDistance;
    double diameter;
};
}

#endif
