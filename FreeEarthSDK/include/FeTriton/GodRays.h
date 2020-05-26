// Copyright (c) 2014 Sundog Software, LLC. All rights reserved worldwide.

#ifndef TRITON_GODRAYS_H
#define TRITON_GODRAYS_H

/** \file GodRays.h
\brief An underwater crepuscular ray effect.
*/
#include <FeTriton/TritonCommon.h>
#include <FeTriton/Vector3.h>
#include <FeTriton/Matrix3.h>
#include <FeTriton/Matrix4.h>
#include <vector>


namespace Triton
{
class Environment;
class ProjectedGrid;

class GodRays : public MemObject
{
public:
    GodRays(const Environment *env);
    virtual ~GodRays();

    virtual bool Initialize();

    /** Retrieve the underlying shader object for the wake spray particles. */
    virtual ShaderHandle GetShader() const {
        return 0;
    }

    /** Dispose of all shader programs required to render god rays. */
    virtual void DeleteShaders() {}

    /** Reload all shader programs required to render god rays. */
    virtual bool ReloadShaders(const TRITON_VECTOR(unsigned int)& shaders) {
        return true;
    }

    virtual bool Draw(const ProjectedGrid *grid);

    void SetTextures(TextureHandle slopeFoam, TextureHandle displacement);

    virtual void D3D9DeviceLost() {}
    virtual void D3D9DeviceReset() {}

	virtual void update() {}

	void setGodRayPosition(Triton::Vector3 pos)
	{
		mGodRayPosition = pos;
	}
	Triton::Vector3 getGodRayPosition()
	{
		return mGodRayPosition;
	}
	Triton::Vector3 getGodRaysExposure()
	{
		return mGodRaysExposure;
	}
	float getGodRaysIntensity()
	{
		return mGodRaysIntensity;
	}
	Triton::Vector3 getSunColor()
	{
		return sunColor;
	}

	virtual void setGodRayPosMat(Triton::Matrix4 m){}

	virtual void setFarCilp(float farCilp){}

protected:

    virtual bool SetupMesh() = 0;

    virtual bool DrawMesh() = 0;

    virtual bool DeleteMesh() = 0;

    bool ComputeConstants(const ProjectedGrid *grid);

    TextureHandle slopeFoamTex, displacementTex;

    const Environment *environment;

    int N;
    float effectDepth;

    Vector3 viewer;
    Vector3 extinction;
    Vector3 origin;
    Vector3 viewDir;
    float spacing;
    double K;
    Vector3 sunDir, sunColor;
    float Lx, Ly;
    float fade;
	Vector3 mGodRaysExposure;
	float mGodRaysIntensity;
	

	Triton::Vector3 mGodRayPosition;

    Matrix3 basis, invBasis;
};
}

#endif
