// Copyright (c) 2004-2015  Sundog Software, LLC All rights reserved.

#define SILVERLININGDLL_API extern "C"
#include <FeSilverliningLib/Renderer.h>
#include <FeSilverliningLib/Atmosphere.h>
#include <FeSilverliningLib/Color.h>
#include <FeSilverliningLib/SilverLiningDLLCommon.h>
#include <FeSilverliningLib/Configuration.h>
#include <FeSilverliningLib/Vector4.h>
#include <FeSilverliningLib/Profiler.h>
#include <FeSilverliningLib/Renderable.h>
#include <string>
#include <stack>
#include <vector>
#include <algorithm>
#include <math.h>

#if defined(WIN32) || defined(WIN64)
#define WIN32_LEAN_AND_MEAN
#define WIN32_EXTRA_LEAN
#include <windows.h>
#else
#include<dlfcn.h>
#endif

using namespace SilverLining;
using namespace std;

/*
 #define FN_TIMER \
    char buf[256]; \
    sprintf_s(buf, 256, "%d", __LINE__); \
    Timer(SL_STRING(buf));
 */

#define FN_TIMER

#if defined(WIN32)
#pragma comment(lib, "winmm.lib")
#endif

#ifdef SILVERLINING_STATIC_RENDERER_DX9
#pragma comment(lib, "d3d9.lib")
#pragma comment(lib, "d3dx9.lib")
#endif

#ifdef SILVERLINING_STATIC_RENDERER_DX10
#pragma comment(lib, "d3d10.lib")
#pragma comment(lib, "d3dx10.lib")
#endif

#ifdef SILVERLINING_STATIC_RENDERER_DX11
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "d3dx11.lib")
#pragma comment(lib, "d3dx11effects.lib")
#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "d3dx10.lib")
#pragma comment(lib, "d2d1.lib")
#pragma comment(lib, "dwrite.lib")
#endif

#ifdef SILVERLINING_STATIC_RENDERER_DX11_1
#pragma comment(lib, "d3d11.lib")
#endif

#if (defined(SILVERLINING_STATIC_RENDERER_OPENGL) || defined(SILVERLINING_STATIC_RENDERER_OPENGL32)) && (defined(WIN32) || defined(WIN64))
#pragma comment(lib, "opengl32.lib")
#pragma comment(lib, "glu32.lib")
#endif

#if defined(SILVERLINING_STATIC_RENDERER_CUSTOM) || defined(SILVERLINING_STATIC_RENDERER_DX9) \
    || defined(SILVERLINING_STATIC_RENDERER_DX10) || defined(SILVERLINING_STATIC_RENDERER_OPENGL) \
    || defined(SILVERLINING_STATIC_RENDERER_DX11) || defined(SILVERLINING_STATIC_RENDERER_OPENGL32) || defined(ANDROID) || defined(OPENGLES) \
    || defined(SILVERLINING_STATIC_RENDERER_DX11_1)
static SET_CONTEXT_PROC SetContextProc = ::SetContext;
static CLEAR_SCREEN_PROC ClearScreenProc = ::ClearScreen;
static CLEAR_DEPTH_PROC ClearDepthProc = ::ClearDepth;
static GET_FRUSTUM_WORLD_SPACE_PROC GetFrustumWorldSpaceProc = ::GetFrustumWorldSpace;
static GET_FRUSTUM_CLIP_SPACE_PROC GetFrustumClipSpaceProc = ::GetFrustumClipSpace;
static LOAD_SHADER_FROM_FILE_PROC LoadShaderFromFileProc = ::LoadShaderFromFile;
static LOAD_SHADER_FROM_SOURCE_PROC LoadShaderFromSourceProc = ::LoadShaderFromSource;
static SET_CONSTANT_VECTOR4_PROC SetConstantVector4Proc = ::SetConstantVector4;
static SET_CONSTANT_MATRIX4_PROC SetConstantMatrix4Proc = ::SetConstantMatrix4;
static BIND_SHADER_PROC BindShaderProc = ::BindShader;
static UNBIND_SHADER_PROC UnbindShaderProc = ::UnbindShader;
static SHUTDOWN_SHADER_SYSTEM_PROC ShutdownShaderSystemProc = ::ShutdownShaderSystem;
static ALLOCATE_VERTEX_BUFFER_PROC AllocateVertexBufferProc = ::AllocateVertexBuffer;
static LOCK_VERTEX_BUFFER_PROC LockVertexBufferProc = ::LockVertexBuffer;
static GET_VERTICES_PROC GetVerticesProc = ::GetVertices;
static UNLOCK_VERTEX_BUFFER_PROC UnlockVertexBufferProc = ::UnlockVertexBuffer;
static RELEASE_VERTEX_BUFFER_PROC ReleaseVertexBufferProc = ::ReleaseVertexBuffer;
static UPDATE_VERTEX_BUFFER_PROC UpdateVertexBufferProc = ::UpdateVertexBuffer;
static GET_VERTEX_BUFFER_PROC GetVertexBufferProc = ::GetVertexBuffer;
static SET_VERTEX_BUFFER_PROC SetVertexBufferProc = ::SetVertexBuffer;
static UNSET_VERTEX_BUFFER_PROC UnsetVertexBufferProc = ::UnsetVertexBuffer;
static ALLOCATE_INDEX_BUFFER_PROC AllocateIndexBufferProc = ::AllocateIndexBuffer;
static LOCK_INDEX_BUFFER_PROC LockIndexBufferProc = ::LockIndexBuffer;
static GET_INDICES_PROC GetIndicesProc = ::GetIndices;
static UNLOCK_INDEX_BUFFER_PROC UnlockIndexBufferProc = ::UnlockIndexBuffer;
static RELEASE_INDEX_BUFFER_PROC ReleaseIndexBufferProc = ::ReleaseIndexBuffer;
static SET_INDEX_BUFFER_PROC SetIndexBufferProc = ::SetIndexBuffer;
static UNSET_INDEX_BUFFER_PROC UnsetIndexBufferProc = ::UnsetIndexBuffer;
static DRAW_STRIP_PROC DrawStripProc = ::DrawStrip;
static DRAW_POINTS_PROC DrawPointsProc = ::DrawPoints;
static DRAW_QUADS_PROC DrawQuadsProc = ::DrawQuads;
static ENABLE_DEPTH_WRITES_PROC EnableDepthWritesProc = ::EnableDepthWrites;
static ENABLE_DEPTH_READS_PROC EnableDepthReadsProc = ::EnableDepthReads;
static ENABLE_TEXTURE_2D_PROC EnableTexture2DProc = ::EnableTexture2D;
static ENABLE_TEXTURE_3D_PROC EnableTexture3DProc = ::EnableTexture3D;
static ENABLE_BACKFACE_CULLING_PROC EnableBackfaceCullingProc = ::EnableBackfaceCulling;
static ENABLE_FOG_PROC EnableFogProc = ::EnableFog;
static ENABLE_LIGHTING_PROC EnableLightingProc = ::EnableLighting;
static GET_TEXTURE_MATRIX_PROC GetTextureMatrixProc = ::GetTextureMatrix;
static SET_TEXTURE_MATRIX_PROC SetTextureMatrixProc = ::SetTextureMatrix;
static GET_PROJECTION_MATRIX_PROC GetProjectionMatrixProc = ::GetProjectionMatrix;
static SET_PROJECTION_MATRIX_PROC SetProjectionMatrixProc = ::SetProjectionMatrix;
static GET_MODELVIEW_MATRIX_PROC GetModelviewMatrixProc = ::GetModelviewMatrix;
static SET_MODELVIEW_MATRIX_PROC SetModelviewMatrixProc = ::SetModelviewMatrix;
static MULTIPLY_MODELVIEW_MATRIX_PROC MultiplyModelviewMatrixProc = ::MultiplyModelviewMatrix;
static PUSH_ALL_STATE_PROC PushAllStateProc = ::PushAllState;
static POP_ALL_STATE_PROC PopAllStateProc = ::PopAllState;
static ENABLE_BLENDING_PROC EnableBlendingProc = ::EnableBlending;
static DISABLE_BLENDING_PROC DisableBlendingProc = ::DisableBlending;
static LOAD_TEXTURE_FROM_FILE_PROC LoadTextureFromFileProc = ::LoadTextureFromFile;
static ENABLE_TEXTURE_PROC EnableTextureProc = ::EnableTexture;
static ENABLE_3D_TEXTURE_PROC Enable3DTextureProc = ::Enable3DTexture;
static DISABLE_TEXTURE_PROC DisableTextureProc = ::DisableTexture;
static RELEASE_TEXTURE_PROC ReleaseTextureProc = ::ReleaseTexture;
static HAS_POINT_SPRITES_PROC HasPointSpritesProc = ::HasPointSprites;
static ENABLE_POINT_SPRITES_PROC EnablePointSpritesProc = ::EnablePointSprites;
static DISABLE_POINT_SPRITES_PROC DisablePointSpritesProc = ::DisablePointSprites;
static GET_VIEWPORT_PROC GetViewportProc = ::GetViewport;
static GET_FOV_PROC GetFOVProc = ::GetFOV;
static LOAD_FLOAT_TEXTURE_RGB_PROC LoadFloatTextureRGBProc = ::LoadFloatTextureRGB;
static HAS_FLOAT_TEXTURES_PROC HasFloatTexturesProc = ::HasFloatTextures;
static LOAD_FLOAT_TEXTURE_PROC LoadFloatTextureProc = ::LoadFloatTexture;
static LOAD_TEXTURE_PROC LoadTextureProc = ::LoadTexture;
static LOAD_3D_TEXTURE_PROC Load3DTextureProc = ::Load3DTexture;
static LOAD_3D_TEXTURE_RGB_PROC Load3DTextureRGBProc = ::Load3DTextureRGB;
static LOAD_3D_TEXTURE_LA_PROC Load3DTextureLAProc = ::Load3DTextureLA;
static SUBLOAD_3D_TEXTURE_LA_PROC SubLoad3DTextureLAProc = ::SubLoad3DTextureLA;
static INIT_RENDER_TARGET_PROC InitRenderTargetProc = ::InitRenderTarget;
static INIT_RENDER_TEXTURE_PROC InitRenderTextureProc = ::InitRenderTexture;
static MAKE_RENDER_TARGET_CURRENT_PROC MakeRenderTargetCurrentProc = ::MakeRenderTargetCurrent;
static MAKE_RENDER_TEXTURE_CURRENT_PROC MakeRenderTextureCurrentProc = ::MakeRenderTextureCurrent;
static RESTORE_RENDER_TARGET_PROC RestoreRenderTargetProc = ::RestoreRenderTarget;
static BIND_RENDER_TEXTURE_PROC BindRenderTextureProc = ::BindRenderTexture;
static GET_RENDER_TEXTURE_TEXTURE_HANDLE_PROC GetRenderTextureTextureHandleProc = ::GetRenderTextureTextureHandle;
static RELEASE_RENDER_TARGET_PROC ReleaseRenderTargetProc = ::ReleaseRenderTarget;
static RELEASE_RENDER_TEXTURE_PROC ReleaseRenderTextureProc = ::ReleaseRenderTexture;
static INIT_RENDER_TEXTURE_CUBE_PROC InitRenderTextureCubeProc = ::InitRenderTextureCube;
static MAKE_RENDER_TEXTURE_CUBE_CURRENT_PROC MakeRenderTextureCubeCurrentProc = ::MakeRenderTextureCubeCurrent;
static BIND_RENDER_TEXTURE_CUBE_PROC BindRenderTextureCubeProc = ::BindRenderTextureCube;
static GET_RENDER_TEXTURE_CUBE_TEXTURE_HANDLE_PROC GetRenderTextureCubeTextureHandleProc = ::GetRenderTextureCubeTextureHandle;
static RELEASE_RENDER_TEXTURE_CUBE_PROC ReleaseRenderTextureCubeProc = ::ReleaseRenderTextureCube;
static GET_PIXELS_PROC GetPixelsProc = ::GetPixels;
static SET_PIXELS_PROC SetPixelsProc = ::SetPixels;
static DRAW_AA_LINE_PROC DrawAALineProc = ::DrawAALine;
static DRAW_AA_LINES_PROC DrawAALinesProc = ::DrawAALines;
static HAS_QUADS_PROC HasQuadsProc = ::HasQuads;
static SET_CURRENT_COLOR_PROC SetCurrentColorProc = ::SetCurrentColor;
static CONFIGURE_FOG_PROC ConfigureFogProc = ::ConfigureFog;
static SET_VIEWPORT_PROC SetViewportProc = ::SetViewport;
static COPY_LUMINANCE_FROM_SCREEN_PROC CopyLuminanceFromScreenProc = ::CopyLuminanceFromScreen;
static CREATE_LUMINANCE_TEXTURE_PROC CreateLuminanceTextureProc = ::CreateLuminanceTexture;
static COPY_LUMINANCE_INTO_TEXTURE_PROC CopyLuminanceIntoTextureProc = ::CopyLuminanceIntoTexture;
static SET_DEFAULT_STATE_PROC SetDefaultStateProc = ::SetDefaultState;
static START_OCCLUSION_QUERY_PROC StartOcclusionQueryProc = ::StartOcclusionQuery;
static END_OCCLUSION_QUERY_PROC EndOcclusionQueryProc = ::EndOcclusionQuery;
static GET_OCCLUSION_QUERY_RESULT_PROC GetOcclusionQueryResultProc = ::GetOcclusionQueryResult;
static SET_DEPTH_RANGE_PROC SetDepthRangeProc = ::SetDepthRange;
static GET_DEPTH_RANGE_PROC GetDepthRangeProc = ::GetDepthRange;
static DEVICE_LOST_PROC DeviceLostProc = ::DeviceLost;
static DEVICE_RESET_PROC DeviceResetProc = ::DeviceReset;
static GET_NATIVE_TEXTURE_PROC GetNativeTextureProc = ::GetNativeTexture;
static SET_USER_SHADERS_PROC SetUserShadersProc = ::SetUserShaders;
static GET_SHADER_PROGRAM_OBJECT_PROC GetShaderProgramObjectProc = ::GetShaderProgramObject;
static DELETE_SHADER_PROC DeleteShaderProc = ::DeleteShader;
static BACKFACE_CULL_CLOCKWISE_PROC BackfaceCullClockwiseProc = ::BackfaceCullClockwise;
#else
#ifdef _WIN32
static HINSTANCE dll = 0;
#else
static void* dll = 0;
#endif
static SET_CONTEXT_PROC SetContextProc = 0;
static CLEAR_SCREEN_PROC ClearScreenProc = 0;
static CLEAR_DEPTH_PROC ClearDepthProc = 0;
static SET_ENV_PROC SetEnvProc = 0;
static GET_FRUSTUM_WORLD_SPACE_PROC GetFrustumWorldSpaceProc = 0;
static GET_FRUSTUM_CLIP_SPACE_PROC GetFrustumClipSpaceProc = 0;
static LOAD_SHADER_FROM_FILE_PROC LoadShaderFromFileProc = 0;
static LOAD_SHADER_FROM_SOURCE_PROC LoadShaderFromSourceProc = 0;
static SET_CONSTANT_VECTOR4_PROC SetConstantVector4Proc = 0;
static SET_CONSTANT_MATRIX4_PROC SetConstantMatrix4Proc = 0;
static BIND_SHADER_PROC BindShaderProc = 0;
static UNBIND_SHADER_PROC UnbindShaderProc = 0;
static SHUTDOWN_SHADER_SYSTEM_PROC ShutdownShaderSystemProc = 0;
static ALLOCATE_VERTEX_BUFFER_PROC AllocateVertexBufferProc = 0;
static LOCK_VERTEX_BUFFER_PROC LockVertexBufferProc = 0;
static GET_VERTICES_PROC GetVerticesProc = 0;
static UNLOCK_VERTEX_BUFFER_PROC UnlockVertexBufferProc = 0;
static RELEASE_VERTEX_BUFFER_PROC ReleaseVertexBufferProc = 0;
static UPDATE_VERTEX_BUFFER_PROC UpdateVertexBufferProc = 0;
static GET_VERTEX_BUFFER_PROC GetVertexBufferProc = 0;
static SET_VERTEX_BUFFER_PROC SetVertexBufferProc = 0;
static UNSET_VERTEX_BUFFER_PROC UnsetVertexBufferProc = 0;
static ALLOCATE_INDEX_BUFFER_PROC AllocateIndexBufferProc = 0;
static LOCK_INDEX_BUFFER_PROC LockIndexBufferProc = 0;
static GET_INDICES_PROC GetIndicesProc = 0;
static UNLOCK_INDEX_BUFFER_PROC UnlockIndexBufferProc = 0;
static RELEASE_INDEX_BUFFER_PROC ReleaseIndexBufferProc = 0;
static SET_INDEX_BUFFER_PROC SetIndexBufferProc = 0;
static UNSET_INDEX_BUFFER_PROC UnsetIndexBufferProc = 0;
static DRAW_STRIP_PROC DrawStripProc = 0;
static DRAW_POINTS_PROC DrawPointsProc = 0;
static DRAW_QUADS_PROC DrawQuadsProc = 0;
static ENABLE_DEPTH_WRITES_PROC EnableDepthWritesProc = 0;
static ENABLE_DEPTH_READS_PROC EnableDepthReadsProc = 0;
static ENABLE_TEXTURE_2D_PROC EnableTexture2DProc = 0;
static ENABLE_TEXTURE_3D_PROC EnableTexture3DProc = 0;
static ENABLE_BACKFACE_CULLING_PROC EnableBackfaceCullingProc = 0;
static ENABLE_FOG_PROC EnableFogProc = 0;
static ENABLE_LIGHTING_PROC EnableLightingProc = 0;
static GET_TEXTURE_MATRIX_PROC GetTextureMatrixProc = 0;
static SET_TEXTURE_MATRIX_PROC SetTextureMatrixProc = 0;
static GET_PROJECTION_MATRIX_PROC GetProjectionMatrixProc = 0;
static SET_PROJECTION_MATRIX_PROC SetProjectionMatrixProc = 0;
static GET_MODELVIEW_MATRIX_PROC GetModelviewMatrixProc = 0;
static SET_MODELVIEW_MATRIX_PROC SetModelviewMatrixProc = 0;
static MULTIPLY_MODELVIEW_MATRIX_PROC MultiplyModelviewMatrixProc = 0;
static POP_TRANSFORMS_PROC PopTransformsProc = 0;
static PUSH_ALL_STATE_PROC PushAllStateProc = 0;
static POP_ALL_STATE_PROC PopAllStateProc = 0;
static ENABLE_BLENDING_PROC EnableBlendingProc = 0;
static DISABLE_BLENDING_PROC DisableBlendingProc = 0;
static LOAD_TEXTURE_FROM_FILE_PROC LoadTextureFromFileProc = 0;
static ENABLE_TEXTURE_PROC EnableTextureProc = 0;
static ENABLE_3D_TEXTURE_PROC Enable3DTextureProc = 0;
static DISABLE_TEXTURE_PROC DisableTextureProc = 0;
static RELEASE_TEXTURE_PROC ReleaseTextureProc = 0;
static HAS_POINT_SPRITES_PROC HasPointSpritesProc = 0;
static ENABLE_POINT_SPRITES_PROC EnablePointSpritesProc = 0;
static DISABLE_POINT_SPRITES_PROC DisablePointSpritesProc = 0;
static GET_VIEWPORT_PROC GetViewportProc = 0;
static GET_FOV_PROC GetFOVProc = 0;
static LOAD_FLOAT_TEXTURE_RGB_PROC LoadFloatTextureRGBProc = 0;
static HAS_FLOAT_TEXTURES_PROC HasFloatTexturesProc = 0;
static LOAD_FLOAT_TEXTURE_PROC LoadFloatTextureProc = 0;
static LOAD_TEXTURE_PROC LoadTextureProc = 0;
static LOAD_3D_TEXTURE_PROC Load3DTextureProc = 0;
static LOAD_3D_TEXTURE_RGB_PROC Load3DTextureRGBProc = 0;
static LOAD_3D_TEXTURE_LA_PROC Load3DTextureLAProc = 0;
static SUBLOAD_3D_TEXTURE_LA_PROC SubLoad3DTextureLAProc = 0;
static INIT_RENDER_TARGET_PROC InitRenderTargetProc = 0;
static INIT_RENDER_TEXTURE_PROC InitRenderTextureProc = 0;
static MAKE_RENDER_TARGET_CURRENT_PROC MakeRenderTargetCurrentProc = 0;
static MAKE_RENDER_TEXTURE_CURRENT_PROC MakeRenderTextureCurrentProc = 0;
static RESTORE_RENDER_TARGET_PROC RestoreRenderTargetProc = 0;
static BIND_RENDER_TEXTURE_PROC BindRenderTextureProc = 0;
static GET_RENDER_TEXTURE_TEXTURE_HANDLE_PROC GetRenderTextureTextureHandleProc = 0;
static RELEASE_RENDER_TARGET_PROC ReleaseRenderTargetProc = 0;
static RELEASE_RENDER_TEXTURE_PROC ReleaseRenderTextureProc = 0;
static INIT_RENDER_TEXTURE_CUBE_PROC InitRenderTextureCubeProc = 0;
static MAKE_RENDER_TEXTURE_CUBE_CURRENT_PROC MakeRenderTextureCubeCurrentProc = 0;
static BIND_RENDER_TEXTURE_CUBE_PROC BindRenderTextureCubeProc = 0;
static GET_RENDER_TEXTURE_CUBE_TEXTURE_HANDLE_PROC GetRenderTextureCubeTextureHandleProc = 0;
static RELEASE_RENDER_TEXTURE_CUBE_PROC ReleaseRenderTextureCubeProc = 0;
static GET_PIXELS_PROC GetPixelsProc = 0;
static SET_PIXELS_PROC SetPixelsProc = 0;
static DRAW_AA_LINE_PROC DrawAALineProc = 0;
static DRAW_AA_LINES_PROC DrawAALinesProc = 0;
static SET_2D_ORTHO_MATRIX_PROC Set2DOrthoMatrixProc = 0;
static HAS_QUADS_PROC HasQuadsProc = 0;
static SET_CURRENT_COLOR_PROC SetCurrentColorProc = 0;
static SET_ORTHO_MATRIX_PROC SetOrthoMatrixProc = 0;
static CONFIGURE_FOG_PROC ConfigureFogProc = 0;
static SET_VIEWPORT_PROC SetViewportProc = 0;
static COPY_LUMINANCE_FROM_SCREEN_PROC CopyLuminanceFromScreenProc = 0;
static CREATE_LUMINANCE_TEXTURE_PROC CreateLuminanceTextureProc = 0;
static COPY_LUMINANCE_INTO_TEXTURE_PROC CopyLuminanceIntoTextureProc = 0;
static SET_DEFAULT_STATE_PROC SetDefaultStateProc = 0;
static START_OCCLUSION_QUERY_PROC StartOcclusionQueryProc = 0;
static END_OCCLUSION_QUERY_PROC EndOcclusionQueryProc = 0;
static GET_OCCLUSION_QUERY_RESULT_PROC GetOcclusionQueryResultProc = 0;
static SET_DEPTH_RANGE_PROC SetDepthRangeProc = 0;
static GET_DEPTH_RANGE_PROC GetDepthRangeProc = 0;
static DEVICE_LOST_PROC DeviceLostProc = 0;
static DEVICE_RESET_PROC DeviceResetProc = 0;
static GET_NATIVE_TEXTURE_PROC GetNativeTextureProc = 0;
static SET_USER_SHADERS_PROC SetUserShadersProc = 0;
static GET_SHADER_PROGRAM_OBJECT_PROC GetShaderProgramObjectProc = 0;
static DELETE_SHADER_PROC DeleteShaderProc = 0;
static BACKFACE_CULL_CLOCKWISE_PROC BackfaceCullClockwiseProc = 0;
#endif

Renderer * Renderer::renderer = 0;

Renderer *Renderer::GetInstance()
{
    if (!renderer) {
        renderer = SL_NEW Renderer();
    }

    return renderer;
}

void Renderer::Destroy()
{
    if (renderer) {
        SL_DELETE renderer;
    }

    renderer = 0;
}

Renderer::Renderer() : blendingEnabled(false), lastSrc(-1), lastDst(-1), lastTex(0), lastShader(0),
    fogEnabled(false), rightHanded(true), disableDepthReads(false), disableDepthWrites(false),
    hasExplicitMatrices(false), hasExplicitViewport(false), cullingUndefined(true), cullingEnabled(false),
    fogDensity(1E-9), fogStart(0), fogEnd(1E9), hasExplicitDepthRange(false), minZ(0.0f), maxZ(1.0f)
{
    upVector = Vector3(0, 1, 0);
    rightVector = Vector3(1, 0, 0);
    ComputeBasis();
}

Renderer::~Renderer()
{
#if !(defined SILVERLINING_STATIC_RENDERER_CUSTOM || defined SILVERLINING_STATIC_RENDERER_DX9 || \
      defined SILVERLINING_STATIC_RENDERER_DX10 || defined SILVERLINING_STATIC_RENDERER_OPENGL || \
      defined SILVERLINING_STATIC_RENDERER_DX11 || defined SILVERLINING_STATIC_RENDERER_OPENGL32 || \
      defined ANDROID || defined OPENGLES || defined SILVERLINING_STATIC_RENDERER_DX11_1 )
#ifdef _WIN32
    if (dll) {
        FreeLibrary(dll);
        dll = 0;
    }
#else
    if(dll)
    {
        dlclose(dll);
        dll = 0;
    }
#endif
#endif
}

void Renderer::SetUpVector(const Vector3& up)
{
    upVector = up;

    ComputeBasis();
}

void Renderer::SetRightVector(const Vector3& right)
{
    rightVector = right;

    ComputeBasis();
}

void Renderer::ComputeBasis()
{
    Vector3 zAxis, yAxis, xAxis;

    zAxis = rightVector.Cross(upVector);
    zAxis.Normalize();
    xAxis = upVector.Cross(zAxis);
    xAxis.Normalize();
    yAxis = zAxis.Cross(xAxis);
    yAxis.Normalize();

    if (!rightHanded) {
        zAxis = zAxis * -1;
    }

    basis = Matrix4();
    basis.elem[0][0] = xAxis.x;
    basis.elem[0][1] = xAxis.y;
    basis.elem[0][2] = xAxis.z;
    basis.elem[1][0] = yAxis.x;
    basis.elem[1][1] = yAxis.y;
    basis.elem[1][2] = yAxis.z;
    basis.elem[2][0] = zAxis.x;
    basis.elem[2][1] = zAxis.y;
    basis.elem[2][2] = zAxis.z;

    invBasis = Matrix4();
    invBasis.elem[0][0] = xAxis.x;
    invBasis.elem[0][1] = yAxis.x;
    invBasis.elem[0][2] = zAxis.x;
    invBasis.elem[1][0] = xAxis.y;
    invBasis.elem[1][1] = yAxis.y;
    invBasis.elem[1][2] = zAxis.y;
    invBasis.elem[2][0] = xAxis.z;
    invBasis.elem[2][1] = yAxis.z;
    invBasis.elem[2][2] = zAxis.z;

    basis3 = Matrix3();
    basis3.elem[0][0] = xAxis.x;
    basis3.elem[0][1] = xAxis.y;
    basis3.elem[0][2] = xAxis.z;
    basis3.elem[1][0] = yAxis.x;
    basis3.elem[1][1] = yAxis.y;
    basis3.elem[1][2] = yAxis.z;
    basis3.elem[2][0] = zAxis.x;
    basis3.elem[2][1] = zAxis.y;
    basis3.elem[2][2] = zAxis.z;

    invBasis3 = Matrix3();
    invBasis3.elem[0][0] = xAxis.x;
    invBasis3.elem[0][1] = yAxis.x;
    invBasis3.elem[0][2] = zAxis.x;
    invBasis3.elem[1][0] = xAxis.y;
    invBasis3.elem[1][1] = yAxis.y;
    invBasis3.elem[1][2] = zAxis.y;
    invBasis3.elem[2][0] = xAxis.z;
    invBasis3.elem[2][1] = yAxis.z;
    invBasis3.elem[2][2] = zAxis.z;
}

int Renderer::Initialize(int renderer, bool rightHanded, void *environment, void **context)
{
    type = renderer;
    this->rightHanded = rightHanded;

#if (defined SILVERLINING_STATIC_RENDERER_DX9 || defined SILVERLINING_STATIC_RENDERER_DX10 || \
     defined SILVERLINING_STATIC_RENDERER_DX11 || defined SILVERLINING_STATIC_RENDERER_OPENGL || \
     defined SILVERLINING_STATIC_RENDERER_CUSTOM || defined SILVERLINING_STATIC_RENDERER_OPENGL32 || \
     defined ANDROID || defined OPENGLES || defined SILVERLINING_STATIC_RENDERER_DX11_1 )

    *context = SetEnvironment(rightHanded, environment, Atmosphere::GetResourceLoader(), Allocator::GetAllocator(),
                              Atmosphere::GetUserDefinedVertString(), Atmosphere::GetUserDefinedFragString());

    if (!*context) {
        return Atmosphere::E_CANT_INITIALIZE_RENDERER_SUBSYSTEM;
    }

    FrameStarted(*context);

    return Atmosphere::E_NOERROR;

#else

//    //#ifdef _DEBUG
//    //    SL_STRING dllName = "c:\\sl\\dll\\Debug\\";
//    //#else
//    SL_STRING dllName = Atmosphere::GetResourceLoader()->GetResourceDirPath();
//#if _MSC_VER >= 1900
//    dllName += "/VC14/";
//#elif _MSC_VER >= 1800
//    dllName += "/VC12/";
//#elif _MSC_VER >= 1700
//    dllName += "/VC11/";
//#elif _MSC_VER >= 1600
//    dllName += "/VC10/";
//#elif _MSC_VER >= 1500
//    dllName += "/VC9/";
//#elif _MSC_VER >= 1400
//    dllName += "/VC8/";
//#elif _MSC_VER >= 1300
//    dllName += "/VC7/";
//#else
//    dllName += "/VC6/";
//#endif
//
//#if _MSC_VER >= 1400
//#ifdef WIN64
//    dllName += "x64/";
//#else
//    dllName += "win32/";
//#endif
//#endif

//#endif

    /*switch (renderer) 
	{
#ifdef SINGLE_THREADED
    case Atmosphere::OPENGL:
        dllName += "SilverLiningOpenGL-ST.dll";
        break;

    case Atmosphere::OPENGL32CORE:
        dllName += "SilverLiningOpenGL32-ST.dll";
        break;

    case Atmosphere::DIRECTX9:
        dllName += "SilverLiningDirectX9-ST.dll";
        break;

    case Atmosphere::DIRECTX10:
        dllName += "SilverLiningDirectX10-ST.dll";
        break;

    case Atmosphere::DIRECTX11:
        dllName += "SilverLiningDirectX11-ST.dll";
        break;

    case Atmosphere::DIRECTX11_1:
        dllName += "SilverLiningDirectX11_1-ST.dll";
        break;

#endif

#ifdef MULTI_THREADED
    case Atmosphere::OPENGL:
        dllName += "SilverLiningOpenGL-MT.dll";
        break;

    case Atmosphere::OPENGL32CORE:
        dllName += "SilverLiningOpenGL32-MT.dll";
        break;

    case Atmosphere::DIRECTX9:
        dllName += "SilverLiningDirectX9-MT.dll";
        break;

    case Atmosphere::DIRECTX10:
        dllName += "SilverLiningDirectX10-MT.dll";
        break;

    case Atmosphere::DIRECTX11:
        dllName += "SilverLiningDirectX11-MT.dll";
        break;

    case Atmosphere::DIRECTX11_1:
        dllName += "SilverLiningDirectX11_1-MT.dll";
        break;
#endif

#ifdef SINGLE_THREADED_DEBUG
    case Atmosphere::OPENGL:
        dllName += "SilverLiningOpenGL-STD.dll";
        break;

    case Atmosphere::OPENGL32CORE:
        dllName += "SilverLiningOpenGL32-STD.dll";
        break;

    case Atmosphere::DIRECTX9:
        dllName += "SilverLiningDirectX9-STD.dll";
        break;

    case Atmosphere::DIRECTX10:
        dllName += "SilverLiningDirectX10-STD.dll";
        break;

    case Atmosphere::DIRECTX11:
        dllName += "SilverLiningDirectX11-STD.dll";
        break;

    case Atmosphere::DIRECTX11_1:
        dllName += "SilverLiningDirectX11_1-STD.dll";
        break;
#endif

#ifdef MULTI_THREADED_DEBUG
    case Atmosphere::OPENGL:
        dllName += "SilverLiningOpenGL-MTD.dll";
        break;

    case Atmosphere::OPENGL32CORE:
        dllName += "SilverLiningOpenGL32-MTD.dll";
        break;

    case Atmosphere::DIRECTX9:
        dllName += "SilverLiningDirectX9-MTD.dll";
        break;

    case Atmosphere::DIRECTX10:
        dllName += "SilverLiningDirectX10-MTD.dll";
        break;

    case Atmosphere::DIRECTX11:
        dllName += "SilverLiningDirectX11-MTD.dll";
        break;

    case Atmosphere::DIRECTX11_1:
        dllName += "SilverLiningDirectX11_1-MTD.dll";
        break;
#endif

#ifdef MULTI_THREADED_DEBUG_DLL
    case Atmosphere::OPENGL:
        dllName += "SilverLiningOpenGL-MTD-DLL.dll";
        break;

    case Atmosphere::OPENGL32CORE:
        dllName += "SilverLiningOpenGL32-MTD-DLL.dll";
        break;

    case Atmosphere::DIRECTX9:
        dllName += "SilverLiningDirectX9-MTD-DLL.dll";
        break;

    case Atmosphere::DIRECTX10:
        dllName += "SilverLiningDirectX10-MTD-DLL.dll";
        break;

    case Atmosphere::DIRECTX11:
        dllName += "SilverLiningDirectX11-MTD-DLL.dll";
        break;

    case Atmosphere::DIRECTX11_1:
        dllName += "SilverLiningDirectX11_1-MTD-DLL.dll";
        break;
#endif

#ifdef MULTI_THREADED_DLL
    case Atmosphere::OPENGL:
        dllName += "SilverLiningOpenGL-MT-DLL.dll";
        break;

    case Atmosphere::OPENGL32CORE:
        dllName += "SilverLiningOpenGL32-MT-DLL.dll";
        break;

    case Atmosphere::DIRECTX9:
        dllName += "SilverLiningDirectX9-MT-DLL.dll";
        break;

    case Atmosphere::DIRECTX10:
        dllName += "SilverLiningDirectX10-MT-DLL.dll";
        break;

    case Atmosphere::DIRECTX11:
        dllName += "SilverLiningDirectX11-MT-DLL.dll";
        break;

    case Atmosphere::DIRECTX11_1:
        dllName += "SilverLiningDirectX11_1-MT-DLL.dll";
        break;
#endif

    default:
        return Atmosphere::E_CANT_LOAD_RENDERER_DLL;
    }*/

#if WIN32
#ifdef _DEBUG
    SL_STRING dllName = "FeSilverliningOpenGLd.dll";
#else
    SL_STRING dllName = "FeSilverliningOpenGL.dll";
#endif
#else
#ifdef _DEBUG
SL_STRING dllName = "libFeSilverliningOpenGLd.so";
#else
SL_STRING dllName = "libFeSilverliningOpenGL.so";
#endif
#endif

#if _WIN32
    if (!dll) dll = LoadLibraryA(dllName.c_str());
#else
    if(!dll) dll = dlopen(dllName.c_str(), RTLD_LAZY | RTLD_GLOBAL );
#endif
#if _WIN32
    if (dll) {
        SetContextProc = (SET_CONTEXT_PROC) GetProcAddress(dll, "SetContext");
        ClearScreenProc = (CLEAR_SCREEN_PROC) GetProcAddress(dll, "ClearScreen");
        ClearDepthProc = (CLEAR_DEPTH_PROC) GetProcAddress(dll, "ClearDepth");
        SetEnvProc = (SET_ENV_PROC) GetProcAddress(dll, "SetEnvironment");
        GetFrustumWorldSpaceProc = (GET_FRUSTUM_WORLD_SPACE_PROC) GetProcAddress(dll, "GetFrustumWorldSpace");
        GetFrustumClipSpaceProc = (GET_FRUSTUM_CLIP_SPACE_PROC) GetProcAddress(dll, "GetFrustumClipSpace");
        LoadShaderFromFileProc = (LOAD_SHADER_FROM_FILE_PROC) GetProcAddress(dll, "LoadShaderFromFile");
        LoadShaderFromSourceProc = (LOAD_SHADER_FROM_SOURCE_PROC) GetProcAddress(dll, "LoadShaderFromSource");
        SetConstantVector4Proc = (SET_CONSTANT_VECTOR4_PROC) GetProcAddress(dll, "SetConstantVector4");
        SetConstantMatrix4Proc = (SET_CONSTANT_MATRIX4_PROC) GetProcAddress(dll, "SetConstantMatrix4");
        BindShaderProc = (BIND_SHADER_PROC) GetProcAddress(dll, "BindShader");
        UnbindShaderProc = (UNBIND_SHADER_PROC) GetProcAddress(dll, "UnbindShader");
        ShutdownShaderSystemProc = (SHUTDOWN_SHADER_SYSTEM_PROC) GetProcAddress(dll, "ShutdownShaderSystem");
        AllocateVertexBufferProc = (ALLOCATE_VERTEX_BUFFER_PROC) GetProcAddress(dll, "AllocateVertexBuffer");
        LockVertexBufferProc = (LOCK_VERTEX_BUFFER_PROC) GetProcAddress(dll, "LockVertexBuffer");
        GetVerticesProc = (GET_VERTICES_PROC) GetProcAddress(dll, "GetVertices");
        UnlockVertexBufferProc = (UNLOCK_VERTEX_BUFFER_PROC) GetProcAddress(dll, "UnlockVertexBuffer");
        GetVertexBufferProc = (GET_VERTEX_BUFFER_PROC) GetProcAddress(dll, "GetVertexBuffer");
        UpdateVertexBufferProc = (UPDATE_VERTEX_BUFFER_PROC) GetProcAddress(dll, "UpdateVertexBuffer");
        ReleaseVertexBufferProc = (RELEASE_VERTEX_BUFFER_PROC) GetProcAddress(dll, "ReleaseVertexBuffer");
        SetVertexBufferProc = (SET_VERTEX_BUFFER_PROC) GetProcAddress(dll, "SetVertexBuffer");
        UnsetVertexBufferProc = (UNSET_VERTEX_BUFFER_PROC) GetProcAddress(dll, "UnsetVertexBuffer");
        AllocateIndexBufferProc = (ALLOCATE_INDEX_BUFFER_PROC) GetProcAddress(dll, "AllocateIndexBuffer");
        LockIndexBufferProc = (LOCK_INDEX_BUFFER_PROC) GetProcAddress(dll, "LockIndexBuffer");
        GetIndicesProc = (GET_INDICES_PROC) GetProcAddress(dll, "GetIndices");
        UnlockIndexBufferProc = (UNLOCK_INDEX_BUFFER_PROC) GetProcAddress(dll, "UnlockIndexBuffer");
        ReleaseIndexBufferProc = (RELEASE_INDEX_BUFFER_PROC) GetProcAddress(dll, "ReleaseIndexBuffer");
        SetIndexBufferProc = (SET_INDEX_BUFFER_PROC) GetProcAddress(dll, "SetIndexBuffer");
        UnsetIndexBufferProc = (UNSET_INDEX_BUFFER_PROC) GetProcAddress(dll, "UnsetIndexBuffer");
        DrawStripProc = (DRAW_STRIP_PROC) GetProcAddress(dll, "DrawStrip");
        DrawPointsProc = (DRAW_POINTS_PROC) GetProcAddress(dll, "DrawPoints");
        EnableDepthWritesProc = (ENABLE_DEPTH_WRITES_PROC) GetProcAddress(dll, "EnableDepthWrites");
        EnableDepthReadsProc = (ENABLE_DEPTH_WRITES_PROC) GetProcAddress(dll, "EnableDepthReads");
        EnableTexture2DProc = (ENABLE_TEXTURE_2D_PROC) GetProcAddress(dll, "EnableTexture2D");
        EnableTexture3DProc = (ENABLE_TEXTURE_3D_PROC) GetProcAddress(dll, "EnableTexture3D");
        EnableBackfaceCullingProc = (ENABLE_BACKFACE_CULLING_PROC) GetProcAddress(dll, "EnableBackfaceCulling");
        EnableFogProc = (ENABLE_FOG_PROC) GetProcAddress(dll, "EnableFog");
        EnableLightingProc = (ENABLE_LIGHTING_PROC) GetProcAddress(dll, "EnableLighting");
        GetTextureMatrixProc = (GET_TEXTURE_MATRIX_PROC) GetProcAddress(dll, "GetTextureMatrix");
        SetTextureMatrixProc = (SET_TEXTURE_MATRIX_PROC) GetProcAddress(dll, "SetTextureMatrix");
        GetProjectionMatrixProc = (GET_PROJECTION_MATRIX_PROC) GetProcAddress(dll, "GetProjectionMatrix");
        SetProjectionMatrixProc = (SET_PROJECTION_MATRIX_PROC) GetProcAddress(dll, "SetProjectionMatrix");
        GetModelviewMatrixProc = (GET_MODELVIEW_MATRIX_PROC) GetProcAddress(dll, "GetModelviewMatrix");
        SetModelviewMatrixProc = (SET_MODELVIEW_MATRIX_PROC) GetProcAddress(dll, "SetModelviewMatrix");
        MultiplyModelviewMatrixProc = (MULTIPLY_MODELVIEW_MATRIX_PROC) GetProcAddress(dll, "MultiplyModelviewMatrix");
        PushAllStateProc = (PUSH_ALL_STATE_PROC) GetProcAddress(dll, "PushAllState");
        PopAllStateProc = (POP_ALL_STATE_PROC) GetProcAddress(dll, "PopAllState");
        EnableBlendingProc = (ENABLE_BLENDING_PROC) GetProcAddress(dll, "EnableBlending");
        DisableBlendingProc = (DISABLE_BLENDING_PROC) GetProcAddress(dll, "DisableBlending");
        LoadTextureFromFileProc = (LOAD_TEXTURE_FROM_FILE_PROC) GetProcAddress(dll, "LoadTextureFromFile");
        EnableTextureProc = (ENABLE_TEXTURE_PROC) GetProcAddress(dll, "EnableTexture");
        Enable3DTextureProc = (ENABLE_3D_TEXTURE_PROC) GetProcAddress(dll, "Enable3DTexture");
        DisableTextureProc = (DISABLE_TEXTURE_PROC) GetProcAddress(dll, "DisableTexture");
        ReleaseTextureProc = (RELEASE_TEXTURE_PROC) GetProcAddress(dll, "ReleaseTexture");
        HasPointSpritesProc = (HAS_POINT_SPRITES_PROC) GetProcAddress(dll, "HasPointSprites");
        EnablePointSpritesProc = (ENABLE_POINT_SPRITES_PROC) GetProcAddress(dll, "EnablePointSprites");
        DisablePointSpritesProc = (DISABLE_POINT_SPRITES_PROC) GetProcAddress(dll, "DisablePointSprites");
        GetViewportProc = (GET_VIEWPORT_PROC) GetProcAddress(dll, "GetViewport");
        GetFOVProc = (GET_FOV_PROC) GetProcAddress(dll, "GetFOV");
        DrawQuadsProc = (DRAW_QUADS_PROC) GetProcAddress(dll, "DrawQuads");
        LoadFloatTextureRGBProc = (LOAD_FLOAT_TEXTURE_RGB_PROC) GetProcAddress(dll, "LoadFloatTextureRGB");
        HasFloatTexturesProc = (HAS_FLOAT_TEXTURES_PROC) GetProcAddress(dll, "HasFloatTextures");
        LoadFloatTextureProc = (LOAD_FLOAT_TEXTURE_PROC) GetProcAddress(dll, "LoadFloatTexture");
        LoadTextureProc = (LOAD_TEXTURE_PROC) GetProcAddress(dll, "LoadTexture");
        Load3DTextureProc = (LOAD_3D_TEXTURE_PROC) GetProcAddress(dll, "Load3DTexture");
        Load3DTextureRGBProc = (LOAD_3D_TEXTURE_RGB_PROC) GetProcAddress(dll, "Load3DTextureRGB");
        Load3DTextureLAProc = (LOAD_3D_TEXTURE_LA_PROC) GetProcAddress(dll, "Load3DTextureLA");
        SubLoad3DTextureLAProc = (SUBLOAD_3D_TEXTURE_LA_PROC) GetProcAddress(dll, "SubLoad3DTextureLA");
        InitRenderTargetProc = (INIT_RENDER_TARGET_PROC) GetProcAddress(dll, "InitRenderTarget");
        InitRenderTextureProc = (INIT_RENDER_TEXTURE_PROC) GetProcAddress(dll, "InitRenderTexture");
        MakeRenderTargetCurrentProc = (MAKE_RENDER_TARGET_CURRENT_PROC) GetProcAddress(dll, "MakeRenderTargetCurrent");
        MakeRenderTextureCurrentProc = (MAKE_RENDER_TEXTURE_CURRENT_PROC) GetProcAddress(dll, "MakeRenderTextureCurrent");
        RestoreRenderTargetProc = (RESTORE_RENDER_TARGET_PROC) GetProcAddress(dll, "RestoreRenderTarget");
        BindRenderTextureProc = (BIND_RENDER_TEXTURE_PROC) GetProcAddress(dll, "BindRenderTexture");
        GetRenderTextureTextureHandleProc = (GET_RENDER_TEXTURE_TEXTURE_HANDLE_PROC) GetProcAddress(dll, "GetRenderTextureTextureHandle");
        ReleaseRenderTextureProc = (RELEASE_RENDER_TEXTURE_PROC) GetProcAddress(dll, "ReleaseRenderTexture");
        ReleaseRenderTargetProc = (RELEASE_RENDER_TARGET_PROC) GetProcAddress(dll, "ReleaseRenderTarget");
        InitRenderTextureCubeProc = (INIT_RENDER_TEXTURE_CUBE_PROC)GetProcAddress(dll, "InitRenderTextureCube");
        MakeRenderTextureCubeCurrentProc = (MAKE_RENDER_TEXTURE_CUBE_CURRENT_PROC)GetProcAddress(dll, "MakeRenderTextureCubeCurrent");
        BindRenderTextureCubeProc = (BIND_RENDER_TEXTURE_CUBE_PROC)GetProcAddress(dll, "BindRenderTextureCube");
        GetRenderTextureCubeTextureHandleProc = (GET_RENDER_TEXTURE_CUBE_TEXTURE_HANDLE_PROC)GetProcAddress(dll, "GetRenderTextureCubeTextureHandle");
        ReleaseRenderTextureCubeProc = (RELEASE_RENDER_TEXTURE_CUBE_PROC)GetProcAddress(dll, "ReleaseRenderTextureCube");
        GetPixelsProc = (GET_PIXELS_PROC) GetProcAddress(dll, "GetPixels");
        SetPixelsProc = (SET_PIXELS_PROC) GetProcAddress(dll, "SetPixels");
        DrawAALineProc = (DRAW_AA_LINE_PROC) GetProcAddress(dll, "DrawAALine");
        DrawAALinesProc = (DRAW_AA_LINES_PROC) GetProcAddress(dll, "DrawAALines");
        Set2DOrthoMatrixProc = (SET_2D_ORTHO_MATRIX_PROC) GetProcAddress(dll, "Set2DOrthoMatrix");
        HasQuadsProc = (HAS_QUADS_PROC) GetProcAddress(dll, "HasQuads");
        SetCurrentColorProc = (SET_CURRENT_COLOR_PROC) GetProcAddress(dll, "SetCurrentColor");
        SetOrthoMatrixProc = (SET_ORTHO_MATRIX_PROC) GetProcAddress(dll, "SetOrthoMatrix");
        ConfigureFogProc = (CONFIGURE_FOG_PROC) GetProcAddress(dll, "ConfigureFog");
        CreateLuminanceTextureProc = (CREATE_LUMINANCE_TEXTURE_PROC) GetProcAddress(dll, "CreateLuminanceTexture");
        CopyLuminanceFromScreenProc = (COPY_LUMINANCE_FROM_SCREEN_PROC) GetProcAddress(dll, "CopyLuminanceFromScreen");
        CopyLuminanceIntoTextureProc = (COPY_LUMINANCE_INTO_TEXTURE_PROC) GetProcAddress(dll, "CopyLuminanceIntoTexture");
        SetDefaultStateProc = (SET_DEFAULT_STATE_PROC) GetProcAddress(dll, "SetDefaultState");
        SetViewportProc = (SET_VIEWPORT_PROC) GetProcAddress(dll, "SetViewport");
        StartOcclusionQueryProc = (START_OCCLUSION_QUERY_PROC) GetProcAddress(dll, "StartOcclusionQuery");
        EndOcclusionQueryProc = (END_OCCLUSION_QUERY_PROC) GetProcAddress(dll, "EndOcclusionQuery");
        GetOcclusionQueryResultProc = (GET_OCCLUSION_QUERY_RESULT_PROC) GetProcAddress(dll, "GetOcclusionQueryResult");
        SetDepthRangeProc = (SET_DEPTH_RANGE_PROC) GetProcAddress(dll, "SetDepthRange" );
        GetDepthRangeProc = (GET_DEPTH_RANGE_PROC) GetProcAddress(dll, "GetDepthRange" );
        DeviceLostProc = (DEVICE_LOST_PROC) GetProcAddress(dll, "DeviceLost");
        DeviceResetProc = (DEVICE_RESET_PROC) GetProcAddress(dll, "DeviceReset");
        GetNativeTextureProc = (GET_NATIVE_TEXTURE_PROC) GetProcAddress(dll, "GetNativeTexture");
        SetUserShadersProc = (SET_USER_SHADERS_PROC)GetProcAddress(dll, "SetUserShaders");
        GetShaderProgramObjectProc = (GET_SHADER_PROGRAM_OBJECT_PROC)GetProcAddress(dll, "GetShaderProgramObject");
        DeleteShaderProc = (DELETE_SHADER_PROC)GetProcAddress(dll, "DeleteShader");
        BackfaceCullClockwiseProc = (BACKFACE_CULL_CLOCKWISE_PROC)GetProcAddress(dll, "BackfaceCullClockwise");
#else
    if (dll) {
        SetContextProc = (SET_CONTEXT_PROC) dlsym(dll, "SetContext");
        ClearScreenProc = (CLEAR_SCREEN_PROC) dlsym(dll, "ClearScreen");
        ClearDepthProc = (CLEAR_DEPTH_PROC) dlsym(dll, "ClearDepth");
        SetEnvProc = (SET_ENV_PROC) dlsym(dll, "SetEnvironment");
        GetFrustumWorldSpaceProc = (GET_FRUSTUM_WORLD_SPACE_PROC) dlsym(dll, "GetFrustumWorldSpace");
        GetFrustumClipSpaceProc = (GET_FRUSTUM_CLIP_SPACE_PROC) dlsym(dll, "GetFrustumClipSpace");
        LoadShaderFromFileProc = (LOAD_SHADER_FROM_FILE_PROC) dlsym(dll, "LoadShaderFromFile");
        LoadShaderFromSourceProc = (LOAD_SHADER_FROM_SOURCE_PROC) dlsym(dll, "LoadShaderFromSource");
        SetConstantVector4Proc = (SET_CONSTANT_VECTOR4_PROC) dlsym(dll, "SetConstantVector4");
        SetConstantMatrix4Proc = (SET_CONSTANT_MATRIX4_PROC) dlsym(dll, "SetConstantMatrix4");
        BindShaderProc = (BIND_SHADER_PROC) dlsym(dll, "BindShader");
        UnbindShaderProc = (UNBIND_SHADER_PROC) dlsym(dll, "UnbindShader");
        ShutdownShaderSystemProc = (SHUTDOWN_SHADER_SYSTEM_PROC) dlsym(dll, "ShutdownShaderSystem");
        AllocateVertexBufferProc = (ALLOCATE_VERTEX_BUFFER_PROC) dlsym(dll, "AllocateVertexBuffer");
        LockVertexBufferProc = (LOCK_VERTEX_BUFFER_PROC) dlsym(dll, "LockVertexBuffer");
        GetVerticesProc = (GET_VERTICES_PROC) dlsym(dll, "GetVertices");
        UnlockVertexBufferProc = (UNLOCK_VERTEX_BUFFER_PROC) dlsym(dll, "UnlockVertexBuffer");
        GetVertexBufferProc = (GET_VERTEX_BUFFER_PROC) dlsym(dll, "GetVertexBuffer");
        UpdateVertexBufferProc = (UPDATE_VERTEX_BUFFER_PROC) dlsym(dll, "UpdateVertexBuffer");
        ReleaseVertexBufferProc = (RELEASE_VERTEX_BUFFER_PROC) dlsym(dll, "ReleaseVertexBuffer");
        SetVertexBufferProc = (SET_VERTEX_BUFFER_PROC) dlsym(dll, "SetVertexBuffer");
        UnsetVertexBufferProc = (UNSET_VERTEX_BUFFER_PROC) dlsym(dll, "UnsetVertexBuffer");
        AllocateIndexBufferProc = (ALLOCATE_INDEX_BUFFER_PROC) dlsym(dll, "AllocateIndexBuffer");
        LockIndexBufferProc = (LOCK_INDEX_BUFFER_PROC) dlsym(dll, "LockIndexBuffer");
        GetIndicesProc = (GET_INDICES_PROC) dlsym(dll, "GetIndices");
        UnlockIndexBufferProc = (UNLOCK_INDEX_BUFFER_PROC) dlsym(dll, "UnlockIndexBuffer");
        ReleaseIndexBufferProc = (RELEASE_INDEX_BUFFER_PROC) dlsym(dll, "ReleaseIndexBuffer");
        SetIndexBufferProc = (SET_INDEX_BUFFER_PROC) dlsym(dll, "SetIndexBuffer");
        UnsetIndexBufferProc = (UNSET_INDEX_BUFFER_PROC) dlsym(dll, "UnsetIndexBuffer");
        DrawStripProc = (DRAW_STRIP_PROC) dlsym(dll, "DrawStrip");
        DrawPointsProc = (DRAW_POINTS_PROC) dlsym(dll, "DrawPoints");
        EnableDepthWritesProc = (ENABLE_DEPTH_WRITES_PROC) dlsym(dll, "EnableDepthWrites");
        EnableDepthReadsProc = (ENABLE_DEPTH_WRITES_PROC) dlsym(dll, "EnableDepthReads");
        EnableTexture2DProc = (ENABLE_TEXTURE_2D_PROC) dlsym(dll, "EnableTexture2D");
        EnableTexture3DProc = (ENABLE_TEXTURE_3D_PROC) dlsym(dll, "EnableTexture3D");
        EnableBackfaceCullingProc = (ENABLE_BACKFACE_CULLING_PROC) dlsym(dll, "EnableBackfaceCulling");
        EnableFogProc = (ENABLE_FOG_PROC) dlsym(dll, "EnableFog");
        EnableLightingProc = (ENABLE_LIGHTING_PROC) dlsym(dll, "EnableLighting");
        GetTextureMatrixProc = (GET_TEXTURE_MATRIX_PROC) dlsym(dll, "GetTextureMatrix");
        SetTextureMatrixProc = (SET_TEXTURE_MATRIX_PROC) dlsym(dll, "SetTextureMatrix");
        GetProjectionMatrixProc = (GET_PROJECTION_MATRIX_PROC) dlsym(dll, "GetProjectionMatrix");
        SetProjectionMatrixProc = (SET_PROJECTION_MATRIX_PROC) dlsym(dll, "SetProjectionMatrix");
        GetModelviewMatrixProc = (GET_MODELVIEW_MATRIX_PROC) dlsym(dll, "GetModelviewMatrix");
        SetModelviewMatrixProc = (SET_MODELVIEW_MATRIX_PROC) dlsym(dll, "SetModelviewMatrix");
        MultiplyModelviewMatrixProc = (MULTIPLY_MODELVIEW_MATRIX_PROC) dlsym(dll, "MultiplyModelviewMatrix");
        PushAllStateProc = (PUSH_ALL_STATE_PROC) dlsym(dll, "PushAllState");
        PopAllStateProc = (POP_ALL_STATE_PROC) dlsym(dll, "PopAllState");
        EnableBlendingProc = (ENABLE_BLENDING_PROC) dlsym(dll, "EnableBlending");
        DisableBlendingProc = (DISABLE_BLENDING_PROC) dlsym(dll, "DisableBlending");
        LoadTextureFromFileProc = (LOAD_TEXTURE_FROM_FILE_PROC) dlsym(dll, "LoadTextureFromFile");
        EnableTextureProc = (ENABLE_TEXTURE_PROC) dlsym(dll, "EnableTexture");
        Enable3DTextureProc = (ENABLE_3D_TEXTURE_PROC) dlsym(dll, "Enable3DTexture");
        DisableTextureProc = (DISABLE_TEXTURE_PROC) dlsym(dll, "DisableTexture");
        ReleaseTextureProc = (RELEASE_TEXTURE_PROC) dlsym(dll, "ReleaseTexture");
        HasPointSpritesProc = (HAS_POINT_SPRITES_PROC) dlsym(dll, "HasPointSprites");
        EnablePointSpritesProc = (ENABLE_POINT_SPRITES_PROC) dlsym(dll, "EnablePointSprites");
        DisablePointSpritesProc = (DISABLE_POINT_SPRITES_PROC) dlsym(dll, "DisablePointSprites");
        GetViewportProc = (GET_VIEWPORT_PROC) dlsym(dll, "GetViewport");
        GetFOVProc = (GET_FOV_PROC) dlsym(dll, "GetFOV");
        DrawQuadsProc = (DRAW_QUADS_PROC) dlsym(dll, "DrawQuads");
        LoadFloatTextureRGBProc = (LOAD_FLOAT_TEXTURE_RGB_PROC) dlsym(dll, "LoadFloatTextureRGB");
        HasFloatTexturesProc = (HAS_FLOAT_TEXTURES_PROC) dlsym(dll, "HasFloatTextures");
        LoadFloatTextureProc = (LOAD_FLOAT_TEXTURE_PROC) dlsym(dll, "LoadFloatTexture");
        LoadTextureProc = (LOAD_TEXTURE_PROC) dlsym(dll, "LoadTexture");
        Load3DTextureProc = (LOAD_3D_TEXTURE_PROC) dlsym(dll, "Load3DTexture");
        Load3DTextureRGBProc = (LOAD_3D_TEXTURE_RGB_PROC) dlsym(dll, "Load3DTextureRGB");
        Load3DTextureLAProc = (LOAD_3D_TEXTURE_LA_PROC) dlsym(dll, "Load3DTextureLA");
        SubLoad3DTextureLAProc = (SUBLOAD_3D_TEXTURE_LA_PROC) dlsym(dll, "SubLoad3DTextureLA");
        InitRenderTargetProc = (INIT_RENDER_TARGET_PROC) dlsym(dll, "InitRenderTarget");
        InitRenderTextureProc = (INIT_RENDER_TEXTURE_PROC) dlsym(dll, "InitRenderTexture");
        MakeRenderTargetCurrentProc = (MAKE_RENDER_TARGET_CURRENT_PROC) dlsym(dll, "MakeRenderTargetCurrent");
        MakeRenderTextureCurrentProc = (MAKE_RENDER_TEXTURE_CURRENT_PROC) dlsym(dll, "MakeRenderTextureCurrent");
        RestoreRenderTargetProc = (RESTORE_RENDER_TARGET_PROC) dlsym(dll, "RestoreRenderTarget");
        BindRenderTextureProc = (BIND_RENDER_TEXTURE_PROC) dlsym(dll, "BindRenderTexture");
        GetRenderTextureTextureHandleProc = (GET_RENDER_TEXTURE_TEXTURE_HANDLE_PROC) dlsym(dll, "GetRenderTextureTextureHandle");
        ReleaseRenderTextureProc = (RELEASE_RENDER_TEXTURE_PROC) dlsym(dll, "ReleaseRenderTexture");
        ReleaseRenderTargetProc = (RELEASE_RENDER_TARGET_PROC) dlsym(dll, "ReleaseRenderTarget");
        InitRenderTextureCubeProc = (INIT_RENDER_TEXTURE_CUBE_PROC)dlsym(dll, "InitRenderTextureCube");
        MakeRenderTextureCubeCurrentProc = (MAKE_RENDER_TEXTURE_CUBE_CURRENT_PROC)dlsym(dll, "MakeRenderTextureCubeCurrent");
        BindRenderTextureCubeProc = (BIND_RENDER_TEXTURE_CUBE_PROC)dlsym(dll, "BindRenderTextureCube");
        GetRenderTextureCubeTextureHandleProc = (GET_RENDER_TEXTURE_CUBE_TEXTURE_HANDLE_PROC)dlsym(dll, "GetRenderTextureCubeTextureHandle");
        ReleaseRenderTextureCubeProc = (RELEASE_RENDER_TEXTURE_CUBE_PROC)dlsym(dll, "ReleaseRenderTextureCube");
        GetPixelsProc = (GET_PIXELS_PROC) dlsym(dll, "GetPixels");
        SetPixelsProc = (SET_PIXELS_PROC) dlsym(dll, "SetPixels");
        DrawAALineProc = (DRAW_AA_LINE_PROC) dlsym(dll, "DrawAALine");
        DrawAALinesProc = (DRAW_AA_LINES_PROC) dlsym(dll, "DrawAALines");
        Set2DOrthoMatrixProc = (SET_2D_ORTHO_MATRIX_PROC) dlsym(dll, "Set2DOrthoMatrix");
        HasQuadsProc = (HAS_QUADS_PROC) dlsym(dll, "HasQuads");
        SetCurrentColorProc = (SET_CURRENT_COLOR_PROC) dlsym(dll, "SetCurrentColor");
        SetOrthoMatrixProc = (SET_ORTHO_MATRIX_PROC) dlsym(dll, "SetOrthoMatrix");
        ConfigureFogProc = (CONFIGURE_FOG_PROC) dlsym(dll, "ConfigureFog");
        CreateLuminanceTextureProc = (CREATE_LUMINANCE_TEXTURE_PROC) dlsym(dll, "CreateLuminanceTexture");
        CopyLuminanceFromScreenProc = (COPY_LUMINANCE_FROM_SCREEN_PROC) dlsym(dll, "CopyLuminanceFromScreen");
        CopyLuminanceIntoTextureProc = (COPY_LUMINANCE_INTO_TEXTURE_PROC) dlsym(dll, "CopyLuminanceIntoTexture");
        SetDefaultStateProc = (SET_DEFAULT_STATE_PROC) dlsym(dll, "SetDefaultState");
        SetViewportProc = (SET_VIEWPORT_PROC) dlsym(dll, "SetViewport");
        StartOcclusionQueryProc = (START_OCCLUSION_QUERY_PROC) dlsym(dll, "StartOcclusionQuery");
        EndOcclusionQueryProc = (END_OCCLUSION_QUERY_PROC) dlsym(dll, "EndOcclusionQuery");
        GetOcclusionQueryResultProc = (GET_OCCLUSION_QUERY_RESULT_PROC) dlsym(dll, "GetOcclusionQueryResult");
        SetDepthRangeProc = (SET_DEPTH_RANGE_PROC) dlsym(dll, "SetDepthRange" );
        GetDepthRangeProc = (GET_DEPTH_RANGE_PROC) dlsym(dll, "GetDepthRange" );
        DeviceLostProc = (DEVICE_LOST_PROC) dlsym(dll, "DeviceLost");
        DeviceResetProc = (DEVICE_RESET_PROC) dlsym(dll, "DeviceReset");
        GetNativeTextureProc = (GET_NATIVE_TEXTURE_PROC) dlsym(dll, "GetNativeTexture");
        SetUserShadersProc = (SET_USER_SHADERS_PROC)dlsym(dll, "SetUserShaders");
        GetShaderProgramObjectProc = (GET_SHADER_PROGRAM_OBJECT_PROC)dlsym(dll, "GetShaderProgramObject");
        DeleteShaderProc = (DELETE_SHADER_PROC)dlsym(dll, "DeleteShader");
        BackfaceCullClockwiseProc = (BACKFACE_CULL_CLOCKWISE_PROC)dlsym(dll, "BackfaceCullClockwise");
#endif
        if (SetEnvProc) {
            *context = SetEnvProc(rightHanded, environment, Atmosphere::GetResourceLoader(), Allocator::GetAllocator(), Atmosphere::GetUserDefinedVertString(), Atmosphere::GetUserDefinedFragString());
            if (!(*context)) {
                return Atmosphere::E_CANT_INITIALIZE_RENDERER_SUBSYSTEM;
            }
        }
    }

    FrameStarted(*context);

    if (dll == 0) {
        return Atmosphere::E_CANT_LOAD_RENDERER_DLL;
    } else {
        return Atmosphere::E_NOERROR;
    }
#endif

//#endif
}

void Renderer::DeviceLost(void *context)
{
    FN_TIMER

    if (DeviceLostProc) {
        DeviceLostProc(context);
    }
}

void Renderer::DeviceReset(void *context)
{
    FN_TIMER

    if (DeviceResetProc) {
        DeviceResetProc(context);
    }
}

void Renderer::ClearScreen(const Color& color)
{
    FN_TIMER

    if (ClearScreenProc) {
        ClearScreenProc(color);
    }

}

void Renderer::ClearDepth()
{
    FN_TIMER


    if (ClearDepthProc) {
        ClearDepthProc();
    }
}

bool Renderer::ExtractViewFrustum(Frustum& pfrustum)
{
    FN_TIMER
    pfrustum = frustum;

    return true;
}

void Renderer::SetUserShaders(const SL_VECTOR(unsigned int)& userShaders)
{
    if (SetUserShadersProc) {
        SetUserShadersProc(userShaders);
    }
}

unsigned int Renderer::GetShaderProgram(ShaderHandle shader)
{
    if (GetShaderProgramObjectProc) {
        return GetShaderProgramObjectProc(shader);
    }

    return 0;
}

ShaderHandle Renderer::LoadShaderFromFile(const char *filename, int shaderType)
{
    FN_TIMER
    bool noShaders = false;
    Configuration::GetBoolValue("disable-shaders", noShaders);

    if (!noShaders) {
        if (LoadShaderFromFileProc) {
            return LoadShaderFromFileProc(filename, shaderType);
        }
    }

    return 0;
}

ShaderHandle Renderer::LoadShaderFromSource(const char *source, int shaderType)
{
    FN_TIMER
    bool noShaders = false;
    Configuration::GetBoolValue("disable-shaders", noShaders);

    if (!noShaders) {
        if (LoadShaderFromSourceProc) {
            return LoadShaderFromSourceProc(source, shaderType);
        }
    }

    return 0;
}

void Renderer::DeleteShader(ShaderHandle shader)
{
    if (DeleteShaderProc && shader) {
        DeleteShaderProc(shader);
    }
}

bool Renderer::SetConstantVector(ShaderHandle shader, const char *varName, const Vector3& vec)
{
    FN_TIMER
    if (SetConstantVector4Proc && shader) {
        float x[4];
        x[0] = (float)vec.x;
        x[1] = (float)vec.y;
        x[2] = (float)vec.z;
        x[3] = 1.0f;

        return SetConstantVector4Proc(shader, varName, x);
    }

    return false;
}

bool Renderer::SetConstantVector4(ShaderHandle shader, const char *varName, const Vector4& vec)
{
    FN_TIMER
    if (SetConstantVector4Proc && shader) {
        float x[4];
        x[0] = (float)vec.x;
        x[1] = (float)vec.y;
        x[2] = (float)vec.z;
        x[3] = (float)vec.w;

        return SetConstantVector4Proc(shader, varName, x);
    }

    return false;
}

bool Renderer::SetConstantMatrix(ShaderHandle shader, const char *varName, const Matrix4& matrix)
{
    FN_TIMER
    if (SetConstantMatrix4Proc && shader) {
        float m[16];
        int i = 0;
        for (int row = 0; row < 4; row++) {
            for (int col = 0; col < 4; col++) {
                m[i++] = (float)(matrix.elem[row][col]);
            }
        }

        return SetConstantMatrix4Proc(shader, varName, m);
    }

    return false;
}

bool Renderer::BindShader(ShaderHandle shader, int shaderType)
{
    FN_TIMER
    if (BindShaderProc && shader) {
        lastShader = shader;
        return BindShaderProc(shader, shaderType);
    }

    return false;
}


bool Renderer::UnbindShader(int shaderType)
{
    FN_TIMER
    if (UnbindShaderProc) {
        lastShader = 0;
        return UnbindShaderProc(shaderType);
    }

    return false;
}

bool Renderer::Shutdown(void *context)
{
    FN_TIMER

    if (ShutdownShaderSystemProc) {
        return ShutdownShaderSystemProc(context);
    }

    return false;
}

VertexBufferHandle Renderer::AllocateVertexBuffer(int nVerts)
{
    FN_TIMER
    if (AllocateVertexBufferProc) {
        return AllocateVertexBufferProc(nVerts);
    }

    return 0;
}

bool Renderer::LockVertexBuffer(VertexBufferHandle bufferHandle)
{
    FN_TIMER
    if (LockVertexBufferProc) {
        return LockVertexBufferProc(bufferHandle);
    }

    return false;
}

Vertex *Renderer::GetVertices(VertexBufferHandle bufferHandle)
{
    FN_TIMER
    if (GetVerticesProc) {
        return GetVerticesProc(bufferHandle);
    }

    return 0;
}

bool Renderer::UnlockVertexBuffer(VertexBufferHandle bufferHandle)
{
    FN_TIMER
    if (UnlockVertexBufferProc) {
        return UnlockVertexBufferProc(bufferHandle);
    }

    return false;
}

bool Renderer::UpdateVertexBuffer(VertexBufferHandle bufferHandle, int offset, Vertex *verts, int nVerts)
{
    FN_TIMER

    bool updateOK = false;
    if (UpdateVertexBufferProc) {
        updateOK = UpdateVertexBufferProc(bufferHandle, offset, verts, nVerts);
    }

    if (!updateOK) {
        if (LockVertexBuffer(bufferHandle)) {
            Vertex *v = GetVertices(bufferHandle);
            if (v) {
                v = v + offset;
                for (int i = 0; i < nVerts; i++) {
                    v[i] = verts[i];
                }
            }
            UnlockVertexBuffer(bufferHandle);
            return true;
        }
    }

    return false;
}

bool Renderer::GetVertexBuffer(VertexBufferHandle bufferHandle, int offset, Vertex *verts, int nVerts)
{
    FN_TIMER

    bool getOK = false;
    if (GetVertexBufferProc) {
        getOK = GetVertexBufferProc(bufferHandle, offset, verts, nVerts);
    }

    if (!getOK) {
        if (LockVertexBuffer(bufferHandle)) {
            Vertex *v = GetVertices(bufferHandle) + offset;
            for (int i = 0; i < nVerts; i++) {
                verts[i] = v[i];
            }
            UnlockVertexBuffer(bufferHandle);
            return true;
        }
    } else {
        return true;
    }

    return false;
}

bool Renderer::ReleaseVertexBuffer(VertexBufferHandle bufferHandle)
{
    FN_TIMER
    if (ReleaseVertexBufferProc) {
        return ReleaseVertexBufferProc(bufferHandle);
    }

    return false;
}

bool Renderer::SetVertexBuffer(VertexBufferHandle bufferHandle, bool vertexColors)
{
    FN_TIMER
    if (SetVertexBufferProc) {
        return SetVertexBufferProc(bufferHandle, vertexColors);
    }

    return false;
}

bool Renderer::UnsetVertexBuffer()
{
    FN_TIMER
    if (UnsetVertexBufferProc) {
        return UnsetVertexBufferProc();
    }

    return false;
}

/** Index Buffers */
IndexBufferHandle Renderer::AllocateIndexBuffer(int nVerts)
{
    FN_TIMER
    if (AllocateIndexBufferProc) {
        return AllocateIndexBufferProc(nVerts);
    }

	IndexBufferHandle handle = 0;
    return handle;
}

bool Renderer::LockIndexBuffer(IndexBufferHandle ib)
{
    FN_TIMER
    if (LockIndexBufferProc) {
        return LockIndexBufferProc(ib);
    }

    return false;
}

Index *Renderer::GetIndices(IndexBufferHandle ib)
{
    FN_TIMER
    if (GetIndicesProc) {
        return GetIndicesProc(ib);
    }

    return 0;
}

bool Renderer::UnlockIndexBuffer(IndexBufferHandle ib)
{
    FN_TIMER
    if (UnlockIndexBufferProc) {
        return UnlockIndexBufferProc(ib);
    }

    return false;
}

bool Renderer::ReleaseIndexBuffer(IndexBufferHandle ib)
{
    FN_TIMER
    if (ReleaseIndexBufferProc) {
        return ReleaseIndexBufferProc(ib);
    }

    return false;
}

bool Renderer::SetIndexBuffer(IndexBufferHandle ib)
{
    FN_TIMER
    if (SetIndexBufferProc) {
        return SetIndexBufferProc(ib);
    }

    return false;
}

bool Renderer::UnsetIndexBuffer()
{
    FN_TIMER
    if (UnsetIndexBufferProc) {
        return UnsetIndexBufferProc();
    }

    return false;
}

/** Drawing methods */
bool Renderer::DrawStrip(VertexBufferHandle vb, IndexBufferHandle ib, int startIdx, int nIndices,
                         int nVerts, bool vertColors)
{
    FN_TIMER
    bool ok = false;
    if (DrawStripProc) {
        if (SetVertexBuffer(vb, vertColors)) {
            if (SetIndexBuffer(ib)) {
                ok = DrawStripProc(ib, startIdx, nIndices, nVerts);
                UnsetIndexBuffer();
            }
            UnsetVertexBuffer();
        }
    }

    return ok;
}

bool Renderer::DrawStripDirect(IndexBufferHandle ib, int startIdx, int nIndices, int nVerts)
{
    FN_TIMER
    bool ok = false;
    if (DrawStripProc) {
        ok = DrawStripProc(ib, startIdx, nIndices, nVerts);
    }

    return ok;
}

bool Renderer::DrawPoints(VertexBufferHandle vb, double pointSize, int nPoints, int start, bool vertColors)
{
    FN_TIMER
    bool ok = false;
    if (DrawPointsProc) {
        if (SetVertexBuffer(vb, vertColors)) {
            ok = DrawPointsProc(pointSize, nPoints, start);
            UnsetVertexBuffer();
        }
    }

    return ok;
}

bool Renderer::HasQuads()
{
    FN_TIMER
    if (HasQuadsProc) {
        return HasQuadsProc();
    }

    return false;
}

bool Renderer::DrawQuads(VertexBufferHandle vb, int nPoints, int start, bool vertColors)
{
    FN_TIMER
    bool ok = false;

    if (DrawQuadsProc) {
        if (SetVertexBuffer(vb, vertColors)) {
            ok = DrawQuadsProc(nPoints, start);
            UnsetVertexBuffer();
        }

    }

    return ok;
}

/** State methods */
bool Renderer::EnableDepthWrites(bool enable)
{
    FN_TIMER

    if (disableDepthWrites) {
        return true;
    }

    if (EnableDepthWritesProc) {
        return EnableDepthWritesProc(enable);
    }

    return false;
}

bool Renderer::EnableDepthReads(bool enable)
{
    FN_TIMER
    if (disableDepthReads) {
        return true;
    }

    if (EnableDepthReadsProc) {
        return EnableDepthReadsProc(enable);
    }

    return false;
}

bool Renderer::EnableTexture2D(bool enable)
{
    FN_TIMER
    if (EnableTexture2DProc) {
        return EnableTexture2DProc(enable);
    }

    return false;
}

bool Renderer::EnableTexture3D(bool enable)
{
    FN_TIMER
    if (EnableTexture3DProc) {
        return EnableTexture3DProc(enable);
    }

    return false;
}

bool Renderer::EnableBackfaceCulling(bool enable)
{
    FN_TIMER

    if (enable != cullingEnabled || cullingUndefined) {
        cullingEnabled = enable;
        cullingUndefined = false;

        if (EnableBackfaceCullingProc) {
            return EnableBackfaceCullingProc(enable);
        } else {
            return false;
        }
    }

    return true;
}

bool Renderer::BackfaceCullClockwise(bool cullCW)
{
    FN_TIMER

    if (BackfaceCullClockwiseProc) {
        return BackfaceCullClockwiseProc(cullCW);
    } else {
        return false;
    }
}

bool Renderer::EnableFog(bool enable)
{
    FN_TIMER

    fogEnabled = enable;

    if (EnableFogProc) {
        return EnableFogProc(enable);
    }

    return false;
}

bool Renderer::ConfigureFog(double density, double start, double end, const Color& c)
{
    fogDensity = density;
    fogStart = start;
    fogEnd = end;
    fogColor = c;

    if (ConfigureFogProc) {
        return ConfigureFogProc(density, start, end, c);
    }

    return false;
}

void Renderer::GetFog(double& density, double &start, double &end, Color& c)
{
    density = fogDensity;
    start = fogStart;
    end = fogEnd;
    c = fogColor;
}

bool Renderer::EnableLighting(bool enable)
{
    FN_TIMER
    if (EnableLightingProc) {
        return EnableLightingProc(enable);
    }

    return false;
}

bool Renderer::SetCurrentColor(const Color& c)
{
    FN_TIMER
    if (SetCurrentColorProc) {
        return SetCurrentColorProc(c);
    }

    return false;
}

bool Renderer::StartOcclusionQuery(QueryHandle *q)
{
    FN_TIMER
    if (StartOcclusionQueryProc) {
        return StartOcclusionQueryProc(q);
    }

    return false;
}

bool Renderer::EndOcclusionQuery(QueryHandle q)
{
    FN_TIMER
    if (EndOcclusionQueryProc) {
        return EndOcclusionQueryProc(q);
    }

    return false;
}

unsigned int Renderer::GetOcclusionQueryResults(QueryHandle q)
{
    FN_TIMER
    if (GetOcclusionQueryResultProc) {
        return GetOcclusionQueryResultProc(q);
    }

    return 0;
}

bool Renderer::PushAllState()
{
    FN_TIMER
    if (PushAllStateProc) {
        return PushAllStateProc();
    }

    return false;
}

bool Renderer::PopAllState()
{
    FN_TIMER
    if (PopAllStateProc) {
        return PopAllStateProc();
    }

    lastSrc = lastDst = -1;
    cullingUndefined = true;
    lastTex = 0;
    lastShader = 0;

    return false;
}

bool Renderer::SetDefaultState()
{
    FN_TIMER

    cullingUndefined = true;
    blendingEnabled = false;
    lastSrc = lastDst = -1;
    lastTex = 0;
    lastShader = 0;

    static bool doDefault = true;
    static bool configRead = false;
    if (!configRead) {
        Configuration::GetBoolValue("set-default-state", doDefault);
        configRead = true;
    }

    if (doDefault && SetDefaultStateProc) {
        blendingEnabled = false;
        return SetDefaultStateProc();
    }

    return false;
}

bool Renderer::GetTextureMatrix(Matrix4 *m)
{
    FN_TIMER
    *m = texture;

    return true;
}

bool Renderer::SetTextureMatrix(const Matrix4& m)
{
    FN_TIMER
    if (SetTextureMatrixProc) {
        texture = m;
        return SetTextureMatrixProc(m);
    }
    return false;
}

bool Renderer::GetProjectionMatrix(Matrix4 *m)
{
    FN_TIMER
    *m = projection;

    return true;
}

bool Renderer::SetProjectionMatrix(const Matrix4& m)
{
    FN_TIMER
    if (SetProjectionMatrixProc) {
        projection = m;
        SetProjectionMatrixProc(m);

        if (GetFrustumWorldSpaceProc) {
            return GetFrustumWorldSpaceProc(frustum);
        }
    }

    return false;
}

bool Renderer::SetInfiniteProjectionMatrix(double nearClip, double fovy, double aspect)
{
    double e = 1.0 / tan(fovy * 0.5);
    const double epsilon = 2.4E-7;
    double zScale = type == Atmosphere::OPENGL ? 2.0 : 1.0;

    Matrix4 proj;

    if (rightHanded) {
        proj.elem[0][0] = e;
        proj.elem[0][1] = 0;
        proj.elem[0][2] = 0;
        proj.elem[0][3] = 0;

        proj.elem[1][0] = 0;
        proj.elem[1][1] = e / aspect;
        proj.elem[1][2] = 0;
        proj.elem[1][3] = 0;

        proj.elem[2][0] = 0;
        proj.elem[2][1] = 0;
        proj.elem[2][2] = epsilon - 1.0;
        proj.elem[2][3] = (epsilon - zScale) * nearClip;

        proj.elem[3][0] = 0;
        proj.elem[3][1] = 0;
        proj.elem[3][2] = -1.0;
        proj.elem[3][3] = 0;
    } else {
        proj.elem[0][0] = e / aspect;
        proj.elem[0][1] = 0;
        proj.elem[0][2] = 0;
        proj.elem[0][3] = 0;
        proj.elem[1][0] = 0;
        proj.elem[1][1] = e;
        proj.elem[1][2] = 0;
        proj.elem[1][3] = 0;
        proj.elem[2][0] = 0;
        proj.elem[2][1] = 0;
        proj.elem[2][2] = 1.0;
        proj.elem[2][3] = (epsilon - zScale) * nearClip;
        proj.elem[3][0] = 0;
        proj.elem[3][1] = 0;
        proj.elem[3][2] = 1.0 + epsilon;
        proj.elem[3][3] = 0;
    }

    return SetProjectionMatrix(proj);
}

bool Renderer::GetNearFarClip(double &znear, double &zfar)
{
    if (GetFrustumClipSpaceProc) {
        Frustum f;
        GetFrustumClipSpaceProc(f);
        zfar = f.GetPlane(Frustum::PBACK).GetDistance();
        znear = f.GetPlane(Frustum::PNEAR).GetDistance() * -1.0;
        return true;
    }

    return false;
}

//static double sgn(float x)
//{
//    return (x > 0.0) ? 1.0 : ((x < 0.0) ? -1.0 : 0.0);
//}

bool Renderer::AdjustNearFarClip(double znear, double zfar)
{
    /*
    // Calculate the clip-space corner point opposite the clipping plane
    // as (sgn(clipPlane.x), sgn(clipPlane.y), 1, 1) and
    // transform it into camera space by multiplying it
    // by the inverse of the projection matrix
    Vector4 clipPlane(0, 0, -1, -znear);

    Vector4 q;
    q.x = (sgn(clipPlane.x) + projection.elem[2][0]) / projection.elem[0][0];
    q.y = (sgn(clipPlane.y) + projection.elem[2][1]) / projection.elem[1][1];
    q.z = -1.0F;
    q.w = (1.0F + projection.elem[2][2]) / projection.elem[2][3];

    // Calculate the scaled plane vector
    Vector4 c = clipPlane * (2.0F / clipPlane.Dot(q));

    // Replace the third row of the projection matrix
    projection.elem[0][2] = c.x;
    projection.elem[1][2] = c.y;
    projection.elem[2][2] = c.z + 1.0F;
    projection.elem[3][2] = c.w;

    SetProjectionMatrixProc(projection);

    if (GetFrustumWorldSpaceProc)
    {
        return GetFrustumWorldSpaceProc(frustum);
    }
    */

    if (GetFrustumClipSpaceProc) {
        Frustum f;
        GetFrustumClipSpaceProc(f);

        double l, r, t, b;
        Vector3 pn = f.GetPlane(Frustum::PLEFT).GetNormal();
        l = (pn.z * znear) / pn.x;
        pn = f.GetPlane(Frustum::PRIGHT).GetNormal();
        r = (pn.z * znear) / pn.x;
        pn = f.GetPlane(Frustum::PTOP).GetNormal();
        t = (pn.z * znear) / pn.y;
        pn = f.GetPlane(Frustum::PBOTTOM).GetNormal();
        b = (pn.z * znear) / pn.y;

        if (!rightHanded) {
            l = -l;
            r = -r;
            t = -t;
            b = -b;
        }

        SetProjectionMatrix(l, r, t, b, znear, zfar);
    }


    return false;
}

bool Renderer::SetProjectionMatrix(double l, double r, double t, double b, double zn, double zf)
{
    Matrix4 proj;

    double zScale = GetIsDirectX() ? 1.0 : 2.0;

    if (rightHanded) {
        proj.elem[0][0] = (2.0*zn)/(r-l);
        proj.elem[0][1] = 0;
        proj.elem[0][2] = (r+l)/(r-l);
        proj.elem[0][3] = 0;
        proj.elem[1][0] = 0;
        proj.elem[1][1] = (2.0*zn)/(t-b);
        proj.elem[1][2] = (t+b)/(t-b);
        proj.elem[1][3] = 0;
        proj.elem[2][0] = 0;
        proj.elem[2][1] = 0;
        proj.elem[2][2] = -((zf+zn)/(zf-zn));
        proj.elem[2][3] = -((zScale*zn*zf)/(zf-zn));
        proj.elem[3][0] = 0;
        proj.elem[3][1] = 0;
        proj.elem[3][2] = -1;
        proj.elem[3][3] = 0;
    } else {
        proj.elem[0][0] = (2.0*zn)/(r-l);
        proj.elem[0][1] = 0;
        proj.elem[0][2] = 0;
        proj.elem[0][3] = 0;
        proj.elem[1][0] = 0;
        proj.elem[1][1] = (2.0*zn)/(t-b);
        proj.elem[1][2] = 0;
        proj.elem[1][3] = 0;
        proj.elem[2][0] = (l+r)/(l-r);
        proj.elem[2][1] = (t+b)/(b-t);
        proj.elem[2][2] = zf/(zf-zn);
        proj.elem[2][3] = 1;
        proj.elem[3][0] = 0;
        proj.elem[3][1] = 0;
        proj.elem[3][2] = (zScale*zn*zf)/(zn-zf);
        proj.elem[3][3] = 0;

        proj.Transpose();
    }

    return SetProjectionMatrix(proj);
}

bool Renderer::SetProjectionMatrix(double nearClip, double farClip, double fovx, double fovy)
{
    FN_TIMER
    double l, r, t, b, n, f;

    n = nearClip;
    f = farClip;

    double halfAngle = fovx * 0.5;
    r = n * tan(halfAngle);
    l = -r;
    halfAngle = fovy * 0.5;
    t = n * tan(halfAngle);
    b = -t;

    return SetProjectionMatrix(l, r, t, b, n, f);
}

bool Renderer::SetOrthoMatrix(double left, double right, double bottom, double top, double pnear, double pfar)
{
    FN_TIMER

    Matrix4 m;

    if (rightHanded) {
        m.elem[0][0] = 2.0 / (right - left);
        m.elem[0][3] = -((right + left) / (right - left));

        m.elem[1][1] = 2.0 / (top - bottom);
        m.elem[1][3] = -((top + bottom) / (top - bottom));

        m.elem[2][2] = -2.0 / (pfar - pnear);
        m.elem[2][3] = -((pfar + pnear) / (pfar - pnear));
    } else {
        m.elem[0][0] = 2.0 / (right - left);
        m.elem[1][1] = 2.0 / (top - bottom);
        m.elem[2][2] = 1.0 / (pfar - pnear);
        m.elem[3][2] = pnear / (pnear - pfar);

        m.Transpose();
    }

    return SetProjectionMatrix(m);
}

bool Renderer::Set2DOrthoMatrix(double w, double h)
{
    FN_TIMER

    return SetOrthoMatrix(0, w, 0, h, 0, 1);
}

bool Renderer::GetModelviewMatrix(Matrix4 *m)
{
    FN_TIMER
    *m = modelview;

    return true;
}

bool Renderer::SetModelviewMatrix(const Matrix4& m)
{
    FN_TIMER
    if (SetModelviewMatrixProc) {
        modelview = m;
        return SetModelviewMatrixProc(m);
    }

    return false;
}

bool Renderer::SetModelviewLookat(const Vector3& eye, const Vector3& at, const Vector3& up)
{
    Vector3 view;

    if (rightHanded)
        view = eye - at;
    else
        view = at - eye;

    view.Normalize();

    Vector3 right = up.Cross(view);
    right.Normalize();

    Vector3 vup = view.Cross(right);

    Matrix4 m;
    m.elem[0][0] = right.x;
    m.elem[0][1] = right.y;
    m.elem[0][2] = right.z;
    m.elem[0][3] = -right.Dot(eye);
    m.elem[1][0] = vup.x;
    m.elem[1][1] = vup.y;
    m.elem[1][2] = vup.z;
    m.elem[1][3] = -vup.Dot(eye);
    m.elem[2][0] = view.x;
    m.elem[2][1] = view.y;
    m.elem[2][2] = view.z;

    m.elem[2][3] = -view.Dot(eye);

    return SetModelviewMatrix(m);
}

bool Renderer::MultiplyModelviewMatrix(const Matrix4& m)
{
    FN_TIMER
    if (MultiplyModelviewMatrixProc) {
        modelview = modelview * m;
        return MultiplyModelviewMatrixProc(m);
    }

    return false;
}

bool Renderer::PushModelviewTransform()
{
    FN_TIMER
    modelviewStack.push(modelview);
    return true;
}

bool Renderer::PopModelviewTransform()
{
    FN_TIMER
    modelview = modelviewStack.top();
    modelviewStack.pop();
    return SetModelviewMatrix(modelview);
}

bool Renderer::PushTransforms()
{
    FN_TIMER
    modelviewStack.push(modelview);
    projectionStack.push(projection);
    return true;
}

bool Renderer::PopTransforms()
{
    FN_TIMER
    modelview = modelviewStack.top();
    modelviewStack.pop();
    projection = projectionStack.top();
    projectionStack.pop();

    SetModelviewMatrix(modelview);
    SetProjectionMatrix(projection);

    return true;
}

bool Renderer::ProjectToScreen(const Vector3& pt, Vector3 *scr)
{
    FN_TIMER
    Vector4 pt4(pt.x, pt.y, pt.z, 1.0);

    pt4 = modelview * pt4;
    pt4 = projection * pt4;

    if (pt4.w == 0.0)
        return false;

    pt4.x /= pt4.w;
    pt4.y /= pt4.w;
    pt4.z /= pt4.w;

    scr->x = vpX + (1 + pt4.x) * vpW / 2;
    scr->y = vpY + (1 + pt4.y) * vpH / 2;
    scr->z = minZ + (1 + pt4.z) * (maxZ - minZ) / 2;

    return true;
}

bool Renderer::EnableBlending(int src, int dst, bool force)
{
    FN_TIMER

    if (!force) {
        if (blendingEnabled && src == lastSrc && dst == lastDst) {
            return true;
        }
    }

    if (EnableBlendingProc) {
        blendingEnabled = true;
        lastSrc = src;
        lastDst = dst;

        return EnableBlendingProc(src,dst);
    }

    return false;
}

bool Renderer::DisableBlending()
{
    FN_TIMER
    if (DisableBlendingProc) {
        blendingEnabled = false;
        lastSrc = lastDst = -1;
        return DisableBlendingProc();
    }

    return false;
}

bool Renderer::LoadTextureFromFile(const char *imgPath, TextureHandle *tex, bool repeatU, bool repeatV)
{
    FN_TIMER
    if (LoadTextureFromFileProc) {
        return LoadTextureFromFileProc(imgPath, tex, repeatU, repeatV);
    }

    return false;
}

bool Renderer::HasFloatTextures()
{
    FN_TIMER
    if (HasFloatTexturesProc) {
        return HasFloatTexturesProc();
    }

    return false;
}

bool Renderer::LoadFloatTextureRGB(float *data, int width, int height, TextureHandle *texture)
{
    FN_TIMER
    if (LoadFloatTextureRGBProc) {
        return LoadFloatTextureRGBProc(data, width, height, texture);
    }

    return false;
}

bool Renderer::LoadFloatTexture(float *data, int width, int height, TextureHandle *texture)
{
    FN_TIMER
    if (LoadFloatTextureProc) {
        return LoadFloatTextureProc(data, width, height, texture);
    }

    return false;
}

bool Renderer::LoadTexture(unsigned char *data, int width, int height, TextureHandle *texture, bool repeatU, bool repeatV)
{
    FN_TIMER
    if (LoadTextureProc) {
        return LoadTextureProc(data, width, height, texture, repeatU, repeatV);
    }

    return false;
}

bool Renderer::Load3DTexture(unsigned char *data, int width, int height, int depth, TextureHandle *texture,
                             bool repeatU, bool repeatV, bool repeatR)
{
    FN_TIMER
    if (Load3DTextureProc) {
        return Load3DTextureProc(data, width, height, depth, texture, repeatU, repeatV, repeatR);
    }

    return false;
}

bool Renderer::Load3DTextureRGB(unsigned char *data, int width, int height, int depth, TextureHandle *texture,
                                bool repeatU, bool repeatV, bool repeatR)
{
    FN_TIMER
    if (Load3DTextureRGBProc) {
        return Load3DTextureRGBProc(data, width, height, depth, texture, repeatU, repeatV, repeatR);
    }

    return false;
}

bool Renderer::Load3DTextureLA(unsigned char *data, int width, int height, int depth, TextureHandle *texture,
                               bool repeatU, bool repeatV, bool repeatR)
{
    FN_TIMER
    if (Load3DTextureLAProc) {
        return Load3DTextureLAProc(data, width, height, depth, texture, repeatU, repeatV, repeatR);
    }

    return false;
}

bool Renderer::SubLoad3DTextureLA(unsigned char *data, int width, int height, int depth, int x, int y, int z,
                                  int dataRowPitch, int dataSlicePitch, TextureHandle texture )
{
    FN_TIMER
    if (SubLoad3DTextureLAProc) {
        return SubLoad3DTextureLAProc(data, width, height, depth, x, y, z, dataRowPitch, dataSlicePitch, texture );
    }

    return false;
}


bool Renderer::EnableTexture(TextureHandle tex, int stage)
{
    FN_TIMER
    if (EnableTextureProc) {
        lastTex = tex;
        return EnableTextureProc(tex, stage);
    }

    return false;
}

bool Renderer::Enable3DTexture(TextureHandle tex, int stage)
{
    FN_TIMER
    if (Enable3DTextureProc) {
        lastTex = tex;
        return Enable3DTextureProc(tex, stage);
    }

    return false;
}

bool Renderer::DisableTexture(int stage)
{
    FN_TIMER
    if (DisableTextureProc) {
        lastTex = 0;
        return DisableTextureProc(stage);
    }

    return false;
}

bool Renderer::ReleaseTexture(TextureHandle tex)
{
    FN_TIMER
    if (ReleaseTextureProc) {
        return ReleaseTextureProc(tex);
    }

    return false;
}

bool Renderer::HasPointSprites()
{
    FN_TIMER
    if (HasPointSpritesProc) {
        return HasPointSpritesProc();
    }

    return false;
}

bool Renderer::EnablePointSprites(double pointSize)
{
    FN_TIMER
    if (EnablePointSpritesProc) {
        return EnablePointSpritesProc(pointSize);
    }

    return false;
}

bool Renderer::DisablePointSprites()
{
    FN_TIMER
    if (DisablePointSpritesProc) {
        return DisablePointSpritesProc();
    }

    return false;
}

bool Renderer::InitRenderTexture(int w, int h, RenderTextureHandle *texture)
{
    FN_TIMER
    if (InitRenderTextureProc) {
        return InitRenderTextureProc(w, h, texture);
    }

    return false;
}

bool Renderer::MakeRenderTextureCurrent(RenderTextureHandle texture, bool clear)
{
    FN_TIMER
    if (MakeRenderTextureCurrentProc) {
        return MakeRenderTextureCurrentProc(texture, clear);
    }

    return false;
}

bool Renderer::BindRenderTexture(RenderTextureHandle texture)
{
    FN_TIMER
    if (BindRenderTextureProc) {
        return BindRenderTextureProc(texture);
    }

    return false;
}

bool Renderer::GetRenderTextureTextureHandle(RenderTextureHandle renTexture, TextureHandle *texture)
{
    FN_TIMER
    if (GetRenderTextureTextureHandleProc) {
        return GetRenderTextureTextureHandleProc(renTexture, texture);
    }

    return false;
}

bool Renderer::ReleaseRenderTexture(RenderTextureHandle texture)
{
    FN_TIMER
    if (ReleaseRenderTextureProc) {
        return ReleaseRenderTextureProc(texture);
    }

    return false;
}

bool Renderer::InitRenderTextureCube(int w, int h, RenderTextureHandle *texture, bool floatingPoint)
{
    FN_TIMER
    if (InitRenderTextureCubeProc) {
        return InitRenderTextureCubeProc(w, h, texture, floatingPoint);
    }

    return false;
}

bool Renderer::MakeRenderTextureCubeCurrent(RenderTextureHandle texture, bool clear, CubeFace face)
{
    FN_TIMER
    if (MakeRenderTextureCubeCurrentProc) {
        return MakeRenderTextureCubeCurrentProc(texture, clear, face);
    }

    return false;
}

bool Renderer::BindRenderTextureCube(RenderTextureHandle texture)
{
    FN_TIMER
    if (BindRenderTextureCubeProc) {
        return BindRenderTextureCubeProc(texture);
    }

    return false;
}

bool Renderer::GetRenderTextureCubeTextureHandle(RenderTextureHandle renTexture, TextureHandle *texture)
{
    FN_TIMER
    if (GetRenderTextureCubeTextureHandleProc) {
        return GetRenderTextureCubeTextureHandleProc(renTexture, texture);
    }

    return false;
}

bool Renderer::ReleaseRenderTextureCube(RenderTextureHandle texture)
{
    FN_TIMER
    if (ReleaseRenderTextureCubeProc) {
        return ReleaseRenderTextureCubeProc(texture);
    }

    return false;
}

bool Renderer::InitRenderTarget(int w, int h, RenderTargetHandle *tgt)
{
    FN_TIMER
    if (InitRenderTargetProc) {
        return InitRenderTargetProc(w, h, tgt);
    }

    return false;
}

bool Renderer::MakeRenderTargetCurrent(RenderTargetHandle tgt)
{
    FN_TIMER
    if (MakeRenderTargetCurrentProc) {
        return MakeRenderTargetCurrentProc(tgt);
    }

    return false;
}

bool Renderer::RestoreRenderTarget(RenderTargetHandle tgt)
{
    FN_TIMER
    if (RestoreRenderTargetProc) {
        return RestoreRenderTargetProc(tgt);
    }

    return false;
}

bool Renderer::ReleaseRenderTarget(RenderTargetHandle tgt)
{
    FN_TIMER
    if (ReleaseRenderTargetProc) {
        return ReleaseRenderTargetProc(tgt);
    }

    return false;
}

bool Renderer::GetPixels(int x, int y, int w, int h, void *pixels, bool immediate)
{
    FN_TIMER
    if (GetPixelsProc) {
        return GetPixelsProc(x, y, w, h, pixels, immediate);
    }

    return false;
}

bool Renderer::SetPixels(int x, int y, int w, int h, void *pixels)
{
    FN_TIMER
    if (SetPixelsProc) {
        return SetPixelsProc(x, y, w, h, pixels);
    }

    return false;
}

bool Renderer::DrawAALine(const Color& c, double width, const Vector3& p1, const Vector3& p2)
{
    FN_TIMER
    if (DrawAALineProc) {
        return DrawAALineProc(c, width, p1, p2);
    }

    return false;
}

bool Renderer::DrawAALines(const Color& c, double width, const SL_VECTOR(Vector3)& points)
{
    FN_TIMER
    if (DrawAALinesProc) {
        return DrawAALinesProc(c, width, points);
    }

    return false;
}

bool Renderer::GetViewport(int& x, int& y, int& w, int &h)
{
    FN_TIMER
    x = vpX;
    y = vpY;
    w = vpW;
    h = vpH;
    return true;
}

bool Renderer::SetViewport(int x, int y, int w, int h)
{
    vpX = x;
    vpY = y;
    vpW = w;
    vpH = h;
    if (SetViewportProc) {
        SetViewportProc(x, y, w, h);
    }

    return true;
}

bool Renderer::GetDepthRange(float& zmin, float& zmax)
{
    FN_TIMER
    zmin = minZ;
    zmax = maxZ;
    return true;
}

bool Renderer::SetDepthRange(float zmin, float zmax)
{
    if (zmin != minZ || zmax != maxZ) {
        minZ = zmin;
        maxZ = zmax;
        if (SetDepthRangeProc) {
            SetDepthRangeProc(zmin, zmax );
        }
    }
    return true;
}

bool Renderer::GetFOV(double& pfov)
{
    FN_TIMER
    pfov = fov;
    return true;
}

bool Renderer::CopyLuminanceFromScreen(int x, int y, int w, int h, unsigned char *buf)
{
    if (CopyLuminanceFromScreenProc) {
        return CopyLuminanceFromScreenProc(x, y, w, h, buf);
    }

    return false;
}

bool Renderer::CopyLuminanceIntoTexture(TextureHandle texture, int w, int h, unsigned char *buf)
{
    if (CopyLuminanceIntoTextureProc) {
        return CopyLuminanceIntoTextureProc(texture, w, h, buf);
    }

    return false;
}

bool Renderer::CreateLuminanceTexture(int w, int h, TextureHandle *texture)
{
    if (CreateLuminanceTextureProc) {
        return CreateLuminanceTextureProc(w, h, texture);
    }

    return false;
}

void Renderer::SetContext(void *context)
{
    if (SetContextProc) {
        SetContextProc(context);
    }
}

void Renderer::FrameStarted(void *context)
{
    FN_TIMER

    if (SetContextProc) {
        SetContextProc(context);
    }

    blendedObjects.clear();

    // Cache all frame-specific data that would stall the pipeline to retrieve
    if (!hasExplicitViewport && GetViewportProc) {
        GetViewportProc(vpX, vpY, vpW, vpH);
    }

    if (!hasExplicitDepthRange && GetDepthRangeProc) {
        GetDepthRangeProc(minZ, maxZ);
    }

    if (GetFOVProc) {
        GetFOVProc(fov);
    }

    if (GetFrustumWorldSpaceProc) {
        GetFrustumWorldSpaceProc(frustum);
    }

    if (GetTextureMatrixProc) {
        //GetTextureMatrixProc(&texture);
        Matrix4 identity;
        texture = identity;
    }

    if (!hasExplicitMatrices && GetProjectionMatrixProc) {
        GetProjectionMatrixProc(&projection);
    }

    if (!hasExplicitMatrices && GetModelviewMatrixProc) {
        GetModelviewMatrixProc(&modelview);
    }

    // Figure out the world-oriented billboarding matrix.
    Vector3 Up = GetUpVector();
    Vector3 Normal;

    bool useNDC = true;
    Configuration::GetBoolValue("billboard-use-ndc", useNDC);

    if (useNDC) {
        // Vector pointing away from the eyepoint into the screen
        Vector3 ndcIn(0, 0, 1);
        // de-project to world space
        Matrix4 view = modelview;
        view.elem[0][3] = view.elem[1][3] = view.elem[2][3] = 0;
        Matrix4 mvp = projection * view;
        Matrix4 invMvp = mvp.Inverse();
        Normal = invMvp * ndcIn;
        Normal.Normalize();
        Normal = Normal * -1.0;
    } else {
        Normal = Vector3(modelview.elem[2][0], modelview.elem[2][1], modelview.elem[2][2]);
    }

    // Things get wonky when the normal vector approaches the fixed world up vector.
    // Blend towards the orthogonal right vector instead when this happens.
    Vector3 WorldRight = GetRightVector();

    double screenBlend = fabs(Up.Dot(Normal));

    double screenBlendFactor = 0.3;
    Configuration::GetDoubleValue("billboard-world-screen-blend-factor", screenBlendFactor);

    screenBlend *= screenBlendFactor;

    Up = (WorldRight * screenBlend) + (Up * (1.0 - screenBlend));

    Vector3 Right = Up.Cross(Normal);
    Right.Normalize();
    Up = Normal.Cross(Right);
    Up.Normalize();

    if (rightHanded) {
        billboard = Matrix4(Right.x, Up.x, Normal.x, 0,
                            Right.y, Up.y, Normal.y, 0,
                            Right.z, Up.z, Normal.z, 0,
                            0,   0,    0,      1);
    } else {
        billboard = Matrix4(Right.x, Up.x, -Normal.x, 0,
                            Right.y, Up.y, -Normal.y, 0,
                            Right.z, Up.z, -Normal.z, 0,
                            0,   0,    0,      1);
    }

    // Rotate the camera position back...
    Vector3 U(modelview.elem[0][0], modelview.elem[0][1], modelview.elem[0][2]);
    Vector3 V(modelview.elem[1][0], modelview.elem[1][1], modelview.elem[1][2]);
    Vector3 N(modelview.elem[2][0], modelview.elem[2][1], modelview.elem[2][2]);
    Vector4 R(-modelview.elem[0][3], -modelview.elem[1][3], -modelview.elem[2][3], 1);

    Matrix4 M1(U.x, V.x, N.x, 0,
               U.y, V.y, N.y, 0,
               U.z, V.z, N.z, 0,
               0,   0,   0,   1);

    Vector4 camPos4 = M1 * R;
    camPos = Vector3(camPos4.x, camPos4.y, camPos4.z);


    blendedObjects.clear();
}

void Renderer::SubmitBlendedObject(Renderable *obj)
{
    if (obj) {
        blendedObjects.push_back(obj);
    }
}

void Renderer::ClearBlendedObjects()
{
    blendedObjects.clear();
}

void Renderer::RemoveBlendedObject(Renderable *obj)
{
    SL_VECTOR(Renderable *) ::iterator it;
    for (it = blendedObjects.begin(); it != blendedObjects.end(); it++) {
        if (*it == obj) {
            blendedObjects.erase(it);
            return;
        }
    }
}

static Matrix4 sortMV;
static Vector3 sortCamPos;
static bool sortRightHanded;

static bool comp(Renderable *c1, Renderable *c2)
{
    Vector3 p1, p2;
    if (c1) p1 = c1->GetSortPosition();
    if (c2) p2 = c2->GetSortPosition();

    double d1 = c1 ? c1->GetDistance(sortCamPos, p2, sortMV, sortRightHanded) + c1->GetDepthOffset() : 0;
    double d2 = c2 ? c2->GetDistance(sortCamPos, p1, sortMV, sortRightHanded) + c2->GetDepthOffset() : 0;

    return (d1 > d2);
}

void Renderer::SortAndDrawBlendedObjects(bool enableDepthTest, bool enableDepthWrites)
{
    SortBlendedObjects();
    DrawBlendedObjects(enableDepthTest, enableDepthWrites);
}

void Renderer::SortBlendedObjects()
{
    Renderer *ren = Renderer::GetInstance();
    ren->GetModelviewMatrix(&sortMV);
    sortCamPos = ren->GetCamPos();
    sortRightHanded = rightHanded;
    sort(blendedObjects.begin(), blendedObjects.end(), comp);
}

void Renderer::DrawBlendedObjects(bool enableDepthTest, bool enableDepthWrites)
{
    // Set the blend and z buffer modes
    EnableBlending(ONE, INVSRCALPHA);
    EnableDepthReads(enableDepthTest);
    if (enableDepthWrites) EnableDepthWrites(true);
    disableDepthReads = !enableDepthTest;
    disableDepthWrites = enableDepthWrites;
    EnableDepthWrites(false);
    EnableTexture2D(true);
    EnableLighting(false);

    SL_VECTOR(Renderable *) ::iterator it;
    for (it = blendedObjects.begin(); it != blendedObjects.end(); it++) {
        (*it)->DrawBlendedObject();
    }

    EnableDepthWrites(true);
    disableDepthReads = false;
    disableDepthWrites = false;
}

bool Renderer::GetIsDirectX() const
{
    return (type == Atmosphere::DIRECTX9 || type == Atmosphere::DIRECTX10 || type == Atmosphere::DIRECTX11
            || type == Atmosphere::DIRECTX11_1);
}

void *Renderer::GetNativeTexture(TextureHandle tex)
{
    if (GetNativeTextureProc) {
        return GetNativeTextureProc(tex);
    }
    return 0;
}
