// Copyright (c) 2011-2015 Sundog Software LLC. All rights reserved worldwide.

#include <FeTriton/WakeManagerOpenGL.h>
#include <FeTriton/Environment.h>
#include <FeTriton/ParticleSystemOpenGL.h>
#include <FeTriton/TGALoader.h>
#include <FeTriton/Matrix4.h>
#include <FeTriton/Configuration.h>

#ifndef _WIN32
// sprintf_s not available on non Windows platforms, but snprintf has same functionality so use instead.
#define sprintf_s snprintf
#endif

using namespace Triton;

WakeManagerOpenGL::~WakeManagerOpenGL()
{
    //Utils::PrintGLErrors(__FILE__, __LINE__);
    DeleteShaders();

    if (rotorWashTexture) {
        GLuint rotorWashID = (GLuint)rotorWashTexture;
        glDeleteTextures(1, &rotorWashID);
        rotorWashTexture = 0;
    }

    if (tidalStreamTexture) {
        GLuint tidalStreamID = (GLuint)tidalStreamTexture;
        glDeleteTextures(1, &tidalStreamID);
        tidalStreamTexture = 0;
    }

    if (sprayTexture) {
        glDeleteTextures(1, &sprayTexture);
        sprayTexture = 0;
    }

    if (displacementTexture) {
        glDeleteTextures(1, &displacementTexture);
        displacementTexture = 0;
    }

    if (circularWaves) {
        TRITON_DELETE[] circularWaves;
    }

    if (patchCircularWaves) {
        TRITON_DELETE[] patchCircularWaves;
    }

    if (kelvinWakes) {
        TRITON_DELETE[] kelvinWakes;
    }

    if (patchKelvinWakes) {
        TRITON_DELETE[] patchKelvinWakes;
    }

    if (propWashes) {
        TRITON_DELETE[] propWashes;
    }

    if (patchPropWashes) {
        TRITON_DELETE[] patchPropWashes;
    }

    if (leewardDampeners) {
        TRITON_DELETE[] leewardDampeners;
    }

    if (patchLeewardDampeners) {
        TRITON_DELETE[] patchLeewardDampeners;
    }

    //Utils::PrintGLErrors(__FILE__, __LINE__);
}

void WakeManagerOpenGL::DeleteShaders()
{
	if(environment)
	{
		osg::GLExtensions* et = environment->GetExtension();
		if(et)
		{
			if (uboHandle) {
				et->glDeleteBuffers(1, &uboHandle);
			}

			if (blockBuffer) {
				TRITON_FREE(blockBuffer);
			}

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
		}
	}
    //Utils::PrintGLErrors(__FILE__, __LINE__);
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

    Utils::DebugMsg("GLSL error detected in wake manager!\n");

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
            et->glGetProgramiv(objCast, GL_INFO_LOG_LENGTH,
                           &infologLength);
        }

        if (infologLength > 0) {
            infoLog = (char *)TRITON_MALLOC(infologLength);
            if (et->glIsShader(objCast)) {
                et->glGetShaderInfoLog(objCast, infologLength, &charsWritten, infoLog);
            } else {
                et->glGetProgramInfoLog(objCast, infologLength, &charsWritten, infoLog);
            }
            Utils::DebugMsg(infoLog);
            TRITON_FREE(infoLog);
        }
    }
}

GLhandleARB WakeManagerOpenGL::LoadShader( const char *shaderName, bool vertexShader, bool userShader,
        const Environment *environment)
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

        bool usingPointSprites = ParticleSystemOpenGL::UsingPointSprites();

        const char *userString = (vertexShader) ?
                                 environment->GetUserDefinedVertString() : environment->GetUserDefinedFragString();
        const char *userDefines = ( userString && userShader ) ? userString : "";
        // --- END NEW --------------------------------------------------------

        if (environment->GetRenderer() >= OPENGL_3_2) {
            if (usingPointSprites) {
                const char defines[] = "#version 150\n#define OPENGL32\n#define POINT_SPRITES\n";
                const char *sources[3] = { defines, userDefines, data};
                et->glShaderSource(shader, 3, sources, NULL);
            } else {
                const char defines[] = "#version 150\n#define OPENGL32\n";
                const char *sources[3] = { defines, userDefines, data};
                et->glShaderSource(shader, 3, sources, NULL);
            }
        } else {
            if (usingPointSprites) {
                const char defines[] = "#version 120\n#define POINT_SPRITES\n";
                const char *ssc[3] = { defines, userDefines, data};
                et->glShaderSource(shader, 3, ssc, NULL);
            } else {
                const char defines[] = "#version 120\n";
                const char *ssc[3] = { defines, userDefines, data};
                et->glShaderSource(shader, 3, ssc, NULL);
            }
        }

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

bool WakeManagerOpenGL::ReloadShaders(const TRITON_VECTOR(unsigned int)& shaders, unsigned int gridShader, unsigned int gridPatchShader)
{
    shader = (ShaderHandle)gridShader;
    patchShader = (ShaderHandle)gridPatchShader;

    DeleteShaders();

    userShaders = shaders;

    SetupShaders();

    GetGridUniforms();

    UniLocs ul;
    // Set shader for sprays
    for (int i = 0; i < numSprays; i++) {
        ParticleSystemOpenGL *ps = (ParticleSystemOpenGL*) sprays[i];
        if(ps) {
            ps->SetShader((ShaderHandle)programCast);
            if(i == 0) {
                ul = ps->GetUniLocsFromShader();
            } else {
                ps->SetShaderUniLocs(ul);
            }
        }
    }

    Utils::PrintGLErrors(__FILE__, __LINE__);

    return true;
}

bool WakeManagerOpenGL::SetupShaders()
{
    Utils::ClearGLErrors();

	if(!environment)
		return false;
	osg::GLExtensions* et = environment->GetExtension();
	if(!et)
		return false;

    OpenGLCachedUniforms::ClearCaches();

    vertShader = LoadShader(Utils::GetParticleShaderFileName(environment, false).c_str(), true, false, environment);
    fragShader = LoadShader(Utils::GetParticleShaderFileName(environment, true).c_str(), false, false, environment);

    userShader = LoadShader(Utils::GetUserShaderFileName().c_str(), false, true, environment);
    userVertShader = LoadShader(Utils::GetUserVertShaderFileName().c_str(), true, true, environment);

#ifdef __APPLE__
    vertCastShader = reinterpret_cast<uintptr_t>(vertShader);
    fragCastShader = reinterpret_cast<uintptr_t>(fragShader);
    userCastShader = reinterpret_cast<uintptr_t>(userShader);
    userVertCastShader = reinterpret_cast<uintptr_t>(userVertShader);
#else
    vertCastShader = vertShader;
    fragCastShader = fragShader;
    userCastShader = userShader;
    userVertCastShader = userVertShader;
#endif

    if (vertCastShader && fragCastShader) {
        programCast = et->glCreateProgram();

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
            Utils::DebugMsg("Failed to link particle shader program for OpenGL.");
            return false;
        }
    }

    Utils::PrintGLErrors(__FILE__, __LINE__);

    return true;
}

static void LoadTexture(TGALoader& loader, const Environment *environment)
{
	if(!environment)
		return;
	osg::GLExtensions* et = environment->GetExtension();
	if(!et)
		return;

#ifdef __APPLE__
    if (loader.GetBitsPerPixel() == 24) {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_COMPRESSED_RGB, loader.GetWidth(), loader.GetHeight(), 0, GL_RGB, GL_UNSIGNED_BYTE, loader.GetPixels());
    } else {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_COMPRESSED_RGBA, loader.GetWidth(), loader.GetHeight(), 0, GL_RGBA, GL_UNSIGNED_BYTE, loader.GetPixels());
    }
    glGenerateMipmap(GL_TEXTURE_2D);
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
    Utils::PrintGLErrors(__FILE__, __LINE__);
}

bool WakeManagerOpenGL::Initialize(const ProjectedGrid *grid, const Matrix3& basis, const Environment *environment)
{
    int i;

    usingUBO = OpenGLExtensionManager::HasUBO(environment);

    WakeManager::Initialize(grid, basis, environment);

    maxKelvinWakes = 25;
    Configuration::GetIntValue("max-wake-waves-opengl", maxKelvinWakes);
    kelvinWakes = TRITON_NEW KelvinWakeLocsOpenGL[maxKelvinWakes];
    patchKelvinWakes = TRITON_NEW KelvinWakeLocsOpenGL[maxKelvinWakes];

    maxCircularWaves = 25;
    Configuration::GetIntValue("max-circular-waves-opengl", maxCircularWaves);
    circularWaves = TRITON_NEW CircularWaveLocsOpenGL[maxCircularWaves];
    patchCircularWaves = TRITON_NEW CircularWaveLocsOpenGL[maxCircularWaves];

    maxPropWashes = 25;
    Configuration::GetIntValue("max-prop-wash-opengl", maxPropWashes);
    propWashes = TRITON_NEW PropWashLocsOpenGL[maxPropWashes];
    patchPropWashes = TRITON_NEW PropWashLocsOpenGL[maxPropWashes];

    maxLeewardDampeners = 5;
    Configuration::GetIntValue("max-leeward-dampeners-opengl", maxLeewardDampeners);
    leewardDampeners = TRITON_NEW LeewardDampenerLocsOpenGL[maxLeewardDampeners];
    patchLeewardDampeners = TRITON_NEW LeewardDampenerLocsOpenGL[maxLeewardDampeners];

    Configuration::GetFloatValue("leeward-dampening-strength", leewardDampeningStrength);

    if (!SetupShaders()) {
        return false;
    }

    TGALoader loader;
    if (!loader.Load("spray.tga", environment->GetResourceLoader())) {
        Utils::DebugMsg("Couldn't load spray texture");
        return false;
    }

    glGenTextures(1, &sprayTexture);
    glBindTexture(GL_TEXTURE_2D, sprayTexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    LoadTexture(loader, environment);

    for (i = 0; i < numSprays; i++) {
        sprays[i] = TRITON_NEW ParticleSystemOpenGL(environment);
        sprays[i]->Initialize((TextureHandle)sprayTexture, (ShaderHandle)programCast, numParticles, grid);
        sprays[i]->SetTransparency((float)sprayTransparency);
        if (particleSystemManager) {
            particleSystemManager->AddParticleSystem(sprays[i]);
        }
    }

    if (!loader.Load("rotor.tga", environment->GetResourceLoader())) {
        Utils::DebugMsg("Couldn't load rotor wash texture");
        return false;
    }

    GLuint rotorTextureID;
    glGenTextures(1, &rotorTextureID);
    glBindTexture(GL_TEXTURE_2D, rotorTextureID);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    LoadTexture(loader, environment);

    rotorWashTexture = (TextureHandle)rotorTextureID;

    if (!loader.Load("tidal-stream.tga", environment->GetResourceLoader())) {
        Utils::DebugMsg("Couldn't load tidal stream texture");
        return false;
    }

    GLuint tidalStreamTextureID;
    glGenTextures(1, &tidalStreamTextureID);
    glBindTexture(GL_TEXTURE_2D, tidalStreamTextureID);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    LoadTexture(loader, environment);

    tidalStreamTexture = (TextureHandle)tidalStreamTextureID;


    if (!loader.Load("wake-interference-normal-height.tga", environment->GetResourceLoader())) {
        Utils::DebugMsg("Couldn't load wake displacement texture");
        return false;
    }

    glGenTextures(1, &displacementTexture);
    glBindTexture(GL_TEXTURE_2D, displacementTexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    LoadTexture(loader, environment);

    GetGridUniforms();

    return Utils::PrintGLErrors(__FILE__, __LINE__);
}

static GLint GetUniformOffset(Triton::ShaderHandle shader, const char *name,const Environment* env)
{
	if(!env)
		return -1;
	osg::GLExtensions* et = env->GetExtension();
	if(!et)
		return -1;

    GLuint index;
    GLint offset;
    const GLchar *names[1];
    names[0] = name;
    et->glGetUniformIndices((GLint)shader, 1, names, &index);

    if (index != GL_INVALID_INDEX) {
        et->glGetActiveUniformsiv((GLint)shader, 1, &index, GL_UNIFORM_OFFSET, &offset);
        return offset;
    } else {
        return -1;
    }
}

static GLint GetUniformElementOffset(Triton::ShaderHandle shader, const char *name, int i,const Environment* env)
{
	if(!env)
		return -1;
	osg::GLExtensions* et = env->GetExtension();
	if(!et)
		return -1;

    char buf[256];
#if (defined(_WIN32) && _MSC_VER < 1400)
    sprintf(buf, name, i);
#else
    sprintf_s(buf, 256, name, i);
#endif

    GLuint index;
    GLint offset;
    const GLchar *names[1];
    names[0] = buf;
    et->glGetUniformIndices((GLint)shader, 1, names, &index);
    if (index != GL_INVALID_INDEX) {
        et->glGetActiveUniformsiv((GLint)shader, 1, &index, GL_UNIFORM_OFFSET, &offset);
        return offset;
    } else {
        return -1;
    }
}

void WakeManagerOpenGL::GetGridUniforms()
{
	if(!environment)
		return;
	osg::GLExtensions* et = environment->GetExtension();
	if(!et)
		return;

    if (shader) {
        displacementTextureLoc = et->glGetUniformLocation((GLint)shader, "trit_displacementTexture");
    }

    if (patchShader) {
        displacementTextureLocPatch = et->glGetUniformLocation((GLint)patchShader, "trit_displacementTexture");
    }

    if (usingUBO && shader && patchShader) {
        blockIndex = et->glGetUniformBlockIndex((GLint)shader, "trit_WakeParameters");
        blockIndexPatch = et->glGetUniformBlockIndex((GLint)patchShader, "trit_WakeParameters");
        et->glGetActiveUniformBlockiv((GLint)shader, blockIndex, GL_UNIFORM_BLOCK_DATA_SIZE, &blockSize);
        blockBuffer = (GLubyte*)TRITON_MALLOC(blockSize);

        memset(blockBuffer, 0, blockSize);

        numPropWashesLoc = GetUniformOffset(shader, "trit_numPropWashes",environment);
        numCircularWavesLoc = GetUniformOffset(shader, "trit_numCircularWaves",environment);
        numKelvinWakesLoc = GetUniformOffset(shader, "trit_numKelvinWakes",environment);
        numLeewardDampenersLoc = GetUniformOffset(shader, "trit_numLeewardDampeners",environment);
        numPropWashesLocPatch = GetUniformOffset(patchShader, "trit_numPropWashes",environment);
        numCircularWavesLocPatch = GetUniformOffset(patchShader, "trit_numCircularWaves",environment);
        numKelvinWakesLocPatch = GetUniformOffset(patchShader, "trit_numKelvinWakes",environment);
        numLeewardDampenersLocPatch = GetUniformOffset(patchShader, "trit_numLeewardDampeners",environment);
        leewardDampeningStrengthLocPatch = GetUniformOffset(patchShader, "trit_leewardDampeningStrength",environment);

        doWakesLoc = GetUniformOffset(shader, "trit_doWakes",environment);
        washLengthLoc = GetUniformOffset(shader, "trit_washLength",environment);
        doWakesLocPatch = GetUniformOffset(patchShader, "trit_doWakes",environment);
        washLengthLocPatch = GetUniformOffset(patchShader, "trit_washLength",environment);

#define GET_OFFSETS(str, var, arr, patchArr) \
   arr[i].var = GetUniformElementOffset(shader, str, i,environment); \
   patchArr[i].var = GetUniformElementOffset(patchShader, str, i,environment);

        int i;
        for (i = 0; i < maxCircularWaves; i++) {
            GET_OFFSETS("trit_circularWaves[%d].amplitude", amplitudeLoc, circularWaves, patchCircularWaves)
            GET_OFFSETS("trit_circularWaves[%d].radius", radiusLoc, circularWaves, patchCircularWaves)
            GET_OFFSETS("trit_circularWaves[%d].k", kLoc, circularWaves, patchCircularWaves)
            GET_OFFSETS("trit_circularWaves[%d].halfWavelength", halfWavelengthLoc, circularWaves, patchCircularWaves)
            GET_OFFSETS("trit_circularWaves[%d].position", positionLoc, circularWaves, patchCircularWaves)
        }

        for (i = 0; i < maxKelvinWakes; i++) {
            GET_OFFSETS("trit_wakes[%d].amplitude", amplitudeLoc, kelvinWakes, patchKelvinWakes)
            GET_OFFSETS("trit_wakes[%d].position", positionLoc, kelvinWakes, patchKelvinWakes)
            GET_OFFSETS("trit_wakes[%d].shipPosition", shipPositionLoc, kelvinWakes, patchKelvinWakes)
            GET_OFFSETS("trit_wakes[%d].foamAmount", foamAmountLoc, kelvinWakes, patchKelvinWakes)
        }

        for (i = 0; i < maxPropWashes; i++) {
            GET_OFFSETS("trit_washes[%d].deltaPos", deltaPosLoc, propWashes, patchPropWashes)
            GET_OFFSETS("trit_washes[%d].washWidth", washWidthLoc, propWashes, patchPropWashes)
            GET_OFFSETS("trit_washes[%d].propPosition", propPositionLoc, propWashes, patchPropWashes)
            GET_OFFSETS("trit_washes[%d].distFromSource", distFromSourceLoc, propWashes, patchPropWashes)
            GET_OFFSETS("trit_washes[%d].washLength", washLengthLoc, propWashes, patchPropWashes)
            GET_OFFSETS("trit_washes[%d].alphaStart", alphaStartLoc, propWashes, patchPropWashes)
            GET_OFFSETS("trit_washes[%d].alphaEnd", alphaEndLoc, propWashes, patchPropWashes)
        }

        for (i = 0; i < maxLeewardDampeners; i++) {
            GET_OFFSETS("trit_leewardDampeners[%d].bowPos", bowPosLoc, leewardDampeners, patchLeewardDampeners)
            GET_OFFSETS("trit_leewardDampeners[%d].sternPos", sternPosLoc, leewardDampeners, patchLeewardDampeners)
            GET_OFFSETS("trit_leewardDampeners[%d].velocityDampening", velocityDampeningLoc, leewardDampeners, patchLeewardDampeners)
        }

        et->glGenBuffers(1, &uboHandle);
        et->glBindBuffer(GL_UNIFORM_BUFFER, uboHandle);
        et->glBufferData(GL_UNIFORM_BUFFER, blockSize, blockBuffer, GL_DYNAMIC_DRAW);
        et->glUniformBlockBinding((GLuint)shader, blockIndex, 0);
        et->glUniformBlockBinding((GLuint)patchShader, blockIndexPatch, 0);
        et->glBindBufferBase(GL_UNIFORM_BUFFER, 0, uboHandle);

    } else {
        char buf[256];
#if (defined(_WIN32) && _MSC_VER < 1400)
#define MAKE_VAR(str, var, arr, patchArr) \
       sprintf(buf, str, i); \
       arr[i].var = glGetUniformLocation((GLint)shader, buf); \
       if (patchShader) patchArr[i].var = glGetUniformLocation((GLint)patchShader, buf);
#else
#define MAKE_VAR(str, var, arr, patchArr) \
       sprintf_s(buf, 256, str, i); \
       arr[i].var = et->glGetUniformLocation((GLint)shader, buf); \
       if (patchShader) patchArr[i].var = et->glGetUniformLocation((GLint)patchShader, buf);
#endif

        int i;

        for (i = 0; i < maxCircularWaves; i++) {
            MAKE_VAR("trit_circularWaves[%d].amplitude", amplitudeLoc, circularWaves, patchCircularWaves)
            MAKE_VAR("trit_circularWaves[%d].radius", radiusLoc, circularWaves, patchCircularWaves)
            MAKE_VAR("trit_circularWaves[%d].k", kLoc, circularWaves, patchCircularWaves)
            MAKE_VAR("trit_circularWaves[%d].halfWavelength", halfWavelengthLoc, circularWaves, patchCircularWaves)
            MAKE_VAR("trit_circularWaves[%d].position", positionLoc, circularWaves, patchCircularWaves)
        }

        for (i = 0; i < maxKelvinWakes; i++) {
            MAKE_VAR("trit_wakes[%d].amplitude", amplitudeLoc, kelvinWakes, patchKelvinWakes)
            MAKE_VAR("trit_wakes[%d].position", positionLoc, kelvinWakes, patchKelvinWakes)
            MAKE_VAR("trit_wakes[%d].shipPosition", shipPositionLoc, kelvinWakes, patchKelvinWakes)
            MAKE_VAR("trit_wakes[%d].foamAmount", foamAmountLoc, kelvinWakes, patchKelvinWakes)
        }

        for (i = 0; i < maxPropWashes; i++) {
            MAKE_VAR("trit_washes[%d].deltaPos", deltaPosLoc, propWashes, patchPropWashes)
            MAKE_VAR("trit_washes[%d].washWidth", washWidthLoc, propWashes, patchPropWashes)
            MAKE_VAR("trit_washes[%d].propPosition", propPositionLoc, propWashes, patchPropWashes)
            MAKE_VAR("trit_washes[%d].distFromSource", distFromSourceLoc, propWashes, patchPropWashes)
            MAKE_VAR("trit_washes[%d].washLength", washLengthLoc, propWashes, patchPropWashes)
            MAKE_VAR("trit_washes[%d].alphaStart", alphaStartLoc, propWashes, patchPropWashes)
            MAKE_VAR("trit_washes[%d].alphaEnd", alphaEndLoc, propWashes, patchPropWashes)
        }

        for (i = 0; i < maxLeewardDampeners; i++) {
            MAKE_VAR("trit_leewardDampeners[%d].bowPos", bowPosLoc, leewardDampeners, patchLeewardDampeners)
            MAKE_VAR("trit_leewardDampeners[%d].sternPos", sternPosLoc, leewardDampeners, patchLeewardDampeners)
            MAKE_VAR("trit_leewardDampeners[%d].velocityDampening", velocityDampeningLoc, leewardDampeners, patchLeewardDampeners)
        }

        numPropWashesLoc = et->glGetUniformLocation((GLint)shader, "trit_numPropWashes");
        numCircularWavesLoc = et->glGetUniformLocation((GLint)shader, "trit_numCircularWaves");
        numKelvinWakesLoc = et->glGetUniformLocation((GLint)shader, "trit_numKelvinWakes");
        numLeewardDampenersLoc = et->glGetUniformLocation((GLint)shader, "trit_numLeewardDampeners");
        leewardDampeningStrengthLoc = et->glGetUniformLocation((GLint)shader, "trit_leewardDampeningStrength");

        if (patchShader) {
            numPropWashesLocPatch = et->glGetUniformLocation((GLint)patchShader, "trit_numPropWashes");
            numCircularWavesLocPatch = et->glGetUniformLocation((GLint)patchShader, "trit_numCircularWaves");
            numKelvinWakesLocPatch = et->glGetUniformLocation((GLint)patchShader, "trit_numKelvinWakes");
            numLeewardDampenersLoc = et->glGetUniformLocation((GLint)patchShader, "trit_numLeewardDampeners");
        }

        if (shader) {
            doWakesLoc = et->glGetUniformLocation((GLint)shader, "trit_doWakes");
            washLengthLoc = et->glGetUniformLocation((GLint)shader, "trit_washLength");
        }

        if (patchShader) {
            doWakesLocPatch = et->glGetUniformLocation((GLint)patchShader, "trit_doWakes");
            washLengthLocPatch = et->glGetUniformLocation((GLint)patchShader, "trit_washLength");
        }
    }

    Utils::PrintGLErrors(__FILE__, __LINE__);
}

int WakeManagerOpenGL::SetCircularWaves(GLint program, CircularWaveLocsOpenGL *waves, const Environment *env)
{
    int numWaves = 0;

    TRITON_LIST(WaterDisturbance *)::iterator it = visibleWaves.begin();
    while (it != visibleWaves.end()) {

        if ( (*it)->GetClassID() == CIRCULAR_WAVE) {
            CircularWave *wave = reinterpret_cast<CircularWave*>(*it);

            if (!env->CullSphere(wave->position, wave->radius + wave->wavelength * 0.25)) {
                if (waves[numWaves].amplitudeLoc != -1) {
                    OpenGLCachedUniforms::SetUniformFloat(env, program, waves[numWaves].amplitudeLoc, (float)(wave->amplitude));
                }
                if (waves[numWaves].radiusLoc != -1) {
                    OpenGLCachedUniforms::SetUniformFloat(env, program, waves[numWaves].radiusLoc, (float)(wave->radius));
                }
                if (waves[numWaves].kLoc != -1) {
                    float k = (float)((2.0 * TRITON_PI) / wave->wavelength);
                    OpenGLCachedUniforms::SetUniformFloat(env, program, waves[numWaves].kLoc, k);
                }
                if (waves[numWaves].halfWavelengthLoc != -1) {
                    OpenGLCachedUniforms::SetUniformFloat(env, program, waves[numWaves].halfWavelengthLoc, (float)(wave->wavelength * 0.5));
                }
                if (waves[numWaves].positionLoc != -1) {
                    Vector3 position = wave->position;
                    if (!env->IsGeocentric()) {
                        position = position * basis;
                    } else {
                        position = position - env->GetCameraPosition();
                    }
                    OpenGLCachedUniforms::SetUniformVec3(env, program, waves[numWaves].positionLoc, (float)position.x, (float)position.y, (float)position.z);
                }
                numWaves++;
                if (numWaves >= maxCircularWaves) {
                    break;
                }
            }
        }
        it++;
    }
    /*
    for (int i = numWaves; i < maxCircularWaves; i++) {
        if (waves[i].amplitudeLoc != -1) {
            glUniform1f(waves[i].amplitudeLoc, 0.0f);
        }
        if (waves[i].halfWavelengthLoc != -1) {
            glUniform1f(waves[i].halfWavelengthLoc, 0.0f);
        }
        if (waves[i].kLoc != -1) {
            glUniform1f(waves[i].kLoc, 0.0f);
        }
        if (waves[i].radiusLoc != -1) {
            glUniform1f(waves[i].radiusLoc, 0.0f);
        }
        if (waves[i].positionLoc != -1) {
            glUniform3f(waves[i].positionLoc, 0.0f, 0.0f, 0.0f);
        }
    }
    */
    return numWaves;
}

int WakeManagerOpenGL::SetPropWashes(GLint program, PropWashLocsOpenGL *washes, const Environment *env)
{
    int numWaves = 0;

    TRITON_LIST(WaterDisturbance *)::iterator it = visibleWaves.begin();
    while (it != visibleWaves.end()) {

        if ( (*it)->GetClassID() == PROP_WASH) {

            PropWashSegment *seg = reinterpret_cast<PropWashSegment*>(*it);

            Vector3 deltaPos = seg->deltaPos;
            Vector3 propPosition = seg->position;
            if (!env->IsGeocentric()) {
                deltaPos = deltaPos * basis;
                propPosition = propPosition * basis;
            } else {
                propPosition = propPosition - env->GetCameraPosition();
            }
            if (washes[numWaves].deltaPosLoc != -1) {
                OpenGLCachedUniforms::SetUniformVec3(env, program, washes[numWaves].deltaPosLoc, (float)deltaPos.x, (float)deltaPos.y, (float)deltaPos.z);
            }

            if (washes[numWaves].distFromSourceLoc != -1) {
                OpenGLCachedUniforms::SetUniformFloat(env, program, washes[numWaves].distFromSourceLoc, (float)seg->GetDistanceFromGenerator());
            }

            if (washes[numWaves].propPositionLoc != -1) {
                OpenGLCachedUniforms::SetUniformVec3(env, program, washes[numWaves].propPositionLoc, (float)propPosition.x, (float)propPosition.y, (float)propPosition.z);
            }

            if (washes[numWaves].washWidthLoc != -1) {
                OpenGLCachedUniforms::SetUniformFloat(env, program, washes[numWaves].washWidthLoc, (float)seg->washWidth);
            }

            if (washes[numWaves].washLengthLoc != -1) {
                OpenGLCachedUniforms::SetUniformFloat(env, program, washes[numWaves].washLengthLoc, seg->generator ? (float)seg->generator->GetDistanceTravelled() : 0);
            }

            if (washes[numWaves].alphaStartLoc != -1) {
                OpenGLCachedUniforms::SetUniformFloat(env, program, washes[numWaves].alphaStartLoc, (float)seg->alphaStart);
            }

            if (washes[numWaves].alphaEndLoc != -1) {
                OpenGLCachedUniforms::SetUniformFloat(env, program, washes[numWaves].alphaEndLoc, (float)seg->alphaEnd);
            }

            numWaves++;
            if (numWaves >= maxPropWashes) {
                break;
            }

        }
        it++;
    }
    /*
    for (int i = numWaves; i < maxPropWashes; i++) {
        if (washes[i].deltaPosLoc != -1) {
            glUniform3f(washes[i].deltaPosLoc, 1.0f, 0.0f, 0.0f);
        }
        if (washes[i].distFromSourceLoc != -1) {
            glUniform1f(washes[i].distFromSourceLoc, 0.0f);
        }
        if (washes[i].propPositionLoc != -1) {
            glUniform3f(washes[i].propPositionLoc, 0.0f, 0.0f, 0.0f);
        }
        if (washes[i].washWidthLoc != -1) {
            glUniform1f(washes[i].washWidthLoc, 0.0f);
        }
        if (washes[i].washLengthLoc != -1) {
            glUniform1f(washes[i].washLengthLoc, 0.0f);
        }
    }
    */

    return numWaves;
}

int WakeManagerOpenGL::SetLeewardDampeners(GLint program, LeewardDampenerLocsOpenGL *dampeners, const Environment *env)
{
    int numDampeners = 0;

    if (leewardDampening) {
        TRITON_LIST(WaterDisturbance *)::iterator it = visibleWaves.begin();
        while (it != visibleWaves.end()) {
            if ((*it)->GetClassID() == LEEWARD_DAMPENER) {
                LeewardDampener *ld = reinterpret_cast<LeewardDampener*>(*it);

                Vector3 bowPos = ld->bowPos;
                Vector3 sternPos = ld->sternPos;

                if (!env->CullSphere(ld->position, (sternPos - bowPos).Length())) {

                    if (!env->IsGeocentric()) {
                        bowPos = bowPos * basis;
                        sternPos = sternPos * basis;
                    } else {
                        bowPos = bowPos - env->GetCameraPosition();
                        sternPos = sternPos - env->GetCameraPosition();
                    }

                    if (dampeners[numDampeners].bowPosLoc != -1) {
                        OpenGLCachedUniforms::SetUniformVec3(env, program, dampeners[numDampeners].bowPosLoc, (float)bowPos.x, (float)bowPos.y, (float)bowPos.z);
                    }

                    if (dampeners[numDampeners].sternPosLoc != -1) {
                        OpenGLCachedUniforms::SetUniformVec3(env, program, dampeners[numDampeners].sternPosLoc, (float)sternPos.x, (float)sternPos.y, (float)sternPos.z);
                    }

                    if (dampeners[numDampeners].velocityDampeningLoc != -1) {
                        OpenGLCachedUniforms::SetUniformFloat(env, program, dampeners[numDampeners].velocityDampeningLoc, ld->velocityDampening);
                    }

                    numDampeners++;
                    if (numDampeners >= maxLeewardDampeners) {
                        break;
                    }
                }
            }

            it++;
        }
    }

    return numDampeners;
}

int WakeManagerOpenGL::SetKelvinWakes(GLint program, KelvinWakeLocsOpenGL *wakes, const Environment *env)
{
    int numWaves = 0;

    TRITON_LIST(WaterDisturbance *)::iterator it = visibleWaves.begin();
    while (it != visibleWaves.end()) {
        if ( (*it)->GetClassID() == KELVIN_WAKE) {
            KelvinWake *wake = reinterpret_cast<KelvinWake*>(*it);

            Vector3 shipPos;
            if (wake->isExplicit) {
                shipPos = wake->explicitOrigin;
            } else {
                shipPos = wake->generator->GetSternPosition();
            }
            Vector3 toOrigin = wake->position - shipPos;
            double distance = toOrigin.Length();

            if (!env->CullSphere(shipPos + toOrigin * 0.5, distance * 0.5 * TRITON_SQRT2)) {
                if (wakes[numWaves].amplitudeLoc != -1) {
                    OpenGLCachedUniforms::SetUniformFloat(env, program, wakes[numWaves].amplitudeLoc, (float)(wake->amplitude));
                }
                if (wakes[numWaves].positionLoc != -1) {
                    Vector3 position = wake->position;
                    if (!env->IsGeocentric()) {
                        position = position * basis;
                    } else {
                        position = position - env->GetCameraPosition();
                    }
                    OpenGLCachedUniforms::SetUniformVec3(env, program, wakes[numWaves].positionLoc, (float)position.x, (float)position.y, (float)position.z);
                }
                if (wakes[numWaves].shipPositionLoc != -1) {
                    if (!env->IsGeocentric()) {
                        shipPos = shipPos * basis;
                    } else {
                        shipPos = shipPos - env->GetCameraPosition();
                    }
                    OpenGLCachedUniforms::SetUniformVec3(env, program, wakes[numWaves].shipPositionLoc, (float)shipPos.x, (float)shipPos.y, (float)shipPos.z);
                }
                if (wakes[numWaves].foamAmountLoc != -1) {
                    OpenGLCachedUniforms::SetUniformFloat(env, program, wakes[numWaves].foamAmountLoc, wake->foamAmount);
                }

                numWaves++;
                if (numWaves >= maxKelvinWakes) {
                    break;
                }
            }
        }
        it++;
    }
    /*
    for (int i = numWaves; i < maxKelvinWakes; i++) {
        if (wakes[i].amplitudeLoc != -1) {
            glUniform1f(wakes[i].amplitudeLoc, 0.0f);
        }
        if (wakes[i].positionLoc != -1) {
            glUniform3f(wakes[i].positionLoc, 0.0f, 0.0f, 0.0f);
        }
        if (wakes[i].shipPositionLoc != -1) {
            glUniform3f(wakes[i].shipPositionLoc, 0.0f, 0.0f, 0.0f);
        }
    }
    */
    return numWaves;
}

bool WakeManagerOpenGL::PrepShaders(const Environment *env)
{
    int numWakeWaves = 0;

    if (!env) return false;

	osg::GLExtensions* et = env->GetExtension();
	if(!et)
		return false;

    Utils::ClearGLErrors();

    if (usingUBO) {
        et->glBindBuffer(GL_UNIFORM_BUFFER, uboHandle);
        et->glBindBufferBase(GL_UNIFORM_BUFFER, 0, uboHandle);
        OpenGLCachedUniforms::SetUBO(blockBuffer);
    }

    int numCircularWaves, numKelvinWakes, numPropWashes, numDampeners;
    numCircularWaves = SetCircularWaves((GLint)shader, circularWaves, env);
    numKelvinWakes = SetKelvinWakes((GLint)shader, kelvinWakes, env);
    numPropWashes = SetPropWashes((GLint)shader, propWashes, env);
    numDampeners = SetLeewardDampeners((GLint)shader, leewardDampeners, env);

    numWakeWaves = numCircularWaves + numKelvinWakes + numPropWashes + numDampeners;

    if (numCircularWavesLoc != -1) {
        OpenGLCachedUniforms::SetUniformInt(env, (GLint)shader, numCircularWavesLoc, numCircularWaves);
    }

    if (numKelvinWakesLoc != -1) {
        OpenGLCachedUniforms::SetUniformInt(env, (GLint)shader, numKelvinWakesLoc, numKelvinWakes);
    }

    if (numPropWashesLoc != -1) {
        OpenGLCachedUniforms::SetUniformInt(env, (GLint)shader, numPropWashesLoc, numPropWashes);
    }

    if (numLeewardDampenersLoc != -1) {
        OpenGLCachedUniforms::SetUniformInt(env, (GLint)shader, numLeewardDampenersLoc, numDampeners);
    }

    if (leewardDampeningStrengthLoc != -1) {
        OpenGLCachedUniforms::SetUniformFloat(env, (GLint)shader, leewardDampeningStrengthLoc, leewardDampeningStrength);
    }

    if (doWakesLoc != -1) {
        int doWakes = numWakeWaves > 0 ? 1 : 0;
        OpenGLCachedUniforms::SetUniformInt(env, (GLint)shader, doWakesLoc, doWakes);
    }

    if (washLengthLoc != -1) {
        float washLength = 1000.0f;
        Configuration::GetFloatValue("wake-wash-length", washLength);
        OpenGLCachedUniforms::SetUniformFloat(env, (GLint)shader, washLengthLoc, washLength / (float)env->GetWorldUnits());
    }

    if (usingUBO) {
        OpenGLCachedUniforms::ClearUBO();
        et->glBufferSubData(GL_UNIFORM_BUFFER, 0, blockSize, blockBuffer);
    }

    if (displacementTextureLoc != -1 && displacementTexture) {
        glActiveTexture(GL_TEXTURE8);
        glBindTexture(GL_TEXTURE_2D, displacementTexture);
        OpenGLCachedUniforms::SetUniformInt(env, (GLint)shader, displacementTextureLoc, 8);
    }

    return Utils::PrintGLErrors(__FILE__, __LINE__);
}

bool WakeManagerOpenGL::PrepPatchShader(const Environment *env)
{
    int numWakeWaves = 0;

    if (!env) return false;
	osg::GLExtensions* et = env->GetExtension();
	if(!et)
		return false;

    Utils::ClearGLErrors();

    int numCircularWaves, numKelvinWakes, numPropWashes, numDampeners;

    if (usingUBO) {
        et->glBindBuffer(GL_UNIFORM_BUFFER, uboHandle);
        et->glBindBufferBase(GL_UNIFORM_BUFFER, 0, uboHandle);
        OpenGLCachedUniforms::SetUBO(blockBuffer);
    }

    numCircularWaves = SetCircularWaves((GLint)patchShader, patchCircularWaves, env);
    numKelvinWakes = SetKelvinWakes((GLint)patchShader, patchKelvinWakes, env);
    numPropWashes = SetPropWashes((GLint)patchShader, patchPropWashes, env);
    numDampeners = SetLeewardDampeners((GLint)patchShader, patchLeewardDampeners, env);

    numWakeWaves = numCircularWaves + numKelvinWakes + numPropWashes + numDampeners;

    if (numCircularWavesLocPatch != -1) {
        OpenGLCachedUniforms::SetUniformInt(env, (GLint)patchShader, numCircularWavesLocPatch, numCircularWaves);
    }

    if (numKelvinWakesLocPatch != -1) {
        OpenGLCachedUniforms::SetUniformInt(env, (GLint)patchShader, numKelvinWakesLocPatch, numKelvinWakes);
    }

    if (numPropWashesLocPatch != -1) {
        OpenGLCachedUniforms::SetUniformInt(env, (GLint)patchShader, numPropWashesLocPatch, numPropWashes);
    }

    if (numLeewardDampenersLocPatch != -1) {
        OpenGLCachedUniforms::SetUniformInt(env, (GLint)patchShader, numLeewardDampenersLocPatch, numDampeners);
    }

    if (leewardDampeningStrengthLocPatch != -1) {
        OpenGLCachedUniforms::SetUniformFloat(env, (GLint)patchShader, leewardDampeningStrengthLocPatch, leewardDampeningStrength);
    }

    if (doWakesLocPatch != -1) {
        int doWakes = numWakeWaves > 0 ? 1 : 0;
        OpenGLCachedUniforms::SetUniformInt(env, (GLint)patchShader, doWakesLocPatch, doWakes);
    }

    if (washLengthLocPatch != -1) {
        float washLength = 1000.0f;
        Configuration::GetFloatValue("wake-wash-length", washLength);
        OpenGLCachedUniforms::SetUniformFloat(env, (GLint)patchShader, washLengthLocPatch, washLength / (float)env->GetWorldUnits());
    }

    if (usingUBO) {
        et->glBufferSubData(GL_UNIFORM_BUFFER, 0, blockSize, blockBuffer);
        OpenGLCachedUniforms::ClearUBO();
    }

    if (displacementTextureLocPatch != -1 && displacementTexture) {
        glActiveTexture(GL_TEXTURE8);
        glBindTexture(GL_TEXTURE_2D, displacementTexture);
        OpenGLCachedUniforms::SetUniformInt(env, (GLint)patchShader, displacementTextureLocPatch, 8);
    }

    return Utils::PrintGLErrors(__FILE__, __LINE__);
}
