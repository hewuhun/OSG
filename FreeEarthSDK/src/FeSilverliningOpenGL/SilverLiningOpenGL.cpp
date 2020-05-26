// SilverLiningOpenGL.cpp : Defines the entry point for the DLL application.
//

#if defined(WIN32) || defined(WIN64)
#include <windows.h>
#endif

#include <FeSilverliningLib/MemAlloc.h>
#include <FeSilverliningOpenGL/SilverLiningOpenGL.h>

#include <stdlib.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stack>
#include <algorithm>
#include <map>

// Causes problems in some deferred setups:
#define USE_PBO_READBACK

#ifdef MAC
// Use the latest glext.h that we include
#define GL_GLEXT_LEGACY
#include <OpenGL/gl.h>
#ifdef GL_VERSION_1_3
#define NATIVE_1_3
#endif
#ifdef GL_VERSION_1_4
#define NATIVE_1_4
#endif
#include "glext.h"
#define NO_GLU
#else
#ifdef __unix__
#define GL_GLEXT_LEGACY
#endif
#include "GL/gl.h"
//#include "GL/glu.h"
#ifdef GL_VERSION_1_3
#define NATIVE_1_3
#endif
#ifdef GL_VERSION_1_4
#define NATIVE_1_4
#endif

#include <GL/glext.h>
#endif

#if defined(WIN32) || defined(WIN64)
#include <gl/wglext.h>
#elif defined(__unix)
#include "GL/glx.h"
#define wglGetProcAddress(A) glXGetProcAddress((GLubyte *)A)
#elif defined(MAC)
#include <dlfcn.h>
static void * MyGLGetProcAddress (const char *name)
{
    static void *glHandle = NULL;
    void **handlePtr;
    void *addr = NULL;

    handlePtr = &glHandle;
    if (NULL == *handlePtr)
        *handlePtr = dlopen("/System/Library/Frameworks/OpenGL.framework/OpenGL", RTLD_LAZY | RTLD_GLOBAL);
    if (NULL != *handlePtr)
        addr = dlsym(*handlePtr, name);

    return addr;
}
#define wglGetProcAddress(A) MyGLGetProcAddress(A)
#else
#error "Target platform not defined"
#endif

#include <FeSilverliningOpenGL/TextureRenderer.h>
#include <FeSilverliningOpenGL/TextureLoader.h>

//#define WIREFRAME

using namespace SilverLining;

class Shader : public MemObject
{
public:
    Shader(unsigned int pContext) : glShader(0), glFragShader(0), glProgram(0), context(pContext) {
    }

    GLhandleARB glShader, glFragShader, glProgram;

    unsigned int context;
    typedef SL_MAP( const char*, GLint ) TUniformMap;
    TUniformMap uniformMap;
};

static SL_VECTOR(Shader *) shaderList;

static TextureLoader texLoader;
static ResourceLoader *resourceLoader = NULL;
static Shader* activeShader = 0;
static bool isATI = false;
static bool enableDebugOutput = false;
static bool hasBindlessGraphics = false;
static bool cullClockWise = true;

const char* userDefinedVertString = 0;
const char* userDefinedFragString = 0;

int ExtensionSupported(const char *extension)
{
    const GLubyte *extensions = NULL;
    const GLubyte *start;
    GLubyte *where, *terminator;

    if (!extensions) {
        extensions = glGetString(GL_EXTENSIONS);
        if (!extensions) return 0;
    }
    start = extensions;
    for (;;) {
        where = (GLubyte *) strstr((const char *) start, extension);
        if (!where)
            break;
        terminator = where + strlen(extension);
        if (where == start || *(where - 1) == ' ') {
            if (*terminator == ' ' || *terminator == '\0') {
                return 1;
            }
        }
        start = terminator;
    }
    return 0;
}

void CheckError(int lineNum)
{
    if (enableDebugOutput) {
        GLenum errCode = glGetError();
        if (errCode != GL_NO_ERROR) {
            printf("GL error line %d: %s\n", lineNum, gluErrorString(errCode));
        }
    }
}

void ClearErrors()
{
    if (enableDebugOutput) {
        GLenum errCode = glGetError();
        while (errCode != GL_NO_ERROR) {
            errCode = glGetError();
        }
    }
}

bool QueryError()
{

    GLenum errCode = glGetError();
    if (errCode != GL_NO_ERROR) {
        return false;
    }

    return true;
}

#ifdef _WIN32
#ifndef SILVERLINING_STATIC_RENDERER_OPENGL
BOOL APIENTRY DllMain( HANDLE hModule,
                       DWORD ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    switch (ul_reason_for_call) {
    case DLL_PROCESS_ATTACH:
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}
#endif
#endif

class MatrixGL : public MemObject
{
public:
    double m[16];
};

#define NUMR_PBO 2
class Context : public MemObject
{
public:
    Context(unsigned int contextID) : context(contextID), hasProjection(false),
        hasModelview(false), hasTexture(false), inited(false), pboX(-1), pboY(-1) {
        for (int i = 0; i < NUMR_PBO; i++) m_pbos[i] = 0;
    }
    unsigned int context;
    MatrixGL mProjection;
    MatrixGL mModelview;
    MatrixGL mTexture;

    bool hasProjection, hasModelview, hasTexture;

    // PBO stuff for pixel read-back
    GLuint m_pbos[NUMR_PBO]; // PBO pool
    int vram2sys;        // index of PBO used to copy from vram to sysmem
    int gpu2vram;        // index of PBO used to copy framebuffer to vram
    bool inited;
    int pboX, pboY;

    SL_VECTOR(GLhandleARB) userShaderList;
};

static bool usingVB = false;
static bool usingPB = false;
static bool usingFramebuffers = false;
static bool usingNVPointSprite = false;
static bool usingFloatBuffers = false;
static bool usingBindlessGraphics = false;
static bool usingOcclusionQuery = false;
static bool usingGLSL = false;
static bool usingMultisample = false;
static bool usingDepthRangedNV = false;
static unsigned int context = 0;
static Context *contextObj = 0;

static TextureRenderer *textureRenderer = NULL;

static PFNGLGENBUFFERSARBPROC glGenBuffersARB = 0;
static PFNGLBINDBUFFERARBPROC glBindBufferARB = 0;
static PFNGLBUFFERDATAARBPROC glBufferDataARB = 0;
static PFNGLMAPBUFFERARBPROC glMapBufferARB = 0;
static PFNGLISBUFFERARBPROC glIsBufferARB = 0;
static PFNGLBUFFERSUBDATAARBPROC glBufferSubDataARB = 0;
static PFNGLGETBUFFERSUBDATAARBPROC glGetBufferSubDataARB = 0;
static PFNGLUNMAPBUFFERARBPROC glUnmapBufferARB = 0;
static PFNGLDELETEBUFFERSARBPROC glDeleteBuffersARB = 0;
static PFNGLPOINTPARAMETERINVPROC glPointParameteriNV = 0;
#ifndef NATIVE_1_3
static PFNGLACTIVETEXTUREARBPROC glActiveTexture = 0;
static PFNGLCLIENTACTIVETEXTUREARBPROC glClientActiveTexture = 0;
#endif
static PFNGLBUFFERADDRESSRANGENVPROC glBufferAddressRangeNV = 0;
static PFNGLVERTEXFORMATNVPROC glVertexFormatNV = 0;
static PFNGLCOLORFORMATNVPROC glColorFormatNV = 0;
static PFNGLTEXCOORDFORMATNVPROC glTexCoordFormatNV = 0;
static PFNGLGETBUFFERPARAMETERUI64VNVPROC glGetBufferParameterui64vNV = 0;
static PFNGLMAKEBUFFERRESIDENTNVPROC glMakeBufferResidentNV = 0;
static PFNGLMAKEBUFFERNONRESIDENTNVPROC glMakeBufferNonResidentNV = 0;
static PFNGLGENQUERIESARBPROC glGenQueriesARB = 0;
static PFNGLDELETEQUERIESARBPROC glDeleteQueriesARB = 0;
static PFNGLBEGINQUERYARBPROC glBeginQueryARB = 0;
static PFNGLENDQUERYARBPROC glEndQueryARB = 0;
static PFNGLGETQUERYOBJECTUIVARBPROC glGetQueryObjectuivARB = 0;
static PFNGLCLAMPCOLORARBPROC glClampColorARB = 0;
static PFNGLCREATESHADEROBJECTARBPROC glCreateShaderObjectARB = 0;
static PFNGLSHADERSOURCEARBPROC glShaderSourceARB = 0;
static PFNGLCOMPILESHADERARBPROC glCompileShaderARB = 0;
static PFNGLCREATEPROGRAMOBJECTARBPROC glCreateProgramObjectARB = 0;
static PFNGLATTACHOBJECTARBPROC glAttachObjectARB = 0;
static PFNGLLINKPROGRAMARBPROC glLinkProgramARB = 0;
static PFNGLUSEPROGRAMOBJECTARBPROC glUseProgramObjectARB = 0;
static PFNGLDETACHOBJECTARBPROC glDetachObjectARB = 0;
static PFNGLDELETEOBJECTARBPROC glDeleteObjectARB = 0;
static PFNGLGETUNIFORMLOCATIONARBPROC glGetUniformLocationARB = 0;
static PFNGLUNIFORM4FARBPROC glUniform4fARB = 0;
static PFNGLUNIFORMMATRIX4FVARBPROC glUniformMatrix4fvARB = 0;
static PFNGLGETOBJECTPARAMETERIVARBPROC glGetObjectParameterivARB = 0;
static PFNGLGETINFOLOGARBPROC glGetInfoLogARB = 0;
static PFNGLUNIFORM1IARBPROC glUniform1iARB = 0;
static PFNGLTEXIMAGE3DEXTPROC glTexImage3DEXT = 0;
static PFNGLTEXSUBIMAGE3DEXTPROC glTexSubImage3DEXT = 0;
static PFNGLGETHANDLEARBPROC glGetHandleARB = 0;
static PFNGLGENFRAMEBUFFERSPROC glGenFramebuffers = 0;
static PFNGLBINDFRAMEBUFFERPROC glBindFramebuffer = 0;
static PFNGLFRAMEBUFFERTEXTURE2DPROC glFramebufferTexture2D = 0;
static PFNGLGENERATEMIPMAPPROC glGenerateMipmap = 0;
static PFNGLDELETEFRAMEBUFFERSPROC glDeleteFramebuffers = 0;
static PFNGLCHECKFRAMEBUFFERSTATUSPROC glCheckFramebufferStatus = 0;
static PFNGLGENRENDERBUFFERSPROC glGenRenderbuffers = 0;
static PFNGLBINDRENDERBUFFERPROC glBindRenderbuffer = 0;
static PFNGLFRAMEBUFFERRENDERBUFFERPROC glFramebufferRenderbuffer = 0;
static PFNGLRENDERBUFFERSTORAGEPROC glRenderbufferStorage = 0;
static PFNGLDELETERENDERBUFFERSPROC glDeleteRenderbuffers = 0;
static PFNGLDEPTHRANGEDNVPROC glDepthRangedNV = 0;
#ifndef NATIVE_1_4
static PFNGLBLENDFUNCSEPARATEPROC glBlendFuncSeparate = 0;
#endif

static bool InitShaderSystem();

SILVERLININGDLL_API void DeviceLost(void *) {}
SILVERLININGDLL_API void DeviceReset(void *) {}

SILVERLININGDLL_API bool BackfaceCullClockwise(bool cullCW)
{
    cullClockWise = cullCW;
    return true;
}

SILVERLININGDLL_API void *SetEnvironment(bool rightHanded, void *env, SilverLining::ResourceLoader *pResourceLoader,
        SilverLining::Allocator *pAllocator, const char* userVertString, const char* userFragString)
{
    resourceLoader = pResourceLoader;
    Allocator::SetAllocator(pAllocator);

    // Set user define strings
    SetUserDefinedVertString( userVertString );
    SetUserDefinedFragString( userFragString );

    // Load extensions.
#if defined(WIN32) || defined(WIN64)
    HGLRC glContext = wglGetCurrentContext();
    if (glContext == NULL) return 0;
#endif

#ifndef NATIVE_1_3
    glActiveTexture = (PFNGLACTIVETEXTUREPROC)wglGetProcAddress("glActiveTexture");
    if (!glActiveTexture) return 0;

    glClientActiveTexture= (PFNGLCLIENTACTIVETEXTUREARBPROC) wglGetProcAddress("glClientActiveTextureARB");
#endif

    const GLubyte *str;
    str = glGetString(GL_VENDOR);
    isATI = strstr((const char *)str, "ATI") != 0;
    str = glGetString(GL_VERSION);
    //float vernum = (float)atof((const char *)str);

    // Older ATI drivers don't seem to like VBO's with my vertex format.
    // DrawElements crashes with texture coodinate arrays enabled; no errors
    // reported. I'm sure it's pointing to valid memory and isn't overflowing.
#if (_MSC_VER <= 1310 || (!defined(WIN32) && !defined(WIN64)))
    bool VBOSupportQuestionable = (isATI) || (getenv("SILVERLINING_NO_VBO") != NULL);
#else
    char *buf = 0;
    size_t bufSize = 0;
    bool VBOSupportQuestionable = false;
    if (_dupenv_s(&buf, &bufSize, "SILVERLINING_NO_VBO") == 0) {
        if (buf != NULL) {
            VBOSupportQuestionable = true;
            free(buf);
        }
    }
    if (isATI) {
        VBOSupportQuestionable = true;
    }
#endif

    //VBOSupportQuestionable = true;

    if (ExtensionSupported("GL_NV_depth_buffer_float")) {
        glDepthRangedNV = (PFNGLDEPTHRANGEDNVPROC)wglGetProcAddress("glDepthRangedNV");
        if (glDepthRangedNV) {
            usingDepthRangedNV = true;
        }
    }

    if (ExtensionSupported("GL_ARB_vertex_buffer_object") && !VBOSupportQuestionable) {
        usingVB = true;

        if (ExtensionSupported("GL_ARB_pixel_buffer_object")) {
            usingPB = true;
        }

        glGenBuffersARB = (PFNGLGENBUFFERSARBPROC)wglGetProcAddress("glGenBuffersARB");
        glBindBufferARB = (PFNGLBINDBUFFERARBPROC)wglGetProcAddress("glBindBufferARB");
        glBufferDataARB = (PFNGLBUFFERDATAARBPROC)wglGetProcAddress("glBufferDataARB");
        glBufferSubDataARB = (PFNGLBUFFERSUBDATAARBPROC)wglGetProcAddress("glBufferSubDataARB");
        glGetBufferSubDataARB = (PFNGLGETBUFFERSUBDATAARBPROC)wglGetProcAddress("glGetBufferSubDataARB");
        glMapBufferARB = (PFNGLMAPBUFFERARBPROC)wglGetProcAddress("glMapBufferARB");
        glUnmapBufferARB = (PFNGLUNMAPBUFFERARBPROC)wglGetProcAddress("glUnmapBufferARB");
        glDeleteBuffersARB = (PFNGLDELETEBUFFERSARBPROC)wglGetProcAddress("glDeleteBuffersARB");
        glIsBufferARB = (PFNGLISBUFFERARBPROC)wglGetProcAddress("glIsBufferARB");
        if (!(glGenBuffersARB && glBindBufferARB && glBufferDataARB && glMapBufferARB
                && glUnmapBufferARB && glDeleteBuffersARB && glBufferSubDataARB
                && glIsBufferARB)) {
            usingVB = usingPB = false;
        }
    }

    if (ExtensionSupported("GL_EXT_framebuffer_object")) {
        usingFramebuffers = true;

        glGenFramebuffers = (PFNGLGENFRAMEBUFFERSPROC)wglGetProcAddress("glGenFramebuffers");
        glBindFramebuffer = (PFNGLBINDFRAMEBUFFERPROC)wglGetProcAddress("glBindFramebuffer");
        glFramebufferTexture2D = (PFNGLFRAMEBUFFERTEXTURE2DPROC)wglGetProcAddress("glFramebufferTexture2D");
        glGenerateMipmap = (PFNGLGENERATEMIPMAPPROC)wglGetProcAddress("glGenerateMipmap");
        glDeleteFramebuffers = (PFNGLDELETEFRAMEBUFFERSPROC)wglGetProcAddress("glDeleteFramebuffers");
        glCheckFramebufferStatus = (PFNGLCHECKFRAMEBUFFERSTATUSPROC)wglGetProcAddress("glCheckFramebufferStatus");
        glGenRenderbuffers = (PFNGLGENRENDERBUFFERSPROC)wglGetProcAddress("glGenRenderbuffers");
        glBindRenderbuffer = (PFNGLBINDRENDERBUFFERPROC)wglGetProcAddress("glBindRenderbuffer");
        glRenderbufferStorage = (PFNGLRENDERBUFFERSTORAGEPROC)wglGetProcAddress("glRenderbufferStorage");
        glFramebufferRenderbuffer = (PFNGLFRAMEBUFFERRENDERBUFFERPROC)wglGetProcAddress("glFramebufferRenderbuffer");
        glDeleteRenderbuffers = (PFNGLDELETERENDERBUFFERSPROC)wglGetProcAddress("glDeleteRenderbuffers");

        if (!(glGenFramebuffers && glBindFramebuffer && glFramebufferTexture2D
                && glGenerateMipmap && glDeleteFramebuffers && glGenRenderbuffers
                && glRenderbufferStorage && glFramebufferRenderbuffer && glDeleteRenderbuffers)) {
            usingFramebuffers = false;
        }
    }

#if (_MSC_VER <= 1310 || (!defined(WIN32) && !defined(WIN64)))
    bool disableBindlessGraphics = (getenv("SILVERLINING_NO_BINDLESS") != NULL);
#else
    bool disableBindlessGraphics = false;
    if (_dupenv_s(&buf, &bufSize, "SILVERLINING_NO_BINDLESS") == 0) {
        if (buf != NULL) {
            disableBindlessGraphics = true;
            free(buf);
        }
    }
#endif

#if (_MSC_VER <= 1310 || (!defined(WIN32) && !defined(WIN64)))
    enableDebugOutput = (getenv("SILVERLINING_ENABLE_DEBUG") != NULL);
#else
    enableDebugOutput = false;
    if (_dupenv_s(&buf, &bufSize, "SILVERLINING_ENABLE_DEBUG") == 0) {
        if (buf != NULL) {
            enableDebugOutput = true;
            free(buf);
        }
    }
#endif

    if (enableDebugOutput) {
        printf("SilverLining OpenGL Debug Output Enabled.\n");
    }

    if (ExtensionSupported("GL_ARB_multisample")) {
        usingMultisample = true;
    }

    if (ExtensionSupported("GL_EXT_texture3D") && ExtensionSupported("GL_ARB_texture_non_power_of_two")) {
        glTexImage3DEXT = (PFNGLTEXIMAGE3DEXTPROC)wglGetProcAddress("glTexImage3DEXT");
        glTexSubImage3DEXT = (PFNGLTEXSUBIMAGE3DEXTPROC)wglGetProcAddress("glTexSubImage3DEXT");
    }

    if (ExtensionSupported("GL_NV_vertex_buffer_unified_memory") &&
            ExtensionSupported("GL_NV_shader_buffer_load")) {
        hasBindlessGraphics = true;
    }

    if (hasBindlessGraphics && usingVB && !disableBindlessGraphics) {
        usingBindlessGraphics = true;

        glBufferAddressRangeNV = (PFNGLBUFFERADDRESSRANGENVPROC)wglGetProcAddress("glBufferAddressRangeNV");
        glVertexFormatNV = (PFNGLVERTEXFORMATNVPROC)wglGetProcAddress("glVertexFormatNV");
        glColorFormatNV = (PFNGLCOLORFORMATNVPROC)wglGetProcAddress("glColorFormatNV");
        glTexCoordFormatNV = (PFNGLTEXCOORDFORMATNVPROC)wglGetProcAddress("glTexCoordFormatNV");
        if (!(glBufferAddressRangeNV && glVertexFormatNV && glColorFormatNV && glTexCoordFormatNV)) {
            usingBindlessGraphics = false;
        }

        glGetBufferParameterui64vNV = (PFNGLGETBUFFERPARAMETERUI64VNVPROC)wglGetProcAddress("glGetBufferParameterui64vNV");
        glMakeBufferResidentNV = (PFNGLMAKEBUFFERRESIDENTNVPROC)wglGetProcAddress("glMakeBufferResidentNV");
        glMakeBufferNonResidentNV = (PFNGLMAKEBUFFERNONRESIDENTNVPROC)wglGetProcAddress("glMakeBufferNonResidentNV");
        if (!(glGetBufferParameterui64vNV && glMakeBufferResidentNV && glMakeBufferNonResidentNV)) {
            usingBindlessGraphics = false;
        }

    }

    if (ExtensionSupported("GL_NV_point_sprite")) {
        usingNVPointSprite = true;

        glPointParameteriNV = (PFNGLPOINTPARAMETERINVPROC)wglGetProcAddress("glPointParameteriNV");
    }

    if (ExtensionSupported("GL_ATI_texture_float")) {
        usingFloatBuffers = true;
    }

    if (ExtensionSupported("GL_ARB_occlusion_query")) {
        glGenQueriesARB = (PFNGLGENQUERIESARBPROC)wglGetProcAddress("glGenQueriesARB");
        glDeleteQueriesARB = (PFNGLDELETEQUERIESARBPROC)wglGetProcAddress("glDeleteQueriesARB");
        glBeginQueryARB = (PFNGLBEGINQUERYARBPROC)wglGetProcAddress("glBeginQueryARB");
        glEndQueryARB = (PFNGLENDQUERYARBPROC)wglGetProcAddress("glEndQueryARB");
        glGetQueryObjectuivARB = (PFNGLGETQUERYOBJECTUIVARBPROC)wglGetProcAddress("glGetQueryObjectuivARB");

        usingOcclusionQuery = glGenQueriesARB && glDeleteQueriesARB && glBeginQueryARB && glEndQueryARB &&
                              glGetQueryObjectuivARB;
    }

    if (ExtensionSupported("GL_ARB_color_buffer_float")) {
        glClampColorARB = (PFNGLCLAMPCOLORARBPROC)wglGetProcAddress("glClampColorARB");
    }

#ifndef NATIVE_1_4
    glBlendFuncSeparate = (PFNGLBLENDFUNCSEPARATEPROC)wglGetProcAddress("glBlendFuncSeparate");
#endif

    bool disableGLSL = false;
    usingGLSL = false;
#if (_MSC_VER <= 1310 || (!defined(WIN32) && !defined(WIN64)))
    disableGLSL = (getenv("SILVERLINING_NO_GLSL") != NULL);
#else
    buf = 0;
    bufSize = 0;
    if (_dupenv_s(&buf, &bufSize, "SILVERLINING_NO_GLSL") == 0) {
        if (buf != NULL) {
            disableGLSL = true;
            usingGLSL = false;
            free(buf);
        }
    }
#endif

    if (!disableGLSL && ExtensionSupported("GL_ARB_fragment_shader") && ExtensionSupported("GL_ARB_vertex_shader") && ExtensionSupported("GL_ARB_shader_objects")) {
        glCreateShaderObjectARB = (PFNGLCREATESHADEROBJECTARBPROC)wglGetProcAddress("glCreateShaderObjectARB");
        glShaderSourceARB = (PFNGLSHADERSOURCEARBPROC)wglGetProcAddress("glShaderSourceARB");
        glCompileShaderARB = (PFNGLCOMPILESHADERARBPROC)wglGetProcAddress("glCompileShaderARB");
        glCreateProgramObjectARB = (PFNGLCREATEPROGRAMOBJECTARBPROC)wglGetProcAddress("glCreateProgramObjectARB");
        glAttachObjectARB = (PFNGLATTACHOBJECTARBPROC)wglGetProcAddress("glAttachObjectARB");
        glLinkProgramARB = (PFNGLLINKPROGRAMARBPROC)wglGetProcAddress("glLinkProgramARB");
        glUseProgramObjectARB = (PFNGLUSEPROGRAMOBJECTARBPROC)wglGetProcAddress("glUseProgramObjectARB");
        glDetachObjectARB = (PFNGLDETACHOBJECTARBPROC)wglGetProcAddress("glDetachObjectARB");
        glDeleteObjectARB = (PFNGLDELETEOBJECTARBPROC)wglGetProcAddress("glDeleteObjectARB");
        glGetUniformLocationARB = (PFNGLGETUNIFORMLOCATIONARBPROC)wglGetProcAddress("glGetUniformLocationARB");
        glUniform4fARB = (PFNGLUNIFORM4FARBPROC)wglGetProcAddress("glUniform4fARB");
        glUniformMatrix4fvARB = (PFNGLUNIFORMMATRIX4FVARBPROC)wglGetProcAddress("glUniformMatrix4fvARB");
        glGetObjectParameterivARB = (PFNGLGETOBJECTPARAMETERIVARBPROC)wglGetProcAddress("glGetObjectParameterivARB");
        glGetInfoLogARB = (PFNGLGETINFOLOGARBPROC)wglGetProcAddress("glGetInfoLogARB");
        glUniform1iARB = (PFNGLUNIFORM1IARBPROC)wglGetProcAddress("glUniform1iARB");
        glGetHandleARB = (PFNGLGETHANDLEARBPROC)wglGetProcAddress("glGetHandleARB");

        if (glCreateShaderObjectARB && glShaderSourceARB && glCompileShaderARB && glCreateProgramObjectARB &&
                glAttachObjectARB && glLinkProgramARB && glUseProgramObjectARB && glDetachObjectARB &&
                glDeleteObjectARB && glGetUniformLocationARB && glUniform4fARB && glUniformMatrix4fvARB &&
                glGetObjectParameterivARB && glGetInfoLogARB && glUniform1iARB) {
            usingGLSL = true;
        }
    }

    textureRenderer = TextureRendererFactory::MakeTextureRenderer();

    CheckError(__LINE__);

    InitShaderSystem();

    static unsigned int contextNum = 0;
    //context = 0;
    contextNum++;
    context = contextNum;

    Context *pContext = SL_NEW Context(context);

    contextObj = pContext;

    return (void*)pContext;
}

SILVERLININGDLL_API void SetContext(void *pContext)
{
    if (pContext) {
        context = ((Context *)pContext)->context;
    }
    contextObj = ((Context *)pContext);
    activeShader = 0;
}

SILVERLININGDLL_API void ClearScreen(const Color& c)
{
    glClearColor((GLclampf)c.r, (GLclampf)c.g, (GLclampf)c.b, (GLclampf)c.a);
    glClear(GL_COLOR_BUFFER_BIT);
    CheckError(__LINE__);
}

SILVERLININGDLL_API void ClearDepth()
{
#ifdef WIREFRAME
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
#else
    glClear(GL_DEPTH_BUFFER_BIT);
#endif
    CheckError(__LINE__);
}

class Matrix4x4 : public MemObject
{
public:
    Matrix4x4() {
    }

    Matrix4x4(GLdouble *mat) {
        for (int row = 0; row < 4; row++) {
            for (int col = 0; col < 4; col++) {
                m[row][col] = *mat++;
            }
        }
    }

    Matrix4x4 Transpose() {
        Matrix4x4 out;

        for (int row = 0; row < 4; row++) {
            for (int col = 0; col < 4; col++) {
                out.m[row][col] = m[col][row];
            }
        }

        return out;
    }

    Matrix4x4 operator * (const Matrix4x4 &mat) const {
        Matrix4x4 out;
        for (int row = 0; row < 4; row++) {
            for (int col = 0; col < 4; col++) {
                out.m[row][col] =
                    m[row][0] * mat.m[0][col] +
                    m[row][1] * mat.m[1][col] +
                    m[row][2] * mat.m[2][col] +
                    m[row][3] * mat.m[3][col];
            }
        }

        return out;
    }

    Matrix4x4 SILVERLINING_API Inverse() {
        const double epsilon = 1E-6;
        Matrix4x4 inverse;

        double a0 = m[0][0]*m[1][1] - m[0][1]*m[1][0];
        double a1 = m[0][0]*m[1][2] - m[0][2]*m[1][0];
        double a2 = m[0][0]*m[1][3] - m[0][3]*m[1][0];
        double a3 = m[0][1]*m[1][2] - m[0][2]*m[1][1];
        double a4 = m[0][1]*m[1][3] - m[0][3]*m[1][1];
        double a5 = m[0][2]*m[1][3] - m[0][3]*m[1][2];
        double b0 = m[2][0]*m[3][1] - m[2][1]*m[3][0];
        double b1 = m[2][0]*m[3][2] - m[2][2]*m[3][0];
        double b2 = m[2][0]*m[3][3] - m[2][3]*m[3][0];
        double b3 = m[2][1]*m[3][2] - m[2][2]*m[3][1];
        double b4 = m[2][1]*m[3][3] - m[2][3]*m[3][1];
        double b5 = m[2][2]*m[3][3] - m[2][3]*m[3][2];

        double det = a0*b5 - a1*b4 + a2*b3 + a3*b2 - a4*b1 + a5*b0;
        if (fabs(det) > epsilon) {
            inverse.m[0][0] = +m[1][1]*b5 - m[1][2]*b4 + m[1][3]*b3;
            inverse.m[1][0] = -m[1][0]*b5 + m[1][2]*b2 - m[1][3]*b1;
            inverse.m[2][0] = +m[1][0]*b4 - m[1][1]*b2 + m[1][3]*b0;
            inverse.m[3][0] = -m[1][0]*b3 + m[1][1]*b1 - m[1][2]*b0;
            inverse.m[0][1] = -m[0][1]*b5 + m[0][2]*b4 - m[0][3]*b3;
            inverse.m[1][1] = +m[0][0]*b5 - m[0][2]*b2 + m[0][3]*b1;
            inverse.m[2][1] = -m[0][0]*b4 + m[0][1]*b2 - m[0][3]*b0;
            inverse.m[3][1] = +m[0][0]*b3 - m[0][1]*b1 + m[0][2]*b0;
            inverse.m[0][2] = +m[3][1]*a5 - m[3][2]*a4 + m[3][3]*a3;
            inverse.m[1][2] = -m[3][0]*a5 + m[3][2]*a2 - m[3][3]*a1;
            inverse.m[2][2] = +m[3][0]*a4 - m[3][1]*a2 + m[3][3]*a0;
            inverse.m[3][2] = -m[3][0]*a3 + m[3][1]*a1 - m[3][2]*a0;
            inverse.m[0][3] = -m[2][1]*a5 + m[2][2]*a4 - m[2][3]*a3;
            inverse.m[1][3] = +m[2][0]*a5 - m[2][2]*a2 + m[2][3]*a1;
            inverse.m[2][3] = -m[2][0]*a4 + m[2][1]*a2 - m[2][3]*a0;
            inverse.m[3][3] = +m[2][0]*a3 - m[2][1]*a1 + m[2][2]*a0;

            double invDet = ((double)1)/det;
            inverse.m[0][0] *= invDet;
            inverse.m[0][1] *= invDet;
            inverse.m[0][2] *= invDet;
            inverse.m[0][3] *= invDet;
            inverse.m[1][0] *= invDet;
            inverse.m[1][1] *= invDet;
            inverse.m[1][2] *= invDet;
            inverse.m[1][3] *= invDet;
            inverse.m[2][0] *= invDet;
            inverse.m[2][1] *= invDet;
            inverse.m[2][2] *= invDet;
            inverse.m[2][3] *= invDet;
            inverse.m[3][0] *= invDet;
            inverse.m[3][1] *= invDet;
            inverse.m[3][2] *= invDet;
            inverse.m[3][3] *= invDet;

        }

        return inverse;
    }

    Vector3 GetRow(int row) const {
        Vector3 out;
        out.x = m[row][0];
        out.y = m[row][1];
        out.z = m[row][2];
        return out;
    }

    double operator () (int x, int y) const {
        return m[x][y];
    }

private:
    double m[4][4];
};

static bool                         GetFrustum(const Matrix4x4& m, SilverLining::Frustum& f)
{
    Vector3 m0 = m.GetRow(0);
    Vector3 m1 = m.GetRow(1);
    Vector3 m2 = m.GetRow(2);
    Vector3 m3 = m.GetRow(3);

    Vector3 n;
    double d;

    n = m3 + m0;
    d = m(3, 3) + m(0, 3);
    Plane left(n, d);

    n = m3 - m0;
    d = m(3, 3) - m(0, 3);
    Plane right(n, d);

    n = m3 + m1;
    d = m(3, 3) + m(1, 3);
    Plane bottom(n, d);

    n = m3 - m1;
    d = m(3, 3) - m(1, 3);
    Plane top(n, d);

    n = m3 + m2;
    d = m(3, 3) + m(2, 3);
    Plane pnear(n, d);

    n = m3 - m2;
    d = m(3, 3) - m(2, 3);
    Plane pfar(n, d);

    pnear.Normalize();
    pfar.Normalize();
    left.Normalize();
    right.Normalize();
    top.Normalize();
    bottom.Normalize();

    f.SetPlane(Frustum::PNEAR, pnear);
    f.SetPlane(Frustum::PBACK, pfar);
    f.SetPlane(Frustum::PLEFT, left);
    f.SetPlane(Frustum::PRIGHT, right);
    f.SetPlane(Frustum::PTOP, top);
    f.SetPlane(Frustum::PBOTTOM, bottom);

    return true;
}

SILVERLININGDLL_API bool             GetFrustumWorldSpace(SilverLining::Frustum& f)
{
    Matrix4x4 viewProj;

    if (contextObj && contextObj->hasModelview && contextObj->hasProjection) {
        Matrix4x4 proj(contextObj->mProjection.m);
        Matrix4x4 mv(contextObj->mModelview.m);

        viewProj = (proj.Transpose() * mv.Transpose());
    } else {
        GLdouble p[16], md[16];
        glGetDoublev(GL_PROJECTION_MATRIX, p);
        glGetDoublev(GL_MODELVIEW_MATRIX, md);

        Matrix4x4 proj(p);
        Matrix4x4 mv(md);

        viewProj = (proj.Transpose() * mv.Transpose());
    }

    GetFrustum(viewProj, f);

    return true;
}

SILVERLININGDLL_API bool            GetFrustumClipSpace(SilverLining::Frustum& f)
{
    if (contextObj && contextObj->hasProjection) {
        Matrix4x4 proj(contextObj->mProjection.m);

        GetFrustum(proj.Transpose(), f);
    } else {
        GLdouble p[16];
        glGetDoublev(GL_PROJECTION_MATRIX, p);

        Matrix4x4 proj(p);

        GetFrustum(proj.Transpose(), f);
    }

    return true;
}

static bool             InitShaderSystem()
{
    //bool vpInited = false, fpInited = false;

    if (glClampColorARB) {
        glClampColorARB(GL_CLAMP_VERTEX_COLOR_ARB, GL_FIXED_ONLY_ARB);
        glClampColorARB(GL_CLAMP_FRAGMENT_COLOR_ARB, GL_FIXED_ONLY_ARB);
        glClampColorARB(GL_CLAMP_READ_COLOR_ARB, GL_FIXED_ONLY_ARB);
    }

    return true;
}

SILVERLININGDLL_API ShaderHandle     LoadShaderFromSource(const char *shaderSource, int shaderType)
{
    return 0;
}


static void PrintGLSLInfoLog(GLhandleARB obj, const char *fname)
{
    if (enableDebugOutput) {
        int infologLength = 0;
        int charsWritten  = 0;
        char *infoLog;

        printf("GLSL error detected in %s!\n", fname);

        glGetObjectParameterivARB(obj, GL_OBJECT_INFO_LOG_LENGTH_ARB,
                                  &infologLength);

        if (infologLength > 0) {
            infoLog = (char *)SL_MALLOC(infologLength);
            glGetInfoLogARB(obj, infologLength, &charsWritten, infoLog);
            printf("%s\n",infoLog);
            SL_FREE(infoLog);
        }
    }
}

char *MakeFilename(const char *cgFileName, bool fragFile)
{
    // Strip the .cg extension and substitute our own...
    int len = (int)strlen(cgFileName);
    if (len < 3) return 0;

    int extraChars = fragFile ? 7 : 2;
    char *glslFileName = SL_NEW char[len + extraChars + 1];
    memset(glslFileName, 0, len + extraChars + 1);

#if (defined(WIN32) || defined(WIN64)) && (_MSC_VER > 1310)
    strncpy_s(glslFileName, len + extraChars + 1, cgFileName, len-3);
    if (fragFile) {
        strcat_s(glslFileName, len + extraChars + 1, "-frag.glsl");
    } else {
        strcat_s(glslFileName, len + extraChars + 1, ".glsl");
    }
#else
    strncpy(glslFileName, cgFileName, len-3);
    if (fragFile) {
        strcat(glslFileName, "-frag.glsl");
    } else {
        strcat(glslFileName, ".glsl");
    }
#endif

    return glslFileName;
}

void SetUserDefinedVertString( const char *userString )
{
    userDefinedVertString = userString;
}

void SetUserDefinedFragString( const char *userString )
{
    userDefinedFragString = userString;
}

const char * GetUserDefinedVertString()
{
    return userDefinedVertString;
}

const char * GetUserDefinedFragString()
{
    return userDefinedFragString;
}

GLhandleARB LoadShader(const char *glslFileName, int shaderType,
                       bool userShader = false)
{
    char *shaderSource;
    unsigned int dataLen;

    const char * userString = (shaderType == VERTEX_PROGRAM) ?
                              GetUserDefinedVertString() : GetUserDefinedFragString();

    const char *userDefines = ( userString && userShader ) ? userString : "";

    bool shaderLoaded = resourceLoader->LoadResource(glslFileName, shaderSource, dataLen, true);

    if (shaderLoaded) {
        GLhandleARB shaderObject=0;

        switch (shaderType) {
        case VERTEX_PROGRAM:
            shaderObject = glCreateShaderObjectARB(GL_VERTEX_SHADER_ARB);
            break;

        case FRAGMENT_PROGRAM:
            shaderObject = glCreateShaderObjectARB(GL_FRAGMENT_SHADER_ARB);
            break;
        }

        const char *sources[2] = { userDefines, shaderSource };
        glShaderSourceARB(shaderObject, 2, sources, NULL);

        glCompileShaderARB(shaderObject);

        resourceLoader->FreeResource(shaderSource);

        GLint ok;
        glGetObjectParameterivARB(shaderObject, GL_OBJECT_COMPILE_STATUS_ARB, &ok);
        if (!ok) {
            PrintGLSLInfoLog(shaderObject, glslFileName);
            return 0;
        } else {
            return shaderObject;
        }
    }

    return 0;
}

SILVERLININGDLL_API void SetUserShaders(const SL_VECTOR(unsigned int)& shaders)
{
    if (contextObj) {
        contextObj->userShaderList.clear();

        SL_VECTOR(unsigned int)::const_iterator iter;
        for (iter = shaders.begin(); iter != shaders.end(); iter++) {
#ifdef MAC
#if (__ENVIRONMENT_MAC_0S_X_VERSION_MIN_REQUIRED__ >= MAC_OS_X_VERSION_10_9)
            contextObj->userShaderList.push_back(reinterpret_cast<GLhandleARB>(*iter));
#else
            contextObj->userShaderList.push_back(*iter);
#endif
#else
            contextObj->userShaderList.push_back(*iter);
#endif
        }
    }
}

SILVERLININGDLL_API unsigned int GetShaderProgramObject(ShaderHandle shader)
{
    Shader *s = (Shader *)shader;
    if (s) {
#ifdef MAC
#if (__ENVIRONMENT_MAC_0S_X_VERSION_MIN_REQUIRED__ >= MAC_OS_X_VERSION_10_9)
        return reinterpret_cast<uintptr_t>(s->glProgram);
#else
        return s->glProgram;
#endif
#else
        return s->glProgram;
#endif
    }

    return 0;
}

SILVERLININGDLL_API ShaderHandle     LoadShaderFromFile(const char *fileName, int shaderType)
{
    if (usingGLSL) {
        // We automatically look for a companion -frag.glsl source file too, since GLSL doesn't have effect
        // files to keep it neat...
        char *baseFileName = MakeFilename(fileName, false);
        char *fragFileName = MakeFilename(fileName, true);

        GLhandleARB shaderObject = LoadShader(baseFileName, shaderType);
        GLhandleARB fragShaderObject = LoadShader(fragFileName, FRAGMENT_PROGRAM);

        GLhandleARB userVertObject = LoadShader("Shaders/UserFunctions.glsl", VERTEX_PROGRAM, true);
        GLhandleARB userFragObject = LoadShader("Shaders/UserFunctions-frag.glsl", FRAGMENT_PROGRAM, true);

        SL_DELETE[] baseFileName;
        SL_DELETE[] fragFileName;

        if (shaderObject) {
            GLhandleARB programObject = glCreateProgramObjectARB();
            glAttachObjectARB(programObject, shaderObject);

            if (fragShaderObject) {
                glAttachObjectARB(programObject, fragShaderObject);
            }

            if (userVertObject) {
                glAttachObjectARB(programObject, userVertObject);
            }

            if (userFragObject) {
                glAttachObjectARB(programObject, userFragObject);
            }

            if (contextObj) {
                SL_VECTOR(GLhandleARB)::const_iterator it;
                for (it = contextObj->userShaderList.begin(); it != contextObj->userShaderList.end(); it++) {
                    glAttachObjectARB(programObject, *it);
                }
            }

            glLinkProgramARB(programObject);

            GLint ok;
            glGetObjectParameterivARB(programObject, GL_OBJECT_LINK_STATUS_ARB, &ok);
            if (!ok) {
                PrintGLSLInfoLog(programObject, fileName);
            } else {
                GLint texLoc = glGetUniformLocationARB(programObject, "sl_tex2D");
                if (texLoc != -1) {
                    glUseProgramObjectARB(programObject);
                    glUniform1iARB(texLoc, 0);
                    glUseProgramObjectARB(0);
                    activeShader = 0;

                    texLoc = glGetUniformLocationARB(programObject, "sl_tex2D2");
                    if (texLoc != -1) {
                        glUseProgramObjectARB(programObject);
                        glUniform1iARB(texLoc, 1);
                        glUseProgramObjectARB(0);
                        activeShader = 0;
                    }
                } else {
                    // Stratocumulus shader uses 2 stages
                    GLint texLoc = glGetUniformLocationARB(programObject, "sl_tex3D");
                    if (texLoc != -1) {
                        glUseProgramObjectARB(programObject);
                        glUniform1iARB(texLoc, 0);
                        glUseProgramObjectARB(0);
                        activeShader = 0;
                    }
                    texLoc = glGetUniformLocationARB(programObject, "sl_tex3D2");
                    if (texLoc != -1) {
                        glUseProgramObjectARB(programObject);
                        glUniform1iARB(texLoc, 1);
                        glUseProgramObjectARB(0);
                        activeShader = 0;
                    }
                }

                Shader *s = SL_NEW Shader(context);
                s->glProgram = programObject;
                s->glShader = shaderObject;
                s->glFragShader = fragShaderObject;
                shaderList.push_back(s);

                CheckError(__LINE__);

                return s;
            }
        }
    }

    return 0;
}

SILVERLININGDLL_API bool SetConstantVector4(ShaderHandle shader, const char *varName, float *data)
{
    if (!shader) return false;

    Shader *s = (Shader *)shader;

    if (usingGLSL) {
        // check cached uniforms; if not found, fetch & cache
        GLint loc = -1;
        Shader::TUniformMap::iterator found = s->uniformMap.find(varName);
        if (found != s->uniformMap.end()) {
            loc = found->second;
        } else {
            loc = glGetUniformLocationARB(s->glProgram, varName);
            if (loc != -1) {
                s->uniformMap.insert(Shader::TUniformMap::value_type(varName, loc));
            }
        }

        if (loc != -1) {
            if (s != activeShader) {
                glUseProgramObjectARB(s->glProgram);
                activeShader = s;
            }
            glUniform4fARB(loc, data[0], data[1], data[2], data[3]);
            CheckError(__LINE__);
            return true;
        }
    }

    return false;
}

SILVERLININGDLL_API bool SetConstantMatrix4(ShaderHandle shader, const char *varName, float *data)
{
    if (!shader) return false;

    Shader *s = (Shader *)shader;

    if (usingGLSL) {
        // check cached uniforms; if not found, fetch & cache
        GLint loc = -1;
        Shader::TUniformMap::iterator found = s->uniformMap.find(varName);
        if (found != s->uniformMap.end()) {
            loc = found->second;
        } else {
            loc = glGetUniformLocationARB(s->glProgram, varName);
            if (loc != -1) {
                s->uniformMap.insert(Shader::TUniformMap::value_type(varName, loc));
            }
        }
        if (loc != -1) {
            if (activeShader != s) {
                glUseProgramObjectARB(s->glProgram);
                activeShader = s;
            }
            glUniformMatrix4fvARB(loc, 1, 1, data);
            CheckError(__LINE__);
            return true;
        }
    }

    return false;
}

SILVERLININGDLL_API bool BindShader(ShaderHandle shader, int shaderType)
{
    if (!shader) return false;

    Shader *s = (Shader *)shader;

    if (usingGLSL) {
        if (activeShader != s) {
            glUseProgramObjectARB(s->glProgram);
            activeShader = s;
            CheckError(__LINE__);
        }
        return true;
    }

    return false;
}

SILVERLININGDLL_API bool UnbindShader(int shaderType)
{
    if (usingGLSL) {
        activeShader = 0;
        glUseProgramObjectARB(0);
        CheckError(__LINE__);
        return true;
    }

    return false;
}

SILVERLININGDLL_API void DeleteShader(ShaderHandle shader)
{
    if (shader) {
        Shader *s = (Shader *)shader;
        if (s->context == context) {
            if (usingGLSL) {
                glDetachObjectARB(s->glProgram, s->glShader);
                if (s->glFragShader) {
                    glDetachObjectARB(s->glProgram, s->glFragShader);
                    glDeleteObjectARB(s->glFragShader);
                }
                glDeleteObjectARB(s->glShader);
                glDeleteObjectARB(s->glProgram);
            }

            SL_DELETE s;
        }

        shaderList.erase(std::remove(shaderList.begin(), shaderList.end(), s), shaderList.end());
    }

}

SILVERLININGDLL_API bool             ShutdownShaderSystem(void *pContext)
{

    SL_VECTOR(Shader*) ::iterator it;
    for (it = shaderList.begin(); it != shaderList.end(); ) {
        Shader *s = (Shader *)(*it);
        if (s->context == ((Context*)pContext)->context) {
            if (usingGLSL) {
                glDetachObjectARB(s->glProgram, s->glShader);
                if (s->glFragShader) {
                    glDetachObjectARB(s->glProgram, s->glFragShader);
                    glDeleteObjectARB(s->glFragShader);
                }
                glDeleteObjectARB(s->glShader);
                glDeleteObjectARB(s->glProgram);
            }

            SL_DELETE s;

            it = shaderList.erase(it);
        } else {
            it++;
        }
    }
    //shaderList.clear();
    activeShader = 0;

    Context *c = (Context*)pContext;
    if (c->inited) {
        glDeleteBuffersARB(NUMR_PBO, c->m_pbos);
    }

    SL_DELETE (Context *) pContext;

    return true;
}

class SL_Buffer : public MemObject
{
public:
    GLuint handle;
    GLuint64EXT addr;
    GLsizeiptr size;
};

class SL_Index_Buffer : public MemObject
{
public:
    GLuint handle;
    Index *buffer;
};

SILVERLININGDLL_API bool SetVertexBuffer(VertexBufferHandle vbh, bool vertexColors)
{
    // Vertex format is xyzwrgbuvst

    char *verts;
    SL_Buffer *vb = 0;

    if (usingVB) {
        vb = (SL_Buffer *)vbh;
        GLuint buffNum = (GLuint)(vb->handle);
        //assert (glIsBufferARB(buffNum) == GL_TRUE);
        glBindBufferARB(GL_ARRAY_BUFFER_ARB, buffNum);
        verts = NULL;
    } else {
        verts = (char *)vbh;
    }

    glEnableClientState(GL_VERTEX_ARRAY);

    if (vertexColors) {
        glEnableClientState(GL_COLOR_ARRAY);
    } else {
        glDisableClientState(GL_COLOR_ARRAY);
    }

    glEnableClientState(GL_TEXTURE_COORD_ARRAY);

    if (usingBindlessGraphics) {
        glEnableClientState(GL_VERTEX_ATTRIB_ARRAY_UNIFIED_NV);

        glBufferAddressRangeNV(GL_VERTEX_ARRAY_ADDRESS_NV,  0, vb->addr, vb->size);
        glBufferAddressRangeNV(GL_COLOR_ARRAY_ADDRESS_NV, 0, vb->addr + 4 * sizeof(float),
                               vb->size - (4 * sizeof(float)));
#ifdef FLOATING_POINT_COLOR
        glBufferAddressRangeNV(GL_TEXTURE_COORD_ARRAY_ADDRESS_NV, 0,
                               vb->addr + 4 * sizeof(float) + 4 * sizeof(float),
                               vb->size - (4 * sizeof(float) + 4 * sizeof(float)));
#else
        glBufferAddressRangeNV(GL_TEXTURE_COORD_ARRAY_ADDRESS_NV, 0,
                               vb->addr + 4 * sizeof(float) + 4 * sizeof(char),
                               vb->size - (4 * sizeof(float) + 4 * sizeof(char)));
#endif
    } else {
        glVertexPointer (4, GL_FLOAT, sizeof(Vertex), verts);
#ifdef FLOATING_POINT_COLOR
        glColorPointer(4, GL_FLOAT, sizeof(Vertex), verts + 4 * sizeof(float));
        glTexCoordPointer(4, GL_FLOAT, sizeof(Vertex), verts + 4 * sizeof(float) + 4 * sizeof(float));
#else
        glColorPointer (4, GL_UNSIGNED_BYTE, sizeof(Vertex), verts + 4 * sizeof(float));
        glTexCoordPointer(4, GL_FLOAT, sizeof(Vertex), verts + 4 * sizeof(float) + 4 * sizeof(char));
#endif
    }

    CheckError(__LINE__);
    return true;
}

SILVERLININGDLL_API bool UnsetVertexBuffer()
{
    // Vertex format is xyzrgbuvst

    if (usingVB) {
        glBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);
    }

    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_COLOR_ARRAY);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);

    if (usingBindlessGraphics) {
        glDisableClientState(GL_VERTEX_ATTRIB_ARRAY_UNIFIED_NV);
    }

    CheckError(__LINE__);
    return true;
}

SILVERLININGDLL_API bool            DrawStrip(IndexBufferHandle ibh, int startIdx, int nIndices, int nVerts)
{
    char *indices = 0;

    assert(ibh);

    SL_Index_Buffer *ib = (SL_Index_Buffer *)ibh;

    if (!usingVB) {
        indices = (char *)ib->buffer;
    } else {
        // assert (glIsBufferARB(ib->handle) == GL_TRUE);
    }

#ifdef WIREFRAME
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
#endif

    glDrawElements(GL_TRIANGLE_STRIP, nIndices,
                   sizeof(Index) == 2 ? GL_UNSIGNED_SHORT : GL_UNSIGNED_INT,
                   indices + startIdx * sizeof(Index));

    CheckError(__LINE__);
    return true;
}

SILVERLININGDLL_API bool             DrawPoints(double pointSize, int nPoints, int start)
{
    glEnable(GL_POINT_SMOOTH);
    glHint(GL_POINT_SMOOTH_HINT, GL_NICEST);

    if (usingNVPointSprite) {
        glDisable(GL_POINT_SPRITE_NV);
    }

    glPointSize((float)pointSize);
    glDrawArrays(GL_POINTS, start, nPoints);

    CheckError(__LINE__);
    return true;
}

SILVERLININGDLL_API bool            HasQuads()
{
    return true;
}

SILVERLININGDLL_API bool            DrawQuads(int nPoints, int start)
{
    glDrawArrays(GL_QUADS, start, nPoints);

    CheckError(__LINE__);
    return true;
}

SILVERLININGDLL_API void *           AllocateVertexBuffer(int nVerts)
{
    if (usingVB) {
        CheckError(__LINE__);

        SL_Buffer *buf = SL_NEW SL_Buffer;
        GLuint buffNum;
        glGenBuffersARB(1, &buffNum);
        buf->handle = buffNum;
        buf->size = nVerts * sizeof(Vertex);
        glBindBufferARB(GL_ARRAY_BUFFER_ARB, buffNum);
        glBufferDataARB(GL_ARRAY_BUFFER_ARB, nVerts * sizeof(Vertex), NULL, GL_DYNAMIC_DRAW_ARB);

        if (usingBindlessGraphics) {
            glGetBufferParameterui64vNV(GL_ARRAY_BUFFER, GL_BUFFER_GPU_ADDRESS_NV, &(buf->addr));
            glMakeBufferResidentNV(GL_ARRAY_BUFFER, GL_READ_ONLY);
#ifdef FLOATING_POINT_COLOR
            glColorFormatNV(4, GL_FLOAT, sizeof(Vertex));
#else
            glColorFormatNV(4, GL_UNSIGNED_BYTE, sizeof(Vertex));
#endif
            glVertexFormatNV(4, GL_FLOAT, sizeof(Vertex));
            glTexCoordFormatNV(4, GL_FLOAT, sizeof(Vertex));
        }
        CheckError(__LINE__);

        return (void *)buf;
    } else {
        return SL_NEW Vertex[nVerts];
    }
}

SILVERLININGDLL_API bool             LockVertexBuffer(void *buffer)
{
    return true;
}

SILVERLININGDLL_API bool GetVertexBuffer(void *buffer, int offset, Vertex *verts, int nVerts)
{
    if (usingVB) {
        SL_Buffer *buf = (SL_Buffer *)buffer;
        GLuint buffName = (GLuint)(buf->handle);
        glBindBufferARB(GL_ARRAY_BUFFER_ARB, buffName);

        glGetBufferSubDataARB(GL_ARRAY_BUFFER_ARB, offset * sizeof(Vertex), nVerts * sizeof(Vertex),
                              (void*)verts);
        CheckError(__LINE__);
    } else {
        Vertex *v = (Vertex *)buffer;
        for (int i = 0; i < nVerts; i++) {
            verts[i] = v[i+offset];
        }
    }

    return true;
}

SILVERLININGDLL_API bool UpdateVertexBuffer(void *buffer, int offset, Vertex *verts, int nVerts)
{
    if (usingVB) {
        SL_Buffer *buf = (SL_Buffer *)buffer;
        GLuint buffName = (GLuint)(buf->handle);
        glBindBufferARB(GL_ARRAY_BUFFER_ARB, buffName);
        glBufferSubDataARB(GL_ARRAY_BUFFER_ARB, offset * sizeof(Vertex), nVerts * sizeof(Vertex),
                           (void*)verts);
        glBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);
        CheckError(__LINE__);
    } else {
        Vertex *v = (Vertex *)buffer;
        for (int i = 0; i < nVerts; i++) {
            v[i+offset] = verts[i];
        }
    }

    return true;
}

SILVERLININGDLL_API SilverLining::Vertex * GetVertices(void *buffer)
{
    if (usingVB) {

        SL_Buffer *buf = (SL_Buffer *)buffer;
        GLuint buffName = (GLuint)(buf->handle);
        glBindBufferARB(GL_ARRAY_BUFFER_ARB, buffName);

        void *p = glMapBufferARB(GL_ARRAY_BUFFER_ARB, GL_WRITE_ONLY_ARB);
        CheckError(__LINE__);
        return (Vertex *)p;
    } else {
        return (Vertex *)buffer;
    }
}

SILVERLININGDLL_API bool             UnlockVertexBuffer(void *buffer)
{
    if (usingVB) {
        SL_Buffer *buf = (SL_Buffer *)buffer;
        GLuint buffName = (GLuint)(buf->handle);
        glBindBufferARB(GL_ARRAY_BUFFER_ARB, buffName);

        GLboolean ok = glUnmapBufferARB(GL_ARRAY_BUFFER_ARB);
#ifdef _DEBUG
        if (ok == GL_FALSE) {
            printf("UnlockVertexBuffer: glUnmapbufferARB failed!\n");
        }
#endif
        if (usingBindlessGraphics) {
            glGetBufferParameterui64vNV(GL_ARRAY_BUFFER, GL_BUFFER_GPU_ADDRESS_NV, &(buf->addr));
        }

        glBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);

        CheckError(__LINE__);

        return (ok == GL_TRUE);
    } else {
        return true;
    }
}

SILVERLININGDLL_API bool             ReleaseVertexBuffer(void *buffer)
{
    if (usingVB) {
        SL_Buffer *buf = (SL_Buffer *)buffer;
        GLuint buffName = (GLuint)(buf->handle);
        GLuint buffer[1] = {buffName};
        glDeleteBuffersARB(1, buffer);
        CheckError(__LINE__);
        SL_DELETE buf;
    } else {
        Vertex *verts = (Vertex *)buffer;
        SL_DELETE[] verts;
    }

    return true;
}

SILVERLININGDLL_API void *AllocateIndexBuffer(int nIndices)
{
    SL_Index_Buffer *ib = SL_NEW SL_Index_Buffer;
    assert(ib);
    ib->buffer = 0;
    ib->handle = 0;

    if (usingVB) {
        GLuint buffNum;
        glGenBuffersARB(1, &buffNum);
        glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, buffNum);
        glBufferDataARB(GL_ELEMENT_ARRAY_BUFFER_ARB, nIndices * sizeof(Index), NULL, GL_DYNAMIC_DRAW_ARB);
        CheckError(__LINE__);
        ib->handle = buffNum;
    } else {
        ib->buffer = SL_NEW Index[nIndices];
    }

    return (void *)ib;
}

SILVERLININGDLL_API bool LockIndexBuffer(IndexBufferHandle ibh)
{
    return true;
}

SILVERLININGDLL_API Index *GetIndices(IndexBufferHandle ibh)
{
    assert(ibh);

    SL_Index_Buffer *ib = (SL_Index_Buffer *)ibh;

    if (usingVB) {
        GLuint buffName = ib->handle;
        glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, buffName);
        void *p = glMapBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, GL_WRITE_ONLY_ARB);
        CheckError(__LINE__);
        return (Index *)p;
    } else {
        return ib->buffer;
    }
}

SILVERLININGDLL_API bool UnlockIndexBuffer(IndexBufferHandle ibh)
{
    assert(ibh);

    SL_Index_Buffer *ib = (SL_Index_Buffer *)ibh;

    if (usingVB) {
        GLuint buffName = ib->handle;
        glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, buffName);
        CheckError(__LINE__);
        GLboolean ok = glUnmapBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB);
#ifdef _DEBUG
        if (ok == GL_FALSE) {
            printf("UnlockIndexBuffer: glUnmapBufferARB failed.\n");
        }
#endif
        glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, 0);

        return (ok == GL_TRUE);
    } else {
        return true;
    }
}

SILVERLININGDLL_API bool ReleaseIndexBuffer(IndexBufferHandle ibh)
{
    assert(ibh);

    SL_Index_Buffer *ib = (SL_Index_Buffer *)ibh;

    if (usingVB) {
        GLuint buffName = ib->handle;
        GLuint buffer[1] = {buffName};
        glDeleteBuffersARB(1, buffer);
        CheckError(__LINE__);
    } else {
        SL_DELETE[] ib->buffer;
    }

    SL_DELETE ib;

    return true;
}

SILVERLININGDLL_API bool SetIndexBuffer(IndexBufferHandle ibh)
{
    assert(ibh);

    SL_Index_Buffer *ib = (SL_Index_Buffer *)ibh;

    if (usingVB) {
        GLuint buffNum = ib->handle;
        glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, buffNum);
    }

    CheckError(__LINE__);
    return true;
}

SILVERLININGDLL_API bool UnsetIndexBuffer()
{
    if (usingVB) {
        glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, 0);
    }

    CheckError(__LINE__);
    return true;
}

SILVERLININGDLL_API bool EnableDepthWrites(bool enable)
{
    glDepthMask(enable ? GL_TRUE : GL_FALSE);
    CheckError(__LINE__);
    return true;
}

SILVERLININGDLL_API bool EnableDepthReads(bool enable)
{
    if (enable) {
        glEnable(GL_DEPTH_TEST);
    } else {
        glDisable(GL_DEPTH_TEST);
    }
    CheckError(__LINE__);
    return true;
}

SILVERLININGDLL_API bool EnableTexture2D(bool enable)
{
    if (enable)
        glEnable(GL_TEXTURE_2D);
    else
        glDisable(GL_TEXTURE_2D);

    CheckError(__LINE__);
    return true;
}

SILVERLININGDLL_API bool EnableTexture3D(bool enable)
{
    if (enable)
        glEnable(GL_TEXTURE_3D);
    else
        glDisable(GL_TEXTURE_3D);

    CheckError(__LINE__);
    return true;
}

SILVERLININGDLL_API bool EnableBackfaceCulling(bool enable)
{
    if (enable)
        glEnable(GL_CULL_FACE);
    else
        glDisable(GL_CULL_FACE);

    CheckError(__LINE__);
    return true;
}

SILVERLININGDLL_API bool EnableFog(bool enable)
{
    if (enable)
        glEnable(GL_FOG);
    else
        glDisable(GL_FOG);

    CheckError(__LINE__);
    return true;
}

SILVERLININGDLL_API bool ConfigureFog(double density, double start, double end, const Color& c)
{
    glFogi(GL_FOG_MODE, GL_EXP);
    glFogf(GL_FOG_DENSITY, (GLfloat)density);
    float FogCol[3]= {c.r, c.g, c.b};
    glFogfv(GL_FOG_COLOR,FogCol);
    glFogf(GL_FOG_START, (GLfloat)start);
    glFogf(GL_FOG_END, (GLfloat)end);
    CheckError(__LINE__);
    return true;
}

SILVERLININGDLL_API bool EnableLighting(bool enable)
{
    if (enable)
        glEnable(GL_LIGHTING);
    else
        glDisable(GL_LIGHTING);

    CheckError(__LINE__);
    return true;
}

SILVERLININGDLL_API bool SetCurrentColor(const Color& c)
{
    glColor4f(c.r, c.g, c.b, c.a);

    CheckError(__LINE__);
    return true;
}

static void ConvertMatrix(const GLdouble *in, Matrix4 *out)
{
    for (int row = 0; row < 4; row++) {
        for (int col = 0; col < 4; col++) {
            out->elem[row][col] = in[col * 4 + row];
        }
    }
}

static void ConvertMatrix(const Matrix4 *in, GLdouble *out)
{
    for (int row = 0; row < 4; row++) {
        for (int col = 0; col < 4; col++) {
            out[col * 4 + row] = in->elem[row][col];
        }
    }
}

SILVERLININGDLL_API bool             GetProjectionMatrix(SilverLining::Matrix4 *m)
{
    GLdouble glmat[16];
    glGetDoublev(GL_PROJECTION_MATRIX, glmat);
    ConvertMatrix(glmat, m);

    CheckError(__LINE__);
    return true;
}

SILVERLININGDLL_API bool             SetProjectionMatrix(const SilverLining::Matrix4& m)
{
    GLdouble glmat[16];
    ConvertMatrix(&m, glmat);
    glMatrixMode(GL_PROJECTION);
    glLoadMatrixd(glmat);

    if (contextObj) {
        memcpy(contextObj->mProjection.m, glmat, 16 * sizeof(GLdouble));
        contextObj->hasProjection = true;
    }

    CheckError(__LINE__);

    return true;
}

SILVERLININGDLL_API bool             GetTextureMatrix(SilverLining::Matrix4 *m)
{
    GLdouble glmat[16];
    glGetDoublev(GL_TEXTURE_MATRIX, glmat);
    ConvertMatrix(glmat, m);

    CheckError(__LINE__);
    return true;
}

SILVERLININGDLL_API bool             SetTextureMatrix(const SilverLining::Matrix4& m)
{
    GLdouble glmat[16];
    ConvertMatrix(&m, glmat);
    glMatrixMode(GL_TEXTURE);
    glLoadMatrixd(glmat);

    if (contextObj) {
        memcpy(contextObj->mTexture.m, glmat, 16 * sizeof(GLdouble));
        contextObj->hasTexture = true;
    }

    CheckError(__LINE__);
    return true;
}

SILVERLININGDLL_API bool             GetModelviewMatrix(SilverLining::Matrix4 *m)
{
    GLdouble glmat[16];
    glGetDoublev(GL_MODELVIEW_MATRIX, glmat);
    ConvertMatrix(glmat, m);

    CheckError(__LINE__);
    return true;
}

SILVERLININGDLL_API bool             SetModelviewMatrix(const SilverLining::Matrix4& m)
{
    GLdouble glmat[16];
    ConvertMatrix(&m, glmat);
    glMatrixMode(GL_MODELVIEW);
    glLoadMatrixd(glmat);

    if (contextObj) {
        memcpy(contextObj->mModelview.m, glmat, 16 * sizeof(GLdouble));
        contextObj->hasModelview = true;
    }

    CheckError(__LINE__);
    return true;
}

SILVERLININGDLL_API bool             MultiplyModelviewMatrix(const SilverLining::Matrix4& m)
{
    GLdouble glmat[16];
    ConvertMatrix(&m, glmat);
    glMatrixMode(GL_MODELVIEW);
    glMultMatrixd(glmat);
    CheckError(__LINE__);
    return true;
}

SILVERLININGDLL_API bool             PushTransforms(void)
{
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    CheckError(__LINE__);
    return true;
}

SILVERLININGDLL_API bool             PopTransforms(void)
{
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
    CheckError(__LINE__);
    return true;
}

static std::stack<GLhandleARB> shaderStack;
static std::stack<GLuint> fboStack;

SILVERLININGDLL_API bool             PushAllState(void)
{
    ClearErrors();
    glPushClientAttrib(GL_CLIENT_VERTEX_ARRAY_BIT);
    glPushAttrib(GL_ALL_ATTRIB_BITS);

    if (glGetHandleARB) {
        GLhandleARB currentProgram = glGetHandleARB(GL_PROGRAM_OBJECT_ARB);
        shaderStack.push(currentProgram);
    }

    if (usingFramebuffers) {
        GLint iFbo = 0;
        glGetIntegerv(GL_FRAMEBUFFER_BINDING, &iFbo);
        fboStack.push(iFbo);
    }

    CheckError(__LINE__);
    return true;
}

SILVERLININGDLL_API bool             PopAllState(void)
{
    if (usingFramebuffers && !fboStack.empty()) {
        GLuint savedFbo = fboStack.top();

        glBindFramebuffer(GL_FRAMEBUFFER, savedFbo);

        fboStack.pop();
    }

    if (!shaderStack.empty()) {
        GLhandleARB savedProgram = shaderStack.top();
        if (savedProgram) {
            glUseProgramObjectARB(savedProgram);
        }
        shaderStack.pop();
    }

    glPopAttrib();
    glPopClientAttrib();
    CheckError(__LINE__);
    ClearErrors();
    return true;
}

SILVERLININGDLL_API bool            SetDefaultState(void)
{
    activeShader = 0;

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
    glEnable(GL_LIGHTING);
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
    glDisable(GL_STENCIL_TEST);
    glDisable(GL_TEXTURE_1D);
    glDisable(GL_TEXTURE_2D);
    glDisable(GL_TEXTURE_GEN_S);
    glDisable(GL_TEXTURE_GEN_T);
    glDisable(GL_TEXTURE_GEN_R);
    glDisable(GL_TEXTURE_GEN_Q);

    static GLint iMaxClipPlanes = 0;

    if (iMaxClipPlanes == 0) {
        glGetIntegerv(GL_MAX_CLIP_PLANES, &iMaxClipPlanes);
    }

    for (int i = 0; i < iMaxClipPlanes; i++) {
        glDisable(GL_CLIP_PLANE0 + i);
    }

    glFrontFace(cullClockWise ? GL_CCW : GL_CW);

    if (usingMultisample) {
        glDisable(GL_SAMPLE_ALPHA_TO_COVERAGE_ARB);
    }

    if (glBindBufferARB) {
        glBindBufferARB( GL_ELEMENT_ARRAY_BUFFER_ARB, 0 );
        glBindBufferARB(GL_ARRAY_BUFFER_ARB,0);
    }

    static GLint iMaxTextures = 0;
    if (iMaxTextures == 0) {
        glGetIntegerv(GL_MAX_TEXTURE_UNITS, &iMaxTextures);
    }

    if (iMaxTextures > 32) iMaxTextures = 32;

    for (int stage = 0; stage < iMaxTextures; stage++) {
        DisableTexture(stage);
    }

    glClientActiveTexture(GL_TEXTURE0_ARB);

    glDisableClientState(GL_NORMAL_ARRAY);
    glDisableClientState(GL_FOG_COORD_ARRAY);
    glDisableClientState(GL_INDEX_ARRAY);
    glDisableClientState(GL_EDGE_FLAG_ARRAY);
    glDisableClientState(GL_SECONDARY_COLOR_ARRAY);

    if (hasBindlessGraphics) {
        glDisableClientState(GL_VERTEX_ATTRIB_ARRAY_UNIFIED_NV);
    }

    if (usingBindlessGraphics) {
#ifdef FLOATING_POINT_COLOR
        glColorFormatNV(4, GL_FLOAT, sizeof(Vertex));
#else
        glColorFormatNV(4, GL_UNSIGNED_BYTE, sizeof(Vertex));
#endif
        glVertexFormatNV(4, GL_FLOAT, sizeof(Vertex));
        glTexCoordFormatNV(4, GL_FLOAT, sizeof(Vertex));
    }

    glDepthFunc(GL_LEQUAL);

    return true;
}

static GLenum ConvertBlendFactor(int f)
{
    GLenum r = GL_ZERO;

    switch (f) {
    case ZERO:
        r = GL_ZERO;
        break;

    case ONE:
        r = GL_ONE;
        break;

    case SRCCOLOR:
        r = GL_SRC_COLOR;
        break;

    case INVSRCCOLOR:
        r = GL_ONE_MINUS_SRC_COLOR;
        break;

    case SRCALPHA:
        r = GL_SRC_ALPHA;
        break;

    case INVSRCALPHA:
        r = GL_ONE_MINUS_SRC_ALPHA;
        break;

    case DSTCOLOR:
        r = GL_DST_COLOR;
        break;

    case INVDSTCOLOR:
        r = GL_ONE_MINUS_DST_COLOR;
        break;

    case DSTALPHA:
        r = GL_DST_ALPHA;
        break;

    case INVDSTALPHA:
        r = GL_ONE_MINUS_DST_ALPHA;
        break;

    case SRCALPHASAT:
        r = GL_SRC_ALPHA_SATURATE;
        break;
    }

    return r;
}

SILVERLININGDLL_API bool             EnableBlending(int srcFactor, int dstFactor)
{
    glEnable(GL_BLEND);

    if (glBlendFuncSeparate && srcFactor == SRCALPHA && dstFactor == INVSRCALPHA) {
        glBlendFuncSeparate(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA,GL_ONE,GL_ONE_MINUS_SRC_ALPHA);
    } else {
        glBlendFunc(ConvertBlendFactor(srcFactor), ConvertBlendFactor(dstFactor));
    }

    CheckError(__LINE__);
    return true;
}

SILVERLININGDLL_API bool             DisableBlending()
{
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_BLEND);

    CheckError(__LINE__);
    return true;
}

SILVERLININGDLL_API void *GetNativeTexture(TextureHandle tex)
{
    if (tex) {
        glTexture *texObj = (glTexture *)tex;
        return (void *)(texObj->TextureID);
    }

    return 0;
}

SILVERLININGDLL_API bool            LoadTextureFromFile(const char *imgPath, TextureHandle *tex, bool repeatU, bool repeatV)
{
    glTexture * texture = SL_NEW glTexture;

    texLoader.SetTextureFilter(txBilinear);
    int result = texLoader.LoadTextureFromDisk(imgPath, texture, resourceLoader);
    if (result == FALSE) {
        SL_DELETE texture;
        return false;
    }

    glBindTexture(GL_TEXTURE_2D, texture->TextureID);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, repeatU ? GL_REPEAT : GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, repeatV ? GL_REPEAT : GL_CLAMP_TO_EDGE);

    *tex = (TextureHandle)texture;

    CheckError(__LINE__);
    return true;
}

SILVERLININGDLL_API bool            HasFloatTextures()
{
    return usingFloatBuffers;
}

SILVERLININGDLL_API bool            LoadFloatTextureRGB(float *data, int width, int height, TextureHandle *texHandle)
{
    if (usingFloatBuffers) {
        glTexture *texture = SL_NEW glTexture;
        texture->Width = width;
        texture->Height = height;
        texture->Type = GL_RGB_FLOAT32_ATI;
        texture->Bpp = 128;

        GLuint texName = 0;
        glGenTextures(1, &texName);
        texture->TextureID = texName;
        glBindTexture(GL_TEXTURE_2D, texName);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, width, height, 0, GL_RGB, GL_FLOAT, data);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        *texHandle = (TextureHandle)texture;

        CheckError(__LINE__);
        return true;
    }

    return false;
}

SILVERLININGDLL_API bool            LoadFloatTexture(float *data, int width, int height, TextureHandle *texHandle)
{
    glTexture *texture = SL_NEW glTexture;
    texture->Width = width;
    texture->Height = height;
    texture->Type = GL_FLOAT;
    texture->Bpp = 64;

    GLuint texName = 0;
    glGenTextures(1, &texName);
    texture->TextureID = texName;
    glBindTexture(GL_TEXTURE_2D, texName);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
#ifdef NO_GLU
    glTexParameteri( GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE_ALPHA, width, height, 0, GL_LUMINANCE_ALPHA, GL_FLOAT, data);
#else
    gluBuild2DMipmaps(GL_TEXTURE_2D, GL_LUMINANCE_ALPHA, width, height, GL_LUMINANCE_ALPHA, GL_FLOAT, data);
#endif

    *texHandle = (TextureHandle)texture;

    CheckError(__LINE__);
    return true;
}

SILVERLININGDLL_API bool            LoadTexture(unsigned char *data, int width, int height, TextureHandle *texHandle,
        bool repeatU, bool repeatV)
{
    glTexture *texture = SL_NEW glTexture;
    texture->Width = width;
    texture->Height = height;
    texture->Type = GL_UNSIGNED_BYTE;
    texture->Bpp = 24;

    GLuint texName = 0;
    glGenTextures(1, &texName);
    texture->TextureID = texName;
    glBindTexture(GL_TEXTURE_2D, texName);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, repeatU ? GL_REPEAT : GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, repeatV ? GL_REPEAT : GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
#ifdef NO_GLU
    glTexParameteri( GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE8_ALPHA8, width, height, 0, GL_LUMINANCE_ALPHA, GL_UNSIGNED_BYTE, data);
#else
    gluBuild2DMipmaps(GL_TEXTURE_2D, GL_LUMINANCE_ALPHA, width, height, GL_LUMINANCE_ALPHA, GL_UNSIGNED_BYTE, data);
#endif

    *texHandle = (TextureHandle)texture;

    CheckError(__LINE__);
    return true;
}

SILVERLININGDLL_API bool            Load3DTexture(unsigned char *data, int width, int height, int depth, TextureHandle *texHandle,
        bool repeatU, bool repeatV, bool repeatR)
{
    if (!glTexImage3DEXT) return false;

    glTexture *texture = SL_NEW glTexture;
    texture->Width = width;
    texture->Height = height;
    texture->Type = GL_UNSIGNED_BYTE;
    texture->Bpp = 8;

    GLuint texName = 0;
    glGenTextures(1, &texName);
    texture->TextureID = texName;

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glBindTexture(GL_TEXTURE_3D, texName);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, repeatU ? GL_REPEAT : GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, repeatV ? GL_REPEAT : GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, repeatR ? GL_REPEAT : GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexImage3DEXT(GL_TEXTURE_3D, 0, GL_LUMINANCE, width, height, depth, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, data);

    *texHandle = (TextureHandle)texture;

    CheckError(__LINE__);
    return true;
}

SILVERLININGDLL_API bool            Load3DTextureRGB(unsigned char *data, int width, int height, int depth, TextureHandle *texHandle,
        bool repeatU, bool repeatV, bool repeatR)
{
    if (!glTexImage3DEXT) return false;

    glTexture *texture = SL_NEW glTexture;
    texture->Width = width;
    texture->Height = height;
    texture->Type = GL_UNSIGNED_BYTE;
    texture->Bpp = 16;

    GLuint texName = 0;
    glGenTextures(1, &texName);
    texture->TextureID = texName;

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glBindTexture(GL_TEXTURE_3D, texName);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, repeatU ? GL_REPEAT : GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, repeatV ? GL_REPEAT : GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, repeatR ? GL_REPEAT : GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    // Make sure error code is reset
    GLenum errCode = glGetError();
    glTexImage3DEXT(GL_TEXTURE_3D, 0, GL_COMPRESSED_RGB_S3TC_DXT1_EXT, width, height, depth, 0, GL_RGB, GL_UNSIGNED_BYTE, data);

    errCode = glGetError();
    if( errCode != GL_NO_ERROR ) // Error, try RGBA as a fallback
        glTexImage3DEXT(GL_TEXTURE_3D, 0, GL_RGBA, width, height, depth, 0, GL_RGB, GL_UNSIGNED_BYTE, data);

    *texHandle = (TextureHandle)texture;

    CheckError(__LINE__);
    return true;
}

SILVERLININGDLL_API bool            Load3DTextureLA(unsigned char *data, int width, int height, int depth, TextureHandle *texHandle,
        bool repeatU, bool repeatV, bool repeatR)
{
    if (!glTexImage3DEXT) return false;

    glTexture *texture = SL_NEW glTexture;
    texture->Width = width;
    texture->Height = height;
    texture->Type = GL_UNSIGNED_BYTE;
    texture->Bpp = 16;

    GLuint texName = 0;
    glGenTextures(1, &texName);
    texture->TextureID = texName;

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glBindTexture(GL_TEXTURE_3D, texName);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, repeatU ? GL_REPEAT : GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, repeatV ? GL_REPEAT : GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, repeatR ? GL_REPEAT : GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexImage3DEXT(GL_TEXTURE_3D, 0, GL_LUMINANCE_ALPHA, width, height, depth, 0, GL_LUMINANCE_ALPHA, GL_UNSIGNED_BYTE, data);

    *texHandle = (TextureHandle)texture;

    CheckError(__LINE__);
    return true;
}

SILVERLININGDLL_API bool          SubLoad3DTextureLA( unsigned char *data, int width, int height, int depth,
        int x, int y, int z, int rowPitch, int slicePitch,
        TextureHandle texHandle )
{
    if (!glTexSubImage3DEXT) return false;

    if( rowPitch < int( width * 2 * sizeof( unsigned char ) ) )
        rowPitch = int( width * 2 * sizeof( unsigned char ) ) ;

    if( slicePitch < int( height * width * 2 * sizeof( unsigned char ) ) )
        slicePitch = int( height * width * 2 * sizeof( unsigned char ) );

    glTexture *texture = (glTexture *)texHandle;

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glPixelStorei(GL_UNPACK_ROW_LENGTH, int( rowPitch / 2 * sizeof( unsigned char ) ) );
    glPixelStorei(GL_UNPACK_IMAGE_HEIGHT, int( slicePitch / rowPitch ) );

    glBindTexture(GL_TEXTURE_3D, texture->TextureID);
    glTexSubImage3DEXT(GL_TEXTURE_3D, 0, x, y, z, width, height, depth, GL_LUMINANCE_ALPHA, GL_UNSIGNED_BYTE, data);
    CheckError(__LINE__);

    // reset
    glPixelStorei(GL_UNPACK_IMAGE_HEIGHT, 0);
    glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);

    return true;
}

SILVERLININGDLL_API bool            EnableTexture(TextureHandle tex, int stage)
{
    glTexture *texture = (glTexture *)tex;
    if (!texture) return false;

    glActiveTexture(GL_TEXTURE0 + stage);

    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, texture->TextureID);
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

    CheckError(__LINE__);
    return true;
}

SILVERLININGDLL_API bool            Enable3DTexture(TextureHandle tex, int stage)
{
    glTexture *texture = (glTexture *)tex;

    glActiveTexture(GL_TEXTURE0 + stage);

    glEnable(GL_TEXTURE_3D);
    glBindTexture(GL_TEXTURE_3D, texture->TextureID);
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

    CheckError(__LINE__);
    return true;
}

SILVERLININGDLL_API bool            DisableTexture(int stage)
{
    glActiveTexture(GL_TEXTURE0 + stage);

    glDisable(GL_TEXTURE_RECTANGLE_NV);
    glDisable(GL_TEXTURE_2D);
    glDisable(GL_TEXTURE_3D);

    glActiveTexture(GL_TEXTURE0);

    CheckError(__LINE__);
    return true;
}

SILVERLININGDLL_API bool            ReleaseTexture(TextureHandle tex)
{
    glTexture *texture = (glTexture *)tex;
    texLoader.FreeTexture(texture);
    SL_DELETE texture;

    CheckError(__LINE__);
    return true;
}

SILVERLININGDLL_API bool            HasPointSprites(void)
{
    return usingNVPointSprite;
}

SILVERLININGDLL_API bool            EnablePointSprites(double pointSize)
{
    if (usingNVPointSprite) {
        glDisable(GL_POINT_SMOOTH);
        glEnable(GL_POINT_SPRITE_NV);
        glTexEnvi(GL_POINT_SPRITE_NV, GL_COORD_REPLACE_NV, GL_TRUE);
        glPointParameteriNV(GL_POINT_SPRITE_R_MODE_NV, GL_R);
        glPointSize((GLfloat)pointSize);
        CheckError(__LINE__);
        return true;
    }

    return false;
}

SILVERLININGDLL_API bool            DisablePointSprites()
{
    if (usingNVPointSprite) {
        glDisable(GL_POINT_SPRITE_NV);
        CheckError(__LINE__);
        return true;
    }

    return false;
}

SILVERLININGDLL_API bool            GetViewport(int& x, int& y, int& w, int& h)
{
    GLint vp[4];
    glGetIntegerv(GL_VIEWPORT, vp);

    x = vp[0];
    y = vp[1];
    w = vp[2];
    h = vp[3];

    CheckError(__LINE__);
    return true;
}

SILVERLININGDLL_API bool            GetDepthRange( float &zmin, float &zmax )
{

    GLfloat depthRange[2];
    glGetFloatv(GL_DEPTH_RANGE, depthRange);

    zmin = depthRange[0];
    zmax = depthRange[1];

    CheckError(__LINE__);


    // assume 0,1 to avoid stall
    //zmin = 0.0f;
    //zmax = 1.0f;

    return true;
}

SILVERLININGDLL_API bool            GetFOV(double &fov)
{
    double h;
    if (contextObj && contextObj->hasProjection) {
        h = contextObj->mProjection.m[5];
    } else {
        GLdouble mat[16];
        glGetDoublev(GL_PROJECTION_MATRIX, mat);
        h = mat[5];
    }
    fov = 2.0 * atan(1.0 / h);
    CheckError(__LINE__);
    return true;
}

SILVERLININGDLL_API bool InitRenderTarget(int w, int h, RenderTargetHandle *tgtHandle)
{
    return false;
}

SILVERLININGDLL_API bool InitRenderTexture(int w, int h, RenderTextureHandle *texHandle)
{
    TextureRenderer *tr = TextureRendererFactory::MakeTextureRenderer();
    if (tr) {
        if (tr->Initialize(w, h)) {
            *texHandle = (void *)tr;
            return true;
        }
    }

    CheckError(__LINE__);
    return false;
}

SILVERLININGDLL_API bool MakeRenderTargetCurrent(RenderTargetHandle tgtHandle)
{
    return false;
}

SILVERLININGDLL_API bool MakeRenderTextureCurrent(RenderTextureHandle texHandle, bool clear)
{
    TextureRenderer *tr = (TextureRenderer *)texHandle;
    if (tr) {
        return tr->MakeCurrent(clear);
    }

    CheckError(__LINE__);
    return false;

}

SILVERLININGDLL_API bool RestoreRenderTarget(RenderTargetHandle tgt)
{
    return false;
}

SILVERLININGDLL_API bool BindRenderTexture(RenderTextureHandle texHandle)
{
    TextureRenderer *tr = (TextureRenderer *)texHandle;
    if (tr) {
        return tr->BindToTexture();
    }

    CheckError(__LINE__);
    return false;
}

SILVERLININGDLL_API bool GetRenderTextureTextureHandle(RenderTextureHandle renTexHandle,
        TextureHandle *texHandle)
{
    TextureRenderer *tr = (TextureRenderer *)renTexHandle;
    if (tr) {
        *texHandle = (TextureHandle)(tr->GetTextureHandle());
        return true;
    }

    return false;
}

SILVERLININGDLL_API bool ReleaseRenderTarget(RenderTargetHandle tgt)
{
    return false;
}

SILVERLININGDLL_API bool ReleaseRenderTexture(RenderTextureHandle texHandle)
{
    TextureRenderer *tr = (TextureRenderer *)texHandle;

    if (tr) {
        SL_DELETE tr;
        return true;
    }

    return false;
}

struct CubeMapRenderTexture {
    GLuint buffer, depth_rb;
    glTexture texture;
    GLint savedFrameBuffer, savedRenderBuffer;
};

SILVERLININGDLL_API bool InitRenderTextureCube(int w, int h, SilverLining::RenderTextureHandle *texHandle, bool floatingPoint)
{
    if (usingFramebuffers) {
        GLuint color_tex;
        GLuint fb, depth_rb;

        GLint savedFrameBuffer, savedRenderBuffer;
        glGetIntegerv(GL_FRAMEBUFFER_BINDING, &savedFrameBuffer);
        glGetIntegerv(GL_RENDERBUFFER_BINDING, &savedRenderBuffer);

        //RGBA8 Cubemap texture, 24 bit depth texture
        glGenTextures(1, &color_tex);
        glBindTexture(GL_TEXTURE_CUBE_MAP, color_tex);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_BASE_LEVEL, 0);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAX_LEVEL, 0);

        CheckError(__LINE__);

        //NULL means reserve texture memory, but texels are undefined
        if (floatingPoint) {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + 0, 0, GL_RGBA16F, w, h, 0, GL_BGRA, GL_FLOAT, NULL);
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + 1, 0, GL_RGBA16F, w, h, 0, GL_BGRA, GL_FLOAT, NULL);
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + 2, 0, GL_RGBA16F, w, h, 0, GL_BGRA, GL_FLOAT, NULL);
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + 3, 0, GL_RGBA16F, w, h, 0, GL_BGRA, GL_FLOAT, NULL);
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + 4, 0, GL_RGBA16F, w, h, 0, GL_BGRA, GL_FLOAT, NULL);
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + 5, 0, GL_RGBA16F, w, h, 0, GL_BGRA, GL_FLOAT, NULL);
        } else {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + 0, 0, GL_RGBA8, w, h, 0, GL_BGRA, GL_UNSIGNED_BYTE, NULL);
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + 1, 0, GL_RGBA8, w, h, 0, GL_BGRA, GL_UNSIGNED_BYTE, NULL);
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + 2, 0, GL_RGBA8, w, h, 0, GL_BGRA, GL_UNSIGNED_BYTE, NULL);
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + 3, 0, GL_RGBA8, w, h, 0, GL_BGRA, GL_UNSIGNED_BYTE, NULL);
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + 4, 0, GL_RGBA8, w, h, 0, GL_BGRA, GL_UNSIGNED_BYTE, NULL);
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + 5, 0, GL_RGBA8, w, h, 0, GL_BGRA, GL_UNSIGNED_BYTE, NULL);
        }
        //-------------------------
        glGenFramebuffers(1, &fb);
        glBindFramebuffer(GL_FRAMEBUFFER, fb);
        //Attach one of the faces of the Cubemap texture to this FBO
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X, color_tex, 0);
        //-------------------------
        glGenRenderbuffers(1, &depth_rb);
        glBindRenderbuffer(GL_RENDERBUFFER, depth_rb);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, w, h);
        //-------------------------
        //Attach depth buffer to FBO
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depth_rb);

        glBindFramebuffer(GL_FRAMEBUFFER, savedFrameBuffer);
        glBindRenderbuffer(GL_RENDERBUFFER, savedRenderBuffer);

        CubeMapRenderTexture *cubeMap = SL_NEW CubeMapRenderTexture;
        cubeMap->texture.TextureID = color_tex;
        cubeMap->texture.Width = w;
        cubeMap->texture.Height = h;
        cubeMap->texture.Bpp = 32;
        cubeMap->texture.Type = GL_RGBA;
        cubeMap->buffer = fb;
        cubeMap->depth_rb = depth_rb;
        cubeMap->savedFrameBuffer = savedFrameBuffer;
        cubeMap->savedRenderBuffer = savedRenderBuffer;

        *texHandle = (RenderTextureHandle *)cubeMap;

        return true;
    }

    return false;
}

SILVERLININGDLL_API bool MakeRenderTextureCubeCurrent(SilverLining::RenderTextureHandle texHandle, bool clear, CubeFace face)
{
    if (usingFramebuffers && texHandle) {
        CubeMapRenderTexture *cubeMap = (CubeMapRenderTexture *)texHandle;

        glGetIntegerv(GL_FRAMEBUFFER_BINDING, &(cubeMap->savedFrameBuffer));
        glGetIntegerv(GL_RENDERBUFFER_BINDING, &(cubeMap->savedRenderBuffer));

        GLuint glFace = GL_TEXTURE_CUBE_MAP_POSITIVE_X;
        switch (face) {
        case POSX:
            glFace = GL_TEXTURE_CUBE_MAP_POSITIVE_X;
            break;

        case NEGX:
            glFace = GL_TEXTURE_CUBE_MAP_NEGATIVE_X;
            break;

        case POSY:
            glFace = GL_TEXTURE_CUBE_MAP_POSITIVE_Y;
            break;

        case NEGY:
            glFace = GL_TEXTURE_CUBE_MAP_NEGATIVE_Y;
            break;

        case POSZ:
            glFace = GL_TEXTURE_CUBE_MAP_POSITIVE_Z;
            break;

        case NEGZ:
            glFace = GL_TEXTURE_CUBE_MAP_NEGATIVE_Z;
            break;
        }

        glBindFramebuffer(GL_FRAMEBUFFER, cubeMap->buffer);
        glBindRenderbuffer(GL_RENDERBUFFER, cubeMap->depth_rb);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, glFace, cubeMap->texture.TextureID, 0);

        CheckError(__LINE__);

        return true;
    }
    return false;
}

SILVERLININGDLL_API bool BindRenderTextureCube(SilverLining::RenderTextureHandle texHandle)
{
    if (usingFramebuffers && texHandle) {
        CubeMapRenderTexture *cubeMap = (CubeMapRenderTexture *)texHandle;
        glBindFramebuffer(GL_FRAMEBUFFER, cubeMap->savedFrameBuffer);
        glBindRenderbuffer(GL_RENDERBUFFER, cubeMap->savedRenderBuffer);
        return true;
    }
    return false;
}

SILVERLININGDLL_API bool GetRenderTextureCubeTextureHandle(SilverLining::RenderTextureHandle renTexHandle, SilverLining::TextureHandle *texHandle)
{
    if (usingFramebuffers && renTexHandle && texHandle) {
        CubeMapRenderTexture *cubeMap = (CubeMapRenderTexture *)renTexHandle;
        *texHandle = (SilverLining::TextureHandle)(&cubeMap->texture);
        return true;
    }
    return false;
}

SILVERLININGDLL_API bool ReleaseRenderTextureCube(SilverLining::RenderTextureHandle texHandle)
{
    if (usingFramebuffers && texHandle) {
        CubeMapRenderTexture *cubeMap = (CubeMapRenderTexture *)texHandle;
        glDeleteRenderbuffers(1, &cubeMap->depth_rb);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glBindRenderbuffer(GL_RENDERBUFFER, 0);
        glDeleteFramebuffers(1, &cubeMap->buffer);
        glDeleteTextures(1, &cubeMap->texture.TextureID);
        SL_DELETE cubeMap;
        return true;
    }
    return false;
}

#ifdef NO_GLU
static void MyOrtho2D(float* mat, float left, float right, float bottom, float top)
{
    // this is basically from
    // http://en.wikipedia.org/wiki/Orthographic_projection_(geometry)
    const float zNear = -1.0f;
    const float zFar = 1.0f;
    const float inv_z = 1.0f / (zFar - zNear);
    const float inv_y = 1.0f / (top - bottom);
    const float inv_x = 1.0f / (right - left);

    //first column
    *mat++ = (2.0f*inv_x);
    *mat++ = (0.0f);
    *mat++ = (0.0f);
    *mat++ = (0.0f);

    //second
    *mat++ = (0.0f);
    *mat++ = (2.0f*inv_y);
    *mat++ = (0.0f);
    *mat++ = (0.0f);

    //third
    *mat++ = (0.0f);
    *mat++ = (0.0f);
    *mat++ = (-2.0f*inv_z);
    *mat++ = (0.0f);

    //fourth
    *mat++ = (-(right + left)*inv_x);
    *mat++ = (-(top + bottom)*inv_y);
    *mat++ = (-(zFar + zNear)*inv_z);
    *mat++ = (1.0f);
}
#endif

static void InitReadback()
{
    if (contextObj) {
        if (!contextObj->inited) {
            contextObj->inited = true;

            if (contextObj->m_pbos[0] == 0)
                glGenBuffersARB(NUMR_PBO, contextObj->m_pbos);

            for (int i = 0; i < NUMR_PBO; i++) {
                glBindBufferARB(GL_PIXEL_PACK_BUFFER_ARB, contextObj->m_pbos[i]);
                glBufferDataARB(GL_PIXEL_PACK_BUFFER_ARB, 4, NULL, GL_STATIC_READ);
            }

            glBindBufferARB(GL_PIXEL_PACK_BUFFER_ARB, 0);
            contextObj->vram2sys = 0;
            contextObj->gpu2vram = NUMR_PBO - 1;
        }
    }
}

SILVERLININGDLL_API bool GetPixels(int x, int y, int w, int h, void *pixels, bool immediate)
{
    bool newQuery = true;

#ifdef USE_PBO_READBACK
    // All runtime queries are for a single pixel, so we'll optimize especially for
    // that case. We use a one-pixel pixel buffer object with a one frame delay
    // to avoid stalling.
    if (usingPB && !immediate && contextObj && w == 1 && h == 1) {
        InitReadback();

        // If this is for a new location, we will kick this off but still return
        // the immediate, non-delayed result.
        //if (x == contextObj->pboX && y == contextObj->pboY) {
        newQuery = false;
        //}

        contextObj->pboX = x;
        contextObj->pboY = y;

        glBindBufferARB(GL_PIXEL_PACK_BUFFER_ARB, contextObj->m_pbos[contextObj->gpu2vram]);
        glReadPixels(x, y, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, 0);

        if (!newQuery) {
            // then copy previous frame from vram to sysmem (membuffer)
            glBindBufferARB(GL_PIXEL_PACK_BUFFER_ARB, contextObj->m_pbos[contextObj->vram2sys]);

            void* data = glMapBufferARB(GL_PIXEL_PACK_BUFFER_ARB, GL_READ_ONLY);
            if (data != NULL) {
                unsigned char *src = (unsigned char *)data;
                unsigned char *dst = (unsigned char *)pixels;
                *dst++ = *src++;
                *dst++ = *src++;
                *dst++ = *src++;
                *dst++ = *src++;
            }

            glUnmapBufferARB(GL_PIXEL_PACK_BUFFER_ARB);
        }

        // unbind PBO
        glBindBufferARB(GL_PIXEL_PACK_BUFFER_ARB, 0);

        // shift names
        GLuint temp = contextObj->m_pbos[0];
        for (int i = 1; i<NUMR_PBO; i++)
            contextObj->m_pbos[i - 1] = contextObj->m_pbos[i];
        contextObj->m_pbos[NUMR_PBO - 1] = temp;

    }
#endif

    if (newQuery) {
        glMatrixMode(GL_PROJECTION);
        glPushMatrix();
#ifdef NO_GLU
        float mat[16];
        MyOrtho2D(mat, 0, w, 0, h);
        glLoadMatrixf(mat);
#else
        glLoadIdentity();
        gluOrtho2D(0, w, 0, h);
#endif
        glMatrixMode(GL_MODELVIEW);
        glPushMatrix();
        glLoadIdentity();

        //glRasterPos2i(0,0);
        //glReadBuffer(GL_BACK);
        glReadPixels(x, y, w, h, GL_RGBA, GL_UNSIGNED_BYTE, pixels);

        glPopMatrix();
        glMatrixMode(GL_PROJECTION);
        glPopMatrix();
        glMatrixMode(GL_MODELVIEW);
    }

    CheckError(__LINE__);

    return true;
}

SILVERLININGDLL_API bool SetPixels(int x, int y, int w, int h, void *pixels)
{
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
#ifdef NO_GLU
    float mat[16];
    MyOrtho2D(mat, 0, w, 0, h);
    glLoadMatrixf(mat);
#else
    glLoadIdentity();
    gluOrtho2D(0, w, 0, h);
#endif
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    glPushAttrib(GL_DEPTH_BUFFER_BIT);
    glDisable(GL_DEPTH_TEST);
    glDepthMask(GL_FALSE);
    glRasterPos2i(x, y);
    //glDrawBuffer(GL_BACK);
    glDrawPixels(w, h, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
    glRasterPos2i(0, 0);
    glPopAttrib();

    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);

    CheckError(__LINE__);
    return true;
}

SILVERLININGDLL_API bool DrawAALine(const Color& c, double width, const Vector3& p1, const Vector3& p2)
{
    glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
    glEnable(GL_LINE_SMOOTH);

    glLineWidth((GLfloat)width);
    glColor4d(c.r, c.g, c.b, c.a);
    glBegin(GL_LINES);
    glTexCoord2d(0, 0);
    glVertex3d(p1.x, p1.y, p1.z);
    glTexCoord2d(1, 1);
    glVertex3d(p2.x, p2.y, p2.z);
    glEnd();
    glColor4d(1,1,1,1);

    glDisable(GL_LINE_SMOOTH);

    CheckError(__LINE__);
    return true;
}

SILVERLININGDLL_API bool DrawAALines(const Color& c, double width, const SL_VECTOR(Vector3)& points)
{
    // Some systems have a real hard time drawing anti-aliased lines, and they aren't
    // really necessary for lightning to look good, so we disable them for performance now.
    glDisable(GL_LINE_SMOOTH);

    glLineWidth((GLfloat)width);
    glColor4d(c.r, c.g, c.b, c.a);
    glBegin(GL_LINE_STRIP);

    SL_VECTOR(Vector3) ::const_iterator it;
    for (it = points.begin(); it != points.end(); it++) {
        glVertex3d((*it).x, (*it).y, (*it).z);
    }

    glEnd();
    glColor4d(1,1,1,1);

    CheckError(__LINE__);
    return true;
}

SILVERLININGDLL_API bool Set2DOrthoMatrix(double w, double h)
{
#ifdef NO_GLU
    float mat[16];
    MyOrtho2D(mat, 0, w, 0, h);
    glLoadMatrixf(mat);
#else
    gluOrtho2D(0, w, 0, h);
#endif

    CheckError(__LINE__);
    return true;
}

SILVERLININGDLL_API bool SetOrthoMatrix(double l, double r, double b, double t, double n, double f)
{
    glOrtho(l, r, b, t, n, f);

    CheckError(__LINE__);
    return true;
}

SILVERLININGDLL_API bool SetViewport(int x, int y, int w, int h)
{
    glViewport(x, y, w, h);
    CheckError(__LINE__);
    return true;
}

SILVERLININGDLL_API bool SetDepthRange(float zmin, float zmax)
{
    if (usingDepthRangedNV && (zmin < 0 || zmin > 1.0 || zmax < 0 || zmax > 1.0)) {
        glDepthRangedNV(zmin, zmax);
    } else {
        glDepthRange(zmin,zmax);
    }
    CheckError(__LINE__);
    return true;
}

SILVERLININGDLL_API bool CreateLuminanceTexture(int w, int h, TextureHandle *texture)
{
    glTexture *glTex = SL_NEW glTexture;

    GLuint tex;
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glTex->TextureID = tex;
    glTex->Width = w;
    glTex->Height = h;
    glTex->Bpp = 32;

    *texture = glTex;

    CheckError(__LINE__);
    return true;
}

SILVERLININGDLL_API bool CopyLuminanceIntoTexture(TextureHandle texture, int w, int h, unsigned char *buf)
{
    glTexture *glTex = (glTexture *)texture;
    glBindTexture(GL_TEXTURE_2D, glTex->TextureID);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, w, h, GL_RGBA, GL_UNSIGNED_BYTE, buf);

    CheckError(__LINE__);
    return true;
}

SILVERLININGDLL_API bool CopyLuminanceFromScreen(int x, int y, int w, int h, unsigned char *buf)
{
    glRasterPos2i(0,0);

    glReadPixels(0, 0, w, h, GL_RGBA, GL_UNSIGNED_BYTE, buf);

    CheckError(__LINE__);
    return true;
}

class OcclusionQuery
{
public:
    OcclusionQuery(GLuint pID) : id(pID) {
    }
    GLuint id;
};

SILVERLININGDLL_API bool StartOcclusionQuery(SilverLining::QueryHandle *queryHandle)
{
    if (usingOcclusionQuery) {
        GLuint q;
        glGenQueriesARB(1, &q);

        OcclusionQuery *query = SL_NEW OcclusionQuery(q);
        *queryHandle = (SilverLining::QueryHandle)query;

        glBeginQueryARB(GL_SAMPLES_PASSED_ARB, q);

        return QueryError();
    }
    return false;
}

SILVERLININGDLL_API bool EndOcclusionQuery(SilverLining::QueryHandle queryHandle)
{
    if (usingOcclusionQuery) {
        glEndQueryARB(GL_SAMPLES_PASSED_ARB);
        return QueryError();
    }
    return false;
}

SILVERLININGDLL_API unsigned int GetOcclusionQueryResult(SilverLining::QueryHandle queryHandle)
{
    if (usingOcclusionQuery) {
        OcclusionQuery *query = (OcclusionQuery *)queryHandle;
        GLuint q = query->id;

        GLuint nSamples;
        glGetQueryObjectuivARB(q, GL_QUERY_RESULT_ARB, &nSamples);
        CheckError(__LINE__);

        glDeleteQueriesARB(1, &q);
        CheckError(__LINE__);

        SL_DELETE (OcclusionQuery *) queryHandle;

        return nSamples;
    }
    return 0;
}

