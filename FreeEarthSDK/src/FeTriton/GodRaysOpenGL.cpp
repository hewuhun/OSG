// Copyright (c) 2014-2015 Sundog Software, LLC. All rights reserved worldwide.

#include <FeTriton/GodRaysOpenGL.h>
#include <FeTriton/Environment.h>
#include <FeUtils/CoordConverter.h>
#include <FeTriton/Ocean.h>
#include <cmath>
using namespace Triton;

GodRaysOpenGL::GodRaysOpenGL(const Environment *env) : GodRays(env), vboID(0), idxID(0), vertexArray(0), nIndices(0),
    program(0), vertShader(0), fragShader(0), userShader(0), userVertShader(0)
{

}

GodRaysOpenGL::~GodRaysOpenGL()
{
    DeleteMesh();

    DeleteShaders();
}

void GodRaysOpenGL::DeleteShaders()
{
	if(environment)
	{
		osg::GLExtensions* et = environment->GetExtension();
		if(et)
		{
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
		}
	}
}

static void PrintGLSLInfoLog(GLhandleARB obj,const Environment *env)
{
    int infologLength = 0;
    int charsWritten = 0;
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

    Utils::DebugMsg("GLSL error detected in god rays!\n");

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

GLhandleARB GodRaysOpenGL::LoadShader(const char *shaderName, bool vertexShader, bool userShader)
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

        const char *sources[3];

        int numSources = 0;

        if (environment->GetRenderer() >= OPENGL_3_2) {
            sources[numSources++] = "#version 150\n#define OPENGL32\n";
        } else {
            sources[numSources++] = "#version 120\n";
        }

        if (userShader) {
            const char *userString = (vertexShader) ?
                                     environment->GetUserDefinedVertString() : environment->GetUserDefinedFragString();
            if (userString)
                sources[numSources++] = userString;
        }

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

bool GodRaysOpenGL::LoadShaders()
{
	if(!environment)
		return false;
	osg::GLExtensions* et = environment->GetExtension();
	if(!et)
		return false;

    Utils::ClearGLErrors();

    OpenGLCachedUniforms::ClearCaches();

    userShader = userVertShader = 0;
    userShaderCast = userVertShaderCast = 0;

    vertShader = LoadShader(Utils::GetGodRayShaderFileName(environment, false).c_str(), true, false);
    fragShader = LoadShader(Utils::GetGodRayShaderFileName(environment, true).c_str(), false, false);

    userShader = LoadShader(Utils::GetUserShaderFileName().c_str(), false, true);
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

        et->glLinkProgram(programCast);

        GLint linkStatus;
        et->glGetProgramiv(programCast, GL_LINK_STATUS, &linkStatus);
        if (linkStatus == GL_FALSE) {
            return false;
        }
    }

    positionLoc = et->glGetAttribLocation(programCast, "position");

    mvpLoc = et->glGetUniformLocation(programCast, "mvProj");
    originLoc = et->glGetUniformLocation(programCast, "origin");
    spacingLoc = et->glGetUniformLocation(programCast, "spacing");
    extinctionLoc = et->glGetUniformLocation(programCast, "extinction");
    viewerLoc = et->glGetUniformLocation(programCast, "viewer");
    sunDirLoc = et->glGetUniformLocation(programCast, "sunDir");
    sunColorLoc = et->glGetUniformLocation(programCast, "sunColor");
    basisLoc = et->glGetUniformLocation(programCast, "basis");
    invBasisLoc = et->glGetUniformLocation(programCast, "invBasis");
    displacementMapLoc = et->glGetUniformLocation(programCast, "displacementMap");
    slopeFoamMapLoc = et->glGetUniformLocation(programCast, "slopeFoamMap");
    textureSizeLoc = et->glGetUniformLocation(programCast, "textureSize");
    viewDirLoc = et->glGetUniformLocation(programCast, "viewDir");
    visibilityLoc = et->glGetUniformLocation(programCast, "visibility");
    fadeLoc = et->glGetUniformLocation(programCast, "fade");

    return Utils::PrintGLErrors(__FILE__, __LINE__);
}

bool GodRaysOpenGL::ReloadShaders(const TRITON_VECTOR(unsigned int)& shaders)
{
    DeleteShaders();

    // No user shaders currently linked into GodRays shader
    //userShaders = shaders;

    return LoadShaders();
}

bool GodRaysOpenGL::SetupMesh()
{
    if (!LoadShaders()) return false;

	if(!environment)
		return false;
	osg::GLExtensions* et = environment->GetExtension();
	if(!et)
		return false;

    // A top and bottom NxN grid is used to create
    // (N-1) x (N-1) parallelepipeds representing each light shaft

    int idx = 0;
    int nVerts = N * N * 2;
    RayVertex *v = TRITON_NEW RayVertex[nVerts];

    // Top mesh
    float halfDim = (float)(N-1) * 0.5f;

    for (float x = -halfDim; x <= halfDim; x += 1.0f) {
        for (float z = -halfDim; z <= halfDim; z += 1.0f) {
            v[idx].x = x;
            v[idx].y = 0;
            v[idx].z = z;
            v[idx].extrusionLength = 0;
            idx++;
        }
    }

    // Bottom mesh
    for (float x = -halfDim; x <= halfDim; x += 1.0f) {
        for (float z = -halfDim; z <= halfDim; z += 1.0f) {
            v[idx].x = x;
            v[idx].y = 0;
            v[idx].z = z;
            v[idx].extrusionLength = effectDepth;
            idx++;
        }
    }

    int bottomStart = N * N;

    int trianglesAcross = (N - 1) * 2;
    nIndices = trianglesAcross * 3 * N * 2;

    nIndices += ((N - 1) * (N - 1) * 6);

    GLuint *i = TRITON_NEW GLuint[nIndices];
    idx = 0;
    int vx, vz;

    // "Front facing" triangles

    for (vz = 0; vz < N; vz++) {
        int rowStart = N * vz;
        for (vx = 0; vx < (N-1); vx++) {
            i[idx++] = rowStart + vx;
            i[idx++] = rowStart + vx + bottomStart;
            i[idx++] = rowStart + vx + 1;

            i[idx++] = rowStart + vx + bottomStart;
            i[idx++] = rowStart + vx + bottomStart + 1;
            i[idx++] = rowStart + vx + 1;
        }
    }

    // "Side facing" triangles
    for (vx = 0; vx < N; vx++) {
        for (vz = 0; vz < (N - 1); vz++) {
            int base = N * vz + vx;
            i[idx++] = base;
            i[idx++] = base + N;
            i[idx++] = base + N + bottomStart;

            i[idx++] = base;
            i[idx++] = base + N + bottomStart;
            i[idx++] = base + bottomStart;
        }
    }

    // Top triangles
    for (vx = 0; vx < (N - 1); vx++) {
        for (vz = 0; vz < (N - 1); vz++) {
            int base = N * vz + vx;
            i[idx++] = base;
            i[idx++] = base + 1;
            i[idx++] = base + N;

            i[idx++] = base + 1;
            i[idx++] = base + N + 1;
            i[idx++] = base + N;
        }
    }

    if (environment && environment->GetRenderer() >= OPENGL_3_2) {
        et->glGenVertexArrays(1, &vertexArray);
    }

    // Upload both to the GPU
    et->glGenBuffers(1, &vboID);
    et->glBindBuffer(GL_ARRAY_BUFFER, vboID);
    et->glBufferData(GL_ARRAY_BUFFER, nVerts * sizeof(RayVertex),
                 (void *)v, GL_STATIC_DRAW_ARB);

    et->glGenBuffers(1, &idxID);
    et->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, idxID);
    et->glBufferData(GL_ELEMENT_ARRAY_BUFFER, nIndices * sizeof(GLuint),
                 (void *)i, GL_STATIC_DRAW);

    et->glBindBuffer(GL_ARRAY_BUFFER, 0);
    et->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    TRITON_DELETE[] v;
    TRITON_DELETE[] i;

    return Utils::PrintGLErrors(__FILE__, __LINE__);
}

bool GodRaysOpenGL::DrawMesh()
{
	if(!environment)
		return false;
	osg::GLExtensions* et = environment->GetExtension();
	if(!et)
		return false;

    if (programCast) {
        et->glUseProgram(programCast);
    }

    Matrix4 view(environment->GetCameraMatrix());
    Matrix4 proj(environment->GetProjectionMatrix());
    Matrix4 mvp = view * proj;

    OpenGLCachedUniforms::SetUniformMat4(environment, programCast, mvpLoc, mvp.ToFloatArray());
    OpenGLCachedUniforms::SetUniformVec3(environment, programCast, originLoc, (float)origin.x, (float)origin.y, (float)origin.z);
    OpenGLCachedUniforms::SetUniformFloat(environment, programCast, spacingLoc, spacing);
    OpenGLCachedUniforms::SetUniformVec3(environment, programCast, extinctionLoc, (float)extinction.x, (float)extinction.y, (float)extinction.z);
    OpenGLCachedUniforms::SetUniformVec3(environment, programCast, viewerLoc, (float)viewer.x, (float)viewer.y, (float)viewer.z);
    OpenGLCachedUniforms::SetUniformVec3(environment, programCast, sunDirLoc, (float)sunDir.x, (float)sunDir.y, (float)sunDir.z);
    OpenGLCachedUniforms::SetUniformVec3(environment, programCast, sunColorLoc, (float)sunColor.x, (float)sunColor.y, (float)sunColor.z);

    OpenGLCachedUniforms::SetUniformVec2(environment, programCast, textureSizeLoc, Lx, Ly);
    OpenGLCachedUniforms::SetUniformMat3(environment, programCast, basisLoc, basis.ToFloatArray());
    OpenGLCachedUniforms::SetUniformMat3(environment, programCast, invBasisLoc, invBasis.ToFloatArray());
    OpenGLCachedUniforms::SetUniformVec3(environment, programCast, viewDirLoc, (float)viewDir.x, (float)viewDir.y, (float)viewDir.z);

    OpenGLCachedUniforms::SetUniformFloat(environment, programCast, fadeLoc, fade);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, (GLuint)displacementTex);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, (GLuint)slopeFoamTex);

    OpenGLCachedUniforms::SetUniformInt(environment, programCast, displacementMapLoc, 0);
    OpenGLCachedUniforms::SetUniformInt(environment, programCast, slopeFoamMapLoc, 1);

    double visibility;
    Vector3 fogColor;
    environment->GetBelowWaterVisibility(visibility, fogColor);
    OpenGLCachedUniforms::SetUniformFloat(environment, programCast, visibilityLoc, (float)visibility);

    // Bind arrays
    if (environment && environment->GetRenderer() >= OPENGL_3_2) {
        et->glBindVertexArray(vertexArray);
        et->glBindBuffer(GL_ARRAY_BUFFER, vboID);
        et->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, idxID);
    } else {
        et->glBindBuffer(GL_ARRAY_BUFFER, vboID);
        et->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, idxID);
        glEnableClientState(GL_VERTEX_ARRAY);
        glVertexPointer(4, GL_FLOAT, 0, 0);
    }

    if (environment && environment->GetRenderer() >= OPENGL_3_2) {
        et->glVertexAttribPointer(positionLoc, 4, GL_FLOAT, GL_FALSE, 0, (GLvoid *)0);
        et->glEnableVertexAttribArray(positionLoc);
    }

    glDepthMask(GL_FALSE);

    glDisable(GL_CULL_FACE);
    glDisable(GL_DEPTH_TEST);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);

    glDrawElements(GL_TRIANGLES, nIndices, GL_UNSIGNED_INT, 0);

    glDepthMask(GL_TRUE);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, 0);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, 0);

    // Unbind arrays
    if (environment && environment->GetRenderer() >= OPENGL_3_2) {
        et->glBindVertexArray(0);
    } else {
        glDisableClientState(GL_VERTEX_ARRAY);
    }

    et->glBindBuffer(GL_ARRAY_BUFFER, 0);
    et->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    return Utils::PrintGLErrors(__FILE__, __LINE__);
}

bool GodRaysOpenGL::DeleteMesh()
{
	if(environment)
	{
		osg::GLExtensions* et = environment->GetExtension();
		if(et)
		{
			if (vertexArray && environment->GetRenderer() >= OPENGL_3_2)
			{
				et->glDeleteVertexArrays(1, &vertexArray);
			}

			if (vboID)
			{
				et->glDeleteBuffers(1, &vboID);
			}

			if (idxID)
			{
				et->glDeleteBuffers(1, &idxID);
			}
		}
	}
    return Utils::PrintGLErrors(__FILE__, __LINE__);
}


HydraxGodRaysOpenGL::HydraxGodRaysOpenGL(const Environment *env) : GodRays(env), vboID(0), idxID(0), vertexArray(0), nIndices(0),
	program(0), vertShader(0), fragShader(0)
	,mNumberOfRays(100)
	,mNearClip(1.0)
	,mFarClip(100.0)
	,mFov(45.0)
	,mAspect(1.0)
	,mNoiseDerivation(3)
	,mNoisePositionMultiplier(50)
	,mNoiseYNormalMultiplier(10)
	,mNoiseNormalMultiplier(0.175)
	,mRaysSize(0.03f)
	,mVertexArr(NULL)
	,nVerts(0)
{

}

Triton::HydraxGodRaysOpenGL::~HydraxGodRaysOpenGL()
{
	DeleteMesh();

	DeleteShaders();

	if(mVertexArr != NULL)
	{
		delete mVertexArr;
		mVertexArr = NULL;
	}
}

void Triton::HydraxGodRaysOpenGL::DeleteShaders()
{
	if(environment)
	{
		osg::GLExtensions* et = environment->GetExtension();
		if(et)
		{
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

			if (programCast) {
				et->glDeleteProgram(programCast);
				programCast = 0;
			}
		}
	}
}

bool Triton::HydraxGodRaysOpenGL::ReloadShaders( const TRITON_VECTOR(unsigned int)& shaders )
{
	DeleteShaders();

	// No user shaders currently linked into GodRays shader
	//userShaders = shaders;

	return LoadShaders();
}

bool Triton::HydraxGodRaysOpenGL::SetupMesh()
{
	if (!LoadShaders()) 
		return false;

	if(!createGodRays())
		return false;

	return true;
}

bool Triton::HydraxGodRaysOpenGL::DrawMesh()
{
	if(mVertexArr == NULL)
		return false;

	if(!environment)
		return false;
	osg::GLExtensions* et = environment->GetExtension();
	if(!et)
		return false;



	if (programCast) {
		et->glUseProgram(programCast);
	}
	
	Matrix4 view(environment->GetCameraMatrix());
	Matrix4 proj(environment->GetProjectionMatrix());
	Matrix4 mvp = mGodRayPosMat * view * proj;

	OpenGLCachedUniforms::SetUniformMat4(environment, programCast, mvpLoc, mvp.ToFloatArray());

	// Bind arrays
	if (environment && environment->GetRenderer() >= OPENGL_3_2) {
		et->glBindVertexArray(vertexArray);
		et->glBindBuffer(GL_ARRAY_BUFFER, vboID);
		et->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, idxID);
	} else {
		et->glBindBuffer(GL_ARRAY_BUFFER, vboID);
		et->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, idxID);
		glEnableClientState(GL_VERTEX_ARRAY);
		glVertexPointer(4, GL_FLOAT, 0, 0);
	}

	if (environment && environment->GetRenderer() >= OPENGL_3_2) {
		et->glVertexAttribPointer(positionLoc, 4, GL_FLOAT, GL_FALSE, 0, (GLvoid *)0);
		et->glEnableVertexAttribArray(positionLoc);
	}

	et->glBindBuffer(GL_ARRAY_BUFFER, vboID);

	et->glBufferSubData(GL_ARRAY_BUFFER,0, nVerts * sizeof(RayVertex),
		(void *)mVertexArr);

	et->glBindBuffer(GL_ARRAY_BUFFER, 0);

	glDepthMask(GL_FALSE);

	glDisable(GL_CULL_FACE);
	glDisable(GL_DEPTH_TEST);

	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE, GL_ONE);

	glDrawElements(GL_TRIANGLES, nIndices, GL_UNSIGNED_INT, 0);

	glDepthMask(GL_TRUE);

	// Unbind arrays
	if (environment && environment->GetRenderer() >= OPENGL_3_2) {
		et->glBindVertexArray(0);
	} else {
		glDisableClientState(GL_VERTEX_ARRAY);
	}

	et->glBindBuffer(GL_ARRAY_BUFFER, 0);
	et->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	return Utils::PrintGLErrors(__FILE__, __LINE__);
}

bool Triton::HydraxGodRaysOpenGL::DeleteMesh()
{
	if(environment)
	{
		osg::GLExtensions* et = environment->GetExtension();
		if(et)
		{
			if (vertexArray && environment->GetRenderer() >= OPENGL_3_2) {
				et->glDeleteVertexArrays(1, &vertexArray);
			}

			if (vboID) {
				et->glDeleteBuffers(1, &vboID);
			}

			if (idxID) {
				et->glDeleteBuffers(1, &idxID);
			}

			return Utils::PrintGLErrors(__FILE__, __LINE__);
		}
	}

	return false;
}

GLhandleARB Triton::HydraxGodRaysOpenGL::LoadShader( const char *shaderName, bool vertexShader, bool userShader )
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
		return false;

	Utils::ClearGLErrors();

	if (environment->GetResourceLoader()->LoadResource(
		shaderName, data, dataLen, true)) {
			shader = et->glCreateShader(vertexShader ? GL_VERTEX_SHADER : GL_FRAGMENT_SHADER);

			const char *sources[3];

			int numSources = 0;

			if (environment->GetRenderer() >= OPENGL_3_2) {
				sources[numSources++] = "#version 150\n#define OPENGL32\n";
			} else {
				sources[numSources++] = "#version 120\n";
			}

			if (userShader) {
				const char *userString = (vertexShader) ?
					environment->GetUserDefinedVertString() : environment->GetUserDefinedFragString();
				if (userString)
					sources[numSources++] = userString;
			}

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

bool Triton::HydraxGodRaysOpenGL::LoadShaders()
{
	if(!environment)
		return false;
	osg::GLExtensions* et = environment->GetExtension();
	if(!et)
		return false;

	Utils::ClearGLErrors();

	OpenGLCachedUniforms::ClearCaches();

	vertShader = LoadShader(Utils::GetHydraxGodRayShaderFileName(environment, false).c_str(), true, false);
	fragShader = LoadShader(Utils::GetHydraxGodRayShaderFileName(environment, true).c_str(), false, false);

#ifdef __APPLE__
	vertShaderCast = reinterpret_cast<uintptr_t>(vertShader);
	fragShaderCast = reinterpret_cast<uintptr_t>(fragShader);
#else
	vertShaderCast = vertShader;
	fragShaderCast = fragShader;
#endif

	if (vertShaderCast && fragShaderCast) {
		programCast = et->glCreateProgram();

		et->glAttachShader(programCast, vertShaderCast);
		et->glAttachShader(programCast, fragShaderCast);

		et->glLinkProgram(programCast);

		GLint linkStatus;
		et->glGetProgramiv(programCast, GL_LINK_STATUS, &linkStatus);
		if (linkStatus == GL_FALSE) {
			return false;
		}
	}

	positionLoc = et->glGetAttribLocation(programCast, "position");
	mvpLoc = et->glGetUniformLocation(programCast, "mvProj");

	return Utils::PrintGLErrors(__FILE__, __LINE__);
}

bool Triton::HydraxGodRaysOpenGL::createGodRays()
{
	if(!environment)
		return false;
	osg::GLExtensions* et = environment->GetExtension();
	if(!et)
		return false;

	int idx = 0;
	nVerts = mNumberOfRays * 12;
	nIndices = nVerts;
	mVertexArr = TRITON_NEW RayVertex[nVerts];
	GLuint *i = TRITON_NEW GLuint[nVerts];

	for(int k = 0; k < mNumberOfRays; k++)
	{
		for (int k = 0; k < 12; k ++)
		{
			mVertexArr[idx].x = 0;
			mVertexArr[idx].y = 0;
			mVertexArr[idx].z = 0;
			mVertexArr[idx].extrusionLength = 1;

			i[idx] = idx;

			idx++;
		}
	}

	if (environment && environment->GetRenderer() >= OPENGL_3_2) {
		et->glGenVertexArrays(1, &vertexArray);
	}

	// Upload both to the GPU
	et->glGenBuffers(1, &vboID);
	et->glBindBuffer(GL_ARRAY_BUFFER, vboID);
	et->glBufferData(GL_ARRAY_BUFFER, nVerts * sizeof(RayVertex),
		(void *)mVertexArr, GL_STREAM_DRAW);

	et->glGenBuffers(1, &idxID);
	et->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, idxID);
	et->glBufferData(GL_ELEMENT_ARRAY_BUFFER, nIndices * sizeof(GLuint),
		(void *)i, GL_STATIC_DRAW);

	et->glBindBuffer(GL_ARRAY_BUFFER, 0);
	et->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	//TRITON_DELETE[] v;
	TRITON_DELETE[] i;

	return Utils::PrintGLErrors(__FILE__, __LINE__);
}

void Triton::HydraxGodRaysOpenGL::updateGodRays()
{	
	if(environment && mVertexArr != NULL)
	{
		float FarWidth = getFarWidth();
		double RaysLength = mFarClip * 1.0;

		osg::Vec2 Pos;
		double Dis, RayLength;
		int idx = 0;

		for(int k = 0; k < mNumberOfRays; k++)
		{
			Pos       = calculateRayPosition(k);
			Dis       = mRaysSize * RaysLength;
			RayLength = RaysLength * (0.95 + Pos.length());

			Pos *= FarWidth / 2;

			mVertexArr[idx].x = 0;
			mVertexArr[idx].y = 0;
			mVertexArr[idx].z = 0;
			mVertexArr[idx].extrusionLength = 1;
			idx++;

			mVertexArr[idx].x = Pos.x();
			mVertexArr[idx].y = Pos.y();
			mVertexArr[idx].z = -RayLength;
			mVertexArr[idx].extrusionLength = 1;
			idx++;

			mVertexArr[idx].x = Pos.x() + Dis;
			mVertexArr[idx].y = Pos.y();
			mVertexArr[idx].z = -RayLength;
			mVertexArr[idx].extrusionLength = 1;
			idx++;

			mVertexArr[idx].x = 0;
			mVertexArr[idx].y = 0;
			mVertexArr[idx].z = 0;
			mVertexArr[idx].extrusionLength = 1;
			idx++;

			mVertexArr[idx].x = Pos.x() + Dis;
			mVertexArr[idx].y = Pos.y() + Dis;
			mVertexArr[idx].z = -RayLength;
			mVertexArr[idx].extrusionLength = 1;
			idx++;

			mVertexArr[idx].x = Pos.x() + Dis;
			mVertexArr[idx].y = Pos.y();
			mVertexArr[idx].z = -RayLength;
			mVertexArr[idx].extrusionLength = 1;
			idx++;

			mVertexArr[idx].x = 0;
			mVertexArr[idx].y = 0;
			mVertexArr[idx].z = 0;
			mVertexArr[idx].extrusionLength = 1;
			idx++;

			mVertexArr[idx].x = Pos.x();
			mVertexArr[idx].y = Pos.y() + Dis;
			mVertexArr[idx].z = -RayLength;
			mVertexArr[idx].extrusionLength = 1;
			idx++;

			mVertexArr[idx].x = Pos.x() + Dis;
			mVertexArr[idx].y = Pos.y() + Dis;
			mVertexArr[idx].z = -RayLength;
			mVertexArr[idx].extrusionLength = 1;
			idx++;

			mVertexArr[idx].x = 0;
			mVertexArr[idx].y = 0;
			mVertexArr[idx].z = 0;
			mVertexArr[idx].extrusionLength = 1;
			idx++;

			mVertexArr[idx].x = Pos.x();
			mVertexArr[idx].y = Pos.y() + Dis;
			mVertexArr[idx].z = -RayLength;
			mVertexArr[idx].extrusionLength = 1;
			idx++;

			mVertexArr[idx].x = Pos.x();
			mVertexArr[idx].y = Pos.y();
			mVertexArr[idx].z = -RayLength;
			mVertexArr[idx].extrusionLength = 1;
			idx++;
		}
	}
}

double Triton::HydraxGodRaysOpenGL::getFarWidth()
{
	double halfAngle = osg::DegreesToRadians(mFov) / 2.0;
	double centerUp = mFarClip * std::tan(halfAngle);
	double centerRight = centerUp * mAspect;
	double farWidth = centerRight * 2;

	return farWidth;
}

float Triton::HydraxGodRaysOpenGL::getWaveHeight( Triton::Vec2 pos )
{
	if(environment)
	{
		TRITON_VECTOR(Ocean *) ocs = environment->getOceans();
		if(!ocs.empty())
		{
			Triton::Ocean* oc = *ocs.begin();
			if(oc)
			{
				double z = -50000;
				Triton::Vector4 s(pos.x,pos.y,0,1.0);
				Triton::Vector4 e(pos.x,pos.y,z,1.0);
				Triton::Matrix4 tmp = mGodRayPosMat;
				tmp.Transpose();
				Triton::Vector4 ws4 = tmp * s;
				Triton::Vector4 we4 = tmp * e;
				Triton::Vector3 ws(ws4.x,ws4.y,ws4.z);
				Triton::Vector3 we(we4.x,we4.y,we4.z);
				Triton::Vector3 wd = we - ws;
				

				float wh = 0;
				Triton::Vector3 wn;
				if(oc->GetHeight(ws,wd,wh,wn,false,false,false,true))
				{
					return wh / environment->GetMaximumWaveHeight();
				}
			}
		}	
	}
	return 0.0f;
}

osg::Vec2 Triton::HydraxGodRaysOpenGL::calculateRayPosition( const int& RayNumber )
{
    double sqrt_NumberOfRays = std::sqrt((double)mNumberOfRays);
	double XCoord = RayNumber;

	while (XCoord >= sqrt_NumberOfRays)
	{
		XCoord -= sqrt_NumberOfRays;
	}

	osg::Vec2 RayPos =
		osg::Vec2( // X coord
		static_cast<int>(XCoord),
		// Y coord
		static_cast<int>((RayNumber+sqrt_NumberOfRays)/sqrt_NumberOfRays)-1);

	RayPos /= sqrt_NumberOfRays;
	RayPos -= osg::Vec2(0.5, 0.5);
	RayPos *= 2;

	osg::Vec2 Position = RayPos * mNoisePositionMultiplier;// + osg::Vec2(getGodRayPosition().x, getGodRayPosition().z);

	Triton::Vec2 mx;
	mx.x = Position.x() - mNoiseDerivation;
	mx.y = 0;
	double mxv = getWaveHeight(mx);

	Triton::Vec2 px;
	px.x = Position.x() + mNoiseDerivation;
	px.y = 0;
	double pxv = getWaveHeight(px);

	Triton::Vec2 my;
	my.x = 0;
	my.y = Position.y() - mNoiseDerivation;
	double myv = getWaveHeight(my);

	Triton::Vec2 py;
	py.x = 0;
	py.y = Position.y() + mNoiseDerivation;
	double pyv = getWaveHeight(py);

	float moveScale = 0.02;
	
	/*osg::Vec3d
		m_x = osg::Vec3d(Position.x() - mNoiseDerivation, mPerlin->getValue(Position.x-mNoiseDerivation,0), 0),
		p_x = osg::Vec3d(Position.x+mNoiseDerivation, mPerlin->getValue(Position.x+mNoiseDerivation,0), 0),
		m_y = osg::Vec3d(0, mPerlin->getValue(0,Position.y-mNoiseDerivation), Position.y-mNoiseDerivation),
		p_y = osg::Vec3d(0, mPerlin->getValue(0,Position.y+mNoiseDerivation), Position.y+mNoiseDerivation);*/

	osg::Vec3d
		m_x = osg::Vec3d(Position.x() - mNoiseDerivation, mxv * moveScale, 0),
		p_x = osg::Vec3d(Position.x() + mNoiseDerivation, pxv * moveScale, 0),
		m_y = osg::Vec3d(0, myv * moveScale, Position.y() - mNoiseDerivation),
		p_y = osg::Vec3d(0, pyv * moveScale, Position.y() + mNoiseDerivation);

	m_x.y() *= mNoiseYNormalMultiplier; p_x.y() *= mNoiseYNormalMultiplier;
	m_y.y() *= mNoiseYNormalMultiplier; p_y.y() *= mNoiseYNormalMultiplier;

	osg::Vec3d Normal = (p_x - m_x) ^ ((p_y - m_y));

	Normal *= mNoiseNormalMultiplier;

	return RayPos + osg::Vec2(Normal.x(), Normal.z());
}

void Triton::HydraxGodRaysOpenGL::update()
{
	updateGodRays();
}




