// Copyright (c) 2004-2014  Sundog Software, LLC. All rights reserved worldwide.

/** \file Billboard.h
   \brief Classes that support the efficient drawing of quads oriented toward the viewer.
 */

#ifndef BILLBOARD_H
#define BILLBOARD_H

#if defined(WIN32) || defined(WIN64)
#pragma warning(disable: 4786)
#endif

#include <list>
#include <map>
#include <FeSilverliningLib/VertexBuffer.h>
#include <FeSilverliningLib/Renderer.h>
#include <FeSilverliningLib/MemAlloc.h>

namespace SilverLining
{
class Atmosphere;

/** A collection of billboard quads drawn as a single triangle strip. */
class QuadBatch : public MemObject
{
public:
/** Destructor; cleans up the index buffer associated with this
   object. */
    virtual ~QuadBatch();

/** Constructor; takes a vector of indices into the vertex buffer object
   provided, and vertex colors associated with each index.

   \param indices A vector of indices into the vertex buffer to be drawn
       as a triangle strip.
   \param pvb A pointer to a VertexBuffer object containing vertex data
               for the triangle strip to be drawn. This QuadBatch object
               will not delete this pointer.
    \param colors A vector of colors that matches each index passed in via
                  the indices parameter.
 */
    QuadBatch(const SL_VECTOR(Index)& indices, VertexBuffer *pvb,
              const SL_VECTOR(Color)& colors);

/** Draws the QuadBatch. Does not do any setup of shaders or any other
    rendering state. */
    void Draw();

private:

    QuadBatch() : vb(0), ib(0) {
    }

    VertexBuffer *vb;
    IndexBuffer *ib;
};

/** A collection of QuadBatches (a vector, to be precise) that are drawn
    together using the same rendering state. This object is returned by
    Billboard::EndBatchDraw() to enable you to quickly re-draw a batch
    of billboards that share the same texture map.*/
class QuadBatches : public MemObject
{
public:
/** Default constructor; doesn't really do much of anything. */
    QuadBatches() : dirty(false) {
    }

/** The destructor deletes all QuadBatch objects that have been added to
    this object. */
    virtual ~QuadBatches();

/** Add a QuadBatch object to this collection. Once added, the
    QuadBatches object will be responsible for its destruction. */
    void Add(QuadBatch *qb) {
        batches.push_back(qb);
    }

/** Draw all attached QuadBatch objects using the texture
    specified. Sets up and tears down any required shaders and
    state. "Fade" controls the transparency of the quads and ranges
   from 0 (transparent) to 1 (fully visible). */
    void Draw(TextureHandle tex, double fade);

/** Marks this object as "dirty". Internally, all this does is set
    a boolean flag. Externally, you may query the IsInavlid() method
    to see if you set this flag earlier. */
    void Invalidate() {
        dirty = true;
    }

/** Queries the "dirty" flag, which will only be set if the Invalidate()
    method has been called on this object. */
    bool IsInvalid() const {
        return dirty;
    }

private:
    SL_VECTOR(QuadBatch *) batches;
    bool dirty;
};

/** A class embodying everything needed to draw a single quad; an index, a
    pointer into a vertex buffer, and a color. It also provides a less-than
    operator to enable "sorting" of BillboardQuad objects, just based on
    the address of the vertex buffer passed in. Sorting by vertex buffer
    is useful for minimizing state changes when drawing. */
class BillboardQuad : public MemObject
{
public:
/** Constructor; just assigns data members to the values passed in.
    \param pIndex An index into the vertex buffer passed in, which marks
    the beginning of the four vertices that make up this quad.
    \param pVb A pointer to a VertexBuffer that pIndex refers to. This
    object will not delete this pointer.
    \param pColor The color to modulate this billboard by.
 */
    BillboardQuad(int pIndex, VertexBuffer *pVb, Color pColor) : index(pIndex), color(pColor),
        vb(pVb) {
    }

// Data members are public for convenience. Normally this would just be a struct,
// but we needed the less-than operator for sorting, thereby requiring a class.
    int index;
    Color color;
    VertexBuffer *vb;

    bool operator < (const BillboardQuad& bq) const {
        return vb < bq.vb;
    }
};

/** A large pool of vertices and indices (to minimize state changes) from which
    vertices for individual quads are vended. */
class SharedVertexBuffer : public MemObject
{
public:
/** Default constructor. */
    SharedVertexBuffer();

/** Copy constructor; this actually only does a shallow copy. */
    SharedVertexBuffer(const SharedVertexBuffer& svb);

/** The destructor deletes this object's vertex and index buffer objects. */
    ~SharedVertexBuffer();

/** Allocate a given number of vertices from this SharedVertexBuffer.

    \param nVerts The number of vertices you desire.
    \param vertIdx A reference to an int. If successful, this will be the
    index into the object's vertex buffer from which your nVerts vertices
    begin.
    \return true if nVerts vertices were successfully allocated from this
    SharedVertexBuffer; false if the SharedVertexBuffer ran out of vertices.
    If false is returned, then you'll need to create a new SharedVertexBuffer
    to create more vertices.
 */
    bool Reserve(int nVerts, unsigned int& vertIdx);

/** Destroys all attached vertex and index buffers. They will be recreated
    if necessary by future Reserve() calls.*/
    void Clear();

/** Returns a pointer to this object's vertex buffer. Do not delete this pointer. */
    VertexBuffer *GetVertexBuffer() const {
        return vb;
    }

/** Returns a pointer to this object's index buffer. Do not delete this pointer. */
    IndexBuffer *GetIndexBuffer() const {
        return quadIndices;
    }

/** When creating an object that consumes vertices from this SharedVertexBuffer
   object, call AddReference() to increment its reference count. */
    void AddReference();

/** When destroying an object that consumes vertices from this SharedVertexBuffer
   object, call RemoveReference() to decrement its reference count. When the
   reference count reaches zero, the SharedVertexBuffer will self-destruct. */
    void RemoveReference();

private:
    VertexBuffer *vb;
    IndexBuffer *quadIndices;
    unsigned int nextAvail;
    unsigned int numReferences;
};

/** The Billboard object represents a quad that is always oriented toward the viewer.
    Since it may be textured with non-symmetric textures, orientation relative to
    the world is important. It will honor the "up" vector and rotate about it when
    possible, however it will orient toward the viewer regardless of the "up"
    direction when necessary for large view angles.

    The Billboard uses every trick it can get away with to maximize drawing speed,
    such as vertex shaders and vertex buffer objects.
 */
class Billboard : public MemObject
{
    friend class QuadBatches;
    friend class SharedVertexBuffer;

public:
/** Constructor.
 */
    Billboard();

/** Initialize the resources for rendering this Billboard; should be called right after instantiating.

    \param randomRotate If true, the billboard will be rolled by a random amount
    relative to the viewpoint. This is used to randomize the orientation of
    cloud puffs that make up a cloud, in order to make them look more natural.
    \param diameter The size of the billboard in world uits
    \param spinRate The maximum rate at which the billboard spins, in radians per second.
 */
    void Initialize(bool randomRotate, const float diameter = 0, const float spinRate = 0);

/** Destructor; also destroys any shared resources that may no longer be in
    use required for the rendering of this billboard. */
    virtual ~Billboard();

/** Sets the diameter of this billboard in terms of screen pixels.  In this
   mode, the billboard will always appear as the same screen size. */
    void SetScreenSize(double diameter);

/** Sets the diameter of this billboard in terms of world units. In this
   mode, the billboard will be subject to perspective correction. */
    void SetWorldSize(double diameter);

/** Sets the diameter of this billboard in terms of view degrees. In this
   mode, the billboard will always appear as the same screen size. Useful
   for astronomical objects, such as the sun and the moon, whose size is known
   as 0.5 degrees across. */
    void SetAngularSize(double degrees);

/** Returns the radius or 1/2 the diameter of this billboard. */
    float GetRadius() const { return radius; }

/** Draws this billboard individually, with the given texture. It's actually
    quite inefficient to draw billboards individually; normally you'd want to
    draw a bunch of them together using StartBatchDraw(), DrawInBatch(), and
    EndBatchDraw(). Otherwise a whole bunch of state needs to be set up
    and torn down as part of this method.

    \param tex The texture to draw this billboard with.
    \param pass Set to 0 for the lighting pass, 1 for the drawing pass.
   \param fade The "alpha" value of the billboard. 0 is transparent, 1 is fully visible.
    \param objectSpace whether the billboard vertices are in object space or world space.
    If in object space, the vertices will be transformed by the local basis.

    \return true if the operation completed successfully.
 */
    virtual bool Draw(TextureHandle tex, int pass, double fade, bool objectSpace);

/** Draws this billboard as part of a batch of billboards, thereby bypassing
    all the state setup required. Must be wrapped by calls to StartBatchDraw()
    and EndBatchDraw().

    \param drawImmediately Normally, all drawing is deferred until the end of
    the frame, so we can sort all of the translucent objects in the scene.
    If instead, you need to draw this billboard *right now*, you can set this
    to true.

    \return true if the operation completed successfully.
 */
    bool DrawInBatch(bool drawImmediately);

/** Tests if this billboard is visible within the given Frustum.

    \param f The Frustum to test visibility against.

    \return true if the billboard is not visible in this frustum, and should
    be culled.
 */
    virtual bool Cull(const Frustum& f);

/** Sets the position of this billboard in world coordinates. */
    void SetWorldPosition(const Vector3f& pos);

/** Retrieves the position of this billboard in world coordinates. */
    const Vector3f& GetWorldPosition() const {
        return centerPos;
    }

/** Sets the color to modulate this billboard by. */
    void SetColor(const Color& col);

/** Gets the color to modulate this billboard by. */
    Color GetColor() const;

/** Set the rate at which to fade this billboard per second. */
    void SetFadeRate(float fadeRate);

/** This static method is called by the Billboard() constructor to allocate vertices
    from a collection of SharedVertexBuffers. This way, vertices are allocated from
    optimally-sized vertex buffer pools. This method manages the on-demand creation
    of new SharedVertexBuffer objects as they fill up, and also increments the
    reference count on the SharedVertexBuffer used.

    \param nVerts The number of vertices desired.
    \param vbIdx A reference to an unsigned int, that returns the index into the
    vertex buffer from which these vertices begin.
    \param vb A pointer to a vertex buffer pointer, which will return the VertexBuffer
    object that the requested vertices are within.
    \param ib A pointer to an index buffer pointer, which will return the IndexBuffer
    object that the requested vertices' indices are within.
    \param sib A pointer to a SharedVertexBuffer pointer, which will return the
    SharedVertexBuffer object that these vertices were allocated from.
 */
    static void GetVerts(int nVerts, unsigned int& vbIdx, VertexBuffer **vb, IndexBuffer **ib,
                         SharedVertexBuffer **sib);

/** Begin drawing a series of Billboards that all share the same texture. A call to
    StartBatchDraw() should precede a series of calls to DrawInBatch() calls, which
    then must be followed by a call to EndBatchDraw(). Drawing billboards in this
    manner is much more efficient than calling Draw() on individual Billboard objects.

    As a further optimization, batches of billboards will create a QuadBatches object
    returned with the call to EndBatchDraw(). Calling QuadBatches::Draw() is even
    faster than drawing batches of individual billboards.

    \param tex The texture to draw the billboards with.
   \param fade The "alpha" value for this billboard. 0 is transparent, 1 is fully visible.
   \param objectSpace whether the vertices of the billboard are in object or world space.

    \return true if the operation completed successfully.
 */
    static bool StartBatchDraw(TextureHandle tex, double fade, bool objectSpace);

/** Signals that you're done drawing a bunch of billboards using DrawInBatch().
    Cleans up any shaders and other state that was set up to draw the billboards.
    Calls to DrawInBatch() must be surrounded by calls to StartBatchDraw()
    and EndBatchDraw().

    \return A QuadBatches object that can be stored to draw this batch again,
    even more quickly. If this group of billboards doesn't change, then you may
    re-use the QuadBatches object to keep drawing it at optimal speed. The caller
    is responsible for the deletion of this object. You may use the
    QuadBatches::Invalidate() method to help you manage when this object is no longer
    valid and must be deleted, and another cycle of StartBatchDraw() /
    DrawInBatch() / EndBatchDraw() is required to generate a fresh QuadBatches
    object.
 */
    static QuadBatches *EndBatchDraw();

/** The modelview matrix is modified when billboards are drawn. The new matrix is
   automatically set so the billboard will face the viewer based on the modelview
   matrix for the scene as it was set upon entering Atmosphere::DrawSky().

   If you wish to set your own modelview matrix to be used while drawing billboards,
   you may override the "default" billboard matrix by calling this method. This matrix
   will be used for all billboards until RestoreBillboardMatrix() is called.

   \param billboardMatrix The modelview matrix to use for future billboard drawing.
 */
    static void OverrideBillboardMatrix(const Matrix4& billboardMatrix);

/** If OverrideBillboardMatrix() was previously called, call RestoreBillboardMatrix()
   to resume using the default billboard matrix computed from the scene's modelview
   matrix. */
    static void RestoreBillboardMatrix();

/** Sets the current modelview matrix to the billboard matrix (either the default
   billboard matrix computed based on the the scene's modelview matrix, or the
   manual override if we're between calls to OverrideBillboardMatrix() and
   RestoreBillboardMatrix()) */
    static void SetBillboardMatrix();

/** Returns a handle to the vertex shader used to efficiently draw billboards. */
    static ShaderHandle GetShaderHandle() {
        return billboardShaderMap[Atmosphere::GetCurrentAtmosphere()];
    }

/** Sets up the vertex shaders used to efficiently draw billboards. Must be called before using
   the billboard class, after the renderer has been initialized. */
    static void SetupShaders();

/** Retrieves the billboard shader used by the current atmosphere. */
    static ShaderHandle GetShader();

/** Reloads the billboard shader used by the current atmosphere. */
    static void ReloadShaders();

/** Billboards are drawn using vertex shaders whenever possible, and so are responsible
   for implementing their own fog effects. Let the billboards know what the fog color is
   by calling SetFogColor() at least once per scene, prior to any billboard drawing. */
    static void SetFogColor(const Color& fogColor) {
        fog = fogColor;
    }

/** Retrives the current fog color being used to render billboards with. */
    static Color GetFogColor() {
        return fog;
    }

/** Billboards are drawn using vertex shaders whenever possible, and so are responsible
   for implementing their own fog effects. Let the billboards know how thick the fog is by
   calling SetFogDensity() at least once per frame, prior to any billboard drawing.

   \param density The density of the fog, to be used in the exponential fog equation.
 */
    static void SetFogDensity(double density) {
        fogDensity = density;
    }

/** Retrieves the current fog density being used to render billboards with.*/
    static double GetFogDensity() {
        return fogDensity;
    }

/** Whether billboard spin is always disabled, for example when rendering shadows. */
    static void SetSpinEnabled(bool enabled) {
        spinEnabled = enabled;
    }

/** Returns current spin enabled flag. */
    static bool GetSpinEnabled() {
        return spinEnabled;
    }

/** Sets the distance used for soft particles at the base of clouds. */
    static void SetSoftness(float pSoftness) {
       softness = pSoftness;
    }

/** Returns the current softness distance */
    static float GetSoftness() {
       return softness;
    }

/** Control whether all billboards are forced to a constant color (used for rendering shadow maps.) */
    static void ForceConstantColor(bool force, const Color& color) {
        forceConstantColor = force;
        forceColor = color;
    }

/** Sets the texture atlas index to use, assuming a 4x4 atlas texture. Index 0 means no atlas is in use
    and the entire texture should be used. Index 1 is the top-left-most texture, and 16 is the bottom-right. */
    void SetAtlasIndex(int idx) {
        atlasIndex = (unsigned char)idx;
    }

/** Retrieves the atlas texture index set via SetAtlasIndex(), or 0 if no atlas is in use. */
    int GetAtlasIndex() const {
        return atlasIndex;
    }

    static bool GetForceConstantColor(Color& color) {color = forceColor; return forceConstantColor;}

    static void RemoveAtmosphere(Atmosphere *atm);

    /// Cleans up static objects at shutdown
    static void Destroy();

    static void SetFadeStartTime(unsigned long ms) {fadeStartTime = ms;}

    static unsigned long GetFadeStartTime() {return fadeStartTime;}

    static void SetVerticalGradient(double gradient) {verticalGradient = gradient;}
    static double GetVerticalGradient() {return verticalGradient;}

protected:

    void UpdateQuadVertices(bool changeSize, double newSize, bool rotate = true, float fadeRate = 0.0f);

    static void SetShaderColor(const Color& c);
    static void UnsetShaderColor();
    static void SetupBillboardShader(double fade, bool objectSpace);
    static void TeardownBillboardShader();

    static SL_MAP( Atmosphere *, SL_LIST(SharedVertexBuffer *) ) vbStackMap;
    static SL_VECTOR(BillboardQuad) batchQuads;

    static QuadBatches *DrawQuadBatches();

    static QuadBatch *DrawQuadBatch(const SL_VECTOR(Index)& indices, VertexBuffer *vb,
                                    const SL_VECTOR(Color)& colors);

// Note - keep data members to a minimum, there are a lot of these objects.

    SharedVertexBuffer *sharedVb;
    unsigned int vbIdx;
    Vector3f centerPos;
    Color billboardColor;
    float rotateAngle, rotateSpeed, radius, fadeRate;
    unsigned char atlasIndex;

    static Matrix4 overrideBillboard;
    static bool overridingMatrix, forceConstantColor;
    static SL_MAP(Atmosphere *, ShaderHandle) billboardShaderMap;
    static Color fog, forceColor;
    static double fogDensity;
    static unsigned long fadeStartTime;
    static bool spinEnabled;
    static double verticalGradient;
    static float softness;
    static float fogColorBoost;
};
}

#endif
