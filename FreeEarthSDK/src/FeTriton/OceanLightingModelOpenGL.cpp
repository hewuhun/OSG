// Copyright (c) 2011-2014 Sundog Software LLC. All rights reserved worldwide.

#include <FeTriton/TritonCommon.h>
#include <FeTriton/OceanLightingModelOpenGL.h>
#include <FeTriton/Environment.h>
#include <FeTriton/TGALoader.h>
#include <FeTriton/Configuration.h>

#if defined(WIN32) || defined(WIN64)
#include <Windows.h>
#endif


using namespace Triton;


OceanLightingModelOpenGL::~OceanLightingModelOpenGL()
{
    if (foamTexture)
        glDeleteTextures(1, &foamTexture);
    if (lightFoamTexture)
        glDeleteTextures(1, &lightFoamTexture);
    if (noiseTexture)
        glDeleteTextures(1, &noiseTexture);
    if (washTexture)
        glDeleteTextures(1, &washTexture);
    if (breakerTexture)
        glDeleteTextures(1, &breakerTexture);
}

static void LoadTexture(TGALoader& loader, const Environment *environment)
{
	osg::GLExtensions* et = environment->GetExtension();
	if(!et)
		return;

#ifdef __APPLE__
    if (loader.GetBitsPerPixel() == 24) {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_COMPRESSED_RGB, loader.GetWidth(), loader.GetHeight(), 0, GL_RGB, GL_UNSIGNED_BYTE, loader.GetPixels());
    } else {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_COMPRESSED_RGBA, loader.GetWidth(), loader.GetHeight(), 0, GL_RGBA, GL_UNSIGNED_BYTE, loader.GetPixels());
    }
    et->glGenerateMipmap(GL_TEXTURE_2D);
#else
    bool hasFBO = OpenGLExtensionManager::HasExtension("GL_EXT_framebuffer_object",environment);
    if (hasFBO || environment->GetRenderer() >= OPENGL_3_2) {
        if (loader.GetBitsPerPixel() == 24) {
            glTexImage2D(GL_TEXTURE_2D, 0, GL_COMPRESSED_RGB, loader.GetWidth(), loader.GetHeight(), 0, GL_RGB, GL_UNSIGNED_BYTE, loader.GetPixels());
        } else {
            glTexImage2D(GL_TEXTURE_2D, 0, GL_COMPRESSED_RGBA, loader.GetWidth(), loader.GetHeight(), 0, GL_RGBA, GL_UNSIGNED_BYTE, loader.GetPixels());
        }
        et->glGenerateMipmap(GL_TEXTURE_2D);
    } else {
        if (loader.GetBitsPerPixel() == 24) {
            gluBuild2DMipmaps(GL_TEXTURE_2D, GL_RGB, loader.GetWidth(), loader.GetHeight(), GL_RGB, GL_UNSIGNED_BYTE, loader.GetPixels());
        } else {
            gluBuild2DMipmaps(GL_TEXTURE_2D, GL_RGBA, loader.GetWidth(), loader.GetHeight(), GL_RGBA, GL_UNSIGNED_BYTE, loader.GetPixels());
        }
    }
#endif
}

void OceanLightingModelOpenGL::ReloadGridUniforms(ShaderHandle gridShader, ShaderHandle gridPatchShader)
{
	if(!environment)
		return;

	osg::GLExtensions* et = environment->GetExtension();
	if(!et)
		return;

    OceanLightingModel::ReloadGridUniforms(gridShader, gridPatchShader);

    Lloc = et->glGetUniformLocation((GLint)gridShader, "trit_L");
    lightColorLoc = et->glGetUniformLocation((GLint)gridShader, "trit_lightColor");
    ambientColorLoc = et->glGetUniformLocation((GLint)gridShader, "trit_ambientColor");
    refractColorLoc = et->glGetUniformLocation((GLint)gridShader, "trit_refractColor");
    cubeMapLoc = et->glGetUniformLocation((GLint)gridShader, "trit_cubeMap");
    shininessLoc = et->glGetUniformLocation((GLint)gridShader, "trit_shininess");
    foamScaleLoc = et->glGetUniformLocation((GLint)gridShader, "trit_foamScale");
    foamBlendLoc = et->glGetUniformLocation((GLint)gridShader, "trit_foamBlend");
    noiseTexLoc = et->glGetUniformLocation((GLint)gridShader, "trit_noiseTex");
    hasEnvMapLoc = et->glGetUniformLocation((GLint)gridShader, "trit_hasEnvMap");
    planarReflectionMapLoc = et->glGetUniformLocation((GLint)gridShader, "trit_planarReflectionMap");
    hasPlanarReflectionMapLoc = et->glGetUniformLocation((GLint)gridShader, "trit_hasPlanarReflectionMap");
    planarReflectionDisplacementScaleLoc = et->glGetUniformLocation((GLint)gridShader, "trit_planarReflectionDisplacementScale");
    washTexLoc = et->glGetUniformLocation((GLint)gridShader, "trit_washTex");
    lightFoamTexLoc = et->glGetUniformLocation((GLint)gridShader, "trit_lightFoamTex");
    breakerTexLoc = et->glGetUniformLocation((GLint)gridShader, "trit_breakerTex");

    LlocPatch = et->glGetUniformLocation((GLint)gridPatchShader, "trit_L");
	LlocCustomPatch = et->glGetUniformLocation((GLint)gridPatchShader, "trit_custom_L");
    lightColorLocPatch = et->glGetUniformLocation((GLint)gridPatchShader, "trit_lightColor");
    ambientColorLocPatch = et->glGetUniformLocation((GLint)gridPatchShader, "trit_ambientColor");
    refractColorLocPatch = et->glGetUniformLocation((GLint)gridPatchShader, "trit_refractColor");
    cubeMapLocPatch = et->glGetUniformLocation((GLint)gridPatchShader, "trit_cubeMap");
    shininessLocPatch = et->glGetUniformLocation((GLint)gridPatchShader, "trit_shininess");
    foamScaleLocPatch = et->glGetUniformLocation((GLint)gridPatchShader, "trit_foamScale");
    foamBlendLocPatch = et->glGetUniformLocation((GLint)gridPatchShader, "trit_foamBlend");
    noiseTexLocPatch = et->glGetUniformLocation((GLint)gridPatchShader, "trit_noiseTex");
    hasEnvMapLocPatch = et->glGetUniformLocation((GLint)gridPatchShader, "trit_hasEnvMap");
    planarReflectionMapLocPatch = et->glGetUniformLocation((GLint)gridPatchShader, "trit_planarReflectionMap");
    hasPlanarReflectionMapLocPatch = et->glGetUniformLocation((GLint)gridPatchShader, "trit_hasPlanarReflectionMap");
    planarReflectionDisplacementScaleLocPatch = et->glGetUniformLocation((GLint)gridPatchShader, "trit_planarReflectionDisplacementScale");
    washTexLocPatch = et->glGetUniformLocation((GLint)gridPatchShader, "trit_washTex");
    lightFoamTexLocPatch = et->glGetUniformLocation((GLint)gridPatchShader, "trit_lightFoamTex");
    breakerTexLocPatch = et->glGetUniformLocation((GLint)gridPatchShader, "trit_breakerTex");
}

bool OceanLightingModelOpenGL::Initialize(ShaderHandle shader, ShaderHandle patchShader)
{
    Utils::ClearGLErrors();

    ReloadGridUniforms(shader, patchShader);

    hiResFoam = false;
    Configuration::GetBoolValue("hi-res-foam", hiResFoam);

    TGALoader loader;

    glGenTextures(1, &lightFoamTexture);
    if (loader.Load(hiResFoam ? "lightfoam-hires.tga" : "lightfoam.tga", environment->GetResourceLoader())) {
        glBindTexture(GL_TEXTURE_2D, lightFoamTexture);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        LoadTexture(loader, environment);
    } else {
        Utils::DebugMsg("Could not load the OpenGL light foam texture.");
        return false;
    }

    glGenTextures(1, &noiseTexture);
    if (loader.Load("perlin.tga", environment->GetResourceLoader())) {
        glBindTexture(GL_TEXTURE_2D, noiseTexture);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        LoadTexture(loader, environment);
    } else {
        Utils::DebugMsg("Could not load the OpenGL noise texture.");
        return false;
    }

    glGenTextures(1, &washTexture);
    if (loader.Load("wash-half.tga", environment->GetResourceLoader())) {
        glBindTexture(GL_TEXTURE_2D, washTexture);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        LoadTexture(loader, environment);
    } else {
        Utils::DebugMsg("Could not load the OpenGL propellor wash texture.");
        return false;
    }

    glGenTextures(1, &breakerTexture);
    if (loader.Load("breaker.tga", environment->GetResourceLoader())) {
        glBindTexture(GL_TEXTURE_2D, breakerTexture);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        LoadTexture(loader, environment);
    } else {
        Utils::DebugMsg("Could not load the OpenGL breaking wave texture.");
        return false;
    }

    if (!Utils::PrintGLErrors(__FILE__, __LINE__)) {
        return false;
    }

    return OceanLightingModel::Initialize(shader, patchShader);
}

bool OceanLightingModelOpenGL::PrepShaders()
{
    if (!environment) return false;

    ReadConfigSettings();

    Utils::ClearGLErrors();

    Vector3 L = environment->GetLightDirection();
    Vector3 C = environment->GetDirectionalLightColor();
    Vector3 A = environment->GetAmbientLightColor();

    //glUseProgram((GLint)shaderHandle);

    OpenGLCachedUniforms::SetUniformVec3(environment, (GLint)shaderHandle, Lloc, (float)L.x, (float)L.y, (float)L.z);
    OpenGLCachedUniforms::SetUniformVec3(environment, (GLint)shaderHandle, lightColorLoc, (float)C.x, (float)C.y, (float)C.z);
    OpenGLCachedUniforms::SetUniformVec3(environment, (GLint)shaderHandle, ambientColorLoc, (float)A.x, (float)A.y, (float)A.z);

    Vector3 refract = GetRefractionColor();
    OpenGLCachedUniforms::SetUniformVec3(environment, (GLint)shaderHandle, refractColorLoc, (float)refract.x, (float)refract.y, (float)refract.z);
    OpenGLCachedUniforms::SetUniformFloat(environment, (GLint)shaderHandle, shininessLoc, (float)shininess);
    OpenGLCachedUniforms::SetUniformFloat(environment, (GLint)shaderHandle, foamScaleLoc, foamScale);
    OpenGLCachedUniforms::SetUniformFloat(environment, (GLint)shaderHandle, foamBlendLoc, foamBlend);

    if (environment->GetEnvironmentMap()) {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP_EXT, (GLuint)environment->GetEnvironmentMap());
        OpenGLCachedUniforms::SetUniformInt(environment, (GLint)shaderHandle, cubeMapLoc, 0);
        OpenGLCachedUniforms::SetUniformInt(environment, (GLint)shaderHandle, hasEnvMapLoc, GL_TRUE);
    } else {
        // MS 7/7/2015: This is similar to WL's note below.  Our application happened to have a cube map bound to GL_TEXTURE0.
        // We should unbind the cube map here, or the last binding will still be bound.
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP_EXT, 0);
        OpenGLCachedUniforms::SetUniformInt(environment, (GLint)shaderHandle, hasEnvMapLoc, GL_FALSE);
    }

    if (planarReflectionDisplacementScaleLoc != -1)
        OpenGLCachedUniforms::SetUniformFloat(environment, (GLint)shaderHandle, planarReflectionDisplacementScaleLoc, environment->GetPlanarReflectionDisplacementScale());

    if (environment->GetPlanarReflectionMap()) {
        glActiveTexture(GL_TEXTURE6);
        glBindTexture(GL_TEXTURE_2D, (GLuint)environment->GetPlanarReflectionMap());
        OpenGLCachedUniforms::SetUniformInt(environment, (GLint)shaderHandle, planarReflectionMapLoc, 6);
        OpenGLCachedUniforms::SetUniformInt(environment, (GLint)shaderHandle, hasPlanarReflectionMapLoc, GL_TRUE);

    } else {
        // WL 08/28/2011: Surpisingly, on my Nvidia GTX 580
        // hasPlanarReflectionMap set to false was not enough for shaders to work correctly.
        // Problem looked like shader was still trying to make lookup to invalid planar
        // texture despite lookup being wrapped with if( hasPlanarReflectionMap ) condition.
        // And probably due to some numerical error during execution was returning non zero
        // planar reflections...
        // I can't explain this. Perhaps I did something incorrectly or thats the bug in shader compiler ??
        // Anyway I noticed setting texture = 0 and texture sampler removed this problem...
        // So this explains why these calls are here..
        glActiveTexture(GL_TEXTURE6);
        glBindTexture(GL_TEXTURE_2D, 0);
        OpenGLCachedUniforms::SetUniformInt(environment, (GLint)shaderHandle, planarReflectionMapLoc, 6);
        OpenGLCachedUniforms::SetUniformInt(environment, (GLint)shaderHandle, hasPlanarReflectionMapLoc, GL_FALSE);
    }

    glActiveTexture(GL_TEXTURE4);
    glBindTexture(GL_TEXTURE_2D, washTexture);
    if (environment->GetRenderer() < OPENGL_3_2) {
        glTexEnvf(GL_TEXTURE_FILTER_CONTROL, GL_TEXTURE_LOD_BIAS, propWashLodBias);
    }
    OpenGLCachedUniforms::SetUniformInt(environment, (GLint)shaderHandle, washTexLoc, 4);

    glActiveTexture(GL_TEXTURE7);
    glBindTexture(GL_TEXTURE_2D, noiseTexture);
    OpenGLCachedUniforms::SetUniformInt(environment, (GLint)shaderHandle, noiseTexLoc, 7);

    if (breakerTexLoc != -1) {
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, breakerTexture);
        OpenGLCachedUniforms::SetUniformInt(environment, (GLint)shaderHandle, breakerTexLoc, 1);
    }

    if (lightFoamTexLoc != -1) {
        glActiveTexture(GL_TEXTURE9);
        glBindTexture(GL_TEXTURE_2D, lightFoamTexture);
        OpenGLCachedUniforms::SetUniformInt(environment, (GLint)shaderHandle, lightFoamTexLoc, 9);
    }

    return Utils::PrintGLErrors(__FILE__, __LINE__);
}

bool OceanLightingModelOpenGL::PrepPatchShader()
{
    if (!environment) return false;

    ReadConfigSettings();

    Utils::ClearGLErrors();

    Vector3 L = environment->GetLightDirection();
    Vector3 C = environment->GetDirectionalLightColor();
    Vector3 A = environment->GetAmbientLightColor();

    //glUseProgram((GLint)shaderHandlePatch);

    OpenGLCachedUniforms::SetUniformVec3(environment, (GLint)shaderHandlePatch, LlocPatch, (float)L.x, (float)L.y, (float)L.z);
	OpenGLCachedUniforms::SetUniformVec3(environment, (GLint)shaderHandlePatch, LlocCustomPatch, (float)L.x, (float)L.y, (float)L.z);
    OpenGLCachedUniforms::SetUniformVec3(environment, (GLint)shaderHandlePatch, lightColorLocPatch, (float)C.x, (float)C.y, (float)C.z);
    OpenGLCachedUniforms::SetUniformVec3(environment, (GLint)shaderHandlePatch, ambientColorLocPatch, (float)A.x, (float)A.y, (float)A.z);

    Vector3 refract = GetRefractionColor();
    OpenGLCachedUniforms::SetUniformVec3(environment, (GLint)shaderHandlePatch, refractColorLocPatch, (float)refract.x, (float)refract.y, (float)refract.z);
    OpenGLCachedUniforms::SetUniformFloat(environment, (GLint)shaderHandlePatch, shininessLocPatch, (float)shininess);
    OpenGLCachedUniforms::SetUniformFloat(environment, (GLint)shaderHandlePatch, foamScaleLocPatch, foamScale);
    OpenGLCachedUniforms::SetUniformFloat(environment, (GLint)shaderHandlePatch, foamBlendLocPatch, foamBlend);

    if (environment->GetEnvironmentMap()) {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP_EXT, (GLuint)environment->GetEnvironmentMap());
        OpenGLCachedUniforms::SetUniformInt(environment, (GLint)shaderHandlePatch, cubeMapLocPatch, 0);
        OpenGLCachedUniforms::SetUniformInt(environment, (GLint)shaderHandlePatch, hasEnvMapLocPatch, GL_TRUE);
    } else {
        // MS 7/7/2015: This is similar to WL's note below.  Our application happened to have a cube map bound to GL_TEXTURE0.
        // We should unbind the cube map here, or the last binding will still be bound.
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP_EXT, 0);
        OpenGLCachedUniforms::SetUniformInt(environment, (GLint)shaderHandlePatch, hasEnvMapLocPatch, GL_FALSE);
    }

    if (planarReflectionDisplacementScaleLocPatch != -1)
        OpenGLCachedUniforms::SetUniformFloat(environment, (GLint)shaderHandlePatch, planarReflectionDisplacementScaleLocPatch, environment->GetPlanarReflectionDisplacementScale());

	
    if (environment->GetPlanarReflectionMap())
	{
        glActiveTexture(GL_TEXTURE6);
        glBindTexture(GL_TEXTURE_2D, (GLuint)environment->GetPlanarReflectionMap());
        OpenGLCachedUniforms::SetUniformInt(environment, (GLint)shaderHandlePatch, planarReflectionMapLocPatch, 6);
        OpenGLCachedUniforms::SetUniformInt(environment, (GLint)shaderHandlePatch, hasPlanarReflectionMapLocPatch, GL_TRUE);

    } 
	else
	{
        // WL 08/28/2011: Surpisingly, on my Nvidia GTX 580
        // hasPlanarReflectionMap set to false was not enough for shaders to work correctly.
        // Problem looked like shader was still trying to make lookup to invalid planar
        // texture despite lookup being wrapped with if( hasPlanarReflectionMap ) condition.
        // And probably due to some numerical error during execution was returning non zero
        // planar reflections...
        // I can't explain this. Perhaps I did something incorrectly or thats the bug in shader compiler ??
        // Anyway I noticed setting texture = 0 and texture sampler removed this problem...
        // So this explains why these calls are here..
        glActiveTexture(GL_TEXTURE6);
        glBindTexture(GL_TEXTURE_2D, 0);
        OpenGLCachedUniforms::SetUniformInt(environment, (GLint)shaderHandlePatch, planarReflectionMapLocPatch, 6);
        OpenGLCachedUniforms::SetUniformInt(environment, (GLint)shaderHandlePatch, hasPlanarReflectionMapLocPatch, GL_FALSE);
    }

    glActiveTexture(GL_TEXTURE4);
    glBindTexture(GL_TEXTURE_2D, washTexture);
    if (environment->GetRenderer() < OPENGL_3_2) {
        glTexEnvf(GL_TEXTURE_FILTER_CONTROL, GL_TEXTURE_LOD_BIAS, propWashLodBias);
    }
    OpenGLCachedUniforms::SetUniformInt(environment, (GLint)shaderHandlePatch, washTexLocPatch, 4);

    glActiveTexture(GL_TEXTURE7);
    glBindTexture(GL_TEXTURE_2D, noiseTexture);
    OpenGLCachedUniforms::SetUniformInt(environment, (GLint)shaderHandlePatch, noiseTexLocPatch, 7);

    if (breakerTexLocPatch != -1) {
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, breakerTexture);
        OpenGLCachedUniforms::SetUniformInt(environment, (GLint)shaderHandlePatch, breakerTexLocPatch, 1);
    }

    if (lightFoamTexLocPatch != -1) {
        glActiveTexture(GL_TEXTURE9);
        glBindTexture(GL_TEXTURE_2D, lightFoamTexture);
        OpenGLCachedUniforms::SetUniformInt(environment, (GLint)shaderHandlePatch, lightFoamTexLocPatch, 9);
    }

    return Utils::PrintGLErrors(__FILE__, __LINE__);
}



