// Copyright (c) 2004-2014  Sundog Software, LLC. All rights reserved worldwide.

/**
    \file Metaball.h
    \brief Models the "puffs" that cumulus clouds are made of.
 */

#ifndef METABALL_H
#define METABALL_H

#include <FeSilverliningLib/Billboard.h>
#include <FeSilverliningLib/Renderer.h>
#include <FeSilverliningLib/Color.h>
#include <map>

namespace SilverLining
{
class Atmosphere;

/** A cumulus cloud is composed of many Voxel objects, which are elements of the cellular
   automata that generates the clouds. Each Voxel contains a Metaball, which is responsible for
   modeling the scattering of light throughout the voxel. The Metaball extends a Billboard,
   which ultimately renders each "puff" that makes up the cloud. */
class Metaball : public Billboard
{
public:

/** There are several methods for modelling the scattering of light through
   a cloud (rayleigh scattering seems to yield the best
   visual results.) Isotropic is just single-scattering. Anistropic adds forward
   scattering based on the angle between the light vector and the camera vector
   from the metaball. Lambertian is "physically correct" but tends to scatter
   too much light for realistic visual results. Rayleigh scattering is simple
   yet effective - it models multiple forward scattering. Finally, we offer
   a double-lobed Henyey-Greenstein function that allows you to tweak the
   amount of forward and back scattering simulated. */
    enum PhaseFunction
    {
        ISOTROPIC,
        ANISOTROPIC,
        LAMBERTIAN,
        RAYLEIGH,
        HENYEY_GREENSTEIN
    };

/** Loads the cloud wisp texture for rendering, and the gaussian texture
   used for lighting, from disk. */
    static bool CreateTexture(Atmosphere *atm);

/** Releases the cloud wisp and gaussian textures; call at shutdown. */
    static void ReleaseTexture(Atmosphere *atm);

/** Returns a handle to a gaussian texture representing the metaball,
   used for lighting purposes. */
    static TextureHandle GetTexture(Atmosphere *atm) {
        return metaballTextureMap[atm];
    }

/** Returns a cloud wisp texture, which is used during rendering for
   better visual results. */
    static TextureHandle GetWispTexture(Atmosphere *atm) {
        return wispTextureMap[atm];
    }

/** Returns a cloud atlas texture, used for hi-res cumulus clouds. */
    static TextureHandle GetAtlasTexture(Atmosphere *atm) {
        return atlasTextureMap[atm];
    }

/** Default constructor. */
    Metaball() {
    }

/** Creates the underlying Billboard. */
    void InitializeMetaball(const float Radius, const float SpinRate, const int atlasIndex, const bool rotate);

/** Destructor. */
    virtual ~Metaball();

/** Sets the modeled radius of the metaball in world units (important
   for modelling the amount of light scattered in the metaball.) */
    void SetRadius(double radius);

/** Retrieves modeled radius of the metaball in world units. */
    double GetRadius() const {
        return radius;
    }

/** Retrieves the maximum spin rate of the metaball in radians per second. */
    double GetSpinRate() const {
        return spinRate;
    }

/** Sets the color to modulate the metaball by. */
    void SetMetaballColor(const Color& color);

/** Retrieves the color previously set by SetColor(). */
    const Color& GetMetaballColor() const {
        return color;
    }

/** Sets the translucency of the metaball.

   \param alpha The alpha value that modulates the metaball's alpha channel.
   0 is transparent, 1 is opaque. */
    void SetAlpha(double alpha);

/** Computes the billboard color for the given pass. Should be called prior to DrawInBatch() */
    void ComputeBillboardColor(int pass, double colorRandomness, float ambientScattering);

/** Tests if the metaball is visible within the specified Frustum.

   \param f The view Frustum to test visibility against.
   \param offset A vector representing the difference between the underlying Billboard's
   position and the Metaball's position in the world, if any. The Billboard may be in
   coordinates relative to its cloud, for example.
   \return True if the object is not visible and should be culled; false otherwise.
 */
    bool Cull(const Frustum& f, const Vector3& offset);

/** Sets an ambient term for the metaball's lighting, which might model ambient
   skylight.
   \sa GetAmbientColor()
 */
    static void SetAmbientColor(const Color& amb) {
        ambient = amb;
    }

/** Retrieves the ambient term for the metaballs' lighting, which might model
   ambient skylight.
   \sa SetAmbientColor()
 */
    static Color GetAmbientColor() {
        return ambient;
    }

/** Sets the normalized direction vector toward the dominant infinitely-distance
   light source. Must be called at least once per frame before Metaballs are drawn. */
    static void SetLightPosition(const Vector3& pos) {
        lightPosition = pos;
    }

/** Sets the world position of the camera. Must be called at least once per
   frame before metaballs are drawn. */
    static void SetCameraPosition(const Vector3& pos) {
        cameraPosition = pos;
    }

/** If the metaball's position is relative to its parent cloud, specify the
   offset between the cloud and world coordinates, so that lighting may be properly
   computed. Must be called at least once per frame before metaballs are drawn. */
    static void SetOffsetPosition(const Vector3& pos) {
        offsetPosition = pos;
    }

protected:
// Note - keep data members at a minimum, there are a lot of these objects.

    float radius, spinRate;
    Color color;

    void ReadConfig();

    double PhaseFunction(double a);

    static SL_MAP(Atmosphere *, TextureHandle) metaballTextureMap;
    static SL_MAP(Atmosphere *, TextureHandle) wispTextureMap;
    static SL_MAP(Atmosphere *, TextureHandle) atlasTextureMap;

    static int textureDimension;

    static Color ambient;
    static int phaseFunction;
    static double henyeyGreensteinG1, henyeyGreensteinG2, henyeyGreensteinF;
    static double anisotropicX;
    static bool inited;
    static Vector3 lightPosition, cameraPosition, offsetPosition;
    static float maxMetaballColor;
};
}

#endif
