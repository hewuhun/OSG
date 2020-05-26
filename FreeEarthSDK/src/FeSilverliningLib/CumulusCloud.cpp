// Copyright (c) 2004-2014  Sundog Software, LLC. All rights reserved worldwide.

#include <FeSilverliningLib/CumulusCloud.h>
#include <FeSilverliningLib/CumulusCloudLayer.h>
#include <FeSilverliningLib/Configuration.h>
#include <FeSilverliningLib/Voxel.h>
#include <FeSilverliningLib/Renderer.h>
#include <FeSilverliningLib/Billboard.h>
#include <FeSilverliningLib/Metaball.h>
#include <FeSilverliningLib/Utils.h>
#include <FeSilverliningLib/CloudImposter.h>
#include <FeSilverliningLib/Sky.h>
#include <algorithm>
#include <assert.h>
#include <sstream>
#include <locale>

using namespace SilverLining;
using namespace std;

int CumulusCloud::recompiles = 0;

bool CumulusCloud::quickLighting = false;

bool CumulusCloud::noUpdate = true;

bool CumulusCloud::cullInteriorVoxels = false;

double CumulusCloud::quickLightingAttenuation = 2000.0;

double CumulusCloud::quickLightingAttenuationHiRes = 3000.0;

int CumulusCloud::recompileBudget = 1;

double CumulusCloud::sortAngleThreshold = cos(RADIANS(10.0));

double CumulusCloud::alphaCullThreshold = 0.1;

double CumulusCloud::colorRandomness = 0.2;

double CumulusCloud::colorRandomnessHiRes = 0.4;

double CumulusCloud::weights[CUMULUS_SMOOTH_DIM][CUMULUS_SMOOTH_DIM][CUMULUS_SMOOTH_DIM];

bool CumulusCloud::darkenWithDensity = true;

double CumulusCloud::darkenAmount = 0.1;

#define ELLIPSOID_BOUNDS 0
#define CULL_FADED_CLOUDS

CumulusCloud::CumulusCloud(CloudLayer *layer) : Cloud(layer), width(0), height(0), depth(0), lastTimeStep(0), timeStepInterval(0),
    voxels(0), compiledObject(0), fadeStartTime(0)
{
}

void CumulusCloud::Shear(double updraftSpeed, const Vector3& wind)
{
    Vector3 minBounds(1.0E6, 1.0E6, 1.0E6);
    Vector3 maxBounds(-1.0E6, -1.0E6, -1.0E6);

    double cloudHeight = (height - 1) * voxelSize;

    double minUpdraftSpeed = (wind.Length() * cloudHeight) / (width * voxelSize);
    if (updraftSpeed < minUpdraftSpeed) {
        updraftSpeed = minUpdraftSpeed;
    }

    double t = cloudHeight / updraftSpeed;

    Vector3 shearCenterOffset = ( wind * t * 0.5 ) * Renderer::GetInstance()->GetInverseBasis3x3();

    for (int k = 0; k < height; k++) {
        double dY = k * voxelSize;
        double t = dY / updraftSpeed;

        Vector3 shearOffset = ( wind * t ) * Renderer::GetInstance()->GetInverseBasis3x3() - shearCenterOffset;

        for (int i = 0; i < width; i++) {
            for (int j = 0; j < depth; j++) {
                Vector3f wpf = voxels[i][j][k]->GetWorldPosition();
                Vector3 wp(wpf.x, wpf.y, wpf.z);
                wp = wp + shearOffset;

                voxels[i][j][k]->SetWorldPosition(wp);

                if (wp.x < minBounds.x) minBounds.x = wp.x;
                if (wp.y < minBounds.y) minBounds.y = wp.y;
                if (wp.z < minBounds.z) minBounds.z = wp.z;
                if (wp.x > maxBounds.x) maxBounds.x = wp.x;
                if (wp.y > maxBounds.y) maxBounds.y = wp.y;
                if (wp.z > maxBounds.z) maxBounds.z = wp.z;
            }
        }
    }

    //SetWorldPosition( GetWorldPosition() + shearCenterOffset );

    double voxelRadius = Voxel::ComputeVoxelRadius(voxelSize);
    bounds.x = (maxBounds.x - minBounds.x) + voxelRadius * 2.0;
    bounds.y = (maxBounds.y - minBounds.y) + voxelRadius * 2.0;
    bounds.z = (maxBounds.z - minBounds.z) + voxelRadius * 2.0;
}

void CumulusCloud::ReadConfig()
{
    if (Configuration::GetDoubleValue("cumulus-sorting-threshold-angle", sortAngleThreshold)) {
        sortAngleThreshold = cos(RADIANS(sortAngleThreshold));
    }

    Configuration::GetIntValue("cumulus-recompile-budget", recompileBudget);
    Configuration::GetBoolValue("cumulus-lighting-quick-and-dirty", quickLighting);
    Configuration::GetDoubleValue("cumulus-lighting-quick-and-dirty-attenuation",
                                  quickLightingAttenuation);
    Configuration::GetDoubleValue("cumulus-lighting-quick-and-dirty-attenuation-hi-res",
                                  quickLightingAttenuationHiRes);
    quickLightingAttenuation *= Atmosphere::GetUnitScale();
    quickLightingAttenuationHiRes *= Atmosphere::GetUnitScale();

    Configuration::GetBoolValue("disable-cloud-growth", noUpdate);
    Configuration::GetBoolValue("cull-interior-voxels", cullInteriorVoxels);
    Configuration::GetDoubleValue("cumulus-cull-fade-threshold", alphaCullThreshold);

    Configuration::GetDoubleValue("metaball-color-randomness", colorRandomness);
    Configuration::GetDoubleValue("metaball-color-randomness-hi-res", colorRandomnessHiRes);

    Configuration::GetBoolValue("cumulus-darken-with-density", darkenWithDensity);
    Configuration::GetDoubleValue("cumulus-density-darkening", darkenAmount);
}

typedef struct puff_s {
    Vector3 pos;
    float radius;
} Puff;

bool CumulusCloud::InitFromFile(const CumulusCloudInit& init, std::istream *stream, double scale)
{
    ReadConfig();

    extinctionProbability = init.extinctionProbability;
    transitionProbability = init.transitionProbability;
    vaporProbability = init.vaporProbability;
    initialVaporProbability = init.initialVaporProbability;
    timeStepInterval = (unsigned int)(init.timeStepInterval * 1000.0);
    albedo = init.albedo;
    dropletsPerCubicCm = init.dropletsPerCubicCm;
    ambientScattering = init.ambientScattering;
    attenuation = init.attenuation;
    verticalGradient = init.verticalGradient;

    // Convert to cubic meters
    double dropletsPerCubicM = dropletsPerCubicCm * 100 * 100 * 100;

    char buf[1024];
    SL_VECTOR(Vector3 *) translationStack;
    Vector3 currentTrans(0,0,0);

    // Build up a vector of puffs that will become voxels
    SL_VECTOR(Puff) puffs;

    Vector3 *trans = 0;

    while (stream->good()) {
        stream->getline(buf, 1024);
        // Look for a sphere definition
        SL_STRING s(buf);
        SL_STRING::size_type pos;
        pos = s.find("geometry Sphere");
        if (pos != SL_STRING::npos) {
            SL_STRING s2 = s.substr(pos);
            float radius;
            std::stringstream converter(s2.c_str());
            converter.imbue(std::locale("C"));
            SL_STRING geometry, sphere, bracket, sradius;
            converter >> geometry >> sphere >> bracket >> sradius >> radius;

            //sscanf(s2.c_str(), "geometry Sphere { radius %f", &radius);

            radius *= (float)scale;
            Puff p;
            p.radius = radius;
            voxelSize = radius;
            p.pos = currentTrans;
            puffs.push_back(p);
        } else {
            // Entering a child, push on a new translation vector
            pos = s.find("{");
            if (pos == SL_STRING::npos) pos = s.find("[");
            if (pos != SL_STRING::npos) {
                trans = SL_NEW Vector3(0, 0, 0);
                translationStack.push_back(trans);
            } else {
                // Exiting a child, pop off a translation vector.
                pos = s.find("}");
                if (pos == SL_STRING::npos) pos = s.find("]");
                if (pos != SL_STRING::npos) {
                    trans = translationStack.back();
                    currentTrans = currentTrans - *trans;
                    translationStack.pop_back();
                    SL_DELETE trans;
                } else {
                    // Look for a translation, update the translation vector
                    pos = s.find("translation");
                    if (pos != SL_STRING::npos) {
                        SL_STRING s2 = s.substr(pos);
                        float x, y, z;

                        std::stringstream converter(s2.c_str());
                        converter.imbue(std::locale("C"));
                        SL_STRING translation;
                        converter >> translation >> x >> y >> z;
                        //sscanf(s2.c_str(), "translation %f %f %f", &x, &y, &z);
                        if (trans) {
                            *trans = Vector3(x * scale, -z * scale, y * scale);
                            currentTrans = currentTrans + *trans;
                        }
                    }
                }
            }
        }
    }

    // Center it.
    double minX, maxX, minY, maxY, minZ, maxZ;
    minX = minY = minZ = 1.0E6;
    maxX = maxY = maxZ = -1.0E6;
    int i;

    for (i = 0; i < (int)puffs.size(); i++) {
        if (puffs[i].pos.x > maxX) maxX = puffs[i].pos.x;
        if (puffs[i].pos.x < minX) minX = puffs[i].pos.x;
        if (puffs[i].pos.y > maxY) maxY = puffs[i].pos.y;
        if (puffs[i].pos.y < minY) minY = puffs[i].pos.y;
        if (puffs[i].pos.z > maxZ) maxZ = puffs[i].pos.z;
        if (puffs[i].pos.z < minZ) minZ = puffs[i].pos.z;
    }
    double yOffset = -minY;
    double xMid = minX + (maxX - minX) * 0.5;
    double zMid = minZ + (maxZ - minZ) * 0.5;
    double xOffset = -xMid;
    double zOffset = -zMid;
    Vector3 offset(xOffset, yOffset, zOffset);

    for (i = 0; i < (int)puffs.size(); i++) {
        puffs[i].pos = puffs[i].pos + offset;
    }

    width = (int)puffs.size();
    height = 1;
    depth = 1;

    Vector3 minBounds(1.0E6, 1.0E6, 1.0E6);
    Vector3 maxBounds(-1.0E6, -1.0E6, -1.0E6);

    if (voxels) {
        FreeVoxels();
    }

    voxels = (Voxel****)(SL_MALLOC(width * sizeof(Voxel*)));
    for (i = 0; i < width; i++) {
        assert(voxels);
        voxels[i] = (Voxel***)(SL_MALLOC(sizeof(Voxel*)));
        voxels[i][0] = (Voxel**)(SL_MALLOC(sizeof(Voxel*)));
        voxels[i][0][0] = SL_NEW Voxel(puffs[i].radius, init.dropletSize, dropletsPerCubicM, init.spinRate);
        voxels[i][0][0]->SetWorldPosition(puffs[i].pos);
        voxels[i][0][0]->SetHasCloud(true);

        if (puffs[i].pos.x < minBounds.x) minBounds.x = puffs[i].pos.x;
        if (puffs[i].pos.y < minBounds.y) minBounds.y = puffs[i].pos.y;
        if (puffs[i].pos.z < minBounds.z) minBounds.z = puffs[i].pos.z;
        if (puffs[i].pos.x > maxBounds.x) maxBounds.x = puffs[i].pos.x;
        if (puffs[i].pos.y > maxBounds.y) maxBounds.y = puffs[i].pos.y;
        if (puffs[i].pos.z > maxBounds.z) maxBounds.z = puffs[i].pos.z;
    }

    double voxelRadius = Voxel::ComputeVoxelRadius(voxelSize);
    bounds.x = (maxBounds.x - minBounds.x) + voxelRadius * 2.0;
    bounds.y = (maxBounds.y - minBounds.y) + voxelRadius * 2.0;
    bounds.z = (maxBounds.z - minBounds.z) + voxelRadius * 2.0;

    return puffs.size() > 0;
}

bool CumulusCloud::Init(const CumulusCloudInit& init)
{
    ReadConfig();

    voxelSize = init.voxelSize;
    assert (voxelSize > 0);

    width = (int)(init.width / voxelSize);
    height = (int)(init.height / voxelSize);
    depth = (int)(init.depth / voxelSize);

    extinctionProbability = init.extinctionProbability;
    transitionProbability = init.transitionProbability;
    vaporProbability = init.vaporProbability;
    initialVaporProbability = init.initialVaporProbability;
    timeStepInterval = (unsigned int)(init.timeStepInterval * 1000.0);
    albedo = init.albedo;
    dropletsPerCubicCm = init.dropletsPerCubicCm;
    ambientScattering = init.ambientScattering;
    attenuation = init.attenuation;
    verticalGradient = init.verticalGradient;

    // Convert to cubic meters
    double dropletsPerCubicM = dropletsPerCubicCm * 100 * 100 * 100;

    voxels = (Voxel****)(SL_MALLOC(width * sizeof(Voxel*)));
    for (int i = 0; i < width; i++) {
        assert(voxels);
        voxels[i] = (Voxel***)(SL_MALLOC(depth * sizeof(Voxel*)));
        for (int j = 0; j < depth; j++) {
            assert(voxels[i]);
            voxels[i][j] = (Voxel**)(SL_MALLOC(height * sizeof(Voxel*)));
            for (int k = 0; k < height; k++) {
                float sizeVariation = (float)voxelSize * 0.2f;
                float dSize = Atmosphere::GetCurrentAtmosphere()->GetRandomNumberGenerator()->UniformRandomFloat() * sizeVariation;
                dSize -= sizeVariation * 0.5f;

                bool fixed = false;
                int atlasIdx = 0;

                if (GetParentCloudLayer()->UsingCloudAtlas()) {
                    float normalizedHeight = (float)(k) / (float)height;
                    float rnd = Atmosphere::GetCurrentAtmosphere()->GetRandomNumberGenerator()->UniformRandomFloat();
                    fixed = normalizedHeight > rnd;

                    if (fixed) {
                        int roll = Atmosphere::GetCurrentAtmosphere()->GetRandomNumberGenerator()->UniformRandomIntRange(0, 13);
                        int hiResTextures[14] = { 1, 2, 3, 4, 5, 6, 7, 8, 10, 11, 12, 14, 15, 16 };
                        atlasIdx = hiResTextures[roll];
                    } else {
                        int roll = Atmosphere::GetCurrentAtmosphere()->GetRandomNumberGenerator()->UniformRandomIntRange(0, 3);
                        int spinningTextures[4] = { 9, 13, 8, 9};
                        atlasIdx = spinningTextures[roll];
                    }
                }

                voxels[i][j][k] = SL_NEW Voxel(voxelSize + dSize, init.dropletSize, dropletsPerCubicM, init.spinRate, atlasIdx, !fixed);
                assert(voxels[i][j][k]);
            }
        }
    }

    // Compute bounds
    // actual cloud size is up to dimension + 2 times spacing of voxels
    // (to account for the position randomization) plus the radius of the
    // actual voxels on each end (*2)
    double voxelRadius = Voxel::ComputeVoxelRadius(voxelSize);
    bounds.x = (width + 2) * voxelSize + voxelRadius * 2.0;
    bounds.z = (depth + 2) * voxelSize + voxelRadius * 2.0;
    bounds.y = (height + 2) * voxelSize + voxelRadius * 2.0;

    InitializeVoxelState();

    return true;
}

void CumulusCloud::FreeVoxels()
{
    if (voxels) {
        int i, j, k;

        for (i = 0; i < width; i++) {
            for (j = 0; j < depth; j++) {
                for (k = 0; k < height; k++) {
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

CumulusCloud::~CumulusCloud()
{
    FreeVoxels();

    if (compiledObject) SL_DELETE compiledObject;
}

void CumulusCloud::GetSize(double& pwidth, double& pdepth, double& pheight) const
{
    pwidth = bounds.x;
    pdepth = bounds.z;
    pheight= bounds.y;
}

void CumulusCloud::Submit(Voxel *v)
{
    drawList.push_back(v);
}

static bool comp(Voxel *v1, Voxel *v2)
{
    return (*v1 < *v2);
}

void CumulusCloud::ShadeCloudFast(const Vector3& lightDir, const Color& lightColor)
{
    /* For each voxel, find the ray from the voxel to the light source. Then,
       find the intersection of this ray with the cloud's bounding ellipsoid.
       Attenuate the light for this voxel by the normalized distance into
       the cloud volume by the light ray hitting it.

       This technique is less accurate since it does not simulate the actual
       cloud voxels shading each other, but allows us to get reasonable visual
       results without the expense of doing pixel read-back during the lighting
       pass.
     */

    SL_VECTOR(Voxel*) ::iterator it;

    Vector3 V(lightDir * -1);

    double w, h, d;
    GetSize(w, d, h);

    // ray / ellipsoid intersection algorithm from Lengyel
    double m = (double)w / (double)(h * 2.0); // height*2 since the volume is a
    // hemi-ellipsoid, we want the full ellipsoid.
    double n = (double)w / (double)d;

    double a = (V.x * V.x) + (m * m * V.y * V.y) + (n * n * V.z * V.z);
    double twoa = 2.0 * a;
    double invTwoA = 1.0 / twoa;
    double foura = 4.0 * a;
    double r = (double)w * 0.5;
    double r2 = r * r;
    double m2 = m * m;
    double n2 = n * n;

    double constTerm = albedo / (4.0 * PI);

    double finalAttenuation = quickLightingAttenuation;
    if (attenuation > 0) {
        finalAttenuation = attenuation;
    }

    if (GetParentCloudLayer()->UsingCloudAtlas()) {
        finalAttenuation = quickLightingAttenuationHiRes;
    }

    double invAttenuation = 1.0 / finalAttenuation;

    for (it = drawList.begin(); it != drawList.end(); it++) {
        const Vector3f& Sf = (*it)->GetWorldPosition(); // The voxel "world position" is really
        // object space
        Vector3 S(Sf.x, Sf.y, Sf.z);
        //S = S * Renderer::GetInstance()->GetInverseBasis3x3();
        double b = 2.0 * (S.x * V.x + m2 * S.y * V.y + n2 * S.z * V.z);
        double c = S.x * S.x + m2 * S.y * S.y + n2 * S.z * S.z - r2;
        double D = b * b - foura * c;

        double t;

        if (D >= 0) {
            t = (-b - sqrt(D)) * invTwoA; // argh, this sqrt seems unavoidable.

            t *= -invAttenuation;

            if (t > 1.0) t = 1.0;
            if (t < 0) t = 0;

            // t is now a parameter along the ray from the voxel to the sun.
            // Flip to represent the distance into the cloud volume.
            t = 1.0 - t;

            t *= constTerm * (*it)->GetOpticalDepth();

            Color voxelColor(t * lightColor.r,
                             t * lightColor.g,
                             t * lightColor.b,
                             1.0);

            (*it)->SetMetaballColor(voxelColor);
        } else {
            (*it)->SetMetaballColor(lightColor * (float)constTerm * (*it)->GetOpticalDepth());
        }
    }

    drawList.clear();
}

void CumulusCloud::ShadeCloud(const Vector3& lightPos, const Color& lightColor)
{
    Renderer *renderer = Renderer::GetInstance();

    // Save off modelview matrix
    Matrix4 mv;
    Vector3 camPos = Atmosphere::GetCurrentAtmosphere()->GetCamPos();
    renderer->GetModelviewMatrix(&mv);

    //Find center in ortho space
    Vector3 center = GetWorldPosition();

    // Translate to cloud position
    Matrix4 xlate;
    xlate.elem[0][3] = center.x;
    xlate.elem[1][3] = center.y;
    xlate.elem[2][3] = center.z;
    renderer->MultiplyModelviewMatrix(xlate);

    Metaball::SetCameraPosition(camPos);
    Metaball::SetOffsetPosition(center);

    // Sort from the sun position
    Voxel::SetOffsetPosition(center);
    Voxel::SetSortFromPosition(lightPos);
    Voxel::SetSortingMode(Voxel::FRONT_TO_BACK);

    sort(drawList.begin(), drawList.end(), comp);

    // Draw every billboard
    Billboard::StartBatchDraw(Metaball::GetTexture(Atmosphere::GetCurrentAtmosphere()), 1.0, true);
    renderer->EnableBlending(ONE, INVSRCALPHA);

    SL_VECTOR(Voxel*) ::iterator it;
    for (it = drawList.begin(); it != drawList.end(); it++) {
        double opticalDepth = (*it)->GetOpticalDepth();

        Vector3 screenPos;
        const Vector3f& worldPosf = (*it)->GetWorldPosition();
        Vector3 worldPos(worldPosf.x, worldPosf.y, worldPosf.z);
        worldPos = worldPos * renderer->GetBasis3x3();
        if (renderer->ProjectToScreen(worldPos, &screenPos)) {
            unsigned int pixel;
            if (renderer->GetPixels((int)screenPos.x, (int)screenPos.y, 1, 1, &pixel, true)) {
                unsigned char r, g, b, a;

                a = (unsigned char)((pixel & 0xFF000000) >> 24);
                b = (unsigned char)((pixel & 0x00FF0000) >> 16);
                g = (unsigned char)((pixel & 0x0000FF00) >> 8);
                r = (unsigned char)((pixel & 0x000000FF));

                double dr = (double)r / 256.0;
                double dg = (double)g / 256.0;
                double db = (double)b / 256.0;
                double Y = 0.2126 * dr + 0.7152 * dg + 0.0722 * db;

                double constTerm = (albedo * opticalDepth) / (4.0 * PI);
                Color pixelColor(Y * lightColor.r * constTerm,
                                 Y * lightColor.g * constTerm,
                                 Y * lightColor.b * constTerm,
                                 1.0);

                (*it)->SetMetaballColor(pixelColor);
                (*it)->ComputeBillboardColor(0, colorRandomness, ambientScattering);
                (*it)->DrawInBatch(true);
            }
        }
    }

    QuadBatches *qbs = Billboard::EndBatchDraw();
    SL_DELETE qbs;

    // Restore modelview matrix
    renderer->SetModelviewMatrix(mv);

    drawList.clear();
}

void CumulusCloud::DrawCloud()
{
    Renderer *renderer = Renderer::GetInstance();

    //TODO: Make billboards play nice with GL_FOG.
    //if (renderer->GetFogEnabled()) return;

    bool wantsUpdate, useImposter;
    imposter->SetContext(&useImposter, &wantsUpdate);

    if (wantsUpdate || !useImposter) {
        // Save off modelview matrix
        Matrix4 mv;
        Vector3 camPos = Atmosphere::GetCurrentAtmosphere()->GetCamPos();
        renderer->GetModelviewMatrix(&mv);

        // Darken clouds with density
        Color currentForce;
        bool darknessApplied = false;
        if (!Billboard::GetForceConstantColor(currentForce) && darkenWithDensity) {
            darknessApplied = true;
            double density = GetParentCloudLayer()->GetDensity();
            if (density < 0) density = 0;
            if (density > 1.0) density = 1.0;
            double factor = 1.0 - (density * density * density * darkenAmount);
            Color darkness(factor, factor, factor, 0.0);
            Billboard::ForceConstantColor(true, darkness);
        }

        Color originalAmbient;
        Vector3 forcedAmbient;
        bool ambientOverride = false, doLighting;
        if (GetParentCloudLayer()->GetOverriddenCloudColor(forcedAmbient, doLighting)) {
            originalAmbient = Metaball::GetAmbientColor();
            Color forcedColor = Color(forcedAmbient.x, forcedAmbient.y, forcedAmbient.z);
            if (doLighting) {
                forcedColor = forcedColor * originalAmbient;
            }
            Metaball::SetAmbientColor(forcedColor);
            Cloud::SetAmbientColor(forcedColor);
            ambientOverride = true;
        }

        // Do a gradient
        Billboard::SetVerticalGradient(verticalGradient);
        Billboard::SetSoftness(GetParentCloudLayer()->GetIsSoft() ? (float)(voxelSize) : 0);

        // Translate to the cloud position
        Matrix4 xlate;
        Vector3 center = GetWorldPosition();
        xlate.elem[0][3] = center.x;
        xlate.elem[1][3] = center.y;
        xlate.elem[2][3] = center.z;
        renderer->MultiplyModelviewMatrix(xlate);

        renderer->EnableBlending(ONE, INVSRCALPHA);
        renderer->EnableDepthReads(true);
        renderer->EnableDepthWrites(false);

        // The sort order burned into the quad batch will look wrong after the camera
        // angle changes enough...
        if (recompiles < recompileBudget) {
            Vector3 v1 = camPos - center;
            Vector3 v2 = lastCameraPos - center;
            v1.Normalize();
            v2.Normalize();
            double camAngle = fabs(v1.Dot(v2));
            if ((camAngle < sortAngleThreshold) || invalid) {
                invalid = false;
                recompiles++;
                lastCameraPos = camPos;
                imposter->Invalidate();
                if (compiledObject) {
                    compiledObject->Invalidate();
                }
            }
        }

        if (compiledObject && !compiledObject->IsInvalid()) {
            TextureHandle cloudTexture;
            if (GetParentCloudLayer()->UsingCloudAtlas()) {
                cloudTexture = Metaball::GetAtlasTexture(Atmosphere::GetCurrentAtmosphere());
            } else {
                cloudTexture = Metaball::GetWispTexture(Atmosphere::GetCurrentAtmosphere());
            }
            compiledObject->Draw(cloudTexture, fade * alpha);
        } else {
            if (compiledObject) {
                SL_DELETE compiledObject;
                compiledObject = 0;
            }

            // Sort from the camera position
            Voxel::SetSortFromPosition(camPos);
            Voxel::SetOffsetPosition(center);
            Voxel::SetSortingMode(Voxel::BACK_TO_FRONT);

            Metaball::SetCameraPosition(camPos);
            Metaball::SetOffsetPosition(center);

            sort(drawList.begin(), drawList.end(), comp);

            // Draw every billboard
            TextureHandle cloudTexture;
            double randomness;
            if (GetParentCloudLayer()->UsingCloudAtlas()) {
                cloudTexture = Metaball::GetAtlasTexture(Atmosphere::GetCurrentAtmosphere());
                randomness = colorRandomnessHiRes;
            } else {
                cloudTexture = Metaball::GetWispTexture(Atmosphere::GetCurrentAtmosphere());
                randomness = colorRandomness;
            }

            Billboard::StartBatchDraw(cloudTexture, fade * alpha, true);

            SL_VECTOR(Voxel*) ::iterator it;
            for (it = drawList.begin(); it != drawList.end(); it++) {
                (*it)->ComputeBillboardColor(1, randomness, ambientScattering);
                (*it)->DrawInBatch(false);
            }

            compiledObject = Billboard::EndBatchDraw();
        }
        //renderer->EnableDepthWrites(true);

        // Restore modelview matrix
        renderer->SetModelviewMatrix(mv);

        if (darknessApplied) {
            Billboard::ForceConstantColor(false, currentForce);
        }

        if (ambientOverride) {
            Metaball::SetAmbientColor(originalAmbient);
            Cloud::SetAmbientColor(originalAmbient);
        }

        Billboard::SetVerticalGradient(0);
    }

    if (useImposter) {
        imposter->RestoreContext();
        imposter->Render(GetWorldPosition(), fade * alpha);
        renderer->EnableBlending(ONE, INVSRCALPHA);
    }

    // Blow away list of clouds
    drawList.clear();
}

bool CumulusCloud::Cull(const Frustum& f)
{
    Vector3 center = GetWorldPosition();

    double w, h, d;
    GetSize(w, d, h);
    w *= 0.5;
    d *= 0.5;

    for (int i = 0; i < f.GetNumCullingPlanes(); i++) {
        const Plane& p = f.GetPlane(i);
        const Vector3& N = p.GetNormal();
        double origin = N.Dot(center) + p.GetDistance();

        double RDotN = w * N.x;
        double SDotN = h * N.y;
        double TDotN = d * N.z;

#if ELLIPSOID_BOUNDS
        double rEff = sqrt(RDotN * RDotN + SDotN * SDotN + TDotN * TDotN);
#else // BOX_BOUNDS
        //  max limit is:  rEff = sqrt(w*w + h*h + d*d);
        double rEff = fabs( RDotN ) + fabs( SDotN ) + fabs( TDotN );
#endif

        if (origin < -rEff) {
            return true;
        }
    }

#ifdef CULL_FADED_CLOUDS
    if (fadeState == FADED_OUT) {
        unsigned int now = Atmosphere::GetCurrentAtmosphere()->GetConditions()->GetMillisecondTimer()->GetMilliseconds();
        if (now - fadeStartTime > timeStepInterval) {
            return true;
        }
    }

    double eyeLength2 = (center - Renderer::GetInstance()->GetCamPos()).SquaredLength();
    double fogExponent2 = eyeLength2 * Billboard::GetFogDensity() * Billboard::GetFogDensity();
    double fogFade = 1.0 - fogExponent2;
    double fade = fogFade;// *GetAlpha();

    if (fade <= alphaCullThreshold) {
        return true;
    }
#endif

    return false;
}

bool CumulusCloud::Update(unsigned long now, bool forceUpdate)
{
    CumulusCloudLayer *cloudLayer = dynamic_cast<CumulusCloudLayer*>(GetParentCloudLayer());
    if (!cloudLayer) return false;

    if (!noUpdate && (forceUpdate || cloudLayer->GetGrowthEnabled())) {
        double dt = (double)(now - lastTimeStep) / (double)timeStepInterval;

        if (dt > 1.0) {
            IncrementTimeStep(now);
            return true;
        }
    }

    return false;
}

bool CumulusCloud::Draw(int pass, const Vector3& lightPos, const Vector3& lightDir,
                        const Color& lightColor, bool pInvalid, const Sky *sky)
{
    drawList.clear();

    nLightColor = lightColor;
    nLightColor.ScaleToUnitOrLess();

    Vector3 dir = (GetWorldPosition() + Renderer::GetInstance()->GetUpVector() * (bounds.y * 0.5))
                  - Atmosphere::GetCurrentAtmosphere()->GetCamPos();
    dir.Normalize();
    dir = dir * Renderer::GetInstance()->GetInverseBasis3x3();
    fogColor = sky->SkyColorAt(dir);

    if (pass == 0) {
        if (!LightingChanged(lightDir) && !IsInvalid() && !pInvalid) {
            return false;
        }

        imposter->Invalidate();

        if (compiledObject) {
            compiledObject->Invalidate();
        }

        invalid = false;
    }

    int i, j, k;

    for (i = 0; i < width; i++) {
        for (j = 0; j < depth; j++) {
            for (k = 0; k < height; k++) {
                if (voxels[i][j][k]->GetHasCloud() || voxels[i][j][k]->GetDestroyed()) {
                    if (cullInteriorVoxels) {
                        if ( (i > 0 && voxels[i-1][j][k]->GetHasCloud()) &&
                                (i < width-1 && voxels[i+1][j][k]->GetHasCloud()) &&
                                (j > 0 && voxels[i][j-1][k]->GetHasCloud()) &&
                                (j < depth-1 && voxels[i][j+1][k]->GetHasCloud()) &&
                                (k > 0 && voxels[i][j][k-1]->GetHasCloud()) &&
                                (k < height-1 && voxels[i][j][k+1]->GetHasCloud()) ) {
                            continue;
                        }
                    }

                    Submit(voxels[i][j][k]);
                }
            }
        }
    }

    if (drawList.size() > 0) {
        if (pass == 0) {
            static bool first = true;
            if ( first ) {
                first = false;
                Configuration::GetBoolValue("cumulus-lighting-quick-and-dirty", quickLighting);
            }

            if (quickLighting) {
                ShadeCloudFast(lightDir, nLightColor);
            } else {
                ShadeCloud(lightPos, nLightColor);
            }
        } else if (pass == 1) {
            Renderer::GetInstance()->SubmitBlendedObject(this);
        }
    }

    return true;
}

void CumulusCloud::DrawBlendedObject()
{
    Color oldFog = Billboard::GetFogColor();

    Billboard::SetFogColor(fogColor);
    Billboard::SetFadeStartTime(fadeStartTime);

    DrawCloud();

    Billboard::SetFogColor(oldFog);
}

void CumulusCloud::InitializeVoxelState()
{
    int i, j, k;
    Vector3 origin(0, 0, 0);
    origin.x -= width * voxelSize * 0.5;
    origin.z -= depth * voxelSize * 0.5;

    double midX = (double)width * 0.5;
    double midY = (double)height; // Use full height to simulate hemi-ellipsoid
    double midZ = (double)depth * 0.5;

    double a2 = midX * midX;
    double b2 = midY * midY;
    double c2 = midZ * midZ;

    for (i = 0; i < width; i++) {
        for (j = 0; j < depth; j++) {
            for (k = 0; k < height; k++) {
                double x = (double)i - midX;
                double y = (double)k;
                double z = (double)j - midZ;
                double dist = (x * x) / a2 + (y * y) / b2 + (z * z) / c2;
                double scale = 1.0 - dist;
                if (scale < 0) scale = 0;

                Vector3 delta(i * voxelSize, k * voxelSize, j * voxelSize);

                Vector3 pos = origin + delta;

                pos.x += Atmosphere::GetCurrentAtmosphere()->GetRandomNumberGenerator()->UniformRandomIntRange(0, (int)(voxelSize * 0.5) - 1);
                pos.z += Atmosphere::GetCurrentAtmosphere()->GetRandomNumberGenerator()->UniformRandomIntRange(0, (int)(voxelSize * 0.5) - 1);

                voxels[i][j][k]->SetWorldPosition(pos);

                voxels[i][j][k]->SetHasCloud(false);

                double rnd = Atmosphere::GetCurrentAtmosphere()->GetRandomNumberGenerator()->UniformRandomDouble();
                rnd *= scale;

                voxels[i][j][k]->SetVapor(rnd > (1.0 - initialVaporProbability));

                voxels[i][j][k]->SetPhaseTransition(false);
            }
        }
    }

    // seed the cloud

    if (width >= 4 && depth >= 4) {
        voxels[width >> 2][depth >> 2][0]->SetPhaseTransition(true);
        voxels[width >> 2][depth - (depth >> 2)][0]->SetPhaseTransition(true);
        voxels[width - (width >> 2)][depth - (depth >> 2)][0]->SetPhaseTransition(true);
        voxels[width - (width >> 2)][depth >> 2][0]->SetPhaseTransition(true);
    }

    lastTimeStep = (unsigned long)time(NULL);
}

void CumulusCloud::FadeIn()
{
    if (fadeState != FADED_IN) {
        unsigned int now = Atmosphere::GetCurrentAtmosphere()->GetConditions()->GetMillisecondTimer()->GetMilliseconds();
        fadeStartTime = now;
        float timeStepSeconds = (float)timeStepInterval * 0.001f;

        int i, j, k;

        for (i = 0; i < width; i++) {
            for (j = 0; j < depth; j++) {
                for (k = 0; k < height; k++) {
                    voxels[i][j][k]->SetFadeRate(1.0f / timeStepSeconds);
                }
            }
        }

        fadeState = FADED_IN;
        invalid = true;
    }
}

void CumulusCloud::FadeOut()
{
    if (fadeState != FADED_OUT) {
        unsigned int now = Atmosphere::GetCurrentAtmosphere()->GetConditions()->GetMillisecondTimer()->GetMilliseconds();
        fadeStartTime = now;
        float timeStepSeconds = (float)timeStepInterval * 0.001f;

        int i, j, k;

        for (i = 0; i < width; i++) {
            for (j = 0; j < depth; j++) {
                for (k = 0; k < height; k++) {
                    voxels[i][j][k]->SetFadeRate(-1.0f / timeStepSeconds);
                }
            }
        }

        fadeState = FADED_OUT;
        invalid = true;
    }
}

void CumulusCloud::IncrementTimeStep(unsigned long now)
{
    fadeStartTime = now;

    double midX = (double)width * 0.5;
    double midY = (double)height; // Use full height to simulate hemi-ellipsoid
    double midZ = (double)depth * 0.5;

    double a2 = midX * midX;
    double b2 = midY * midY;
    double c2 = midZ * midZ;

    int i, j, k;

    for (i = 0; i < width; i++) {
        for (j = 0; j < depth; j++) {
            for (k = 0; k < height; k++) {
                double x = (double)i - midX;
                double y = (double)k;
                double z = (double)j - midZ;
                double dist = (x * x) / a2 + (y * y) / b2 + (z * z) / c2;
                double scale = 1.0 - dist;
                if (scale < 0) scale = 0;

                bool fAct = (
                                (i+1 < width  ? voxels[i+1][j][k]->GetPhaseTransition() : false) ||
                                (j+1 < depth  ? voxels[i][j+1][k]->GetPhaseTransition() : false) ||
                                (k+1 < height ? voxels[i][j][k+1]->GetPhaseTransition() : false) ||
                                (i-1 >= 0     ? voxels[i-1][j][k]->GetPhaseTransition() : false) ||
                                (j-1 >= 0     ? voxels[i][j-1][k]->GetPhaseTransition() : false) ||
                                (k-1 >= 0     ? voxels[i][j][k-1]->GetPhaseTransition() : false) ||
                                (i-2 >= 0     ? voxels[i-2][j][k]->GetPhaseTransition() : false) ||
                                (i+2 < width  ? voxels[i+2][j][k]->GetPhaseTransition() : false) ||
                                (j-2 >= 0     ? voxels[i][j-2][k]->GetPhaseTransition() : false) ||
                                (j+2 < depth  ? voxels[i][j+2][k]->GetPhaseTransition() : false) ||
                                (k-2 >= 0     ? voxels[i][j][k-2]->GetPhaseTransition() : false)
                            );

                bool thisAct = voxels[i][j][k]->GetPhaseTransition();

                double rnd;
                rnd = Atmosphere::GetCurrentAtmosphere()->GetRandomNumberGenerator()->UniformRandomDouble();

                voxels[i][j][k]->SetPhaseTransition((
                                                        (!thisAct) &&
                                                        voxels[i][j][k]->GetVapor() &&
                                                        fAct) || (rnd < transitionProbability * scale));

                rnd = Atmosphere::GetCurrentAtmosphere()->GetRandomNumberGenerator()->UniformRandomDouble();
                voxels[i][j][k]->SetVapor(
                    (voxels[i][j][k]->GetVapor() && !thisAct) || (rnd < vaporProbability * scale));

                rnd = Atmosphere::GetCurrentAtmosphere()->GetRandomNumberGenerator()->UniformRandomDouble();
                bool oldHasCloud = voxels[i][j][k]->GetHasCloud();
                bool newHasCloud = ((oldHasCloud || thisAct) && (rnd > extinctionProbability * (1.0 - scale)));

                float timeStepSeconds = (float)timeStepInterval * 0.001f;
                if (newHasCloud && !oldHasCloud) {
                    voxels[i][j][k]->SetFadeRate(1.0f / timeStepSeconds);
                    voxels[i][j][k]->SetCreated(true);
                    voxels[i][j][k]->SetDestroyed(false);
                } else if (!newHasCloud && oldHasCloud) {
                    voxels[i][j][k]->SetFadeRate(-1.0f / timeStepSeconds);
                    voxels[i][j][k]->SetCreated(false);
                    voxels[i][j][k]->SetDestroyed(true);
                } else {
                    voxels[i][j][k]->SetFadeRate(0.0);
                    voxels[i][j][k]->SetCreated(false);
                    voxels[i][j][k]->SetDestroyed(false);
                }

                voxels[i][j][k]->SetHasCloud(newHasCloud);
            }
        }
    }

    invalid = true;

    lastTimeStep = now;
}

bool CumulusCloud::Unserialize(std::istream& s)
{
    ReadConfig();

    Vector3 pos;
    pos.Unserialize(s);
    SetWorldPosition(pos);

    s.read((char *)&width, sizeof(int));
    s.read((char *)&height, sizeof(int));
    s.read((char *)&depth, sizeof(int));

    layerPosition.Unserialize(s);

    s.read((char *)&vaporProbability, sizeof(double));
    s.read((char *)&transitionProbability, sizeof(double));
    s.read((char *)&extinctionProbability, sizeof(double));
    s.read((char *)&initialVaporProbability, sizeof(double));
    s.read((char *)&albedo, sizeof(double));
    s.read((char *)&dropletsPerCubicCm, sizeof(double));
    s.read((char *)&attenuation, sizeof(double));
    s.read((char *)&ambientScattering, sizeof(float));

    s.read((char *)&voxelSize, sizeof(double));

    s.read((char *)&verticalGradient, sizeof(double));
    nLightColor.Unserialize(s);

    bounds.Unserialize(s);

    voxels = (Voxel****)(SL_MALLOC(width * sizeof(Voxel*)));
    for (int i = 0; i < width; i++) {
        assert(voxels);
        voxels[i] = (Voxel***)(SL_MALLOC(depth * sizeof(Voxel*)));
        for (int j = 0; j < depth; j++) {
            assert(voxels[i]);
            voxels[i][j] = (Voxel**)(SL_MALLOC(height * sizeof(Voxel*)));
            for (int k = 0; k < height; k++) {
                assert(voxels[i][j]);
                voxels[i][j][k] = SL_NEW Voxel(s);
            }
        }
    }

    return true;
}

bool CumulusCloud::Serialize(std::ostream& s)
{
    Vector3 pos = GetWorldPosition();
    pos.Serialize(s);

    s.write((char *)&width, sizeof(int));
    s.write((char *)&height, sizeof(int));
    s.write((char *)&depth, sizeof(int));

    layerPosition.Serialize(s);

    s.write((char *)&vaporProbability, sizeof(double));
    s.write((char *)&transitionProbability, sizeof(double));
    s.write((char *)&extinctionProbability, sizeof(double));
    s.write((char *)&initialVaporProbability, sizeof(double));
    s.write((char *)&albedo, sizeof(double));
    s.write((char *)&dropletsPerCubicCm, sizeof(double));
    s.write((char *)&attenuation, sizeof(double));
    s.write((char *)&ambientScattering, sizeof(float));

    s.write((char *)&voxelSize, sizeof(double));

    s.write((char *)&verticalGradient, sizeof(double));
    nLightColor.Serialize(s);

    bounds.Serialize(s);

    int i, j, k;

    for (i = 0; i < width; i++) {
        for (j = 0; j < depth; j++) {
            for (k = 0; k < height; k++) {
                voxels[i][j][k]->Serialize(s);
            }
        }
    }

    return true;
}

bool CumulusCloud::ExportToVRML(const char *filename) const
{
    FILE *f = fopen(filename, "wt");
    if (f) {
        fprintf(f, "#VRML V2.0 utf8\n\n");

        int i, j, k;
        int n = 1;

        for (i = 0; i < width; i++) {
            for (j = 0; j < depth; j++) {
                for (k = 0; k < height; k++) {
                    if (voxels[i][j][k]->GetHasCloud()) {
                        fprintf(f, "DEF Sphere%d Transform {\n", n++);
                        fprintf(f, "  translation %f %f %f\n", voxels[i][j][k]->GetWorldPosition().x,
                                voxels[i][j][k]->GetWorldPosition().y, voxels[i][j][k]->GetWorldPosition().z);
                        fprintf(f, "  children [\n");
                        fprintf(f, "    Shape {\n");
                        fprintf(f, "      geometry Sphere { radius %f }\n", voxelSize);
                        fprintf(f, "    }\n");
                        fprintf(f, "  ]\n");
                        fprintf(f, "}\n\n");
                    }
                }
            }
        }

        fclose(f);
    }
    return true;
}

void CumulusCloud::Intersect( const Vector3& origin,
                              const Vector3& direction,
                              SL_VECTOR(double)& hit_ranges )
{
    // Some Linear Algebra...
    // plane eq. -  point-normal form
    // n dot (r - r0) = 0 expands to n dot r = n dot r0
    // or d = -(n dot r0)
    // Ray-Plane intersection:
    // Ray: P = P0 + tV
    // Plane: P dot N + d = 0
    // Substituting for P, we get:
    // (P0 + tV) dot N + d = 0
    // Solving for t:
    // t = -(P0 dot N + d) / V dot N
    // Which gives point on the plane:
    // P = P0 + tV

    SL_VECTOR(Voxel*) ::iterator it;

    Renderer *renderer = Renderer::GetInstance();

    // world cloud position
    Vector3 cloud_pos = GetWorldPosition();

    // world billboard matrix
    Matrix4 billboard = renderer->GetBillboardMatrix();

    // world space billboard normal
    Vector3 normal = Vector3( billboard.elem[0][2],
                              billboard.elem[1][2],
                              billboard.elem[2][2] );

    int i, j, k;

    for (i = 0; i < width; i++) {
        for (j = 0; j < depth; j++) {
            for (k = 0; k < height; k++) {
                if (voxels[i][j][k]->GetHasCloud() || voxels[i][j][k]->GetDestroyed()) {
                    if (cullInteriorVoxels) {
                        if ( (i > 0 && voxels[i-1][j][k]->GetHasCloud()) &&
                                (i < width-1 && voxels[i+1][j][k]->GetHasCloud()) &&
                                (j > 0 && voxels[i][j-1][k]->GetHasCloud()) &&
                                (j < depth-1 && voxels[i][j+1][k]->GetHasCloud()) &&
                                (k > 0 && voxels[i][j][k-1]->GetHasCloud()) &&
                                (k < height-1 && voxels[i][j][k+1]->GetHasCloud()) ) {
                            continue;
                        }
                    }

                    Voxel* v = voxels[i][j][k];

                    // get voxel offset in object space
                    Vector3f vf_pos = v->GetWorldPosition();

                    // convert to vector of doubles in order to transform to world space
                    Vector3 voxel_pos = Vector3( vf_pos.x, vf_pos.y, vf_pos.z );

                    // compute voxel offset in world space
                    voxel_pos = voxel_pos * renderer->GetBasis3x3();

                    // add the offset to the parent cloud's world center position
                    // to obtain the voxel's final world space position
                    Vector3 pos( cloud_pos.x + voxel_pos.x,
                                 cloud_pos.y + voxel_pos.y,
                                 cloud_pos.z + voxel_pos.z );

                    // Get the half-size of the square-shaped billboard
                    double radius = v->GetRadius();

                    // Compute our 'd' for the voxel's plane equat}ion
                    // plane eq: PdotN + d = 0 so d = -PdotN
                    double d = -pos.Dot( normal );

                    // We now compute the intersection of our
                    // ray and this voxel's plane equation
                    // eye vector as the plane's normal but we negate it
                    // See derivation above
                    Vector3 dir = direction;

                    double t = -( origin.Dot( normal ) + d ) / dir.Dot( normal );

                    // We now have the point on the plane
                    Vector3 p = origin + dir * t;

                    // We test for containment using this voxel's radius
                    // Compute the vector between the center of our billboard
                    // and the intersection point on the plane
                    // Note: we are testing against a square not a circle
                    Vector3 delta = p - pos;

                    // Check if the hit point is within a square of width
                    // 2 * radius positioned at the voxel's center
                    if( delta.x < -radius ||
                            delta.x > radius ||
                            delta.y < -radius ||
                            delta.y > radius ||
                            delta.z < -radius ||
                            delta.z > radius ) {
                        continue;
                    }

                    // We have a valid hit!
                    hit_ranges.push_back( t );
                }
            }
        }
    }
}
