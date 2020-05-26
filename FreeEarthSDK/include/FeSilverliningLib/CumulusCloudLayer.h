// Copyright (c) 2004-2014  Sundog Software, LLC. All rights reserved worldwide.

/**
    \file CumulusCloudLayer.h
    \brief A CloudLayer that contains many individual cumulus clouds, and optionally
    generates a shadow map from these clouds.
 */

#ifndef CUMULUS_CLOUD_LAYER_H
#define CUMULUS_CLOUD_LAYER_H

#include <FeSilverliningLib/CloudLayer.h>
#include <FeSilverliningLib/CumulusCloud.h>
#include <FeSilverliningLib/Renderer.h>
#include <string>
#include <vector>
#include <iostream>

namespace SilverLining
{
class CloudGenerator;
class CloudDistributor;

/** A collection of CumulusClouds that fill the CloudLayer's volume of space. */
class CumulusCloudLayer : public CloudLayer
{
public:
/** Default constructor. */
    CumulusCloudLayer();

/** Virtual destructor. */
    virtual ~CumulusCloudLayer();

/** Instantiates a CloudGenerator and CloudDistributor to fill the CloudLayer
   with clouds, up to the specified coverage amount. */
    virtual bool SILVERLINING_API SeedClouds(const Atmosphere& atm);

  /** Controls cloud animation effects over time. This only affect cumulus cloud types.
        \param voxelSpinRate For cumulus clouds, the maximum rate at which individual cloud voxels rotate to
                             simulate convection in the cloud. Specified in radians per second. Set to 0 for
                             no animation.
        \param enableGrowth  Controls whether the shape of the cloud itself evolves over time, via cellular
                             automata techniques. This can cause clouds to "grow" over time, but comes with a
                             performance cost.
        \param initialGrowthIterations If enableGrowth is true, this control the initial number of iterations
                             of the cullular automata to run to form each cloud prior to the first frame.
                             If you leave this set to zero, the default vaule for the specific cloud type will
                             be used, resulting in a fully-formed cloud at the outset. However, you could set this
                             as low as 1, resulting in clouds growing over time from very small clouds.
        \param timeStepInterval The time between cellular automata iterations, in seconds. Changes will be interpolated
                                across this time. Longer times will result in slower cloud formation. To use the
                                default settings for this cloud type, use a value of 0.
    */
    virtual void SILVERLINING_API SetCloudAnimationEffects(double voxelSpinRate, bool enableGrowth, 
        int initialGrowthIterations = 0, int timeStepInterval = 0);

    /** Retrieves the maximum rate at which individual cloud voxels rotate, in radians per second.
        \sa SetCloudAnimationEffects()
    */
    double SILVERLINING_API GetSpinRate() const {
        return spinRate;
    }

    /** Retrieves whether cloud growth over time is simulated at runtime.
        \sa SetCloudAnimationEffects()
    */
    bool SILVERLINING_API GetGrowthEnabled() const {
        return growth;
    }

    virtual bool SILVERLINING_API AddCloudAt(const Atmosphere& atm, const Vector3& relativePosition, const Vector3& dimensions);
    virtual bool SILVERLINING_API SupportsAddCloudAt() const { return true; }

    virtual double SILVERLINING_API GetMaxHeight() const { return maxHeight; }

    // Internal use only:
    virtual void SILVERLINING_API  MoveClouds(double x, double y, double z);

    // Deprecated:
    virtual bool SILVERLINING_API SupportsShadowMaps();
    virtual void SILVERLINING_API GenerateShadowMaps(bool enable);
    virtual bool SILVERLINING_API BindShadowMap(int textureStage, double *m) const;

protected:

/** Specific implementations of CumulusCloudLayer will need to create specific Cloud
   types, and must implement this method to create a cloud to put within the CloudLayer. */
    virtual CumulusCloud *CreateCloud(CloudLayer *layer) = 0;

/** Configures the CumulusCloudLayer based on config entries with the given prefix.
   By convention, this matches the cloud type in question (ie, "cumulus-congestus".) */
    void ReadConfiguration(const SL_STRING& configPrefix);

/** Sets up rendering states, projection matrices, and rendering contexts appropriate
   to the pass that are common to all clouds drawn in this layer. Must be balanced by a call
   to EndDraw() after the clouds are drawn.

   \param pass Set to 0 for the lighting pass, or 1 for the rendering pass.
   \param lightPos A normalized direction vector toward the infinitely distant dominant
   light source.
   \param lightColor The color of the dominant light source.
   \return true if the operation succeeded.
 */
    bool DrawSetup(int pass, const Vector3 *lightPos, const Color *lightColor);

/** After all the clouds have been drawn, call EndDraw() to restore the rendering states
   prior to calling DrawSetup().

   \param pass Set to 0 for the lighting pass, or 1 for the rendering pass.
   \return true if the operation succeeded.
 */
    bool EndDraw(int pass);

/** Saves the individual clouds for a future run. */
    virtual bool SaveClouds(std::ostream& s) const;

/** Restore the individual clouds from a previously saved run. */
    virtual bool RestoreClouds(const Atmosphere& atm, std::istream& s);

/** Allocates the shadow texture. */
    void SetupSurfaces();

// Catalogs the cloud files available to choose from (if any)
    void ScanCloudFiles(const Atmosphere& atm);

// Creates a cloud from a random cloud file. Assumes ScanCloudFiles was called previously.
    bool CreateCloudFromFile(const Atmosphere& atm, double scaleTo = 0);

    void InitCoverageMap();
    void DeleteCoverageMap();
    void AddToCoverage(Cloud *cloud);
    float GetCoverage();
    void CreateCloud(const Atmosphere& atm, double cloudWidth, double cloudDepth, double cloudHeight);

    bool *coverageMap;
    int totalCoverageCells;
    int cellsCovered;
    int coverageW, coverageL;

    CumulusCloudInit parameters;

    CloudGenerator *generator;
    CloudDistributor *distributor;

    Vector3 shadowMapElapsedWind;

    bool makeShadowMaps;
    bool renderLightingPassOffscreen;
    bool quickLighting;

    TextureHandle shadowTex;
    RenderTargetHandle offscreen;
    Matrix4 lightProjView;

    Vector3 shadowMapPos;

    bool restoreBackBuffer;
    char *savedBackBuffer;

    SL_STRING cloudTypeStr;
    SL_VECTOR(SL_STRING) cloudFiles;

    int vpx, vpy, vpw, vph, shadowTexDim;

    double spinRate, maxHeight;
};
}

#endif
