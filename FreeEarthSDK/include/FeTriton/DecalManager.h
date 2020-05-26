// Copyright (c) 2013-2015 Sundog Software, LLC. All rights reserved worldwide.

#ifndef TRITON_DECAL_MANAGER_H
#define TRITON_DECAL_MANAGER_H

/** \file DecalManager.h
\brief Class to manage a collection of Decals.
*/

#include <FeTriton/Decal.h>
#include <list>
#include <vector>

namespace Triton
{
class Environment;

/** Base class to manage the rendering of floating Decal textures. */
class DecalManager : public MemObject
{
public:
    /** Constructor.
        \param env The Environment associated with the decals.
    */
    DecalManager(const Environment *env);

    /** Virtual destructor. */
    virtual ~DecalManager();

    /** Creates a Decal object. Use AddDecal to allow it to start rendering.
        \param texture The texture used to draw this decal.
                        Under OpenGL, this must be a GLuint indicating the ID of the GL_TEXTURE_2D returned from
                        glGenTextures. Under DirectX9, this must be a LPDIRECT3DTEXTURE9. Under DirectX11, this must be a
                        ID3D11ShaderResourceView pointer with an underlying ViewDimension of D3D11_SRV_DIMENSION_TEXTURE2D.
        \param size The width and depth of this decal, in world units.
        \return The Decal object created.
    */
    virtual Decal *CreateDecal(TextureHandle texture, float size) = 0;

    /** Adds a Decal to the DecalManager and begins rendering it each frame. */
    void AddDecal(Decal * decal);

    /** Removes the decal from the DecalManager, but does not delete it. */
    void RemoveDecal(Decal *decal);

    /** Draws all decals over the ocean (must be called after the ocean has drawn.)
        \param depthTexture The GLuint, LPDIRECT3DTEXTURE9, or ID3D11ShaderResourceView* containing a single-channel
                            floating point texture with z/w depth information for the current scene in screen space.
    */
    virtual void DrawDecals(TextureHandle depthTexture) = 0;

    /** Returns true if at least one active decal is being managed. */
    bool HasDecals() const;

    /** Load all shader programs required to render decals. */
    virtual bool LoadShaders() = 0;

    /** Dispose of all shader programs required to render decals. */
    virtual void DeleteShaders() = 0;

    /** Reload all shader programs required to render decals. */
    virtual bool ReloadShaders(const TRITON_VECTOR(unsigned int)& shaders) {
        return true;
    }

    /** Retrieve the underlying shader object for the decals. */
    virtual ShaderHandle GetShader() const {
        return 0;
    }

    /** Call to handle DirectX9 device lost events. */
    virtual void D3D9DeviceLost() {}

    /** Call to handle DirectX9 device reset events. */
    virtual void D3D9DeviceReset() {}

protected:
    double GetNearClipDistance(const Matrix4& m);
    void ComputeBasis();

    TRITON_LIST(Decal *) decals;
    const Environment *environment;
    float depthOffset;
    bool getDepthRange;
    Matrix4 basis, invBasis;
};
}

#endif

