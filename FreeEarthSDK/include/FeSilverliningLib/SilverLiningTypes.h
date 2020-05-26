// Copyright (c) 2010-2013 Sundog Software, LLC. All rights reserved worldwide.

#ifndef SILVERLINING_TYPES_H
#define SILVERLINING_TYPES_H

/** Types of shaders, either a vertex or a fragment shader. */
enum ShaderTypes
{
    VERTEX_PROGRAM,
    FRAGMENT_PROGRAM
};

/** Blend modes for blending equation coefficients. */
enum BlendFactors
{
    ZERO,
    ONE,
    SRCCOLOR,
    INVSRCCOLOR,
    SRCALPHA,
    INVSRCALPHA,
    DSTCOLOR,
    INVDSTCOLOR,
    DSTALPHA,
    INVDSTALPHA,
    SRCALPHASAT,
    NUM_BLEND_FACTORS
};

/** Cubemap faces */
enum CubeFace
{
    POSX, 
    NEGX, 
    POSY, 
    NEGY, 
    POSZ, 
    NEGZ
};

/** Sky models */
enum SkyModel
{
    PREETHAM,
    HOSEK_WILKIE
};

enum LightningDischargeMode
{
  AUTO_DISCHARGE,
  FORCE_ON_OFF,
  FIRE_AND_FORGET,
  MAX_DISCHARGE_MODE,
};
namespace SilverLining
{
typedef unsigned int Index;
typedef void * IndexBufferHandle;
typedef void * VertexBufferHandle;
typedef void * ShaderHandle;
typedef void * TextureHandle;
typedef void * RenderTextureHandle;
typedef void * RenderTargetHandle;
typedef void * QueryHandle;
typedef void * CameraHandle;
}

#endif
