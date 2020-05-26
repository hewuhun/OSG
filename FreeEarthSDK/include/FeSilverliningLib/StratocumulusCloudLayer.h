// Copyright (c) 2010-2015  Sundog Software, LLC. All rights reserved worldwide.

/**
    \file StratocumulusCloudLayer.h
    \brief A CloudLayer that contains many individual low, puffy clouds that together make for a thick
    cloud layer with a few sun breaks. Unlike a broken stratus cloud layer, this layer is rendered volumetrically.
 */

#ifndef STRATOCUMULUS_CLOUD_LAYER_H
#define STRATOCUMULUS_CLOUD_LAYER_H

#include <FeSilverliningLib/CloudLayer.h>
#include <FeSilverliningLib/Renderable.h>
#include <FeSilverliningLib/Renderer.h>
#include <string>
#include <vector>
#include <iostream>

namespace SilverLining
{

class CloudBackdropManager;

class StratocumulusVoxel
{
public:
    StratocumulusVoxel() : states(0), extinctionProbability(1.0), phaseTransitionProbability(0), vaporProbability(0) {
    }

    unsigned char states;
    float extinctionProbability, phaseTransitionProbability, vaporProbability;
};

/** A collection of cellular-automata-grown puffy clouds within a volume rendered as volumetric slices. */
class StratocumulusCloudLayer : public CloudLayer, public Renderable
{
public:
/** Default constructor. */
    StratocumulusCloudLayer();

/** Virtual destructor. */
    virtual ~StratocumulusCloudLayer();

/** Instantiates a CloudGenerator and CloudDistributor to fill the CloudLayer
   with clouds, up to the specified coverage amount. */
    virtual bool SILVERLINING_API SeedClouds(const Atmosphere& atm);

    virtual bool SILVERLINING_API Draw(int pass, const Vector3 *lightPos, const Color *lightColor,
                      bool invalid, bool wantsLightingUpdate, unsigned long now, const Sky *sky, CloudBackdropManager *cbm);

    virtual void DrawBlendedObject();

    virtual const Vector3& GetWorldPosition() const;

/** Flattens this object and everything in it to a stream buffer. */
    virtual bool SILVERLINING_API Serialize(std::ostream& stream);

/** Restores this object from the stream created using Serialize() */
    virtual bool SILVERLINING_API Unserialize(const Atmosphere& atm, std::istream& stream);

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

    /** We implement our own SetBaseWidth so we can enforce the upper bound set in stratocumulus-max-size. */
    virtual void SILVERLINING_API SetBaseWidth(double meters);

    /** We implement our own SetBaseLength so we can enforce the upper bound set in stratocumulus-max-size. */
    virtual void SILVERLINING_API SetBaseLength(double meters);

    virtual ShaderHandle GetShader() const;

    virtual void ReloadShaders();

protected:

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

    virtual bool SaveClouds(std::ostream& s) const {
        return true;
    }
    virtual bool RestoreClouds(const Atmosphere& atm, std::istream& s) {
        return true;
    }

    virtual Vector3 GetSortPosition() const {
      double x, y, z;
      CloudLayer::GetSortPosition(x, y, z);
      return Vector3(x, y, z);
    }

    virtual double GetDistance(const Vector3& from, const Vector3& to, const Matrix4& mvp, bool rightHanded) const {
      Vector3 clipSpace = mvp * GetSortPosition();
      return rightHanded ? -clipSpace.z : clipSpace.z;
    }

    void ComputeBoundingBox();
    void ComputeTexCoords(Vertex *v);
    void UploadVolumeTexture();
    void UploadNoiseTexture();
    void IncrementGrowth();
    void FreeVoxels();
    void TrimExcessVoxels();

    // Compute lighting volume
    int ComputeVolumeLighting( int & updatedSliceMin, int & updatedSliceMax );

    virtual void SILVERLINING_API MoveClouds(double x, double y, double z);

    double voxelDimension, lightSamplingDistance, multipleScatteringTerm;
    double dropletSize, ambientScattering, dropletsPerCubicCm, albedo, jitter, inscatteringTerm;
    double extinctionProbability, transitionProbability, vaporProbability;
    int initialEvolve;
    Vector3 cloudMotion, noiseOffset;
    double maxSize, fadeFalloff;

    int voxelWidth, voxelDepth, voxelHeight;    

    StratocumulusVoxel ****voxels;
    unsigned char *noiseTexData, *cloudTexData;
    float *randomData;

    VertexBuffer *vb;
    IndexBuffer *ib;

    ShaderHandle shader, hdrShader;
    TextureHandle volumeData, noiseVolumeData;

    Matrix3 lastBasis;
    Vector3 cameraMotion, lastCameraPos;

    Color lightColor;
    Vector3 lightWorldPos;
    Vector3 lightObjectPos;
    Vector3 originTexCoords;

    Vector3 volumeLightingVector;
    double  volumeLightingVectorFullRefreshTolerance;
    double  volumeLightingVectorSliceRefreshTolerance;
    int     volumeLightingSlice;
    bool    invalid;

};
}

#endif
