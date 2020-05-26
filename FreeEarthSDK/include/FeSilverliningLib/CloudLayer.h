﻿// Copyright (c) 2004-2015  Sundog Software, LLC. All rights reserved worldwide.

/** \file CloudLayer.h
   \brief Methods for configuration and initializing cloud decks.
 */

#ifndef CLOUDLAYER_H
#define CLOUDLAYER_H

#ifdef SWIG
#define SILVERLINING_API
#define SL_MAP(a, b) std::map<a, b>
%module SilverLiningCloudLayer
%include <FeSilverliningLib/Atmosphere.h>
%{
#include <FeSilverliningLib/CloudLayer.h>
using namespace SilverLining;
%}
#endif

#include <FeSilverliningLib/Atmosphere.h>
#include <FeSilverliningLib/Vector3.h>
#include <FeSilverliningLib/Matrix3.h>
#include <FeSilverliningLib/CloudTypes.h>
#include <vector>
#include <map>
#include <stdio.h>
#include <iostream>
#include <FeSilverliningLib/Export.h>

#pragma pack(push)
#pragma pack(8)

namespace SilverLining
{
class Cloud;
class Frustum;
class Color;
class Sky;
class LightningListener;
class CloudBackdropManager;

/** A collection of clouds of uniform classification organized into a "deck" of similar altitudes.

   CloudLayer is a bse class that cannot be directly instantiated; instead, it is returned by
   the class factory CloudLayerFactory. Call CloudLayerFactory::Create() to obtain a CloudLayer object
   for a specific cloud type.

   Once created, you'll want to set the CloudLayer's dimensions and density (coverage) using the
   SetBaseWidth(), SetBaseLength(), SetBaseAltitude(), SetThickness(), and SetDensity() methods.
   You will also need to position it using SetLayerPosition().

   Initially, a CloudLayer will not contain any clouds. Once specifying your preferences for the
   layer's dimensions and density, you must then call SeedClouds() to simulate cloud growth within
   the cloud deck.

   Once your CloudLayer object is created, configured, and seeded, you may then pass it into
   AtmosphericConditions::AddCloudLayer() to add it to your scene.

    Once "seeded", the above methods to modify the layer's dimensions and density
    will have no effect - they are burned in at that point. You may, however, call SetLayerPosition()
    at runtime. To change the other properties of the cloud layer at runtime, you need to remove
    the cloud layer and re-create it.

   See SetBaseWidth() for information on important differences between different types of
   CloudLayers.
 */
class FESILVERLININGLIB_EXPORT CloudLayer : public MemObject
{
public:
/** Default constructor. This is a virtual base class, so this constructor is not directly
   invoked by your application. CloudLayer objects are instantiated by calling
   CloudLayerFactory::Create().
 */
    CloudLayer();

/** Destructor. Removes and deletes all clouds added to this CloudLayer. */
    virtual ~CloudLayer();

/** Removes all clouds from this cloud layer. */
    void ClearClouds();

/** Returns the cloud type of this layer. */
    CloudTypes SILVERLINING_API GetType() const {
        return layerType;
    }

/** Sets the type of this cloud layer. */
    void SILVERLINING_API SetType(CloudTypes type) {
        layerType = type;
    }

/** Test if ray intersects this cloud layer. Currently only supported on cumulus cloud layers.*/
    bool SILVERLINING_API Intersect(const Vector3& Origin, const Vector3& Direction, double& range) const;

/** Enable rendering of this cloud layer.

   New cloud layers are enabled by default.

   Why would you want to explicitly enable and disable a cloud layer? Well, creating and
   seeding cloud layers - especially cumulus cloud layers - is computationally expensive
   and can take up to a second or two. If your application requires several cloud layers
   that are displayed at different times, it may make sense to create them all when your
   application initializes, and then just enable and disable them as needed, which is a
   much faster operation.

    Optionally, you may fade cloud layers in or out over time to smoothly introduce them
    into your scene. Specify fadeTimeMS in order to enable this effect when enabling or
    disabling a cloud layer. Fading effects on cumulus layers are only supported on systems
    that support vertex shaders.

   \param enabled True if this cloud layer should be rendered during Atmosphere::DrawObjects(),
                false if it should not be rendered.
   \param fadeTimeMS The amount of time, in milliseconds, for the cloud layer to fade in
       or out.
 */
    void SILVERLINING_API SetEnabled(bool enabled, unsigned long fadeTimeMS = 0);

/** Accesses if the CloudLayer is currently enabled for rendering.

   \sa SetEnabled()
 */
    bool SILVERLINING_API GetEnabled() const {
        return layerEnabled;
    }

/** Returns whether this cloud layer needs to be rendered at all; if it is disabled via SetEnabled()
    and any fading time has expired, this will return false.

    \sa SetEnabled()
*/
    bool SILVERLINING_API IsRenderable() const {
        return renderingEnabled;
    }

/** Sets the size of the CloudLayer along the X axis. The cloud layer must be at least as
   big as the size of individual clouds within it. The size of the cloud layer may have
   important performance and visual impacts, so it should be chosen with care. Here are some
   general guidelines for setting the cloud layer dimensions:

   \li Cumulus congestus, mediocris, and sandstorms: The number of clouds created is proportional to the
   square of the layer dimensions, so performance will degrade quickly if unreasonably high
   dimensions are chosen. A balance must be found between performance (not having
   too many clouds to render at real-time speed) and visual acceptability (having enough
   clouds to fill the scene.) Dimensions of around 30 km or so generally fulfill both demands,
   but the number of clouds also depends on the cloud layer density (see SetDensity()).

   \li Stratus: The width represents the size of the entire stratus cloud layer. If this is an
   infinite stratus layer, this value is ignored and the SilverLining.config setting
   stratus-deck-width is used instead.

   \li Cumulonimbus: Due to the large size of cumulonimbus clouds and their isolated organization
   along "squall lines", it's not realistic to have a rectangular deck of cumulonimbus clouds.
   Instead, the size of the cumulonimbus cloud layer will reflect the size of a single
   cumulonimbus cloud that fills the entire layer. Choose dimensions that reflect the size of
   the cloud itself, typically on the order of 5 km.

   \li Cirrus: Cirrus decks are rendered as a high, textured plane, but it does not extend
   infinitely like stratus decks do. A large enough dimension should be chosen to fill the
   sky of your scene at the deck's altitude; typically, 100,000 meters is appropriate.

   \li Stratocumulus: Stratocumulus clouds are rendered using 3D volumetric data that can be very
   memory-intensive. Cloud layer sizes above 50,000 meters are not recommended, and will also result
   in sampling artifacts. This upper bound is enforced by the stratocumulus-max-size setting in
   resources/silverlining.config.
 */
    virtual void SILVERLINING_API SetBaseWidth(double meters)              {
        baseWidth = meters;
    }

/** Retrieves the dimension of the CloudLayer along the X axis.
   \sa SetBaseWidth()
 */
    double SILVERLINING_API GetBaseWidth() const {
        return baseWidth;
    }

/** Sets the size of the CloudLayer along the Z axis. See SetBaseWidth() for guidance on
   appropriate values for different types of CloudLayers. */
    virtual void SILVERLINING_API SetBaseLength(double meters)             {
        baseLength = meters;
    }

/** Retrieves the dimension of the CloudLayer along the Z axis.
   \sa SetBaseLength()
 */
    double SILVERLINING_API GetBaseLength() const {
        return baseLength;
    }

/** Sets the altitude above sea level of the bottom of the CloudLayer. Here's some guidance
   on realistic settings (in meters) for various CloudLayer types:

   \li Cumulus congestus, cumulus mediocris: Typically found fairly low; between 1000-4000m.
   \li Cumulonimbus: Generally extends to the ground if it's raining. 400m is an appropriate setting.
   \li Stratus: Found very low; typically 500-2000 m.
   \li Cirrus: Found very high; 6000 m or higher.
   \li Sandstorm: Set to the local ground altitude to position the sandstorm on the ground.

    SetBaseAltitude() may be called at runtime after a cloud layer has been seeded to change its position,
    and should be called before seeding the cloud layer.

    The updateCloudPositions indicates whether the cloud altitudes within this layer should be updated
    immediately. If set to false, individual clouds will not update until they are repositioned by other
    means, such as wrapping around an infinite cloud layer or a call to SeedClouds().
 */
    virtual void SILVERLINING_API SetBaseAltitude(double meters, bool updateCloudPositions = true);

/** Retrieves the altitude above sea level of the CloudLayer botton in meters. */
    double SILVERLINING_API GetBaseAltitude() const {
        return baseAltitude;
    }

/** Sets the thickness of the CloudLayer in meters. "Thickness" has different meanings for
   different CloudLayer types:

   \li Cumulus congestus, mediocris: The thickness specifies the variance in the altitude of
   the bottoms of each individual cloud within the CloudLayer. The actual height of each cloud
   is determined procedurally as part of the cloud growth model, and is not affected by
   SetThickness(). So, setting the thickness to 0 would result in a layer of clouds that all
   start at the same base altitude; setting a thickness of 100 would provide for some variance
   for increased realism. In reality, cloud layers rest on top of an air mass that cause most
   of the clouds to have flattened bottoms that start at roughly the same altitude.

   \li Cumulonimbus: SetThickness() is unused for cumulonimbus clouds. The height of the cloud
   will be determined proportionally from the setting in SetBaseWidth().

   \li Stratus: The thickness in this case specifies the actual thickness of the stratus deck,
   that is, the distance between the top and bottom of the stratus layer. Stratus clouds vary
   in thickness; 1000-2000 m are reasonable values. The thickness of the stratus deck will not
   impact performance, unlike cumulus decks.

   \li Stratocumulus: Like stratus, the thickness in this case specifies the actual thickness
   of the stratocumulus layer. This needs to be thick enough to contain the clouds within this
   layer; typically 3000 meters should suffice. If it's too small, the clouds will be unrealistically
   small and also transmit too much sunlight, so set this with care.

   \li Cirrus: The thickness is ignored for cirrus decks; cirrus decks are modeled as infinitely
   thin layers of ice crystals at high altitude.

   \li Sandstorm: Pass 0 to ensure a flat bottom to the sandstorm, or just leave this unspecified.
 */
    virtual void SILVERLINING_API SetThickness(double meters) {
        thickness = meters;
    }

/** Retrieves the thickness of the CloudLayer in meters. See SetThickness() for descriptions of
   what this value represents for various cloud types. */
    double SILVERLINING_API GetThickness() const {
        return thickness;
    }

/** Sets the percentage of the sky that should be covered by this CloudLayer, from 0 - 1.0.
   For cumulus congestus and cumulus mediocris decks, this will affect the number of clouds
   generated during SeedClouds(), within a coverage tolerance of 5%. For stratus decks, 
   setting a value less than 1.0 will result in a "broken" cloud deck, which results in 
   extremely high performance characteristics for variable cloud clover, but is much less 
   visually realistic than a cumulus congestus deck. This value is ignored for cirrus and 
   cumulonimbus cloud decks, which always represent a single cloud.

   \param coveragePercentage The amount of the sky that should be covered by clouds, from 0 - 1.0.
 */
    virtual void SILVERLINING_API SetDensity(double coveragePercentage) {
        density = coveragePercentage;
    }

/** Retrieves the cloud coverage set for this CloudLayer.

   \return The cloud coverage of this deck from 0 - 1.0.
   \sa SetDensity()
 */
    double SILVERLINING_API GetDensity() const {
        return density;
    }

/** Specifies the location of the center of the CloudLayer. Sets the "East" and "South" coordinates
   of the CloudLayer's center position. For a right-handed coordinate
    system where Z points in, you'd pass in the X and Z coordinates of where you want the cloudlayer
    to be centered over, for example. A right-handed coordinate system where Z points up would pass
    in X and -Y instead.

    Cloud positions may move over time in response to
   simulated wind (see AtmosphericConditions::SetWind()).

    If you are using a custom set of basis vectors, be sure that you have called
    Atmosphere::SetUpVector() and Atmosphere::SetRightVector() before calling this method. The cloud
    layer will be oriented according to the basis defined by the current up and right vectors
    when SetLayerPosition() is called. If you're in a geocentric, round-Earth system, you'll
    want to set the up and right vectors to reflect the local basis at the cloud's location,
    and then set these back to reflect the eyepoint before rendering with Atmosphere::DrawSky()
    and Atmosphere::DrawObjects().

   Calling SetLayerPosition will reset any accumulated wind drift as well.

    SetLayerPosition() and SetBaseAltitude() are the only methods on a CloudLayer that you
    may change at runtime, after the CloudLayer has been seeded and added into the scene. Modifying
    other properties of the CloudLayer after it's been seeded require removing the CloudLayer from
    your AtmosphericConditions, and re-creating the layer.

    This method is not needed with infinite cloud layers, which are always positioned relative to the camera.

   \param eastCoord The east coordinate of the CloudLayer's center position, in meters.
   \param southCoord The south coordinate of the CloudLayer's center position, in meters.
 */
    void SILVERLINING_API SetLayerPosition(double eastCoord, double southCoord);

/** Retrieves the east and south coordinates of the CloudLayer's center, in meters.

   \sa SetLayerPosition()
 */
    void SILVERLINING_API GetLayerPosition(double& east, double& south) const {
        east = layerX;
        south = layerZ;
    }

/** Selects whether this cloud layer is "infinite" - that is, it will always surround the camera when
   feasible, and never gets blown away by wind. For layers of cumulus clouds, individual clouds will wrap
   around the boundaries of the layer to keep the layer centered at the camera position at all times.

   The specific effect of "infinite" varies by the cloud layer type. Cumulus congestus and mediocris cloud
   layers behave as described above, where individual clouds wrap around the cloud layer's boundaries relative
   to the camera position. For cirrus and cirrocumulus clouds, setting them to "infinite"  means they
   are not affected by wind and remain fixed above the camera position. Infinite stratus cloud layers remain
   positioned relative to the camera position and simulate relative motion by rotating its texture coordinates.
   This method does not affect cumulonimbus cloud layers, as they only contain a single cloud.

   Be default, cloud layers are not infinite and must be positioned using SetLayerPosition().

   \sa GetIsInfinite()
 */
    virtual void SILVERLINING_API SetIsInfinite(bool inf) {
        isInfinite = inf;
    }

/** Returns whether this cloud layer is "infinite" and attempts to remain fixed relative to the camera position
   as best it can. \sa SetIsInfinite() */
    bool SILVERLINING_API GetIsInfinite() const {
        return isInfinite;
    }

/** Sets a cloud layer specific wind vector which is additive to any global wind set at 
   the AtmosphericConditions level. The vector itself specifies the direction the wind is
   coming from, and the length of the vector provides the wind velocity.

   \param windX The east vector of the CloudLayer's specific wind, in meters/s.
   \param windZ The south vector of the CloudLayer's specific wind, in meters/s.
   \sa GetWind()
 */
    void SILVERLINING_API SetWind(double windX, double windZ) {
        localWindX = windX;
        localWindZ = windZ;
    }

/** Retrieves the cloud layer specific wind vector.
   \sa SetWind()
 */
    void SILVERLINING_API GetWind(double& windX, double& windZ) const {
        windX = localWindX;
        windZ = localWindZ;
    }

    /** Controls cloud animation effects over time. This only affects cumulus cloud types. Must be called
        prior to CloudLayer::SeedClouds().
        \param voxelSpinRate For cumulus clouds, the maximum rate at which individual cloud voxels rotate to
                             simulate convection in the cloud. Specified in radians per second. Set to 0 for
                             no animation.
        \param enableGrowth  Controls whether the shape of the cloud itself evolves over time, via cellular
                             automata techniques. This can cause clouds to "grow" over time, but comes with a
                             performance cost.
        \param initialGrowthIterations If enableGrowth is true, this controls the initial number of iterations
                             of the cullular automata to run to form each cloud prior to the first frame.
                             If you leave this set to zero, the default vaule for the specific cloud type will
                             be used, resulting in a fully-formed cloud at the outset. However, you could set this
                             as low as 1, resulting in clouds growing over time from very small clouds.
        \param timeStepInterval The time between cellular automata iterations, in seconds. Changes will be interpolated
                                across this time. Longer times will result in slower cloud formation. To use the
                                default settings for this cloud type, use a value of 0.
    */
    virtual void SILVERLINING_API SetCloudAnimationEffects(double voxelSpinRate, bool enableGrowth, 
        int initialGrowthIterations = 0, int timeStepInterval = 0) {}

/** Infinite cloud layers will always wrap the clouds around the camera position, but you can also enable this behavior
    relative to the position of a non-infinite cloud layer. This means that as the wind blows individual clouds, they will
    remain within the original bounds of the cloud layer rather than allowing the entire layer to move with the wind.
    Cirrus, cirrocumulus, and cumulonimbus cloud layers cannot be wrapped.
    This mode is false by default. \sa GetCloudWrapping() */
    void SILVERLINING_API SetCloudWrapping(bool wrap) {
        cloudWrap = wrap;
    }

/** Returns whether non-infinite cloud layer types will wrap wind-blown clouds to remain within the original bounds of the
    layer, or if wind will move the entire layer indefinitely. */
    bool SILVERLINING_API GetCloudWrapping() const {
        return cloudWrap;
    }

/** For infinite cloud layers or cloud layers with "wrapping" enabled this
    option allows you to automatically fade clouds out as they approach the boundary of the cloud layer where they will pop
    out and reappear on the opposite side of the layer. This fading hides the popping in the distance.
    \sa SetCloudWrapping()
    \sa SetIsInfinite() */
    void SILVERLINING_API SetFadeTowardEdges(bool fade) {
        fadeTowardEdges = fade;
    }

/** Retrieves whether an infinite or wrapping cloud layer has fade effects enabled as the clouds approach the boundaries of
    the layer. \sa SetFadeTowardEdges() . */
    bool SILVERLINING_API GetFadeTowardEdges() const {
        return fadeTowardEdges;
    }

    enum PrecipitationTypes
    {
        NONE,
        RAIN,
        DRY_SNOW,
        WET_SNOW,
        SLEET,
        NUM_PRECIP_TYPES
    };

/** Simulates precipitation under this cloud layer of a specified type. Any time the camera is under
   a cloud of this layer, precipitation effects will display if the type is set to something other than
   NONE.

    Note, you may also set global precipitation effects that are not associated with a CloudLayer
    with the similar AtmosphericConditions::SetPrecipitation() method.

    For mixed precipitation, you may call SetPrecipitation multiple times with different precipitation
    types. To clear all precipitation, call SetPrecipitation with a type of NONE. If you call this method
    multiple times for the same precipitation type, the intensity specified will overwrite the intensity
    previously specified for that type.

   \param type The type of precipitation to simulate under this cloud layer - NONE, RAIN, WET_SNOW,
                DRY_SNOW, or SLEET.
   \param intensity The simulated rate of precipitation, in millimeters per hour. Reasonable ranges
   might be between 1 for light rain or 20 for heavier rain. This value will be clamped to the value
   specified by rain-max-intensity, snow-max-intensity, or sleet-max-intensity in
    resources/SilverLining.config, which is 30 by default.

   \param nearClip How close to the camera the closest precipitation particles will be rendered. The
   near clipping plane will be adjusted to include this distance while the precipitation is being 
   rendererd. Values less than zero will result in the default value being applied.

   \param farClip The farthest distance from the camera that precipitation particles will be rendered.
   Since there is an upper bound on the number of particles rendered per frame, changing the difference
   between nearClip and farClip may result in changes to the intensity of the precipitation. By default
   this difference is 7.5 world units. Values less than zero will result in the default value being
   applied.

   \param useDepthBuffer Set this to true if you want to enable depth buffer testing of precipitation 
   particles against your scene.

   \sa HasPrecipitationAtPosition()
     \sa GetPrecipitation()
 */
    void SILVERLINING_API SetPrecipitation(int type, double intensity, double nearClip = -1.0, double farClip = -1.0, bool useDepthBuffer = false);

/** Retrieves a map of precipitation types to intensities in millimeters per hour that are
    associated with this cloud layer. Precipitation types include RAIN, WET_SNOW, DRY_SNOW,
    or SLEET.

    \sa SetPrecipitation()
    \sa HasPrecipitationAtPosition()
 */
    const SL_MAP(int, double)& SILVERLINING_API GetPrecipitation() const {
        return precipitationEffects;
    }

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

/** Sets translucency for the cloud layer; 0 = transparent, 1.0 = normal opacity. Default is 1.0.
    This can be used to give clouds a softer appearance and blend into the background better. */
    void SILVERLINING_API SetAlpha(double pAlpha) {
        alpha = pAlpha;
    }

/** Retrieves explicit translucency set by SetAlpha(). */
    double SILVERLINING_API GetAlpha() const {
        return alpha;
    }

/** Generates clouds within the cloud layer. SeedClouds() may only be called after configuring
   the CloudLayer by calling the SetBaseWidth(), SetBaseLength(), SetBaseAltitude(),
   SetThickness(), SetDensity(), and SetLayerPosition() methods. When called, it will
   invoke a cloud growth model to fill the cloud layer with clouds to meet the specified
   properties of the CloudLayer.

   Once a cloud layer is seeded, you may use the Save() method to save it to disk
   if you'd like to be able to quickly restore it with the Restore() method, instead
   of recreating the layer from scratch every time.

   No clouds will appear within your CloudLayer unless SeedClouds() is called!

   \param atm A const reference to the Atmosphere object that will ultimately contain this
   CloudLayer. This atmosphere must have been initialized with Atmosphere::Initialize(), or
   this method will fail.

   \sa Save()
   \sa Restore();
 */
    virtual bool SILVERLINING_API SeedClouds(const Atmosphere& atm) = 0;

/** Adds an individual cloud to an already-seeded cloud layer. Note this method will only work
    on cloud layers that contain many clouds, such as cumulus congestus and cumulus mediocris. If you
    attempt to use this on a layer that inherently contains a single cloud (such as stratus, 
    cumulonimbus, or cirrus,) this method will do nothing and return false. To be sure, use
    the CloudLayer::SupportsAddCloudAt() method to see if this method is avaialble on this cloud layer
    type.
    
    All positions are relative to the cloud layer position, and must fall within the bounds 
    defined by the cloud layer's width, depth, and height. If it does not, false will be returned
    and the cloud will not be added.

    This method may be used to provide finer control over cloud formations and frontal boundaries.
    Even if you plan to manually position every cloud in the layer, you must still call
    CloudLayer::SeedClouds() first with a cloud coverage of 0 (via CloudLayer::SetDensity() ). 
    SeedClouds() performs some required initialization for the cloud layer. If you attempt to 
    call AddCloudAt() on a layer that has not been seeded, again false will be returned.

    Again: first call CloudLayer::SeedClouds(), then CloudLayer::AddCloudAt() repeatedly for each cloud
    you wish to add.

    You may also mix and match procedurally-created clouds with your own clouds by setting the
    cloud layer coverage to a non-zero value, calling SeedClouds(), and then calling AddClouds() to
    add your clouds to the proceduraly generated ones.

    \param atm A const reference to the Atmosphere object that will ultimately contain this
    CloudLayer. This atmosphere must have been initialized with Atmosphere::Initialize(), or
    this method will fail.

    \param relativePosition The center position of the cloud relative to the center of the cloud layer.
    Regardless of your coordinate system, this position should assume that positive Y is "up",
    X is along the "east" axis, and Z along the "south". These coordinates must fall within the bounds
    defined by the cloud layer's width and length, or this method will return false.
    Note that in infinite cloud layers (CloudLayer::SetIsInfinite()), each cloud may get repositioned
    to wrap around the edges of the layer as the camera moves. You should take care not to position
    clouds too closely on top of each other, or popping artifacts may result if they re-sort in a different
    order as the view angle changes.

    \param dimensions The size of this cloud. The X component represents the cloud width along the "east" dimension,
    Y represents the cloud height, and Z the cloud length along the "south" dimension.

    \return True if the cloud was successfully added. False may be returned if the cloud layer has not 
    yet been seeded with CloudLayer::SeedClouds(), if an attempt was made to place a cloud outside of
    the cloud layer's bounds, if an uninitialized Atmosphere was passed in, this size of the cloud is
    smaller than the size of a single cloud puff, or if this cloud layer does not support AddCloudAt().
    */
    virtual bool SILVERLINING_API AddCloudAt(const Atmosphere& atm, const Vector3& relativePosition, const Vector3& dimensions) { return false; }

/** Indicates whether this cloud layer supports the CloudLayer::AddCloudAt() method for adding user-positioned clouds to 
    the procedural clouds. */
    virtual bool SILVERLINING_API SupportsAddCloudAt() const { return false; }

/** Adds a LightningListener - derived class to the list of LightningListeners to receive
   notifications of lightning strikes created by this CloudLayer. The CloudLayer will
   not destroy the LightningListener; its memory management is up to the caller.

   \sa ClearLightningListeners()
 */
    void SILVERLINING_API AddLightningListener(LightningListener *listener);

/** Clears the list of LightningListener objects that receive notifications of lightning
   strikes from this CloudLayer. It does not delete the LightingListener objects.

   \sa AddLightningListener()
 */
    void SILVERLINING_API ClearLightningListeners();

/** Forces a lightning strike starting with this frame on cloud layers that support lightning. If the
    lightning discharge mode is FORCE_ON_OFF, the value parameter controls whether lightning is on or off. */
    virtual bool SILVERLINING_API ForceLightning(bool value = true) { return false; }

    /** Sets the lightning mode. AUTO_DISCHARGE will fire lightning automatically at random. FORCE_ON_OFF gives
    the application explicit control via CloudLayer::ForceLightning(). FIRE_AND_FORGET allows lightning to be
    initiated via CloudLayer::ForceLightning(), but the lightning strike will then finish automatically. */
    virtual void SILVERLINING_API SetLightningDischargeMode(LightningDischargeMode mode) { return; }

    /** Returns the current lightning discharge mode for this cloud layer, or MAX_DISCHARGE_MODE on layer that
        don't support lightning. */
    virtual LightningDischargeMode SILVERLINING_API GetLightningDischargeMode() { return MAX_DISCHARGE_MODE; }

#ifndef SWIG
/** Saves this cloud deck's clouds to a file for fast restoring later. This is faster than
   calling SeedClouds to generate the deck at runtime, and also allows a way to reproduce
   the same cloud decks by bypassing the randomness of SeedClouds.

   \param filePath The fully qualified path to the file you wish to save this cloud deck's
   configuration to.

   \sa SeedClouds()
   \sa Restore()

   \return True if the file was successfully written.
 */
    virtual bool SILVERLINING_API Save(const char *filePath);

/** Restores a cloud deck from a file previously generated with the Save() method. It is not
   necessary to explicitly configure the cloud's dimensions, position, and density, or to
   call SeedClouds(), when Restore() is called on a newly instantiated cloud layer.
   Restore() offers a way to quickly load a known good cloud layer without recreating it
   from scratch every time using SeedClouds(). It will create the layer more quickly than
   SeedClouds(), but you'll lose the randomness of the clouds within the layer by using
   this method.

   \param atm A const reference to the Atmosphere object that will ultimately contain this
   CloudLayer.
   \param filePath The fully qualified path to the file previously created with Save().

   \sa Save()
   \sa SeedClouds()

   \return True if the file was successfully restored; false if the file could not be found
   or was an invalid file format.
 */
    virtual bool SILVERLINING_API Restore(const Atmosphere& atm, const char *filePath);

/** Exports each individual cloud into a numbered .WRL file within the
   specified directory path. Each .WRL file will be a simple collection
   of translated spheres that represent the metaballs that make up
   each cloud.

   These .WRL files may then be tweaked using a 3D graphics package, and
   then re-imported by placing them in resources/clouds/(cloudtype)/.
   (cloudtype) is the config file prefix for the settings for the given
   cloud layer type in resources/Atmosphere.config, for example
   cumulus-congestus or cumulonimbus.

   If a given cloud type has .WRL files in its matching resources/clouds/
   subdirectory, SilverLining will randomly select these files as the
   layer is initialized in SeedClouds(), instead of generating the clouds
   algorithmically from a cellular automata.

   This will currently only produce output for cumulus cloud decks.

   \sa SeedClouds()
 */
    virtual bool SILVERLINING_API ExportToVRML(const char *dirPath);

/** Flattens this object and everything in it to a stream buffer. */
    virtual bool SILVERLINING_API Serialize(std::ostream& stream);

/** Restores this object from the stream created using Serialize() */
    virtual bool SILVERLINING_API Unserialize(const Atmosphere& atm, std::istream& stream);

/** Retrieve the vector of underlying cloud objects (useful only to licensed users with the full source and private headers.) */
    const SL_VECTOR(Cloud *)& SILVERLINING_API GetClouds() const {return clouds;}

/** Retrieves the height of the bounding box enclosing this cloud layer. For cumulus cloud types, this is different
    than GetThickness(), which specifies the variation in cloud base heights. */
    virtual double SILVERLINING_API GetMaxHeight() const {return thickness;}

/** This allows you to reduce the coverage of cloud layers that contain multiple clouds (currently cumulus congestus
    or cumulus mediocris) at runtime. The cloud layer will initially create enough clouds in the scene to achieve the
    coverage specified in CloudLayer::SetDensity(). By calling CloudLayer::SetDensityMultiplier, we'll randomly
    fade some clouds in or out to reduce the coverage from the original density. The clouds you don't see will still 
    be using memory, so use this technique with care and only if you need to vary the layer density continuously.

    So, one technique would be to create a cloud layer with SetDensity(1.0), and then use SetDensityMultiplier() to
    change the density to whatever you want at runtime. 

    \param multiplier A value from 0 to 1.0 controlling what percentage of this cloud layer's clouds will actually
    be displayed.
*/
    virtual void SILVERLINING_API SetDensityMultiplier(double multiplier) {coverageMultiplier = multiplier; usingRuntimeDensity = true;}

/** Returns the value last set by CloudLayer::SetDensityMultiplier(), or the default value of 1.0. */
    virtual double SILVERLINING_API GetDensityMultiplier() const { return coverageMultiplier; }

/** Sets whether the cloud layer should curve down to meet the ground at its extents. Can be useful for creating the illusion of 
    cloud layers that are bigger than they really are. Generally this should be called before seeding the cloud layer. Default is false.
    With cumulus clouds, this effect is strongest when the camera is near the ground (as determined from the altitude passed in via 
    the Location object) and goes away when the altitude reaches the cloud layer's base altitude. */
    virtual void SILVERLINING_API SetCurveTowardGround(bool curve) {curveTowardGround = curve;}

/** Retrieves whether this cloud layer was created with SetCurveTowardGround(true). */
    virtual bool SILVERLINING_API GetCurveTowardGround() const {
        return curveTowardGround;
    }

/** Force the clouds in this layer to use a specific color for their directional and ambient lighting. Call
    CloudLayer::ClearCloudColorOverride() to go back to using the simulated color for the given time
    and location. The color passed in will be used as-is, so be sure to scale it for day vs. night for
    example. 
    
    \param forcedCloudColor The color you wish to force this cloud layer to become.
    \param doLighting Whether the color should be darkened automatically according to the time of day.
    */
    void SILVERLINING_API OverrideCloudColor(const Vector3& forcedCloudColor, bool doLighting = true);

/** Clears usage of a specific cloud lighting color set previously with CloudLayer::OverrideCloudColor(). */
    void SILVERLINING_API ClearCloudColorOverride();

/** Returns true and populates the color parameter with the cloud color override set via 
    CloudLayer::OverrideCloudColor(), or returns false if a cloud color override is not currently
    in use for this layer. */
    bool SILVERLINING_API GetOverriddenCloudColor(Vector3& color, bool& doLighting) const;

// The following methods are deprecated. See Atmosphere::GetShadowMap() for current shadow support.
    virtual bool SILVERLINING_API SupportsShadowMaps()    {
        return false;
    }

    virtual void SILVERLINING_API GenerateShadowMaps(bool) {
    }

    virtual bool SILVERLINING_API BindShadowMap(int textureStage, double * m) const {
        return false;
    }

// The following methods are for internal use only, and should not be called by
// application developers:
    virtual void SetCulled() {culled = true;}
    virtual bool GetCulled() const {return culled;}
    virtual void ClearCulled() {culled = false;}
    virtual void ClearCulledChildren();
    virtual void DoCulling(const Frustum *f);
    virtual void DoUpdates(const Sky* sky, unsigned long now);
    virtual void SILVERLINING_API  MoveClouds(double x, double y, double z);
    virtual void SILVERLINING_API WrapClouds(bool geocentric);
    void SILVERLINING_API AddCloud(Cloud *cloud) {
        clouds.push_back(cloud);
    }
    void SILVERLINING_API LightningNotify(const Vector3& pos);
    virtual void SILVERLINING_API ProcessAtmosphericEffects(Sky *);
    virtual bool SILVERLINING_API Draw(int pass, const Vector3 *lightPos, const Color *lightColor,
                      bool invalid, bool wantsLightingUpdate, unsigned long now, const Sky* sky, CloudBackdropManager *cb);
    virtual void SILVERLINING_API GetSortPosition(double& x, double& y, double& z) const;
    virtual void SILVERLINING_API ProcessFade(unsigned long now);
    virtual void SILVERLINING_API ApplyPrecipitation();
    void SILVERLINING_API SetLocalUpVector(const Vector3& up) {
        localUp = up;
    }
    void SILVERLINING_API SetLocalBasis(const Matrix3& basis) {
        localBasis = basis;
    }
    void SILVERLINING_API SetLocalBasis4(const Matrix4& basis4) {
       localBasis4 = basis4;
    }
    const Vector3& SILVERLINING_API GetLocalUpVector() const;
    const Matrix3& SILVERLINING_API GetLocalBasis() const;
    const Matrix4& SILVERLINING_API GetLocalBasis4() const;
    virtual double SILVERLINING_API ComputeWrapFade(Cloud *c, const Vector3& anchor);
    bool SILVERLINING_API Cull(const Frustum& f);
    double SILVERLINING_API GetFade() const {return fade;}
    virtual double  ComputeSkyCoverage() const;
    const Vector3*  GetBounds() const {return bounds;}
    double GetBaseAltitudeGeocentric() const {return geocentricAltitude;}
    virtual ShaderHandle GetShader() const {return 0;}
    virtual void ReloadShaders() {}
    void SetUsingCloudAtlas(bool on) { usingCloudAtlas = on; }
    bool UsingCloudAtlas() const { return usingCloudAtlas; }
    double GetEdgeFadeFactor(double cloudScale = 1.0);
    bool GetIsSoft() const {return soft;}
    void SetIsSoft(bool on) { soft = on; }
    virtual void UpdateOvercast(Sky *s) {}

protected:
    virtual void ComputeBounds();
    virtual bool DrawSetup(int pass, const Vector3 *lightPos, const Color *lightColor) = 0;
    virtual bool EndDraw(int pass) = 0;
    virtual bool SaveClouds(std::ostream& s) const;
    virtual bool RestoreClouds(const Atmosphere& atm, std::istream& s) = 0;

    virtual void AdjustGeocentricPositions(bool geocentric);
    virtual void AdjustForCurvature(bool geocentric);
    bool IntersectEllipsoid(const Vector3& p0, const Vector3& D, Vector3& intersect) const;

    enum FadeModes
    {
        NO_FADE,
        FADE_IN,
        FADE_OUT
    };

    double edgeFadeThreshold;
    double fade, alpha, fadeBegin;
    unsigned long fadeTime, fadeStart;
    FadeModes fadeMode;
    double layerX, layerZ;
    double baseWidth, baseLength;
    double baseAltitude, thickness;
    double geocentricAltitude;
    double density, coverageMultiplier;
    bool layerEnabled;
    bool renderingEnabled;
    bool forceLightingUpdate;
    bool firstFrameDrawn;
    bool isInfinite;
    int lastCloudUpdated;
    double localWindX, localWindZ;
    bool cloudWrap;
    bool fadeTowardEdges;
    bool culled;
    bool curveTowardGround;
    CloudTypes layerType;
    Vector3 localUp;
    Matrix3 localBasis;
    Matrix4 localBasis4;
    Vector3 bounds[8];
    bool growth, enableCulling, usingRuntimeDensity;
    Vector3 earthRadii;
    Vector3 forcedCloudColor;
    bool hasForcedCloudColor;
    bool forcedCloudColorLighting;
    bool usingCloudAtlas;
    bool soft;
    SL_MAP(int, double) precipitationEffects;
    SL_MAP(int, double) precipitationNearClips;
    SL_MAP(int, double) precipitationFarClips;
    SL_MAP(int, bool) precipitationUseDepthBuffer;
    SL_VECTOR(Cloud*) clouds;
    SL_VECTOR(LightningListener *) lightningListeners;
    SL_VECTOR(Cloud *) drawList;
#endif //SWIG
};
}

#pragma pack(pop)

#endif

