// Copyright (c) 2011-2014 Sundog Software, LLC. All rights reserved worldwide.

#ifndef TRITON_FFT_WATERMODEL_H
#define TRITON_FFT_WATERMODEL_H

/** \file FFTWaterModel.h
   \brief An FFT-based physical model of ocean waves.
 */

#include <FeTriton/Ocean.h>
#include <FeTriton/WaterModel.h>
#include <FeTriton/ComplexNumber.h>
#include <FeTriton/Matrix4.h>
#include <vector>

// #define DEBUG_FFT

namespace Triton
{
class Environment;
class Vector4;
class FFT;

/** A WaterModel implementation based on Fast Fourier Transforms, simulating very large
    numbers of waves at once and yielding the most realistic results. */
class FFTWaterModel : public WaterModel
{
public:
    FFTWaterModel(const Environment *env, WaterModelTypes type, OceanQuality quality);
    virtual ~FFTWaterModel();

    virtual bool Initialize(ShaderHandle shader, ShaderHandle patchShader, bool enableHeightReads, bool noGPU);

    virtual bool UpdateSimulation(double time);

    virtual const char *GetFFTName() const;

    virtual void UpdateHeightMap(Environment *env) {}

    virtual const FFT* GetFFT() const {
        return fft;
    }

    virtual bool PrepShaders(double time);

    virtual void NotifySwellsChanged();

    virtual void SetConfused(bool pConfused) {
        confused = pConfused;
    }

    float GetDepth(const Vector3& position, const ProjectedGrid *grid);

    /** Retrieves a list of points in world coordinates where spray particles should be generated. */
    const TRITON_VECTOR(Vector3)& GetSprays();

    virtual void EnableSpray(bool enabled);

    virtual void GetTextures(TextureHandle& slopeFoam, TextureHandle& displacement) {
        slopeFoam = slopeFoamTexHandle;
        displacement = displacementTexHandle;
    }

    virtual void GetFftTextureSize(float& x, float& y) const {
        x = Lx;
        y = Ly;
    }

protected:

    virtual float GetHeight(const Vector3& position, const ProjectedGrid *grid, Vector3& normal, bool hiRes);

    virtual float GetBreakingWaveHeight(const Vector3& position, const ProjectedGrid *grid, float depth, float time);

    virtual void AddSwell(float amplitude, float wavelength, float direction, float phase);

    virtual void ClearSwells();

    virtual void UpdateBreakingWaveDirection(Environment *env);

    virtual float GetHeightData(const Vector3& position, float defaultDepth);

    virtual bool HasHeightMap();

    int N, M, fftInputHeight, fftInputWidth;
    float Lx, Ly;
    float phillipsConstant, phillipsConstantPierson, phillipsConstantJONSWAP;
    float reflectionFilter;
    float lastT, lastDepth, lastSlope, lastLoopingPeriod;
    float Sx, Sz, tauBeta, tauBetaPrime;
    float precisionLimit;

    float *heightMap;
    int heightMapWidth, heightMapHeight;
    Matrix4 heightMapMatrix;

    bool swellsChanged, confused;
    float swellMultiplier;

    bool doChop, exactNormals;
    float maxChop;

    float *displacementMap, *slopeFoamMap;
    ComplexNumber *H, *H0, *T0, *slopeX, *slopeZ, *chopX, *chopZ;
    float *hOut, *sxOut, *szOut, *chopXOut, *chopZOut;

    TextureHandle slopeFoamTexHandle, displacementTexHandle;

    struct Precalc {
        float kx, kz;
        float kLen, wk;
        ComplexNumber slopeXTerm, slopeZTerm;
        float chopXTerm, chopZTerm;
    };
    Precalc *kVectors;

    struct H0Precalc {
        float k2, w, w5, kxn, kzn, oneOverK4, negOneOverK2, theta, scale;
    };
    H0Precalc *h0Precalcs;

    struct Gaussians {
        float gaussian1, gaussian2;
    };
    Gaussians *gaussians;

    float cachedWindSpeed, cachedWaveHeading, cachedFetchLength;
    float foamDecay, breakingSlope, foamFade, foamBlend;
    float noiseAmplitude, noiseAmplitudeScale;
    float noiseDistance;
    bool doFoam;
    float lodBias;
    double maxSlopeDistance;

    WaterModelTypes spectrumModel, actualSpectrumModel;

    TRITON_VECTOR(Vector3) positions;

    struct Swell {
        int n, m;
        float fourierAmplitude;
        float phase;
    };

    TRITON_VECTOR(Swell) swells;

    FFT *fft;

    double lastTimeStamp;

    bool fftFrameDelayed;

    int stillWaterUpdateCount;

    float gravity, swellPhaseOffset;

private:
    void Gaussian(float& gauss1, float& gauss2) const;
    void ComputeKVector(int x, int y, float& kx, float& kz);
    void ComputeHT(int x, int y, float t, float chopTermX, float chopTermZ, ComplexNumber& H, ComplexNumber& chopX, ComplexNumber& chopZ);
    bool ComputeHeightFieldAndNormals(float windSpeed, float t, float fetchLength);
    void ComputeOmega();
    void PrecalcH0();
    void ApplySwells();

    struct Constants {
        float dirX, dirZ, oneOverL2, w2;
        float U10, wp, alpha, thetaw;
        float invSqRt2, gamma, g, oneOverTwoSqrtPi;
    };

    float FourierAmplitude(const H0Precalc& precalcs, const Constants& consts);
    float FourierAmplitudeJONSWAP(const H0Precalc& precalcs, const Constants& consts);
    float FourierAmplitudeTessendorf(const H0Precalc& precalcs, const Constants& consts);
    float FourierAmplitudePiersonMoskowitz(const H0Precalc& precalcs, const Constants& consts);

#ifdef DEBUG_FFT
    void DumpToBitmap(const char *filename, int x, int y, float *data);
#endif
};
}

#endif