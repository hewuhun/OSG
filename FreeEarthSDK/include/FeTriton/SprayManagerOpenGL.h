// Copyright (c) 2011-2013 Sundog Software LLC. All rights reserved worldwide.

/**
    \file SprayManagerOpenGL.h
    \brief OpenGL implementation of SprayManager.
*/

#ifndef TRITON_SPRAYMANAGER_OPENGL_H
#define TRITON_SPRAYMANAGER_OPENGL_H

#include <FeTriton/TritonOpenGL.h>
#include <FeTriton/SprayManager.h>
#include <vector>
#ifndef _WIN32
#include <stdint.h>
#endif

namespace Triton
{
/** An OpenGL implementation of SprayManager. */
class SprayManagerOpenGL : public SprayManager
{
public:
    SprayManagerOpenGL(const Environment *env, const TRITON_VECTOR(unsigned int)& userShaders);

    virtual ~SprayManagerOpenGL();

    virtual bool Initialize(const ProjectedGrid *grid);

    virtual ShaderHandle GetShader() const {
        return (ShaderHandle)programCast;
    }

    virtual bool ReloadShaders(const TRITON_VECTOR(unsigned int)& shaders);

private:
    bool SetupShaders();
    void DeleteShaders();

    GLhandleARB LoadShader(const char *shaderName, bool vertexShader, bool userShader, const Environment *env);

    GLhandleARB vertShader, fragShader, userShader, userVertShader, program;

    GLuint sprayTexture, vertCastShader, fragCastShader, userCastShader, userVertCastShader, programCast;

    TRITON_VECTOR(unsigned int) userShaders;
};
}

#endif