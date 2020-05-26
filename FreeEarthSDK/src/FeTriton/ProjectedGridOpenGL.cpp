// Copyright (c) 2011-2015 Sundog Software, LLC. All rights reserved worldwide.

#include <FeTriton/TritonCommon.h>
#include <FeTriton/ProjectedGridOpenGL.h>
#include <FeTriton/Environment.h>
#include <FeTriton/Matrix4.h>
#include <FeTriton/WaterModel.h>
#include <FeTriton/OceanLightingModel.h>
#include <FeTriton/WakeManager.h>
#include <FeTriton/Configuration.h>
#include <stdio.h>
#include <osg/BufferObject>

#ifdef __APPLE__
// Without the color attachment on Mac, the depth framebuffer is considered incomplete.
#define DUMMY_COLOR_ATTACHMENT
#endif

//#define USE_DEPTH_STENCIL_ATTACHMENT

// For PBO/VBO offsets
#define BUFFER_OFFSET(i) (GLvoid *)((char *)0 + (i))

using namespace Triton;

typedef struct Vertex_S {
    float x, y;
} Vertex;

bool ProjectedGridOpenGL::Initialize(const Environment *env, WaterModelTypes type,
                                     unsigned int gridResolution, unsigned int meshGridResolution)
{
    if (ProjectedGrid::Initialize(env, type, gridResolution, meshGridResolution)) {
        nIndices = 0;

        hasUnifiedVA = OpenGLExtensionManager::HasExtension("GL_NV_vertex_buffer_unified_memory",env)
                       && OpenGLExtensionManager::HasExtension("GL_NV_shader_buffer_load",env);
        hasMultisample = OpenGLExtensionManager::HasExtension("GL_ARB_multisample",env);

        if (!TestCapabilities()) return false;
        //if (!SetupVBO(gridResolution)) return false;
        if (!SetupShaders()) return false;
        if (!SetupMesh()) return false;

        glGenTextures(1, &dummyHeightMapID);
        glBindTexture(GL_TEXTURE_2D, dummyHeightMapID);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);

        Configuration::GetBoolValue("set-default-state", setDefaultState);

#if PROJECTEDGRIDOPENGL_GLGET_OPT
        iMaxClipPlanes = -1;
        iMaxTextures = -1;
        iMaxFixedFunctionTextures = -1;
#endif  // PROJECTEDGRIDOPENGL_GLGET_OPT

        return true;
    }

    return false;
}

ProjectedGridOpenGL::~ProjectedGridOpenGL()
{
    //Utils::ClearGLErrors();
	if(environment)
	{
		osg::GLExtensions* et = environment->GetExtension();
		if(et)
		{
			if (hasVertexArrays) {
				et->glDeleteVertexArrays(1, &vertexArray);
			}

			if (vboID) {
				et->glDeleteBuffers(1, &vboID);
			}

			if (idxID) {
				et->glDeleteBuffers(1, &idxID);
			}

			if (hasVertexArrays && meshVertexArray) {
				et->glDeleteVertexArrays(1, &meshVertexArray);
			}

			if (meshVboID) {
				et->glDeleteBuffers(1, &meshVboID);
			}

			if (meshIdxID) {
				et->glDeleteBuffers(1, &meshIdxID);
			}

			if (dummyHeightMapID) {
				glDeleteTextures(1, &dummyHeightMapID);
			}

			DeleteShaders();

			DeleteDepthTexture();
		}
	}
}

void ProjectedGridOpenGL::SetupDepthTexture()
{
    if (depthFBO || depthTexture) return;

	if(!environment)
		return;
	osg::GLExtensions* et = environment->GetExtension();
	if(!et)
		return;

    GLint savedFrameBuffer = 0;
    glGetIntegerv(GL_FRAMEBUFFER_BINDING, &savedFrameBuffer);

    et->glGenFramebuffers(1, &depthFBO);
    et->glBindFramebuffer(GL_FRAMEBUFFER, depthFBO);

    // create texture for framebuffer
    unsigned int fb_tex = 0;
    glGenTextures(1, &fb_tex);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, fb_tex);

    glTexImage2D(
        GL_TEXTURE_2D,
        0,
#ifdef USE_DEPTH_STENCIL_ATTACHMENT
        GL_DEPTH_STENCIL,
#else
#ifdef __APPLE__
        GL_DEPTH_COMPONENT32,
#else
        GL_DEPTH_COMPONENT,
#endif
#endif
        depthTextureSize,
        depthTextureSize,
        0,
        GL_DEPTH_COMPONENT,
        GL_UNSIGNED_INT,
        NULL
    );

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    // attach depth texture to framebuffer
#ifdef USE_DEPTH_STENCIL_ATTACHMENT
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, fb_tex, 0);
#else
    et->glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, fb_tex, 0);
#endif

#ifdef DUMMY_COLOR_ATTACHMENT
    GLuint colorTex;
    glGenTextures(1, &colorTex);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, colorTex);
    glTexImage2D(
        GL_TEXTURE_2D,
        0,
        GL_RGBA,
        depthTextureSize,
        depthTextureSize,
        0,
        GL_RGBA,
        GL_UNSIGNED_BYTE,
        NULL
    );

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorTex, 0);

    dummyColorBuffer = colorTex;
#else
    dummyColorBuffer = 0;
#endif

    // tell framebuffer not to use any colour drawing outputs
    GLenum draw_bufs[] = { GL_NONE };
    et->glDrawBuffers(1, draw_bufs);
    glReadBuffer(GL_NONE);
    glDrawBuffer(GL_NONE);

    // bind default framebuffer again
    et->glBindFramebuffer(GL_FRAMEBUFFER, savedFrameBuffer);

    depthTexture = (TextureHandle)fb_tex;

    Utils::PrintGLErrors(__FILE__, __LINE__);
}

void ProjectedGridOpenGL::DeleteDepthTexture()
{
    GLuint fb_tex = (unsigned int)depthTexture;
    if (fb_tex) {
        glDeleteTextures(1, &fb_tex);
        depthTexture = 0;
    }

    if (dummyColorBuffer) {
        glDeleteTextures(1, &dummyColorBuffer);
        dummyColorBuffer = 0;
    }

	if(environment)
	{
		osg::GLExtensions* et = environment->GetExtension();
		if(et)
		{
			if (depthFBO)
			{
				et->glDeleteFramebuffers(1, &depthFBO);
				depthFBO = 0;
			}
		}
	}   
}

bool ProjectedGridOpenGL::ReloadShaders(const TRITON_VECTOR(unsigned int)& shaders)
{
    bool ok = false;

    userShaders = shaders;

    DeleteShaders();
    ok = SetupShaders();

    if (ok && GetWaterModel() && GetLightingModel()) {
        GetWaterModel()->ReloadGridUniforms(GetShader(), GetPatchShader());
        GetLightingModel()->ReloadGridUniforms(GetShader(), GetPatchShader());
    }

    return ok;
}

void ProjectedGridOpenGL::DeleteShaders()
{
	if(environment)
	{
		osg::GLExtensions* et = environment->GetExtension();
		if(et)
		{
			if (vertCastShader && programCast) {
				et->glDetachShader(programCast, vertCastShader);
				et->glDeleteShader(vertCastShader);
				vertCastShader = 0;
			}

			if (fragCastShader && programCast) {
				et->glDetachShader(programCast, fragCastShader);
				et->glDeleteShader(fragCastShader);
				fragCastShader = 0;
			}

			if (userCastShader && programCast) {
				et->glDetachShader(programCast, userCastShader);
				et->glDeleteShader(userCastShader);
				userCastShader = 0;
			}

			if (userVertCastShader && programCast) {
				et->glDetachShader(programCast, userVertCastShader);
				et->glDeleteShader(userVertCastShader);
				userVertCastShader = 0;
			}

			if (programCast) {
				et->glDeleteProgram(programCast);
				programCast = 0;
			}

			if (vertCastShaderPatch && programCastPatch) {
				et->glDetachShader(programCastPatch, vertCastShaderPatch);
				et->glDeleteShader(vertCastShaderPatch);
				vertCastShaderPatch = 0;
			}

			if (fragCastShaderPatch && programCastPatch) {
				et->glDetachShader(programCastPatch, fragCastShaderPatch);
				et->glDeleteShader(fragCastShaderPatch);
				fragCastShaderPatch = 0;
			}

			if (programCastPatch) {
				et->glDeleteProgram(programCastPatch);
				programCastPatch = 0;
			}
		}
	}
}

bool ProjectedGridOpenGL::TestCapabilities()
{
    const GLubyte *version = glGetString(GL_VERSION);
    if (!version) return false;

    if (!(version[0] >= '2')) {
        Utils::DebugMsg("OpenGL 2.0 capabilities not present on this system.");
        return false;
    }

    return true;
}

static void PrintGLSLInfoLog(GLhandleARB obj,const Environment* env)
{
    int infologLength = 0;
    int charsWritten  = 0;
    char *infoLog;
    GLuint objCast;

	if(!env)
		return;
	osg::GLExtensions* et = env->GetExtension();
	if(!et)
		return;

#ifdef __APPLE__
    objCast = reinterpret_cast<uintptr_t>(obj);
#else
    objCast = obj;
#endif

    Utils::DebugMsg("GLSL error detected in projected grid!\n");

    if (OpenGLExtensionManager::HasExtension("GL_ARB_shader_objects",env)) {
        et->glGetObjectParameterivARB(obj, GL_OBJECT_INFO_LOG_LENGTH_ARB,
                                  &infologLength);

        if (infologLength > 0) {
            infoLog = (char *)TRITON_MALLOC(infologLength);
            et->glGetInfoLogARB(obj, infologLength, &charsWritten, infoLog);
            Utils::DebugMsg(infoLog);
            TRITON_FREE(infoLog);
        }
    } else {
        if (et->glIsShader(objCast)) {
            et->glGetShaderiv(objCast, GL_INFO_LOG_LENGTH, &infologLength);
        } else {
            et->glGetProgramiv(objCast, GL_INFO_LOG_LENGTH, &infologLength);
        }

        if (infologLength > 0) {
            infoLog = (char *)TRITON_MALLOC(infologLength);
            if (!et->glIsShader(objCast)) {
                et->glGetProgramInfoLog(objCast, infologLength, &charsWritten, infoLog);
            } else {
                et->glGetShaderInfoLog(objCast, infologLength, &charsWritten, infoLog);
            }
            Utils::DebugMsg(infoLog);
            TRITON_FREE(infoLog);
        }
    }
}

GLhandleARB ProjectedGridOpenGL::LoadShader(const char *shaderName, bool vertexShader,
        bool userShader )
{
    char *data = 0;
    unsigned int dataLen = 0;

#ifdef __APPLE__
    GLuint shader = 0;
#else
    GLhandleARB shader = 0;
#endif

    if (!environment) return 0;

	osg::GLExtensions* et = environment->GetExtension();
	if(!et)
		return 0;

    Utils::ClearGLErrors();

    if (environment->GetResourceLoader()->LoadResource(
                shaderName, data, dataLen, true)) {
        shader = et->glCreateShader(vertexShader ? GL_VERTEX_SHADER : GL_FRAGMENT_SHADER);

        bool enableBackwash = false;
        Configuration::GetBoolValue("wake-propeller-backwash", enableBackwash);

        bool enableBreakingWavesMap = false;
        Configuration::GetBoolValue("enable-breaking-waves-map", enableBreakingWavesMap);

        bool enableKelvin = true;
        Configuration::GetBoolValue("wake-kelvin-wakes", enableKelvin);

        bool noKelvinATI = false;
        Configuration::GetBoolValue("disable-kelvin-wakes-AMD", noKelvinATI);
        if (noKelvinATI) {
            const GLubyte *str;
            str = glGetString(GL_VENDOR);
            if (str) {
                bool isATI = strstr((const char *)str, "ATI") != 0 || strstr((const char*)str, "Advanced Micro Devices, Inc.") != 0;
                if (isATI) enableKelvin = false;
            }
        }

        GLint numTextureUnits;
        glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &numTextureUnits);
        if (numTextureUnits <= 8) enableKelvin = false;

        bool leewardDampening = false;
        Configuration::GetBoolValue("leeward-dampening", leewardDampening);

        bool displacementDetail = false;
        Configuration::GetBoolValue("detail-displacement", displacementDetail);

        bool perFragmentPropWash = false;
        Configuration::GetBoolValue("per-fragment-prop-wash", perFragmentPropWash);

        float displacementFactor = 0.5f;
        Configuration::GetFloatValue("detail-displacement-intensity", displacementFactor);

        bool fresnelApproximation = false;
        Configuration::GetBoolValue("fresnel-approximation", fresnelApproximation);

        bool highAlt = false;
        Configuration::GetBoolValue("high-altitude-effects", highAlt);

        bool sparkle = false;
        Configuration::GetBoolValue("enable-specular-sparkle", sparkle);

        const char *sources[21];
        int numSources = 0;

        if (environment->GetRenderer() >= OPENGL_3_2) {
            sources[numSources++] = "#version 150\n#define OPENGL32\n";
        } else {
            sources[numSources++] = "#version 120\n";
        }

        if ( userShader ) {
            const char *userString = (vertexShader) ?
                                     environment->GetUserDefinedVertString() : environment->GetUserDefinedFragString();
            if( userString )
                sources[numSources++] = userString;
        }

        if (highAlt) {
            sources[numSources++] = "#define HIGHALT\n";
        }

        if (displacementDetail) {
            sources[numSources++] = "#define DISPLACEMENT_DETAIL\n";
        }

        if (perFragmentPropWash) {
            sources[numSources++] = "#define PER_FRAGMENT_PROP_WASH\n";
        }

        if (fresnelApproximation) {
            sources[numSources++] = "#define FAST_FRESNEL\n";
        }

        if (sparkle) {
            sources[numSources++] = "#define SPARKLE\n";
        }

        char buf[256], buf2[256], buf3[256], buf4[256], buf5[256], buf6[256];
#if (_MSC_VER > 1310)
        sprintf_s(buf5, 256, "#define DISPLACEMENT_DETAIL_FACTOR %f\n", displacementFactor);
#else
        sprintf(buf5, "#define DISPLACEMENT_DETAIL_FACTOR %f\n", displacementFactor);
#endif
        sources[numSources++] = buf5;

        if (OpenGLExtensionManager::HasUBO(environment)) {
            if (environment->GetRenderer() >= OPENGL_3_2) {
                sources[numSources++] = "#define USE_UBO\n";
            } else {
                sources[numSources++] = "#extension GL_ARB_uniform_buffer_object : enable\n#define USE_UBO\n";
            }
        }

        if (enableBreakingWavesMap) {
            sources[numSources++] = "#define BREAKING_WAVES_MAP\n";
        }

        if (enableBreakingWaves) {
            sources[numSources++] = "#define BREAKING_WAVES\n";
        }

        if (environment->GetHDREnabled()) {
            sources[numSources++] = "#define HDR\n";
        }

        if (leewardDampening) {
            sources[numSources++] = "#define LEEWARD_DAMPENING\n";
        }

        int maxWakes = 25;
        Configuration::GetIntValue("max-wake-waves-opengl", maxWakes);
#if (_MSC_VER > 1310)
        sprintf_s(buf, 256, "#define MAX_KELVIN_WAKES %d\n", maxWakes);
#else
        sprintf(buf, "#define MAX_KELVIN_WAKES %d\n", maxWakes);
#endif
        sources[numSources++] = buf;

        if (maxWakes < 1) {
            enableKelvin = false;
        }

        if (enableKelvin) {
            sources[numSources++] = "#define KELVIN_WAKES\n";
        }

        Configuration::GetIntValue("max-circular-waves-opengl", maxWakes);
#if (_MSC_VER > 1310)
        sprintf_s(buf2, 256, "#define MAX_CIRCULAR_WAVES %d\n", maxWakes);
#else
        sprintf(buf2, "#define MAX_CIRCULAR_WAVES %d\n", maxWakes);
#endif
        sources[numSources++] = buf2;

        Configuration::GetIntValue("max-prop-wash-opengl", maxWakes);
#if (_MSC_VER > 1310)
        sprintf_s(buf3, 256, "#define MAX_PROP_WASHES %d\n", maxWakes);
#else
        sprintf(buf3, "#define MAX_PROP_WASHES %d\n", maxWakes);
#endif

        sources[numSources++] = buf3;

        if (maxWakes < 1) {
            enableBackwash = false;
        }

        if (enableBackwash) {
            sources[numSources++] = "#define PROPELLER_WASH\n";
        }

        Configuration::GetIntValue("max-leeward-dampeners-opengl", maxWakes);
#if (_MSC_VER > 1310)
        sprintf_s(buf4, 256, "#define MAX_LEEWARD_DAMPENERS %d\n", maxWakes);
#else
        sprintf(buf4, "#define MAX_LEEWARD_DAMPENERS %d\n", maxWakes);
#endif

        sources[numSources++] = buf4;

#if (_MSC_VER > 1310)
        sprintf_s(buf6, 256, "#define NUM_OCTAVES %d\n", numOctaves);
#else
        sprintf(buf6, "#define NUM_OCTAVES %d\n", numOctaves);
#endif

        sources[numSources++] = buf6;

        sources[numSources++] = data;

        et->glShaderSource(shader, numSources, sources, NULL);

        et->glCompileShader(shader);

        environment->GetResourceLoader()->FreeResource(data);

        GLint ok;
        et->glGetShaderiv(shader, GL_COMPILE_STATUS, &ok);
        if (!ok) {
#ifdef __APPLE__
            PrintGLSLInfoLog(reinterpret_cast<void *>(shader));
#else
            PrintGLSLInfoLog(shader,environment);
#endif
            return 0;
        }

        if (!Utils::PrintGLErrors(__FILE__, __LINE__)) {
            return 0;
        } else {
#ifdef __APPLE__
            return reinterpret_cast<void *>(shader);
#else
            return shader;
#endif
        }
    }

    return 0;
}

/** SetupShaders() - Called by Initialize() and ReloadShaders() to
 *    create our 2 projected grid shader programs, and query their
 *    uniform and attribute locations for future reference.
 */

bool ProjectedGridOpenGL::SetupShaders()
{
    if (!environment) return false;

	osg::GLExtensions* et = environment->GetExtension();
	if(!et)
		return false;

    OpenGLCachedUniforms::ClearCaches();

    Utils::ClearGLErrors();

    vertCastShader = fragCastShader = programCast = 0;
    vertCastShaderPatch = fragCastShaderPatch = userCastShader = 0;

    hasVertexArrays = false;

    if (vertexArray) {
        et->glDeleteVertexArrays(1, &vertexArray);
    }

    if (OpenGLExtensionManager::HasExtension("GL_ARB_vertex_array_object",environment)) {
        et->glGenVertexArrays(1, &vertexArray);
        hasVertexArrays = true;
    }

    vertShader = LoadShader(Utils::GetWaterShaderFileName(environment, true, false, false).c_str(), true, false);
    fragShader = LoadShader(Utils::GetWaterShaderFileName(environment, true, true, false).c_str(), false, false);
	vertShaderPatch = LoadShader(Utils::GetWaterShaderFileName(environment, true, false, true).c_str(), true, false);
	fragShaderPatch = LoadShader(Utils::GetWaterShaderFileName(environment, true, true, true).c_str(), false, false);

    userShader = LoadShader(Utils::GetUserShaderFileName().c_str(), false, true );
    userVertShader = LoadShader(Utils::GetUserVertShaderFileName().c_str(), true, true );

#ifdef __APPLE__
    vertCastShader = reinterpret_cast<uintptr_t>(vertShader);
    fragCastShader = reinterpret_cast<uintptr_t>(fragShader);
    vertCastShaderPatch = reinterpret_cast<uintptr_t>(vertShaderPatch);
    fragCastShaderPatch = reinterpret_cast<uintptr_t>(fragShaderPatch);
    userCastShader = reinterpret_cast<uintptr_t>(userShader);
    userVertCastShader = reinterpret_cast<uintptr_t>(userVertShader);
#else
    vertCastShader = vertShader;
    fragCastShader = fragShader;
    vertCastShaderPatch = vertShaderPatch;
    fragCastShaderPatch = fragShaderPatch;
    userCastShader = userShader;
    userVertCastShader = userVertShader;
#endif

    if (vertCastShader && fragCastShader) {
        programCast = et->glCreateProgram();

        if (programCast == 0) {
            printf("programCast is zero.\n");
        }

        et->glAttachShader(programCast, vertCastShader);
        et->glAttachShader(programCast, fragCastShader);

        if (userCastShader) et->glAttachShader(programCast, userCastShader);
        if (userVertCastShader) et->glAttachShader(programCast, userVertCastShader);

        TRITON_VECTOR(unsigned int)::const_iterator it;
        for (it = userShaders.begin(); it != userShaders.end(); it++) {
            et->glAttachShader(programCast, *it);
        }

        et->glLinkProgram(programCast);

        GLint linkStatus;
        et->glGetProgramiv(programCast, GL_LINK_STATUS, &linkStatus);
        if (linkStatus == GL_FALSE) {
            Utils::DebugMsg("Failed to link shader program for OpenGL.");
#ifdef __APPLE__
            PrintGLSLInfoLog(reinterpret_cast<void *>(programCast));
#else
            PrintGLSLInfoLog(programCast,environment);
#endif
            return false;
        }

    }

    // Query uniforms
    if ( programCast ) {
        invmvprojLoc = et->glGetUniformLocation(programCast, "trit_invModelviewProj");
        planeLoc = et->glGetUniformLocation(programCast, "trit_plane");
        radiiLoc = et->glGetUniformLocation(programCast, "trit_radii");
        invRadiiLoc = et->glGetUniformLocation(programCast, "trit_oneOverRadii");
        basisLoc = et->glGetUniformLocation(programCast, "trit_basis");
        invBasisLoc = et->glGetUniformLocation(programCast, "trit_invBasis");
        northPoleLoc = et->glGetUniformLocation(programCast, "trit_northPole");
        northLoc = et->glGetUniformLocation(programCast, "trit_north");
        eastLoc = et->glGetUniformLocation(programCast, "trit_east");
        cameraPosLoc = et->glGetUniformLocation(programCast, "trit_cameraPos");
        gridSizeLoc = et->glGetUniformLocation(programCast, "trit_gridSize");
        gridScaleLoc = et->glGetUniformLocation(programCast, "trit_gridScale");
        antiAliasingLoc = et->glGetUniformLocation(programCast, "trit_antiAliasing");
        fogColorLoc = et->glGetUniformLocation(programCast, "trit_fogColor");
        fogDensityLoc = et->glGetUniformLocation(programCast, "trit_fogDensity");
        fogDensityBelowLoc = et->glGetUniformLocation(programCast, "trit_fogDensityBelow");
        cubeMapMatrixLoc = et->glGetUniformLocation(programCast, "trit_cubeMapMatrix");
        planarReflectionMapMatrixLoc = et->glGetUniformLocation(programCast, "trit_planarReflectionMapMatrix");
        floorPlanePointLoc = et->glGetUniformLocation(programCast, "trit_floorPlanePoint");
        floorPlaneNormalLoc = et->glGetUniformLocation(programCast, "trit_floorPlaneNormal");
        planarHeightLoc = et->glGetUniformLocation(programCast, "trit_planarHeight");
        planarAdjustLoc = et->glGetUniformLocation(programCast, "trit_planarAdjust");
        referenceLocationLoc = et->glGetUniformLocation(programCast, "trit_referenceLocation");
        hasHeightMapLoc = et->glGetUniformLocation(programCast, "trit_hasHeightMap");
        hasUserHeightMapLoc = et->glGetUniformLocation(programCast, "trit_hasUserHeightMap");
        heightMapLoc = et->glGetUniformLocation(programCast, "trit_heightMap");
        heightMapRangeOffsetLoc = et->glGetUniformLocation(programCast, "trit_heightMapRangeOffset");
        heightMapMatrixLoc = et->glGetUniformLocation(programCast, "trit_heightMapMatrix");
        hasDepthMapLoc = et->glGetUniformLocation(programCast, "trit_hasDepthMap");
        depthMapLoc = et->glGetUniformLocation(programCast, "trit_depthMap");
        seaLevelLoc = et->glGetUniformLocation(programCast, "trit_seaLevel");
        planarReflectionBlendLoc = et->glGetUniformLocation(programCast, "trit_planarReflectionBlend");
        mvLoc = et->glGetUniformLocation(programCast, "trit_modelview");
        projLoc = et->glGetUniformLocation(programCast, "trit_projection");
        depthOffsetLoc = et->glGetUniformLocation(programCast, "trit_depthOffset");
        depthOnlyLoc = et->glGetUniformLocation(programCast, "trit_depthOnly");
        windDirLoc = et->glGetUniformLocation(programCast, "trit_windDir");
        underwaterLoc = et->glGetUniformLocation(programCast, "trit_underwater");
        doubleRefractionColorLoc = et->glGetUniformLocation(programCast, "trit_doubleRefractionColor");
        doubleRefractionIntensityLoc = et->glGetUniformLocation(programCast, "trit_doubleRefractionIntensity");
        oneOverGammaLoc = et->glGetUniformLocation(programCast, "trit_oneOverGamma");
        sunIntensityLoc = et->glGetUniformLocation(programCast, "trit_sunIntensity");
        invZoomLoc = et->glGetUniformLocation(programCast, "trit_invZoom");
        zNearFarLoc = et->glGetUniformLocation(programCast, "trit_zNearFar");
        reflectivityScaleLoc = et->glGetUniformLocation(programCast, "trit_reflectivityScale");

        if (enableBreakingWaves) {
            hasBreakingWaveMapLoc = et->glGetUniformLocation(programCast, "trit_hasBreakingWaveMap");
            breakingWaveMapLoc = et->glGetUniformLocation(programCast, "trit_breakingWaveMap");
            breakingWaveMapMatrixLoc = et->glGetUniformLocation(programCast, "trit_breakingWaveMapMatrix");
            kexpLoc = et->glGetUniformLocation(programCast, "trit_kexp");
            breakerWavelengthLoc = et->glGetUniformLocation(programCast, "trit_breakerWavelength");
            breakerWavelengthVarianceLoc = et->glGetUniformLocation(programCast, "trit_breakerWavelengthVariance");
            breakerDirectionLoc = et->glGetUniformLocation(programCast, "trit_breakerDirection");
            breakerAmplitudeLoc = et->glGetUniformLocation(programCast, "trit_breakerAmplitude");
            breakerPhaseConstantLoc = et->glGetUniformLocation(programCast, "trit_breakerPhaseConstant");
            surgeDepthLoc = et->glGetUniformLocation(programCast, "trit_surgeDepth");
            steepnessVarianceLoc = et->glGetUniformLocation(programCast, "trit_steepnessVariance");
            breakerDepthFalloffLoc = et->glGetUniformLocation(programCast, "trit_breakerDepthFalloff");

            enableBreakingWaves = kexpLoc != -1 && breakerWavelengthLoc != -1 && breakerWavelengthVarianceLoc != -1 &&
                                  breakerDirectionLoc != -1 && breakerAmplitudeLoc != -1 && breakerPhaseConstantLoc != -1
                                  && surgeDepthLoc != -1 && steepnessVarianceLoc != -1;
        } else {
            hasBreakingWaveMapLoc = breakingWaveMapLoc = breakingWaveMapMatrixLoc = kexpLoc = breakerWavelengthLoc =
                                        breakerWavelengthVarianceLoc = breakerDirectionLoc = breakerAmplitudeLoc = breakerPhaseConstantLoc =
                                                    surgeDepthLoc = steepnessVarianceLoc = breakerDepthFalloffLoc = -1;
        }

        if (environment && environment->GetRenderer() >= OPENGL_3_2) {
            vertexLoc = et->glGetAttribLocation(programCast, "vertex");
        }
    }

    if (vertCastShaderPatch && fragCastShaderPatch) {
        programCastPatch = et->glCreateProgram();

        et->glAttachShader(programCastPatch, vertCastShaderPatch);
        et->glAttachShader(programCastPatch, fragCastShaderPatch);

        if (userCastShader) et->glAttachShader(programCastPatch, userCastShader);
        if (userVertCastShader) et->glAttachShader(programCastPatch, userVertCastShader);

        TRITON_VECTOR(unsigned int)::const_iterator it;
        for (it = userShaders.begin(); it != userShaders.end(); it++) {
            et->glAttachShader(programCastPatch, *it);
        }

        et->glLinkProgram(programCastPatch);

        GLint linkStatus;
        et->glGetProgramiv(programCastPatch, GL_LINK_STATUS, &linkStatus);
        if (linkStatus == GL_FALSE) {
            Utils::DebugMsg("Failed to link shader program for OpenGL.");
#ifdef __APPLE__
            PrintGLSLInfoLog(reinterpret_cast<void *>(programCastPatch));
#else
            PrintGLSLInfoLog(programCastPatch,environment);
#endif
            return false;
        }

        invmvprojLocPatch = et->glGetUniformLocation(programCastPatch, "trit_invModelviewProj");
        planeLocPatch = et->glGetUniformLocation(programCastPatch, "trit_plane");
        radiiLocPatch = et->glGetUniformLocation(programCastPatch, "trit_radii");
        invRadiiLocPatch = et->glGetUniformLocation(programCastPatch, "trit_oneOverRadii");
        basisLocPatch = et->glGetUniformLocation(programCastPatch, "trit_basis");
        invBasisLocPatch = et->glGetUniformLocation(programCastPatch, "trit_invBasis");
        northLocPatch = et->glGetUniformLocation(programCastPatch, "trit_north");
        northPoleLocPatch = et->glGetUniformLocation(programCastPatch, "trit_northPole");
        eastLocPatch = et->glGetUniformLocation(programCastPatch, "trit_east");
        cameraPosLocPatch = et->glGetUniformLocation(programCastPatch, "trit_cameraPos");
        gridSizeLocPatch = et->glGetUniformLocation(programCastPatch, "trit_gridSize");
        gridScaleLocPatch = et->glGetUniformLocation(programCastPatch, "trit_gridScale");
        antiAliasingLocPatch = et->glGetUniformLocation(programCastPatch, "trit_antiAliasing");
        fogColorLocPatch = et->glGetUniformLocation(programCastPatch, "trit_fogColor");
        fogDensityLocPatch = et->glGetUniformLocation(programCastPatch, "trit_fogDensity");
        fogDensityBelowLocPatch = et->glGetUniformLocation(programCastPatch, "trit_fogDensityBelow");
        cubeMapMatrixLocPatch = et->glGetUniformLocation(programCastPatch, "trit_cubeMapMatrix");
        planarReflectionMapMatrixLocPatch = et->glGetUniformLocation(programCastPatch, "trit_planarReflectionMapMatrix");
        floorPlanePointLocPatch = et->glGetUniformLocation(programCastPatch, "trit_floorPlanePoint");
        floorPlaneNormalLocPatch = et->glGetUniformLocation(programCastPatch, "trit_floorPlaneNormal");
        planarHeightLocPatch = et->glGetUniformLocation(programCastPatch, "trit_planarHeight");
        modelMatrixLocPatch = et->glGetUniformLocation(programCastPatch, "trit_modelMatrix");
        planarAdjustLocPatch = et->glGetUniformLocation(programCastPatch, "trit_planarAdjust");
        referenceLocationLocPatch = et->glGetUniformLocation(programCastPatch, "trit_referenceLocation");
        hasHeightMapLocPatch = et->glGetUniformLocation(programCastPatch, "trit_hasHeightMap");
        hasUserHeightMapLocPatch = et->glGetUniformLocation(programCastPatch, "trit_hasUserHeightMap");
        heightMapLocPatch = et->glGetUniformLocation(programCastPatch, "trit_heightMap");
        heightMapRangeOffsetLocPatch = et->glGetUniformLocation(programCastPatch, "trit_heightMapRangeOffset");
        heightMapMatrixLocPatch = et->glGetUniformLocation(programCastPatch, "trit_heightMapMatrix");
        hasDepthMapLocPatch = et->glGetUniformLocation(programCastPatch, "trit_hasDepthMap");
        depthMapLocPatch = et->glGetUniformLocation(programCastPatch, "trit_depthMap");
        seaLevelLocPatch = et->glGetUniformLocation(programCastPatch, "trit_seaLevel");
        planarReflectionBlendLocPatch = et->glGetUniformLocation(programCastPatch, "trit_planarReflectionBlend");
        mvLocPatch = et->glGetUniformLocation(programCastPatch, "trit_modelview");
        projLocPatch = et->glGetUniformLocation(programCastPatch, "trit_projection");
        depthOffsetLocPatch = et->glGetUniformLocation(programCastPatch, "trit_depthOffset");
        depthOnlyLocPatch = et->glGetUniformLocation(programCastPatch, "trit_depthOnly");
        windDirLocPatch = et->glGetUniformLocation(programCastPatch, "trit_windDir");
        underwaterLocPatch = et->glGetUniformLocation(programCastPatch, "trit_underwater");
        doubleRefractionColorLocPatch = et->glGetUniformLocation(programCastPatch, "trit_doubleRefractionColor");
        doubleRefractionIntensityLocPatch = et->glGetUniformLocation(programCastPatch, "trit_doubleRefractionIntensity");
        oneOverGammaLocPatch = et->glGetUniformLocation(programCastPatch, "trit_oneOverGamma");
        sunIntensityLocPatch = et->glGetUniformLocation(programCastPatch, "trit_sunIntensity");
        invZoomLocPatch = et->glGetUniformLocation(programCastPatch, "trit_invZoom");
        zNearFarLocPatch = et->glGetUniformLocation(programCastPatch, "trit_zNearFar");
        reflectivityScaleLocPatch = et->glGetUniformLocation(programCastPatch, "trit_reflectivityScale");
		camHightLocPatch = et->glGetUniformLocation(programCastPatch, "trit_camHight");

        if (enableBreakingWaves) {
            hasBreakingWaveMapLocPatch = et->glGetUniformLocation(programCastPatch, "trit_hasBreakingWaveMap");
            breakingWaveMapLocPatch = et->glGetUniformLocation(programCastPatch, "trit_breakingWaveMap");
            breakingWaveMapMatrixLocPatch = et->glGetUniformLocation(programCastPatch, "trit_breakingWaveMapMatrix");
            kexpLocPatch = et->glGetUniformLocation(programCastPatch, "trit_kexp");
            breakerWavelengthLocPatch = et->glGetUniformLocation(programCastPatch, "trit_breakerWavelength");
            breakerWavelengthVarianceLocPatch = et->glGetUniformLocation(programCastPatch, "trit_breakerWavelengthVariance");
            breakerDirectionLocPatch = et->glGetUniformLocation(programCastPatch, "trit_breakerDirection");
            breakerAmplitudeLocPatch = et->glGetUniformLocation(programCastPatch, "trit_breakerAmplitude");
            breakerPhaseConstantLocPatch = et->glGetUniformLocation(programCastPatch, "trit_breakerPhaseConstant");
            surgeDepthLocPatch = et->glGetUniformLocation(programCastPatch, "trit_surgeDepth");
            steepnessVarianceLocPatch = et->glGetUniformLocation(programCastPatch, "trit_steepnessVariance");
            breakerDepthFalloffLocPatch = et->glGetUniformLocation(programCastPatch, "trit_breakerDepthFalloff");

            enableBreakingWaves = kexpLocPatch != -1 && breakerWavelengthLocPatch != -1 && breakerWavelengthVarianceLocPatch != -1 &&
                                  breakerDirectionLocPatch != -1 && breakerAmplitudeLocPatch != -1 && breakerPhaseConstantLocPatch != -1
                                  && surgeDepthLocPatch != -1 && steepnessVarianceLocPatch != -1;
        } else {
            hasBreakingWaveMapLocPatch = breakingWaveMapLocPatch = breakingWaveMapMatrixLocPatch = kexpLocPatch =
                                             breakerWavelengthLocPatch = breakerWavelengthVarianceLocPatch = breakerDirectionLocPatch =
                                                     breakerAmplitudeLocPatch = breakerPhaseConstantLocPatch = surgeDepthLocPatch =
                                                             steepnessVarianceLocPatch = breakerDepthFalloffLocPatch = -1;
        }

        if (environment && environment->GetRenderer() >= OPENGL_3_2) {
            vertexLocPatch = et->glGetAttribLocation(programCastPatch, "vertex");
        }
    }

    return Utils::PrintGLErrors(__FILE__, __LINE__);
}

bool ProjectedGridOpenGL::PrepShaders(bool aboveWater)
{
    if (!environment) return false;

	osg::GLExtensions* et = environment->GetExtension();
	if(!et)
		return false;

    Utils::ClearGLErrors();

    et->glUseProgram(programCast);

    OpenGLCachedUniforms::SetUniformFloat(environment, programCast, antiAliasingLoc, (float)antiAliasing);

    Matrix4 mv(environment->GetCameraMatrix());

    Vector3 camPos(environment->GetCameraPosition());
    OpenGLCachedUniforms::SetUniformVec3(environment, programCast, cameraPosLoc, (float)camPos.x, (float)camPos.y, (float)camPos.z);

    mv.elem[3][0] = mv.elem[3][1] = mv.elem[3][2] = 0;

    Matrix4 proj(environment->GetProjectionMatrix());

    Matrix4 mvproj =  mv * proj;

    Matrix4 invMvProj = mvproj.InverseCramers();
    OpenGLCachedUniforms::SetUniformMat4(environment, programCast, invmvprojLoc, invMvProj.ToFloatArray());
    OpenGLCachedUniforms::SetUniformMat4(environment, programCast, mvLoc, mv.ToFloatArray());
    OpenGLCachedUniforms::SetUniformMat4(environment, programCast, projLoc, proj.ToFloatArray());

    OpenGLCachedUniforms::SetUniformFloat(environment, programCast, depthOffsetLoc, depthOffset);

    OpenGLCachedUniforms::SetUniformVec2(environment, programCast, zNearFarLoc, (float)nearZ, (float)farZ);

    OpenGLCachedUniforms::SetUniformMat3(environment, programCast, cubeMapMatrixLoc, environment->GetEnvironmentMapMatrix().ToFloatArray());
    OpenGLCachedUniforms::SetUniformMat3(environment, programCast, planarReflectionMapMatrixLoc, environment->GetPlanarReflectionMapMatrix().ToFloatArray());

    Matrix4 scale = ComputeGridMatrix(environment->GetMaximumWaveHeight());

    OpenGLCachedUniforms::SetUniformMat4(environment, programCast, gridScaleLoc, scale.ToFloatArray());

    OpenGLCachedUniforms::SetUniformMat3(environment, programCast, basisLoc, basis.ToFloatArray());
    OpenGLCachedUniforms::SetUniformMat3(environment, programCast, invBasisLoc, invBasis.ToFloatArray());

    OpenGLCachedUniforms::SetUniformFloat(environment, programCast, gridSizeLoc, (float)resolution);

    OpenGLCachedUniforms::SetUniformInt(environment, programCast, depthOnlyLoc, 0);

    double windSpeed = 0, windDirection = 0, fetchLength = 0;
    environment->GetWind(environment->GetCameraPosition(), windSpeed, windDirection, fetchLength);
    Vector3 windDir(-sin(windDirection), -cos(windDirection), 0.0f);

    OpenGLCachedUniforms::SetUniformVec3(environment, programCast, windDirLoc, (float)windDir.x, (float)windDir.y, (float)windDir.z);

    OpenGLCachedUniforms::SetUniformVec3(environment, programCast, doubleRefractionColorLoc, (float)doubleRefractionColor.x,
                                         (float)doubleRefractionColor.y, (float)doubleRefractionColor.z);
    OpenGLCachedUniforms::SetUniformFloat(environment, programCast, doubleRefractionIntensityLoc, doubleRefractionIntensity);

    OpenGLCachedUniforms::SetUniformFloat(environment, programCast, oneOverGammaLoc, 1.0f / gamma);
    OpenGLCachedUniforms::SetUniformFloat(environment, programCast, sunIntensityLoc, environment->GetSunIntensity());
    OpenGLCachedUniforms::SetUniformFloat(environment, programCast, invZoomLoc, 1.0f / environment->GetZoomLevel());

    OpenGLCachedUniforms::SetUniformFloat(environment, programCast, reflectivityScaleLoc, reflectionScale);

    if (environment->GetHeightMap()) {
        glActiveTexture(GL_TEXTURE5);
        glBindTexture(GL_TEXTURE_2D, (GLuint)environment->GetHeightMap());
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        OpenGLCachedUniforms::SetUniformInt(environment, programCast, heightMapLoc, 5);
        OpenGLCachedUniforms::SetUniformVec2(environment, programCast, heightMapRangeOffsetLoc,
                                             environment->GetHeightMapRange(), environment->GetHeightMapOffset() );
        OpenGLCachedUniforms::SetUniformInt(environment, programCast, hasHeightMapLoc, GL_TRUE);
        OpenGLCachedUniforms::SetUniformMat4(environment, programCast, heightMapMatrixLoc, environment->GetHeightMapMatrix().ToFloatArray());
    } else {
        OpenGLCachedUniforms::SetUniformInt(environment, programCast, hasHeightMapLoc, GL_FALSE);
        OpenGLCachedUniforms::SetUniformInt(environment, programCast, heightMapLoc, 5);
        glActiveTexture(GL_TEXTURE5);
        glBindTexture(GL_TEXTURE_2D, dummyHeightMapID);
        Matrix4 identity;
        OpenGLCachedUniforms::SetUniformMat4(environment, programCast, heightMapMatrixLoc, identity.ToFloatArray());
    }

    if ( environment->GetUserHeightCB() ) {
        OpenGLCachedUniforms::SetUniformInt(environment, programCast, hasUserHeightMapLoc, GL_TRUE);
        OpenGLCachedUniforms::SetUniformVec2(environment, programCast, heightMapRangeOffsetLoc,
                                             environment->GetHeightMapRange(), environment->GetHeightMapOffset() );
    } else {
        OpenGLCachedUniforms::SetUniformInt(environment, programCast, hasUserHeightMapLoc, GL_FALSE);
    }

    if (environment->GetDepthMap()) {
        glActiveTexture(GL_TEXTURE11);
        glBindTexture(GL_TEXTURE_2D, (GLuint)environment->GetDepthMap());
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        OpenGLCachedUniforms::SetUniformInt(environment, programCast, depthMapLoc, 11);
        OpenGLCachedUniforms::SetUniformInt(environment, programCast, hasDepthMapLoc, GL_TRUE);
    } else {
        OpenGLCachedUniforms::SetUniformInt(environment, programCast, hasDepthMapLoc, GL_FALSE);
        OpenGLCachedUniforms::SetUniformInt(environment, programCast, depthMapLoc, 11);
        glActiveTexture(GL_TEXTURE11);
        glBindTexture(GL_TEXTURE_2D, dummyHeightMapID);
    }

    if (enableBreakingWaves && environment->GetBreakingWaveMap()) {
        glActiveTexture(GL_TEXTURE10);
        glBindTexture(GL_TEXTURE_2D, (GLuint)environment->GetBreakingWaveMap());
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        OpenGLCachedUniforms::SetUniformInt(environment, programCast, breakingWaveMapLoc, 10);
        OpenGLCachedUniforms::SetUniformInt(environment, programCast, hasBreakingWaveMapLoc, GL_TRUE);
        OpenGLCachedUniforms::SetUniformMat4(environment, programCast, breakingWaveMapMatrixLoc, environment->GetBreakingWaveMapMatrix().ToFloatArray());
    } else {
        OpenGLCachedUniforms::SetUniformInt(environment, programCast, hasBreakingWaveMapLoc, GL_FALSE);
        OpenGLCachedUniforms::SetUniformInt(environment, programCast, breakingWaveMapLoc, 10);
        glActiveTexture(GL_TEXTURE10);
        glBindTexture(GL_TEXTURE_2D, dummyHeightMapID);
        Matrix4 identity;
        OpenGLCachedUniforms::SetUniformMat4(environment, programCast, breakingWaveMapMatrixLoc, identity.ToFloatArray());
    }

    Vector3 floorPlanePoint, floorNormal;
    GetFloorPlane(floorPlanePoint, floorNormal);
    OpenGLCachedUniforms::SetUniformVec3(environment, programCast, floorPlanePointLoc, (float)floorPlanePoint.x,
                                         (float)floorPlanePoint.y, (float)floorPlanePoint.z);
    OpenGLCachedUniforms::SetUniformVec3(environment, programCast, floorPlaneNormalLoc, (float)floorNormal.x,
                                         (float)floorNormal.y, (float)floorNormal.z);

    Vector3 fogColor;
    double visibility, fogDensity;

    if (aboveWater) {
        environment->GetAboveWaterVisibility(visibility, fogColor);
        OpenGLCachedUniforms::SetUniformInt(environment, programCast, underwaterLoc, 0);
    } else {
        environment->GetBelowWaterVisibility(visibility, fogColor);
        OpenGLCachedUniforms::SetUniformInt(environment, programCast, underwaterLoc, 1);
    }
    fogDensity = 3.912 / visibility;

    OpenGLCachedUniforms::SetUniformFloat(environment, programCast, seaLevelLoc, (float)environment->GetSeaLevel());

    OpenGLCachedUniforms::SetUniformVec3(environment, programCast, fogColorLoc, (GLfloat)fogColor.x, (GLfloat)fogColor.y, (GLfloat)fogColor.z);
    OpenGLCachedUniforms::SetUniformFloat(environment, programCast, fogDensityLoc, (GLfloat)fogDensity);

    visibility = environment->GetWaveBlendDepth();
    OpenGLCachedUniforms::SetUniformFloat(environment, programCast, fogDensityBelowLoc, (GLfloat)(3.912 / visibility));

    OpenGLCachedUniforms::SetUniformFloat(environment, programCast, planarHeightLoc, planarHeight);

    OpenGLCachedUniforms::SetUniformFloat(environment, programCast, planarReflectionBlendLoc, planarReflectionBlend);

    if (enableBreakingWaves) {
        const BreakingWavesParameters& params = environment->GetBreakingWavesParameters();
        OpenGLCachedUniforms::SetUniformFloat(environment, programCast, kexpLoc, params.GetExpK());
        OpenGLCachedUniforms::SetUniformFloat(environment, programCast, breakerWavelengthLoc, params.GetWavelength());
        OpenGLCachedUniforms::SetUniformFloat(environment, programCast, breakerWavelengthVarianceLoc, params.GetWavelengthVariance());
        const Vector3& waveDirection = params.GetWaveDirection();
        OpenGLCachedUniforms::SetUniformVec3(environment, programCast, breakerDirectionLoc, (float)waveDirection.x, (float)waveDirection.y, (float)waveDirection.z);
        OpenGLCachedUniforms::SetUniformFloat(environment, programCast, breakerAmplitudeLoc, params.GetAmplitude());
        OpenGLCachedUniforms::SetUniformFloat(environment, programCast, breakerPhaseConstantLoc, params.GetPhaseConstant());
        OpenGLCachedUniforms::SetUniformFloat(environment, programCast, surgeDepthLoc, params.GetSurgeDepth());
        OpenGLCachedUniforms::SetUniformFloat(environment, programCast, steepnessVarianceLoc, params.GetSteepnessVariance());
        OpenGLCachedUniforms::SetUniformFloat(environment, programCast, breakerDepthFalloffLoc, params.GetDepthFalloff());
    }

    switch (environment->GetCoordinateSystem()) {
    case FLAT_ZUP: {
        if (planeLoc != -1) {
            float D = (float)camPos.z - (float)environment->GetSeaLevel();
            OpenGLCachedUniforms::SetUniformVec4(environment, programCast, planeLoc, 0, 0, 1, D);
        }
    }
    break;

    case FLAT_YUP: {
        if (planeLoc != -1) {
            float D = (float)camPos.y - (float)environment->GetSeaLevel();
            OpenGLCachedUniforms::SetUniformVec4(environment, programCast, planeLoc, 0, 1, 0, D);
        }
    }
    break;

    case WGS84_ZUP:
    case WGS84_YUP:
    case SPHERICAL_ZUP:
    case SPHERICAL_YUP: {
        double msl = environment->GetSeaLevel();
        Vector3 radii = earthRadii + Vector3(msl, msl, msl);
        Vector3 northPole;
        ComputeDirections(camPos, northPole);
        OpenGLCachedUniforms::SetUniformVec3(environment, programCast, northPoleLoc, (float)north.x, (float)north.y, (float)north.z);
        OpenGLCachedUniforms::SetUniformVec3(environment, programCast, northLoc, (float)localNorth.x, (float)localNorth.y, (float)localNorth.z);
        OpenGLCachedUniforms::SetUniformVec3(environment, programCast, eastLoc, (float)localEast.x, (float)localEast.y, (float)localEast.z);
        OpenGLCachedUniforms::SetUniformVec3(environment, programCast, radiiLoc, (float)radii.x, (float)radii.y, (float)radii.z);
        OpenGLCachedUniforms::SetUniformVec3(environment, programCast, invRadiiLoc, 1.0f / (float)radii.x, 1.0f / (float)radii.y,
                                             1.0f / (float)radii.z);
        if (planeLoc != -1) {
            Vector3 camPos(environment->GetCameraPosition());
            Vector3 up = camPos;
            up.Normalize();
            Vector3 surfacePos;
            IntersectEllipsoid(camPos, up * -1.0, surfacePos, 0);

            double A = surfacePos.x / (radii.x * radii.x);
            double B = surfacePos.y / (radii.y * radii.y);
            double C = surfacePos.z / (radii.z * radii.z);
            double distance = Vector3(A, B, C).Length();
            A /= distance;
            B /= distance;
            C /= distance;

            double D = (camPos - surfacePos).Length();
            if (D <= 0.1 && D >= -0.1) {
                if (aboveWater) {
                    D = -0.1;
                } else {
                    D = 0.1;
                }
            }
            if (camPos.SquaredLength() < surfacePos.SquaredLength()) D *= -1.0;
            OpenGLCachedUniforms::SetUniformVec4(environment, programCast, planeLoc, (float)A, (float)B, (float)C, (float)D);

            if (planarAdjustLoc != -1) {
                OpenGLCachedUniforms::SetUniformFloat(environment, programCast, planarAdjustLoc, planarAdjust);
            }
            if (referenceLocationLoc != -1) {
#define BLOCKSIZE 100000.0
#define OFFSET     50000.0
                Vector3 refPt = camPos + Vector3(OFFSET, OFFSET, OFFSET);
                Vector3 pt = Vector3(fmod(refPt.x, BLOCKSIZE), fmod(refPt.y, BLOCKSIZE), fmod(refPt.z, BLOCKSIZE));
                OpenGLCachedUniforms::SetUniformVec3(environment, programCast, referenceLocationLoc, (float)pt.x, (float)pt.y, (float)pt.z);
            }
        }


    }
    break;
    }

    return Utils::PrintGLErrors(__FILE__, __LINE__);
}

bool ProjectedGridOpenGL::SetPatchShader(double time, int vertexStride, int vertexPositionOffset, bool doublePrecisionVertices, const Matrix4& modelMatrix, bool aboveWater, bool depthPass)
{
    if (!environment) return false;

	osg::GLExtensions* et = environment->GetExtension();
	if(!et)
		return false;

    if (depthPass && !generateDepthTexture) return false;

    et->glUseProgram(programCastPatch);

    if (waterModel && lightingModel && wakeManager && wakeManager->PrepPatchShader(environment) &&
            waterModel->PrepPatchShader(time) && lightingModel->PrepPatchShader()) {

        if (environment && environment->GetRenderer() >= OPENGL_3_2) {
            //glVertexAttribPointer(vertexLocPatch, 3, doublePrecisionVertices ? GL_DOUBLE : GL_FLOAT, GL_FALSE, vertexStride, (GLvoid *)vertexPositionOffset);
            // Compile fix for GCC 4.8.1
            et->glVertexAttribPointer(vertexLocPatch, 3, doublePrecisionVertices ? GL_DOUBLE : GL_FLOAT, GL_FALSE, vertexStride, BUFFER_OFFSET(vertexPositionOffset));
            et->glEnableVertexAttribArray(vertexLocPatch);
        }

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        Utils::ClearGLErrors();

        //glUseProgram(programCastPatch);

        OpenGLCachedUniforms::SetUniformFloat(environment, programCastPatch, antiAliasingLocPatch, (float)antiAliasing);

        Matrix4 mv(environment->GetCameraMatrix());

        Vector3 camPos(environment->GetCameraPosition());
        OpenGLCachedUniforms::SetUniformVec3(environment, programCastPatch, cameraPosLocPatch, (float)camPos.x, (float)camPos.y, (float)camPos.z);

        if (environment->IsGeocentric()) {
            mv.elem[3][0] = mv.elem[3][1] = mv.elem[3][2] = 0;
        }

        Matrix4 proj(environment->GetProjectionMatrix());

        Matrix4 mvproj =  mv * proj;

        Matrix4 invMvProj = mvproj.InverseCramers();

        OpenGLCachedUniforms::SetUniformMat4(environment, programCastPatch, invmvprojLocPatch, invMvProj.ToFloatArray());
        OpenGLCachedUniforms::SetUniformMat4(environment, programCastPatch, modelMatrixLocPatch, modelMatrix.ToFloatArray());
        OpenGLCachedUniforms::SetUniformMat4(environment, programCastPatch, mvLocPatch, mv.ToFloatArray());
        OpenGLCachedUniforms::SetUniformMat4(environment, programCastPatch, projLocPatch, proj.ToFloatArray());
        OpenGLCachedUniforms::SetUniformFloat(environment, programCastPatch, depthOffsetLocPatch, depthOffset);

        OpenGLCachedUniforms::SetUniformVec2(environment, programCastPatch, zNearFarLocPatch, (float)nearZ, (float)farZ);

        OpenGLCachedUniforms::SetUniformMat3(environment, programCastPatch, cubeMapMatrixLocPatch, environment->GetEnvironmentMapMatrix().ToFloatArray());

        OpenGLCachedUniforms::SetUniformMat3(environment, programCastPatch, planarReflectionMapMatrixLocPatch, environment->GetPlanarReflectionMapMatrix().ToFloatArray());

        Matrix4 scale = ComputeGridMatrix(environment->GetMaximumWaveHeight());

        OpenGLCachedUniforms::SetUniformMat4(environment, programCastPatch, gridScaleLocPatch, scale.ToFloatArray());

        OpenGLCachedUniforms::SetUniformMat3(environment, programCastPatch, basisLocPatch, basis.ToFloatArray());
        OpenGLCachedUniforms::SetUniformMat3(environment, programCastPatch, invBasisLocPatch, invBasis.ToFloatArray());

        OpenGLCachedUniforms::SetUniformFloat(environment, programCastPatch, gridSizeLocPatch, (float)resolution);

        OpenGLCachedUniforms::SetUniformInt(environment, programCastPatch, depthOnlyLocPatch, 0);

        OpenGLCachedUniforms::SetUniformFloat(environment, programCastPatch, reflectivityScaleLocPatch, reflectionScale);

		OpenGLCachedUniforms::SetUniformFloat(environment, programCastPatch, camHightLocPatch, environment->GetCamHeight());

        Vector3 floorPlanePoint, floorNormal;
        GetFloorPlane(floorPlanePoint, floorNormal);
        OpenGLCachedUniforms::SetUniformVec3(environment, programCastPatch, floorPlanePointLocPatch, (float)floorPlanePoint.x,
                                             (float)floorPlanePoint.y, (float)floorPlanePoint.z);
        OpenGLCachedUniforms::SetUniformVec3(environment, programCastPatch, floorPlaneNormalLocPatch, (float)floorNormal.x,
                                             (float)floorNormal.y, (float)floorNormal.z);

        Vector3 fogColor;
        double visibility, fogDensity;

        if (aboveWater) {
            environment->GetAboveWaterVisibility(visibility, fogColor);
            OpenGLCachedUniforms::SetUniformInt(environment, programCastPatch, underwaterLocPatch, 0);
        } else {
            environment->GetBelowWaterVisibility(visibility, fogColor);
            OpenGLCachedUniforms::SetUniformInt(environment, programCastPatch, underwaterLocPatch, 1);
        }

        OpenGLCachedUniforms::SetUniformFloat(environment, programCastPatch, seaLevelLocPatch, (float)environment->GetSeaLevel());

        fogDensity = 3.912 / visibility;

        OpenGLCachedUniforms::SetUniformVec3(environment, programCastPatch, fogColorLocPatch, (GLfloat)fogColor.x, (GLfloat)fogColor.y, (GLfloat)fogColor.z);
        OpenGLCachedUniforms::SetUniformFloat(environment, programCastPatch, fogDensityLocPatch, (GLfloat)fogDensity);

        visibility = environment->GetWaveBlendDepth();
        OpenGLCachedUniforms::SetUniformFloat(environment, programCastPatch, fogDensityBelowLocPatch, (GLfloat)(3.912 / visibility));

        OpenGLCachedUniforms::SetUniformFloat(environment, programCastPatch, planarHeightLocPatch, planarHeight);

        OpenGLCachedUniforms::SetUniformFloat(environment, programCastPatch, planarReflectionBlendLocPatch, planarReflectionBlend);

        OpenGLCachedUniforms::SetUniformVec3(environment, programCastPatch, doubleRefractionColorLocPatch, (float)doubleRefractionColor.x,
                                             (float)doubleRefractionColor.y, (float)doubleRefractionColor.z);

		double curDoubleRefractionIntensity = doubleRefractionIntensity;
		if(aboveWater)
			curDoubleRefractionIntensity *= 0.3;
        OpenGLCachedUniforms::SetUniformFloat(environment, programCastPatch, doubleRefractionIntensityLocPatch, curDoubleRefractionIntensity);

        OpenGLCachedUniforms::SetUniformFloat(environment, programCastPatch, oneOverGammaLocPatch, 1.0f / gamma);
        OpenGLCachedUniforms::SetUniformFloat(environment, programCastPatch, sunIntensityLocPatch, environment->GetSunIntensity());
        OpenGLCachedUniforms::SetUniformFloat(environment, programCastPatch, invZoomLocPatch, 1.0f / environment->GetZoomLevel());

        double windSpeed = 0, windDirection = 0, fetchLength = 0;
        environment->GetWind(environment->GetCameraPosition(), windSpeed, windDirection, fetchLength);
        Vector3 windDir(-sin(windDirection), -cos(windDirection), 0.0f);
        OpenGLCachedUniforms::SetUniformVec3(environment, programCastPatch, windDirLocPatch, (float)windDir.x, (float)windDir.y, (float)windDir.z);

        if (environment->GetHeightMap()) {
            glActiveTexture(GL_TEXTURE5);
            glBindTexture(GL_TEXTURE_2D, (GLuint)environment->GetHeightMap());
            OpenGLCachedUniforms::SetUniformInt(environment, programCastPatch, heightMapLocPatch, 5);
            OpenGLCachedUniforms::SetUniformVec2(environment, programCastPatch, heightMapRangeOffsetLocPatch,
                                                 environment->GetHeightMapRange(), environment->GetHeightMapOffset() );
            OpenGLCachedUniforms::SetUniformInt(environment, programCastPatch, hasHeightMapLocPatch, GL_TRUE);
            OpenGLCachedUniforms::SetUniformMat4(environment, programCastPatch, heightMapMatrixLocPatch, environment->GetHeightMapMatrix().ToFloatArray());
        } else {
            OpenGLCachedUniforms::SetUniformInt(environment, programCastPatch, heightMapLocPatch, 5);
            OpenGLCachedUniforms::SetUniformInt(environment, programCastPatch, hasHeightMapLocPatch, GL_FALSE);
            glActiveTexture(GL_TEXTURE5);
            glBindTexture(GL_TEXTURE_2D, dummyHeightMapID);
            Matrix4 identity;
            OpenGLCachedUniforms::SetUniformMat4(environment, programCastPatch, heightMapMatrixLocPatch, identity.ToFloatArray());
        }

        if ( environment->GetUserHeightCB() ) {
            OpenGLCachedUniforms::SetUniformInt(environment, programCastPatch, hasUserHeightMapLocPatch, GL_TRUE);
            OpenGLCachedUniforms::SetUniformVec2(environment, programCastPatch, heightMapRangeOffsetLocPatch,
                                                 environment->GetHeightMapRange(), environment->GetHeightMapOffset() );
        } else {
            OpenGLCachedUniforms::SetUniformInt(environment, programCastPatch, hasUserHeightMapLocPatch, GL_FALSE);
        }

        if (environment->GetDepthMap()) {
            glActiveTexture(GL_TEXTURE11);
            glBindTexture(GL_TEXTURE_2D, (GLuint)environment->GetDepthMap());
            OpenGLCachedUniforms::SetUniformInt(environment, programCastPatch, depthMapLocPatch, 11);
            OpenGLCachedUniforms::SetUniformInt(environment, programCastPatch, hasDepthMapLocPatch, GL_TRUE);
        } else {
            OpenGLCachedUniforms::SetUniformInt(environment, programCastPatch, depthMapLocPatch, 11);
            OpenGLCachedUniforms::SetUniformInt(environment, programCastPatch, hasDepthMapLocPatch, GL_FALSE);
            glActiveTexture(GL_TEXTURE11);
            glBindTexture(GL_TEXTURE_2D, dummyHeightMapID);
        }

        if (enableBreakingWaves && environment->GetBreakingWaveMap()) {
            glActiveTexture(GL_TEXTURE10);
            glBindTexture(GL_TEXTURE_2D, (GLuint)environment->GetBreakingWaveMap());
            OpenGLCachedUniforms::SetUniformInt(environment, programCastPatch, breakingWaveMapLocPatch, 10);
            OpenGLCachedUniforms::SetUniformInt(environment, programCastPatch, hasBreakingWaveMapLocPatch, GL_TRUE);
            OpenGLCachedUniforms::SetUniformMat4(environment, programCastPatch, breakingWaveMapMatrixLocPatch, environment->GetBreakingWaveMapMatrix().ToFloatArray());
        } else {
            OpenGLCachedUniforms::SetUniformInt(environment, programCastPatch, breakingWaveMapLocPatch, 10);
            OpenGLCachedUniforms::SetUniformInt(environment, programCastPatch, hasBreakingWaveMapLocPatch, GL_FALSE);
            glActiveTexture(GL_TEXTURE10);
            glBindTexture(GL_TEXTURE_2D, dummyHeightMapID);
            Matrix4 identity;
            OpenGLCachedUniforms::SetUniformMat4(environment, programCastPatch, breakingWaveMapMatrixLocPatch, identity.ToFloatArray());
        }

        if (enableBreakingWaves) {
            const BreakingWavesParameters& params = environment->GetBreakingWavesParameters();
            OpenGLCachedUniforms::SetUniformFloat(environment, programCastPatch, kexpLocPatch, params.GetExpK());
            OpenGLCachedUniforms::SetUniformFloat(environment, programCastPatch, breakerWavelengthLocPatch, params.GetWavelength());
            OpenGLCachedUniforms::SetUniformFloat(environment, programCastPatch, breakerWavelengthVarianceLocPatch, params.GetWavelengthVariance());
            const Vector3& waveDirection = params.GetWaveDirection();
            OpenGLCachedUniforms::SetUniformVec3(environment, programCastPatch, breakerDirectionLocPatch, (float)waveDirection.x, (float)waveDirection.y, (float)waveDirection.z);
            OpenGLCachedUniforms::SetUniformFloat(environment, programCastPatch, breakerAmplitudeLocPatch, params.GetAmplitude());
            OpenGLCachedUniforms::SetUniformFloat(environment, programCastPatch, breakerPhaseConstantLocPatch, params.GetPhaseConstant());
            OpenGLCachedUniforms::SetUniformFloat(environment, programCastPatch, surgeDepthLocPatch, params.GetSurgeDepth());
            OpenGLCachedUniforms::SetUniformFloat(environment, programCastPatch, steepnessVarianceLocPatch, params.GetSteepnessVariance());
            OpenGLCachedUniforms::SetUniformFloat(environment, programCastPatch, breakerDepthFalloffLocPatch, params.GetDepthFalloff());
        }

        switch (environment->GetCoordinateSystem()) {
        case FLAT_ZUP: {
            if (planeLocPatch != -1) {
                float D = (float)environment->GetSeaLevel();
                if (D == camPos.z) D += 0.1f;
                OpenGLCachedUniforms::SetUniformVec4(environment, programCastPatch, planeLocPatch, 0, 0, 1, -D);
            }
        }
        break;

        case FLAT_YUP: {
            if (planeLocPatch != -1) {
                float D = (float)environment->GetSeaLevel();
                if (D == camPos.y) D += 0.1f;
                OpenGLCachedUniforms::SetUniformVec4(environment, programCastPatch, planeLocPatch, 0, 1, 0, -D);
            }
        }
        break;

        case WGS84_ZUP:
        case WGS84_YUP:
        case SPHERICAL_ZUP:
        case SPHERICAL_YUP: {
            double msl = environment->GetSeaLevel();
            Vector3 radii = earthRadii + Vector3(msl, msl, msl);
            Vector3 northPole;
            ComputeDirections(camPos, northPole);
            OpenGLCachedUniforms::SetUniformVec3(environment, programCastPatch, northPoleLocPatch, (float)northPole.x, (float)northPole.y, (float)northPole.z);
            OpenGLCachedUniforms::SetUniformVec3(environment, programCastPatch, northLocPatch, (float)localNorth.x, (float)localNorth.y, (float)localNorth.z);
            OpenGLCachedUniforms::SetUniformVec3(environment, programCastPatch, eastLocPatch, (float)localEast.x, (float)localEast.y, (float)localEast.z);
            OpenGLCachedUniforms::SetUniformVec3(environment, programCastPatch, radiiLocPatch, (float)radii.x, (float)radii.y, (float)radii.z);
            OpenGLCachedUniforms::SetUniformVec3(environment, programCastPatch, invRadiiLocPatch, 1.0f / (float)radii.x, 1.0f / (float)radii.y,
                                                 1.0f / (float)radii.z);
            if (planeLocPatch != -1) {
                Vector3 camPos(environment->GetCameraPosition());
                Vector3 up = camPos;
                up.Normalize();
                Vector3 surfacePos;
                IntersectEllipsoid(camPos, up * -1.0, surfacePos, 0);

                double A = surfacePos.x / (radii.x * radii.x);
                double B = surfacePos.y / (radii.y * radii.y);
                double C = surfacePos.z / (radii.z * radii.z);
                double distance = Vector3(A, B, C).Length();
                A /= distance;
                B /= distance;
                C /= distance;
                double D = (camPos - surfacePos).Length();
                if (D <= 0.1 && D >= -0.1) {
                    if (aboveWater) {
                        D = -0.1;
                    } else {
                        D = 0.1;
                    }
                }
                if (camPos.SquaredLength() < surfacePos.SquaredLength()) D *= -1.0;
                OpenGLCachedUniforms::SetUniformVec4(environment, programCastPatch, planeLoc, (float)A, (float)B, (float)C, (float)D);
            }
            if (planarAdjustLocPatch != -1) {
                OpenGLCachedUniforms::SetUniformFloat(environment, programCastPatch, planarAdjustLocPatch, planarAdjust);
            }
            if (referenceLocationLocPatch != -1) {
#define BLOCKSIZE 100000.0
#define OFFSET     50000.0
                Vector3 refPt = camPos + Vector3(OFFSET, OFFSET, OFFSET);
                Vector3 pt = Vector3(fmod(refPt.x, BLOCKSIZE), fmod(refPt.y, BLOCKSIZE), fmod(refPt.z, BLOCKSIZE));
                OpenGLCachedUniforms::SetUniformVec3(environment, programCastPatch, referenceLocationLocPatch, (float)pt.x, (float)pt.y, (float)pt.z);
            }
        }
        break;
        }
    }

    if (depthPass) {

        if (generateDepthTexture) {
            SetupDepthTexture();
        }

        if (generateDepthTexture && depthTexture && depthFBO) {
            if (environment && environment->GetRenderer() < OPENGL_3_2) {
                glPushAttrib(GL_VIEWPORT_BIT);
            } else {
                if (!environment->GetViewport(mySavedVP[0], mySavedVP[1], mySavedVP[2], mySavedVP[3])) {
                    glGetIntegerv(GL_VIEWPORT, mySavedVP);
                }
            }

            OpenGLCachedUniforms::SetUniformInt(environment, programCastPatch, depthOnlyLocPatch, 1);

            glGetIntegerv(GL_FRAMEBUFFER_BINDING, &mySavedFrameBuffer);

            et->glBindFramebuffer(GL_FRAMEBUFFER, depthFBO);
            glViewport(0, 0, depthTextureSize, depthTextureSize);
            glClear(GL_DEPTH_BUFFER_BIT);
            glEnable(GL_DEPTH_TEST);
        } else {
            return false;
        }
    }

    return Utils::PrintGLErrors(__FILE__, __LINE__);
}

bool ProjectedGridOpenGL::SetPatchMatrix(const Matrix4& modelMatrix)
{
    OpenGLCachedUniforms::SetUniformMat4(environment, programCastPatch, modelMatrixLocPatch, modelMatrix.ToFloatArray());
    return true;
}

bool ProjectedGridOpenGL::UnsetPatchShader(bool depthPass)
{
    if (depthPass && !generateDepthTexture) return true;

	if(!environment)
		return true;
	osg::GLExtensions* et = environment->GetExtension();
	if(!et)
		return true;

    et->glUseProgram(0);

    if (depthPass && generateDepthTexture && depthTexture && depthFBO) {
        et->glBindFramebuffer(GL_FRAMEBUFFER, mySavedFrameBuffer);

        if (environment && environment->GetRenderer() < OPENGL_3_2) {
            glPopAttrib();
        } else {
            glViewport(mySavedVP[0], mySavedVP[1], mySavedVP[2], mySavedVP[3]);
        }
    }
    Utils::PrintGLErrors(__FILE__, __LINE__);
    return true;
}

void ProjectedGridOpenGL::Draw(double time, bool depthWrites, bool aboveWater)
{
	if(!environment)
		return;
	osg::GLExtensions* et = environment->GetExtension();
	if(!et)
		return;
	
    if (generateDepthTexture) {
        SetupDepthTexture();
    }

	//if (SwitchToMesh()) 
	if(true)
	{
		DrawMesh(time, depthWrites, aboveWater);
		return;
	}

    Utils::ClearGLErrors();

    if (waterModel && lightingModel && wakeManager && PrepShaders(aboveWater) && wakeManager->PrepShaders(environment) &&
            waterModel->PrepShaders(time) && lightingModel->PrepShaders()) {

        if (vboID && idxID) {

            if (environment && environment->GetRenderer() >= OPENGL_3_2) {
                et->glBindVertexArray(vertexArray);
                et->glBindBuffer(GL_ARRAY_BUFFER, vboID);
                et->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, idxID);
                et->glVertexAttribPointer(vertexLoc, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid *)0);
                et->glEnableVertexAttribArray(vertexLoc);
            } else {
                et->glBindBuffer(GL_ARRAY_BUFFER, vboID);
                et->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, idxID);
                glEnableClientState(GL_VERTEX_ARRAY);
                glVertexPointer(2, GL_FLOAT, 0, 0);
            }

            if (wireframe) glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

            if (doubleSided) {
                glDisable(GL_CULL_FACE);
            } else {
                glEnable(GL_CULL_FACE);
            }
            glFrontFace(GL_CW);

            glEnable(GL_DEPTH_TEST);
            glDepthMask(depthWrites ? GL_TRUE : GL_FALSE);

            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

            if (alphaMask) glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_FALSE);

            glDrawElements(GL_TRIANGLE_STRIP, nIndices, GL_UNSIGNED_INT, 0);

            DrawDepthTexture(nIndices, false);

            if (alphaMask) glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);

            glDepthMask(GL_TRUE);
            glFrontFace(GL_CCW);

            if (wireframe) glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

            if (environment && environment->GetRenderer() >= OPENGL_3_2) {
                et->glBindVertexArray(0);
            } else {
                glDisableClientState(GL_VERTEX_ARRAY);
            }

            et->glBindBuffer(GL_ARRAY_BUFFER, 0);
            et->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
        }

        //glUseProgram(0);
    }

    Utils::PrintGLErrors(__FILE__, __LINE__);
}

bool ProjectedGridOpenGL::SetupVBO(unsigned int gridResolution)
{
	if(!environment)
		return false;
	osg::GLExtensions* et = environment->GetExtension();
	if(!et)
		return false;

    bool ok = false;

    Utils::ClearGLErrors();

    vboID = idxID = 0;

    // Create the vertex array in normalized device coordinates
    Vertex *v = TRITON_NEW Vertex[gridResolution * gridResolution];
    if (!v) {
        Utils::DebugMsg("Failed to allocate system memory for OpenGL vertex buffer");
        return false;
    }

    unsigned int idx = 0;

    for (unsigned int row = 0; row < gridResolution; row++) {
        for (unsigned int col = 0; col < gridResolution; col++) {
            float x = ((float)row / ((float)gridResolution - 1.0f)) * 2.0f - 1.0f;
            float y = ((float)col / ((float)gridResolution - 1.0f)) * 2.0f - 1.0f;

            v[idx].x = x;
            v[idx].y = y;

            idx++;
        }
    }

    // Create the index array
    unsigned int stripRows = gridResolution - 1;
    unsigned int indicesPerStrip = gridResolution * 2 + 2;
    nIndices = indicesPerStrip * stripRows;

    GLuint *i = TRITON_NEW GLuint[nIndices];
    if (!i) {
        Utils::DebugMsg("Failed to allocate system memory for OpenGL index buffer");
        return false;
    }

    unsigned int vIdx = 0, iIdx = 0;

    for (unsigned int stripRow = 0; stripRow < stripRows; stripRow++) {
        i[iIdx++] = vIdx;
        for (unsigned int col = 0; col < gridResolution; col++) {
            i[iIdx++] = vIdx;
            i[iIdx++] = vIdx + gridResolution;
            vIdx++;
        }
        i[iIdx++] = vIdx - 1 + gridResolution;
    }

    // Upload both to the GPU
    et->glGenBuffers(1, &vboID);
    et->glBindBuffer(GL_ARRAY_BUFFER, vboID);
    et->glBufferData(GL_ARRAY_BUFFER, gridResolution * gridResolution * sizeof(Vertex),
                 (void *)v, GL_STATIC_DRAW_ARB);

    et->glGenBuffers(1, &idxID);
    et->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, idxID);
    et->glBufferData(GL_ELEMENT_ARRAY_BUFFER, nIndices * sizeof(GLuint),
                 (void *)i, GL_STATIC_DRAW);

    et->glBindBuffer(GL_ARRAY_BUFFER, 0);
    et->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    ok = true;

    // Clean up
    TRITON_DELETE[] v;
    TRITON_DELETE[] i;

    if (!Utils::PrintGLErrors(__FILE__, __LINE__)) ok = false;

    return ok;
}

bool ProjectedGridOpenGL::PushAllState(void)
{
    Utils::ClearGLErrors();

    if (environment && environment->GetRenderer() < OPENGL_3_2) {
        GLint currentProgram;
        glGetIntegerv(GL_CURRENT_PROGRAM, &currentProgram);
        programStack.push(currentProgram);

        glPushClientAttrib(GL_CLIENT_VERTEX_ARRAY_BIT);
        glPushAttrib(GL_ALL_ATTRIB_BITS);
    }

    return Utils::PrintGLErrors(__FILE__, __LINE__);
}

bool ProjectedGridOpenGL::PopAllState(void)
{
	if(!environment)
		return false;
	osg::GLExtensions* et = environment->GetExtension();
	if(!et)
		return false;

#if PROJECTEDGRIDOPENGL_GLGET_OPT
    if (iMaxTextures < 0)
        glGetIntegerv(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, &iMaxTextures);
#else   // PROJECTEDGRIDOPENGL_GLGET_OPT
    GLint iMaxTextures = 1;
    glGetIntegerv(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, &iMaxTextures);
#endif  // PROJECTEDGRIDOPENGL_GLGET_OPT

    if (iMaxTextures > 32) iMaxTextures = 32;

    for (int stage = 0; stage < iMaxTextures; stage++) {
        glActiveTexture(GL_TEXTURE0 + stage);
        glBindTexture(GL_TEXTURE_2D, 0);
        // MS 7/7/2015: Triton uses one cube map bound to unit 0, but
        // we will just unbind from all units.
        glBindTexture(GL_TEXTURE_CUBE_MAP_EXT, 0);
    }

    Utils::ClearGLErrors();

    if (environment && environment->GetRenderer() < OPENGL_3_2) {
        if (!programStack.empty()) {
            GLint savedProgram = programStack.top();
            et->glUseProgram(savedProgram);
            programStack.pop();
        }

        glPopAttrib();
        glPopClientAttrib();
    }

    return Utils::PrintGLErrors(__FILE__, __LINE__);
}

bool ProjectedGridOpenGL::SetDefaultState(void)
{
    if (!environment) return false;

	osg::GLExtensions* et = environment->GetExtension();
	if(!et)
		return false;

    if (setDefaultState && environment->GetRenderer() < OPENGL_3_2) {
        glDisable(GL_ALPHA_TEST);
        glDisable(GL_AUTO_NORMAL);
        glDisable(GL_BLEND);
        glDisable(GL_COLOR_MATERIAL);
        glEnable(GL_CULL_FACE);
        glEnable(GL_DEPTH_TEST);
        glDisable(GL_DITHER);
        glDisable(GL_FOG);
        glDisable(GL_LIGHT0);
        glDisable(GL_LIGHT1);
        glDisable(GL_LIGHT2);
        glDisable(GL_LIGHT3);
        glDisable(GL_LIGHTING);
        glEnable(GL_LINE_SMOOTH);
        glDisable(GL_LINE_STIPPLE);
        glDisable(GL_COLOR_LOGIC_OP);
        glDisable(GL_INDEX_LOGIC_OP);
        glDisable(GL_NORMALIZE);
        glDisable(GL_POINT_SMOOTH);
        glDisable(GL_POLYGON_OFFSET_LINE);
        glDisable(GL_POLYGON_OFFSET_FILL);
        glDisable(GL_POLYGON_OFFSET_POINT);
        glDisable(GL_POLYGON_SMOOTH);
        glDisable(GL_POLYGON_STIPPLE);
        glDisable(GL_SCISSOR_TEST);
        //glDisable(GL_STENCIL_TEST);


#if PROJECTEDGRIDOPENGL_GLGET_OPT
        if (iMaxClipPlanes < 0)
            glGetIntegerv(GL_MAX_CLIP_PLANES, &iMaxClipPlanes);
#else   // PROJECTEDGRIDOPENGL_GLGET_OPT
        GLint iMaxClipPlanes;
        glGetIntegerv(GL_MAX_CLIP_PLANES, &iMaxClipPlanes);
#endif  // PROJECTEDGRIDOPENGL_GLGET_OPT

        for (int i = 0; i < iMaxClipPlanes; i++) {
            glDisable(GL_CLIP_PLANE0 + i);
        }

        glFrontFace(GL_CCW);

        if (hasMultisample) {
            glDisable(GL_SAMPLE_ALPHA_TO_COVERAGE_ARB);
        }

        if (&et->glBindBuffer) {
            et->glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, 0 );
            et->glBindBuffer(GL_ARRAY_BUFFER,0);
        }

#if PROJECTEDGRIDOPENGL_GLGET_OPT
        if (iMaxTextures < 0)
            glGetIntegerv(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, &iMaxTextures);
        if (iMaxFixedFunctionTextures < 0)
            glGetIntegerv(GL_MAX_TEXTURE_UNITS, &iMaxFixedFunctionTextures);
#else   // PROJECTEDGRIDOPENGL_GLGET_OPT
        GLint iMaxTextures = 1;
        glGetIntegerv(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, &iMaxTextures);
        GLint iMaxFixedFunctionTextures a= 1;
        glGetIntegerv(GL_MAX_TEXTURE_UNITS, &iMaxFixedFunctionTextures);
#endif  // PROJECTEDGRIDOPENGL_GLGET_OPT

        int stage;
        for (stage = 0; stage < iMaxFixedFunctionTextures; stage++) {
            glActiveTexture(GL_TEXTURE0 + stage);

            glDisable(GL_TEXTURE_RECTANGLE_NV);
            glDisable(GL_TEXTURE_1D);
            glDisable(GL_TEXTURE_2D);
            glDisable(GL_TEXTURE_3D);

            glDisable(GL_TEXTURE_GEN_S);
            glDisable(GL_TEXTURE_GEN_T);
            glDisable(GL_TEXTURE_GEN_R);
            glDisable(GL_TEXTURE_GEN_Q);
        }

        glActiveTexture(GL_TEXTURE0);

        glClientActiveTexture(GL_TEXTURE0);

        glDisableClientState(GL_NORMAL_ARRAY);
        glDisableClientState(GL_FOG_COORD_ARRAY);
        glDisableClientState(GL_INDEX_ARRAY);
        glDisableClientState(GL_EDGE_FLAG_ARRAY);
        glDisableClientState(GL_SECONDARY_COLOR_ARRAY);

        if (hasUnifiedVA) {
            glDisableClientState(GL_VERTEX_ATTRIB_ARRAY_UNIFIED_NV);
        }
    } else {
        // OpenGL 3.2+
        if (et->glBindVertexArray) {
            et->glBindVertexArray(0);
        }

        if (&et->glBindBuffer) {
            et->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
            et->glBindBuffer(GL_ARRAY_BUFFER,0);
        }

        glDisable(GL_BLEND);
        glEnable(GL_CULL_FACE);
        glEnable(GL_DEPTH_TEST);
        glDisable(GL_DITHER);
        glDisable(GL_COLOR_LOGIC_OP);
        glDisable(GL_POLYGON_OFFSET_LINE);
        glDisable(GL_POLYGON_OFFSET_FILL);
        glDisable(GL_POLYGON_OFFSET_POINT);
        glDisable(GL_POLYGON_SMOOTH);
        glDisable(GL_SCISSOR_TEST);
        //glDisable(GL_STENCIL_TEST);

        if (hasMultisample) {
            glDisable(GL_SAMPLE_ALPHA_TO_COVERAGE_ARB);
        }
    }

    Utils::PrintGLErrors(__FILE__, __LINE__);

    return true;
}

typedef struct MeshVertex_S {
    float x, y, z, w;
} MeshVertex;

bool ProjectedGridOpenGL::SetupMesh()
{
	    if(!environment)
			return false;
		osg::GLExtensions* et = environment->GetExtension();
		if(!et)
			return false;
	    
		meshVboID = meshIdxID = 0;

		if (!useMesh) return true;

		int nVerts = meshGridResolution * meshGridResolution;
		// Create the vertex array in normalized device coordinates
		MeshVertex *v = TRITON_NEW MeshVertex[nVerts];
		if (!v) {
			return false;
		}

		unsigned int idx = 0;

		float cellWidth = 1.0f / (float)(meshGridResolution - 1.0f);

		float foregroundCutoff = Ly * 2;
		float innerResolution = foregroundCutoff / (meshGridResolution / 2);

		for (int row = 0; row < meshGridResolution; row++) {

			float r;
			if (row < meshGridResolution / 2) {
				r = row * innerResolution;
			} else {
				float x = (float)(row - meshGridResolution / 2) / (float)(meshGridResolution / 2);
				x = x * x * x;
				r = foregroundCutoff + x * (float)(meshSize - foregroundCutoff);
			}

			for (int col = 0; col < meshGridResolution; col++) {

				float theta = (float)(col * cellWidth) * (float)TRITON_TWOPI;

				float x = r * sinf(theta);
				float y = r * cosf(theta);

				float error = 0;

				if (environment && environment->IsGeocentric()) {
					double d = sqrt(earthRadii.x * earthRadii.x + r * r);
					error = (float)(d - earthRadii.x);
				}

				v[idx].x = x;
				v[idx].y = y;
				v[idx].z = -error;

				v[idx].w = 1.0;

				idx++;
			}
		}

		// Create the index array
		unsigned int stripRows = meshGridResolution - 1;
		unsigned int indicesPerStrip = meshGridResolution * 2 + 2;
		meshNIndices = indicesPerStrip * stripRows;

		GLuint *i = TRITON_NEW GLuint[meshNIndices];
		if (!i) {
			return false;
		}

		unsigned int vIdx = 0, iIdx = 0;

		for (unsigned int stripRow = 0; stripRow < stripRows; stripRow++) {
			i[iIdx++] = vIdx;
			for (int col = 0; col < meshGridResolution; col++) {
				i[iIdx++] = vIdx;
				i[iIdx++] = vIdx + meshGridResolution;
				vIdx++;
			}
			i[iIdx++] = vIdx - 1 + meshGridResolution;
		}

		et->glGenVertexArrays(1, &meshVertexArray);

		// Upload both to the GPU
		et->glGenBuffers(1, &meshVboID);
		et->glBindBuffer(GL_ARRAY_BUFFER, meshVboID);
		et->glBufferData(GL_ARRAY_BUFFER, meshGridResolution * meshGridResolution * sizeof(MeshVertex),
			(void *)v, GL_DYNAMIC_DRAW_ARB);

		et->glGenBuffers(1, &meshIdxID);
		et->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, meshIdxID);
		et->glBufferData(GL_ELEMENT_ARRAY_BUFFER, meshNIndices * sizeof(GLuint),
			(void *)i, GL_DYNAMIC_DRAW);

		et->glBindBuffer(GL_ARRAY_BUFFER, 0);
		et->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

		// Clean up
		TRITON_DELETE[] v;
		TRITON_DELETE[] i;

		Utils::PrintGLErrors(__FILE__, __LINE__);

		return true;
//    meshVboID = meshIdxID = 0;
//
//    if (!useMesh) return true;
//
//    int nVerts = meshGridResolution * meshGridResolution;
//    // Create the vertex array in normalized device coordinates
//    MeshVertex *v = TRITON_NEW MeshVertex[nVerts];
//    if (!v) {
//        return false;
//    }
//
//    float halfMesh = (float)meshGridResolution * 0.5f;
//    unsigned int idx = 0;
//
//    float foregroundCutoff = Ly * 2.0f;
//
//    int innerCells = (int)((float)meshGridResolution * 0.75f);
//    float outerCells = (float)meshGridResolution * 0.25f;
//
//    float innerResolution = foregroundCutoff * 2.0f / (float)innerCells;
//    float nScale = (float)meshSize / ((float)meshGridResolution * innerResolution * 2.0f);
//    nScale /= 0.5f;
//
//    for (int row = 0; row < meshGridResolution; row++) {
//        for (int col = 0; col < meshGridResolution; col++) {
//
//            float x, y;
//            int lowBound = (meshGridResolution >> 1) - (innerCells >> 1);
//            int highBound = (meshGridResolution >> 1) + (innerCells >> 1);
//            if (row >= lowBound && row <= highBound && col >= lowBound && col <= highBound) {
//                x = ((float)col - halfMesh) * innerResolution;
//                y = ((float)row - halfMesh) * innerResolution;
//            } else {
//
//#define MAX(a, b) ( (a) > (b) ? (a) : (b) )
//
//                int ringNum = 0;
//                int r = 0;
//                if (row > highBound) {
//                    r = MAX(r, row - highBound);
//                }
//                if (row < lowBound) {
//                    r = MAX(r, lowBound - row);
//                }
//                if (col < lowBound) {
//                    r = MAX(r, lowBound - col);
//                }
//                if (col > highBound) {
//                    r = MAX(r, col - highBound);
//                }
//
//                ringNum = r;
//
//                float n = (float)(ringNum) * 2.0f  / (float)outerCells;
//                n = n * n * n * n * n * n;
//                n *= nScale;
//                n += 1.0f;
//
//                x = ((float)col - halfMesh) * innerResolution * n;
//                y = ((float)row - halfMesh) * innerResolution * n;
//            }
//
//
//            float error = 0;
//            if (environment && environment->IsGeocentric()) {
//                double rsqr = (x*x + y*y);
//                double d = sqrt(earthRadii.x * earthRadii.x + rsqr);
//                error = (float)(d - earthRadii.x);
//            }
//
//            v[idx].x = x;
//            v[idx].y = y;
//            v[idx].z = -error;
//
//            v[idx].w = 1.0;
//
//            idx++;
//        }
//    }
//
//    // Create the index array
//    unsigned int stripRows = meshGridResolution - 1;
//    unsigned int indicesPerStrip = meshGridResolution * 2 + 2;
//    meshNIndices = indicesPerStrip * stripRows;
//
//    GLuint *i = TRITON_NEW GLuint[meshNIndices];
//    if (!i) {
//        return false;
//    }
//
//    unsigned int vIdx = 0, iIdx = 0;
//
//    for (unsigned int stripRow = 0; stripRow < stripRows; stripRow++) {
//        i[iIdx++] = vIdx;
//        for (int col = 0; col < meshGridResolution; col++) {
//            i[iIdx++] = vIdx;
//            i[iIdx++] = vIdx + meshGridResolution;
//            vIdx++;
//        }
//        i[iIdx++] = vIdx - 1 + meshGridResolution;
//    }
//
//
//    glGenVertexArrays(1, &meshVertexArray);
//
//    // Upload both to the GPU
//    glGenBuffers(1, &meshVboID);
//    glBindBuffer(GL_ARRAY_BUFFER, meshVboID);
//    glBufferData(GL_ARRAY_BUFFER, meshGridResolution * meshGridResolution * sizeof(MeshVertex),
//                 (void *)v, GL_STATIC_DRAW_ARB);
//
//    glGenBuffers(1, &meshIdxID);
//    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, meshIdxID);
//    glBufferData(GL_ELEMENT_ARRAY_BUFFER, meshNIndices * sizeof(GLuint),
//                 (void *)i, GL_STATIC_DRAW);
//
//    glBindBuffer(GL_ARRAY_BUFFER, 0);
//    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
//
//    // Clean up
//    TRITON_DELETE[] v;
//    TRITON_DELETE[] i;
//
//    Utils::PrintGLErrors(__FILE__, __LINE__);
//
//    return true;
}

bool ProjectedGridOpenGL::DrawMesh(double time, bool depthWrites, bool aboveWater)
{
	if(!environment)
		return false;
	osg::GLExtensions* et = environment->GetExtension();
	if(!et)
		return false;

    Utils::PrintGLErrors(__FILE__, __LINE__);

    if (environment && environment->GetRenderer() >= OPENGL_3_2) {
        et->glBindVertexArray(meshVertexArray);
        et->glBindBuffer(GL_ARRAY_BUFFER, meshVboID);
        et->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, meshIdxID);
    } else {
        et->glBindBuffer(GL_ARRAY_BUFFER, meshVboID);
        et->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, meshIdxID);
        glEnableClientState(GL_VERTEX_ARRAY);
        glVertexPointer(4, GL_FLOAT, 0, 0);
    }

    Utils::PrintGLErrors(__FILE__, __LINE__);

    Matrix4 meshMatrix = ComputeMeshMatrix();

    SetPatchShader(time, 16, 0, false, meshMatrix, aboveWater, false);

    if (doubleSided) {
        glDisable(GL_CULL_FACE);
    } else {
        glEnable(GL_CULL_FACE);
    }

    glEnable(GL_DEPTH_TEST);
    glDepthMask(depthWrites ? GL_TRUE : GL_FALSE);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    if (alphaMask) glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_FALSE);

    if (wireframe) glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    glDrawElements(GL_TRIANGLE_STRIP, meshNIndices, GL_UNSIGNED_INT, 0);

    DrawDepthTexture(meshNIndices, true);

    if (alphaMask) glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);

    glDepthMask(GL_TRUE);

    if (wireframe) glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    if (environment && environment->GetRenderer() >= OPENGL_3_2) {
        et->glBindVertexArray(0);
    } else {
        glDisableClientState(GL_VERTEX_ARRAY);
    }

    et->glBindBuffer(GL_ARRAY_BUFFER, 0);
    et->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    UnsetPatchShader(false);

    if (environment && environment->GetRenderer() >= OPENGL_3_2) {
        et->glBindVertexArray(0);
    }

    //glUseProgram(0);

    Utils::PrintGLErrors(__FILE__, __LINE__);

    return true;
}

void ProjectedGridOpenGL::DrawDepthTexture(int numIndices, bool patch)
{
	if(!environment)
		return;
	osg::GLExtensions* et = environment->GetExtension();
	if(!et)
		return;

    if (generateDepthTexture && depthTexture && depthFBO) {
        int savedVP[4];
        if (environment && environment->GetRenderer() < OPENGL_3_2) {
            glPushAttrib(GL_VIEWPORT_BIT);
        } else {
            if (!environment->GetViewport(savedVP[0], savedVP[1], savedVP[2], savedVP[3])) {
                glGetIntegerv(GL_VIEWPORT, savedVP);
            }
        }

        if (patch) {
            OpenGLCachedUniforms::SetUniformInt(environment, programCastPatch, depthOnlyLocPatch, 1);
        } else {
            OpenGLCachedUniforms::SetUniformInt(environment, programCast, depthOnlyLoc, 1);
        }

        GLint savedFrameBuffer = 0;
        glGetIntegerv(GL_FRAMEBUFFER_BINDING, &savedFrameBuffer);

        et->glBindFramebuffer(GL_FRAMEBUFFER, depthFBO);
        glViewport(0, 0, depthTextureSize, depthTextureSize);
        glClear(GL_DEPTH_BUFFER_BIT);
        glDrawElements(GL_TRIANGLE_STRIP, numIndices, GL_UNSIGNED_INT, 0);
        et->glBindFramebuffer(GL_FRAMEBUFFER, savedFrameBuffer);

        if (environment && environment->GetRenderer() < OPENGL_3_2) {
            glPopAttrib();
        } else {
            glViewport(savedVP[0], savedVP[1], savedVP[2], savedVP[3]);
        }
    }

    Utils::PrintGLErrors(__FILE__, __LINE__);
}

