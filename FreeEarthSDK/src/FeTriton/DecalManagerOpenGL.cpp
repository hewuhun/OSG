// Copyright (c) 2013-2015 Sundog Software LLC. All rights reserved worldwide.

#include <FeTriton/DecalOpenGL.h>
#include <FeTriton/DecalManagerOpenGL.h>
#include <FeTriton/Environment.h>
#include <FeTriton/Plane.h>

using namespace Triton;

DecalManagerOpenGL::DecalManagerOpenGL(const Environment *env, const TRITON_VECTOR(unsigned int)& pUserShaders) : userShaders(pUserShaders),
    program(0), vertShader(0), fragShader(0), userShader(0), userVertShader(0), inverseViewLoc(-1), decalTextureLoc(-1), decalMatrixLoc(-1),
    depthTextureLoc(-1), mvpLoc(-1), inverseViewportLoc(-1), projMatrixLoc(-1), viewportLoc(-1), inverseProjectionLoc(-1),
    depthRangeLoc(-1), depthOffsetLoc(-1), alphaLoc(-1), lightColorLoc(-1), positionLoc(-1), uvOffsetLoc(-1), upLoc(-1), DecalManager(env)
{

}

DecalManagerOpenGL::~DecalManagerOpenGL()
{
    DeleteShaders();
}

Decal *DecalManagerOpenGL::CreateDecal(TextureHandle texture, float size)
{
    return TRITON_NEW DecalOpenGL(environment, texture, size);
}

bool DecalManagerOpenGL::LoadShaders()
{
    Utils::ClearGLErrors();

    OpenGLCachedUniforms::ClearCaches();

	if(!environment)
		return false;
	osg::GLExtensions* et = environment->GetExtension();
	if(!et)
		return false;

    vertShader = LoadShader(Utils::GetDecalShaderFileName(environment, false).c_str(), true, false);
    fragShader = LoadShader(Utils::GetDecalShaderFileName(environment, true).c_str(), false, false);
    userShader = LoadShader(Utils::GetUserShaderFileName().c_str(), false, true );
    userVertShader = LoadShader(Utils::GetUserVertShaderFileName().c_str(), true, true);

#ifdef __APPLE__
    vertShaderCast = reinterpret_cast<uintptr_t>(vertShader);
    fragShaderCast = reinterpret_cast<uintptr_t>(fragShader);
    userShaderCast = reinterpret_cast<uintptr_t>(userShader);
    userVertShaderCast = reinterpret_cast<uintptr_t>(userVertShader);
#else
    vertShaderCast = vertShader;
    fragShaderCast = fragShader;
    userShaderCast = userShader;
    userVertShaderCast = userVertShader;
#endif

    if (vertShaderCast && fragShaderCast) {
        programCast = et->glCreateProgram();

        et->glAttachShader(programCast, vertShaderCast);
        et->glAttachShader(programCast, fragShaderCast);

        if (userShaderCast) et->glAttachShader(programCast, userShaderCast);
        if (userVertShaderCast) et->glAttachShader(programCast, userVertShaderCast);

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

    //glUseProgram(programCast);

    positionLoc = et->glGetAttribLocation(programCast, "position");

    mvpLoc = et->glGetUniformLocation(programCast, "trit_mvProj");
    depthTextureLoc = et->glGetUniformLocation(programCast, "trit_depthTexture");
    decalTextureLoc = et->glGetUniformLocation(programCast, "trit_decalTexture");
    inverseViewLoc = et->glGetUniformLocation(programCast, "trit_inverseView");
    decalMatrixLoc = et->glGetUniformLocation(programCast, "trit_decalMatrix");
    inverseViewportLoc = et->glGetUniformLocation(programCast, "trit_inverseViewport");
    projMatrixLoc = et->glGetUniformLocation(programCast, "trit_projMatrix");
    inverseProjectionLoc = et->glGetUniformLocation(programCast, "trit_inverseProjection");
    viewportLoc = et->glGetUniformLocation(programCast, "trit_viewport");
    depthRangeLoc = et->glGetUniformLocation(programCast, "trit_depthRange");
    depthOffsetLoc = et->glGetUniformLocation(programCast, "trit_depthOffset");
    alphaLoc = et->glGetUniformLocation(programCast, "trit_alpha");
    lightColorLoc = et->glGetUniformLocation(programCast, "trit_lightColor");
    uvOffsetLoc = et->glGetUniformLocation(programCast, "trit_colorUVOffset");
    upLoc = et->glGetUniformLocation(programCast, "trit_up");

    //glUseProgram(0);
    Utils::PrintGLErrors(__FILE__, __LINE__);
    return true;
}

void DecalManagerOpenGL::DeleteShaders()
{
	if(!environment)
		return;
	osg::GLExtensions* et = environment->GetExtension();
	if(!et)
		return;

    if (vertShaderCast && programCast) {
        et->glDetachShader(programCast, vertShaderCast);
        et->glDeleteShader(vertShaderCast);
        vertShaderCast = 0;
    }

    if (fragShaderCast && programCast) {
        et->glDetachShader(programCast, fragShaderCast);
        et->glDeleteShader(fragShaderCast);
        fragShaderCast = 0;
    }

    if (userShaderCast && programCast) {
        et->glDetachShader(programCast, userShaderCast);
        et->glDeleteShader(userShaderCast);
        userShaderCast = 0;
    }

    if (userVertShaderCast && programCast) {
        et->glDetachShader(programCast, userVertShaderCast);
        et->glDeleteShader(userVertShaderCast);
        userVertShaderCast = 0;
    }

    if (programCast) {
        et->glDeleteProgram(programCast);
        programCast = 0;
    }

    //Utils::PrintGLErrors(__FILE__, __LINE__);
}

bool DecalManagerOpenGL::ReloadShaders(const TRITON_VECTOR(unsigned int)& shaders)
{
    DeleteShaders();

    userShaders = shaders;

    return LoadShaders();
}

void DecalManagerOpenGL::SetupShaders(TextureHandle depthTexture)
{
	if(!environment)
		return;
	osg::GLExtensions* et = environment->GetExtension();
	if(!et)
		return;

    if (programCast) {
        et->glUseProgram(programCast);
    }

    Matrix4 view(environment->GetCameraMatrix());
    Matrix4 proj(environment->GetProjectionMatrix());

    if (inverseViewLoc != -1) {
        // Make relative to camera to preserve precision
        view.elem[3][0] = view.elem[3][1] = view.elem[3][2] = 0;
        Matrix4 invView = view.InverseCramers();
        OpenGLCachedUniforms::SetUniformMat4(environment, programCast, inverseViewLoc, invView.ToFloatArray());
    }

    if (depthTextureLoc != -1) {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, (GLuint)depthTexture);
        OpenGLCachedUniforms::SetUniformInt(environment, programCast, depthTextureLoc, 0);
    }

    GLint vp[4];

    if (!environment->GetViewport(vp[0], vp[1], vp[2], vp[3])) {
        glGetIntegerv(GL_VIEWPORT, vp);
    }


    if (inverseViewportLoc != -1) {
        OpenGLCachedUniforms::SetUniformVec2(environment, programCast, inverseViewportLoc, 1.0f / (float)vp[2], 1.0f / (float)vp[3]);
    }

    if (viewportLoc != -1) {
        OpenGLCachedUniforms::SetUniformVec4(environment, programCast, viewportLoc, (float)vp[0], (float)vp[1], (float)vp[2], (float)vp[3]);
    }

    if (projMatrixLoc != -1) {
        OpenGLCachedUniforms::SetUniformMat4(environment, programCast, projMatrixLoc, proj.ToFloatArray());
    }

    if (inverseProjectionLoc != -1) {
        Matrix4 invProj = proj.InverseCramers();
        OpenGLCachedUniforms::SetUniformMat4(environment, programCast, inverseProjectionLoc, invProj.ToFloatArray());
    }

    if (upLoc != -1) {
        OpenGLCachedUniforms::SetUniformVec3(environment, programCast, upLoc, (float)(environment->GetUpVector().x),
                                             (float)(environment->GetUpVector().y), (float)(environment->GetUpVector().z));
    }

    if (depthRangeLoc != -1) {
        GLfloat dr[2];
        if (getDepthRange) {
            glGetFloatv(GL_DEPTH_RANGE, dr);
        } else {
            dr[0] = 0;
            dr[1] = 1.0;
        }
        OpenGLCachedUniforms::SetUniformVec2(environment, programCast, depthRangeLoc, dr[0], dr[1]);
    }

    if (depthOffsetLoc != -1) {
        OpenGLCachedUniforms::SetUniformFloat(environment, programCast, depthOffsetLoc, depthOffset);
    }

    //Utils::PrintGLErrors(__FILE__, __LINE__);
}

void DecalManagerOpenGL::SetUniforms(Decal *decal)
{
    Matrix4 view(environment->GetCameraMatrix());
    Matrix4 proj(environment->GetProjectionMatrix());

    if (decal) {

        Matrix4 rotate; // identity by default

        if (decal->GetRotation() != 0) {
            Vector3 up = environment->GetUpVector();
            double theta = decal->GetRotation();
            double costheta = cos(theta);
            double sintheta = sin(theta);

            rotate.elem[0][0] = costheta + up.x * up.x * (1.0 - costheta);
            rotate.elem[0][1] = up.x * up.y * (1.0 - costheta) - up.z * sintheta;
            rotate.elem[0][2] = up.x * up.z * (1.0 - costheta) + up.y * sintheta;

            rotate.elem[1][0] = up.y * up.x * (1.0 - costheta) + up.z * sintheta;
            rotate.elem[1][1] = costheta + up.y * up.y * (1.0 - costheta);
            rotate.elem[1][2] = up.y * up.z * (1.0 - costheta) - up.x * sintheta;

            rotate.elem[2][0] = up.z * up.x * (1.0 - costheta) - up.y * sintheta;
            rotate.elem[2][1] = up.z * up.y * (1.0 - costheta) + up.x * sintheta;
            rotate.elem[2][2] = costheta + up.z * up.z * (1.0 - costheta);
        }

        if (mvpLoc != -1) {
            Vector3 pos = decal->GetPosition();// - environment->GetCameraPosition();
            Matrix4 model;
            model.elem[3][0] = pos.x;
            model.elem[3][1] = pos.y;
            model.elem[3][2] = pos.z;

            Matrix4 scale;
            float sx, sz;
            decal->GetScale(sx, sz);
            scale.elem[0][0] = sx;
            scale.elem[2][2] = sz;

            scale = basis * scale * invBasis;

            //view.elem[3][0] = view.elem[3][1] = view.elem[3][2] = 0;

            Matrix4 mvproj = scale * rotate * model * view * proj;

            OpenGLCachedUniforms::SetUniformMat4(environment, programCast, mvpLoc, mvproj.ToFloatArray());
        }

        if (decalMatrixLoc != -1) {
            Matrix4 decalMatrix = decal->GetDecalMatrix();
            OpenGLCachedUniforms::SetUniformMat4(environment, programCast, decalMatrixLoc, decalMatrix.ToFloatArray());
        }

        if (uvOffsetLoc != -1) {
            float u, v;
            decal->GetUVOffset(u, v);
            OpenGLCachedUniforms::SetUniformVec2(environment, programCast, uvOffsetLoc, u, v);
        }

        if (decalTextureLoc != -1) {
            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D, (GLuint)decal->GetDecalTexture());
            OpenGLCachedUniforms::SetUniformInt(environment, programCast, decalTextureLoc, 1);
        }

        if (alphaLoc != -1) {
            OpenGLCachedUniforms::SetUniformFloat(environment, programCast, alphaLoc, decal->GetAlpha());
        }

        if (lightColorLoc != -1) {
            const Vector3& color = decal->GetLightColor();
            OpenGLCachedUniforms::SetUniformVec4(environment, programCast, lightColorLoc, (float)color.x, (float)color.y, (float)color.z, 1.0f);
        }

        glActiveTexture(GL_TEXTURE0);
    }

    Utils::PrintGLErrors(__FILE__, __LINE__);
}

void DecalManagerOpenGL::DrawDecals(TextureHandle depthTexture)
{
    if (decals.size() > 0) {

        ComputeBasis();

        SetupShaders(depthTexture);

        TRITON_LIST(Decal *)::iterator it;
        for (it = decals.begin(); it != decals.end(); it++) {
            Decal *decal = *it;

            OrientedBoundingBox obb = decal->GetOrientedBoundingBox();
            float scaleX, scaleZ;
            decal->GetScale(scaleX, scaleZ);
            obb.halfDistances[0] *= scaleX;
            obb.halfDistances[2] *= scaleZ;

            double rotation = decal->GetRotation();
            Matrix3 rotate;

            if (rotation != 0) {
                Vector3 up = environment->GetUpVector();
                double theta = rotation;
                double costheta = cos(theta);
                double sintheta = sin(theta);

                rotate.elem[0][0] = costheta + up.x * up.x * (1.0 - costheta);
                rotate.elem[0][1] = up.x * up.y * (1.0 - costheta) - up.z * sintheta;
                rotate.elem[0][2] = up.x * up.z * (1.0 - costheta) + up.y * sintheta;

                rotate.elem[1][0] = up.y * up.x * (1.0 - costheta) + up.z * sintheta;
                rotate.elem[1][1] = costheta + up.y * up.y * (1.0 - costheta);
                rotate.elem[1][2] = up.y * up.z * (1.0 - costheta) - up.x * sintheta;

                rotate.elem[2][0] = up.z * up.x * (1.0 - costheta) - up.y * sintheta;
                rotate.elem[2][1] = up.z * up.y * (1.0 - costheta) + up.x * sintheta;
                rotate.elem[2][2] = costheta + up.z * up.z * (1.0 - costheta);

                rotate = rotate.Transpose();

                obb.axes[0] = rotate * obb.axes[0];
                obb.axes[2] = rotate * obb.axes[2];
                obb.axes[1] = rotate * obb.axes[1];

                obb.RecomputeBasis();
            } else {
                rotate = Matrix3::Identity;
            }

            if (!environment->CullOrientedBoundingBox(obb)) {

               decal->SetCullState(false);

                decal->SetLightColor(environment->GetAmbientLightColor());

                decal->ComputeDecalMatrix(environment->GetCameraPosition(), rotate);

                Matrix4 projection = environment->GetProjectionMatrix();
                projection.Transpose();
                bool inside = obb.PointInBox(environment->GetCameraPosition(), GetNearClipDistance(projection));

                SetUniforms(decal);

                /*decal->BindArrays();

                if (environment && environment->GetRenderer() >= OPENGL_3_2) {
                    glVertexAttribPointer(positionLoc, 4, GL_FLOAT, GL_FALSE, 0, (GLvoid *)0);
                    glEnableVertexAttribArray(positionLoc);
                }*/

                decal->Draw(inside);

                decal->UnbindArrays();

            } else {
                decal->SetCullState(true);
            }
        }

        //glUseProgram(0);
    }

    Utils::PrintGLErrors(__FILE__, __LINE__);
}

static void PrintGLSLInfoLog(GLhandleARB obj,const Environment* en)
{
    int infologLength = 0;
    int charsWritten  = 0;
    char *infoLog;
    GLuint objCast;

	if(!en)
		return;
	osg::GLExtensions* et = en->GetExtension();
	if(!et)
		return;

#ifdef __APPLE__
    objCast = reinterpret_cast<uintptr_t>(obj);
#else
    objCast = obj;
#endif

    Utils::DebugMsg("GLSL error detected in decal manager!\n");

    if (OpenGLExtensionManager::HasExtension("GL_ARB_shader_objects",en)) {
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

GLhandleARB DecalManagerOpenGL::LoadShader(const char *shaderName, bool vertexShader,
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

        const char * userString = (vertexShader) ?
                                  environment->GetUserDefinedVertString() : environment->GetUserDefinedFragString();

        const char *userDefines = ( userString && userShader ) ? userString : "";

        if (environment->GetRenderer() >= OPENGL_3_2) {
            const char defines[] = "#version 150\n#define OPENGL32\n";
            const char *sources[3] = { defines, userDefines, data };
            et->glShaderSource(shader, 3, sources, NULL);
        } else {
            const char defines[] = "#version 120\n";
            const char *ssc[3] = { defines, userDefines, data };
            et->glShaderSource(shader, 3, ssc, NULL);
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
