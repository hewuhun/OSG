// Copyright (c) 2011-2015 Sundog Software, LLC. All rights reserved worldwide.

#ifndef TRITON_PROJECTEDGRID_H
#define TRITON_PROJECTEDGRID_H

/** \file ProjectedGrid.h
    \brief A class that manages the actual drawing of the Ocean.
*/

#include <FeTriton/TritonCommon.h>
#include <FeTriton/Vector3.h>
#include <FeTriton/Matrix3.h>
#include <FeTriton/Vector4.h>
#include <FeTriton/Ocean.h>

namespace Triton
{
class Environment;
class WaterModel;
class OceanLightingModel;
class WakeManager;

/** The ProjectedGrid class renders the water as, well, a projected grid. The idea is to start with a 2D grid in
    screen space, project these points such that they intersect with the surface of the water, displace them,
    and then project them back into screen space. This results in what's essentially a perfect LOD scheme for
    the water - each polygon of the water mesh always takes up a constant amount of screen space.

    As the class responsible for rendering the water's geometry, this class also pulls together all the physical
    simulation and rendering objects in Triton as well. It loads the shaders for rendering the water, and
    collaborates with other classes to populate the parameters for these shaders.
*/
class ProjectedGrid : public MemObject
{
public:
    /** Constructor. */
    ProjectedGrid(bool pEnableBreakingWaves);

    /** Initializes the projected grid; must be called prior to using it.

        \param env The Environment object for the simulation.
        \param type The type of water model to create to power the simulation of the waves.
        \param gridResolution The dimension of the projected grid; a value of N will result in a
                              NxN grid in screen space making up the geometry of the water.
                              Higher values will result in higher resolution and the ability to
                              render smaller waves, but increases the load on the vertex program
                              by N-squared as this parameter increases.
        \return False if the grid failed to initialize for any reason.
    */
    virtual bool Initialize(const Environment *env, WaterModelTypes type, unsigned int gridResolution, unsigned int meshGridResolution);

    /** Virtual destructor. */
    virtual ~ProjectedGrid();

    /** Associates a WaterModel with this grid. */
    void SetWaterModel(WaterModel *wm) {
        waterModel = wm;
    }

    /** Retrieves the WaterModel associated with this grid via SetWaterModel(). */
    WaterModel *GetWaterModel() const {
        return waterModel;
    }

    /** Associates an OceanLightingModel with this grid. */
    void SetLightingModel(OceanLightingModel *lm) {
        lightingModel = lm;
    }

    /** Retrieves the OceanLightingModel associated with this grid via SetLightingModel(). */
    OceanLightingModel *GetLightingModel() const {
        return lightingModel;
    }

    /** Associates a WakeManager with this grid. */
    void SetWakeManager(WakeManager *wm) {
        wakeManager = wm;
    }

    /** Retrieves the WakeManager associated with this grid via SetWakeManager(). */
    WakeManager *GetWakeManager() const {
        return wakeManager;
    }

    /** Sets a depth offset to mitigate z-fighting issues against shorelines. A good value is 0.01 if this is an issue. */
    void SetDepthOffset(float offset) {
        depthOffset = offset;
    }

    /** Retrieves the depth offset set via SetDepthOffset(), or the default of 0. */
    float GetDepthOffset() const {
        return depthOffset;
    }

    /** Retrieves the choppiness value associated with this grid (how pointy the wave crests are.) */
    float GetChoppiness() const;

    /** Sets the choppiness value for this grid (how pointy the wave crests are.) Valid values are between 0.0
        and around 3.0 before waves start to intersect themselves. */
    void SetChoppiness(float chop);

    /** Retrieves the simulated depth, in world units, of the water simulated by this grid. \sa SetDepth() */
    float GetDepth(Vector3& floorNormal) const;

    /** Sets the simulated depth, in world units, of the water simulated by this grid, and the
        surface normal of the sea floor, at the camera position. */
    void SetDepth(float depth, const Vector3& floorNormal);

    /** Retrieves the slope and heading of the slope based on what was passed in via SetDepth(). */
    float GetSlope(float& slopeHeading);

    /** Retrieves the basis matrix representing the orientation of the world (transforms into a Z-is-up system.) */
    const Matrix3& GetBasis() const {
        return basis;
    }

    /** Retrieves the inverse of the basis matrix representing the orientation of the world (transforms from a Z-is-up system.) */
    const Matrix3& GetInverseBasis() const {
        return invBasis;
    }

    /** Retrieves the North direction (pointing from the center of the Earth to the North pole) in geocentric
        coordinate systems. */
    const Vector3& GetNorth() const {
        return north;
    }

    /** Enables or disables wireframe rendering of the projected grid, making the grid visible. For high
        grid resolutions that approach the resolution of the screen, it may be hard to see. */
    void EnableWireframe(bool on) {
        wireframe = on;
    }

    /** Update the basis and inverse basis matrices when the position changes in a geocentric system.
    These matrices transform positions into a local "Z is up" and "X is East" coordinate system.
    \sa GetBasis() \sa GetInverseBasis() */
    void UpdateBasisMatrices();

    /** Sets the shaders and graphics state necessary to render an ocean patch in world space. */
    virtual bool SetPatchShader(double time, int vertexStride, int positionOffset, bool doublePrecision, const Matrix4& modelMat, bool aboveWater, bool depthPass) {
        return false;
    }

    virtual bool SetPatchMatrix(const Matrix4& modelMat) {
        return false;
    }

    /** Finishes the state set by SetPatchShader. */
    virtual bool UnsetPatchShader(bool depthPass) {
        return false;
    }

    /** Draw the water for the given time sample, in seconds. */
    virtual void Draw(double time, bool depthWrites, bool aboveWater) {}

    /** Retrieve the underlying shader object for the projected grid. */
    virtual ShaderHandle GetShader() const {
        return 0;
    }

    /** Retrieve the underlying shader object for world-space patches. */
    virtual ShaderHandle GetPatchShader() const {
        return 0;
    }

    /** Call in response to lost DirectX9 devices, prior to resetting the device. */
    virtual void D3D9DeviceLost() {}

    /** Call following a device reset in DirectX9, to recreate Triton's resources. */
    virtual void D3D9DeviceReset() {}

    /** Pushes all state associated with the graphics context so we can restore it later. This
        is called before Triton does anything so we don't mess up the host application. */
    virtual bool PushAllState() {
        return false;
    }

    /** Pops all state recorded with PushAllState(), restoring the graphics context to what it was. */
    virtual bool PopAllState() {
        return false;
    }

    /** Sets a default rendering state with the graphics context, used to clear out any state
        the application may have set that might interfere with us. */
    virtual bool SetDefaultState() {
        return false;
    }

    /** Retrieves the number of triangles rendered by this grid (not counting degenerate triangles used
        to stitch rows together) */
    unsigned int GetNumTriangles() const {
        return resolution * resolution * 2;
    }

    /** Returns whether the current camera position is above the water surface. */
    bool IsAboveWater();

    /** Finds an intersection between a ray and the ocean surface, if any.
        \param rayPosition The origin of the ray to intersect with the ocean surface.
        \param rayDirection The direction vector of the ray.
        \param intersectPoint Receives the intersection point of the ray with the surface, if any
        \return True if an intersection was found and intersectPoint was populated, false if not.
    */
    bool GetIntersection(const Vector3& rayPosition, const Vector3& rayDirection, Vector3& intersectPoint) const;

    /** Sets the blend percentage for planar reflections (0-1) with the base water color. */
    void SetPlanarReflectionBlend(float blend) {
        planarReflectionBlend = blend;
    }

    /** Retrieves the blend percentage (0-1) for planar reflectoins with the base water color. */
    float GetPlanarReflectionBlend() const {
        return planarReflectionBlend;
    }

    /** Scales the intensity of reflections on the water (environment, planar, and light reflections combined.)
    Default is 1.0. */
    void TRITONAPI SetReflectionScale(float scale) {
        reflectionScale = scale;
    }

    /** Retrieves any reflection scale set previously by SetReflectionScale(), or 1.0 otherwise. */
    float TRITONAPI GetReflectionScale() const {
        return reflectionScale;
    }

    /** Retrieves whether breaking wave effects at shoreline are enabled. */
    bool GetBreakingWavesEnabled() const {
        return enableBreakingWaves;
    }

    /** OpenGL only: reload the shader programs, linking in a new list of user-supplied shader objects. */
    virtual bool ReloadShaders(const TRITON_VECTOR(unsigned int)& shaders) {
        return true;
    }

    /** Sets whether a depth texture from the water surface should be generated, which may be retrieved with
        ProjectedGrid::GetDepthTexture(). */
    void SetGenerateDepthTexture(bool on) {
        generateDepthTexture = on;
    }

    /** Retrieves a depth texture for the water surface, created from the previous Draw() call. Returns null if
        SetGenerateDepthTexture(true) was not called.
    */
    TextureHandle GetDepthTexture() const {
        return depthTexture;
    }

    /** Sets gamma correction applied to final output. Normally this should be 1.0 (the default) unless you are trying
        to move into linear space with 1.0/2.2 - otherwise gamma correction will be double-applied by the monitor. */
    void SetGamma(float g) {
        gamma = g;
    }

    /** Gets the gamma value set via SetGamma() */
    float GetGamma() const {
        return gamma;
    }

    void SetGodRayFade(float fade) {
        godRayFade = fade;
    }

    float GetGodRayFade() const {
        return godRayFade;
    }

	Vector3 TRITONAPI GetDoubleRefractionColor()
	{
		return doubleRefractionColor;
	}

protected:
    Matrix4 ComputeGridMatrix(double maxAmplitude);
    bool ProjectToPlane(const Vector4& p0, const Vector4& D, Vector4& intersect, double zOffset) const;
    bool ProjectToEllipsoid(const Vector4& p0, const Vector4& D, Vector4& intersect, double zOffset) const;
    bool IntersectEllipsoid(const Vector3& p0, const Vector3& D, Vector3& intersect, double zOffset) const;
    bool IntersectEllipsoidSegment(const Vector3& p0, const Vector3& D, Vector3& intersect, double zOffset) const;
    bool GetFloorPlane(Vector3& floorPlanePoint, Vector3& floorPlaneNormal);
    bool GetGridOffsetRoll(Vector3& offset, double& roll);
    void ComputeDirections(const Vector3& camPos, Vector3& northPole);
    bool SwitchToMesh();
    Matrix4 ComputeMeshMatrix();

    unsigned int resolution;
    const Environment *environment;
    Vector3 earthRadii;
    Matrix3 basis, invBasis;
    Vector3 north, localNorth, localEast;
    WaterModel *waterModel;
    OceanLightingModel *lightingModel;
    WakeManager *wakeManager;
    double antiAliasing;
    WaterModelTypes waterModelType;
    bool wireframe;
    double aspectRatio;
    float planarHeight;
    double maxAllowableDisplacement, gridExpansionScale;
    float planarAdjust, planarReflectionBlend;
    bool enableBreakingWaves;
    float depthOffset;
    double horizonOffset;
    bool doubleSided;
    bool alphaMask;
    double intersectionVectorLength, intersectionVectorLengthAboveWater, intersectionVectorLengthProjection;
    double gridClipFactor, nearZ, farZ;
    float reflectionScale;

    float Lx, Ly;
    double meshSwitchAltitude, meshSize;
    bool useMesh;
    bool generateDepthTexture;
    TextureHandle depthTexture;
    int depthTextureSize;
    int meshGridResolution;
    int numOctaves;
    Vector3 doubleRefractionColor;
    float doubleRefractionIntensity;

    float gamma;
    float godRayFade;
};
}

#endif