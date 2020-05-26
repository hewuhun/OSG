// Copyright (c) 2004-2014  Sundog Software, LLC. All rights reserved worldwide.

#include <FeSilverliningLib/CumulusCloudLayer.h>
#include <FeSilverliningLib/ResourceLoader.h>
#include <FeSilverliningLib/CumulusCloud.h>
#include <FeSilverliningLib/Configuration.h>
#include <FeSilverliningLib/CloudGenerator.h>
#include <FeSilverliningLib/CloudDistributor.h>
#include <FeSilverliningLib/Renderer.h>
#include <FeSilverliningLib/Billboard.h>
#include <algorithm>
#include <FeSilverliningLib/CloudGeneratorFactory.h>
#include <FeSilverliningLib/CloudDistributorFactory.h>
#include <FeSilverliningLib/Utils.h>
#include <sstream>

using namespace SilverLining;
using namespace std;

CumulusCloudLayer::CumulusCloudLayer() : CloudLayer(),  generator(0), distributor(0), makeShadowMaps(false), shadowTex(0),
    offscreen(0), restoreBackBuffer(false), savedBackBuffer(0), spinRate(0.1), maxHeight(0), coverageMap(0)
{
    shadowMapPos = Vector3(0,0,0);

    quickLighting = false;
    Configuration::GetBoolValue("cumulus-lighting-quick-and-dirty", quickLighting);

    renderLightingPassOffscreen = false;
    Configuration::GetBoolValue("render-offscreen", renderLightingPassOffscreen);

    if (Renderer::GetInstance()->GetType() >= Atmosphere::DIRECTX10) {
        renderLightingPassOffscreen = true;
    }

    if (quickLighting) {
        renderLightingPassOffscreen = false;
    }
}

CumulusCloudLayer::~CumulusCloudLayer()
{
    if (generator) {
        SL_DELETE generator;
    }

    if (distributor) {
        SL_DELETE distributor;
    }

    if (shadowTex) {
        Renderer::GetInstance()->ReleaseTexture(shadowTex);
        shadowTex = 0;
    }

    if (offscreen) {
        Renderer::GetInstance()->ReleaseRenderTarget(offscreen);
        offscreen = 0;
    }

    if (savedBackBuffer) {
        SL_DELETE[] savedBackBuffer;
    }
}

void CumulusCloudLayer::InitCoverageMap()
{
    coverageW = (int)(CloudLayer::GetBaseWidth() / parameters.voxelSize);
    coverageL = (int)(CloudLayer::GetBaseLength() / parameters.voxelSize);
    totalCoverageCells = coverageW * coverageL;
    cellsCovered = 0;

    if (coverageMap) SL_DELETE[] coverageMap;
    coverageMap = SL_NEW bool[totalCoverageCells];
    for (int i = 0; i < totalCoverageCells; i++) {
        coverageMap[i] = false;
    }
}

void CumulusCloudLayer::DeleteCoverageMap()
{
    if (coverageMap) SL_DELETE[] coverageMap;
    coverageMap = 0;
}

void CumulusCloudLayer::AddToCoverage(Cloud *cloud)
{
    Vector3 pos = cloud->GetWorldPosition() * Renderer::GetInstance()->GetInverseBasis3x3();
    double layerX, layerZ;
    CloudLayer::GetLayerPosition(layerX, layerZ);
    double x = pos.x - (layerX - CloudLayer::GetBaseWidth() * 0.5);
    double z = pos.z - (layerZ - CloudLayer::GetBaseLength() * 0.5);

    double cloudWidth, cloudDepth, cloudHeight;
    cloud->GetSize(cloudWidth, cloudDepth, cloudHeight);
    int cellsWide = (int)(cloudWidth / parameters.voxelSize);
    int cellsDeep = (int)(cloudDepth / parameters.voxelSize);

    int cellX = (int)(x / parameters.voxelSize);
    int cellZ = (int)(z / parameters.voxelSize);

    for (int xx = cellX - cellsWide / 2; xx < cellX + cellsWide / 2; xx++) {
        for (int z = cellZ - cellsDeep / 2; z < cellZ + cellsDeep / 2; z++) {
            int idx = xx + z * coverageW;
            if (idx >= 0 && idx < totalCoverageCells) {
                if (!coverageMap[idx]) {
                    cellsCovered++;
                }
                coverageMap[idx] = true;
            }
        }
    }

}

float CumulusCloudLayer::GetCoverage()
{
    return (float)cellsCovered / (float)totalCoverageCells;
}

void CumulusCloudLayer::SetCloudAnimationEffects(double pSpinRate, bool pGrowth, int pIterations, int pTimeStepInterval)
{
    spinRate = pSpinRate;
    growth = pGrowth;

    if (pIterations > 0) {
        parameters.initialEvolveSteps = pIterations;
    }

    if (pTimeStepInterval > 0) {
        parameters.timeStepInterval = pTimeStepInterval;
    }
}

bool CumulusCloudLayer::SupportsShadowMaps()
{
    return (!renderLightingPassOffscreen);
}

void CumulusCloudLayer::SetupSurfaces()
{
    if (renderLightingPassOffscreen) {
        int offscreenDim = 512;
        Configuration::GetIntValue("offscreen-dimension", offscreenDim);
        renderLightingPassOffscreen = Renderer::GetInstance()->InitRenderTarget(offscreenDim, offscreenDim,
                                      &offscreen);
        shadowTexDim = offscreenDim;
    }

    if (!renderLightingPassOffscreen) {
        int shadowMapDim = 512;
        Configuration::GetIntValue("shadow-map-dimension", shadowMapDim);
        Renderer::GetInstance()->CreateLuminanceTexture(shadowMapDim, shadowMapDim, &shadowTex);
        shadowTexDim = shadowMapDim;
    }
}

bool SILVERLINING_API CumulusCloudLayer::AddCloudAt(const Atmosphere& atm, const Vector3& relativePosition, const Vector3& dimensions)
{
    if (!atm.IsInitialized()) return false;

    if (fabs(relativePosition.x) > GetBaseWidth() * 0.5) return false;
    if (fabs(relativePosition.z) > GetBaseLength() * 0.5) return false;
    if (relativePosition.y < 0) return false;

    if (dimensions.x < parameters.voxelSize || dimensions.y < parameters.voxelSize ||
            dimensions.z < parameters.voxelSize) return false;

    parameters.width = dimensions.x;
    parameters.depth = dimensions.z;
    parameters.height = dimensions.y;
    parameters.spinRate = GetSpinRate();

    CumulusCloud *cloud = CreateCloud(this);
    cloud->Init(parameters);

    double velocity, heading;
    atm.GetConditions().GetWind(velocity, heading, GetBaseAltitude());
    double dx = localWindX + sin(RADIANS(heading)) * velocity;
    double dz = localWindZ + cos(RADIANS(heading)) * velocity;
    cloud->ApplyWindShear(Vector3(dx, 0, dz) * Renderer::GetInstance()->GetBasis3x3());

    double layerX, layerZ;
    double baseAlt;
    GetLayerPosition(layerX, layerZ);
    baseAlt = GetBaseAltitude();

    Vector3 center(layerX, baseAlt, layerZ);

    Vector3 cloudPos = relativePosition + center;
    Vector3 layerPos = relativePosition;

    cloud->SetNeedsGeocentricPlacement(true);
    cloud->SetWorldPosition(cloudPos * Renderer::GetInstance()->GetBasis3x3());
    cloud->SetLayerPosition(layerPos);

    AddCloud(cloud);

    for (int i = 0; i < parameters.initialEvolveSteps; i++) {
        cloud->IncrementTimeStep(0);
    }

    cloud->CreateImposter();

    cloud->CloudPlaced(atm);

    return true;
}

void CumulusCloudLayer::CreateCloud(const Atmosphere& atm, double cloudWidth, double cloudDepth, double cloudHeight)
{
    if (cloudWidth > parameters.voxelSize && cloudDepth > parameters.voxelSize
            && cloudHeight > parameters.voxelSize) {
        if (!CreateCloudFromFile(atm)) {
            parameters.width = cloudWidth;
            parameters.depth = cloudDepth;
            parameters.height = cloudHeight;
            parameters.spinRate = GetSpinRate();
            CumulusCloud *cloud = CreateCloud(this);
            cloud->Init(parameters);

            double worldWidth, worldDepth, worldHeight;
            cloud->GetSize(worldWidth, worldDepth, worldHeight);
            if (worldHeight > maxHeight) maxHeight = worldHeight;

            double velocity, heading;
            atm.GetConditions().GetWind(velocity, heading, GetBaseAltitude());
            double dx = localWindX + sin(RADIANS(heading)) * velocity;
            double dz = localWindZ + cos(RADIANS(heading)) * velocity;
            cloud->ApplyWindShear(Vector3(dx, 0, dz) * Renderer::GetInstance()->GetBasis3x3());

            if (distributor->PlaceCloud(cloud, parameters.minimumDistanceScale)) {
                AddCloud(cloud);

                AddToCoverage(cloud);

                for (int i = 0; i < parameters.initialEvolveSteps; i++) {
                    cloud->IncrementTimeStep(0);
                }

                cloud->CreateImposter();

                cloud->CloudPlaced(atm);
            } else {
                SL_DELETE cloud;
            }
        }
    }

}

bool CumulusCloudLayer::SeedClouds(const Atmosphere& atm)
{
    if (!atm.IsInitialized()) return false;

    ClearClouds();

    ScanCloudFiles(atm);

    SetupSurfaces();

    if (generator && distributor) {
        generator->SetDesiredArea(GetBaseWidth() * GetBaseLength());
        generator->SetDesiredCoverage(GetDensity());

        double layerX, layerZ;
        double width, depth;
        double baseAlt, thickness;
        GetLayerPosition(layerX, layerZ);
        baseAlt = GetBaseAltitude();
        thickness = GetThickness();
        width = GetBaseWidth();
        depth = GetBaseLength();

        Vector3 minCorner(layerX - (width * 0.5), baseAlt, layerZ - (depth * 0.5));
        Vector3 maxCorner(layerX + (width * 0.5), baseAlt + thickness, layerZ + (depth * 0.5));

        distributor->Init(minCorner, maxCorner, parameters.voxelSize);

        generator->StartGeneration();

        double cloudWidth, cloudDepth, cloudHeight;

        maxHeight = 0;

        InitCoverageMap();

        while (generator->GetNextCloud(cloudWidth, cloudDepth, cloudHeight)) {
            CreateCloud(atm, cloudWidth, cloudDepth, cloudHeight);
        }

        int maxExtraRuns = 1;
        Configuration::GetIntValue("cumulus-max-extra-generation-runs", maxExtraRuns);
        double densityPrecision = 0.05;
        Configuration::GetDoubleValue("cumulus-density-precision", densityPrecision);

        int extraRuns = 0;
        while (GetCoverage() < GetDensity() * (1.0 - densityPrecision)) {
            extraRuns++;
            if (extraRuns > maxExtraRuns) {
                break;
            }

            generator->StartGeneration();
            while (generator->GetNextCloud(cloudWidth, cloudDepth, cloudHeight)) {
                CreateCloud(atm, cloudWidth, cloudDepth, cloudHeight);
                if (GetCoverage() >= GetDensity()) {
                    break;
                }
            }
        }

        DeleteCoverageMap();
    }

    return true;
}

void CumulusCloudLayer::ScanCloudFiles(const Atmosphere& atm)
{
    SL_STRING cloudPath("Clouds/");
    cloudPath += cloudTypeStr;
    cloudPath += "/";

    SL_VECTOR(SL_STRING) cloudFileNames;
    Atmosphere::GetResourceLoader()->GetFilesInDirectory(cloudPath.c_str(), cloudFileNames);

    cloudFiles.clear();
    SL_VECTOR(SL_STRING) ::iterator it;
    for (it = cloudFileNames.begin(); it != cloudFileNames.end(); it++) {
        cloudFiles.push_back(cloudPath + (*it));
    }
}

bool CumulusCloudLayer::CreateCloudFromFile(const Atmosphere& atm, double scaleTo)
{
    bool success = false;

    int nClouds = (int)cloudFiles.size();
    if (nClouds > 0) {
        int idx = Atmosphere::GetCurrentAtmosphere()->GetRandomNumberGenerator()->UniformRandomIntRange(0, nClouds - 1);
        SL_STRING fname = cloudFiles[idx];

        char *data;
        unsigned int dataLen;
        if (Atmosphere::GetResourceLoader()->LoadResource(fname.c_str(), data, dataLen, true)) {
            std::stringstream *stream = SL_NEW std::stringstream(std::string(data));
            CumulusCloud *cloud = CreateCloud(this);
            if (cloud->InitFromFile(parameters, stream)) {
                if (scaleTo > 0) {
                    double width, depth, height;
                    cloud->GetSize(width, depth, height);
                    double s = scaleTo / width;
                    stream->clear();
                    stream->seekg(0, std::ios_base::beg);
                    cloud->InitFromFile(parameters, stream, s);
                    cloud->GetSize(width, depth, height);
                    maxHeight = height;
                }

                if (distributor) {
                    distributor->PlaceCloud(cloud, parameters.minimumDistanceScale);
                } else {
                    double x, z;
                    GetLayerPosition(x, z);
                    Vector3 pos(x, GetBaseAltitude(), z);
                    pos = pos * Renderer::GetInstance()->GetBasis3x3();
                    cloud->SetWorldPosition(pos);
                }

                AddCloud(cloud);
                cloud->CreateImposter();
                cloud->CloudPlaced(atm);
                success = true;
            } else {
                SL_DELETE cloud;
            }

            SL_DELETE stream;
            Atmosphere::GetResourceLoader()->FreeResource(data);
        }
    }

    return success;
}

bool CumulusCloudLayer::DrawSetup(int pass, const Vector3 *lightPos, const Color *lightColor)
{
    Renderer *renderer = Renderer::GetInstance();

    if (pass == 0) {
        Configuration::GetBoolValue("cumulus-lighting-quick-and-dirty", quickLighting);

        if (!quickLighting) {
            renderer->GetViewport(vpx, vpy, vpw, vph);

            if (!renderLightingPassOffscreen) {
                // Set viewport to something texture-map-friendly
                Configuration::GetBoolValue("preserve-back-buffer", restoreBackBuffer);

                if (restoreBackBuffer) {
                    if (savedBackBuffer != 0) SL_DELETE[] savedBackBuffer;
                    savedBackBuffer = SL_NEW char[vpw * vph * 4];
                    renderer->GetPixels(vpx, vpy, vpw, vph, savedBackBuffer, true);
                }

                if (makeShadowMaps) {
                    int minDim = vpw < vph ? vpw : vph;

                    int maxShadowTexDim = 1 << (int)(floor(log((double)minDim)/log(2.0)));
                    int targetShadowMapDim = 512;
                    Configuration::GetIntValue("shadow-map-dimension", targetShadowMapDim);
                    if (maxShadowTexDim > targetShadowMapDim) {
                        maxShadowTexDim = targetShadowMapDim;
                    }

                    if (maxShadowTexDim != shadowTexDim) {
                        if (shadowTex)
                            Renderer::GetInstance()->ReleaseTexture(shadowTex);

                        shadowTex = 0;

                        Renderer::GetInstance()->CreateLuminanceTexture(maxShadowTexDim, maxShadowTexDim, &shadowTex);
                    }

                    shadowTexDim = maxShadowTexDim;

                    renderer->SetViewport(vpx, vpy, shadowTexDim, shadowTexDim);
                }
            } else {
                renderer->MakeRenderTargetCurrent(offscreen);
                renderer->SetViewport(0, 0, shadowTexDim, shadowTexDim);
            }

            // Set parallel projection
            Vector3 topCenter(layerX, geocentricAltitude + thickness, layerZ);
            topCenter = topCenter * renderer->GetBasis3x3();
            double deckRadius = sqrt(GetBaseWidth() * GetBaseWidth() + GetBaseLength() * GetBaseLength()) * 0.5;

            renderer->PushTransforms();
            renderer->SetModelviewLookat(*lightPos, topCenter, renderer->GetUpVector());
            renderer->SetOrthoMatrix(-deckRadius, deckRadius, -deckRadius, deckRadius, 1, deckRadius * 4.0);

            // Store off proj * view
            Matrix4 lightProj, lightMV;
            renderer->GetProjectionMatrix(&lightProj);
            renderer->GetModelviewMatrix(&lightMV);
            lightProjView = lightProj * lightMV;

            // hack the billboard matrix
            Matrix4 modelview, billboard;
            renderer->GetModelviewMatrix(&modelview);

            for (int i = 0; i < 3; i++) {
                for (int j = 0; j < 3; j++) {
                    billboard.elem[i][j] = modelview.elem[j][i];
                }
            }
            Billboard::OverrideBillboardMatrix(billboard);

            // Clear screen
            renderer->ClearScreen(Color(1, 1, 1, 1));

            renderer->EnableDepthReads(true);
            renderer->EnableDepthWrites(false);
            renderer->EnableBlending(ZERO, INVSRCALPHA);
            renderer->EnableTexture2D(true);
            renderer->EnableLighting(false);
        }
    } else {
        // Set the blend and z buffer modes
        renderer->EnableBlending(ONE, INVSRCALPHA);
        renderer->EnableDepthReads(true);
        renderer->EnableDepthWrites(false);
        renderer->EnableTexture2D(true);
        renderer->EnableLighting(false);
    }

    return true;
}

bool CumulusCloudLayer::EndDraw(int pass)
{
    Renderer *renderer = Renderer::GetInstance();
    if (pass == 0) {

        Configuration::GetBoolValue("cumulus-lighting-quick-and-dirty", quickLighting);

        if (!quickLighting) {
            if (makeShadowMaps && shadowTex) {
                // preserve a 1 pixel wide white border to prevent clamping
                unsigned char *buf = SL_NEW unsigned char[shadowTexDim * shadowTexDim * 4];
                if (buf) {
                    renderer->CopyLuminanceFromScreen(vpx, vpy, shadowTexDim, shadowTexDim, buf);

                    static double darkness = 0.5;
                    static bool first = true;
                    if ( first ) {
                        first = false;
                        Configuration::GetDoubleValue("shadow-map-darkness", darkness);
                    }

                    int j = 0;
                    int i = 0;

                    for (int row = 0; row < shadowTexDim; row++) {
                        for (int col = 0; col < shadowTexDim; col++) {
                            unsigned char s;

                            if (row == 0 || row == (shadowTexDim - 1) ||
                                    col == 0 || col == (shadowTexDim - 1)) {
                                s = 255;
                            } else {
                                int startIdx = i << 2;
                                if (buf[startIdx] < 255 || buf[startIdx+1] < 255 || buf[startIdx+2] < 255) {
                                    s = (unsigned char)(darkness * 255.0);
                                } else {
                                    s = 255;
                                }
                            }

                            buf[j++] = s;
                            buf[j++] = s;
                            buf[j++] = s;
                            buf[j++] = 255;

                            i++;
                        }
                    }

                    renderer->CopyLuminanceIntoTexture(shadowTex, shadowTexDim, shadowTexDim, buf);

                    SL_DELETE[] buf;

                    GetLayerPosition(shadowMapPos.x, shadowMapPos.z);
                    shadowMapElapsedWind = Vector3(0, 0, 0);
                }
            }
            renderer->SetViewport(vpx, vpy, vpw, vph);

            Billboard::RestoreBillboardMatrix();
            renderer->EnableDepthReads(true);
            //renderer->EnableDepthWrites(true);
            renderer->PopTransforms();
            renderer->DisableBlending();
            renderer->EnableTexture2D(false);

            if (renderLightingPassOffscreen) {
                renderer->RestoreRenderTarget(offscreen);
            } else {
                if (restoreBackBuffer && savedBackBuffer) {
                    renderer->SetPixels(vpx, vpy, vpw, vph, savedBackBuffer);
                    SL_DELETE[] savedBackBuffer;
                    savedBackBuffer = 0;
                }
            }
        }
    } else {
        //renderer->EnableDepthWrites(true);
    }

    return true;
}

void CumulusCloudLayer::ReadConfiguration(const SL_STRING& configPrefix)
{
    cloudTypeStr = configPrefix;

    Configuration::GetDoubleValue((configPrefix + "-voxel-dimension").c_str(),
                                  parameters.voxelSize);
    parameters.voxelSize *= Atmosphere::GetUnitScale();

    Configuration::GetDoubleValue((configPrefix + "-extinction-probability").c_str(),
                                  parameters.extinctionProbability);
    Configuration::GetDoubleValue((configPrefix + "-transition-probability").c_str(),
                                  parameters.transitionProbability);
    Configuration::GetDoubleValue((configPrefix + "-vapor-probability").c_str(),
                                  parameters.vaporProbability);
    Configuration::GetDoubleValue((configPrefix + "-initial-vapor-probability").c_str(),
                                  parameters.initialVaporProbability);
    Configuration::GetIntValue((configPrefix + "-initial-evolve").c_str(),
                               parameters.initialEvolveSteps);
    Configuration::GetIntValue((configPrefix + "-time-step-interval").c_str(),
                               parameters.timeStepInterval);
    Configuration::GetDoubleValue((configPrefix + "-droplet-size").c_str(),
                                  parameters.dropletSize);
    parameters.dropletSize *= Atmosphere::GetUnitScale();

    Configuration::GetDoubleValue((configPrefix + "-water-content").c_str(),
                                  parameters.waterContent);
    parameters.waterContent /= pow(Atmosphere::GetUnitScale(), 3.0);

    Configuration::GetDoubleValue((configPrefix + "-albedo").c_str(),
                                  parameters.albedo);

    Configuration::GetDoubleValue((configPrefix + "-droplets-per-cubic-cm").c_str(),
                                  parameters.dropletsPerCubicCm);
    parameters.dropletsPerCubicCm /= pow(Atmosphere::GetUnitScale(), 3.0);

    parameters.ambientScattering = 0;
    Configuration::GetFloatValue((configPrefix + "-ambient-scattering").c_str(),
                                 parameters.ambientScattering);

    parameters.attenuation = 0;
    Configuration::GetDoubleValue((configPrefix + "-attenuation").c_str(),
                                  parameters.attenuation);
    parameters.attenuation *= Atmosphere::GetUnitScale();

    parameters.minimumDistanceScale = 1.0;
    Configuration::GetDoubleValue((configPrefix + "-minimum-cloud-distance-scale").c_str(),
                                  parameters.minimumDistanceScale);

    parameters.verticalGradient = 0;
    Configuration::GetDoubleValue((configPrefix + "-vertical-gradient").c_str(),
                                  parameters.verticalGradient);

    generator = CloudGeneratorFactory::Create(configPrefix);
    distributor = CloudDistributorFactory::Create(configPrefix);
}

void CumulusCloudLayer::GenerateShadowMaps(bool enable)
{
    makeShadowMaps = enable;
}

void CumulusCloudLayer::MoveClouds(double x, double y, double z)
{
    CloudLayer::MoveClouds(x, y, z);
    shadowMapElapsedWind = shadowMapElapsedWind + Vector3(x, y, z);
}

bool CumulusCloudLayer::BindShadowMap(int textureStage, double *m) const
{
    if (!shadowTex) return false;
    Renderer *ren = Renderer::GetInstance();

    ren->EnableTexture(shadowTex, textureStage);

    Vector3 currentPos(0,0,0);
    GetLayerPosition(currentPos.x, currentPos.z);

    Matrix4 lightTrans;
    if (!GetIsInfinite()) {
        lightTrans.elem[0][3] = shadowMapPos.x - currentPos.x;
        lightTrans.elem[2][3] = shadowMapPos.z - currentPos.z;
    } else {
        lightTrans.elem[0][3] = -shadowMapElapsedWind.x;
        lightTrans.elem[2][3] = -shadowMapElapsedWind.z;
    }

    Matrix4 lightProjViewTrans = lightProjView * lightTrans;

    for (int row = 0; row < 4; row++) {
        for (int col = 0; col < 4; col++) {
            *m++ = lightProjViewTrans.elem[row][col];
        }
    }

    return true;
}

bool CumulusCloudLayer::SaveClouds(std::ostream& s) const
{
    s.write((char *)&parameters, sizeof(CumulusCloudInit));
    s.write((char *)&makeShadowMaps, sizeof(bool));

    s.write((char *)&spinRate, sizeof(double));
    s.write((char *)&growth, sizeof(bool));

    return CloudLayer::SaveClouds(s);
}

bool CumulusCloudLayer::RestoreClouds(const Atmosphere& atm, std::istream& s)
{
    s.read((char *)&parameters, sizeof(CumulusCloudInit));
    s.read((char *)&makeShadowMaps, sizeof(bool));

    s.read((char *)&spinRate, sizeof(double));
    s.read((char *)&growth, sizeof(bool));

    int nClouds;
    s.read((char *)&nClouds, sizeof(int));

    SetupSurfaces();

    for (int i = 0; i < nClouds; i++) {
        CumulusCloud *cloud = CreateCloud(this);
        if (cloud->Unserialize(s)) {
            AddCloud(cloud);

            double width, depth, height;
            cloud->GetSize(width, depth, height);
            if (height > maxHeight) {
                maxHeight = height;
            }

            cloud->CreateImposter();
            cloud->CloudPlaced(atm);
        } else {
            SL_DELETE cloud;
        }
    }

    return true;
}

