// Copyright (c) 2004-2014  Sundog Software, LLC. All rights reserved worldwide.

/**
    \file CumulusCloud.h
    \brief A cloud that is composed of overlapping lit voxels, or metaballs.
 */

#ifndef CUMULUS_CLOUD_H
#define CUMULUS_CLOUD_H

#include <FeSilverliningLib/Cloud.h>
#include <vector>
#include <iostream>

#define CUMULUS_SMOOTH_WIDTH 1
#define CUMULUS_SMOOTH_DIM (CUMULUS_SMOOTH_WIDTH * 2 + 1)

namespace SilverLining
{
class Voxel;
class QuadBatches;

/** This structure contains initialization parameters for a CumulusCloud that are read
   from the config file for a given cumulus cloud type. */
typedef struct CumulusCloudInit_S
{
    double width, height, depth;
    double vaporProbability, transitionProbability, extinctionProbability;
    double initialVaporProbability;
    double voxelSize;
    int initialEvolveSteps;
    int timeStepInterval;
    double dropletSize, waterContent, albedo, dropletsPerCubicCm;
    double spinRate;
    float ambientScattering;
    double attenuation;
    double minimumDistanceScale;
    double verticalGradient;
} CumulusCloudInit;

/** A Cloud that is composed of many lit voxels, which contain metaballs - or in simpler terms,
   a collection of puffs. Its creation and growth is driven by a cellular automata process.*/
class CumulusCloud : public Cloud
{
public:
/** Default constructor. */
    CumulusCloud(CloudLayer *layer);

/** Virtual destructor. */
    virtual ~CumulusCloud();

/** Configures the CumulusCloud object, instantiates and positions its voxels, and
   sets up the initial conditions for the cellular automata.

   \param init A CumulusCloudInit structure that contains initialization parameters read
   from the config file.
   \return true if the operation succeeded.
 */
    virtual bool Init(const CumulusCloudInit& init);

/** Tests if this cloud is visible within the given Frustum.

   \param f The frustum to test visibility against.
   \return true if the cloud is not visible and should be culled; false if it is visible.
 */
    virtual bool Cull(const Frustum& f);

/** Draw the voxels that make up this cloud (or rather, schedule it for later drawing when
   it's sorted against the other translucent objects in the scene.)

   \param pass Set to 0 for the lighting pass, or 1 for the rendering pass.
   \param lightPos The normalized direction toward the dominant infinitely distant light source.
   \param lightColor The color of the dominant light source.
   \param invalid Forces the cloud to recompute its lighting
   \param sky Pointer to the sky object, used to compute the sky color in the cloud's direction for fog.
 */
    virtual bool Draw(int pass, const Vector3& lightPos, const Vector3& lightDir,
                      const Color& lightColor, bool invalid, const Sky *sky);

/** Performs the actual drawing of the cloud, once the framework has sorted all translucent
   objects in the scene. */
    virtual void DrawBlendedObject();

/** Increment the time step of the cellular automata, allowing the cloud to grow. If
   inadequate time has passed to trigger a time increment, just update the voxel densities.
   Smooth growth over time could be implemented by blending the densities between time
   steps, but this is not yet implemented.

   \param now The current timestamp, in milliseconds.
   \param forceUpdate Force the automata to increment, regardless of other conditions.
 */
    virtual bool Update(unsigned long now, bool forceUpdate);

/** Cloud growth is influenced by the current wind conditions. Call shear to offset the
   cloud voxels based on the current wind conditions.

   \param updraftSpeed The vertical updraft within the cloud, in meters per second.
   \param wind The current wind vector at the cloud's altitude, in meters per second.
 */
    void Shear(double updraftSpeed, const Vector3& wind);

/** Retrieves the dimensions of the bounding box this cloud fits within. */
    virtual void GetSize(double& width, double& depth, double& height) const;

/** Implements the cellular automata; called by the Update() method. */
    virtual void IncrementTimeStep(unsigned long now) = 0;

/** Saves this cloud's information to a stream, in proprietary binary format. */
    virtual bool Serialize(std::ostream& s);

/** Restores this cloud from disk, from our proprietary binary format. */
    virtual bool Unserialize(std::istream& s);

/** Exports this cloud as a collection of spheres to a VRML .WRL file. */
    virtual bool ExportToVRML(const char *filename) const;

/** Imports this cloud from a VRML '97 .WRL file. Call instead of Init()*/
    virtual bool InitFromFile(const CumulusCloudInit& init, std::istream *s, double scale = 1.0);

/** Call when a frame starts, so we know to reset our count of how many
   cloud recompiles we're allowed to do. */
    static void FrameStarted() {
        recompiles = 0;
    }

/** Smoothly fade the cloud in from transparent to opaque */
    virtual void FadeIn();

/** Smoothly fade the cloud out from opaque to transparent */
    virtual void FadeOut();

/** Intersect this cloud using a ray originating from a world space origin and direction.
    Return all hits in hit_ranges */
    virtual void Intersect( const Vector3& origin,
                            const Vector3& direction,
                            SL_VECTOR(double)& hit_ranges );
    
protected:

/** Sets up the initial conditions for the cellular automata, based on the config-based
   initialization parameters. */
    virtual void InitializeVoxelState() = 0;

/** As a cloud is lit, adds any active voxels to a list of voxels that must be rendered. */
    void Submit(Voxel *v);

/** Performs the rendering of the cloud, by drawing its component voxels back-to-front
   from the camera. */
    void DrawCloud();

/** Performs the lighting of the cloud in a seperate pass, by drawing the voxels
   front-to-back from the light source in an orthographic projection from the light source.
   After each voxel is drawn, its color is read back from the frame buffer - in this way,
   we model light passing through and scattering through each voxel from the sun or
   moon. At the same time, the resulting framebuffer is captured and used as a shadow map.

   \param lightPos The normalized direction vector to the infinitely distant dominant light
   source.
   \param lightColor The color of the dominant light source.
 */
    void ShadeCloud(const Vector3& lightPos, const Color& lightColor);

/** Performs a faster cloud shading operation by estimating the cloud to fill
   its bounding ellipsoid. The farther the distance from the point where the
   light source -> voxel ray intersects the ellipsoid, the darker it is.

   \param lightPos The normalized direction vector to the infinitely distant dominant light
   source.
   \param lightColor The color of the dominant light source.
 */
    void ShadeCloudFast(const Vector3& lightPos, const Color& lightColor);

// Read static member constants from the config file.
    void ReadConfig();

    void FreeVoxels();

    int width, height, depth;

    unsigned long lastTimeStep, timeStepInterval;

    Voxel ****voxels;
    SL_VECTOR(Voxel*) drawList;
    double voxelSize;
    Vector3 bounds;

    double vaporProbability, transitionProbability, extinctionProbability;
    double initialVaporProbability;
    double albedo, dropletsPerCubicCm;
    float ambientScattering;
    double attenuation;
    double verticalGradient;
    Vector3 lastCameraPos;
    Color nLightColor;

    static double weights[CUMULUS_SMOOTH_DIM][CUMULUS_SMOOTH_DIM][CUMULUS_SMOOTH_DIM];
    static double sortAngleThreshold;
    static int recompiles;
    static int recompileBudget;
    static bool quickLighting;
    static double colorRandomness, colorRandomnessHiRes;
    static double quickLightingAttenuation, quickLightingAttenuationHiRes;
    static bool noUpdate;
    static bool cullInteriorVoxels;
    static double alphaCullThreshold;
    static bool darkenWithDensity;
    static double darkenAmount;
    unsigned long fadeStartTime;

    QuadBatches *compiledObject;
};
}

#endif
