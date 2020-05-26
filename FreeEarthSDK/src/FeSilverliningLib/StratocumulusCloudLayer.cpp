// Copyright (c) 2010-2015 Sundog Software LLC. All rights reserved worldwide.

#include <FeSilverliningLib/StratocumulusCloudLayer.h>
#include <FeSilverliningLib/StratocumulusCloud.h>
#include <FeSilverliningLib/Configuration.h>
#include <FeSilverliningLib/Vector4.h>
#include <FeSilverliningLib/CloudGenerator.h>
#include <FeSilverliningLib/CloudGeneratorFactory.h>
#include <FeSilverliningLib/Utils.h>
#include <FeSilverliningLib/Metaball.h>
#include <FeSilverliningLib/ResourceLoader.h>
#include <math.h>

using namespace SilverLining;
using namespace std;

#define HAS_CLOUD_BIT 0x01
#define PHASE_TRANSITION_BIT 0x02
#define VAPOR_BIT 0x04
#define BILINEAR_LIGHTING 1
#define COLOR_RANDOMNESS 0.2

// The minimum voxel height with a cloud in it, to prevent clamping artifacts
#define MIN_HEIGHT 1

StratocumulusCloudLayer::StratocumulusCloudLayer() : voxels(0), noiseTexData(0), cloudTexData(0), vb(0), ib(0),
    volumeData(0), noiseVolumeData(0), randomData(0),
    volumeLightingVector( 0, 0, 0 ),
    volumeLightingVectorFullRefreshTolerance( cos( 1.0 * PI / 180 ) ),
    volumeLightingVectorSliceRefreshTolerance( cos( 0.1 * PI / 180 ) ),
    volumeLightingSlice( 0 ),
    inscatteringTerm( 0.75 ),
    invalid( true )
{
    Configuration::GetDoubleValue("stratocumulus-voxel-dimension", voxelDimension);
    voxelDimension *= Atmosphere::GetUnitScale();
    Configuration::GetDoubleValue("stratocumulus-droplet-size", dropletSize);
    dropletSize *= Atmosphere::GetUnitScale();
    Configuration::GetDoubleValue("stratocumulus-droplets-per-cubic-cm", dropletsPerCubicCm);
    dropletsPerCubicCm /= pow(Atmosphere::GetUnitScale(), 3.0);
    Configuration::GetDoubleValue("stratocumulus-albedo", albedo);
    Configuration::GetDoubleValue("stratocumulus-extinction-probability", extinctionProbability);
    Configuration::GetDoubleValue("stratocumulus-transition-probability", transitionProbability);
    Configuration::GetDoubleValue("stratocumulus-vapor-probability", vaporProbability);
    Configuration::GetIntValue("stratocumulus-initial-evolve", initialEvolve);
    Configuration::GetDoubleValue("stratocumulus-skylight-term", ambientScattering);
    Configuration::GetDoubleValue("stratocumulus-inscattering-term", inscatteringTerm);
    Configuration::GetDoubleValue("stratocumulus-light-sampling-distance", lightSamplingDistance);
    lightSamplingDistance *= Atmosphere::GetUnitScale();
    Configuration::GetDoubleValue("stratocumulus-multiple-scattering-term", multipleScatteringTerm);
    Configuration::GetDoubleValue("stratocumulus-jitter", jitter);
    jitter *= Atmosphere::GetUnitScale();
    Configuration::GetDoubleValue("stratocumulus-max-size", maxSize);
    maxSize *= Atmosphere::GetUnitScale();
    Configuration::GetDoubleValue("stratocumulus-fade-falloff", fadeFalloff);

    shader = Renderer::GetInstance()->LoadShaderFromFile("Shaders/Stratocumulus.cg", VERTEX_PROGRAM);
    hdrShader = Renderer::GetInstance()->LoadShaderFromFile("Shaders/StratocumulusHDR.cg", VERTEX_PROGRAM);

    cloudMotion = Vector3(0,0,0);

    // lastBasis - initialize
    for (int m = 0; m < 3; m++) {
        for (int n = 0; n < 3; n++) {
            if (n == m)
                lastBasis.elem[m][n] = 1.0;
            else
                lastBasis.elem[m][n] = 0.0;
        }
    }
}

StratocumulusCloudLayer::~StratocumulusCloudLayer()
{
    FreeVoxels();

    if (vb) SL_DELETE vb;
    if (ib) SL_DELETE ib;

    if (volumeData) Renderer::GetInstance()->ReleaseTexture(volumeData);
    if (noiseVolumeData) Renderer::GetInstance()->ReleaseTexture(noiseVolumeData);

    if (noiseTexData) SL_DELETE[] noiseTexData;
    if (cloudTexData) SL_DELETE[] cloudTexData;
#ifdef COLOR_RANDOMNESS
    if (randomData) SL_DELETE[] randomData;
#endif  // COLOR_RANDOMNESS
}

void StratocumulusCloudLayer::SetBaseWidth(double meters)
{
    if (meters > maxSize) meters = maxSize;
    CloudLayer::SetBaseWidth(meters);
}

void StratocumulusCloudLayer::SetBaseLength(double meters)
{
    if (meters > maxSize) meters = maxSize;
    CloudLayer::SetBaseLength(meters);
}

void StratocumulusCloudLayer::MoveClouds(double x, double y, double z)
{
    Vector3 v(x, y, z);

    if (!cloudWrap && !isInfinite) {
        v = v * Renderer::GetInstance()->GetInverseBasis3x3();
        layerX += v.x;
        layerZ += v.z;
        baseAltitude += v.y;
    } else {
        cloudMotion = cloudMotion + Vector3(x, y, z);
    }

    noiseOffset = noiseOffset + Vector3(x, y, z);
}

void StratocumulusCloudLayer::FreeVoxels()
{
    if (voxels) {
        int i, j, k;

        for (i = 0; i < voxelWidth; i++) {
            for (j = 0; j < voxelDepth; j++) {
                for (k = 0; k < voxelHeight; k++) {
                    SL_DELETE voxels[i][j][k];
                }
                SL_FREE(voxels[i][j]);
            }
            SL_FREE(voxels[i]);
        }
        SL_FREE(voxels);
    }
    voxels = 0;
}

void StratocumulusCloudLayer::UploadVolumeTexture()
{
    unsigned int texBytes = (voxelWidth * voxelHeight * voxelDepth) * sizeof(unsigned char);
    cloudTexData = SL_NEW unsigned char[texBytes * 2];
    unsigned char *p = cloudTexData;

#ifdef COLOR_RANDOMNESS
    randomData = SL_NEW float[voxelWidth * voxelHeight  * voxelDepth];
    float *r = randomData;
#endif  // COLOR_RANDOMNESS

    //int renType = Renderer::GetInstance()->GetType();

    for (int h = 0; h < voxelHeight; h++) {
        for (int d = 0; d < voxelDepth; d++) {
            for (int w = 0; w < voxelWidth; w++) {
                float col = (voxels[w][d][h]->states & HAS_CLOUD_BIT) ? 1.0f : 0.0f;
                *p++ = (unsigned char)(col * 255.0f);
                *p++ = 0;

#ifdef COLOR_RANDOMNESS
                float rnd = SilverLining::Atmosphere::GetCurrentAtmosphere()->GetRandomNumberGenerator()->UniformRandomFloat();
                *r++ = 1.0f - rnd * (float)COLOR_RANDOMNESS;
#endif  // COLOR_RANDOMNESS
            }
        }
    }

    Renderer *ren = Renderer::GetInstance();
    if (volumeData) ren->ReleaseTexture(volumeData);
    volumeData = 0;
    ren->Load3DTextureLA(cloudTexData, voxelWidth, voxelDepth, voxelHeight, &volumeData, true, true, false);
}


void StratocumulusCloudLayer::UploadNoiseTexture()
{
#define NOISE_DIM 32

    unsigned int texBytes = (NOISE_DIM * NOISE_DIM * NOISE_DIM * 3) * sizeof(unsigned char);
    noiseTexData = SL_NEW unsigned char[texBytes];

    Vector3 colors[NOISE_DIM][NOISE_DIM][NOISE_DIM];

    int w, h, d;

    Atmosphere* atm = Atmosphere::GetCurrentAtmosphere();

    for ( w = 0; w < NOISE_DIM; w++) {
        for ( h = 0; h < NOISE_DIM; h++) {
            for ( d = 0; d < NOISE_DIM; d++) {
                colors[w][h][d].x = atm->GetRandomNumberGenerator()->UniformRandomDouble();
                colors[w][h][d].y = atm->GetRandomNumberGenerator()->UniformRandomDouble();
                colors[w][h][d].z = atm->GetRandomNumberGenerator()->UniformRandomDouble();
            }
        }
    }

    int blurWidth = 1;
    unsigned char *p = noiseTexData;
    for ( w = 0; w < NOISE_DIM; w++) {
        for ( h = 0; h < NOISE_DIM; h++) {
            for ( d = 0; d < NOISE_DIM; d++) {
                Vector3 avg;
                int samples = 0;
                for (int bw = w - blurWidth; bw <= w + blurWidth; bw++) {
                    for (int bh = h - blurWidth; bh <= h + blurWidth; bh++) {
                        for (int bd = d - blurWidth; bd <= d + blurWidth; bd++) {
                            if (bw >= 0 && bw < NOISE_DIM && bh >= 0 && bh < NOISE_DIM && bd >= 0 && bd < NOISE_DIM) {
                                samples++;
                                avg = avg + colors[bw][bh][bd];
                            }
                        }
                    }
                }
                avg = avg * (1.0 / (float)samples);
                *p++ = (unsigned char)(avg.x * 255.0);
                *p++ = (unsigned char)(avg.y * 255.0);
                *p++ = (unsigned char)(avg.z * 255.0);
            }
        }
    }

    Renderer *ren = Renderer::GetInstance();
    if (noiseVolumeData) ren->ReleaseTexture(noiseVolumeData);
    noiseVolumeData = 0;
    ren->Load3DTextureRGB(noiseTexData, NOISE_DIM, NOISE_DIM, NOISE_DIM, &noiseVolumeData, true, true, true);
}

void StratocumulusCloudLayer::TrimExcessVoxels()
{
    int i, j, k;

    int maxH = 0;

    for (i = 0; i < voxelWidth; i++) {
        for (j = 0; j < voxelDepth; j++) {
            for (k = 0; k < voxelHeight; k++) {
                if (voxels[i][j][k]->states & HAS_CLOUD_BIT) {
                    if (k > maxH) maxH = k;
                }
            }
        }
    }

    if (maxH < voxelHeight - 1 - MIN_HEIGHT) {
        int newVoxelHeight = maxH + 1 + MIN_HEIGHT;
        for (i = 0; i < voxelWidth; i++) {
            for (j = 0; j < voxelDepth; j++) {
                StratocumulusVoxel **newVoxels = (StratocumulusVoxel**)(SL_MALLOC(newVoxelHeight * sizeof(StratocumulusVoxel*)));

                for (k = 0; k < newVoxelHeight; k++)
                    newVoxels[k] = voxels[i][j][k];

                for (k = newVoxelHeight; k < voxelHeight; k++)
                    SL_DELETE voxels[i][j][k];

                SL_FREE(voxels[i][j]);
                voxels[i][j] = newVoxels;
            }
        }

        voxelHeight = newVoxelHeight;
        SetThickness(voxelHeight * voxelDimension);
    }
}

bool StratocumulusCloudLayer::SeedClouds(const Atmosphere& atm)
{
    if (!atm.IsInitialized()) return false;

    ClearClouds();

    Vector3 layerWorldPosition;
    double x, z;
    GetLayerPosition(x, z);
    layerWorldPosition = Vector3(x, GetBaseAltitude(), z);

    int i, j, k;

    // Allocate the voxels
    voxelWidth = (int)(GetBaseWidth() / voxelDimension);
    voxelHeight = (int)(GetThickness() / voxelDimension) + MIN_HEIGHT;
    voxelDepth = (int)(GetBaseLength() / voxelDimension);

    voxels = (StratocumulusVoxel****)(SL_MALLOC(voxelWidth * sizeof(StratocumulusVoxel*)));
    for ( i = 0; i < voxelWidth; i++) {
        voxels[i] = (StratocumulusVoxel***)(SL_MALLOC(voxelDepth * sizeof(StratocumulusVoxel*)));
        for ( j = 0; j < voxelDepth; j++) {
            voxels[i][j] = (StratocumulusVoxel**)(SL_MALLOC(voxelHeight * sizeof(StratocumulusVoxel*)));
            for ( k = 0; k < voxelHeight; k++) {
                voxels[i][j][k] = SL_NEW StratocumulusVoxel();
            }
        }
    }

    CloudGenerator *generator = CloudGeneratorFactory::Create("stratocumulus");
    if (generator) {
        generator->SetDesiredArea(GetBaseWidth() * GetBaseLength());

        double densityMultiplier = 1.0;
        Configuration::GetDoubleValue("stratocumulus-density", densityMultiplier);
        generator->SetDesiredCoverage(GetDensity() * densityMultiplier);

        generator->StartGeneration();

        double cloudWidth, cloudDepth, cloudHeight;

        while (generator->GetNextCloud(cloudWidth, cloudDepth, cloudHeight)) {
            if (cloudWidth > voxelDimension && cloudDepth > voxelDimension
                    && cloudHeight > voxelDimension * MIN_HEIGHT) {
                unsigned int vWidth = (int)(cloudWidth / voxelDimension);
                unsigned int vHeight = (int)(cloudHeight / voxelDimension);
                unsigned int vDepth = (int)(cloudDepth / voxelDimension);

                unsigned int vHalfWidth = vWidth >> 1;
                //unsigned int vHalfHeight = vHeight >> 1;
                unsigned int vHalfDepth = vDepth >> 1;

                // Pick a random origin for a bounding ellipsoid
                Atmosphere *atm = Atmosphere::GetCurrentAtmosphere();
                int cw = atm->GetRandomNumberGenerator()->UniformRandomIntRange(0, voxelWidth);
                int cd = atm->GetRandomNumberGenerator()->UniformRandomIntRange(0, voxelDepth);
                int ch = MIN_HEIGHT;

                // Keep track of each cloud in the model so we can expose their individual bounding volume info
                Vector3 posInLayer(cw * voxelDimension, 0, cd * voxelDimension);
                posInLayer = posInLayer - Vector3(GetBaseWidth() / 2.0, 0.0, GetBaseLength() / 2.0);
                Vector3 posInWorld = (layerWorldPosition + posInLayer) * Renderer::GetInstance()->GetBasis3x3();
                StratocumulusCloud *cl = SL_NEW StratocumulusCloud(this);
                cl->SetWorldPosition(posInWorld);
                cl->SetLayerPosition(posInLayer);
                cl->SetSize(cloudWidth, cloudDepth, cloudHeight);
                AddCloud(cl);

                for (int w = cw - (int)vHalfWidth; w < cw + (int)vHalfWidth; w++) {
                    for (int d = cd - (int)vHalfDepth; d < cd + (int)vHalfDepth; d++) {
                        for (int h = ch; h < ch + (int)vHeight; h++) {
                            if (w >= 0 && d >= 0 && h >=0 && w < voxelWidth && d < voxelDepth && h < voxelHeight) {
                                double x = (double)(w - cw);
                                double y = (double)(d - cd);
                                double z = (double)(h - ch);
                                double depth = 1.0 - ((x * x) / (vHalfWidth * vHalfWidth) +
                                                      (y * y) / (vHalfDepth * vHalfDepth) + (z * z) / (vHeight * vHeight));
                                if (depth >= 0 && depth <= 1.0) {
                                    voxels[w][d][h]->vaporProbability = (float)(vaporProbability * depth);
                                    voxels[w][d][h]->phaseTransitionProbability = (float)(transitionProbability * depth);
                                    voxels[w][d][h]->extinctionProbability = (float)(extinctionProbability * (1.0 - depth));
                                }
                            }
                        }
                    }
                }
            }
        }
        SL_DELETE generator;
    }

    for ( i = 0; i < initialEvolve; i++) {
        IncrementGrowth();
    }

    TrimExcessVoxels();
    UploadVolumeTexture();
    UploadNoiseTexture();

    FreeVoxels();

    // Allocate geometry and indices for bounding box.
    vb = SL_NEW VertexBuffer(8);

    // initialize bounding box vertices before serialization.
    ComputeBoundingBox();

    const int indicesBack[] =   {3, 7, 0, 4,
                                 1, 5, 2, 6,
                                 2, 3, 1, 0,
                                 7, 6, 4, 5,
                                 4, 5, 0, 1,
                                 3, 2, 7, 6
                                };

    const int indicesFront[] =  {0, 4, 3, 7,
                                 2, 6, 1, 5,
                                 3, 2, 0, 1,
                                 4, 5, 7, 6,
                                 0, 1, 4, 5,
                                 7, 6, 3, 2
                                };

    ib = SL_NEW IndexBuffer(4 * 6 * 2);
    if (ib->LockBuffer()) {
        Index *idx = ib->GetIndices();

        for ( i = 0; i < 4 * 6; i++) {
            idx[i] = (Index)indicesFront[i];
        }

        for ( i = 0; i < 4 * 6; i++) {
            idx[i + 4 * 6] = (Index)indicesBack[i];
        }

        ib->UnlockBuffer();
    }

    return true;
}

const Vector3& StratocumulusCloudLayer::GetWorldPosition() const
{
    double layerX, layerZ;
    double baseAlt;
    GetLayerPosition(layerX, layerZ);
    baseAlt = GetBaseAltitudeGeocentric();

    static Vector3 layerPos;
    layerPos.x = layerX;
    layerPos.y = baseAlt;
    layerPos.z = layerZ;

    return layerPos;
}

void StratocumulusCloudLayer::ComputeTexCoords(Vertex *v)
{
    Renderer *ren = Renderer::GetInstance();

    // Get the cloud position in world coords
    double layerX, layerZ;
    double width, depth;
    double baseAlt, thickness;
    GetLayerPosition(layerX, layerZ);
    baseAlt = GetBaseAltitudeGeocentric();
    thickness = GetThickness();
    width = GetBaseWidth();
    depth = GetBaseLength();
    layerX -= width * 0.5;
    layerZ -= depth * 0.5;

    // Transform from eye coords to world coords
    Vector4 pt(v->x, v->y, v->z, 1.0);
    Matrix4 modelview;
    ren->GetModelviewMatrix(&modelview);
    if (GetIsInfinite()) {
        modelview.elem[0][3] = modelview.elem[1][3] = modelview.elem[2][3] = 0;
    }
    modelview = modelview.Inverse();
    pt = modelview * pt;

    // add in motion for stationary layers with wrapping clouds
    if (!GetIsInfinite()) {
        pt.x -= cloudMotion.x;
        pt.y -= cloudMotion.y;
        pt.z -= cloudMotion.z;
    }

    // Tranform to standard basis
    Vector3 pt3(pt.x, pt.y, pt.z);
    pt3 = GetLocalBasis() * pt3;

    // Now figure it out
    if (GetIsInfinite()) {
        Vector3 camPos = Renderer::GetInstance()->GetCamPos() * Renderer::GetInstance()->GetInverseBasis3x3();
        v->u = (float)(pt3.x / width) + 0.5f;
        v->v = (float)((pt3.y - baseAlt + camPos.y) / thickness);
        v->t = (float)(pt3.z / depth) + 0.5f;
    } else {
        v->u = (float)((pt3.x - layerX) / width);
        v->v = (float)((pt3.y - baseAlt) / thickness);
        v->t = (float)((pt3.z - layerZ) / depth);
    }
}

void StratocumulusCloudLayer::ComputeBoundingBox()
{
    int i;

    // Construct a bounding box for the cloud layer in world coords
    Vector3 bbox[8];

    double layerX, layerZ;
    double width, depth;
    double baseAlt, thickness;
    GetLayerPosition(layerX, layerZ);
    baseAlt = GetBaseAltitudeGeocentric();
    thickness = GetThickness();
    width = GetBaseWidth();
    depth = GetBaseLength();

    Matrix4 mv;
    Renderer::GetInstance()->GetModelviewMatrix(&mv);

    // Figure out the texcoords of the box origin and store it for the shader
    if (GetIsInfinite()) {
        Vector3 camPos0 = Atmosphere::GetCurrentAtmosphere()->GetCamPos();
        Vector3 cameraPos = lastBasis * camPos0;
        Vector3 delta = cameraPos - lastCameraPos;
        cameraMotion = cameraMotion + delta;
        lastBasis = Renderer::GetInstance()->GetBasis3x3();
        lastCameraPos = lastBasis * Atmosphere::GetCurrentAtmosphere()->GetCamPos();

        Vector3 motion = cloudMotion * Renderer::GetInstance()->GetInverseBasis3x3();
        originTexCoords = Vector3( (cameraMotion.x - motion.x) / width,
                                   motion.y / thickness,
                                   (cameraMotion.z - motion.z) / depth);

        layerX = layerZ = 0;
        Vector3 camPos =  Renderer::GetInstance()->GetCamPos() * Renderer::GetInstance()->GetInverseBasis3x3();
        baseAlt = baseAlt - camPos.y;
    } else {
        originTexCoords = Vector3(0, 0, 0);
    }

    const Matrix3& basis = GetLocalBasis();
    Vector3 layerPos = Vector3(layerX, baseAlt, layerZ) * basis;

    Vector3 halfRight = Vector3(width, 0, 0) * basis * 0.5;
    Vector3 fullBack = Vector3(0, 0, -depth) * basis;
    Vector3 halfBack = fullBack * 0.5;
    Vector3 up = Vector3(0, thickness, 0) * basis;

    bbox[0] = layerPos - halfRight - halfBack;
    bbox[1] = bbox[0] + up;
    bbox[2] = layerPos + halfRight - halfBack + up;
    bbox[3] = bbox[2] - up;
    bbox[4] = bbox[0] + fullBack;
    bbox[5] = bbox[1] + fullBack;
    bbox[6] = bbox[2] + fullBack;
    bbox[7] = bbox[3] + fullBack;

    // Transform it into eye coords

    if (GetIsInfinite()) {
        mv.elem[0][3] = mv.elem[1][3] = mv.elem[2][3] = 0;
    }

    for (i = 0; i < 8; i++) {
        Vector4 v(bbox[i]);
        v = mv * v;
        bbox[i] = Vector3(v.x, v.y, v.z);
    }

    if (vb->LockBuffer()) {
        Vertex *verts = vb->GetVertices();

        Color vertColor(fade * alpha, fade * alpha, fade * alpha, fade * alpha);

        for (int i = 0; i < 8; i++) {
            verts[i].x = (float)bbox[i].x;
            verts[i].y = (float)bbox[i].y;
            verts[i].z = (float)bbox[i].z;
            verts[i].w = 1.0f;
            verts[i].SetColor(vertColor);
            ComputeTexCoords(&verts[i]);
        }

        vb->UnlockBuffer();
    }
}

bool StratocumulusCloudLayer::Draw(int pass, const Vector3 *lightPos, const Color *pLightColor,
                                   bool pInvalid, bool wantsLightingUpdate, unsigned long now, const Sky*, CloudBackdropManager *)
{
    if (pInvalid) {
        invalid = true;
    }

    if (pass == 1) {
        ComputeBoundingBox();

        if (hasForcedCloudColor) {
            lightColor = Color(forcedCloudColor.x, forcedCloudColor.y, forcedCloudColor.z);
            if (forcedCloudColorLighting) {
                lightColor = lightColor * (*pLightColor);
            }
        } else {
            lightColor = *pLightColor;
        }
        lightColor.ScaleToUnitOrLess();
        lightObjectPos = *lightPos;
        lightWorldPos = lightObjectPos * Renderer::GetInstance()->GetBasis3x3();

        Renderer::GetInstance()->SubmitBlendedObject(this);
    }
    return true;
}

void StratocumulusCloudLayer::DrawBlendedObject()
{
    ShaderHandle currentShader = Atmosphere::GetHDREnabled() ? hdrShader : shader;

    if (vb && currentShader) {
        Renderer *ren = Renderer::GetInstance();

        {
            // Check and compute volume lighting and update volume texture if neccessary.
            int updateSliceMin = 0, updateSliceMax = 0;
            if( ComputeVolumeLighting( updateSliceMin, updateSliceMax ) ) {
                int rowSize = voxelWidth * 2 * sizeof( unsigned char );
                int sliceSize = voxelDepth * rowSize;
                unsigned char * data = cloudTexData + updateSliceMin * sliceSize;
                ren->SubLoad3DTextureLA( data,
                                         voxelWidth, voxelDepth, updateSliceMax - updateSliceMin,
                                         0, 0, updateSliceMin,
                                         rowSize, sliceSize, volumeData );
            }
        }

        ren->EnableDepthReads(true);
        ren->EnableDepthWrites(false);
        ren->EnableTexture2D(false);
        ren->EnableTexture3D(true);
        ren->EnableLighting(false);
        ren->EnableBackfaceCulling(true);
        ren->EnableBlending(ONE, INVSRCALPHA);
        ren->Enable3DTexture(volumeData, 0);
        ren->Enable3DTexture(noiseVolumeData, 1);

        Matrix4 proj, view;
        ren->BindShader(currentShader, VERTEX_PROGRAM);


        float outputScale = Atmosphere::GetCurrentAtmosphere()->GetOutputScale();
        Renderer::GetInstance()->SetConstantVector(currentShader, "sl_outputScale", Vector3((double)outputScale, 0.0, 0.0));

        ren->GetProjectionMatrix(&proj);
        ren->SetConstantMatrix(currentShader, "sl_projectionMatrix", proj);

        double extinctionCrossSection = PI * dropletSize * dropletSize;
        double extinctionCoefficient = extinctionCrossSection * (dropletsPerCubicCm * 100 * 100 * 100);

        double opticalDepth = extinctionCoefficient * lightSamplingDistance;
        double extinction = 1.0 - exp(-opticalDepth);
        ren->SetConstantVector(currentShader, "sl_extinctionCoefficient", Vector3(extinctionCoefficient, extinctionCoefficient, extinctionCoefficient));

        double constTerm = (albedo * opticalDepth) / (4.0 * PI);
        Vector4 lightObjectPosAndConstTerm(lightObjectPos.x, lightObjectPos.y, lightObjectPos.z, constTerm);
        ren->SetConstantVector4(currentShader, "sl_lightObjectDirAndConstTerm", lightObjectPosAndConstTerm);

        Vector4 lightWorldDirAndExtinction(lightWorldPos.x, lightWorldPos.y, lightWorldPos.z, extinction);
        ren->SetConstantVector4(currentShader, "sl_lightWorldDirAndExtinction", lightWorldDirAndExtinction);

        Vertex camPos;
        camPos.x = 0;
        camPos.y = 0;
        camPos.z = 0;
        camPos.w = 1.0f;
        ComputeTexCoords(&camPos);
        Vector3 camTexCoords(camPos.u, camPos.v, camPos.t);
        ren->SetConstantVector(currentShader, "sl_cameraTexCoords", camTexCoords);

        Vector3 lightTexCoords = Vector3(lightObjectPos.x * (double)voxelWidth,
                                         lightObjectPos.y * (double)voxelHeight, lightObjectPos.z * (double)voxelDepth);
        lightTexCoords.Normalize();

        ren->SetConstantVector(currentShader, "sl_lightTexCoords", lightTexCoords);

        Color forceColor;
        float fade = GetFadeTowardEdges() ? 1.0f : 0.0f;
        Vector3 fadeFlag(fade, fade, fadeFalloff);

        if (Billboard::GetForceConstantColor(forceColor)) {
            ren->SetConstantVector(currentShader, "sl_lightColor", forceColor.ToVector3());
            ren->SetConstantVector(currentShader, "sl_skyLightColor", forceColor.ToVector3() * ambientScattering);
            ren->SetConstantVector(currentShader, "sl_multipleScatteringTerm", forceColor.ToVector3() * multipleScatteringTerm);
        } else {
            ren->SetConstantVector(currentShader, "sl_lightColor", Vector3(lightColor.r, lightColor.g, lightColor.b));

            Color skyLight = Metaball::GetAmbientColor();
            ren->SetConstantVector(currentShader, "sl_skyLightColor", Vector3(skyLight.r, skyLight.g, skyLight.b) * ambientScattering);

            Vector3 multipleScattering = Vector3(lightColor.r, lightColor.g, lightColor.b) * multipleScatteringTerm;
            ren->SetConstantVector(currentShader, "sl_multipleScatteringTerm", multipleScattering);
        }

        ren->SetConstantVector(currentShader, "sl_fadeFlag", fadeFlag);

        Vector3 voxelDimensions = Vector3(voxelWidth, voxelHeight, voxelDepth);
        ren->SetConstantVector(currentShader, "sl_voxelDimensions", voxelDimensions * voxelDimension);
        ren->SetConstantVector(currentShader, "sl_viewSampleDimensions", Vector3(1.0 / (voxelWidth * voxelDimension),
                               1.0 / (voxelHeight * voxelDimension), 1.0 / (voxelDepth * voxelDimension) ));
        ren->SetConstantVector(currentShader, "sl_lightSampleDimensions", Vector3(lightSamplingDistance / (voxelWidth * voxelDimension),
                               lightSamplingDistance / (voxelHeight * voxelDimension), lightSamplingDistance / (voxelDepth * voxelDimension) ));
        ren->SetConstantVector(currentShader, "sl_noiseOffset", Vector3(noiseOffset.x / (voxelWidth * voxelDimension),
                               noiseOffset.y / (voxelHeight * voxelDimension), noiseOffset.z / (voxelDepth * voxelDimension) ));
        ren->SetConstantVector(currentShader, "sl_jitter", Vector3(jitter / (voxelWidth * voxelDimension), jitter / (voxelHeight * voxelDimension),
                               jitter / (voxelDepth * voxelDimension)));

        ren->SetConstantVector(currentShader, "sl_originTexCoords", originTexCoords);

        Color fogColor = Metaball::GetFogColor();
        double fogDensity = Metaball::GetFogDensity();
        Vector4 fogColorAndDensity(fogColor.r, fogColor.g, fogColor.b, fogDensity);
        ren->SetConstantVector4(currentShader, "sl_fogColorAndDensity", fogColorAndDensity);

        Vector3 unitScale = Vector3(Atmosphere::GetUnitScale(), 0, 0);
        ren->SetConstantVector(currentShader, "sl_unitScale", unitScale);

        Vector3 posInObject = camTexCoords;
        bool isInside = (posInObject.x > 0 && posInObject.x < 1.0 && posInObject.y > 0 && posInObject.y < 1.0
                         && posInObject.z > 0 && posInObject.z < 1.0);

        // Don't render edges for orthographic views
        bool isOrthographic = proj.elem[3][0] == 0 && proj.elem[3][1] == 0 && proj.elem[3][2] == 0 && proj.elem[3][3] == 1.0;
        int nFaces = isOrthographic ? 2 : 6;

        for (int face = 0; face < nFaces; face++) {
            if (isInside) {
                ren->DrawStrip(vb->GetHandle(), ib->GetHandle(), face * 4 + 4 * 6, 4, 8, true);
            } else {
                ren->DrawStrip(vb->GetHandle(), ib->GetHandle(), face * 4, 4, 8, true);
            }
        }

        ren->UnbindShader(VERTEX_PROGRAM);
        ren->EnableTexture3D(false);
        ren->EnableDepthWrites(true);
        ren->DisableTexture(0);
        ren->DisableTexture(1);
        ren->EnableBackfaceCulling(false);
    }
}

bool StratocumulusCloudLayer::DrawSetup(int pass, const Vector3 *lightPos, const Color *lightColor)
{
    return true;
}

bool StratocumulusCloudLayer::EndDraw(int pass)
{
    return true;
}

bool StratocumulusCloudLayer::Serialize(std::ostream& s)
{
#define WRITE_DOUBLE(X) s.write((char *)&X, sizeof(double))
#define WRITE_INT(X) s.write((char *)&X, sizeof(int))

    WRITE_DOUBLE(voxelDimension);
    WRITE_DOUBLE(lightSamplingDistance);
    WRITE_DOUBLE(multipleScatteringTerm);
    WRITE_DOUBLE(dropletSize);
    WRITE_DOUBLE(ambientScattering);
    WRITE_DOUBLE(dropletsPerCubicCm);
    WRITE_DOUBLE(albedo);
    WRITE_DOUBLE(extinctionProbability);
    WRITE_DOUBLE(transitionProbability);
    WRITE_DOUBLE(vaporProbability);
    WRITE_DOUBLE(jitter);
    WRITE_DOUBLE(maxSize);
    WRITE_INT(initialEvolve);
    WRITE_INT(voxelWidth);
    WRITE_INT(voxelDepth);
    WRITE_INT(voxelHeight);
    WRITE_DOUBLE(inscatteringTerm);

    cloudMotion.Serialize(s);
    noiseOffset.Serialize(s);
    lightColor.Serialize(s);
    lightWorldPos.Serialize(s);
    lightObjectPos.Serialize(s);
    originTexCoords.Serialize(s);

    if (vb->LockBuffer()) {
        int vbSize = vb->GetNumVertices();
        WRITE_INT(vbSize);
        Vertex *v = vb->GetVertices();
        s.write((char *)v, vbSize * sizeof(Vertex));
        vb->UnlockBuffer();
    } else {
        int vbSize = 0;
        WRITE_INT(vbSize);
    }

    if (ib->LockBuffer()) {
        int ibSize = ib->GetNumIndices();
        WRITE_INT(ibSize);
        Index *i = ib->GetIndices();
        s.write((char *)i, ibSize * sizeof(Index));
        ib->UnlockBuffer();
    } else {
        int ibSize = 0;
        WRITE_INT(ibSize);
    }

    s.write((char *)cloudTexData, voxelWidth * voxelHeight * voxelDepth * 2 * sizeof(unsigned char));
    s.write((char *)noiseTexData, NOISE_DIM * NOISE_DIM * NOISE_DIM * 3 * sizeof(unsigned char));

    return CloudLayer::Serialize(s);
}

bool StratocumulusCloudLayer::Unserialize(const Atmosphere& atm, std::istream& s)
{
#define READ_DOUBLE(X) s.read((char *)&X, sizeof(double))
#define READ_INT(X) s.read((char *)&X, sizeof(int))

    READ_DOUBLE(voxelDimension);
    READ_DOUBLE(lightSamplingDistance);
    READ_DOUBLE(multipleScatteringTerm);
    READ_DOUBLE(dropletSize);
    READ_DOUBLE(ambientScattering);
    READ_DOUBLE(dropletsPerCubicCm);
    READ_DOUBLE(albedo);
    READ_DOUBLE(extinctionProbability);
    READ_DOUBLE(transitionProbability);
    READ_DOUBLE(vaporProbability);
    READ_DOUBLE(jitter);
    READ_DOUBLE(maxSize);
    READ_INT(initialEvolve);
    READ_INT(voxelWidth);
    READ_INT(voxelDepth);
    READ_INT(voxelHeight);
    READ_DOUBLE(inscatteringTerm);

    cloudMotion.Unserialize(s);
    noiseOffset.Unserialize(s);
    lightColor.Unserialize(s);
    lightWorldPos.Unserialize(s);
    lightObjectPos.Unserialize(s);
    originTexCoords.Unserialize(s);

    int numVerts, numIndices;
    READ_INT(numVerts);
    if (numVerts > 0) {
        vb = SL_NEW VertexBuffer(numVerts);
        if (vb->LockBuffer()) {
            Vertex *v = vb->GetVertices();
            s.read((char *)v, numVerts * sizeof(Vertex));
            vb->UnlockBuffer();
        }
    }

    READ_INT(numIndices);
    if (numIndices > 0) {
        ib = SL_NEW IndexBuffer(numIndices);
        if (ib->LockBuffer()) {
            Index *i = ib->GetIndices();
            s.read((char *)i, numIndices * sizeof(Index));
            ib->UnlockBuffer();
        }
    }

    if (cloudTexData) SL_DELETE[] cloudTexData;
    unsigned int cloudTexBytes = voxelWidth * voxelHeight * voxelDepth * 2 * sizeof(unsigned char);
    cloudTexData = SL_NEW unsigned char[cloudTexBytes];
    s.read((char *)cloudTexData, cloudTexBytes);
    Renderer *ren = Renderer::GetInstance();
    if (volumeData) ren->ReleaseTexture(volumeData);
    volumeData = 0;
    ren->Load3DTextureLA(cloudTexData, voxelWidth, voxelDepth, voxelHeight, &volumeData, true, true, false);

    if (noiseTexData) SL_DELETE[] noiseTexData;
    unsigned int noiseTexBytes = NOISE_DIM * NOISE_DIM * NOISE_DIM * 3 * sizeof(unsigned char);
    noiseTexData = SL_NEW unsigned char[noiseTexBytes];
    s.read((char *)noiseTexData, noiseTexBytes);
    if (noiseVolumeData) ren->ReleaseTexture(noiseVolumeData);
    noiseVolumeData = 0;
    ren->Load3DTextureRGB(noiseTexData, NOISE_DIM, NOISE_DIM, NOISE_DIM, &noiseVolumeData, true, true, true);

#ifdef COLOR_RANDOMNESS
    randomData = SL_NEW float[voxelWidth * voxelHeight  * voxelDepth];
    float *r = randomData;

    for (int h = 0; h < voxelHeight; h++) {
        for (int d = 0; d < voxelDepth; d++) {
            for (int w = 0; w < voxelWidth; w++) {
                *r++ = 1.0f - atm.GetRandomNumberGenerator()->UniformRandomFloat() * (float)COLOR_RANDOMNESS;
            }
        }
    }
#endif  // COLOR_RANDOMNESS

    return CloudLayer::Unserialize(atm, s);
}

void StratocumulusCloudLayer::IncrementGrowth()
{
    int i, j, k;

    for (i = 0; i < voxelWidth; i++) {
        for (j = 0; j < voxelDepth; j++) {
            for (k = MIN_HEIGHT; k < voxelHeight; k++) {

                bool fAct = (
                                (i+1 < voxelWidth  ? voxels[i+1][j][k]->states & PHASE_TRANSITION_BIT : false) ||
                                (j+1 < voxelDepth  ? voxels[i][j+1][k]->states & PHASE_TRANSITION_BIT : false) ||
                                (k+1 < voxelHeight ? voxels[i][j][k+1]->states & PHASE_TRANSITION_BIT : false) ||
                                (i-1 >= 0          ? voxels[i-1][j][k]->states & PHASE_TRANSITION_BIT : false) ||
                                (j-1 >= 0          ? voxels[i][j-1][k]->states & PHASE_TRANSITION_BIT : false) ||
                                (k-1 >= 0          ? voxels[i][j][k-1]->states & PHASE_TRANSITION_BIT : false) ||
                                (i-2 >= 0          ? voxels[i-2][j][k]->states & PHASE_TRANSITION_BIT : false) ||
                                (i+2 < voxelWidth  ? voxels[i+2][j][k]->states & PHASE_TRANSITION_BIT : false) ||
                                (j-2 >= 0          ? voxels[i][j-2][k]->states & PHASE_TRANSITION_BIT : false) ||
                                (j+2 < voxelDepth  ? voxels[i][j+2][k]->states & PHASE_TRANSITION_BIT : false) ||
                                (k-2 >= 0          ? voxels[i][j][k-2]->states & PHASE_TRANSITION_BIT : false)
                            );

                bool thisAct = (voxels[i][j][k]->states & PHASE_TRANSITION_BIT) != 0;

                Atmosphere *atm = Atmosphere::GetCurrentAtmosphere();
                double rnd;
                rnd = atm->GetRandomNumberGenerator()->UniformRandomDouble();

                bool phaseTransition = ((!thisAct) && (voxels[i][j][k]->states & VAPOR_BIT) && fAct) ||
                                       (rnd < voxels[i][j][k]->phaseTransitionProbability);
                phaseTransition ? voxels[i][j][k]->states |= PHASE_TRANSITION_BIT : voxels[i][j][k]->states &= ~PHASE_TRANSITION_BIT;

                rnd = atm->GetRandomNumberGenerator()->UniformRandomDouble();

                bool vapor = ((voxels[i][j][k]->states & VAPOR_BIT) && !thisAct) || (rnd < voxels[i][j][k]->vaporProbability);
                vapor ? voxels[i][j][k]->states |= VAPOR_BIT : voxels[i][j][k]->states &= ~VAPOR_BIT;

                rnd = atm->GetRandomNumberGenerator()->UniformRandomDouble();

                bool hasCloud = ( (voxels[i][j][k]->states & HAS_CLOUD_BIT) || thisAct) && (rnd > voxels[i][j][k]->extinctionProbability);
                hasCloud ? voxels[i][j][k]->states |= HAS_CLOUD_BIT : voxels[i][j][k]->states &= ~HAS_CLOUD_BIT;

            }
        }
    }
}

bool StratocumulusCloudLayer::HasPrecipitationAtPosition(double x, double y, double z) const
{
    // Bail if we have no precip at all.
    if (precipitationEffects.size() == 0 || !IsRenderable()) return false;

    // Don't assume which way is up.
    Renderer *ren = Renderer::GetInstance();
    Vector3 camPos = Vector3(x, y, z) * ren->GetInverseBasis3x3();

    // Bail if you're not under the entire layer at all.
    if (camPos.y > GetBaseAltitudeGeocentric()) return false;

    if (!GetIsInfinite()) {
        if (camPos.x < (layerX - GetBaseWidth() * 0.5)) return false;
        if (camPos.x > (layerX + GetBaseWidth() * 0.5)) return false;
        if (camPos.z > (layerZ + GetBaseLength() * 0.5)) return false;
        if (camPos.z < (layerZ - GetBaseLength() * 0.5)) return false;
    }

    return true;
}

bool StratocumulusCloudLayer::IsInsideCloud(double x, double y, double z) const
{
    // Don't assume which way is up.
    Renderer *ren = Renderer::GetInstance();
    Vector3 camPos = Vector3(x, y, z) * ren->GetInverseBasis3x3();

    // Bail if you're not in the entire layer at all.
    if (camPos.y < GetBaseAltitudeGeocentric()) return false;
    if (camPos.y > GetBaseAltitudeGeocentric() + GetThickness()) return false;

    if (!GetIsInfinite()) {
        if (camPos.x < (layerX - GetBaseWidth() * 0.5)) return false;
        if (camPos.x > (layerX + GetBaseWidth() * 0.5)) return false;
        if (camPos.z > (layerZ + GetBaseLength() * 0.5)) return false;
        if (camPos.z < (layerZ - GetBaseLength() * 0.5)) return false;
    }

    return true;
}

#if BILINEAR_LIGHTING
static inline float sample( unsigned char * data, float coord[3], int c[3] )
{
    int z0 = int( floor( coord[2] ) ), z1 = z0 + 1;
    float fz0 = z1 - coord[2], fz1 = coord[2] - z0;

    int y0 = int( floor( coord[1] ) ), y1 = y0 + 1;
    float fy0 = y1 - coord[1], fy1 = coord[1] - y0;

    int x0 = int( floor( coord[0] ) ), x1 = x0 + 1;
    float fx0 = x1 - coord[0], fx1 = coord[0] - x0;

    if( z0 < 0 ) z0 = 0;
    else if( z0 >= c[2] ) z0 = c[2]-1;

    if( z1 < 0 ) z1 = 0;
    else if( z1 >= c[2] ) z1 = c[2]-1;

    z0 *= c[0] * c[1] * 2;
    z1 *= c[0] * c[1] * 2;
    y0 = ( ( y0 + c[1] ) % c[1] ) * c[0] * 2;
    y1 = ( ( y1 + c[1] ) % c[1] ) * c[0] * 2;
    x0 = ( ( x0 + c[0] ) % c[0] ) * 2;
    x1 = ( ( x1 + c[0] ) % c[0] ) * 2;

    return data[ z0 + y0 + x0 ] * fx0 * fy0 * fz0 +
           data[ z1 + y0 + x0 ] * fx0 * fy0 * fz1 +
           data[ z0 + y1 + x0 ] * fx0 * fy1 * fz0 +
           data[ z1 + y1 + x0 ] * fx0 * fy1 * fz1 +
           data[ z0 + y0 + x1 ] * fx1 * fy0 * fz0 +
           data[ z1 + y0 + x1 ] * fx1 * fy0 * fz1 +
           data[ z0 + y1 + x1 ] * fx1 * fy1 * fz0 +
           data[ z1 + y1 + x1 ] * fx1 * fy1 * fz1;
}
#else // NEAREST_LIGHTING
static inline float sample( unsigned char * data, float coord[3], int c[3] )
{
    int z = int( floor( coord[2] + 0.5 ) );
    int y = int( floor( coord[1] + 0.5 ) );
    int x = int( floor( coord[0] + 0.5 ) );
    if( z < 0 ) z = 0;
    else if( z >= c[2] ) z = c[2]-1;
    z *= c[0] * c[1] * 2;
    y = ( ( y + c[1] ) % c[1] ) * c[0] * 2;
    x = ( ( x + c[0] ) % c[0] ) * 2;

    return data[ z + y + x ];
}
#endif

static inline float randomSample(float *data, float coord[3], int c[3] )
{
    int z = int( floor( coord[2] + 0.5 ) );
    int y = int( floor( coord[1] + 0.5 ) );
    int x = int( floor( coord[0] + 0.5 ) );
    if( z < 0 ) z = 0;
    else if( z >= c[2] ) z = c[2]-1;
    z *= c[0] * c[1];
    y = ( ( y + c[1] ) % c[1] ) * c[0];
    x = ( ( x + c[0] ) % c[0] );

    return data[ z + y + x ];
}

int StratocumulusCloudLayer::ComputeVolumeLighting( int & updateSliceMin, int & updateSliceMax )
{
    updateSliceMin = updateSliceMax = 0;
    // Lot of 3 element vectors.  We juggle these elements with varying indices.
    int   c[3] = { voxelWidth, voxelDepth, voxelHeight };

    Vector3 lightVector = lightObjectPos;

    if( lightVector.Length() > 0 )
        lightVector.Normalize();
    else
        lightVector = Vector3( 0,0,1 );

    double change  = volumeLightingVector.Dot( lightVector );

    if (invalid) {
        change = 0;
        invalid = false;
    }

    if( change > volumeLightingVectorSliceRefreshTolerance && volumeLightingSlice == 0 )
        return 0; // nothing to do - lighting is up to date

    int slice = 0, sliceCount = c[2];

    if( change > volumeLightingVectorFullRefreshTolerance ) {
        if( volumeLightingSlice == 0 )
            volumeLightingVector = lightVector; // record light vector for consecutive slices

        slice = volumeLightingSlice;
        sliceCount = 1;
        volumeLightingSlice++;

        if( volumeLightingSlice >= c[2] )
            volumeLightingSlice = 0; // mark it ready for new iteration loop
    } else {
        volumeLightingVector = lightVector;
        volumeLightingSlice = 0;
    }

    float l[3] = { float( volumeLightingVector.x ), float( volumeLightingVector.z ), float( volumeLightingVector.y ) };
    float labs[3] = { fabsf(l[0]), fabsf(l[1]), fabsf(l[2]) };

    int   i0 = labs[1] < labs[0] ? 1 : 0, // primary coord from x,y is the one with larger absolute
          i1 = labs[1] < labs[0] ? 0 : 1; // secondary coord from x,y is the one with smaller absolute

    int imax = labs[2] > labs[i1] ? 2 : i1; // imax is an index of max absolute coord
    //int imin = labs[2] < labs[i0] ? 2 : i0; // imin is an index of min absolute coord
    //int imid = imax == 2 ? i1 : ( imin == 2 ? i0 : 2 ); // imid is abs median coordinate index

    l[0] /= labs[imax]; // compute constant offset to prior samples overshadowing (or not) computed sample
    l[1] /= labs[imax];
    l[2] /= labs[imax];

    double volumeLightSamplingDistance = lightObjectPos.Length() * voxelDimension / labs[imax];
    double extinctionCrossSection = PI * dropletSize * dropletSize;
    double extinctionCoefficient = extinctionCrossSection * (dropletsPerCubicCm * 100 * 100 * 100);

    double opticalDepth = extinctionCoefficient * volumeLightSamplingDistance;

    // Harris model uses following value for constTerm:
    // albedo * opticalDepth * phaseFunction( -lightVector, lightVector ) * steradians_area_of_inscattering_integration  / (4.0 * PI)
    // above model does not work well here so instead we use constant inscatteringTerm to replace all factors except albedo

    float constTerm = float( albedo * inscatteringTerm );
    float extinction = constTerm + expf(-(float)opticalDepth);

    int s[3] = { l[0] < 0 ? -1 : 1, // increments/decrements for updating the samples in right direction
                 l[1] < 0 ? -1 : 1, // so that we accumulate light values from previously computed ones
                 l[2] < 0 ? -1 : 1
               }; // direction is as important as direction of mem move in overlapping regions

    int d[3] = { 2,  2 * c[0], 2 * c[0] * c[1] }; // memory deltas for one unit steps along each coord

    int o[3] = { l[0] < 0 ? c[0]-1-slice : slice, // origin - starting position for iterations
                 l[1] < 0 ? c[1]-1-slice : slice,
                 l[2] < 0 ? c[2]-1-slice : slice
               };

    if( sliceCount == 1 ) {
        updateSliceMin = o[2];
        updateSliceMax = o[2]+1;
    } else {
        updateSliceMin = 0;
        updateSliceMax = c[2];
    }

    float p[3];

    unsigned char * pb = cloudTexData + o[2] * d[2] + o[1] * d[1] + o[0] * d[0];
    unsigned char * cloudTexDataLast = &(cloudTexData[2 * voxelWidth * voxelHeight * voxelDepth - 1]);

    int d0 = d[i0]*s[i0];
    int d1 = d[i1]*s[i1] - c[i0]*d0;
    int d2 = d[2]*s[2] - c[i1]*d[i1]*s[i1];

    for( p[2] = o[2] - l[2]; sliceCount-- > 0; pb += d2, p[2] += s[2] ) {
        p[i1] = o[i1] - l[i1];
        for( int c1 = c[i1]; c1-- > 0; pb += d1, p[i1] += s[i1] ) {
            p[i0] = o[i0] - l[i0];
            for( int c0 = c[i0]; c0-- > 0; pb += d0, p[i0] += s[i0] ) {
                if (pb < cloudTexData) {
                    if (d0 <= 0.f) {
                        pb += c0 * d0;
                        break;
                    }
                } else if (pb >= cloudTexDataLast) {
                    if (d0 >= 0.f) {
                        pb += c0 * d0;
                        break;
                    }
                } else {
                    pb[1] = 0xFF;
                    float src = sample( cloudTexData, p, c );

                    double r = 1.0;
#ifdef COLOR_RANDOMNESS
                    r = (double)randomSample(randomData, p, c);
#endif
                    double scale = ( pb[0] == 255 ? extinction : 1. ) * r;
                    pb[1] = static_cast<unsigned char>( scale * src );
                }
            }
        }
    }

    return updateSliceMax - updateSliceMin;
}

ShaderHandle StratocumulusCloudLayer::GetShader() const
{
    return Atmosphere::GetHDREnabled() ? hdrShader : shader;
}

void StratocumulusCloudLayer::ReloadShaders()
{
    Renderer *ren = Renderer::GetInstance();
    if (ren) {
        ren->DeleteShader(shader);
        ren->DeleteShader(hdrShader);

        shader = ren->LoadShaderFromFile("Shaders/Stratocumulus.cg", VERTEX_PROGRAM);
        hdrShader = ren->LoadShaderFromFile("Shaders/StratocumulusHDR.cg", VERTEX_PROGRAM);
    }
}
