// Copyright (c) 2011-2015 Sundog Software LLC. All rights reserved worldwide.

#include <FeTriton/ParticleSystemOpenGL.h>
#include <FeTriton/Configuration.h>
#include <FeTriton/Environment.h>
#include <FeTriton/ProjectedGrid.h>
#include <FeTriton/Matrix4.h>
#include <string.h>
#ifdef _OPENMP
#include <omp.h>
#endif

// Persistent mapping is actually slower....
//#define USE_PERSISTENT_MAP
#define USE_BUFFER_SUBDATA

using namespace Triton;

typedef struct Vertex_S {
    float initialPosX, initialPosY, initialPosZ;
    float initialVelocityX, initialVelocityY, initialVelocityZ;
    float size, startTime;
    float offsetX, offsetY;
    float texCoordX, texCoordY;
} Vertex;

#define VERTEX_ELEMENTS 12

ParticleSystemOpenGL::~ParticleSystemOpenGL()
{
	if(environment)
	{
		osg::GLExtensions* et = environment->GetExtension();
		if(et)
		{
			et->glDeleteVertexArrays(1, &vertexArray);

			if (vboID) {
				et->glDeleteBuffers(1, &vboID);
			}
		}
	}
}

bool ParticleSystemOpenGL::UsingPointSprites()
{
#ifdef __APPLE__
    bool forcePointSprites = true;
    Configuration::GetBoolValue("spray-force-point-sprites-mac", forcePointSprites);
#else
    bool forcePointSprites = false;
    Configuration::GetBoolValue("spray-force-point-sprites", forcePointSprites);
#endif
    if (forcePointSprites) {
        return true;
    }

    bool usePointSprites = false;
    Configuration::GetBoolValue("spray-use-point-sprites", usePointSprites);

    if (usePointSprites) {
        // NVidia clamps point sprites to 63 pixels which doesn't cut it.
        int maxPointSize = 0;
        glGetIntegerv(GL_POINT_SIZE_MAX, &maxPointSize);
        return maxPointSize > 256;
    }

    return false;
}

void ParticleSystemOpenGL::SetShader(ShaderHandle shader)
{
    program = (GLhandleARB)shader;

#ifdef __APPLE__
    programCast = reinterpret_cast<uintptr_t>(program);
#else
    programCast = program;
#endif

}

UniLocs ParticleSystemOpenGL::GetUniLocsFromShader()
{
	if(environment)
	{
		osg::GLExtensions* et = environment->GetExtension();
		if(et)
		{
			ul.initialPosLoc = et->glGetAttribLocation(programCast, "initialPosition");
			ul.initialVelLoc = et->glGetAttribLocation(programCast, "initialVelocity");
			ul.sizeLoc = et->glGetAttribLocation(programCast, "size");
			ul.startTimeLoc = et->glGetAttribLocation(programCast, "startTime");
			ul.offsetXLoc = et->glGetAttribLocation(programCast, "offsetX");
			ul.offsetYLoc = et->glGetAttribLocation(programCast, "offsetY");
			ul.texCoordXLoc = et->glGetAttribLocation(programCast, "texCoordX");
			ul.texCoordYLoc = et->glGetAttribLocation(programCast, "texCoordY");

			ul.modelViewLoc = et->glGetUniformLocation(programCast, "trit_modelView");
			ul.modelViewProjLoc = et->glGetUniformLocation(programCast, "trit_mvProj");
			ul.timeLoc = et->glGetUniformLocation(programCast, "trit_time");
			ul.gLoc = et->glGetUniformLocation(programCast, "trit_g");
			ul.particleTextureLoc = et->glGetUniformLocation(programCast, "trit_particleTexture");
			ul.lightColorLoc = et->glGetUniformLocation(programCast, "trit_lightColor");
			ul.transparencyLoc = et->glGetUniformLocation(programCast, "trit_transparency");
			ul.cameraPosLoc = et->glGetUniformLocation(programCast, "trit_cameraPos");

			ul.heightMapLoc = et->glGetUniformLocation(programCast, "trit_heightMap");
			ul.heightMapRangeOffsetLoc = et->glGetUniformLocation(programCast, "trit_heightMapRangeOffset");
			ul.heightMapMatrixLoc = et->glGetUniformLocation(programCast, "trit_heightMapMatrix");
			ul.hasHeightMapLoc = et->glGetUniformLocation(programCast, "trit_hasHeightMap");
			ul.hasUserHeightMapLoc = et->glGetUniformLocation(programCast, "trit_hasUserHeightMap");
			ul.invSizeFactorLoc = et->glGetUniformLocation(programCast, "trit_invSizeFactor");
			ul.seaLevelLoc = et->glGetUniformLocation(programCast, "trit_seaLevel");

			ul.refOffsetLoc = et->glGetUniformLocation(programCast, "trit_refOffset");

			ul.fogDensityLoc = et->glGetUniformLocation(programCast, "trit_fogDensity");
			ul.fogColorLoc = et->glGetUniformLocation(programCast, "trit_fogColor");
		}
	}
   
    return ul;
}


bool ParticleSystemOpenGL::Initialize(TextureHandle texture, ShaderHandle shader, int pNumParticles, const ProjectedGrid *pGrid)
{
    if (!texture || !shader || !pGrid) return false;

	if(!environment)
		return false;
	osg::GLExtensions* et = environment->GetExtension();
	if(!et)
		return false;

    grid = pGrid;

    bool ok = true;

    numParticles = pNumParticles;

    program = (GLhandleARB)shader;

#ifdef __APPLE__
    programCast = reinterpret_cast<uintptr_t>(program);
#else
    programCast = program;
#endif

    textureID = (GLint)texture;

    usePointSprites = UsingPointSprites();

    //Utils::ClearGLErrors();

    vertsPerParticle = usePointSprites ? 1 : 6;

    et->glGenBuffers(1, &vboID);
    et->glBindBuffer(GL_ARRAY_BUFFER, vboID);
    int buffSize = pNumParticles * sizeof(Vertex)* vertsPerParticle;
    char *zeros = TRITON_NEW char[buffSize];
    memset(zeros, 0, buffSize);

#ifdef USE_PERSISTENT_MAP
    if (OpenGLExtensionManager::HasExtension("GL_ARB_buffer_storage")) {
        GLbitfield mapFlags = GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT;
        GLbitfield createFlags = mapFlags | GL_DYNAMIC_STORAGE_BIT;
        glBufferStorage(GL_ARRAY_BUFFER, buffSize, zeros, createFlags);
        vertexDataPtr = glMapBufferRange(GL_ARRAY_BUFFER, 0, buffSize, mapFlags);
    } else {
        glBufferData(GL_ARRAY_BUFFER, buffSize, zeros, GL_DYNAMIC_DRAW);
    }
#else
    et->glBufferData(GL_ARRAY_BUFFER, buffSize, zeros, GL_DYNAMIC_DRAW);
#endif
    TRITON_DELETE[] zeros;
    et->glBindBuffer(GL_ARRAY_BUFFER, 0);

    if (OpenGLExtensionManager::HasExtension("GL_ARB_vertex_array_object",environment)) {
        et->glGenVertexArrays(1, &vertexArray);
    }

    GetUniLocsFromShader();

    if (ul.modelViewLoc == -1 || ul.modelViewProjLoc == -1 || ul.timeLoc == -1
            || ul.gLoc == -1 || ul.particleTextureLoc == -1 || ul.lightColorLoc == -1
            || ul.transparencyLoc == -1 || ul.cameraPosLoc == -1 || ul.hasHeightMapLoc == -1
            || ul.heightMapMatrixLoc == -1 || ul.heightMapLoc == -1 || ul.heightMapRangeOffsetLoc == -1
            || ul.invSizeFactorLoc == -1 || ul.refOffsetLoc == -1 || ul.fogDensityLoc == -1
            || ul.fogColorLoc == -1 || ul.hasUserHeightMapLoc == -1 || ul.seaLevelLoc == -1) {
        Utils::DebugMsg("Expected particle uniforms not found.");
        return false;
    }

    //if (!Utils::PrintGLErrors(__FILE__, __LINE__)) ok = false;

    return ok;
}

void ParticleSystemOpenGL::SetVerts(float *buffer, int startIdx, const Vector3& position, const Vector3& direction, float size, float time)
{
    int idx = startIdx;

    Vector3 localReference, diffFromCamera;
    GetLocalReference(localReference, diffFromCamera);

    if (usePointSprites) {
        buffer[idx++] = (float)(position.x - localReference.x);
        buffer[idx++] = (float)(position.y - localReference.y);
        buffer[idx++] = (float)(position.z - localReference.z);
        buffer[idx++] = (float)direction.x;
        buffer[idx++] = (float)direction.y;
        buffer[idx++] = (float)direction.z;
        buffer[idx++] = size;
        buffer[idx++] = time;
        buffer[idx++] = 0;
        buffer[idx++] = 0;
        buffer[idx++] = 0;
        buffer[idx++] = 0;
    } else {
        //  2 5-4
        //  |\ \|
        //  0-1 3

        struct Offset {
            float x, y;
        } ;
        Offset offsets[6];
        Offset texCoords[6];

        float halfSize = size * invSizeFactor * 0.5f;
        offsets[0].x = -halfSize;
        offsets[0].y = -halfSize;
        texCoords[0].x = 0.0f;
        texCoords[0].y = 1.0f;

        offsets[1].x = halfSize;
        offsets[1].y = -halfSize;
        texCoords[1].x = 1.0f;
        texCoords[1].y = 1.0f;

        offsets[2].x = -halfSize;
        offsets[2].y = halfSize;
        texCoords[2].x = 0.0f;
        texCoords[2].y = 0.0f;

        offsets[3] = offsets[1];
        texCoords[3] = texCoords[1];

        offsets[4].x = halfSize;
        offsets[4].y = halfSize;
        texCoords[4].x = 1.0f;
        texCoords[4].y = 0.0f;

        offsets[5] = offsets[2];
        texCoords[5] = texCoords[2];

        for (int vert = 0; vert < 6; vert++) {
            buffer[idx++] = (float)(position.x - localReference.x);
            buffer[idx++] = (float)(position.y - localReference.y);
            buffer[idx++] = (float)(position.z - localReference.z);
            buffer[idx++] = (float)direction.x;
            buffer[idx++] = (float)direction.y;
            buffer[idx++] = (float)direction.z;
            buffer[idx++] = size;
            buffer[idx++] = time;
            buffer[idx++] = offsets[vert].x;
            buffer[idx++] = offsets[vert].y;
            buffer[idx++] = texCoords[vert].x;
            buffer[idx++] = texCoords[vert].y;
        }
    }
}

void ParticleSystemOpenGL::StartBatch(double pStartTime, const TRITON_VECTOR(Vector3)&positions,
                                      const Vector3& direction, float dirVariation, float velocity, float velVariation,
                                      float posVariation, float size, float sizeVariation)
{
    //Utils::ClearGLErrors();
	if(!environment)
		return;
	osg::GLExtensions* et = environment->GetExtension();
	if(!et)
		return;

    startTime = pStartTime;

    if (referenceTime == 0) referenceTime = pStartTime;

    GLint vp[4];

    if (!environment->GetViewport(vp[0], vp[1], vp[2], vp[3])) {
        glGetIntegerv(GL_VIEWPORT, vp);
    }

    invSizeFactor = 1.0f / (float)vp[2];

    et->glBindBuffer(GL_ARRAY_BUFFER, vboID);

    void *p = 0;
    float *buffer = 0;

    int numPositions = (int)positions.size();
    int startOffset = offset;
    if (numPositions + offset >= numParticles) {
        numPositions = numParticles - offset - 1;
        offset = 0;
    } else {
        offset += numPositions;
    }

    int startIdx = 0;

#ifdef USE_BUFFER_SUBDATA
    p = TRITON_NEW float[numPositions * sizeof(Vertex) * vertsPerParticle];
#else
    if (vertexDataPtr) {
        p = vertexDataPtr;
        startIdx = startOffset * VERTEX_ELEMENTS;
    } else {
        if (OpenGLExtensionManager::HasExtension("GL_ARB_map_buffer_range")) {
            p = glMapBufferRange(GL_ARRAY_BUFFER, startOffset * sizeof(Vertex)* vertsPerParticle, positions.size() * sizeof(Vertex)* vertsPerParticle,
                                 GL_MAP_WRITE_BIT | GL_MAP_UNSYNCHRONIZED_BIT | GL_MAP_INVALIDATE_RANGE_BIT);
        } else {
            p = glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY_ARB);
            startIdx = startOffset * VERTEX_ELEMENTS;
        }
    }
#endif

    if (p) {
        buffer = (float *)p;
        float particleTime = (float)(pStartTime - referenceTime);

        const RandomNumberGenerator *rng = environment->GetRandomNumberGenerator();
        if (!rng) return;

        if (environment->GetOpenMPEnabled()) {
            #pragma omp parallel for
            for (int i = 0; i < numPositions; i++) {
                Vector3 pos = positions[i];
                Vector3 poffset(rng->GetRandomDouble(-posVariation, posVariation),
                                rng->GetRandomDouble(-posVariation, posVariation), rng->GetRandomDouble(-posVariation, posVariation));
                pos = pos + poffset;

                Vector3 dir = direction;
                Vector3 dirOffset(rng->GetRandomDouble(-dirVariation, dirVariation),
                                  rng->GetRandomDouble(-dirVariation, dirVariation), rng->GetRandomDouble(-dirVariation, dirVariation));
                dir = dir + dirOffset;
                dir.Normalize();
                dir = dir * (velocity + rng->GetRandomDouble(-velVariation, velVariation));

                float newSize = size + (float)rng->GetRandomDouble(-sizeVariation, sizeVariation);

                int idx = startIdx + i * VERTEX_ELEMENTS * vertsPerParticle;

                SetVerts(buffer, idx, pos, dir, newSize * vp[2] * environment->GetZoomLevel(), particleTime);
            }
        } else {
            for (int i = 0; i < numPositions; i++) {
                Vector3 pos = positions[i];
                Vector3 poffset(rng->GetRandomDouble(-posVariation, posVariation),
                                rng->GetRandomDouble(-posVariation, posVariation), rng->GetRandomDouble(-posVariation, posVariation));
                pos = pos + poffset;

                Vector3 dir = direction;
                Vector3 dirOffset(rng->GetRandomDouble(-dirVariation, dirVariation),
                                  rng->GetRandomDouble(-dirVariation, dirVariation), rng->GetRandomDouble(-dirVariation, dirVariation));
                dir = dir + dirOffset;
                dir.Normalize();
                dir = dir * (velocity + rng->GetRandomDouble(-velVariation, velVariation));

                float newSize = size + (float)rng->GetRandomDouble(-sizeVariation, sizeVariation);

                int idx = startIdx + i * VERTEX_ELEMENTS * vertsPerParticle;
                SetVerts(buffer, idx, pos, dir, newSize * vp[2] * environment->GetZoomLevel(), particleTime);
            }
        }

#ifdef USE_BUFFER_SUBDATA
        et->glBufferSubData(GL_ARRAY_BUFFER, startOffset * sizeof(Vertex) * vertsPerParticle, numPositions * sizeof(Vertex) * vertsPerParticle, p);
        TRITON_DELETE[] (float*)p;
#else
        if (!vertexDataPtr) {
            glUnmapBuffer(GL_ARRAY_BUFFER);
        }
#endif

    } else {
        Utils::DebugMsg("Could not map particle buffer.");
    }

    et->glBindBuffer(GL_ARRAY_BUFFER, 0);

    //Utils::PrintGLErrors(__FILE__, __LINE__);
}

void ParticleSystemOpenGL::Start(double pStartTime, const Vector3& direction, float dirVariation,
                                 float velocity, float velVariation, const Vector3& position, float posVariation,
                                 float size, float sizeVariation)
{
    //Utils::ClearGLErrors();
	if(!environment)
		return;
	osg::GLExtensions* et = environment->GetExtension();
	if(!et)
		return;

    if (referenceTime == 0) referenceTime = pStartTime;

    double maxP0 = posVariation;
    double maxV0 = velocity + velVariation;
    ComputeExpirationTime(pStartTime, maxP0, maxV0);

    GLint vp[4];
    if (!environment->GetViewport(vp[0], vp[1], vp[2], vp[3])) {
        glGetIntegerv(GL_VIEWPORT, vp);
    }

    invSizeFactor = 1.0f / (float)vp[2];

    et->glBindBuffer(GL_ARRAY_BUFFER, vboID);

    void *p;
    if (vertexDataPtr) {
        p = vertexDataPtr;
    } else {
        p = et->glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY_ARB);
    }

    if (p) {
        float *buffer = (float *)p;

        const RandomNumberGenerator *rng = environment->GetRandomNumberGenerator();
        if (!rng) return;

        for (int i = 0; i < numParticles; i++) {
            Vector3 pos = position;
            Vector3 offset(rng->GetRandomDouble(-posVariation, posVariation),
                           rng->GetRandomDouble(-posVariation, posVariation), rng->GetRandomDouble(-posVariation, posVariation));

            // Project onto the ocean plane
            Vector3 P = pos + offset;
            Vector3 O = pos;
            Vector3 N = environment->GetUpVector();
            Vector3 Q = P - N * ( (P-O).Dot(N) );
            pos = Q;

            Vector3 dir = direction;
            Vector3 dirOffset(rng->GetRandomDouble(-dirVariation, dirVariation),
                              rng->GetRandomDouble(-dirVariation, dirVariation), rng->GetRandomDouble(-dirVariation, dirVariation));
            dir = dir + dirOffset;
            dir.Normalize();
            dir = dir * (velocity + rng->GetRandomDouble(-velVariation, velVariation));

            float newSize = size + (float)rng->GetRandomDouble(-sizeVariation, sizeVariation);
            int idx = i * VERTEX_ELEMENTS * vertsPerParticle;
            SetVerts(buffer, idx, pos, dir, newSize * vp[2] * environment->GetZoomLevel(), (float)(pStartTime - referenceTime));
        }

        if (!vertexDataPtr) et->glUnmapBuffer(GL_ARRAY_BUFFER);
    } else {
        Utils::DebugMsg("Could not map particle buffer.");
    }

    et->glBindBuffer(GL_ARRAY_BUFFER, 0);

    //Utils::PrintGLErrors(__FILE__, __LINE__);
}

void ParticleSystemOpenGL::SetupCommonState(double time)
{
    if (!environment || !program) return;

    //Utils::ClearGLErrors();

	if(!environment)
		return;
	osg::GLExtensions* et = environment->GetExtension();
	if(!et)
		return;

    et->glUseProgram(programCast);

    Matrix4 mv(environment->GetCameraMatrix());
    mv.elem[3][0] = 0;
    mv.elem[3][1] = 0;
    mv.elem[3][2] = 0;
    Matrix4 proj(environment->GetProjectionMatrix());
    Matrix4 mvproj = mv * proj;

    OpenGLCachedUniforms::SetUniformMat4(environment, programCast, ul.modelViewLoc, mv.ToFloatArray());
    OpenGLCachedUniforms::SetUniformMat4(environment, programCast, ul.modelViewProjLoc, mvproj.ToFloatArray());
    OpenGLCachedUniforms::SetUniformFloat(environment, programCast, ul.timeLoc, (float)(time - referenceTime));
    Vector3 g = environment->GetUpVector() * ((gravity * -1) / environment->GetWorldUnits());
    OpenGLCachedUniforms::SetUniformVec3(environment, programCast, ul.gLoc, (float)g.x, (float)g.y, (float)g.z);
    Vector3 camPos = environment->GetCameraPosition();
    OpenGLCachedUniforms::SetUniformVec3(environment, programCast, ul.cameraPosLoc, (float)camPos.x, (float)camPos.y, (float)camPos.z);
    const Vector3& lightColor = environment->GetAmbientLightColor();
    OpenGLCachedUniforms::SetUniformVec4(environment, programCast, ul.lightColorLoc, (float)lightColor.x, (float)lightColor.y, (float)lightColor.z, 1.0f);

    OpenGLCachedUniforms::SetUniformInt(environment, programCast, ul.particleTextureLoc, 0);

    Vector3 fogColor;
    double visibility, fogDensity;
    environment->GetAboveWaterVisibility(visibility, fogColor);
    fogDensity = 3.912 / visibility;
    OpenGLCachedUniforms::SetUniformVec3(environment, programCast, ul.fogColorLoc, (GLfloat)fogColor.x, (GLfloat)fogColor.y, (GLfloat)fogColor.z);
    OpenGLCachedUniforms::SetUniformFloat(environment, programCast, ul.fogDensityLoc, (GLfloat)fogDensity);

    if (environment->GetHeightMap()) {
        glActiveTexture(GL_TEXTURE5);
        glBindTexture(GL_TEXTURE_2D, (GLuint)environment->GetHeightMap());
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        OpenGLCachedUniforms::SetUniformInt(environment, programCast, ul.heightMapLoc, 5);
        OpenGLCachedUniforms::SetUniformVec2(environment, programCast, ul.heightMapRangeOffsetLoc,
                                             environment->GetHeightMapRange(), environment->GetHeightMapOffset());
        OpenGLCachedUniforms::SetUniformInt(environment, programCast, ul.hasHeightMapLoc, GL_TRUE);
        OpenGLCachedUniforms::SetUniformMat4(environment, programCast, ul.heightMapMatrixLoc, environment->GetHeightMapMatrix().ToFloatArray());
        OpenGLCachedUniforms::SetUniformFloat(environment, programCast, ul.invSizeFactorLoc, invSizeFactor);
        OpenGLCachedUniforms::SetUniformFloat(environment, programCast, ul.seaLevelLoc, (float)environment->GetSeaLevel());
    } else {
        OpenGLCachedUniforms::SetUniformInt(environment, programCast, ul.hasHeightMapLoc, GL_FALSE);
    }

    if (environment->GetUserHeightCB()) {
        OpenGLCachedUniforms::SetUniformInt(environment, programCast, ul.hasUserHeightMapLoc, GL_TRUE);
        OpenGLCachedUniforms::SetUniformVec2(environment, programCast, ul.heightMapRangeOffsetLoc,
                                             environment->GetHeightMapRange(), environment->GetHeightMapOffset());
        OpenGLCachedUniforms::SetUniformFloat(environment, programCast, ul.invSizeFactorLoc, invSizeFactor);
        OpenGLCachedUniforms::SetUniformFloat(environment, programCast, ul.seaLevelLoc, (float)environment->GetSeaLevel());
    } else {
        OpenGLCachedUniforms::SetUniformInt(environment, programCast, ul.hasUserHeightMapLoc, GL_FALSE);
    }

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textureID);

    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, pureAdditive ? GL_ONE : GL_ONE_MINUS_SRC_COLOR);
    glDepthMask(GL_FALSE);

    glEnable(GL_DEPTH_TEST);
    if (usePointSprites) {
        glEnable(GL_VERTEX_PROGRAM_POINT_SIZE);
    }
    glDisable(GL_CULL_FACE);

    //Utils::PrintGLErrors(__FILE__, __LINE__);
}

void ParticleSystemOpenGL::CleanupCommonState()
{
	if(!environment)
		return;
	osg::GLExtensions* et = environment->GetExtension();
	if(!et)
		return;


    if (environment && environment->GetRenderer() >= OPENGL_3_2) {
        et->glBindVertexArray(vertexArray);
    }

    glDisable(GL_VERTEX_PROGRAM_POINT_SIZE);
    et->glDisableVertexAttribArray(ul.initialPosLoc);
    et->glDisableVertexAttribArray(ul.initialVelLoc);
    et->glDisableVertexAttribArray(ul.sizeLoc);
    et->glDisableVertexAttribArray(ul.startTimeLoc);
    et->glDisableVertexAttribArray(ul.offsetXLoc);
    et->glDisableVertexAttribArray(ul.offsetYLoc);

    if (ul.texCoordXLoc != -1 && ul.texCoordYLoc != -1) {
        et->glDisableVertexAttribArray(ul.texCoordXLoc);
        et->glDisableVertexAttribArray(ul.texCoordYLoc);
    }

    if (environment && environment->GetRenderer() >= OPENGL_3_2) {
        et->glBindVertexArray(0);
    } else {
        glDisable(GL_POINT_SPRITE);
        glTexEnvi(GL_POINT_SPRITE, GL_COORD_REPLACE, GL_FALSE);
        glDisableClientState(GL_VERTEX_ARRAY);
    }

    et->glBindBuffer(GL_ARRAY_BUFFER, 0);

    glEnable(GL_CULL_FACE);
    glDisable(GL_BLEND);
    glDepthMask(GL_TRUE);

    //glUseProgram(0);

    //Utils::PrintGLErrors(__FILE__, __LINE__);
}

bool ParticleSystemOpenGL::Draw()
{
    if (!environment || !program) return false;

	osg::GLExtensions* et = environment->GetExtension();
	if(!et)
		return false;

    //Utils::ClearGLErrors();

    OpenGLCachedUniforms::SetUniformFloat(environment, programCast, ul.transparencyLoc, transparency);

    Vector3 localReference, diffFromCamera;
    GetLocalReference(localReference, diffFromCamera);
    OpenGLCachedUniforms::SetUniformVec3(environment, programCast, ul.refOffsetLoc, (float)diffFromCamera.x, (float)diffFromCamera.y, (float)diffFromCamera.z);

    if (vboID) {

        if (environment && environment->GetRenderer() >= OPENGL_3_2) {
            et->glBindVertexArray(vertexArray);
        } else {
            if (usePointSprites) {
                glEnable(GL_POINT_SPRITE);
                glTexEnvi(GL_POINT_SPRITE, GL_COORD_REPLACE, GL_TRUE);
            }
            glEnableClientState(GL_VERTEX_ARRAY);
        }

        et->glBindBuffer(GL_ARRAY_BUFFER, vboID);
        et->glVertexAttribPointer(ul.initialPosLoc, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid *)0);
        et->glVertexAttribPointer(ul.initialVelLoc, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid *)(3 * sizeof(float)));
        et->glVertexAttribPointer(ul.sizeLoc, 1, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid *)(6 * sizeof(float)));
        et->glVertexAttribPointer(ul.startTimeLoc, 1, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid *)(7 * sizeof(float)));
        et->glVertexAttribPointer(ul.offsetXLoc, 1, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)(8 * sizeof(float)));
        et->glVertexAttribPointer(ul.offsetYLoc, 1, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)(9 * sizeof(float)));

        if (!usePointSprites && ul.texCoordXLoc != -1 && ul.texCoordYLoc != -1) {
            et->glVertexAttribPointer(ul.texCoordXLoc, 1, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)(10 * sizeof(float)));
            et->glVertexAttribPointer(ul.texCoordYLoc, 1, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)(11 * sizeof(float)));
            et->glEnableVertexAttribArray(ul.texCoordXLoc);
            et->glEnableVertexAttribArray(ul.texCoordYLoc);
        }

        et->glEnableVertexAttribArray(ul.initialPosLoc);
        et->glEnableVertexAttribArray(ul.initialVelLoc);
        et->glEnableVertexAttribArray(ul.sizeLoc);
        et->glEnableVertexAttribArray(ul.startTimeLoc);
        et->glEnableVertexAttribArray(ul.offsetXLoc);
        et->glEnableVertexAttribArray(ul.offsetYLoc);

        if (alphaMask) glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_FALSE);

        glDrawArrays(usePointSprites ? GL_POINTS : GL_TRIANGLES, 0, numParticles * vertsPerParticle);

        if (alphaMask) glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
    }


    //return Utils::PrintGLErrors(__FILE__, __LINE__);
	return true;
}
