// Copyright (c) 2006-2014 Sundog Software, LLC. All rights reserved worldwide.

/**
    \file StratusCloudLayer.h
    \brief Models a CloudLayer that contains a single StratusCloud.
 */

#ifndef STRATUS_CLOUD_LAYER_H
#define STRATUS_CLOUD_LAYER_H

#include <FeSilverliningLib/CloudLayer.h>
#include <FeSilverliningLib/StratusCloud.h>
#include <istream>

namespace SilverLining
{
/** Models a CloudLayer that contains a single StratusCloud. */
class StratusCloudLayer : public CloudLayer
{
public:
/** Default constructor. */
    StratusCloudLayer();

/** Virtual destructor. */
    virtual ~StratusCloudLayer();

/** Populates the StratusCloudLayer with its single StratusCloud. */
    bool SILVERLINING_API SeedClouds(const Atmosphere& atm);

/** Gives the stratus cloud layer an opportunity to tell the Sky object
   that it should render itself with overcast conditions. */
    void SILVERLINING_API ProcessAtmosphericEffects(Sky *s);

/** Returns whether a precipitation type other than NONE will be simulated at the given camera position.
   If you're under a cloud and precipitation has been assigned to this cloud layer using
   SetPrecipitation(), this will return true. The specific effect may be retrieved with
   GetPrecipitation().

   \param x The position, in world coordinates, for which you wish to test for precipitation effects.
   \param y The position, in world coordinates, for which you wish to test for precipitation effects.
   \param z The position, in world coordinates, for which you wish to test for precipitation effects.
 */
    virtual bool SILVERLINING_API HasPrecipitationAtPosition(double x, double y, double z) const;

/** Returns whether the given location is inside the bounding box of any of the clouds within this cloud
     layer.

   \param x The position, in world coordinates, for which you wish to test for a cloud intersection.
   \param y The position, in world coordinates, for which you wish to test for a cloud intersection.
   \param z The position, in world coordinates, for which you wish to test for a cloud intersection.
 */
    virtual bool SILVERLINING_API IsInsideCloud(double x, double y, double z) const;

    virtual void SILVERLINING_API WrapClouds(bool);

    virtual ShaderHandle GetShader() const;

    virtual void ReloadShaders();
    
    virtual void UpdateOvercast(Sky *s);

protected:
    virtual void ComputeBounds();
    double SILVERLINING_API ComputeWrapFade(Cloud *c, const Vector3& anchor) {return 1.0;}
    bool DrawSetup(int pass, const Vector3 *lightPos, const Color *lightColor);
    bool EndDraw(int pass);
    virtual bool RestoreClouds(const Atmosphere& atm, std::istream& s);
    virtual void AdjustForCurvature() {}
    StratusCloud *myCloud;
};
}

#endif
