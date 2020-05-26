// Copyright (c) 2011-2015 Sundog Software LLC. All rights reserved worldwide.

#include <FeTriton/SprayManagerOpenGL.h>
#include <FeTriton/TGALoader.h>
#include <FeTriton/Environment.h>
#include <FeTriton/ResourceLoader.h>
#include <FeTriton/ParticleSystemOpenGL.h>

using namespace Triton;

SprayManagerOpenGL::SprayManagerOpenGL(const Environment *env, const TRITON_VECTOR(unsigned int)& shaders) : SprayManager(env)
{
    userShaders = shaders;
    vertShader = fragShader = userShader = userVertShader = program = 0;
    sprayTexture = vertCastShader = fragCastShader = userCastShader = userVertCastShader = programCast = 0;
}

SprayManagerOpenGL::~SprayManagerOpenGL()
{
    DeleteShaders();

    if (sprayTexture) {
        glDeleteTextures(1, &sprayTexture);
    }

    //Utils::PrintGLErrors(__FILE__, __LINE__);
}

void SprayManagerOpenGL::DeleteShaders()
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

			//Utils::PrintGLErrors(__FILE__, __LINE__);
		}
	}
}

bool SprayManagerOpenGL::ReloadShaders(const TRITON_VECTOR(unsigned int)& shaders)
{
    DeleteShaders();

    userShaders = shaders;

    if (SetupShaders()) {
        if (system) {
            ParticleSystemOpenGL *systemOpenGL = (ParticleSystemOpenGL *)system;
            systemOpenGL->SetShader((ShaderHandle)programCast);
            systemOpenGL->GetUniLocsFromShader();
            return true;
        }
    }

    return false;
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

    Utils::DebugMsg("GLSL error detected in spray manager!\n");

    if (OpenGLExtensionManager::HasExtension("GL_ARB_shader_objects", env)) {
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

GLhandleARB SprayManagerOpenGL::LoadShader( const char *shaderName, bool vertexShader,
        bool userShader,
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

        if (environment->GetRenderer() >= OPENGL_3_2) {
            if (usingPointSprites) {
                const char defines[] = "#version 150\n#define OPENGL32\n#define POINT_SPRITES\n";
                const char *sources[3] = { defines, userDefines, data };
                et->glShaderSource(shader, 3, sources, NULL);
            } else {
                const char defines[] = "#version 150\n#define OPENGL32\n";
                const char *sources[3] = { defines, userDefines, data };
                et->glShaderSource(shader, 3, sources, NULL);
            }
        } else {
            if (usingPointSprites) {
                const char defines[] = "#version 120\n#define POINT_SPRITES\n";
                const char *ssc[3] = { defines, userDefines, data };
                et->glShaderSource(shader, 3, ssc, NULL);
            } else {
                const char defines[] = "#version 120\n";
                const char *ssc[3] = { defines, userDefines, data };
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

bool SprayManagerOpenGL::SetupShaders()
{
    OpenGLCachedUniforms::ClearCaches();

	if(!environment)
		return false;
	osg::GLExtensions* et = environment->GetExtension();
	if(!et)
		return false;

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

    if (vertShader && fragShader) {

        programCast = et->glCreateProgram();

        et->glAttachShader(programCast, vertCastShader);
        et->glAttachShader(programCast, fragCastShader);

        if (userShader) et->glAttachShader(programCast, userCastShader);
        if (userVertShader) et->glAttachShader(programCast, userVertCastShader);

        TRITON_VECTOR(unsigned int)::const_iterator it;
        for (it = userShaders.begin(); it != userShaders.end(); it++) {
            et->glAttachShader(programCast, *it);
        }

        et->glLinkProgram(programCast);

        GLint linkStatus;
        et->glGetProgramiv(programCast, GL_LINK_STATUS, &linkStatus);
        if (linkStatus == GL_FALSE) {
            return false;
        }
    }

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

bool SprayManagerOpenGL::Initialize(const ProjectedGrid *grid)
{
    if (!grid) return false;

    if (!SprayManager::Initialize(grid)) return false;

    Utils::ClearGLErrors();

    if (!SetupShaders()) return false;

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

    system = TRITON_NEW ParticleSystemOpenGL(environment);
    system->Initialize((TextureHandle)sprayTexture, (ShaderHandle)programCast, numSprays, grid);
    system->SetTransparency(sprayTransparency);
    system->SetGravity(sprayGravity);

    Utils::PrintGLErrors(__FILE__, __LINE__);

    return true;
}
