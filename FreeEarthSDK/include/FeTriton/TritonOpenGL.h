// Copyright (c) 2011-2015 Sundog Software LLC. All rights reserved worldwide.

#ifndef TRITON_OPENGL_H
#define TRITON_OPENGL_H

#if defined(WIN32) || defined(WIN64)
#include <windows.h>
#endif

#include <FeTriton/TritonCommon.h>
#include <osg/GL>

#ifdef __APPLE__
#define GL_GLEXT_LEGACY
#include <OpenGL/gl.h>
//#include <OpenGL/glu.h>
#else
#ifndef _WIN32
#define GL_GLEXT_LEGACY
#endif
#include <GL/gl.h>
#include <GL/glu.h>
#endif

typedef GLuint GLhandleARB;
typedef struct __GLsync *GLsync;

#include <map>

#ifndef GL_TEXTURE0
#define GL_TEXTURE0 0x84C0
#endif
#ifndef GL_TEXTURE1
#define GL_TEXTURE1 0x84C1
#endif
#ifndef GL_TEXTURE2
#define GL_TEXTURE2 0x84C2
#endif
#ifndef GL_TEXTURE3
#define GL_TEXTURE3 0x84C3
#endif
#ifndef GL_TEXTURE4
#define GL_TEXTURE4 0x84C4
#endif
#ifndef GL_TEXTURE5
#define GL_TEXTURE5 0x84C5
#endif
#ifndef GL_TEXTURE6
#define GL_TEXTURE6 0x84C6
#endif
#ifndef GL_TEXTURE7
#define GL_TEXTURE7 0x84C7
#endif
#ifndef GL_TEXTURE8
#define GL_TEXTURE8 0x84C8
#endif
#ifndef GL_TEXTURE9
#define GL_TEXTURE9 0x84C9
#endif
#ifndef GL_TEXTURE10
#define GL_TEXTURE10 0x84CA
#endif
#ifndef GL_TEXTURE11
#define GL_TEXTURE11 0x84CB
#endif

#ifndef GL_TEXTURE_LOD_BIAS
#define GL_TEXTURE_LOD_BIAS 0x8501
#endif
#ifndef GL_POINT_SIZE_MAX
#define GL_POINT_SIZE_MAX 0x8127
#endif
#ifndef GL_DYNAMIC_DRAW
#define GL_DYNAMIC_DRAW 0x88E8
#endif
#ifndef GL_WRITE_ONLY_ARB
#define GL_WRITE_ONLY_ARB 0x88B9
#endif

#ifndef GL_OBJECT_INFO_LOG_LENGTH_ARB
#define GL_OBJECT_INFO_LOG_LENGTH_ARB 0x8B84
#endif
#ifndef GL_ARRAY_BUFFER
#define GL_ARRAY_BUFFER 0x8892
#endif
#ifndef GL_ELEMENT_ARRAY_BUFFER
#define GL_ELEMENT_ARRAY_BUFFER 0x8893
#endif
#ifndef GL_STATIC_DRAW_ARB
#define GL_STATIC_DRAW_ARB 0x88E4
#endif
#ifndef GL_STATIC_DRAW
#define GL_STATIC_DRAW 0x88E4
#endif
#ifndef GL_RGBA32F
#define GL_RGBA32F 0x8814
#endif
#ifndef GL_GENERATE_MIPMAP
#define GL_GENERATE_MIPMAP 0x8191
#endif
#ifndef GL_READ_ONLY
#define GL_READ_ONLY 0x88B8
#endif
#ifndef GL_COMPRESSED_RED_RGTC1
#define GL_COMPRESSED_RED_RGTC1 0x8DBB
#endif
#ifndef GL_COMPRESSED_LUMINANCE_LATC1_EXT
#define GL_COMPRESSED_LUMINANCE_LATC1_EXT 0x8C70
#endif
#ifndef GL_LUMINANCE32F_ARB
#define GL_LUMINANCE32F_ARB 0x8818
#endif
#ifndef GL_PIXEL_PACK_BUFFER_ARB
#define GL_PIXEL_PACK_BUFFER_ARB 0x88EB
#endif
#ifndef GL_PIXEL_PACK_BUFFER_ARB
#define GL_PIXEL_PACK_BUFFER_ARB 0x88E1
#endif
#ifndef GL_STREAM_READ_ARB
#define GL_STREAM_READ_ARB 0x88E1
#endif
#ifndef GL_COMPRESSED_RGB
#define GL_COMPRESSED_RGB 0x84ED
#endif
#ifndef GL_COMPRESSED_RGBA
#define GL_COMPRESSED_RGBA 0x84EE
#endif
#ifndef GL_CLAMP_TO_EDGE
#define GL_CLAMP_TO_EDGE 0x812F
#endif
#ifndef GL_TEXTURE_CUBE_MAP_EXT
#define GL_TEXTURE_CUBE_MAP_EXT 0x8513
#endif
#ifndef GL_TEXTURE_FILTER_CONTROL
#define GL_TEXTURE_FILTER_CONTROL 0x8500
#endif
#ifndef GL_FRAMEBUFFER_BINDING
#define GL_FRAMEBUFFER_BINDING 0x8CA6
#endif
#ifndef GL_FRAMEBUFFER
#define GL_FRAMEBUFFER 0x8D40
#endif
#ifndef GL_DEPTH_ATTACHMENT
#define GL_DEPTH_ATTACHMENT 0x8D00
#endif
#ifndef GL_SAMPLE_ALPHA_TO_COVERAGE_ARB
#define GL_SAMPLE_ALPHA_TO_COVERAGE_ARB 0x809E
#endif
#ifndef GL_MAX_TEXTURE_UNITS
#define GL_MAX_TEXTURE_UNITS 0x84E2
#endif
#ifndef GL_TEXTURE_RECTANGLE_NV
#define GL_TEXTURE_RECTANGLE_NV 0x84F5
#endif
#ifndef GL_TEXTURE_3D
#define GL_TEXTURE_3D 0x806F
#endif
#ifndef GL_FOG_COORDINATE_ARRAY
#define GL_FOG_COORDINATE_ARRAY 0x8457
#endif
#ifndef GL_FOG_COORD_ARRAY
#define GL_FOG_COORD_ARRAY GL_FOG_COORDINATE_ARRAY
#endif
#ifndef GL_SECONDARY_COLOR_ARRAY
#define GL_SECONDARY_COLOR_ARRAY 0x845E
#endif
#ifndef GL_VERTEX_ATTRIB_ARRAY_UNIFIED_NV
#define GL_VERTEX_ATTRIB_ARRAY_UNIFIED_NV 0x8F1E
#endif

#ifdef GL_VERSION_1_4
#define NATIVE_1_4
#endif

#ifdef GL_VERSION_1_3
#define NATIVE_1_3
#endif

#ifdef GL_VERSION_2_0
#define NATIVE_2_0
#endif

#ifdef GL_VERSION_3_0
#define NATIVE_3_0
#endif

#ifdef GL_VERSION_3_1
#define NATIVE_3_1
#endif

#ifdef GL_VERSION_3_2
#define NATIVE_3_2
#endif

#ifdef GL_VERSION_4_1
#define NATIVE_4_1
#endif

#ifdef GL_VERSION_4_4
#define NATIVE_4_4
#endif


namespace Triton
{
	typedef void (GL_APIENTRY * PFNGLACTIVETEXTUREPROC) (GLenum texture);
	typedef void (GL_APIENTRY * PFNGLCLIENTACTIVETEXTUREPROC) (GLenum texture);
	typedef const GLubyte* (GL_APIENTRY* PFNGLGETSTRINGIPROC) (GLenum name, GLuint index);

	class OpenGLExtensionManager
	{
	public:
		static void LoadGLExtensions();
		static bool HasExtension(const char *extensionName,const Environment *env);
		static bool HasUBO(const Environment *env);
		static bool HasProgramUniform();
	};

	class UniformCacheKey
	{
	public:
		UniformCacheKey(const Environment *env, GLint prog, GLint loc) : environment(env), program(prog), location(loc) {
			sortKey = (prog << 16) | loc;
		}

		GLint program, location;
		const Environment *environment;
		unsigned int sortKey;

		bool operator < (const UniformCacheKey& key) const {
			if (environment < key.environment) return true;

			if (environment == key.environment) {
				return sortKey < key.sortKey;
			}

			return false;
		}

		bool operator == (const UniformCacheKey&  key) const {
			return (environment == key.environment) && (program == key.program) && (location == key.location);
		}

		bool operator != (const UniformCacheKey& key) const {
			return (environment != key.environment) || (program != key.program) || (location != key.location);
		}

	};

	class Vec2
	{
	public:
		float x, y;
	};

	class Vec3
	{
	public:
		float x, y, z;
	};

	class Vec4
	{
	public:
		float x, y, z, w;
	};

	class Mat4
	{
	public:
		float mat[16];
	};

	class Mat3
	{
	public:
		float mat[9];
	};

	class OpenGLCachedUniforms
	{
	public:
		static void SetUniformFloat(const Environment *env, GLint program, GLint location, float value);
		static void SetUniformVec2(const Environment *env, GLint program, GLint location, float x, float y);
		static void SetUniformVec3(const Environment *env, GLint program, GLint location, float x, float y, float z);
		static void SetUniformVec4(const Environment *env, GLint program, GLint location, float x, float y, float z, float w);
		static void SetUniformInt(const Environment *env, GLint program, GLint location, int value);
		static void SetUniformMat3(const Environment *env, GLint program, GLint location, const float *matrix);
		static void SetUniformMat4(const Environment *env, GLint program, GLint location, const float *matrix);
		static void ClearCaches();

		static void SetUBO(GLubyte *uboBuffer);
		static void ClearUBO();

		static TRITON_MAP(UniformCacheKey, float) floatCache;
		static TRITON_MAP(UniformCacheKey, Vec2) vec2Cache;
		static TRITON_MAP(UniformCacheKey, Vec3) vec3Cache;
		static TRITON_MAP(UniformCacheKey, Vec4) vec4Cache;
		static TRITON_MAP(UniformCacheKey, int) intCache;
		static TRITON_MAP(UniformCacheKey, Mat4) mat4Cache;
		static TRITON_MAP(UniformCacheKey, Mat3) mat3Cache;

		static GLubyte *uboBuffer;

		static bool cacheUniforms;
		static bool hasProgramUniforms;
	};

	#ifndef NATIVE_3_0
		extern PFNGLGETSTRINGIPROC glGetStringi;
	#endif

	#ifndef NATIVE_1_3
		extern PFNGLACTIVETEXTUREPROC glActiveTexture;
		extern PFNGLCLIENTACTIVETEXTUREPROC glClientActiveTexture;
	#endif
}

#endif



//// Copyright (c) 2011-2015 Sundog Software LLC. All rights reserved worldwide.
//
//#ifndef TRITON_OPENGL_H
//#define TRITON_OPENGL_H
//
//#if defined(WIN32) || defined(WIN64)
//#include <windows.h>
//#endif
//
////#include <GL/glew.h>
//#include <FeTriton/TritonCommon.h>
//
//#ifdef __APPLE__
//#define GL_GLEXT_LEGACY
//#include <OpenGL/gl.h>
////#include <OpenGL/glu.h>
//#else
//#ifndef _WIN32
//#define GL_GLEXT_LEGACY
//#endif
//#include <GL/gl.h>
//#include <GL/glu.h>
//#endif
//
//#ifdef GL_VERSION_1_4
//#define NATIVE_1_4
//#endif
//
//#ifdef GL_VERSION_1_3
//#define NATIVE_1_3
//#endif
//
//#ifdef GL_VERSION_2_0
//#define NATIVE_2_0
//#endif
//
//#ifdef GL_VERSION_3_0
//#define NATIVE_3_0
//#endif
//
//#ifdef GL_VERSION_3_1
//#define NATIVE_3_1
//#endif
//
//#ifdef GL_VERSION_3_2
//#define NATIVE_3_2
//#endif
//
//#ifdef GL_VERSION_4_1
//#define NATIVE_4_1
//#endif
//
//#ifdef GL_VERSION_4_4
//#define NATIVE_4_4
//#endif
//
//#include <map>
//
//namespace Triton
//{
//class OpenGLExtensionManager
//{
//public:
//    static void LoadGLExtensions();
//    static bool HasExtension(const char *extensionName);
//    static bool HasUBO();
//    static bool HasProgramUniform();
//};
//
//class UniformCacheKey
//{
//public:
//    UniformCacheKey(const Environment *env, GLint prog, GLint loc) : environment(env), program(prog), location(loc) {
//        sortKey = (prog << 16) | loc;
//    }
//
//    GLint program, location;
//    const Environment *environment;
//    unsigned int sortKey;
//
//    bool operator < (const UniformCacheKey& key) const {
//        if (environment < key.environment) return true;
//
//        if (environment == key.environment) {
//            return sortKey < key.sortKey;
//        }
//
//        return false;
//    }
//
//    bool operator == (const UniformCacheKey&  key) const {
//        return (environment == key.environment) && (program == key.program) && (location == key.location);
//    }
//
//    bool operator != (const UniformCacheKey& key) const {
//        return (environment != key.environment) || (program != key.program) || (location != key.location);
//    }
//
//};
//
//class Vec2
//{
//public:
//    float x, y;
//};
//
//class Vec3
//{
//public:
//    float x, y, z;
//};
//
//class Vec4
//{
//public:
//    float x, y, z, w;
//};
//
//class Mat4
//{
//public:
//    float mat[16];
//};
//
//class Mat3
//{
//public:
//    float mat[9];
//};
//
//class OpenGLCachedUniforms
//{
//public:
//    static void SetUniformFloat(const Environment *env, GLint program, GLint location, float value);
//    static void SetUniformVec2(const Environment *env, GLint program, GLint location, float x, float y);
//    static void SetUniformVec3(const Environment *env, GLint program, GLint location, float x, float y, float z);
//    static void SetUniformVec4(const Environment *env, GLint program, GLint location, float x, float y, float z, float w);
//    static void SetUniformInt(const Environment *env, GLint program, GLint location, int value);
//    static void SetUniformMat3(const Environment *env, GLint program, GLint location, const float *matrix);
//    static void SetUniformMat4(const Environment *env, GLint program, GLint location, const float *matrix);
//    static void ClearCaches();
//
//    static void SetUBO(GLubyte *uboBuffer);
//    static void ClearUBO();
//
//    static TRITON_MAP(UniformCacheKey, float) floatCache;
//    static TRITON_MAP(UniformCacheKey, Vec2) vec2Cache;
//    static TRITON_MAP(UniformCacheKey, Vec3) vec3Cache;
//    static TRITON_MAP(UniformCacheKey, Vec4) vec4Cache;
//    static TRITON_MAP(UniformCacheKey, int) intCache;
//    static TRITON_MAP(UniformCacheKey, Mat4) mat4Cache;
//    static TRITON_MAP(UniformCacheKey, Mat3) mat3Cache;
//
//    static GLubyte *uboBuffer;
//
//    static bool cacheUniforms;
//    static bool hasProgramUniforms;
//};
//
////extern PFNGLGETOBJECTPARAMETERIVARBPROC glGetObjectParameterivARB;
////extern PFNGLGETINFOLOGARBPROC glGetInfoLogARB;
//
//#ifndef NATIVE_4_4
//#ifndef GL_VERSION_4_4
//typedef void (APIENTRYP PFNGLBUFFERSTORAGEPROC) (GLenum target, GLsizeiptr size, const void *data, GLbitfield flags);
//#endif
//extern PFNGLBUFFERSTORAGEPROC glBufferStorage;
//#endif
//
//#ifndef NATIVE_4_1
//#ifndef GL_VERSION_4_1
//typedef void (APIENTRYP PFNGLPROGRAMUNIFORM1IPROC) (GLuint program, GLint location, GLint v0);
//typedef void (APIENTRYP PFNGLPROGRAMUNIFORM2FPROC) (GLuint program, GLint location, GLfloat v0, GLfloat v1);
//typedef void (APIENTRYP PFNGLPROGRAMUNIFORM1FPROC) (GLuint program, GLint location, GLfloat v0);
//typedef void (APIENTRYP PFNGLPROGRAMUNIFORM3FPROC) (GLuint program, GLint location, GLfloat v0, GLfloat v1, GLfloat v2);
//typedef void (APIENTRYP PFNGLPROGRAMUNIFORM4FPROC) (GLuint program, GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3);
//typedef void (APIENTRYP PFNGLPROGRAMUNIFORMMATRIX3FVPROC) (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
//typedef void (APIENTRYP PFNGLPROGRAMUNIFORMMATRIX4FVPROC) (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
//#endif
//extern PFNGLPROGRAMUNIFORM1IPROC glProgramUniform1i;
//extern PFNGLPROGRAMUNIFORM2FPROC glProgramUniform2f;
//extern PFNGLPROGRAMUNIFORM1FPROC glProgramUniform1f;
//extern PFNGLPROGRAMUNIFORM3FPROC glProgramUniform3f;
//extern PFNGLPROGRAMUNIFORM4FPROC glProgramUniform4f;
//extern PFNGLPROGRAMUNIFORMMATRIX4FVPROC glProgramUniformMatrix4fv;
//extern PFNGLPROGRAMUNIFORMMATRIX3FVPROC glProgramUniformMatrix3fv;
//#endif
//
//#ifndef NATIVE_3_2
//extern PFNGLGENFRAMEBUFFERSPROC glGenFramebuffers;
//extern PFNGLDELETEFRAMEBUFFERSPROC glDeleteFramebuffers;
//extern PFNGLFRAMEBUFFERTEXTURE2DPROC glFramebufferTexture2D;
//extern PFNGLBINDFRAMEBUFFERPROC glBindFramebuffer;
//extern PFNGLCHECKFRAMEBUFFERSTATUSPROC glCheckFramebufferStatus;
//extern PFNGLCLIENTWAITSYNCPROC glClientWaitSync;
//extern PFNGLFENCESYNCPROC glFenceSync;
//extern PFNGLDELETESYNCPROC glDeleteSync;
//#endif
//
//#ifndef NATIVE_3_1
//extern PFNGLGETUNIFORMBLOCKINDEXPROC glGetUniformBlockIndex;
//extern PFNGLUNIFORMBLOCKBINDINGPROC glUniformBlockBinding;
//extern PFNGLGETACTIVEUNIFORMBLOCKIVPROC glGetActiveUniformBlockiv;
//extern PFNGLGETACTIVEUNIFORMSIVPROC glGetActiveUniformsiv;
//extern PFNGLGETUNIFORMINDICESPROC glGetUniformIndices;
//#endif
//
//#ifndef NATIVE_3_0
//extern PFNGLGENERATEMIPMAPPROC glGenerateMipmap;
//extern PFNGLDELETEVERTEXARRAYSPROC glDeleteVertexArrays;
//extern PFNGLGENVERTEXARRAYSPROC glGenVertexArrays;
//extern PFNGLBINDVERTEXARRAYPROC glBindVertexArray;
//extern PFNGLGETSTRINGIPROC glGetStringi;
//extern PFNGLBINDBUFFERBASEPROC glBindBufferBase;
//#endif
//
//#ifndef NATIVE_1_3
//extern PFNGLACTIVETEXTUREPROC glActiveTexture;
//extern PFNGLCLIENTACTIVETEXTUREPROC glClientActiveTexture;
//#endif
//
//#ifndef NATIVE_1_4
//extern PFNGLPOINTPARAMETERFPROC glPointParameterf;
//extern PFNGLPOINTPARAMETERFVPROC glPointParameterfv;
//#endif
//
//#ifndef NATIVE_2_0
//extern PFNGLGETUNIFORMLOCATIONPROC glGetUniformLocation;
//extern PFNGLUNIFORM1IPROC glUniform1i;
//extern PFNGLUNIFORM2FPROC glUniform2f;
//extern PFNGLUSEPROGRAMPROC glUseProgram;
//extern PFNGLUNIFORM1FPROC glUniform1f;
//extern PFNGLDELETEBUFFERSPROC glDeleteBuffers;
//extern PFNGLDETACHSHADERPROC glDetachShader;
//extern PFNGLGETSHADERIVPROC glGetShaderiv;
//extern PFNGLGETSHADERINFOLOGPROC glGetShaderInfoLog;
//extern PFNGLGETPROGRAMINFOLOGPROC glGetProgramInfoLog;
//extern PFNGLCREATESHADERPROC glCreateShader;
//extern PFNGLCOMPILESHADERPROC glCompileShader;
//extern PFNGLSHADERSOURCEPROC glShaderSource;
//extern PFNGLCREATEPROGRAMPROC glCreateProgram;
//extern PFNGLATTACHSHADERPROC glAttachShader;
//extern PFNGLLINKPROGRAMPROC glLinkProgram;
//extern PFNGLGETPROGRAMIVPROC glGetProgramiv;
//extern PFNGLGETATTRIBLOCATIONPROC glGetAttribLocation;
//extern PFNGLUNIFORM3FPROC glUniform3f;
//extern PFNGLUNIFORM4FPROC glUniform4f;
//extern PFNGLUNIFORMMATRIX4FVPROC glUniformMatrix4fv;
//extern PFNGLUNIFORMMATRIX3FVPROC glUniformMatrix3fv;
//extern PFNGLBINDBUFFERPROC glBindBuffer;
//extern PFNGLVERTEXATTRIBPOINTERPROC glVertexAttribPointer;
//extern PFNGLENABLEVERTEXATTRIBARRAYPROC glEnableVertexAttribArray;
//extern PFNGLGENBUFFERSPROC glGenBuffers;
//extern PFNGLBUFFERDATAPROC glBufferData;
//extern PFNGLDELETESHADERPROC glDeleteShader;
//extern PFNGLDELETEPROGRAMPROC glDeleteProgram;
//extern PFNGLMAPBUFFERPROC glMapBuffer;
//extern PFNGLUNMAPBUFFERPROC glUnmapBuffer;
//extern PFNGLDISABLEVERTEXATTRIBARRAYPROC glDisableVertexAttribArray;
//extern PFNGLBUFFERSUBDATAPROC glBufferSubData;
//extern PFNGLMAPBUFFERRANGEPROC glMapBufferRange;
//extern PFNGLISSHADERPROC glIsShader;
//extern PFNGLDRAWBUFFERSPROC glDrawBuffers;
//#endif
//}
//
//#endif
