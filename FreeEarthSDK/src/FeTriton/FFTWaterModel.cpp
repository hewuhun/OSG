// Copyright (c) 2011-2014 Sundog Software, LLC. All rights reserved worldwide.

#include <FeTriton/FFTWaterModel.h>
#include <FeTriton/FFT.h>
#include <FeTriton/FFTIPP.h>
#include <FeTriton/FFTCPU.h>
#include <FeTriton/Configuration.h>
#include <FeTriton/Environment.h>
#include <FeTriton/Vector4.h>
#include <FeTriton/ProjectedGrid.h>
#ifdef _OPENMP
#include <omp.h>
#endif
#include <float.h>

#define MIN_DEPTH 1.0f
#define TESSENDORF_FFT
#define HIRES_QUERIES
//#define MEASURE_SIG_WAVE_HEIGHT

#ifdef MEASURE_SIG_WAVE_HEIGHT
#include <algorithm>
#endif

using namespace Triton;

static bool useSqrtApproximation = true;

FFTWaterModel::FFTWaterModel(const Environment *env, WaterModelTypes pType, OceanQuality quality) : WaterModel(env), spectrumModel(pType), swellsChanged(true), confused(false), lastTimeStamp(0), stillWaterUpdateCount(0),
    slopeFoamTexHandle(0), displacementTexHandle(0)
{
    if (!env) {
        Utils::DebugMsg("Null environment passed into the FFTWaterModel constructor.");
    }

    displacementMap = 0;
    slopeFoamMap = 0;
    fft = 0;
    kVectors = 0;
    h0Precalcs = 0;
    lastT = 0;
    lastDepth = -1;
    lastSlope = -1;
    lastLoopingPeriod = -1;
    heightMap = 0;
    heightMapWidth = heightMapHeight = 0;

    H = H0 = T0 = slopeX = slopeZ = chopX = chopZ = 0;
    hOut = sxOut = szOut = chopXOut = chopZOut = 0;

    cachedWindSpeed = -1;
    cachedWaveHeading = -1;
    cachedFetchLength = -1;

    fftFrameDelayed = false;
    Configuration::GetBoolValue("fft-texture-update-frame-delayed", fftFrameDelayed);

    lodBias = 0.8f;
    Configuration::GetFloatValue("fft-lod-bias", lodBias);

    N = M = 128;
    Configuration::GetIntValue("fft-grid-dimension-x", N);
    Configuration::GetIntValue("fft-grid-dimension-y", M);
    Lx = Ly = 1024.0;
    Configuration::GetFloatValue("fft-grid-size-x", Lx);
    Configuration::GetFloatValue("fft-grid-size-y", Ly);

    if (quality == BEST) {
        N *= 2;
        M *= 2;
    }

    precisionLimit = 1E-9f;
    Configuration::GetFloatValue("fft-precision-limit", precisionLimit);

    Lx /= (float)environment->GetWorldUnits();
    Ly /= (float)environment->GetWorldUnits();

    useSqrtApproximation = true;
    Configuration::GetBoolValue("fft-use-sqrt-approximation", useSqrtApproximation);

    noiseAmplitudeScale = 0.05f;
    Configuration::GetFloatValue("fft-noise-amplitude", noiseAmplitudeScale);
    noiseAmplitudeScale *= (float)environment->GetWorldUnits();

    // The mysterious "numeric constant" A from Tessendorf's paper... it seems to relate to the
    // square of our world unit scale, and controls the relationship between wave height and
    // wind speed.
    phillipsConstant = 0.008f;
    Configuration::GetFloatValue("fft-phillips-constant", phillipsConstant);
    phillipsConstant *= (float)environment->GetWorldUnits() * (float)environment->GetWorldUnits();

    phillipsConstantPierson = 0.0000081f;
    Configuration::GetFloatValue("fft-phillips-constant-pierson-moskowitz", phillipsConstantPierson);
    phillipsConstantPierson *= (float)environment->GetWorldUnits() * (float)environment->GetWorldUnits();

    phillipsConstantJONSWAP = 0.000076f;
    Configuration::GetFloatValue("fft-phillips-constant-jonswap", phillipsConstantJONSWAP);
    phillipsConstantJONSWAP *= (float)environment->GetWorldUnits() * (float)environment->GetWorldUnits();

    reflectionFilter = 0.4f;
    Configuration::GetFloatValue("fft-reflection-filter", reflectionFilter);
    doChop = true;
    Configuration::GetBoolValue("fft-enable-choppiness", doChop);
    exactNormals = true;
    Configuration::GetBoolValue("fft-exact-normals", exactNormals);

    noiseDistance = 500.0f;
    Configuration::GetFloatValue("fft-noise-distance", noiseDistance);
    noiseDistance /= (float)environment->GetWorldUnits();

    displacementDampingDistance = 300.0f;
    Configuration::GetFloatValue("fft-displacement-damping-distance", displacementDampingDistance);
    displacementDampingDistance /= (float)environment->GetWorldUnits();

    maxChop = 1.0f;
    Configuration::GetFloatValue("fft-height-max-chop", maxChop);

    swellMultiplier = 1.0f;
    Configuration::GetFloatValue("swell-height-multiplier", swellMultiplier);

#ifdef MEASURE_SIG_WAVE_HEIGHT
    exactNormals = true;
#endif

    doFoam = true;
    Configuration::GetBoolValue("do-foam", doFoam);

    displacementMap = TRITON_NEW float[N * M * 3];
    slopeFoamMap = TRITON_NEW float[N * M * 4];

    memset(displacementMap, 0, N * M * 3 * sizeof(float));
    memset(slopeFoamMap, 0, N  * M * 4 * sizeof(float));

    maxSlopeDistance = 1000.0;
    Configuration::GetDoubleValue("max-slope-distance", maxSlopeDistance);
    maxSlopeDistance /= environment->GetWorldUnits();

    gravity = 9.81f;
    Configuration::GetFloatValue("wave-gravity-force", gravity);

    swellPhaseOffset = 0;
    Configuration::GetFloatValue("swell-phase-offset", swellPhaseOffset);
}

FFTWaterModel::~FFTWaterModel()
{
    if (displacementMap) TRITON_DELETE[] displacementMap;
    if (slopeFoamMap) TRITON_DELETE[] slopeFoamMap;

    if (heightMap) TRITON_DELETE[] heightMap;

    if (H) TRITON_DELETE[] H;
    if (H0) TRITON_DELETE[] H0;
    if (T0) TRITON_DELETE[] T0;

    if (hOut) TRITON_DELETE[] hOut;
    if (sxOut) TRITON_DELETE[] sxOut;
    if (szOut) TRITON_DELETE[] szOut;
    if (chopXOut) TRITON_DELETE[] chopXOut;
    if (chopZOut) TRITON_DELETE[] chopZOut;

    if (slopeX) TRITON_DELETE[] slopeX;
    if (slopeZ) TRITON_DELETE[] slopeZ;
    if (chopX) TRITON_DELETE[] chopX;
    if (chopZ) TRITON_DELETE[] chopZ;

    if (kVectors) TRITON_DELETE[] kVectors;

    if (gaussians) TRITON_DELETE[] gaussians;

    if (h0Precalcs) TRITON_DELETE[] h0Precalcs;

    if (fft) TRITON_DELETE fft;
}

float FFTWaterModel::GetHeightData(const Vector3& position, float defaultDepth)
{
    if (heightMap) {
        Vector4 texCoords = Vector4(position) * heightMapMatrix;
        if (texCoords.x < 0 || texCoords.x >= 1.0 || texCoords.y < 0 || texCoords.y >= 1.0) {
            return -defaultDepth;
        }
        int x = (int)((double)heightMapWidth * texCoords.x);
        int y = (int)((double)heightMapHeight * texCoords.y);
        return heightMap[y * heightMapWidth + x]*environment->GetHeightMapRange() +
               environment->GetHeightMapOffset();
    }

    return -defaultDepth;
}

bool FFTWaterModel::HasHeightMap()
{
    return ( heightMap != 0 || ( environment && environment->GetUserHeightCB()) );
}

float FFTWaterModel::GetDepth(const Vector3& position, const ProjectedGrid *grid)
{
    Vector3 normal;
    float defaultDepth = grid ? grid->GetDepth(normal) : 1000.0f;
    if (environment) {

        float height = -defaultDepth;
        GETUSERHEIGHTPROC userHeightCB = environment->GetUserHeightCB();
        // Try to obtain height from the user-provided method first.
        if ( userHeightCB ) {
            height = userHeightCB( position );
        } else {
            height = GetHeightData(position, defaultDepth);
        }

        return (height * -1.0f + (float)environment->GetSeaLevel());
    } else {
        return defaultDepth;
    }
}

#ifndef max
#define max(a, b) ( a > b ? a : b )
#endif
float FFTWaterModel::GetBreakingWaveHeight(const Vector3& position, const ProjectedGrid *grid, float depth, float time)
{
    if (grid && HasHeightMap() && environment) {
        // Transform so z is up
        Vector3 v = position * grid->GetBasis();

        BreakingWavesParameters params = environment->GetBreakingWavesParameters();

        if (params.GetAmplitude() > 0.0f && params.GetWavelength() > 1.0f && params.GetExpK() > 0.0f) {

            float halfWavelength = params.GetWavelength() * 0.5f;
            float scaleFactor = ((depth - halfWavelength) / halfWavelength);
            float surgeTerm = (1.0f / params.GetSurgeDepth()) * (max(params.GetSurgeDepth(), depth) - depth);
            float wavelength = params.GetWavelength() + scaleFactor * params.GetWavelengthVariance();

            float breakHeight = 0.75f * depth;

            float halfKexp = params.GetExpK() * 0.5f;
            scaleFactor = (depth - halfKexp) / halfKexp;
            scaleFactor *= 1.0f + params.GetSteepnessVariance();
            float k = params.GetExpK() + scaleFactor;
            Vector3 localDir = params.GetWaveDirection() * grid->GetBasis();
            float dot = (float)(localDir.x * v.x + localDir.y * v.y);

            if (wavelength > 0) {
                float dotResult = dot * (float)TRITON_TWOPI / wavelength;

                float finalz = (dotResult + params.GetPhaseConstant() * time);
                finalz = (sinf(finalz) + 1.0f) * 0.5f;
                finalz = params.GetAmplitude() * powf(finalz, k);
                finalz *= 1.0f - surgeTerm;

                return finalz;
            }
        }
    }

    return 0.0f;
}

bool FFTWaterModel::PrepShaders(double time)
{
    //if (Configuration::GetFloatValue("fft-displacement-damping-distance", displacementDampingDistance)) {
    //    displacementDampingDistance /= (float)environment->GetWorldUnits();
    //}

    lastTimeStamp = time;

    if (fftFrameDelayed) {
        return true;
    }

    if (fft) {
        return fft->UploadTextures();
    } else {
        return false;
    }
}

const char *FFTWaterModel::GetFFTName() const
{
    if (fft) {
        return fft->GetName();
    } else {
        return "No FFT found";
    }
}

void FFTWaterModel::EnableSpray(bool enabled)
{
    WaterModel::EnableSpray(enabled);
    if (fft) {
        fft->EnableSpray(enabled);
    }
}

bool FFTWaterModel::Initialize(ShaderHandle shader, ShaderHandle patchShader, bool enableHeightReads, bool noGPU)
{
    int batches = 1;
    if (doChop) batches += 2;
    if (exactNormals) batches += 2;

#ifdef MEASURE_SIG_WAVE_HEIGHT
    fft = TRITON_NEW FFTCPU();
    if (!fft->Setup(N, M, batches, environment, enableHeightReads)) {
        return false;
    }
#else
    fft = FFT::Create(environment, noGPU);
    if (!fft) return false;

    if (!fft->Setup(M, N, batches, environment, enableHeightReads)) {
        Utils::DebugMsg("FFT::Setup failed; falling back to the IPP implementation.");
        // Fall back to CPU
        TRITON_DELETE fft;
        FFTIPP::LoadDLL(environment->GetResourceLoader());
        fft = TRITON_NEW FFTIPP();
        if (!fft || !fft->Setup(N, M, batches, environment, enableHeightReads)) {
            Utils::DebugMsg("IPP setup also failed (did you move Triton's DLL out of the "
                            "resources folder or are you using DLL's from a different version of Triton?) "
                            "Falling back to the CPU FFT.");
            TRITON_DELETE fft;
            fft = TRITON_NEW FFTCPU();
            if (!fft->Setup(N, M, batches, environment, enableHeightReads)) {
                Utils::DebugMsg("The CPU FFT failed to set up, there's nothing more we can do.");
                return false;
            }
        }
    }
#endif
    fftInputWidth = fft->ExploitsConjugate() ? M/2+1 : M;
    fftInputHeight = N;

    H0 = TRITON_NEW ComplexNumber[(N+1) * (M+1)];
    if (!H0) {
        Utils::DebugMsg("Failed to allocate the H0 array.");
        return false;
    }

    T0 = TRITON_NEW ComplexNumber[(N+1) * (M+1)];
    if (!T0) {
        Utils::DebugMsg("Failed to allocate the T0 array.");
        return false;
    }

    int fftSize = fftInputWidth * fftInputHeight;

    if (!fft->TransformsEndToEnd()) {
        positions.reserve(N * M);

        hOut = TRITON_NEW float[N * M];
        sxOut = TRITON_NEW float[N * M];
        szOut = TRITON_NEW float[N * M];
        chopXOut = TRITON_NEW float[N * M];
        chopZOut = TRITON_NEW float[N * M];

        H = TRITON_NEW ComplexNumber[fftSize];
        slopeX = TRITON_NEW ComplexNumber[fftSize];
        slopeZ = TRITON_NEW ComplexNumber[fftSize];
        chopX = TRITON_NEW ComplexNumber[fftSize];
        chopZ = TRITON_NEW ComplexNumber[fftSize];

        if (!hOut || !sxOut || !szOut || !chopXOut || !chopZOut || !H
                || !slopeX || !slopeZ || !chopX || !chopZ) {
            Utils::DebugMsg("Failed to allocate memory for FFTWaterModel's buffers.");
            return false;
        }
    }

    // Ensure consistent waves across multiple channels
    environment->GetRandomNumberGenerator()->SetRandomSeed(1234);

    gaussians = TRITON_NEW Gaussians[(N+1) * (M+1)];
    if (!gaussians) {
        Utils::DebugMsg("Failed to allocate Gaussians");
        return false;
    }
    for (int y = 0; y < N+1; y++) {
        for (int x = 0; x < M+1; x++) {
            int idx = y * (M+1) + x;
            Gaussian(gaussians[idx].gaussian1, gaussians[idx].gaussian2);
        }
    }

    kVectors = TRITON_NEW Precalc[N * M];
    if (!kVectors) {
        Utils::DebugMsg("Failed to allocate kVectors");
        return false;
    }

    for (int y = 0; y < N; y++) {
        for (int x = 0; x < M; x++) {
            float kx, kz;
            ComputeKVector(x, y, kx, kz);
            int idx = y*M+x;
            kVectors[idx].kx = kx;
            kVectors[idx].kz = kz;
            float kLen = sqrt(kx*kx + kz*kz);
            kVectors[idx].kLen = kLen;
            kVectors[idx].slopeXTerm = ComplexNumber(0, kx);
            kVectors[idx].slopeZTerm = ComplexNumber(0, kz);
            if (kLen != 0) {
                kVectors[idx].chopXTerm = kx / kLen;// ComplexNumber(0, kx / kLen);
                kVectors[idx].chopZTerm = kz / kLen; // ComplexNumber(0, kz / kLen);
            } else {
                kVectors[idx].chopXTerm = kVectors[idx].chopZTerm = 0; //ComplexNumber(0, 0);
            }
        }
    }

    PrecalcH0();

    maxAmplitude = 10.0 / environment->GetWorldUnits();

    return WaterModel::Initialize(shader, patchShader, enableHeightReads, noGPU);
}

void FFTWaterModel::ComputeOmega()
{
    float d = depth;
    if (d < MIN_DEPTH) d = MIN_DEPTH;

    float loopingFrequenceReciprocal = float( loopingPeriod / TRITON_TWOPI );

    if (environment) {
        for (int y = 0; y < N; y++) {
            for (int x = 0; x < M; x++) {
                int idx = y * M + x;
                kVectors[idx].wk = sqrtf((gravity / (float)environment->GetWorldUnits()) * kVectors[idx].kLen * tanhf(kVectors[idx].kLen * d));

                if( loopingFrequenceReciprocal > 0.f )
                    kVectors[idx].wk = int( kVectors[idx].wk * loopingFrequenceReciprocal ) / loopingFrequenceReciprocal;
            }
        }
    }

#ifndef TESSENDORF_FFT
    Sx = 1.0f / (1.0f - expf(-0.11f * d));
    Sz = Sx * (1.0f - expf(-0.09f * d));
    tauBeta = sinf(slope) * expf(-0.1f * d);
    tauBetaPrime = sqrtf(1.0f - tauBeta);
#endif
}

bool FFTWaterModel::UpdateSimulation(double time)
{
    if (!environment) return false;

    double windSpeed, windDirection, fetchLength;
    environment->GetWind(Vector3(environment->GetCameraPosition()), windSpeed, windDirection, fetchLength);

    noiseAmplitude = (float)((windSpeed / 10.0) * noiseAmplitudeScale);

    bool ok = true;

    bool doUpdate = true;
    if ((windSpeed == 0 && environment->GetSwells().size() == 0)) {
        doUpdate = stillWaterUpdateCount < 2;
        stillWaterUpdateCount++;
    } else {
        stillWaterUpdateCount = 0;
    }

    if (doUpdate) {
        ok = ComputeHeightFieldAndNormals((float)windSpeed, (float)time, (float)fetchLength);
    }

    lastTimeStamp = time;

    return ok;
}

void FFTWaterModel::Gaussian(float& gauss1, float& gauss2) const
{
    float x1, x2, w;

    do {
        float ranf = (float)environment->GetRandomNumberGenerator()->GetRandomDouble(0.0, 1.0);
        x1 = 2.0f * ranf - 1.0f;
        ranf = (float)environment->GetRandomNumberGenerator()->GetRandomDouble(0.0, 1.0);
        x2 = 2.0f * ranf - 1.0f;
        w = x1 * x1 + x2 * x2;
    } while ( w >= 1.0f );

    w = sqrt( (-2.0f * log( w ) ) / w );
    gauss1 = x1 * w;
    gauss2 = x2 * w;
}

void FFTWaterModel::ComputeKVector(int x, int y, float& kx, float& kz)
{
    int halfN = N >> 1;
    int halfM = M >> 1;

    float n = (float)(x - halfN);
    float m = (float)(y - halfM);

    if (Lx != 0 && Ly != 0) {
        kx = ((float)TRITON_TWOPI * n) / (float)Lx;
        kz = ((float)TRITON_TWOPI * m) / (float)Ly;
    } else {
        kx = kz = 0;
    }
}

void FFTWaterModel::PrecalcH0()
{
    h0Precalcs = TRITON_NEW H0Precalc[(N+1)*(M+1)];
    for (int y = 0; y < N+1; y++) {
        for (int x = 0; x < M+1; x++) {
            float kx, kz;
            ComputeKVector(x, y, kx, kz);
            int idx = y * (M+1) + x;
            float k2 = (kx * kx + kz * kz);
            float klen = sqrt(k2);
            float kxn = 0, kzn = 0;
            if (klen != 0.0f) {
                kxn = kx / klen;
                kzn = kz / klen;
            }
            h0Precalcs[idx].k2 = k2;

            float d = depth;
            if (d < MIN_DEPTH) d = MIN_DEPTH;

            float loopingFrequencyReciprocal = float( loopingPeriod / TRITON_TWOPI );
            float w = sqrtf((gravity / (float)environment->GetWorldUnits()) * klen * tanhf(klen * d));
            if( loopingFrequencyReciprocal > 0.f )
                w = int( w * loopingFrequencyReciprocal ) / loopingFrequencyReciprocal;

            h0Precalcs[idx].w = w;
            h0Precalcs[idx].w5 = w * w * w * w * w;
            h0Precalcs[idx].oneOverK4 = 1.0f / (k2 * k2);
            h0Precalcs[idx].kxn = kxn;
            h0Precalcs[idx].kzn = kzn;
            h0Precalcs[idx].negOneOverK2 = -1.0f / k2;
            h0Precalcs[idx].theta = atan2(-kzn, kxn);
            if (klen <= 0)
                h0Precalcs[idx].scale = 0;
            else
                h0Precalcs[idx].scale = (1.0f / (2.0f * klen)) * sqrtf(gravity / klen);
        }
    }
}

void FFTWaterModel::ComputeHT(int x, int y, float t, float chopTermX, float chopTermZ, ComplexNumber& H, ComplexNumber &chopX, ComplexNumber &chopZ)
{
    int idx = y * (M+1) + x;
    const ComplexNumber& h0 = H0[idx];

    idx = (N - y) * (M + 1) + (M - x);
    const ComplexNumber& h0NegK = H0[idx];
    ComplexNumber h0NegKConj = h0NegK.getConjugate();
#ifndef TESSENDORF_FFT
    const ComplexNumber& t0 = T0[idx];
    const ComplexNumber& t0NegK = T0[idx];
    ComplexNumber t0NegKConj = t0NegK.getConjugate();
#endif

    float wkt = kVectors[y*M + x].wk * t;

    float swkt = sinf(wkt);
    float cwkt = cosf(wkt);

#ifdef TESSENDORF_FFT

    ComplexNumber tmp1(cwkt, swkt);
    ComplexNumber tmp2(cwkt, -swkt);

    ComplexNumber term1 = h0 * tmp1;
    ComplexNumber term2 = h0NegKConj * tmp2;

    H = term1 + term2;
    chopX = ComplexNumber(0, chopTermX) * H;
    chopZ = ComplexNumber(0, chopTermZ) * H;

#else

    ComplexNumber tmp1(cwkt, swkt); // exp(iwkt)
    ComplexNumber tmp2(cwkt, -swkt); // exp(-iwkt)

    ComplexNumber term1 = h0 * tmp2;
    ComplexNumber term2 = h0NegKConj * tmp1;
    H = (term1 + term2) * ComplexNumber(-1, -1);

    term1 = t0 * tmp2;
    ComplexNumber term1b = ComplexNumber(0, -chopTermX) * term1;
    term2 = t0NegKConj * tmp1;
    ComplexNumber term2b = ComplexNumber(0, -chopTermX) * term2;
    chopX = term1b + term2b;

    term1b = ComplexNumber(0, -chopTermZ) * term1;
    term2b = ComplexNumber(0, -chopTermZ) * term2;
    chopZ = term1b + term2b;

#endif
}

const TRITON_VECTOR(Vector3)& FFTWaterModel::GetSprays()
{
    if (fft && fft->TransformsEndToEnd()) {
        return fft->GetSprays();
    }

    positions.clear();

    if (!sprayEnabled) {
        return positions;
    }

    float sprayThreshold = 1.0;
    Configuration::GetFloatValue("fft-spray-threshold", sprayThreshold);

    Vector3 v;

    float invM = 1.0f / (float)M;
    float invN = 1.0f / (float)N;

#define ALLOW_OPENMP false

    if (environment->GetOpenMPEnabled() && ALLOW_OPENMP) {
        #pragma omp parallel for
        for (int y = 0; y < N; y++) {
            for (int x = 0; x < M; x++) {
                int sidx = y*M*4+x*4;
                int didx = y*M*3+x*3;
                if (slopeFoamMap[sidx+3] > sprayThreshold) {
                    float height = displacementMap[didx+2];
                    float chopX = displacementMap[didx];
                    float chopZ = displacementMap[didx+1];
                    float fx = Lx * ( (float)x * invM );
                    float fy = Ly * ( (float)y * invN );
                    v.x = fx + chopX;
                    v.y = fy + chopZ;
                    v.z = height;
                    #pragma omp critical
                    {
                        positions.push_back(v);
                    }
                }
            }
        }
    } else {
        for (int y = 0; y < N; y++) {
            for (int x = 0; x < M; x++) {
                int sidx = y*M*4+x*4;
                int didx = y*M*3+x*3;
                if (slopeFoamMap[sidx+3] > sprayThreshold) {
                    float height = displacementMap[didx+2];
                    float chopX = displacementMap[didx];
                    float chopZ = displacementMap[didx+1];
                    float fx = Lx * ( (float)x * invM );
                    float fy = Ly * ( (float)y * invN );
                    v.x = fx + chopX;
                    v.y = fy + chopZ;
                    v.z = height;
                    positions.push_back(v);
                }
            }
        }
    }

    return positions;
}

#define EXP_CACHE_SIZE 100000
static float exp_cache[EXP_CACHE_SIZE];

static void initExpCache()
{
    static bool inited = false;

    if (!inited) {
        inited = true;
        for (int i = 0; i < EXP_CACHE_SIZE; i++) {
            exp_cache[i] = expf(-((float)i / (float)EXP_CACHE_SIZE));
        }
    }
}

static inline float fast_exp(const float a)
{
    int idx = (int)(-a * EXP_CACHE_SIZE);
    if (idx > 0 && idx < EXP_CACHE_SIZE) {
        return exp_cache[idx];
    } else {
        return expf(a);
    }
}

// Sqrt approximation, good to about 95% accuracy in our tests
static inline float fastSqrt_2(const float x)
{
    if (useSqrtApproximation) {
        union {
            int i;
            float x;
        } u;
        u.x = x;
        u.i = (1<<29) + (u.i >> 1) - (1<<22);

        return u.x;
    } else {
        return sqrtf(x);
    }
}

// Thanks to http://www.johndcook.com for the gamma function implementation.

static double gamma_func(double);

static double logGamma(double x)
{
    if (x <= 0.0) {
        return 0;
    }

    if (x < 12.0) {
        return log(fabs(gamma_func(x)));
    }

    // Abramowitz and Stegun 6.1.41
    // Asymptotic series should be good to at least 11 or 12 figures
    // For error analysis, see Whittiker and Watson
    // A Course in Modern Analysis (1927), page 252

    static const double c[8] = {
        1.0/12.0,
        -1.0/360.0,
        1.0/1260.0,
        -1.0/1680.0,
        1.0/1188.0,
        -691.0/360360.0,
        1.0/156.0,
        -3617.0/122400.0
    };
    double z = 1.0/(x*x);
    double sum = c[7];
    for (int i=6; i >= 0; i--) {
        sum *= z;
        sum += c[i];
    }
    double series = sum/x;

    static const double halfLogTwoPi = 0.91893853320467274178032973640562;
    double logGamma = (x - 0.5)*log(x) - x + halfLogTwoPi + series;
    return logGamma;
}

static double gamma_func (double x)
{
    if (x <= 0.0) {
        return 0;
    }

    // Split the function domain into three intervals:
    // (0, 0.001), [0.001, 12), and (12, infinity)

    ///////////////////////////////////////////////////////////////////////////
    // First interval: (0, 0.001)
    //
    // For small x, 1/Gamma(x) has power series x + gamma x^2  - ...
    // So in this range, 1/Gamma(x) = x + gamma x^2 with error on the order of x^3.
    // The relative error over this interval is less than 6e-7.

    const double gamma = 0.577215664901532860606512090; // Euler's gamma constant

    if (x < 0.001)
        return 1.0/(x*(1.0 + gamma*x));

    ///////////////////////////////////////////////////////////////////////////
    // Second interval: [0.001, 12)

    if (x < 12.0) {
        // The algorithm directly approximates gamma over (1,2) and uses
        // reduction identities to reduce other arguments to this interval.

        double y = x;
        int n = 0;
        bool arg_was_less_than_one = (y < 1.0);

        // Add or subtract integers as necessary to bring y into (1,2)
        // Will correct for this below
        if (arg_was_less_than_one) {
            y += 1.0;
        } else {
            n = static_cast<int> (floor(y)) - 1;  // will use n later
            y -= n;
        }

        // numerator coefficients for approximation over the interval (1,2)
        static const double p[] = {
            -1.71618513886549492533811E+0,
            2.47656508055759199108314E+1,
            -3.79804256470945635097577E+2,
            6.29331155312818442661052E+2,
            8.66966202790413211295064E+2,
            -3.14512729688483675254357E+4,
            -3.61444134186911729807069E+4,
            6.64561438202405440627855E+4
        };

        // denominator coefficients for approximation over the interval (1,2)
        static const double q[] = {
            -3.08402300119738975254353E+1,
            3.15350626979604161529144E+2,
            -1.01515636749021914166146E+3,
            -3.10777167157231109440444E+3,
            2.25381184209801510330112E+4,
            4.75584627752788110767815E+3,
            -1.34659959864969306392456E+5,
            -1.15132259675553483497211E+5
        };

        double num = 0.0;
        double den = 1.0;
        int i;

        double z = y - 1;
        for (i = 0; i < 8; i++) {
            num = (num + p[i])*z;
            den = den*z + q[i];
        }
        double result = num/den + 1.0;

        // Apply correction if argument was not initially in (1,2)
        if (arg_was_less_than_one) {
            // Use identity gamma(z) = gamma(z+1)/z
            // The variable "result" now holds gamma of the original y + 1
            // Thus we use y-1 to get back the orginal y.
            result /= (y-1.0);
        } else {
            // Use the identity gamma(z+n) = z*(z+1)* ... *(z+n-1)*gamma(z)
            for (i = 0; i < n; i++)
                result *= y++;
        }

        return result;
    }

    ///////////////////////////////////////////////////////////////////////////
    // Third interval: [12, infinity)

    if (x > 171.624) {
        // Correct answer too large to display.
        return FLT_MAX - 1.0f;
    }

    return exp(logGamma(x));
}

float FFTWaterModel::FourierAmplitudePiersonMoskowitz(const H0Precalc& precalcs, const Constants& consts)
{
    float Ek = 0.0f;

    if (consts.U10 > 0.0) {
        float Sp = (consts.alpha * consts.g * consts.g) / precalcs.w5; // Phillips

        float wpOverw = consts.wp / precalcs.w;
        float wpOverw4 = wpOverw * wpOverw * wpOverw * wpOverw;
        float Spm = Sp * fast_exp( (-5.0f / 4.0f) * wpOverw4 ); // Pierson - Moskowitz

        if (Spm < precisionLimit) Spm = precisionLimit;

        float mu = precalcs.w <= consts.wp ? 5.0f : -2.5f;
        float sw = 11.5f * powf(consts.g / (consts.wp * consts.U10), 2.5f) * powf(precalcs.w / consts.wp, mu);
        float Nsw = consts.oneOverTwoSqrtPi * ((float)gamma_func(sw + 1.0f) / (float)gamma_func(sw + 0.5f));

        float exponent = 2.0f * sw;
        float dTheta = consts.thetaw - precalcs.theta;
        if (dTheta >= TRITON_PIF) {
            dTheta -= TRITON_TWOPIF;
        } else {
            if (dTheta <= -TRITON_PIF) {
                dTheta += TRITON_TWOPIF;
            }
        }
        float halfDTheta = dTheta * 0.5f;

        float Dwtheta = Nsw * powf(cosf(halfDTheta), exponent);
        if (Dwtheta < precisionLimit || Dwtheta != Dwtheta) Dwtheta = precisionLimit;

        float Ewtheta = Spm * (confused ? consts.oneOverTwoSqrtPi : Dwtheta);

        Ek = Ewtheta * precalcs.scale;
    }

    return Ek;
}

float FFTWaterModel::FourierAmplitudeJONSWAP(const H0Precalc& precalcs, const Constants& consts)
{
    float Ek = 0.0f;

    if (consts.U10 > 0.0) {
        float Sp = (consts.alpha * consts.g * consts.g) / precalcs.w5; // Phillips

        float wpOverw = consts.wp / precalcs.w;
        float wpOverw4 = wpOverw * wpOverw * wpOverw * wpOverw;
        float Spm = Sp * fast_exp( (-5.0f / 4.0f) * wpOverw4 ); // Pierson - Moskowitz
        float sigma = precalcs.w <= consts.wp ? 0.07f : 0.09f;
        float wMinusWp = precalcs.w - consts.wp;
        float wMinusWpSq = wMinusWp * wMinusWp;
        float r = fast_exp(-( wMinusWpSq / (2.0f * sigma * sigma * consts.wp * consts.wp) ) );
        float Sj = Spm * powf(consts.gamma, r); // JONSWAP

        if (Sj < precisionLimit) Sj = precisionLimit;

        float mu = precalcs.w <= consts.wp ? 5.0f : -2.5f;
        float sw = 11.5f * powf(consts.g / (consts.wp * consts.U10), 2.5f) * powf(precalcs.w / consts.wp, mu);
        float Nsw = consts.oneOverTwoSqrtPi * (float)(gamma_func(sw + 1.0f) / gamma_func(sw + 0.5f));

        float exponent = 2.0f * sw;
        float dTheta = consts.thetaw - precalcs.theta;
        if (dTheta >= TRITON_PIF) {
            dTheta -= TRITON_TWOPIF;
        } else {
            if (dTheta <= -TRITON_PIF) {
                dTheta += TRITON_TWOPIF;
            }
        }
        float halfDTheta = dTheta * 0.5f;
        float Dwtheta = Nsw * powf(cosf(halfDTheta), exponent);

        if (Dwtheta < precisionLimit || Dwtheta != Dwtheta) Dwtheta = precisionLimit;

        float Ewtheta = Sj * (confused ? consts.oneOverTwoSqrtPi : Dwtheta);

        Ek = Ewtheta * precalcs.scale;
    }

    return Ek;
}

float FFTWaterModel::FourierAmplitudeTessendorf(const H0Precalc& precalcs, const Constants& consts)
{
    float kDotW = precalcs.kxn * consts.dirX + precalcs.kzn * consts.dirZ;

    float damping  = (kDotW < 0.0f) ? reflectionFilter : 1.0f;

    if (confused) damping = 0.5f;

    float Ph = precalcs.k2 == 0.0f ? 0.0f : (phillipsConstant *
               fast_exp(precalcs.negOneOverK2 * consts.oneOverL2) * kDotW * kDotW) *  precalcs.oneOverK4;
    Ph *= fast_exp(-precalcs.k2 * consts.w2);
    Ph *= damping;

    if (Ph < precisionLimit) Ph = precisionLimit;

    return Ph;
}

float FFTWaterModel::FourierAmplitude(const H0Precalc& precalcs, const Constants& consts)
{
    switch (actualSpectrumModel) {
    case TESSENDORF:
        return FourierAmplitudeTessendorf(precalcs, consts);
        break;

    case PIERSON_MOSKOWITZ:
        return FourierAmplitudePiersonMoskowitz(precalcs, consts);
        break;

    case JONSWAP:
        return FourierAmplitudeJONSWAP(precalcs, consts);
        break;

    default:
        return 0;
        break;
    }
}

bool FFTWaterModel::ComputeHeightFieldAndNormals(float windSpeed, float t, float fetchLength)
{
    float dt = 0;
    if (lastT) {
        dt = t - lastT;
    }
    lastT = t;

    if (depth != lastDepth || slope != lastSlope || loopingPeriod != lastLoopingPeriod ) {
        ComputeOmega();
        lastDepth = depth;
        lastSlope = slope;
        lastLoopingPeriod = loopingPeriod;
    }

    bool H0changed = false;
    if (windSpeed != cachedWindSpeed || waveHeading != cachedWaveHeading || fetchLength != cachedFetchLength || swellsChanged) {

        initExpCache();

        int maxIdx = (N+1) * (M+1);

        if (environment) {
            confused = environment->GetConfusedSeas();
        }

        Constants consts;

        consts.invSqRt2 = 1.0f / (float)TRITON_SQRT2;
        consts.g = gravity / (float)environment->GetWorldUnits();
        consts.gamma = 3.3f;
        consts.oneOverTwoSqrtPi = 1.0f / (2.0f * sqrtf((float)(TRITON_PI)));

        // JONSWAP requires a realistic wind fetch length. If none is provided,
        // fall back to Pierson/Moskowitz - as JONSWAP is just an extension of it
        // that accounts for fetch-limited seas.
        actualSpectrumModel = spectrumModel;
        if (fetchLength == 0 && spectrumModel == JONSWAP) {
            actualSpectrumModel = PIERSON_MOSKOWITZ;
        }

        switch (actualSpectrumModel) {
        case TESSENDORF: {
            float cosHeading = cosf(waveHeading);
            float sinHeading = sinf(waveHeading);
            consts.dirX = sinHeading;
            consts.dirZ = cosHeading;
            float L = (windSpeed * windSpeed) / consts.g;
            consts.oneOverL2 = 1.0f / (L * L);
            float w = L * 0.001f;
            consts.w2 = w * w;
        }
        break;

        case PIERSON_MOSKOWITZ: {
            consts.U10 = windSpeed;
            consts.alpha = phillipsConstantPierson;
            consts.wp = (0.855f * consts.g) / consts.U10;
            consts.thetaw = waveHeading - TRITON_PIF * 0.5f;
            if (consts.thetaw < -TRITON_PIF) consts.thetaw += TRITON_TWOPIF;
        }
        break;

        case JONSWAP: {
            consts.U10 = windSpeed; // Wind speed at 10m
            float F = fetchLength;
            consts.alpha = phillipsConstantJONSWAP * powf((consts.U10 * consts.U10) / (F * consts.g), 0.22f);
            consts.wp = 22.0f * powf( (consts.g * consts.g) / (consts.U10 * F) , 1.0f / 3.0f); // peak angular frequency
            consts.thetaw = waveHeading - TRITON_PIF * 0.5f;
            if (consts.thetaw < -TRITON_PIF) consts.thetaw += TRITON_TWOPIF;
        }
        break;

        default: {
            Utils::DebugMsg("Unexpected water model type encountered.");
        }
        break;
        };

        if (environment->GetOpenMPEnabled()) {
            #pragma omp parallel for
            for (int idx = 0; idx < maxIdx; idx++) {
                const H0Precalc& precalc = h0Precalcs[idx];
                float Ek = FourierAmplitude(precalc, consts);

                float coeff = consts.invSqRt2 * fastSqrt_2(Ek);

#ifdef TESSENDORF_FFT
                H0[idx].real = coeff * gaussians[idx].gaussian1;
                H0[idx].img = coeff * gaussians[idx].gaussian2;
#else
                H0[idx] = ComplexNumber(coeff * gaussians[idx].gaussian1 * tauBetaPrime * Sz, coeff * gaussians[idx].gaussian2 * tauBeta * Sx);
                T0[idx] = ComplexNumber(coeff * gaussians[idx].gaussian1 * tauBetaPrime * Sx, coeff * gaussians[idx].gaussian2 * tauBeta * Sz);
#endif

            }
        } else {
            for (int idx = 0; idx < maxIdx; idx++) {
                const H0Precalc& precalc = h0Precalcs[idx];
                float Ek = FourierAmplitude(precalc, consts);

                float coeff = consts.invSqRt2 * fastSqrt_2(Ek);

#ifdef TESSENDORF_FFT
                H0[idx].real = coeff * gaussians[idx].gaussian1;
                H0[idx].img = coeff * gaussians[idx].gaussian2;
#else
                H0[idx] = ComplexNumber(coeff * gaussians[idx].gaussian1 * tauBetaPrime * Sz, coeff * gaussians[idx].gaussian2 * tauBeta * Sx);
                T0[idx] = ComplexNumber(coeff * gaussians[idx].gaussian1 * tauBetaPrime * Sx, coeff * gaussians[idx].gaussian2 * tauBeta * Sz);
#endif

            }
        }

        ApplySwells();

        H0changed = true;
        cachedWindSpeed = windSpeed;
        cachedWaveHeading = waveHeading;
        swellsChanged = false;
        cachedFetchLength = fetchLength;
    }

    bool ok = false;

    if (!fft->TransformsEndToEnd()) {

        if (environment->GetOpenMPEnabled()) {
            #pragma omp parallel for
            for (int y = 0; y < fftInputHeight; y++) {
                for (int x = 0; x < fftInputWidth; x++) {
                    int idx = y*fftInputWidth+x;
                    const Precalc& kVector = kVectors[y*M+x];
                    ComputeHT(x, y, t, kVector.chopXTerm, kVector.chopZTerm, H[idx], chopX[idx], chopZ[idx]);

                    if (exactNormals) {
                        slopeX[idx] = kVector.slopeXTerm * H[idx];
                        slopeZ[idx] = kVector.slopeZTerm * H[idx];
                    }

                    if (kVector.kLen == 0.0) {
                        chopX[idx] = chopZ[idx] = ComplexNumber(0, 0);
                    }
                }
            }
        } else {
            for (int y = 0; y < fftInputHeight; y++) {
                for (int x = 0; x < fftInputWidth; x++) {
                    int idx = y*fftInputWidth+x;
                    const Precalc& kVector = kVectors[y*M+x];
                    ComputeHT(x, y, t, kVector.chopXTerm, kVector.chopZTerm, H[idx], chopX[idx], chopZ[idx]);

                    if (exactNormals) {
                        slopeX[idx] = kVector.slopeXTerm * H[idx];
                        slopeZ[idx] = kVector.slopeZTerm * H[idx];
                    }

                    if (kVector.kLen == 0.0) {
                        chopX[idx] = chopZ[idx] = ComplexNumber(0, 0);
                    }
                }
            }
        }

        ComplexNumber *inputs[5];
        float *outputs[5];

        inputs[0] = H;
        outputs[0] = hOut;

        int numFFTs = 1;
        if (exactNormals) {
            inputs[numFFTs] = slopeX;
            outputs[numFFTs] = sxOut;
            numFFTs++;
            inputs[numFFTs] = slopeZ;
            outputs[numFFTs] = szOut;
            numFFTs++;
        }

        if (doChop) {
            inputs[numFFTs] = chopX;
            outputs[numFFTs] = chopXOut;
            numFFTs++;
            inputs[numFFTs] = chopZ;
            outputs[numFFTs] = chopZOut;
            numFFTs++;
        }

        ok = fft->InverseTransform2D(inputs, outputs, numFFTs);

#ifdef DEBUG_FFT
        DumpToBitmap("height.bmp", N, M, hOut);
        DumpToBitmap("slopeX.bmp", N, M, sxOut);
        DumpToBitmap("slopeZ.bmp", N, M, szOut);
        DumpToBitmap("chopX.bmp", N, M, chopXOut);
        DumpToBitmap("chopZ.bmp", N, M, chopZOut);

        exit(0);
#endif

        if (ok && N != 0 && M != 0) {
            const float twoCellsWidth =  ((Lx * 2.0f) / (float)M);
            const float twoCellsHeight = ((Ly * 2.0f) / (float)N);

#ifdef MEASURE_SIG_WAVE_HEIGHT
            float maxH = 0;
            //TRITON_VECTOR(float) wavePeaks;
#endif

            if (environment->GetOpenMPEnabled()) {
#ifndef MEASURE_SIG_WAVE_HEIGHT
                #pragma omp parallel for
#endif
                for (int y = 0; y < N; y++) {
                    for (int x = 0; x < M; x++) {
                        int idx = y*M+x;
                        int dMapIdx = y*M*3+x*3;
                        int nMapIdx = y*M*4+x*4;

                        if (doChop) {
                            displacementMap[dMapIdx] = chopXOut[idx] * chopScale;
                            displacementMap[dMapIdx+1] = chopZOut[idx] * chopScale;
                        } else {
                            displacementMap[dMapIdx] = displacementMap[dMapIdx+1] = 0;
                        }

                        displacementMap[dMapIdx+2] = hOut[idx];

                        float dx, dy, sxx, syy, sxy, syx;

                        if (exactNormals) {
                            dx = sxOut[idx];
                            dy = szOut[idx];

#ifdef MEASURE_SIG_WAVE_HEIGHT
                            if (hOut[idx] > maxH) maxH = hOut[idx];
                            //if (fabs(sxx) < 0.005 && fabs(syy) < 0.005) {
                            //    wavePeaks.push_back(hOut[idx]);
                            //}
#endif
                        } else {
                            int prevX = x > 0 ? x-1 : M-1;
                            int nextX = x < M-1 ? x + 1 : 0;
                            int prevY = y > 0 ? y - 1 : N-1;
                            int nextY = y < N - 1 ? y + 1 : 0;

                            float xWidth = twoCellsWidth + chopXOut[y * M + nextX] - chopXOut[y * M + prevX];
                            float yDepth = twoCellsHeight + chopZOut[nextY * M + x] - chopZOut[prevY * M + x];
                            float xDelta = (hOut[y * M + nextX] - hOut[y * M + prevX]);
                            float yDelta = (hOut[nextY * M + x] - hOut[prevY * M + x]);


                            sxy = (chopXOut[nextY * M + x] - chopXOut[prevY * M + x]) / yDepth;

                            syy = (chopZOut[nextY * M + x] - chopZOut[prevY * M + x]) / yDepth;

                            syx = (chopZOut[y * M + nextX] - chopZOut[y * M + prevX]) / xWidth;

                            sxx = (chopXOut[y * M + nextX] - chopXOut[y * M + prevX]) / xWidth;

                            dx = xDelta / xWidth;
                            dy = yDelta / xWidth;
                        }

                        Vector3 sx(1.0, 0.0, dx);
                        Vector3 sy(0.0, 1.0, dy);
                        Vector3 norm = sx.Cross(sy);
                        norm.Normalize();

                        slopeFoamMap[nMapIdx] = (float)norm.x;
                        slopeFoamMap[nMapIdx+1] = (float)norm.y;
                        slopeFoamMap[nMapIdx+2] = (float)norm.z;

                        if (doFoam && !exactNormals) {
                            float Jxx = 1.0f + chopScale * sxx;
                            float Jyy = 1.0f + chopScale * syy;
                            float Jxy = chopScale * sxy;
                            float Jyx = chopScale * syx;
                            float J = Jxx * Jyy - Jxy * Jyx;

                            float foam = 1.0f - J;

                            slopeFoamMap[nMapIdx+3] = foam;
                        }
                    }
                }
            } else {
                for (int y = 0; y < N; y++) {
                    for (int x = 0; x < M; x++) {
                        int idx = y*M+x;
                        int dMapIdx = y*M*3+x*3;
                        int nMapIdx = y*M*4+x*4;

                        if (doChop) {
                            displacementMap[dMapIdx] = chopXOut[idx] * chopScale;
                            displacementMap[dMapIdx+1] = chopZOut[idx] * chopScale;
                        } else {
                            displacementMap[dMapIdx] = displacementMap[dMapIdx+1] = 0;
                        }

                        displacementMap[dMapIdx+2] = hOut[idx];

                        float dx, dy, sxx, syy, sxy, syx;

                        if (exactNormals) {
                            dx = sxOut[idx];
                            dy = szOut[idx];

#ifdef MEASURE_SIG_WAVE_HEIGHT
                            if (hOut[idx] > maxH) maxH = hOut[idx];
                            //if (fabs(sxx) < 0.005 && fabs(syy) < 0.005) {
                            //    wavePeaks.push_back(hOut[idx]);
                            //}
#endif
                        } else {
                            int prevX = x > 0 ? x-1 : M-1;
                            int nextX = x < M-1 ? x + 1 : 0;
                            int prevY = y > 0 ? y - 1 : N-1;
                            int nextY = y < N - 1 ? y + 1 : 0;

                            float xWidth = twoCellsWidth + chopXOut[y * M + nextX] - chopXOut[y * M + prevX];
                            float yDepth = twoCellsHeight + chopZOut[nextY * M + x] - chopZOut[prevY * M + x];
                            float xDelta = (hOut[y * M + nextX] - hOut[y * M + prevX]);
                            float yDelta = (hOut[nextY * M + x] - hOut[prevY * M + x]);


                            sxy = (chopXOut[nextY * M + x] - chopXOut[prevY * M + x]) / yDepth;

                            syy = (chopZOut[nextY * M + x] - chopZOut[prevY * M + x]) / yDepth;

                            syx = (chopZOut[y * M + nextX] - chopZOut[y * M + prevX]) / xWidth;

                            sxx = (chopXOut[y * M + nextX] - chopXOut[y * M + prevX]) / xWidth;

                            dx = xDelta / xWidth;
                            dy = yDelta / xWidth;
                        }

                        Vector3 sx(1.0, 0.0, dx);
                        Vector3 sy(0.0, 1.0, dy);
                        Vector3 norm = sx.Cross(sy);
                        norm.Normalize();

                        slopeFoamMap[nMapIdx] = (float)norm.x;
                        slopeFoamMap[nMapIdx+1] = (float)norm.y;
                        slopeFoamMap[nMapIdx+2] = (float)norm.z;

                        if (doFoam && !exactNormals) {
                            float Jxx = 1.0f + chopScale * sxx;
                            float Jyy = 1.0f + chopScale * syy;
                            float Jxy = chopScale * sxy;
                            float Jyx = chopScale * syx;
                            float J = Jxx * Jyy - Jxy * Jyx;

                            float foam = 1.0f - J;

                            slopeFoamMap[nMapIdx+3] = foam;
                        }
                    }
                }
            }
#ifdef MEASURE_SIG_WAVE_HEIGHT
            printf("MaxH is %f\n", maxH);
            /*
            printf("Found %d peaks\n", wavePeaks.size());
            std::sort(wavePeaks.rbegin(), wavePeaks.rend());
            int topThirdPeaks = wavePeaks.size() / 3;
            float total = 0;
            for (int i = 0; i < topThirdPeaks; i++) {
                total += fabs(wavePeaks[i] * 2.0f);
            }
            float sig = (total / (float)topThirdPeaks);
            printf("Significant Wave Height is %f\n", sig);
            */
#endif
        }
    } else {
        if (fftFrameDelayed) {
            fft->UploadTextures();
        }
        ok = fft->TransformEndToEnd(H0, t, dt, chopScale, depth < MIN_DEPTH ? MIN_DEPTH : depth, loopingPeriod, H0changed);
    }

    return ok;
}

void FFTWaterModel::NotifySwellsChanged()
{
    swellsChanged = true;
}

void FFTWaterModel::AddSwell(float amplitude, float wavelength, float direction, float phase)
{
    Swell swell;
    const float oneOverTwoSqrtPi = 1.0f / (2.0f * sqrtf((float)(TRITON_PI)));

    swell.fourierAmplitude = oneOverTwoSqrtPi * amplitude;

    float k = (float)TRITON_TWOPI / wavelength;
    float n = (Lx * k) / (float)TRITON_TWOPI;
    int y = (int)(n * cos(direction));
    int x = (int)(n * sin(direction));
    if (x < -M/2 || x > M/2) return;
    if (y < -N/2 || y > N/2) return;

    swell.m = x + M/2;
    swell.n = y + N/2;

    swell.phase = phase;

    swells.push_back(swell);
}

void FFTWaterModel::ClearSwells()
{
    swells.clear();
}

void FFTWaterModel::ApplySwells()
{
    swells.clear();

    if (environment) {
        const TRITON_VECTOR(SwellDescription)& envSwells = environment->GetSwells();
        TRITON_VECTOR(SwellDescription)::const_iterator it;
        for (it = envSwells.begin(); it != envSwells.end(); it++) {
            const SwellDescription& swell = *it;
            AddSwell(it->height * swellMultiplier, it->wavelength, it->direction, it->phase);
        }
    }

    TRITON_VECTOR(Swell)::const_iterator it;
    for (it = swells.begin(); it != swells.end(); it++) {
        const Swell& swell = *it;
        int idx = swell.n * (M+1) + swell.m;

        float phase = swellPhaseOffset + swell.phase;
        H0[idx].real = swell.fourierAmplitude * cosf(phase);
        H0[idx].img = swell.fourierAmplitude * sinf(phase);
    }
}

static double inline Repeat(double x, double y)
{
    if (0 == y)
        return x;

    if (x < 0 && x > -1E-6) x = 0;

    return x - y * floor(x/y);
}

float FFTWaterModel::GetHeight(const Vector3& position, const ProjectedGrid *grid, Vector3& normal, bool hiRes)
{
    if (!environment || !grid) return 0;

    if (Lx == 0 || Ly == 0 || M == 0 || N == 0) return 0;

    float shorelineDampening = 1.0, breakingWave = 0.0;
    if (grid->GetBreakingWavesEnabled() && !environment->GetBreakingWaveMap()) {
        double depth = GetDepth(position, grid);
        double surgeDepth = environment->GetBreakingWavesParameters().GetSurgeDepth();
        double min = surgeDepth < depth ? surgeDepth : depth;

        if (surgeDepth > 0) {
            shorelineDampening = (float)(min / surgeDepth);

            if (lastTimeStamp > 0) {
                breakingWave = GetBreakingWaveHeight(position, grid, (float)depth, (float)lastTimeStamp);
            }
        }
    }

    double s, t;

    Vector3 north = grid->GetNorth();
    Vector3 up = environment->GetUpVector();
    Vector3 east = north.Cross(up);
    const double epsilon = 1E-10;
    if (east.SquaredLength() < epsilon) {
        // North or south pole, pick an arbitrary North direction.
        Vector3 fakeNorth(1, 0, 0);
        east = fakeNorth.Cross(up);
    }
    east.Normalize();
    north = up.Cross(east);
    north.Normalize();

    // Convert position to texture coords
    if (environment->IsGeocentric()) {
        // Sync up with shader reference frame
#define BLOCKSIZE 100000.0
#define OFFSET     50000.0
        Vector3 camPos = position;// Vector3(environment->GetCameraPosition());
        Vector3 refPt = camPos + Vector3(OFFSET, OFFSET, OFFSET);
        Vector3 localReference = Vector3(fmod(refPt.x, BLOCKSIZE), fmod(refPt.y, BLOCKSIZE), fmod(refPt.z, BLOCKSIZE));
        double camOffsetX = 0, camOffsetY = 0;
        if (environment->IsGeocentric()) {
            camOffsetX = localReference.Dot(east);
            camOffsetY = localReference.Dot(north);
        }
        double e = position.Dot(east);
        double n = position.Dot(north);
        s = Repeat(e + camOffsetX, (double)Lx);
        t = Repeat(n + camOffsetY, (double)Ly);
    } else {
        // Transform to z-is-up
        Vector3 xPos = position * grid->GetBasis();
        s = Repeat(xPos.x, (double)Lx);
        t = Repeat(xPos.y, (double)Ly);
    }

    float height = 0.0f;
    float sx = 0.0f, sy = 0.0f;

    float st = (float)s / Lx;
    float tt = (float)t / Ly;

    if (fft->TransformsEndToEnd()) {
        // Offset the sample point to account for local choppiness
        float chopX, chopZ;
        fft->GetChoppinessDisplacement(st, tt, chopX, chopZ, hiRes);

        s -= chopX;
        t -= chopZ;

        s = Repeat(s, Lx);
        t = Repeat(t, Ly);
        st = (float)s / Lx;
        tt = (float)t / Ly;

        height = fft->GetHeight(st, tt, hiRes) * shorelineDampening + breakingWave;

        fft->GetSlope(st, tt, sx, sy, environment, hiRes);
    } else {
        // Offset the sample point to account for local choppiness
        int x = (int)(st * (float)M);
        int y = (int)(tt * (float)N);
        int idx = y*M*3 + x*3;

        float chopX = 0, chopZ = 0;

        int xlow = (int)(st * (float)M);
        int ylow = (int)(tt * (float)N);

        int xhigh, yhigh;
        float quadS, quadT;
        const float A = 0.5f;

        if (hiRes) {
            xhigh = xlow + 1;
            yhigh = ylow + 1;
            if (xhigh >= (int)M) xhigh = 0;
            if (yhigh >= (int)N) yhigh = 0;

            quadS = (st - (float)xlow / (float)M) * M;
            quadT = (tt - (float)ylow / (float)N) * N;

            if (quadS < quadT) {
                float x0 = displacementMap[xlow * 3 + yhigh * M * 3];
                float x1 = displacementMap[xlow * 3 + ylow * M * 3];
                float x2 = displacementMap[xhigh * 3 + yhigh * M * 3];

                float z0 = displacementMap[xlow * 3 + yhigh * M * 3 + 1];
                float z1 = displacementMap[xlow * 3 + ylow * M * 3 + 1];
                float z2 = displacementMap[xhigh * 3 + yhigh * M * 3 + 1];

                float A1 = (1.0f - quadT) * 0.5f;
                float A2 = quadS * 0.5f;
                float A0 = A - (A1 + A2);

                chopX = (x0 * A0 + x1 * A1 + x2 * A2) / A;
                chopZ = (z0 * A0 + z1 * A1 + z2 * A2) / A;
            } else {
                float x0 = displacementMap[xlow * 3 + ylow * M * 3];
                float x1 = displacementMap[xhigh * 3 + ylow * M * 3];
                float x2 = displacementMap[xhigh * 3 + yhigh * M * 3];

                float z0 = displacementMap[xlow * 3 + ylow * M * 3 + 1];
                float z1 = displacementMap[xhigh * 3 + ylow * M * 3 + 1];
                float z2 = displacementMap[xhigh * 3 + yhigh * M * 3 + 1];

                float A2 = quadT * 0.5f;
                float A0 = (1.0f - quadS) * 0.5f;
                float A1 = A - (A0 + A2);

                chopX = (x0 * A0 + x1 * A1 + x2 * A2) / A;
                chopZ = (z0 * A0 + z1 * A1 + z2 * A2) / A;
            }
        } else {
            chopX = displacementMap[xlow * 3 + ylow * M * 3];
            chopZ = displacementMap[xlow * 3 + ylow * M * 3 + 1];
        }

        s -= chopX;
        t -= chopZ;
        s = Repeat(s, Lx);
        t = Repeat(t, Ly);
        st = (float)s / Lx;
        tt = (float)t / Ly;

        if (hiRes ) {
            if (quadS < quadT) {
                float h0 = displacementMap[xlow * 3 + yhigh * M * 3 + 2];
                float h1 = displacementMap[xlow * 3 + ylow * M * 3 + 2];
                float h2 = displacementMap[xhigh * 3 + yhigh * M * 3 + 2];
                float A1 = (1.0f - quadT) * 0.5f;
                float A2 = quadS * 0.5f;
                float A0 = A - (A1 + A2);

                height = (h0 * A0 + h1 * A1 + h2 * A2) / A;
            } else {
                float h0 = displacementMap[xlow * 3 + ylow * M * 3 + 2];
                float h1 = displacementMap[xhigh * 3 + ylow * M * 3 + 2];
                float h2 = displacementMap[xhigh * 3 + yhigh * M * 3 + 2];
                float A2 = quadT * 0.5f;
                float A0 = (1.0f - quadS) * 0.5f;
                float A1 = A - (A0 + A2);

                height = (h0 * A0 + h1 * A1 + h2 * A2) / A;
            }
        } else {
            height = displacementMap[xlow * 3 + ylow * M * 3 + 2] * shorelineDampening;
        }

        height *= shorelineDampening;
        height += breakingWave;

        float mInc = 1.0f / (float)M;
        float nInc = 1.0f / (float)N;
        float prevSt = st - mInc;
        if (prevSt < 0.0f) prevSt += 1.0f;
        float nextSt = st + mInc;
        if (nextSt >= 1.0f) nextSt -= 1.0f;
        float prevTt = tt - nInc;
        if (prevTt < 0.0f) prevTt += 1.0f;
        float nextTt = tt + nInc;
        if (nextTt >= 1.0f) nextTt -= 1.0f;

        int prevX = (int)(prevSt * (float)M);
        int nextX = (int)(nextSt * (float)M);
        int prevY = (int)(prevTt * (float)N);
        int nextY = (int)(nextTt * (float)N);
        int prevXIdx = y*M*3 + prevX*3 + 2;
        int nextXIdx = y*M*3 + nextX*3 + 2;
        int prevYIdx = prevY*M*3 + x*3 + 2;
        int nextYIdx = nextY*M*3 + x*3 + 2;

        sx = (displacementMap[nextXIdx] * shorelineDampening - displacementMap[prevXIdx] * shorelineDampening) / ( (Lx * 2.0f) / (float)M );
        sy = (displacementMap[nextYIdx] * shorelineDampening - displacementMap[prevYIdx] * shorelineDampening) / ( (Ly * 2.0f) / (float)N );
    }

    Vector3 sxv(1.0, 0.0, sx);
    Vector3 syv(0.0, 1.0, sy);
    normal = sxv.Cross(syv);
    normal.Normalize();

    if (environment->IsGeocentric()) {
        normal = (east * normal.x + north * normal.y + up * normal.z);
        normal.Normalize();
    } else {
        normal = normal * grid->GetInverseBasis();
    }

    return height;
}

void FFTWaterModel::UpdateBreakingWaveDirection(Environment *env)
{
    if (env) {
        BreakingWavesParameters bwp = env->GetBreakingWavesParameters();
        if (bwp.GetAutoWaveDirection()) {
            Matrix4 texCoordToPos;
            texCoordToPos = heightMapMatrix.InverseCramers(1E-12);

#if 0
            Vector3 cameraPos3 = Vector3(env->GetCameraPosition());
            Vector3 upperRightPos3 = cameraPos3 + env->GetRightVector() * 500.0;
            Vector3 in = env->GetRightVector().Cross(env->GetUpVector());
            Vector3 lowerLeftPos3 = cameraPos3 + in * 500.0;

            float depth = GetDepth(cameraPos3, 0);
            cameraPos3 = cameraPos3 - env->GetUpVector() * depth;
            depth = GetDepth(upperRightPos3, 0);
            upperRightPos3 = upperRightPos3 - env->GetUpVector() * depth;
            depth = GetDepth(lowerLeftPos3, 0);
            lowerLeftPos3 = lowerLeftPos3 - env->GetUpVector() * depth;

            Vector3 v1 = upperRightPos3 - cameraPos3;
            Vector3 v2 = lowerLeftPos3 - cameraPos3;
#else
            Vector4 upperLeftPos = Vector4(0.0, 0.99, 0.0, 1.0) * texCoordToPos;
            Vector3 upperLeftPos3 = Vector3(upperLeftPos.x, upperLeftPos.y, upperLeftPos.z);
            Vector4 upperRightPos = Vector4(0.99, 0.99, 0.0, 1.0) * texCoordToPos;
            Vector3 upperRightPos3 = Vector3(upperRightPos.x, upperRightPos.y, upperRightPos.z);
            Vector4 lowerLeftPos = Vector4(0.0, 0.0, 0.0, 1.0) * texCoordToPos;
            Vector3 lowerLeftPos3 = Vector3(lowerLeftPos.x, lowerLeftPos.y, lowerLeftPos.z);

            double maxSqLength = maxSlopeDistance * maxSlopeDistance;
            Vector4 centerPos = Vector4(0.5, 0.5, 0.0, 1.0) * texCoordToPos;
            Vector3 centerPos3 = Vector3(centerPos.x, centerPos.y, centerPos.z);

            Vector3 rightDir = upperRightPos3 - upperLeftPos3;
            Vector3 downDir = lowerLeftPos3 - upperLeftPos3;

            if ( rightDir.SquaredLength() > maxSqLength || downDir.SquaredLength() > maxSqLength ) {
                rightDir.Normalize();
                downDir.Normalize();
                upperLeftPos3 = centerPos3 - rightDir * (maxSlopeDistance * 0.5) - downDir * (maxSlopeDistance * 0.5);
                upperRightPos3 = upperLeftPos3 + rightDir * maxSlopeDistance;
                lowerLeftPos3 = upperLeftPos3 + downDir * maxSlopeDistance;
            }

            float depth = GetDepth(upperLeftPos3, 0);
            upperLeftPos3 = upperLeftPos3 - env->GetUpVector() * depth;
            depth = GetDepth(upperRightPos3, 0);
            upperRightPos3 = upperRightPos3 - env->GetUpVector() * depth;
            depth = GetDepth(lowerLeftPos3, 0);
            lowerLeftPos3 = lowerLeftPos3 - env->GetUpVector() * depth;

            Vector3 v1 = upperRightPos3 - upperLeftPos3;
            Vector3 v2 = lowerLeftPos3 - upperLeftPos3;
#endif
            v1.Normalize();
            v2.Normalize();
            Vector3 N = v2.Cross(v1);
            N.Normalize();

            if (N.Dot(env->GetUpVector()) < 0) {
                N = v1.Cross(v2);
                N.Normalize();
            }

            Vector3 dir = env->GetUpVector() - N;

            if (dir.SquaredLength() > 0) {
                dir.Normalize();
                bwp.SetWaveDirection(dir);
                env->SetBreakingWavesParameters(bwp);
            }
        }
    }
}

#ifdef DEBUG_FFT
void FFTWaterModel::DumpToBitmap(const char *filename, int x, int y, float *data)
{
    struct bmpfile_magic {
        unsigned char magic[2];
    };

    typedef unsigned int uint32_t;
    typedef unsigned short uint16_t;
    typedef int int32_t;
    typedef short int16_t;

    struct bmpfile_header {
        uint32_t filesz;
        uint16_t creator1;
        uint16_t creator2;
        uint32_t bmp_offset;
    };

    typedef struct {
        uint32_t header_sz;
        int32_t width;
        int32_t height;
        uint16_t nplanes;
        uint16_t bitspp;
        uint32_t compress_type;
        uint32_t bmp_bytesz;
        int32_t hres;
        int32_t vres;
        uint32_t ncolors;
        uint32_t nimpcolors;
    } BITMAPINFOHEADER;

    bmpfile_magic magic;
    magic.magic[0] = 'B';
    magic.magic[1] = 'M';
    bmpfile_header hdr;
    hdr.bmp_offset = sizeof(bmpfile_magic) + sizeof(bmpfile_header) + sizeof(BITMAPINFOHEADER);
    hdr.filesz = hdr.bmp_offset + x * y * 4;
    hdr.creator1 = hdr.creator2 = 0;
    BITMAPINFOHEADER bi;
    bi.bitspp = 32;
    bi.bmp_bytesz = x * y * 4;
    bi.compress_type = 0;
    bi.header_sz = sizeof(BITMAPINFOHEADER);
    bi.height = y;
    bi.hres = 0;
    bi.ncolors = 0;
    bi.nimpcolors = 0;
    bi.nplanes = 1;
    bi.vres = 0;
    bi.width = x;

    float min = 100000.0f, max=-100000.0f;
    for (int i = 0; i < x * y; i++) {
        if (data[i] < min) min = data[i];
        if (data[i] > max) max = data[i];
    }
    float range = max - min;

    FILE *f = fopen(filename, "wb");
    if (f) {
        fwrite(&magic, sizeof(bmpfile_magic), 1, f);
        fwrite(&hdr, sizeof(bmpfile_header), 1, f);
        fwrite(&bi, sizeof(BITMAPINFOHEADER), 1, f);

        float *p = data;
        for (int row = 0; row < y; row++) {
            for (int col = 0; col < x; col++) {
                float val = *p++;
                float scaled = (val - min) / (range);

                unsigned char pix = (unsigned char)(scaled * 255.0f);
                fwrite((void*)&pix, 1, 1, f);
                fwrite((void*)&pix, 1, 1, f);
                fwrite((void*)&pix, 1, 1, f);
                pix = 255;
                fwrite((void*)&pix, 1, 1, f);
            }
        }
        fclose(f);
    }
}
#endif
