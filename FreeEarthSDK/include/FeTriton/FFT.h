// Copyright (c) 2011-2013 Sundog Software LLC. All rights reserved worldwide.

#ifndef TRITON_FFT_H
#define TRITON_FFT_H

/** \file FFT.h
   \brief An interface for classes that execute fast Fourier transforms.
 */

#include <FeTriton/ComplexNumber.h>
#include <vector>
#include <FeTriton/Vector3.h>

//#if (defined(WIN32) || defined(WIN64))
//#if !defined(DIRECTX9_FOUND)
//#define DIRECTX9_FOUND 1
//#endif
//#else
//#define DIRECTX9_FOUND 0
//#endif

#if DIRECTX9_FOUND==1
#include <d3d9.h>
#else
// forward declare to get round build issues
struct DX11Texture;
struct IDirect3DTexture9;
struct ID3D11Device;
#endif


namespace Triton
{
class Environment;

/** The FFT base class provides an interface for performing inverse FFT on wave data generated in the frequency domain.
    Derived classes implement platform-specific means of performing FFT's in the most optimal manner. This class will
    also perform "end to end" transformation from frequency data all the way to generating texture maps for the
    ocean rendering on implementations that may do so entirely on the GPU. */
class FFT : public MemObject
{
public:
    /** Instantiates the most optimal FFT implementation for the system we're on, and for the Environment specified. */
    static FFT *Create(const Environment *env, bool noGPU);

    /** Virtual destructor. */
    virtual ~FFT() {}

    /** Sets up any resources and precomputation required for the fast fourier transforms you expect to
        execute.

        \param Nx   The X dimension of the 2D grids you'll be transforming from the frequency to spatial domain
        \param Ny   The Y dimension of the 2D grids you'll be transforming
        \param batchSize    The number of grids you'll be transforming at once. For us, it's typically 3 -
                            one for each dimension of displacement. In CPU FFT implementations, you might
                            tack on two more for computing exact normal vectors (one for X and one for Y.)
                            If you're using an FFT implementation that supports "End to End" transformation
                            of the ocean (see FFT::TransformEndToEnd() ) then batchSize MUST be 3.
        \param env  A pointer to our Environment object. We need this to know what sort of renderer we
                    need to integrate with on the "end to end" transforms, and to get at the resource loader
                    so we know where to load our FFT DLL's from.
        \param enableHeightReads Whether the application will need to call Ocean::GetHeight() to retrieve
                    height information back from the ocean surface. If false, we can optimize for not
                    needing to read this information back from the GPU and calls to Ocean::GetHeight()
                    will all return zero.
        \return     Whether the FFT successfully initialized or not. If false, using this FFT object will have
                    no effect, and you should try using a different implementation of this interface.
    */
    virtual bool Setup(unsigned int Nx, unsigned int Ny, unsigned int batchSize, const Environment *env,
                       bool enableHeightReads) = 0;

    /** Transforms an array of 2D complex grids to an array of 2D real grids using an inverse fast Fourier
        transform. This is only implemented on FFT's where FFT::TransformsEndToEnd() returns false, otherwise
        you should be calling FFT::TransformEndToEnd() instead.

        \param  inputArray  An array of 2D arrays of ComplexNumbers, of the dimensions specified in
                            FFT::Setup()... unless FFT::ExploitsConjugate() returns true. In that
                            case, we can take advantage of the fact that your input data is symmetric,
                            and the input arrays should only have a width of X/2+1 (for grids of width X).
        \param  outputArray An array of 2D arrays of real floating-point numbers to receive the output of
                            the transforms.
        \param batchSize    The number of arrays to transform at once. This should not exceed the batchSize
                            passed into the FFT::Setup() function.
        \return             Whether the FFT succeeded or not.
    */
    virtual bool InverseTransform2D(ComplexNumber **inputArray, float ** outputArray, unsigned int batchSize) {
        return false;
    }

    /** Returns whether this FFT implementation can take advantage of the fact that your input data is symmetric.
        If it returns true, then you should only pass in the leftmost X/2+1 columns of complex input data to
        FFT::InverseTransform2D(). If it returns false, pass in the full 2D arrays
        of input data instead. */
    virtual bool ExploitsConjugate() const = 0;

    /** Returns a human-readable description of the FFT implementation */
    virtual const char *GetName() const = 0;

    /** If FFT::TransformsEndToEnd() returns true, then this method will be implemented - meaning you can conduct the entire
        ocean wave simulation from wave frequency data to the textures used by our shaders entirely on the GPU. Otherwise,
        you should be using FFT::InverseTransform2D() instead and doing the rest of the simulation on the CPU (see the
        FFTWaterModel class for the CPU-based fallback processing.)

        \param  H0  A 2D array of complex numbers of dimensions X+1 by Y+1 for an FFT grid of X x Y. This represents
                    The H-sub-0 data described in Jerry Tessendorf's "Simulating Ocean Water" paper from 1999, which
                    is freely available on the web.
        \param  t   The simulated time sample, in seconds.
        \param  dt  The amount of time elapsed since the previous frame, in seconds.
        \param  choppiness  The choppiness of the waves desired. 0 represents no chop, and you generally don't want to go above
                            3.0 or so as the waves will start to curl into themselves and render improperly.
        \param  depth       The simulated depth of the ocean, in world units.
        \param  loopingPeriod The looping period of waves. If non zero, wave sumulation loops after that period. 0 is the default.
                              FFT Wave model uses mixture of compound frequency waves. When Period value is set, these compound frequencies
                              are constrained to be a multiply of looping frequency. Hence period may affect the wave shapes.
                              General rule is to use highest possible period length to minimize the imapact on wave field realism.
                              We recommend periods larger than 30 seconds.
        \param H0changed Whether the H0 data has changed since the previous call.
        \return     Whether the transformation succeeded or not.
    */
    virtual bool TransformEndToEnd(ComplexNumber *H0, float t, float dt, float choppiness, float depth, float loopingPeriod, bool H0changed) {
        return false;
    }

    /** Copies wave data to the texture maps for the shaders. Called from the rendering thread, while TransformEndToEnd may be called from the
        update thread. */
    virtual bool UploadTextures() {
        return true;
    }

    /** Returns whether this FFT implementation is capable of performing the entire ocean wave simulation on the GPU.
        If true, callers should use the FFT::TransformEndToEnd() method. If false, use FFT::InverseTransform2D() instead.
    */
    virtual bool TransformsEndToEnd() const {
        return false;
    }

    /** Returns the height at the given texture coordinates within the FFT patch. Valid only if
        TransformsEndToEnd() returns true. */
    virtual float GetHeight(float s, float t, bool highResolution) {
        return 0.0f;
    }

    /** Returns the displacement due to choppiness at the given texture coordinates within the FFT patch.
        Valid only if TransformsEndToEnd() returns true. */
    virtual void GetChoppinessDisplacement(float s, float t, float& chopX, float& chopZ, bool hiRes) {
        chopX = chopZ = 0.0f;
    }

    /** Retrieves a vector of positions at which spray particles should be generated. Valid only if
        TransformsEndToEnd() returns true. */
    virtual const TRITON_VECTOR(Vector3)& GetSprays() const {
        static TRITON_VECTOR(Vector3) empty;
        return empty;
    }

    /** Enables or disables spray particles on wave crests at runtime. */
    virtual void EnableSpray(bool enabled) {}

    /** Returns the slope in X and Y of the surface at the given texture coordinates within the FFT
        patch. Valid only if TransformsEndToEnd() returns true. */
    virtual void GetSlope(float s, float t, float& sx, float &sy, const Environment *env, bool hiRes);

    /** Sets the OpenGL texture maps to receive the displacement vectors, slope vectors, and amount of foam for each
        output point. Only used if FFT::TransformEndToEnd() is being used, and your renderer is a flavor of OpenGL.

        \param displacement     A floating point RGBA texture to receive the X, Y, and Z displacement for each location.
        \param slopeFoam        A floating point RGBA texture to receive the X and Y slopes of each point, and the
                                amount of foam to blend in at each point (based on the slope of the wave.)
        \return Whether the input textures were succesfully added to the FFT pipeline.
    */
    virtual bool SetOutputTextures(unsigned int displacement, unsigned int slopeFoam) {
        return true;
    }

    /** Sets the DirectX9 texture maps to receive the displacement vectors, slope vectors, and amount of foam for each
       output point. Only used if FFT::TransformEndToEnd() is being used, and your renderer is DirectX9.

       \param displacement     A floating point D3DFMT_A32B32G32R32F texture to receive the X, Y, and Z displacement for each location.
       \param slopeFoam        A floating point D3DFMT_A32B32G32R32F texture to receive the X and Y slopes of each point, and the
                               amount of foam to blend in at each point (based on the slope of the wave.)
       \return Whether the input textures were succesfully added to the FFT pipeline.
    */
    virtual bool SetOutputTextures(IDirect3DTexture9 *displacement, IDirect3DTexture9 *slopeFoam) {
        return true;
    }

    /** Sets the DirectX11 texture maps to receive the displacement vectors, slope vectors, and amount of foam for each
       output point. Only used if FFT::TransformEndToEnd() is being used, and your renderer is DirectX11.

       \param displacement     A floating point DXGI_FORMAT_R32G32B32A32_FLOAT texture to receive the X, Y, and Z displacement for each location.
       \param slopeFoam        A floating point DXGI_FORMAT_R32G32B32A32_FLOAT texture to receive the X and Y slopes of each point, and the
                               amount of foam to blend in at each point (based on the slope of the wave.)
       \return Whether the input textures were succesfully added to the FFT pipeline.
    */
    virtual bool SetOutputTextures(DX11Texture* displacement, DX11Texture* slopeFoam) {
        return true;
    }

    /** Must be called upon a lost device, prior to resetting the device, under DirectX9. Otherwise the reset will fail.
        The water model class will call this in response to its own method being called. */
    virtual bool D3D9DeviceLost() {
        return true;
    }

    /** Must be called after a device reset on DirectX9 to restore the FFT's resources. The water model class will call
        this in response to its own method being called. */
    virtual bool D3D9DeviceReset() {
        return true;
    }

    /** Retrieves the dimensions of the FFT grid. */
    void GetGridDimensions(unsigned int& xDim, unsigned int& yDim) {
        xDim = dimX;
        yDim = dimY;
    }

protected:
    FFT() : environment(0), dimX(0), dimY(0), batchSize(0), computedSlopeSteps(false),
        sInc(0), tInc(0), gridSizeX2(0), gridSizeY2(0) {}

    unsigned int dimX, dimY, batchSize;

    bool computedSlopeSteps;

    float sInc, tInc, gridSizeX2, gridSizeY2;

    const Environment *environment;
};
}

#endif