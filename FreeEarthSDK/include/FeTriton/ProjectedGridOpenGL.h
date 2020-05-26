// Copyright (c) 2011-2015 Sundog Software LLC. All rights reserved worldwide.

#ifndef TRITON_PROJECTEDGRIDOPENGL_H
#define TRITON_PROJECTEDGRIDOPENGL_H

/** \file ProjectedGridOpenGL.h
    \brief A specialization of ProjectedGrid for OpenGL.
*/
#include <FeTriton/ProjectedGrid.h>
#include <FeTriton/TritonOpenGL.h>
#include <stack>
#ifndef _WIN32
#include <stdint.h>
#endif

// cache result of frequently called glGet*() functions (performance bottleneck)
#define PROJECTEDGRIDOPENGL_GLGET_OPT   1

namespace Triton
{
/** A specialization of ProjectedGrid for OpenGL. */
class ProjectedGridOpenGL : public ProjectedGrid
{
public:
    ProjectedGridOpenGL(bool pEnableBreakingWaves, const TRITON_VECTOR(unsigned int)& shaders) : vboID(0), idxID(0), vertShader(0), fragShader(0), program(0),
        vertexArray(0), hasUnifiedVA(false), hasMultisample(false), hasVertexArrays(false), userShaders(shaders), userShader(0), userVertShader(0),
        vertShaderPatch(0), fragShaderPatch(0), programPatch(0), meshVboID(0), meshIdxID(0), meshVertexArray(0),
        meshNIndices(0), depthFBO(0), dummyColorBuffer(0), programCast(0), programCastPatch(0), setDefaultState(true),
        ProjectedGrid(pEnableBreakingWaves) {}

    virtual bool Initialize(const Environment *env, WaterModelTypes type, unsigned int gridResolution, unsigned int meshGridResolution);

    virtual ~ProjectedGridOpenGL();

    /** Sets the shaders and graphics state necessary to render an ocean patch in world space. */
    virtual bool SetPatchShader(double time, int stride, int offset, bool doublePrecision, const Matrix4& modelMatrix, bool aboveWater, bool depthPass);
    virtual bool SetPatchMatrix(const Matrix4& modelMatrix);

    /** Finishes the state set by SetPatchShader. */
    virtual bool UnsetPatchShader(bool depthPass);

    virtual void Draw(double time, bool depthWrites, bool aboveWater);

    virtual ShaderHandle GetShader() const {
        return (ShaderHandle)programCast;
    }

    virtual ShaderHandle GetPatchShader() const {
        return (ShaderHandle)programCastPatch;
    }

    virtual bool PushAllState();

    virtual bool PopAllState();

    virtual bool SetDefaultState();

    virtual bool ReloadShaders(const TRITON_VECTOR(unsigned int)& shaders);

private:
    bool SetupVBO(unsigned int gridResolution);
    bool SetupShaders();
    bool PrepShaders(bool aboveWater);
    bool TestCapabilities();
    bool SetupMesh();
    bool DrawMesh(double time, bool depthWrites, bool aboveWater);
    void DeleteShaders();
    void SetupDepthTexture();
    void DeleteDepthTexture();
    void DrawDepthTexture(int numIndices, bool patch);

    GLhandleARB LoadShader(const char *shaderName, bool vertexShader, bool userShader );

    GLuint vboID, idxID, dummyHeightMapID;
    GLhandleARB vertShader, fragShader, userShader, userVertShader, vertShaderPatch, fragShaderPatch, program, programPatch;
    GLuint vertCastShader, fragCastShader, userCastShader, userVertCastShader, vertCastShaderPatch, fragCastShaderPatch, programCast, programCastPatch;
    unsigned int nIndices;
    GLint invmvprojLoc, planeLoc, radiiLoc, invRadiiLoc;
    GLint basisLoc, invBasisLoc, northLoc, northPoleLoc, eastLoc, cameraPosLoc, gridSizeLoc;
    GLint gridScaleLoc, antiAliasingLoc, vertexLoc;
    GLint fogColorLoc, fogDensityLoc, fogDensityBelowLoc;
    GLint cubeMapMatrixLoc, planarReflectionMapMatrixLoc;
    GLint floorPlanePointLoc, floorPlaneNormalLoc, planarHeightLoc, planarAdjustLoc;
    GLint referenceLocationLoc, hasHeightMapLoc, heightMapLoc, heightMapMatrixLoc;
    GLint hasUserHeightMapLoc;
    GLint heightMapRangeOffsetLoc;
    GLint hasDepthMapLoc, depthMapLoc;
    GLint hasBreakingWaveMapLoc, breakingWaveMapLoc, breakingWaveMapMatrixLoc;
    GLint kexpLoc, breakerWavelengthLoc, breakerWavelengthVarianceLoc, breakerDirectionLoc;
    GLint breakerAmplitudeLoc, breakerPhaseConstantLoc, breakerDepthFalloffLoc;
    GLint surgeDepthLoc, steepnessVarianceLoc, seaLevelLoc, planarReflectionBlendLoc;
    GLint mvLoc, projLoc, depthOffsetLoc, depthOnlyLoc, windDirLoc, underwaterLoc;
    GLint doubleRefractionColorLoc, doubleRefractionIntensityLoc, oneOverGammaLoc, sunIntensityLoc;
    GLint invZoomLoc, zNearFarLoc, reflectivityScaleLoc;

    GLint invmvprojLocPatch, planeLocPatch, radiiLocPatch, invRadiiLocPatch;
    GLint basisLocPatch, invBasisLocPatch, northLocPatch, northPoleLocPatch, eastLocPatch;
    GLint cameraPosLocPatch, gridSizeLocPatch;
    GLint gridScaleLocPatch, antiAliasingLocPatch, vertexLocPatch;
    GLint fogColorLocPatch, fogDensityLocPatch, fogDensityBelowLocPatch;
    GLint cubeMapMatrixLocPatch, planarReflectionMapMatrixLocPatch;
    GLint floorPlanePointLocPatch, floorPlaneNormalLocPatch, planarHeightLocPatch;
    GLint modelMatrixLocPatch, referenceLocationLocPatch, planarAdjustLocPatch;
    GLint hasHeightMapLocPatch, heightMapLocPatch, heightMapMatrixLocPatch;
    GLint hasUserHeightMapLocPatch;
    GLint heightMapRangeOffsetLocPatch;
    GLint hasDepthMapLocPatch, depthMapLocPatch;
    GLint hasBreakingWaveMapLocPatch, breakingWaveMapLocPatch, breakingWaveMapMatrixLocPatch;
    GLint kexpLocPatch, breakerWavelengthLocPatch, breakerWavelengthVarianceLocPatch, breakerDirectionLocPatch;
    GLint breakerAmplitudeLocPatch, breakerPhaseConstantLocPatch, breakerDepthFalloffLocPatch;
    GLint surgeDepthLocPatch, steepnessVarianceLocPatch, seaLevelLocPatch, planarReflectionBlendLocPatch;
    GLint mvLocPatch, projLocPatch, depthOffsetLocPatch, depthOnlyLocPatch, windDirLocPatch, underwaterLocPatch;
    GLint doubleRefractionColorLocPatch, doubleRefractionIntensityLocPatch, oneOverGammaLocPatch, sunIntensityLocPatch;
    GLint invZoomLocPatch, zNearFarLocPatch, reflectivityScaleLocPatch;
	GLint camHightLocPatch;

    GLuint vertexArray;
    bool hasUnifiedVA, hasMultisample, hasVertexArrays;

    TRITON_STACK(GLint) programStack;

    TRITON_VECTOR(unsigned int) userShaders;

    GLuint meshVboID, meshIdxID, meshVertexArray;
    int meshNIndices;

    GLuint depthFBO, dummyColorBuffer;
    GLint mySavedFrameBuffer;
    int mySavedVP[4];

    bool setDefaultState;

#if PROJECTEDGRIDOPENGL_GLGET_OPT
    GLint iMaxClipPlanes;
    GLint iMaxTextures, iMaxFixedFunctionTextures;
#endif  // PROJECTEDGRIDOPENGL_GLGET_OPT
};
}

#endif