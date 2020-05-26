// Copyright (c) 2011-2015 Sundog Software LLC. All rights reserved worldwide.

#include <FeTriton/TritonOpenGL.h>
#include <FeTriton/TritonCommon.h>
#include <FeTriton/Configuration.h>
#include <FeTriton/Environment.h>

#include <string.h>
namespace Triton
{
TRITON_MAP(UniformCacheKey, float) OpenGLCachedUniforms::floatCache;
TRITON_MAP(UniformCacheKey, Vec2) OpenGLCachedUniforms::vec2Cache;
TRITON_MAP(UniformCacheKey, Vec3) OpenGLCachedUniforms::vec3Cache;
TRITON_MAP(UniformCacheKey, Vec4) OpenGLCachedUniforms::vec4Cache;
TRITON_MAP(UniformCacheKey, int) OpenGLCachedUniforms::intCache;
TRITON_MAP(UniformCacheKey, Mat4) OpenGLCachedUniforms::mat4Cache;
TRITON_MAP(UniformCacheKey, Mat3) OpenGLCachedUniforms::mat3Cache;
GLubyte* OpenGLCachedUniforms::uboBuffer = 0;
bool OpenGLCachedUniforms::cacheUniforms = false;
bool OpenGLCachedUniforms::hasProgramUniforms = false;

#ifndef NATIVE_1_3
PFNGLACTIVETEXTUREPROC glActiveTexture;
PFNGLCLIENTACTIVETEXTUREPROC glClientActiveTexture;
#endif

//#ifndef NATIVE_3_0 crg
PFNGLGETSTRINGIPROC glGetStringi;
//#endif

void OpenGLExtensionManager::LoadGLExtensions()
{
#ifndef NATIVE_1_3
    glActiveTexture = (PFNGLACTIVETEXTUREPROC)wglGetProcAddress("glActiveTexture");
    glClientActiveTexture = (PFNGLCLIENTACTIVETEXTUREPROC)wglGetProcAddress("glClientActiveTexture");
#endif

#ifndef NATIVE_3_0
	glGetStringi = (PFNGLGETSTRINGIPROC)wglGetProcAddress("glGetStringi");
#endif
}

bool OpenGLExtensionManager::HasUBO(const Environment *env)
{
	if(!env)
		return false;
	osg::GLExtensions* et = env->GetExtension();
	if(!et)
		return false;

#ifndef _WIN32
    return false;
#else
    bool uboAvailable = OpenGLExtensionManager::HasExtension("GL_ARB_uniform_buffer_object",env) && &et->glBufferData && et->glBindBufferBase &&
                        et->glGetUniformBlockIndex && et->glUniformBlockBinding && et->glGetActiveUniformBlockiv && et->glGetActiveUniformsiv
						&& et->glGetUniformIndices;

    bool allowUBO = true;
    Configuration::GetBoolValue("use-uniform-buffer-objects", allowUBO);
    /*
    const GLubyte *str;
    str = glGetString(GL_VENDOR);
    if (str) {
        bool isATI = strstr((const char *)str, "ATI") != 0 || strstr((const char*)str, "Advanced Micro Devices, Inc.") != 0;
        if (isATI) return false;
    }
    */
    return uboAvailable && allowUBO;
#endif
}

bool OpenGLExtensionManager::HasProgramUniform()
{
	return false;
	/*bool puAvailable = OpenGLExtensionManager::HasExtension("GL_ARB_separate_shader_objects") && glProgramUniform1i && glProgramUniform2f &&
	glProgramUniform1f && glProgramUniform3f && glProgramUniform4f && glProgramUniformMatrix4fv && glProgramUniformMatrix3fv;

	bool allowPU = true;
	Configuration::GetBoolValue("use-program-uniforms", allowPU);

	return puAvailable && allowPU;*/
}

bool OpenGLExtensionManager::HasExtension(const char *name,const Environment *env)
{
	if(!env)
		return false;
	osg::GLExtensions* et = env->GetExtension();
	if(!et)
		return false;

    Utils::ClearGLErrors();
    GLint n = 0, i;

	#  ifndef GL_NUM_EXTENSIONS
	#    define GL_NUM_EXTENSIONS 0x821D
	#  endif

    glGetIntegerv(GL_NUM_EXTENSIONS, &n);
    GLenum err = glGetError();
    if (err == GL_NO_ERROR && n > 0 && glGetStringi != NULL) {
        for (i = 0; i < n; i++) {
            const char *ext = (const char *)glGetStringi(GL_EXTENSIONS, i);
            if (ext) {
                if (strcmp(ext, name) == 0)
                    return true;
            }
        }
    } else {
        const GLubyte *extensions = NULL;
        const GLubyte *start;
        GLubyte *where, *terminator;

        Utils::ClearGLErrors();
        extensions = glGetString(GL_EXTENSIONS);
        if (!extensions) return false;

        start = extensions;
        for (;;) {
            where = (GLubyte *) strstr((const char *) start, name);
            if (!where)
                break;
            terminator = where + strlen(name);
            if (where == start || *(where - 1) == ' ') {
                if (*terminator == ' ' || *terminator == '\0') {
                    return true;
                }
            }
            start = terminator;
        }
    }
    return false;
}


void OpenGLCachedUniforms::ClearCaches()
{
    Configuration::GetBoolValue("cache-uniforms", cacheUniforms);

    if (cacheUniforms) {
        floatCache.clear();
        vec2Cache.clear();
        vec3Cache.clear();
        vec4Cache.clear();
        intCache.clear();
        mat4Cache.clear();
        mat3Cache.clear();
    }

	/*static bool checkedPU = false;
	if (!checkedPU) 
	{
	hasProgramUniforms = OpenGLExtensionManager::HasProgramUniform();
	checkedPU = true;
	}*/
}

void OpenGLCachedUniforms::SetUBO(GLubyte *buffer)
{
    uboBuffer = buffer;
}

void OpenGLCachedUniforms::ClearUBO()
{
    uboBuffer = 0;
}

void OpenGLCachedUniforms::SetUniformFloat(const Environment *env, GLint program, GLint location, float value)
{
    if (location == -1) return;

	if(!env)
		return;

	osg::GLExtensions* et = env->GetExtension();
	if(!et)
		return;

    if (uboBuffer) {
        *((float *)(uboBuffer + location)) = value;
        return;
    }

    if (cacheUniforms) {
        UniformCacheKey key(env, program, location);

        TRITON_MAP(UniformCacheKey, float)::iterator it;
        it = floatCache.find(key);
        if (it != floatCache.end()) {
            if (value != (it->second)) {
                it->second = value;
                et->glUniform1f(location, value);
            }
        } else {
            floatCache[key] = value;
            et->glUniform1f(location, value);
        }
    } else {
        et->glUniform1f(location, value);
    }
}

void OpenGLCachedUniforms::SetUniformInt(const Environment *env, GLint program, GLint location, int value)
{
    if (location == -1) return;

	if(!env)
		return;

	osg::GLExtensions* et = env->GetExtension();
	if(!et)
		return;

    if (uboBuffer) {
        *((int *)(uboBuffer + location)) = value;
        return;
    }

    if (cacheUniforms) {
        UniformCacheKey key(env, program, location);

        TRITON_MAP(UniformCacheKey, int)::iterator it;
        it = intCache.find(key);
        if (it != intCache.end()) {
            if (value != (it->second)) {
                it->second = value;
                et->glUniform1i(location, value);
            }
        } else {
            intCache[key] = value;
            et->glUniform1i(location, value);
        }
    } else {
        et->glUniform1i(location, value);
    }
}

void OpenGLCachedUniforms::SetUniformVec2(const Environment *env, GLint program, GLint location, float x, float y)
{
    if (location == -1) return;

	if(!env)
		return;

	osg::GLExtensions* et = env->GetExtension();
	if(!et)
		return;

    if (uboBuffer) {
        *((float *)(uboBuffer + location)) = x;
        *((float *)(uboBuffer + location + 4)) = y;
        return;
    }

    if (cacheUniforms) {
        UniformCacheKey key(env, program, location);

        TRITON_MAP(UniformCacheKey, Vec2)::iterator it;
        it = vec2Cache.find(key);
        if (it != vec2Cache.end()) {
            if (x != it->second.x || y != it->second.y) {
                it->second.x = x;
                it->second.y = y;
                et->glUniform2f(location, x, y);
            }
        } else {
            Vec2 value;
            value.x = x;
            value.y = y;
            vec2Cache[key] = value;
            et->glUniform2f(location, x, y);
        }
    } else {
        et->glUniform2f(location, x, y);
    }
}

void OpenGLCachedUniforms::SetUniformVec3(const Environment *env, GLint program, GLint location, float x, float y, float z)
{
    if (location == -1) return;

	if(!env)
		return;

	osg::GLExtensions* et = env->GetExtension();
	if(!et)
		return;

    if (uboBuffer) {
        *((float *)(uboBuffer + location)) = x;
        *((float *)(uboBuffer + location + 4)) = y;
        *((float *)(uboBuffer + location + 8)) = z;
        return;
    }

    if (cacheUniforms) {
        UniformCacheKey key(env, program, location);

        TRITON_MAP(UniformCacheKey, Vec3)::iterator it;
        it = vec3Cache.find(key);
        if (it != vec3Cache.end()) {
            if (x != it->second.x || y != it->second.y || z != it->second.z) {
                it->second.x = x;
                it->second.y = y;
                it->second.z = z;
                et->glUniform3f(location, x, y, z);
            }
        } else {
            Vec3 value;
            value.x = x;
            value.y = y;
            value.z = z;
            vec3Cache[key] = value;
            et->glUniform3f(location, x, y, z);
        }
    } else {
        et->glUniform3f(location, x, y, z);
    }
}

void OpenGLCachedUniforms::SetUniformVec4(const Environment *env, GLint program, GLint location, float x, float y, float z, float w)
{
    if (location == -1) return;

	if(!env)
		return;

	osg::GLExtensions* et = env->GetExtension();
	if(!et)
		return;

    if (uboBuffer) {
        *((float *)(uboBuffer + location)) = x;
        *((float *)(uboBuffer + location + 4)) = y;
        *((float *)(uboBuffer + location + 8)) = z;
        *((float *)(uboBuffer + location + 12)) = w;
        return;
    }

    if (cacheUniforms) {
        UniformCacheKey key(env, program, location);

        TRITON_MAP(UniformCacheKey, Vec4)::iterator it;
        it = vec4Cache.find(key);
        if (it != vec4Cache.end()) {
            if (x != it->second.x || y != it->second.y || z != it->second.z || w != it->second.w) {
                it->second.x = x;
                it->second.y = y;
                it->second.z = z;
                it->second.w = w;
                et->glUniform4f(location, x, y, z, w);
            }
        } else {
            Vec4 value;
            value.x = x;
            value.y = y;
            value.z = z;
            value.w = w;
            vec4Cache[key] = value;
            et->glUniform4f(location, x, y, z, w);
        }
    } else {
        et->glUniform4f(location, x, y, z, w);
    }
}

void OpenGLCachedUniforms::SetUniformMat4(const Environment *env, GLint program, GLint location, const float* value)
{
    if (!value || location == -1) return;

	if(!env)
		return;

	osg::GLExtensions* et = env->GetExtension();
	if(!et)
		return;

    if (uboBuffer) {

        GLuint stride = 16;

        for (int i = 0; i < 4; i++) {
            GLuint offset = location + stride * i;
            for (int j = 0; j < 4; j++) {
                *((float *)(uboBuffer + offset)) = value[i * 4 + j];
                offset += sizeof(GLfloat);
            }
        }
        return;
    }

    if (cacheUniforms) {
        UniformCacheKey key(env, program, location);

        TRITON_MAP(UniformCacheKey, Mat4)::iterator it;
        it = mat4Cache.find(key);
        if (it != mat4Cache.end()) {

            bool same = true;
            for (int i = 0; i < 16; i++) {
                if (value[i] != (it->second).mat[i]) {
                    same = false;
                    break;
                }
            }

            if (!same) {
                for (int i = 0; i < 16; i++) {
                    it->second.mat[i] = value[i];
                }
                et->glUniformMatrix4fv(location, 1, GL_FALSE, value);
            }
        } else {
            Mat4 mat;
            for (int i = 0; i < 16; i++) {
                mat.mat[i] = value[i];
            }
            mat4Cache[key] = mat;
            et->glUniformMatrix4fv(location, 1, GL_FALSE, value);
        }
    } else {
        et->glUniformMatrix4fv(location, 1, GL_FALSE, value);
    }
}

void OpenGLCachedUniforms::SetUniformMat3(const Environment *env, GLint program, GLint location, const float* value)
{
    if (!value || location == -1) return;

	if(!env)
		return;

	osg::GLExtensions* et = env->GetExtension();
	if(!et)
		return;

    if (uboBuffer) {

        GLuint stride = 16;

        for (int i = 0; i < 3; i++) {
            GLuint offset = location + stride * i;
            for (int j = 0; j < 3; j++) {
                *((float *)(uboBuffer + offset)) = value[i * 4 + j];
                offset += sizeof(GLfloat);
            }
        }
        return;
    }

    if (cacheUniforms) {
        UniformCacheKey key(env, program, location);

        TRITON_MAP(UniformCacheKey, Mat3)::iterator it;
        it = mat3Cache.find(key);
        if (it != mat3Cache.end()) {

            bool same = true;
            for (int i = 0; i < 9; i++) {
                if (value[i] != (it->second).mat[i]) {
                    same = false;
                    break;
                }
            }

            if (!same) {
                for (int i = 0; i < 9; i++) {
                    it->second.mat[i] = value[i];
                }
                et->glUniformMatrix3fv(location, 1, GL_FALSE, value);
            }
        } else {
            Mat3 mat;
            for (int i = 0; i < 9; i++) {
                mat.mat[i] = value[i];
            }
            mat3Cache[key] = mat;
            et->glUniformMatrix3fv(location, 1, GL_FALSE, value);
        }
    } else {
        et->glUniformMatrix3fv(location, 1, GL_FALSE, value);
    }
}

}



//// Copyright (c) 2011-2015 Sundog Software LLC. All rights reserved worldwide.
//
//#include <FeTriton/TritonOpenGL.h>
//#include <FeTriton/TritonCommon.h>
//#include <FeTriton/Configuration.h>
//
//#include <string.h>
//
//#ifndef _WIN32
//#ifdef __APPLE__
//// Mac:
//#include <dlfcn.h>
//static void *wglGetProcAddress(const char *name)
//{
//    static void *glHandle = NULL;
//    void **handlePtr;
//    void *addr = NULL;
//    handlePtr = &glHandle;
//    if (NULL == *handlePtr)
//        *handlePtr = dlopen("/System/Library/Frameworks/OpenGL.framework/OpenGL", RTLD_LAZY | RTLD_GLOBAL);
//    if (NULL != *handlePtr)
//        addr = dlsym(*handlePtr, name);
//
//    return addr;
//}
//#else
//// Linux:
//#include "GL/glx.h"
//#define wglGetProcAddress(A) glXGetProcAddress((GLubyte *)A)
//#endif
//#endif
//
//namespace Triton
//{
//PFNGLGETOBJECTPARAMETERIVARBPROC glGetObjectParameterivARB;
//PFNGLGETINFOLOGARBPROC glGetInfoLogARB;
//
//#ifndef NATIVE_4_4
//PFNGLBUFFERSTORAGEPROC glBufferStorage;
//#endif
//
//#ifndef NATIVE_4_1
//PFNGLPROGRAMUNIFORM1IPROC glProgramUniform1i;
//PFNGLPROGRAMUNIFORM2FPROC glProgramUniform2f;
//PFNGLPROGRAMUNIFORM1FPROC glProgramUniform1f;
//PFNGLPROGRAMUNIFORM3FPROC glProgramUniform3f;
//PFNGLPROGRAMUNIFORM4FPROC glProgramUniform4f;
//PFNGLPROGRAMUNIFORMMATRIX4FVPROC glProgramUniformMatrix4fv;
//PFNGLPROGRAMUNIFORMMATRIX3FVPROC glProgramUniformMatrix3fv;
//#endif
//
//#ifndef NATIVE_3_2
//PFNGLGENFRAMEBUFFERSPROC glGenFramebuffers;
//PFNGLDELETEFRAMEBUFFERSPROC glDeleteFramebuffers;
//PFNGLFRAMEBUFFERTEXTURE2DPROC glFramebufferTexture2D;
//PFNGLBINDFRAMEBUFFERPROC glBindFramebuffer;
//PFNGLCHECKFRAMEBUFFERSTATUSPROC glCheckFramebufferStatus;
//PFNGLCLIENTWAITSYNCPROC glClientWaitSync;
//PFNGLFENCESYNCPROC glFenceSync;
//PFNGLDELETESYNCPROC glDeleteSync;
//#endif
//
//#ifndef NATIVE_3_1
//PFNGLGETUNIFORMBLOCKINDEXPROC glGetUniformBlockIndex;
//PFNGLUNIFORMBLOCKBINDINGPROC glUniformBlockBinding;
//PFNGLGETACTIVEUNIFORMBLOCKIVPROC glGetActiveUniformBlockiv;
//PFNGLGETACTIVEUNIFORMSIVPROC glGetActiveUniformsiv;
//PFNGLGETUNIFORMINDICESPROC glGetUniformIndices;
//#endif
//
//#ifndef NATIVE_3_0
//PFNGLGENERATEMIPMAPPROC glGenerateMipmap;
//PFNGLDELETEVERTEXARRAYSPROC glDeleteVertexArrays;
//PFNGLGENVERTEXARRAYSPROC glGenVertexArrays;
//PFNGLBINDVERTEXARRAYPROC glBindVertexArray;
//PFNGLGETSTRINGIPROC glGetStringi;
//PFNGLBINDBUFFERBASEPROC glBindBufferBase;
//#endif
//
//#ifndef NATIVE_1_3
//PFNGLACTIVETEXTUREPROC glActiveTexture;
//PFNGLCLIENTACTIVETEXTUREPROC glClientActiveTexture;
//#endif
//
//#ifndef NATIVE_1_4
//PFNGLPOINTPARAMETERFPROC glPointParameterf;
//PFNGLPOINTPARAMETERFVPROC glPointParameterfv;
//#endif
//
//#ifndef NATIVE_2_0
//PFNGLGETUNIFORMLOCATIONPROC glGetUniformLocation;
//PFNGLUNIFORM1IPROC glUniform1i;
//PFNGLUNIFORM2FPROC glUniform2f;
//PFNGLUSEPROGRAMPROC glUseProgram;
//PFNGLUNIFORM1FPROC glUniform1f;
//PFNGLDELETEBUFFERSPROC glDeleteBuffers;
//PFNGLDETACHSHADERPROC glDetachShader;
//PFNGLGETSHADERIVPROC glGetShaderiv;
//PFNGLGETPROGRAMINFOLOGPROC glGetProgramInfoLog;
//PFNGLGETSHADERINFOLOGPROC glGetShaderInfoLog;
//PFNGLCREATESHADERPROC glCreateShader;
//PFNGLCOMPILESHADERPROC glCompileShader;
//PFNGLSHADERSOURCEPROC glShaderSource;
//PFNGLCREATEPROGRAMPROC glCreateProgram;
//PFNGLATTACHSHADERPROC glAttachShader;
//PFNGLLINKPROGRAMPROC glLinkProgram;
//PFNGLGETPROGRAMIVPROC glGetProgramiv;
//PFNGLGETATTRIBLOCATIONPROC glGetAttribLocation;
//PFNGLUNIFORM3FPROC glUniform3f;
//PFNGLUNIFORM4FPROC glUniform4f;
//PFNGLUNIFORMMATRIX4FVPROC glUniformMatrix4fv;
//PFNGLUNIFORMMATRIX3FVPROC glUniformMatrix3fv;
//PFNGLBINDBUFFERPROC glBindBuffer;
//PFNGLVERTEXATTRIBPOINTERPROC glVertexAttribPointer;
//PFNGLENABLEVERTEXATTRIBARRAYPROC glEnableVertexAttribArray;
//PFNGLGENBUFFERSPROC glGenBuffers;
//PFNGLBUFFERDATAPROC glBufferData;
//PFNGLDELETESHADERPROC glDeleteShader;
//PFNGLDELETEPROGRAMPROC glDeleteProgram;
//PFNGLMAPBUFFERPROC glMapBuffer;
//PFNGLUNMAPBUFFERPROC glUnmapBuffer;
//PFNGLDISABLEVERTEXATTRIBARRAYPROC glDisableVertexAttribArray;
//PFNGLBUFFERSUBDATAPROC glBufferSubData;
//PFNGLMAPBUFFERRANGEPROC glMapBufferRange;
//PFNGLISSHADERPROC glIsShader;
//PFNGLDRAWBUFFERSPROC glDrawBuffers;
//#endif
//
//void OpenGLExtensionManager::LoadGLExtensions()
//{
//    glGetObjectParameterivARB = (PFNGLGETOBJECTPARAMETERIVARBPROC)wglGetProcAddress("glGetObjectParameterivARB");
//    glGetInfoLogARB = (PFNGLGETINFOLOGARBPROC)wglGetProcAddress("glGetInfoLogARB");
//
//#ifndef NATIVE_4_4
//    glBufferStorage = (PFNGLBUFFERSTORAGEPROC)wglGetProcAddress("glBufferStorage");
//#endif
//
//#ifndef NATIVE_4_1
//    glProgramUniform1i = (PFNGLPROGRAMUNIFORM1IPROC)wglGetProcAddress("glProgramUniform1i");
//    glProgramUniform2f = (PFNGLPROGRAMUNIFORM2FPROC)wglGetProcAddress("glProgramUniform2f");
//    glProgramUniform1f = (PFNGLPROGRAMUNIFORM1FPROC)wglGetProcAddress("glProgramUniform1f");
//    glProgramUniform3f = (PFNGLPROGRAMUNIFORM3FPROC)wglGetProcAddress("glProgramUniform3f");
//    glProgramUniform4f = (PFNGLPROGRAMUNIFORM4FPROC)wglGetProcAddress("glProgramUniform4f");
//    glProgramUniformMatrix4fv = (PFNGLPROGRAMUNIFORMMATRIX4FVPROC)wglGetProcAddress("glProgramUniformMatrix4fv");
//    glProgramUniformMatrix3fv = (PFNGLPROGRAMUNIFORMMATRIX3FVPROC)wglGetProcAddress("glProgramUniformMatrix3fv");
//#endif
//
//#ifndef NATIVE_3_2
//    glGenFramebuffers = (PFNGLGENFRAMEBUFFERSPROC)wglGetProcAddress("glGenFramebuffers");
//    glDeleteFramebuffers = (PFNGLDELETEFRAMEBUFFERSPROC)wglGetProcAddress("glDeleteFramebuffers");
//    glFramebufferTexture2D = (PFNGLFRAMEBUFFERTEXTURE2DPROC)wglGetProcAddress("glFramebufferTexture2D");
//    glBindFramebuffer = (PFNGLBINDFRAMEBUFFERPROC)wglGetProcAddress("glBindFramebuffer");
//    glCheckFramebufferStatus = (PFNGLCHECKFRAMEBUFFERSTATUSPROC)wglGetProcAddress("glCheckFramebufferStatus");
//    glClientWaitSync = (PFNGLCLIENTWAITSYNCPROC)wglGetProcAddress("glClientWaitSync");
//    glFenceSync = (PFNGLFENCESYNCPROC)wglGetProcAddress("glFenceSync");
//    glDeleteSync = (PFNGLDELETESYNCPROC)wglGetProcAddress("glDeleteSync");
//#endif
//
//#ifndef NATIVE_3_1
//    glGetUniformBlockIndex = (PFNGLGETUNIFORMBLOCKINDEXPROC)wglGetProcAddress("glGetUniformBlockIndex");
//    glUniformBlockBinding = (PFNGLUNIFORMBLOCKBINDINGPROC)wglGetProcAddress("glUniformBlockBinding");
//    glGetActiveUniformBlockiv = (PFNGLGETACTIVEUNIFORMBLOCKIVPROC)wglGetProcAddress("glGetActiveUniformBlockiv");
//    glGetActiveUniformsiv = (PFNGLGETACTIVEUNIFORMSIVPROC)wglGetProcAddress("glGetActiveUniformsiv");
//    glGetUniformIndices = (PFNGLGETUNIFORMINDICESPROC)wglGetProcAddress("glGetUniformIndices");
//#endif
//
//#ifndef NATIVE_3_0
//    glGenerateMipmap = (PFNGLGENERATEMIPMAPPROC)wglGetProcAddress("glGenerateMipmap");
//    glDeleteVertexArrays = (PFNGLDELETEVERTEXARRAYSPROC)wglGetProcAddress("glDeleteVertexArrays");
//    glBindVertexArray = (PFNGLBINDVERTEXARRAYPROC)wglGetProcAddress("glBindVertexArray");
//    glGenVertexArrays = (PFNGLGENVERTEXARRAYSPROC)wglGetProcAddress("glGenVertexArrays");
//    glGetStringi = (PFNGLGETSTRINGIPROC)wglGetProcAddress("glGetStringi");
//    glBindBufferBase = (PFNGLBINDBUFFERBASEPROC)wglGetProcAddress("glBindBufferBase");
//#endif
//
//#ifndef NATIVE_1_3
//    glActiveTexture = (PFNGLACTIVETEXTUREPROC)wglGetProcAddress("glActiveTexture");
//    glClientActiveTexture = (PFNGLCLIENTACTIVETEXTUREPROC)wglGetProcAddress("glClientActiveTexture");
//#endif
//
//#ifndef NATIVE_1_4
//    glPointParameterf = (PFNGLPOINTPARAMETERFPROC)wglGetProcAddress("glPointParameterf");
//    glPointParameterfv = (PFNGLPOINTPARAMETERFVPROC)wglGetProcAddress("glPointParameterfv");
//#endif
//
//#ifndef NATIVE_2_0
//    glGetUniformLocation = (PFNGLGETUNIFORMLOCATIONPROC)wglGetProcAddress("glGetUniformLocation");
//
//    glUniform1i = (PFNGLUNIFORM1IPROC)wglGetProcAddress("glUniform1i");
//    glUniform2f = (PFNGLUNIFORM2FPROC)wglGetProcAddress("glUniform2f");
//    glUniform1f = (PFNGLUNIFORM1FPROC)wglGetProcAddress("glUniform1f");
//    glUseProgram = (PFNGLUSEPROGRAMPROC)wglGetProcAddress("glUseProgram");
//
//    glDeleteBuffers = (PFNGLDELETEBUFFERSPROC)wglGetProcAddress("glDeleteBuffers");
//    glDetachShader = (PFNGLDETACHSHADERPROC)wglGetProcAddress("glDetachShader");
//    glGetShaderiv = (PFNGLGETSHADERIVPROC)wglGetProcAddress("glGetShaderiv");
//    glGetProgramInfoLog = (PFNGLGETPROGRAMINFOLOGPROC)wglGetProcAddress("glGetProgramInfoLog");
//    glGetShaderInfoLog = (PFNGLGETSHADERINFOLOGPROC)wglGetProcAddress("glGetShaderInfoLog");
//    glCreateShader = (PFNGLCREATESHADERPROC)wglGetProcAddress("glCreateShader");
//    glCompileShader = (PFNGLCOMPILESHADERPROC)wglGetProcAddress("glCompileShader");
//    glShaderSource = (PFNGLSHADERSOURCEPROC)wglGetProcAddress("glShaderSource");
//
//    glCreateProgram = (PFNGLCREATEPROGRAMPROC)wglGetProcAddress("glCreateProgram");
//    glAttachShader = (PFNGLATTACHSHADERPROC)wglGetProcAddress("glAttachShader");
//    glLinkProgram = (PFNGLLINKPROGRAMPROC)wglGetProcAddress("glLinkProgram");
//    glGetProgramiv = (PFNGLGETPROGRAMIVPROC)wglGetProcAddress("glGetProgramiv");
//    glGetAttribLocation = (PFNGLGETATTRIBLOCATIONPROC)wglGetProcAddress("glGetAttribLocation");
//    glUniform3f = (PFNGLUNIFORM3FPROC)wglGetProcAddress("glUniform3f");
//    glUniform4f = (PFNGLUNIFORM4FPROC)wglGetProcAddress("glUniform4f");
//    glUniformMatrix4fv = (PFNGLUNIFORMMATRIX4FVPROC)wglGetProcAddress("glUniformMatrix4fv");
//    glUniformMatrix3fv = (PFNGLUNIFORMMATRIX3FVPROC)wglGetProcAddress("glUniformMatrix3fv");
//
//    glBindBuffer = (PFNGLBINDBUFFERPROC)wglGetProcAddress("glBindBuffer");
//    glVertexAttribPointer = (PFNGLVERTEXATTRIBPOINTERPROC)wglGetProcAddress("glVertexAttribPointer");
//    glEnableVertexAttribArray = (PFNGLENABLEVERTEXATTRIBARRAYPROC)wglGetProcAddress("glEnableVertexAttribArray");
//    glGenBuffers = (PFNGLGENBUFFERSPROC)wglGetProcAddress("glGenBuffers");
//    glBufferData = (PFNGLBUFFERDATAPROC)wglGetProcAddress("glBufferData");
//    glDeleteShader = (PFNGLDELETESHADERPROC)wglGetProcAddress("glDeleteShader");
//    glMapBuffer = (PFNGLMAPBUFFERPROC)wglGetProcAddress("glMapBuffer");
//    glUnmapBuffer = (PFNGLUNMAPBUFFERPROC)wglGetProcAddress("glUnmapBuffer");
//    glDisableVertexAttribArray = (PFNGLDISABLEVERTEXATTRIBARRAYPROC)wglGetProcAddress("glDisableVertexAttribArray");
//    glDeleteProgram = (PFNGLDELETEPROGRAMPROC)wglGetProcAddress("glDeleteProgram");
//    glBufferSubData = (PFNGLBUFFERSUBDATAPROC)wglGetProcAddress("glBufferSubData");
//    glMapBufferRange = (PFNGLMAPBUFFERRANGEPROC)wglGetProcAddress("glMapBufferRange");
//
//    glIsShader = (PFNGLISSHADERPROC)wglGetProcAddress("glIsShader");
//
//    glDrawBuffers = (PFNGLDRAWBUFFERSPROC)wglGetProcAddress("glDrawBuffers");
//#endif
//}
//
//bool OpenGLExtensionManager::HasUBO()
//{
//#ifndef _WIN32
//    return false;
//#else
//    bool uboAvailable = OpenGLExtensionManager::HasExtension("GL_ARB_uniform_buffer_object") && &glBufferData && glBindBufferBase &&
//                        glGetUniformBlockIndex && glUniformBlockBinding && glGetActiveUniformBlockiv && glGetActiveUniformsiv && glGetUniformIndices;
//
//    bool allowUBO = true;
//    Configuration::GetBoolValue("use-uniform-buffer-objects", allowUBO);
//    /*
//    const GLubyte *str;
//    str = glGetString(GL_VENDOR);
//    if (str) {
//        bool isATI = strstr((const char *)str, "ATI") != 0 || strstr((const char*)str, "Advanced Micro Devices, Inc.") != 0;
//        if (isATI) return false;
//    }
//    */
//    return uboAvailable && allowUBO;
//#endif
//}
//
//bool OpenGLExtensionManager::HasProgramUniform()
//{
//    bool puAvailable = OpenGLExtensionManager::HasExtension("GL_ARB_separate_shader_objects") && glProgramUniform1i && glProgramUniform2f &&
//                       glProgramUniform1f && glProgramUniform3f && glProgramUniform4f && glProgramUniformMatrix4fv && glProgramUniformMatrix3fv;
//
//    bool allowPU = true;
//    Configuration::GetBoolValue("use-program-uniforms", allowPU);
//
//    return puAvailable && allowPU;
//}
//
//bool OpenGLExtensionManager::HasExtension(const char *name)
//{
//    Utils::ClearGLErrors();
//    GLint n = 0, i;
//    glGetIntegerv(GL_NUM_EXTENSIONS, &n);
//    GLenum err = glGetError();
//    if (err == GL_NO_ERROR && n > 0 && glGetStringi != NULL) {
//        for (i = 0; i < n; i++) {
//            const char *ext = (const char *)glGetStringi(GL_EXTENSIONS, i);
//            if (ext) {
//                if (strcmp(ext, name) == 0)
//                    return true;
//            }
//        }
//    } else {
//        const GLubyte *extensions = NULL;
//        const GLubyte *start;
//        GLubyte *where, *terminator;
//
//        Utils::ClearGLErrors();
//        extensions = glGetString(GL_EXTENSIONS);
//        if (!extensions) return false;
//
//        start = extensions;
//        for (;;) {
//            where = (GLubyte *) strstr((const char *) start, name);
//            if (!where)
//                break;
//            terminator = where + strlen(name);
//            if (where == start || *(where - 1) == ' ') {
//                if (*terminator == ' ' || *terminator == '\0') {
//                    return true;
//                }
//            }
//            start = terminator;
//        }
//    }
//    return false;
//}
//
//
//TRITON_MAP(UniformCacheKey, float) OpenGLCachedUniforms::floatCache;
//TRITON_MAP(UniformCacheKey, Vec2) OpenGLCachedUniforms::vec2Cache;
//TRITON_MAP(UniformCacheKey, Vec3) OpenGLCachedUniforms::vec3Cache;
//TRITON_MAP(UniformCacheKey, Vec4) OpenGLCachedUniforms::vec4Cache;
//TRITON_MAP(UniformCacheKey, int) OpenGLCachedUniforms::intCache;
//TRITON_MAP(UniformCacheKey, Mat4) OpenGLCachedUniforms::mat4Cache;
//TRITON_MAP(UniformCacheKey, Mat3) OpenGLCachedUniforms::mat3Cache;
//GLubyte* OpenGLCachedUniforms::uboBuffer = 0;
//bool OpenGLCachedUniforms::cacheUniforms = false;
//bool OpenGLCachedUniforms::hasProgramUniforms = false;
//
//void OpenGLCachedUniforms::ClearCaches()
//{
//    Configuration::GetBoolValue("cache-uniforms", cacheUniforms);
//
//    if (cacheUniforms) {
//        floatCache.clear();
//        vec2Cache.clear();
//        vec3Cache.clear();
//        vec4Cache.clear();
//        intCache.clear();
//        mat4Cache.clear();
//        mat3Cache.clear();
//    }
//
//    static bool checkedPU = false;
//    if (!checkedPU) {
//        hasProgramUniforms = OpenGLExtensionManager::HasProgramUniform();
//        checkedPU = true;
//    }
//}
//
//void OpenGLCachedUniforms::SetUBO(GLubyte *buffer)
//{
//    uboBuffer = buffer;
//}
//
//void OpenGLCachedUniforms::ClearUBO()
//{
//    uboBuffer = 0;
//}
//
//void OpenGLCachedUniforms::SetUniformFloat(const Environment *env, GLint program, GLint location, float value)
//{
//    if (location == -1) return;
//
//    if (uboBuffer) {
//        *((float *)(uboBuffer + location)) = value;
//        return;
//    }
//
//    if (cacheUniforms) {
//        UniformCacheKey key(env, program, location);
//
//        TRITON_MAP(UniformCacheKey, float)::iterator it;
//        it = floatCache.find(key);
//        if (it != floatCache.end()) {
//            if (value != (it->second)) {
//                it->second = value;
//                hasProgramUniforms ? glProgramUniform1f(program, location, value) : glUniform1f(location, value);
//            }
//        } else {
//            floatCache[key] = value;
//            hasProgramUniforms ? glProgramUniform1f(program, location, value) : glUniform1f(location, value);
//        }
//    } else {
//        hasProgramUniforms ? glProgramUniform1f(program, location, value) : glUniform1f(location, value);
//    }
//}
//
//void OpenGLCachedUniforms::SetUniformInt(const Environment *env, GLint program, GLint location, int value)
//{
//    if (location == -1) return;
//
//    if (uboBuffer) {
//        *((int *)(uboBuffer + location)) = value;
//        return;
//    }
//
//    if (cacheUniforms) {
//        UniformCacheKey key(env, program, location);
//
//        TRITON_MAP(UniformCacheKey, int)::iterator it;
//        it = intCache.find(key);
//        if (it != intCache.end()) {
//            if (value != (it->second)) {
//                it->second = value;
//                hasProgramUniforms ? glProgramUniform1i(program, location, value) : glUniform1i(location, value);
//            }
//        } else {
//            intCache[key] = value;
//            hasProgramUniforms ? glProgramUniform1i(program, location, value) : glUniform1i(location, value);
//        }
//    } else {
//        hasProgramUniforms ? glProgramUniform1i(program, location, value) : glUniform1i(location, value);
//    }
//}
//
//void OpenGLCachedUniforms::SetUniformVec2(const Environment *env, GLint program, GLint location, float x, float y)
//{
//    if (location == -1) return;
//
//    if (uboBuffer) {
//        *((float *)(uboBuffer + location)) = x;
//        *((float *)(uboBuffer + location + 4)) = y;
//        return;
//    }
//
//    if (cacheUniforms) {
//        UniformCacheKey key(env, program, location);
//
//        TRITON_MAP(UniformCacheKey, Vec2)::iterator it;
//        it = vec2Cache.find(key);
//        if (it != vec2Cache.end()) {
//            if (x != it->second.x || y != it->second.y) {
//                it->second.x = x;
//                it->second.y = y;
//                hasProgramUniforms ? glProgramUniform2f(program, location, x, y) : glUniform2f(location, x, y);
//            }
//        } else {
//            Vec2 value;
//            value.x = x;
//            value.y = y;
//            vec2Cache[key] = value;
//            hasProgramUniforms ? glProgramUniform2f(program, location, x, y) : glUniform2f(location, x, y);
//        }
//    } else {
//        hasProgramUniforms ? glProgramUniform2f(program, location, x, y) : glUniform2f(location, x, y);
//    }
//}
//
//void OpenGLCachedUniforms::SetUniformVec3(const Environment *env, GLint program, GLint location, float x, float y, float z)
//{
//    if (location == -1) return;
//
//    if (uboBuffer) {
//        *((float *)(uboBuffer + location)) = x;
//        *((float *)(uboBuffer + location + 4)) = y;
//        *((float *)(uboBuffer + location + 8)) = z;
//        return;
//    }
//
//    if (cacheUniforms) {
//        UniformCacheKey key(env, program, location);
//
//        TRITON_MAP(UniformCacheKey, Vec3)::iterator it;
//        it = vec3Cache.find(key);
//        if (it != vec3Cache.end()) {
//            if (x != it->second.x || y != it->second.y || z != it->second.z) {
//                it->second.x = x;
//                it->second.y = y;
//                it->second.z = z;
//                hasProgramUniforms ? glProgramUniform3f(program, location, x, y, z) : glUniform3f(location, x, y, z);
//            }
//        } else {
//            Vec3 value;
//            value.x = x;
//            value.y = y;
//            value.z = z;
//            vec3Cache[key] = value;
//            hasProgramUniforms ? glProgramUniform3f(program, location, x, y, z) : glUniform3f(location, x, y, z);
//        }
//    } else {
//        hasProgramUniforms ? glProgramUniform3f(program, location, x, y, z) : glUniform3f(location, x, y, z);
//    }
//}
//
//void OpenGLCachedUniforms::SetUniformVec4(const Environment *env, GLint program, GLint location, float x, float y, float z, float w)
//{
//    if (location == -1) return;
//
//    if (uboBuffer) {
//        *((float *)(uboBuffer + location)) = x;
//        *((float *)(uboBuffer + location + 4)) = y;
//        *((float *)(uboBuffer + location + 8)) = z;
//        *((float *)(uboBuffer + location + 12)) = w;
//        return;
//    }
//
//    if (cacheUniforms) {
//        UniformCacheKey key(env, program, location);
//
//        TRITON_MAP(UniformCacheKey, Vec4)::iterator it;
//        it = vec4Cache.find(key);
//        if (it != vec4Cache.end()) {
//            if (x != it->second.x || y != it->second.y || z != it->second.z || w != it->second.w) {
//                it->second.x = x;
//                it->second.y = y;
//                it->second.z = z;
//                it->second.w = w;
//                hasProgramUniforms ? glProgramUniform4f(program, location, x, y, z, w) : glUniform4f(location, x, y, z, w);
//            }
//        } else {
//            Vec4 value;
//            value.x = x;
//            value.y = y;
//            value.z = z;
//            value.w = w;
//            vec4Cache[key] = value;
//            hasProgramUniforms ? glProgramUniform4f(program, location, x, y, z, w) : glUniform4f(location, x, y, z, w);
//        }
//    } else {
//        hasProgramUniforms ? glProgramUniform4f(program, location, x, y, z, w) : glUniform4f(location, x, y, z, w);
//    }
//}
//
//void OpenGLCachedUniforms::SetUniformMat4(const Environment *env, GLint program, GLint location, const float* value)
//{
//    if (!value || location == -1) return;
//
//    if (uboBuffer) {
//
//        GLuint stride = 16;
//
//        for (int i = 0; i < 4; i++) {
//            GLuint offset = location + stride * i;
//            for (int j = 0; j < 4; j++) {
//                *((float *)(uboBuffer + offset)) = value[i * 4 + j];
//                offset += sizeof(GLfloat);
//            }
//        }
//        return;
//    }
//
//    if (cacheUniforms) {
//        UniformCacheKey key(env, program, location);
//
//        TRITON_MAP(UniformCacheKey, Mat4)::iterator it;
//        it = mat4Cache.find(key);
//        if (it != mat4Cache.end()) {
//
//            bool same = true;
//            for (int i = 0; i < 16; i++) {
//                if (value[i] != (it->second).mat[i]) {
//                    same = false;
//                    break;
//                }
//            }
//
//            if (!same) {
//                for (int i = 0; i < 16; i++) {
//                    it->second.mat[i] = value[i];
//                }
//                hasProgramUniforms ? glProgramUniformMatrix4fv(program, location, 1, GL_FALSE, value) : glUniformMatrix4fv(location, 1, GL_FALSE, value);
//            }
//        } else {
//            Mat4 mat;
//            for (int i = 0; i < 16; i++) {
//                mat.mat[i] = value[i];
//            }
//            mat4Cache[key] = mat;
//            hasProgramUniforms ? glProgramUniformMatrix4fv(program, location, 1, GL_FALSE, value) : glUniformMatrix4fv(location, 1, GL_FALSE, value);
//        }
//    } else {
//        hasProgramUniforms ? glProgramUniformMatrix4fv(program, location, 1, GL_FALSE, value) : glUniformMatrix4fv(location, 1, GL_FALSE, value);
//    }
//}
//
//void OpenGLCachedUniforms::SetUniformMat3(const Environment *env, GLint program, GLint location, const float* value)
//{
//    if (!value || location == -1) return;
//
//    if (uboBuffer) {
//
//        GLuint stride = 16;
//
//        for (int i = 0; i < 3; i++) {
//            GLuint offset = location + stride * i;
//            for (int j = 0; j < 3; j++) {
//                *((float *)(uboBuffer + offset)) = value[i * 4 + j];
//                offset += sizeof(GLfloat);
//            }
//        }
//        return;
//    }
//
//    if (cacheUniforms) {
//        UniformCacheKey key(env, program, location);
//
//        TRITON_MAP(UniformCacheKey, Mat3)::iterator it;
//        it = mat3Cache.find(key);
//        if (it != mat3Cache.end()) {
//
//            bool same = true;
//            for (int i = 0; i < 9; i++) {
//                if (value[i] != (it->second).mat[i]) {
//                    same = false;
//                    break;
//                }
//            }
//
//            if (!same) {
//                for (int i = 0; i < 9; i++) {
//                    it->second.mat[i] = value[i];
//                }
//                hasProgramUniforms ? glProgramUniformMatrix3fv(program, location, 1, GL_FALSE, value) : glUniformMatrix3fv(location, 1, GL_FALSE, value);
//            }
//        } else {
//            Mat3 mat;
//            for (int i = 0; i < 9; i++) {
//                mat.mat[i] = value[i];
//            }
//            mat3Cache[key] = mat;
//            hasProgramUniforms ? glProgramUniformMatrix3fv(program, location, 1, GL_FALSE, value) : glUniformMatrix3fv(location, 1, GL_FALSE, value);
//        }
//    } else {
//        hasProgramUniforms ? glProgramUniformMatrix3fv(program, location, 1, GL_FALSE, value) : glUniformMatrix3fv(location, 1, GL_FALSE, value);
//    }
//}
//
//}
