// Copyright (c) 2006-2014 Sundog Software, LLC. All rights reserved worldwide.

/** \file CirrusCloudLayer.h
   \brief A class to define a cirrus cloud "layer", which consists of a single large cirrus "cloud".
 */

#ifndef CIRRUS_CLOUD_LAYER_H
#define CIRRUS_CLOUD_LAYER_H

#include <FeSilverliningLib/CloudLayer.h>
#include <FeSilverliningLib/CirrusCloud.h>
#include <istream>

namespace SilverLining
{
/** A "layer" of cirrus clouds. A CirrusCloudLayer contains only a single
   CirrusCloud object. */
class CirrusCloudLayer : public CloudLayer
{
public:
/** Default constructor, just invokes the parent's CloudLayer() constructor. */
    CirrusCloudLayer();

/** Default destructor; does nothing. */
    virtual ~CirrusCloudLayer();

/** Creates a single CirrusCloud object to fill the CirrusCloudLayer's specified
   dimensions. */
    bool SILVERLINING_API SeedClouds(const Atmosphere& atm);

/** Controls fog inside the cloud */
    void SILVERLINING_API ProcessAtmosphericEffects(Sky *s);

    virtual void SILVERLINING_API WrapClouds(bool);

    virtual double ComputeSkyCoverage() const {return 0;}

    virtual ShaderHandle GetShader() const;

    virtual void ReloadShaders();

protected:

    double SILVERLINING_API ComputeWrapFade(Cloud *c, const Vector3& anchor) {return 1.0;}
    bool DrawSetup(int pass, const Vector3 *lightPos, const Color *lightColor);
    bool EndDraw(int pass);
    virtual bool RestoreClouds(const Atmosphere& atm, std::istream& s);
    virtual void AdjustForCurvature() {}
};
}

#endif
