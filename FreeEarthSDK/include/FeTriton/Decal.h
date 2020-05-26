// Copyright (c) 2013-2014 Sundog Software, LLC. All rights reserved worldwide.

#ifndef TRITON_DECAL_H
#define TRITON_DECAL_H

/** \file Decal.h
\brief A decal texture applied to the dynamic ocean surface.
*/
#include <FeTriton/TritonCommon.h>
#include <FeTriton/Vector3.h>
#include <FeTriton/Matrix4.h>
#include <FeTriton/OrientedBoundingBox.h>

namespace Triton
{
class Environment;

/** Base class for a floating decal texture. */
class Decal : public MemObject
{
public:
    /** Constructor.
        \param env The Environment associated with this decal.
        \param pTexture The texture to be applied with this decal.
                        Under OpenGL, this must be a GLuint indicating the ID of the GL_TEXTURE_2D returned from
                        glGenTextures. Under DirectX9, this must be a LPDIRECT3DTEXTURE9. Under DirectX11, this must be a
                        ID3D11ShaderResourceView pointer with an underlying ViewDimension of D3D11_SRV_DIMENSION_TEXTURE2D.
        \param pSize The width and depth of this decal, in world units.
    */
    Decal(const Environment *env, TextureHandle pTexture, float pSize);

    /** Virtual destructor. */
    virtual ~Decal() {}

    /** Creates any resources required to render this decal's geometry. */
    virtual void Setup() = 0;

    /** Draws the decal's geometry, using the currently bound depth map and decal textures. Does not actually bind the buffers or set shaders.
       param inside Whether the viewpoint is inside the decal's bounding volume.
    */
    virtual void Draw(bool inside) = 0;

    /** Binds any vertex and index buffers required to draw the decal's geometry. */
    virtual void BindArrays() {}

    /** If necessary, unbinds the vertex and index buffers for this decal. */
    virtual void UnbindArrays() {}

    /** Returns the oriented bounding bound enclosing this decal. */
    const OrientedBoundingBox& GetOrientedBoundingBox() const {
        return obb;
    }

    /** Sets the center position of this decal (at sea level.) */
    void SetPosition(const Vector3& pos);

    /** Gets the center position of this decal, in world units. */
    const Vector3& GetPosition() const {
        return position;
    }

    /** Sets the rotation of the decal about the up axis (in radians) */
    void SetRotation(double radians) {
        rotation = radians;
    }

    /** Gets the rotation of this decal, in radians. */
    double GetRotation() const {
        return rotation;
    }

    /** Returns the matrix used to define an orthographic view * projection looking down at the decal. */
    const Matrix4& GetDecalMatrix() const {
        return decalMatrix;
    }

    /** Returns the decal texture provided in the constructor. */
    TextureHandle GetDecalTexture() const {
        return texture;
    }

    /** Sets the light color to multiply the texture by. */
    void SetLightColor(const Vector3& color) {
        lightColor = color;
    }

    /** Returns the light color used to multiply the texture by. */
    const Vector3& GetLightColor() const {
        return lightColor;
    }

    /** Sets an alpha blend value for this decal. */
    void SetAlpha(float a) {
        alpha = a;
    }

    /** Retrieves the alpha blend value for this decal. */
    float GetAlpha() const {
        return alpha;
    }

    /** Sets whether this decal is drawn additively (with blend mode SRC_ALPHA, ONE) or blended
        with SRC_ALPHA, INV_SRC_ALPHA instead.
        \param a True for additive blending, false for normal blending.
    */
    void SetAdditive(bool a) {
        additive = a;
    }

    /** Returns if this decal is to be drawn with additive blending or not. */
    bool GetAdditive() const {
        return additive;
    }

    /** Sets a scale factor to be applied to the decal at runtime.
        \param sx A scale factor for the width of the decal (ie, 2.0 = 2X larger than the given size.)
        \param sz A scale factor for the depth of the decal.
    */
    void SetScale(float sx, float sz) {
        scaleX = sx;
        scaleZ = sz;
    }

    /** Retrieves the scaling factors applied to the decal at runtime, set via SetScale(). */
    void GetScale(float& sx, float& sz) const {
        sx = scaleX;
        sz = scaleZ;
    }

    /** Sets a texcoord offset used when looking up the decal color (not alpha) components */
    void SetUVOffset(float u, float v) {
        uOffset = u;
        vOffset = v;
    }

    /** Retrieves texcoord offset for color lookups */
    void GetUVOffset(float& u, float& v) const {
        u = uOffset;
        v = vOffset;
    }

    /** Recomputes the view / projection matrix for an orthographic view looking down at the
        decal. */
    void ComputeDecalMatrix(const Vector3& camPos, const Matrix3& rotate);

    /** Returns true if this decal is not currently culled out or otherwise disabled. */
    bool GetActive() const {
        return !culled && alpha > 0.0f;
    }

    /** Sets if this decal is currently culled or not. */
    void SetCullState(bool cullState) {
        culled = cullState;
    }

protected:

    void ComputeOBB();

    TextureHandle texture;
    Vector3 position;
    double size;
    Matrix4 decalMatrix;
    const Environment *environment;
    double boxHeight;
    float volumeScale;
    float alpha;
    Vector3 lightColor;
    float scaleX, scaleZ;
    bool culled;
    double rotation;
    float uOffset, vOffset;

    OrientedBoundingBox obb;

    bool additive;
};
}

#endif
