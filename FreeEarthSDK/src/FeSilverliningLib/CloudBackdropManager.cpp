// Copyright (c) 2012 Sundog Software LLC. All rights reserved worldwide.

#include <FeSilverliningLib/CloudBackdropManager.h>
#include <FeSilverliningLib/Cloud.h>
#include <FeSilverliningLib/Renderer.h>
#include <FeSilverliningLib/Utils.h>
#include <FeSilverliningLib/Configuration.h>

using namespace SilverLining;

CloudBackdropManager::CloudBackdropManager(int numBackdrops, double pDistance) : lastBackdropUpdated(0)
{
    double angularSize = TWOPI / (double)numBackdrops;
    double angle = 0.0;
    distance = pDistance;
    initialFrame = true;
    enabled = true;
    lastFogDensity = 0;

    double backdropAngularHeight = 45.0;
    Configuration::GetDoubleValue("cloud-backdrop-angular-height", backdropAngularHeight);
    cosBackdropAngularHeight = cos(RADIANS(backdropAngularHeight) * 0.5);

    horizontalThreshold = 0.15;
    Configuration::GetDoubleValue("cloud-backdrop-horizontal-threshold", horizontalThreshold);
    double cullDistance = distance + distance * horizontalThreshold * 2.0;
    cullDistanceSquared = cullDistance * cullDistance;

    for (int i = 0; i < numBackdrops; i++) {
        CloudBackdrop *cb = SL_NEW CloudBackdrop(angle, angularSize, distance);
        angle += angularSize;
        backdrops.push_back(cb);
    }
}

bool CloudBackdropManager::IsCloudInBackdrop(Cloud *cloud, const Vector3& camPos)
{
    if (!enabled) return false;

    CloudTypes type = cloud->GetParentCloudLayer()->GetType();
    if (type != CUMULUS_MEDIOCRIS && type != CUMULUS_CONGESTUS) return false;

    Renderer *ren = Renderer::GetInstance();
    Vector3 camPosYUp = camPos * ren->GetInverseBasis3x3();
    Vector3 cloudPosYUp = cloud->GetWorldPosition() * ren->GetInverseBasis3x3();

    double dx = camPosYUp.x - cloudPosYUp.x;
    double dz = camPosYUp.z - cloudPosYUp.z;
    double distSq = dx * dx + dz * dz;

    if (distSq < cullDistanceSquared) {
        return false;
    }

    Vector3 cloudRel = cloudPosYUp - camPosYUp;
    Vector3 ground = cloudRel;
    ground.y = 0;
    ground.Normalize();
    cloudRel.Normalize();

    double dot = ground.Dot(cloudRel);

    bool inRange = (dot > cosBackdropAngularHeight);
    return inRange;
}

bool CloudBackdropManager::Initialize()
{
    Renderer *ren = Renderer::GetInstance();
    if (!ren) return false;

    backdropShader = ren->LoadShaderFromFile("Shaders/Backdrop.cg", VERTEX_PROGRAM);

    //bool ok = true;
    SL_VECTOR(CloudBackdrop*)::iterator it;
    for (it = backdrops.begin(); it != backdrops.end(); it++) {
        if (!(*it)->Initialize(backdropShader)) return false;
    }

    return true;
}

CloudBackdropManager::~CloudBackdropManager()
{
    SL_VECTOR(CloudBackdrop*)::iterator it;
    for (it = backdrops.begin(); it != backdrops.end(); it++) {
        SL_DELETE *it;
    }

    backdrops.clear();

    Renderer::GetInstance()->DeleteShader(backdropShader);
    backdropShader = 0;
}

bool CloudBackdropManager::Update(const Frustum& f, const SL_VECTOR(CloudLayer*)& cloudLayers, unsigned long now, const Sky& sky,
                                  const Vector3& lightDir, const Color& lightColor)
{
    // Have the cloud layers themselves changed?
    bool force = false;
    if (cloudLayers.size() != lastCloudLayers.size()) {
        force = true;
    } else {
        for (unsigned int i = 0; i < cloudLayers.size(); i++) {
            double east, south;
            cloudLayers[i]->GetLayerPosition(east, south);
            Vector3 cloudPos(east, south, cloudLayers[i]->GetBaseAltitudeGeocentric());
            double diff = (cloudPos - lastCloudLayerPositions[i]).Length();
            if (diff > distance * horizontalThreshold) {
                force = true;
                break;
            }

            if ((cloudLayers[i] != lastCloudLayers[i]) || (cloudLayers[i]->GetEnabled() != lastCloudLayerEnabled[i])) {
                force = true;
                break;
            }
        }
    }

    if (fabs(Billboard::GetFogDensity() - lastFogDensity) > 1E-3) {
        force = true;
        lastFogDensity = Billboard::GetFogDensity();
    }

    if (initialFrame || force) {
        SL_VECTOR(CloudBackdrop*)::iterator it;
        for (it = backdrops.begin(); it != backdrops.end(); it++) {
            CloudBackdrop *cb = *it;
            cb->Update(f, cloudLayers, now, sky, lightDir, lightColor, true);
        }

        initialFrame = false;

        lastCloudLayers.clear();
        lastCloudLayerPositions.clear();
        lastCloudLayerEnabled.clear();
        for (unsigned int i = 0; i < cloudLayers.size(); i++) {
            lastCloudLayers.push_back(cloudLayers[i]);
            double east, south;
            cloudLayers[i]->GetLayerPosition(east, south);
            Vector3 cloudPos(east, south, cloudLayers[i]->GetBaseAltitudeGeocentric());
            lastCloudLayerPositions.push_back(cloudPos);
            lastCloudLayerEnabled.push_back(cloudLayers[i]->GetEnabled());
        }

        return true;
    }

    if (backdrops.size() < 1) return false;

    backdrops[lastBackdropUpdated]->Update(f, cloudLayers, now, sky, lightDir, lightColor, false);

    lastBackdropUpdated++;

    if (lastBackdropUpdated >= (int)backdrops.size()) {
        lastBackdropUpdated = 0;
    }

    return true;
}

bool CloudBackdropManager::Draw(const Frustum& f)
{
    Renderer *ren = Renderer::GetInstance();
    if (!ren) return false;

    bool ok = true;

    SL_VECTOR(CloudBackdrop*)::iterator it;
    for (it = backdrops.begin(); it != backdrops.end(); it++) {
        CloudBackdrop *cb = *it;
        if (!cb->Cull(f)) {
            if (!cb->Draw(1)) {
                ok = false;
            }
        }
    }

    return ok;
}



