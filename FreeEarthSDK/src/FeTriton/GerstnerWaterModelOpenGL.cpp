// Copyright (c) 2011-2013 Sundog Software LLC. All rights reserved worldwide.

#include "GerstnerWaterModelOpenGL.h"
#include <stdio.h>

using namespace Triton;

bool GerstnerWaterModelOpenGL::Initialize(ShaderHandle pShader, ShaderHandle patchShader, bool enableHeightReads, bool noGPU)
{
    shader = pShader;

    if (!shader) return false;

    if (!GerstnerWaterModel::Initialize(shader, patchShader, enableHeightReads, noGPU)) return false;

    Utils::ClearGLErrors();

    waveLocs = TRITON_NEW WaveLocsOpenGL[numWaves];
    if (!waveLocs) return false;

    char buf[256];
    for (int i = 0; i < numWaves; i++) {
#if (_MSC_VER > 1310)
        sprintf_s(buf, 256, "waves[%d].frequency", i);
        waveLocs[i].frequencyLoc = glGetUniformLocation((GLint)shader, buf);
        sprintf_s(buf, 256, "waves[%d].phaseSpeed", i);
        waveLocs[i].phaseSpeedLoc = glGetUniformLocation((GLint)shader, buf);
        sprintf_s(buf, 256, "waves[%d].amplitude", i);
        waveLocs[i].amplitudeLoc = glGetUniformLocation((GLint)shader, buf);
        sprintf_s(buf, 256, "waves[%d].direction", i);
        waveLocs[i].directionLoc = glGetUniformLocation((GLint)shader, buf);
        sprintf_s(buf, 256, "waves[%d].steepness", i);
        waveLocs[i].steepnessLoc = glGetUniformLocation((GLint)shader, buf);
#else
        sprintf(buf, "waves[%d].frequency", i);
        waveLocs[i].frequencyLoc = glGetUniformLocation((GLint)shader, buf);
        sprintf(buf, "waves[%d].phaseSpeed", i);
        waveLocs[i].phaseSpeedLoc = glGetUniformLocation((GLint)shader, buf);
        sprintf(buf, "waves[%d].amplitude", i);
        waveLocs[i].amplitudeLoc = glGetUniformLocation((GLint)shader, buf);
        sprintf(buf, "waves[%d].direction", i);
        waveLocs[i].directionLoc = glGetUniformLocation((GLint)shader, buf);
        sprintf(buf, "waves[%d].steepness", i);
        waveLocs[i].steepnessLoc = glGetUniformLocation((GLint)shader, buf);
#endif
        if (waveLocs[i].frequencyLoc == -1 || waveLocs[i].phaseSpeedLoc == -1 ||
                waveLocs[i].amplitudeLoc == -1 || waveLocs[i].directionLoc == -1 ||
                waveLocs[i].steepnessLoc == -1) {
            Utils::DebugMsg("One or more wave parameters not found in the OpenGL "
                            "Gerstner shader. Did you change the number of waves in the config file "
                            "but not update the MAX_WAVES definition in the .glsl files?");
            return false;
        }
    }

    numWavesLoc = glGetUniformLocation((GLint)shader, "numWaves");
    if (numWavesLoc == -1) {
        Utils::DebugMsg("numWavesLoc parameter not found in the Gerstner GL shader.");
        return false;
    }

    timeLoc = glGetUniformLocation((GLint)shader, "time");
    if (timeLoc == -1) {
        Utils::DebugMsg("time parameter not found in the Gerstner GL shader.");
        return false;
    }

    return Utils::PrintGLErrors(__FILE__, __LINE__);
}

GerstnerWaterModelOpenGL::~GerstnerWaterModelOpenGL()
{
    if (waveLocs) {
        TRITON_DELETE[] waveLocs;
    }
}

bool GerstnerWaterModelOpenGL::PrepShaders(double time)
{
    if (!GerstnerWaterModel::PrepShaders(time)) return false;

    Utils::ClearGLErrors();

    glUseProgram((GLint)shader);
    glUniform1f(timeLoc, (float)time);
    glUniform1i(numWavesLoc, numWaves);

    for (int i = 0; i < numWaves; i++) {
        glUniform1f(waveLocs[i].frequencyLoc, waves[i].frequency);
        glUniform1f(waveLocs[i].amplitudeLoc, waves[i].amplitude);
        glUniform1f(waveLocs[i].steepnessLoc, waves[i].steepness);
        glUniform1f(waveLocs[i].phaseSpeedLoc, waves[i].phaseSpeed);
        glUniform2f(waveLocs[i].directionLoc, waves[i].dirX, waves[i].dirY);
    }

    return Utils::PrintGLErrors(__FILE__, __LINE__);
}
