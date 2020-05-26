// Copyright (c) 2011-2015 Sundog Software LLC. All rights reserved worldwide.

#include <FeTriton/Configuration.h>
#include <FeTriton/FFTWaterModelOpenGL.h>
#include <FeTriton/Environment.h>
#include <FeTriton/FFT.h>
#include <string.h>

using namespace Triton;

FFTWaterModelOpenGL::FFTWaterModelOpenGL(const Environment *env, WaterModelTypes type, OceanQuality quality) : FFTWaterModel(env, type, quality)
{
    displacementTexture = slopeFoamTexture = 0;
    hasFBO = true;
    hasPBO = false;

    pbo[0] = pbo[1] = 0;
    sync[0] = sync[1] = 0;
    pboValid[0] = pboValid[1] = false;
    pboIndex = 0;
    pboMapped[0] = pboMapped[1] = false;
    mappedData = 0;
}

FFTWaterModelOpenGL::~FFTWaterModelOpenGL()
{
    if (displacementTexture) {
        glDeleteTextures(1, &displacementTexture);
        displacementTexture = 0;
    }

    if (slopeFoamTexture) {
        glDeleteTextures(1, &slopeFoamTexture);
        slopeFoamTexture = 0;
    }

	if(environment)
	{
		osg::GLExtensions* et = environment->GetExtension();
		if(et)
		{
			if (sync[0]) {
				et->glDeleteSync(sync[0]);
				sync[0] = 0;
			}

			if (sync[1]) {
				et->glDeleteSync(sync[1]);
				sync[1] = 0;
			}

			if (pbo[0]) {
				if (pboMapped[0]) {
					et->glBindBuffer(GL_PIXEL_PACK_BUFFER, pbo[0]);
					et->glUnmapBuffer(GL_PIXEL_PACK_BUFFER);
					pboMapped[0] = false;
					et->glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);
				}
				et->glDeleteBuffers(1, &pbo[0]);
				heightMap = 0;
			}

			if (pbo[1]) {
				if (pboMapped[1]) {
					et->glBindBuffer(GL_PIXEL_PACK_BUFFER, pbo[1]);
					et->glUnmapBuffer(GL_PIXEL_PACK_BUFFER);
					pboMapped[1] = false;
					et->glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);
				}
				et->glDeleteBuffers(1, &pbo[1]);
				heightMap = 0;
			}
		}
	}
}

void FFTWaterModelOpenGL::ReloadGridUniforms(ShaderHandle gridShader, ShaderHandle gridPatchShader)
{
	if(!environment)
		return;

	osg::GLExtensions* et = environment->GetExtension();
	if(!et)
		return;

    shader = gridShader;
    patchShader = gridPatchShader;

    displacementLoc = et->glGetUniformLocation((GLint)shader, "trit_displacementMap");

    slopeFoamLoc = et->glGetUniformLocation((GLint)shader, "trit_slopeFoamMap");

    textureSizeLoc = et->glGetUniformLocation((GLint)shader, "trit_textureSize");

    noiseAmplitudeLoc = et->glGetUniformLocation((GLint)shader, "trit_noiseAmplitude");

    invNoiseDistanceLoc = et->glGetUniformLocation((GLint)shader, "trit_invNoiseDistance");

    invDampingDistanceLoc = et->glGetUniformLocation((GLint)shader, "trit_invDampingDistance");

    lodBiasLoc = et->glGetUniformLocation((GLint)shader, "trit_textureLODBias");

    timeLoc = et->glGetUniformLocation((GLint)shader, "trit_time");

    displacementLocPatch = et->glGetUniformLocation((GLint)patchShader, "trit_displacementMap");

    slopeFoamLocPatch = et->glGetUniformLocation((GLint)patchShader, "trit_slopeFoamMap");

    textureSizeLocPatch = et->glGetUniformLocation((GLint)patchShader, "trit_textureSize");

    noiseAmplitudeLocPatch = et->glGetUniformLocation((GLint)patchShader, "trit_noiseAmplitude");

    invNoiseDistanceLocPatch = et->glGetUniformLocation((GLint)patchShader, "trit_invNoiseDistance");

    invDampingDistanceLocPatch = et->glGetUniformLocation((GLint)patchShader, "trit_invDampingDistance");

    lodBiasLocPatch = et->glGetUniformLocation((GLint)patchShader, "trit_textureLODBias");

    timeLocPatch = et->glGetUniformLocation((GLint)patchShader, "trit_time");

    OpenGLCachedUniforms::ClearCaches();
}

bool FFTWaterModelOpenGL::Initialize(ShaderHandle shader, ShaderHandle patchShader, bool enableHeightReads, bool noGPU)
{
    Utils::ClearGLErrors();

	if(!environment)
		return false;
	osg::GLExtensions* et = environment->GetExtension();
	if(!et)
		return false;

    if (FFTWaterModel::Initialize(shader, patchShader, enableHeightReads, noGPU)) {

        hasFBO = OpenGLExtensionManager::HasExtension("GL_EXT_framebuffer_object",environment);

        hasPBO = OpenGLExtensionManager::HasExtension("GL_ARB_pixel_buffer_object",environment)  &&
                 OpenGLExtensionManager::HasExtension("GL_ARB_sync",environment);

        bool pboEnabled = false;
        Configuration::GetBoolValue("height-map-copy-use-pbo", pboEnabled);
        if (!pboEnabled) hasPBO = false;

        ReloadGridUniforms(shader, patchShader);

        glGenTextures(1, &displacementTexture);
        glBindTexture(GL_TEXTURE_2D, displacementTexture);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, N, M, 0, GL_RGB, GL_FLOAT, NULL);

        glGenTextures(1, &slopeFoamTexture);
        glBindTexture(GL_TEXTURE_2D, slopeFoamTexture);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

        float *buf = TRITON_NEW float[N * M * 4];
        memset(buf, 0, N * M * sizeof(float) * 4);

        if (environment && (!hasFBO && environment->GetRenderer() < OPENGL_3_2)) {
            glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, N, M, 0, GL_RGBA, GL_FLOAT, buf);
        } else {
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, N, M, 0, GL_RGBA, GL_FLOAT, buf);
            et->glGenerateMipmap(GL_TEXTURE_2D);
        }
        TRITON_DELETE[] buf;

        if (!fft->SetOutputTextures(displacementTexture, slopeFoamTexture)) {
            return false;
        }

        slopeFoamTexHandle = (TextureHandle)slopeFoamTexture;
        displacementTexHandle = (TextureHandle)displacementTexture;

        return Utils::PrintGLErrors(__FILE__, __LINE__);
    }
    return false;
}

bool FFTWaterModelOpenGL::HasHeightMap()
{
    return (pbo[0] && pbo[1]) || heightMap || ( environment && environment->GetUserHeightCB());
}

float FFTWaterModelOpenGL::GetHeightData(const Vector3& position, float defaultDepth)
{
    if (pbo[0] && pbo[1]) 
	{
		if(environment)
		{
			osg::GLExtensions* et = environment->GetExtension();
			if(et)
			{
				Utils::ClearGLErrors();

				bool dataReady = false;

				if (sync[pboIndex]) {
					dataReady = et->glClientWaitSync(sync[pboIndex], 0, 0) == GL_ALREADY_SIGNALED;
				}

				int readIndex = pboIndex;
				if (!dataReady) {
					readIndex = readIndex == 0 ? 1 : 0;
				}

				if (pboValid[readIndex]) {

					if (!pboMapped[readIndex]) {
						et->glBindBuffer(GL_PIXEL_PACK_BUFFER, pbo[readIndex]);
						mappedData = (float*)et->glMapBuffer(GL_PIXEL_PACK_BUFFER, GL_READ_ONLY);
						pboMapped[readIndex] = true;
						et->glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);
					}

					if (mappedData) {
						heightMapMatrix = newHeightMapMatrix;

						Vector4 texCoords = Vector4(position) * heightMapMatrix;
						if (texCoords.x < 0 || texCoords.x >= 1.0 || texCoords.y < 0 || texCoords.y >= 1.0) {
							return -defaultDepth;
						}
						int x = (int)((double)heightMapWidth * texCoords.x);
						int y = (int)((double)heightMapHeight * texCoords.y);

						float height = mappedData[y * heightMapWidth + x]*environment->GetHeightMapRange() +
							environment->GetHeightMapOffset();

						return height;
					} else {
						Utils::DebugMsg("Error mapping height map buffer.");
					}

				}

				Utils::PrintGLErrors(__FILE__, __LINE__);
			}
		}    
    }

    return FFTWaterModel::GetHeightData(position, defaultDepth);
}

void FFTWaterModelOpenGL::UpdateHeightMap(Environment *env)
{
    Utils::ClearGLErrors();

	if(!environment)
		return;
	osg::GLExtensions* et = environment->GetExtension();
	if(!et)
		return;

    TextureHandle tex = env->GetHeightMap();
    if (tex) {
        GLuint texID = (GLuint)tex;
        glBindTexture(GL_TEXTURE_2D, texID);

        int width, height;
        glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &width);
        glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &height);

        if (width != heightMapWidth || height != heightMapHeight) {
            heightMapWidth = width;
            heightMapHeight = height;

            if (hasPBO) {
                int format;
                glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_INTERNAL_FORMAT, &format);

                int dataSize = 0;
                switch (format) {
                case GL_COMPRESSED_RED_RGTC1:
                case GL_COMPRESSED_LUMINANCE_LATC1_EXT:
                case GL_LUMINANCE32F_ARB:
                    dataSize = width * height * 4;
                    break;
                }

                if (dataSize > 0) {
                    for (int idx = 0; idx < 2; idx++) {
                        if (pbo[idx]) {
                            if (pboMapped[idx]) {
                                et->glBindBuffer(GL_PIXEL_PACK_BUFFER_ARB, pbo[idx]);
                                et->glUnmapBuffer(GL_PIXEL_PACK_BUFFER_ARB);
                                et->glBindBuffer(GL_PIXEL_PACK_BUFFER_ARB, 0);
                                pboMapped[idx] = false;
                            }
                            et->glDeleteBuffers(1, &pbo[idx]);
                        }
                        et->glGenBuffers(1, &pbo[idx]);
                        et->glBindBuffer(GL_PIXEL_PACK_BUFFER_ARB, pbo[idx]);
                        et->glBufferData(GL_PIXEL_PACK_BUFFER_ARB, dataSize, 0, GL_STREAM_READ_ARB);// GL_DYNAMIC_READ);
                        et->glBindBuffer(GL_PIXEL_PACK_BUFFER_ARB, 0);
                        pboValid[idx] = false;
                    }
                }
            }

            if (pbo[0] == 0) {
                if (heightMap) TRITON_DELETE[] heightMap;
                heightMap = TRITON_NEW float[heightMapWidth * heightMapHeight];
            }
        }

        if (pbo[0] && pbo[1]) {

            newHeightMapMatrix = env->GetHeightMapMatrix();

            pboIndex = pboIndex == 0 ? 1 : 0;

            et->glBindBuffer(GL_PIXEL_PACK_BUFFER, pbo[pboIndex]);

            if (pboMapped[pboIndex]) {
                et->glUnmapBuffer(GL_PIXEL_PACK_BUFFER);
                pboMapped[pboIndex] = false;
            }

            glGetTexImage(GL_TEXTURE_2D, 0, GL_RED, GL_FLOAT, 0);

            if (sync[pboIndex]) {
                et->glDeleteSync(sync[pboIndex]);
            }
            sync[pboIndex] = et->glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);

            pboValid[pboIndex] = true;

            et->glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);
        } else {
            heightMapMatrix = env->GetHeightMapMatrix();
            glGetTexImage(GL_TEXTURE_2D, 0, GL_RED, GL_FLOAT, (void *)heightMap);
        }

        UpdateBreakingWaveDirection(env);
    }

    Utils::PrintGLErrors(__FILE__ , __LINE__);
}

bool FFTWaterModelOpenGL::PrepShaders(double time)
{
    if (!FFTWaterModel::PrepShaders(time))
        return false;

	if(!environment)
		return false;
	osg::GLExtensions* et = environment->GetExtension();
	if(!et)
		return false;

    Utils::ClearGLErrors();

    //glUseProgram((GLint)shader);

    if (!fft->TransformsEndToEnd()) {
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, displacementTexture);
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, N, M, GL_RGB, GL_FLOAT, displacementMap);

        glActiveTexture(GL_TEXTURE3);
        glBindTexture(GL_TEXTURE_2D, slopeFoamTexture);

        // Mipmap auto-generation not working reliably on all systems, sadly.
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, N, M, GL_RGBA, GL_FLOAT, slopeFoamMap);
        if (hasFBO || environment->GetRenderer() >= OPENGL_3_2) {
            et->glGenerateMipmap(GL_TEXTURE_2D);
        }

    } else {
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, displacementTexture);

        glActiveTexture(GL_TEXTURE3);
        glBindTexture(GL_TEXTURE_2D, slopeFoamTexture);

        if (hasFBO || environment->GetRenderer() >= OPENGL_3_2) {
            et->glGenerateMipmap(GL_TEXTURE_2D);
        }
    }



    OpenGLCachedUniforms::SetUniformInt(environment, (GLint)shader, displacementLoc, 2);
    OpenGLCachedUniforms::SetUniformInt(environment, (GLint)shader, slopeFoamLoc, 3);
    OpenGLCachedUniforms::SetUniformVec2(environment, (GLint)shader, textureSizeLoc, Lx, Ly);
    OpenGLCachedUniforms::SetUniformFloat(environment, (GLint)shader, noiseAmplitudeLoc, noiseAmplitude);
    OpenGLCachedUniforms::SetUniformFloat(environment, (GLint)shader, invNoiseDistanceLoc, 1.0f / (noiseDistance * environment->GetZoomLevel()));
    OpenGLCachedUniforms::SetUniformFloat(environment, (GLint)shader, invDampingDistanceLoc, 1.0f / (displacementDampingDistance * environment->GetZoomLevel()));
    OpenGLCachedUniforms::SetUniformFloat(environment, (GLint)shader, lodBiasLoc, lodBias);
    OpenGLCachedUniforms::SetUniformFloat(environment, (GLint)shader, timeLoc, (float)time);

    return Utils::PrintGLErrors(__FILE__ , __LINE__);
}


bool FFTWaterModelOpenGL::PrepPatchShader(double time)
{
    if (!FFTWaterModel::PrepShaders(time))
        return false;

	if(!environment)
		return false;
	osg::GLExtensions* et = environment->GetExtension();
	if(!et)
		return false;

    Utils::ClearGLErrors();

    //glUseProgram((GLint)patchShader);

    if (!fft->TransformsEndToEnd()) {
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, displacementTexture);
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, N, M, GL_RGB, GL_FLOAT, displacementMap);

        glActiveTexture(GL_TEXTURE3);
        glBindTexture(GL_TEXTURE_2D, slopeFoamTexture);

        // Mipmap auto-generation not working reliably on all systems, sadly.
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, N, M, GL_RGBA, GL_FLOAT, slopeFoamMap);
        if (hasFBO || environment->GetRenderer() >= OPENGL_3_2) {
            et->glGenerateMipmap(GL_TEXTURE_2D);
        }

    } else {
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, displacementTexture);

        glActiveTexture(GL_TEXTURE3);
        glBindTexture(GL_TEXTURE_2D, slopeFoamTexture);

        if (hasFBO || environment->GetRenderer() >= OPENGL_3_2) {
            et->glGenerateMipmap(GL_TEXTURE_2D);
        }
    }

    OpenGLCachedUniforms::SetUniformInt(environment, (GLint)patchShader, displacementLocPatch, 2);
    OpenGLCachedUniforms::SetUniformInt(environment, (GLint)patchShader, slopeFoamLocPatch, 3);
    OpenGLCachedUniforms::SetUniformVec2(environment, (GLint)patchShader, textureSizeLocPatch, Lx, Ly);
    OpenGLCachedUniforms::SetUniformFloat(environment, (GLint)patchShader, noiseAmplitudeLocPatch, noiseAmplitude);
    OpenGLCachedUniforms::SetUniformFloat(environment, (GLint)patchShader, invNoiseDistanceLocPatch, 1.0f / (noiseDistance * environment->GetZoomLevel()));
    OpenGLCachedUniforms::SetUniformFloat(environment, (GLint)patchShader, invDampingDistanceLocPatch, 1.0f / (displacementDampingDistance * environment->GetZoomLevel()));
    OpenGLCachedUniforms::SetUniformFloat(environment, (GLint)patchShader, lodBiasLocPatch, lodBias);
    OpenGLCachedUniforms::SetUniformFloat(environment, (GLint)patchShader, timeLocPatch, (float)time);

    return Utils::PrintGLErrors(__FILE__ , __LINE__);
}
