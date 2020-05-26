// Copyright (c) 2014 Sundog Software, LLC. All rights reserved worldwide.

#ifndef TRITON_GODRAYS_OPENGL_H
#define TRITON_GODRAYS_OPENGL_H

/** \file GodRaysOpenGL.h
\brief An underwater crepuscular ray effect implemented in OpenGL.
*/
#include <FeTriton/GodRays.h>
#include <FeTriton/TritonOpenGL.h>
#include <osg/Camera>

namespace Triton
{
class GodRaysOpenGL : public GodRays
{
public:
    GodRaysOpenGL(const Environment *env);
    virtual ~GodRaysOpenGL();

    /** Retrieve the underlying shader object for the god rays. */
    virtual ShaderHandle GetShader() const {
        return (ShaderHandle)programCast;
    }

    virtual void DeleteShaders();

    /** Reload all shader programs required to render god rays. */
    virtual bool ReloadShaders(const TRITON_VECTOR(unsigned int)& shaders);

protected:

    virtual bool SetupMesh();

    virtual bool DrawMesh();

    virtual bool DeleteMesh();

private:
    GLhandleARB LoadShader(const char *shaderName, bool vertexShader, bool userShader);
    bool LoadShaders();

    GLuint vboID, idxID, vertexArray;
    GLhandleARB vertShader, fragShader, userShader, userVertShader, program;
    GLuint vertShaderCast, fragShaderCast, userShaderCast, userVertShaderCast, programCast;
    GLint positionLoc, mvpLoc, spacingLoc, originLoc, extinctionLoc, viewerLoc, sunDirLoc;
    GLint basisLoc, invBasisLoc, displacementMapLoc, slopeFoamMapLoc, textureSizeLoc, visibilityLoc;
    GLint viewDirLoc, sunColorLoc, fadeLoc;
    int nIndices;
};

typedef struct RayVertex_S {
	float x, y, z, extrusionLength;
} RayVertex;

class HydraxGodRaysOpenGL : public GodRays
{
public:
	HydraxGodRaysOpenGL(const Environment *env);
	virtual ~HydraxGodRaysOpenGL();

	/** Retrieve the underlying shader object for the god rays. */
	virtual ShaderHandle GetShader() const {
		return (ShaderHandle)programCast;
	}

	virtual void DeleteShaders();

	/** Reload all shader programs required to render god rays. */
	virtual bool ReloadShaders(const TRITON_VECTOR(unsigned int)& shaders);

	void setGodRayPosMat(Triton::Matrix4 m)
	{
		mGodRayPosMat = m;
	}

	void setFarCilp(float farCilp)
	{
		mFarClip = farCilp;
	}

	void update();

protected:

	virtual bool SetupMesh();

	virtual bool DrawMesh();

	virtual bool DeleteMesh();

private:
	GLhandleARB LoadShader(const char *shaderName, bool vertexShader, bool userShader);
	bool LoadShaders();
	bool createGodRays();
	void updateGodRays();
	double getFarWidth();
	osg::Vec2 calculateRayPosition(const int& RayNumber);
	float getWaveHeight(Triton::Vec2 pos);

	GLuint vboID, idxID, vertexArray;
	GLhandleARB vertShader, fragShader, program;
	GLuint vertShaderCast, fragShaderCast,programCast;
	GLint positionLoc,mvpLoc;
	int nIndices;
	int nVerts;

	int mNumberOfRays;
	double mNoiseDerivation;
	double mNoisePositionMultiplier;
	double mNoiseYNormalMultiplier;
	double mNoiseNormalMultiplier;
	double mRaysSize;
	
	float mNearClip,mFarClip,mFov,mAspect;

	Triton::Matrix4 mGodRayPosMat;
	RayVertex *mVertexArr;
	
};

}

#endif
