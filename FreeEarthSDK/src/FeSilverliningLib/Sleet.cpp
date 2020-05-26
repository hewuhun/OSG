// Copyright (c) 2009-2015 Sundog Software LLC, All rights reserved worldwide.

#include <FeSilverliningLib/Sleet.h>
#include <FeSilverliningLib/VertexBuffer.h>
#include <FeSilverliningLib/Renderer.h>
#include <FeSilverliningLib/Matrix4.h>
#include <FeSilverliningLib/Configuration.h>
#include <FeSilverliningLib/Metaball.h>
#include <FeSilverliningLib/Utils.h>

//#define CUBE_VOLUME

#ifdef _OPENMP
#include <omp.h>
#endif

using namespace SilverLining;
using namespace std;

#define NUM_RADII 65536
static Vector3f randomOffsets[NUM_RADII];

Sleet::Sleet(const Atmosphere *atm)
{
    lastComputedIntensity = lastComputedFogDensity = -1;
    atmosphere = atm;
    maxParticles = 100000;
    maxIntensity = 30.0;
    sleetWidthMultiplier = 1.0f;
    maxRenderedParticles = 3000;
    visibilityMultiplier = 1.0;
    visibility = 0;
    fogDensity = 0;
    sleetStreakCameraCoords = false;
    sleetAlpha = 0.7f;
    minParticlePixels = 5.0;

    Configuration::GetBoolValue("sleet-streak-camera-coords", sleetStreakCameraCoords);
    Configuration::GetIntValue("sleet-max-particles", maxParticles);
    Configuration::GetIntValue("sleet-max-rendered-particles", maxRenderedParticles);
    Configuration::GetDoubleValue("sleet-max-intensity", maxIntensity);
    Configuration::GetFloatValue("sleet-streak-width-multiplier", sleetWidthMultiplier);
    Configuration::GetDoubleValue("sleet-visibility-multiplier", visibilityMultiplier);
    Configuration::GetDoubleValue("sleet-near-clip", nearClip);
    nearClip *= Atmosphere::GetUnitScale();
    Configuration::GetDoubleValue("sleet-volume-radius", farClip);
    farClip *= Atmosphere::GetUnitScale();
    Configuration::GetFloatValue("sleet-alpha", sleetAlpha);
    Configuration::GetDoubleValue("sleet-streak-frame-time", streakFrameTime);
    Configuration::GetDoubleValue("sleet-minimum-pixels", minParticlePixels);
    Configuration::GetBoolValue("sleet-use-depth-buffer", useDepthBuffer);

    InitializeBuffers();
    InitializePositions();
    LoadStreakTextures();
}

#define RAND_RADIUS Atmosphere::GetCurrentAtmosphere()->GetRandomNumberGenerator()->UniformRandomIntRange(0, NUM_RADII - 1)

void SleetParticle::InitializeParticlePosition(const Vector3& camPos, int idxOffset)
{
    int index = RAND_RADIUS;
    index += idxOffset;
    index %= NUM_RADII;
    const Vector3f& offset = randomOffsets[index];

    pos.x = camPos.x + offset.x;
    pos.y = camPos.y + offset.y;
    pos.z = camPos.z + offset.z;
}

void Sleet::InitializePositions()
{
    Vector3 camPos = Atmosphere::GetCurrentAtmosphere()->GetCamPos();

    float range = (float)(farClip - nearClip);

    const RandomNumberGenerator *rng = Atmosphere::GetCurrentAtmosphere()->GetRandomNumberGenerator();

    for (int i = 0; i < NUM_RADII; i++) {
#ifdef CUBE_VOLUME
        float x = 0, y = 0, z = 0;
        while ( x <= nearClip && x >= -nearClip && y <= nearClip && y >= -nearClip && z <= nearClip && z >= -nearClip ) {
            x = rng->UniformRandomFloat() * (float)farClip * 2.0f - (float)farClip;
            y = rng->UniformRandomFloat() * (float)farClip * 2.0f - (float)farClip;
            z = rng->UniformRandomFloat() * (float)farClip * 2.0f - (float)farClip;
        }
        randomOffsets[i] = Vector3f(x, y, z);
#else
        float x = 0, y = 0, z = 0;
        float r2 = 0;
        do {
            x = rng->UniformRandomFloat() * (float)farClip * 2.0f - (float)farClip;
            y = rng->UniformRandomFloat() * (float)farClip * 2.0f - (float)farClip;
            z = rng->UniformRandomFloat() * (float)farClip * 2.0f - (float)farClip;
            r2 = x*x + y*y + z*z;
        } while (r2 < (nearClip * nearClip) || r2 >(farClip * farClip));

        randomOffsets[i] = Vector3f(x, y, z);
#endif
    }

    SL_VECTOR(SleetParticle) ::iterator it;
    for (it = particles.begin(); it != particles.end(); it++) {
        (*it).InitializeParticlePosition(camPos, 0);

    }
}

void Sleet::SetEffectRange(double fNear, double fFar, bool bUseDepthBuffer)
{
    if (fNear > 0 && fFar > 0 && fFar > fNear) {
        if (fNear != nearClip || fFar != farClip || bUseDepthBuffer != useDepthBuffer) {
            Precipitation::SetEffectRange(fNear, fFar, bUseDepthBuffer);
            InitializePositions();
        }
    }
}

void Sleet::InitializeBuffers()
{
    vertexBuffer = SL_NEW VertexBuffer(4);
    indexBuffer = SL_NEW IndexBuffer(4);

    if (vertexBuffer && indexBuffer) {
        if (vertexBuffer->LockBuffer()) {
            if (indexBuffer->LockBuffer()) {
                Vertex *v = vertexBuffer->GetVertices();
                Index *idx = indexBuffer->GetIndices();

                if (v && idx) {
                    particles.reserve(maxParticles);
                    for (int i = 0; i < maxParticles; i++) {
                        particles.push_back(SleetParticle());
                    }

                    Color startColor(1.0f, 1.0f, 1.0f, 1.0f);
                    Color endColor(1.0f, 1.0f, 1.0f, 1.0f);

                    v[0].SetColor(startColor);
                    v[0].SetUV(0, 0);
                    v[0].x = -0.5;
                    v[0].y = 0.5;
                    v[0].z = 0;
                    v[0].w = 1.0f;
                    idx[0] = 0;

                    v[1].SetColor(startColor);
                    v[1].SetUV(1, 0);
                    v[1].x = 0.5;
                    v[1].y = 0.5;
                    v[1].z = 0;
                    v[1].w = 1.0f;
                    idx[1] = 1;

                    v[2].SetColor(endColor);
                    v[2].SetUV(0, 1);
                    v[2].x = -0.5;
                    v[2].y = -0.5;
                    v[2].z = 0;
                    v[2].w = 1.0f;
                    idx[2] = 2;

                    v[3].SetColor(endColor);
                    v[3].SetUV(1, 1);
                    v[3].x = 0.5;
                    v[3].y = -0.5;
                    v[3].z = 0;
                    v[3].w = 1.0f;
                    idx[3] = 3;
                }
                indexBuffer->UnlockBuffer();
            }
            vertexBuffer->UnlockBuffer();
        }
    }
}

void Sleet::LoadStreakTextures()
{
    Renderer *ren = Renderer::GetInstance();

    for (int j = 0; j < NUM_SLEET_TEXTURE_OSCILLATIONS; j++) {
        char filePath[1024];
        sprintf(filePath, "sleet/cv0_osc%d.TGA", j);

        sleetTextures[j] = 0;
        ren->LoadTextureFromFile(filePath, &sleetTextures[j], true, true);
    }
}

Sleet::~Sleet()
{
    if (indexBuffer) SL_DELETE indexBuffer;
    if (vertexBuffer) SL_DELETE vertexBuffer;

    for (int j = 0; j < NUM_SLEET_TEXTURE_OSCILLATIONS; j++) {
        if (sleetTextures[j]) Renderer::GetInstance()->ReleaseTexture(sleetTextures[j]);
    }
}

void Sleet::SetIntensity(double R)
{
    // Source: Marshall, J.S., and W. McK. Palmer, 1948, "The Distribution of Raindrops With Size",
    // Journal of Meteorology, 5, 165-166.

    // Their work has really stood the test of time...

    if (R > maxIntensity) R = maxIntensity;
    if (R < 0) R = 0;

    intensity = R;

    if (intensity == 0) {
        fogDensity = 0;
        return;
    }

    if (R == lastComputedIntensity) {
        fogDensity = lastComputedFogDensity;
        return;
    }

    const double N0 = 8000.0;
    double A = 41.0 * pow(R, -0.21);
    double Nd[NUM_PARTICLE_DIAMETERS];
    double NdTotal = 0;
    int i;

    for (i = 0; i < NUM_PARTICLE_DIAMETERS; i++) {
        double D = MIN_PARTICLE_DIAMETER + (double)i * SLEET_DIAMETER_INCREMENT;
        Nd[i] = N0 * exp(-A * D * 0.001);
        NdTotal += Nd[i];
        terminalVelocities[i] = pow( 3.778 * D, 0.67) * Atmosphere::GetUnitScale();
        dropDiameters[i] = D * Atmosphere::GetUnitScale();
    }

    // Normalize and multiply by available particles.
    double totalParticles = (R / maxIntensity) * (double)maxParticles;
    for (i = 0; i < NUM_PARTICLE_DIAMETERS; i++) {
        double n = (Nd[i] / NdTotal) * totalParticles;
        nDrops[i] = (int)n;
    }

    // Now estimate visibility per Atlas, David, 1953, "Optical Extinction by Rainfall", Journal of Meterology Vol. 10
    // 486-488. Another oldie but goodie
    visibility = 11.6 * pow(R, -0.63); // This assumes "Bergeron type" rainfall. Orographic has a different equation
    visibility *= visibilityMultiplier;
    visibility *= Atmosphere::GetUnitScale();

    // From Koschmeider equation
    fogDensity = (3.91 / (visibility * 1000.0)) / 1.3;

    lastComputedIntensity = intensity;
    lastComputedFogDensity = fogDensity;
}

void Sleet::Render(double dt, const Frustum& f, const Color& lightColor)
{
    if (intensity == 0) return;

    Color sleetColor = lightColor;
    sleetColor.a = sleetAlpha;

    Renderer *ren = Renderer::GetInstance();

    // Clamp dt to something reasonable
    if (dt > (1.0 / 20.0))
        dt = 1.0 / 20.0;

    Matrix4 savedProj;
    ren->GetProjectionMatrix(&savedProj);

    Matrix4 savedModelview;
    ren->GetModelviewMatrix(&savedModelview);

    float nearDepth, farDepth;
    ren->GetDepthRange(nearDepth, farDepth);
    ren->SetDepthRange(0.0f, 1.0f);

    //Set and later restore near clip - need to see streaks very close to the camera.
    //We disable use of the depth buffer so this has no ill effects.

    if (!useDepthBuffer) {
        ren->AdjustNearFarClip(nearClip, farClip);
    }

    Frustum adjustedFrustum;
    ren->ExtractViewFrustum(adjustedFrustum);

    Matrix4 proj;
    ren->GetProjectionMatrix(&proj);
    Matrix4 invProj = proj.Inverse();

    int particleIdx = 0;

    // Set up the proper state.
    ren->EnableBackfaceCulling(false);
    ren->EnableTexture2D(true);
    ren->EnableLighting(false);
#ifdef ANDROID
    ren->EnableBlending(SRCALPHA, ONE);
#else
    ren->EnableBlending(SRCALPHA, INVSRCALPHA);
#endif
    ren->EnableDepthReads(useDepthBuffer);
    ren->EnableDepthWrites(false);

    Vector3 currentCamPos = Atmosphere::GetCurrentAtmosphere()->GetCamPos();

    double velocity, heading;
    atmosphere->GetConditions().GetWind(velocity, heading, atmosphere->GetConditions().GetLocation().GetAltitude());
    double windX, windZ;
    atmosphere->GetConditions().GetPrecipitationWind(windX, windZ);
    double dx = windX + sin(RADIANS(-heading)) * velocity;
    double dz = windZ + cos(RADIANS(-heading)) * velocity;
    Vector3 wind(Vector3(dx, 0, dz) * ren->GetBasis3x3() * 0.2 * dt);
    Vector3 streakWind;
    streakWind = Vector3(Vector3(dx, 0, dz) * ren->GetBasis3x3() * 0.2 * streakFrameTime);

    int numStreaksRendered = 0;

    // Vector pointing away from the eyepoint into the screen
    bool useNDC = true;
    Configuration::GetBoolValue("billboard-use-ndc", useNDC);
    Vector3 camDir;
    if (useNDC) {
        Vector3 ndcIn(0, 0, 1);
        // de-project to world space
        Matrix4 view = savedModelview;
        view.elem[0][3] = view.elem[1][3] = view.elem[2][3] = 0;
        Matrix4 mvp = savedProj * view;
        Matrix4 invMvp = mvp.Inverse();
        camDir = invMvp * ndcIn;
        camDir.Normalize();
    } else {
        camDir = Vector3(-savedModelview.elem[2][0], -savedModelview.elem[2][1], -savedModelview.elem[2][2]);
    }

    Configuration::GetFloatValue("sleet-streak-width-multiplier", sleetWidthMultiplier);

    for (int i = 0; i < NUM_PARTICLE_DIAMETERS; i++) {
        // Compute terminal velocity of particle based on its diameter from Atlas and Ulbrich (1977)
        double v = terminalVelocities[i];
        double dropDiameter = dropDiameters[i];

        double dy = -v * dt;

        Vector3 down = Vector3(0, dy, 0) * ren->GetBasis3x3();
        Vector3 streakDown;

        streakDown = Vector3(0, -v * streakFrameTime, 0) * ren->GetBasis3x3();

        Vector3 motion = down + wind;
        Vector3 streakMotion = streakDown + streakWind;

        // Compute vector of raindrop's movement relative to the camera since the last frame
        Vector3 dp = (previousCamPos - currentCamPos) + motion;
        Vector3 streakDp = (previousCamPos - currentCamPos) + streakMotion;
        if (!sleetStreakCameraCoords) {
            streakDp = streakMotion;
        }

        Matrix4 rotateTranslate;

        double streakLength = streakDp.Length();

        if (dp.SquaredLength() > 0) {
            ren->SetCurrentColor(sleetColor);

            // Compute basis of motion vector, make a rotation matrix to properly orient
            // the streaks as a billboard using the motion vector as a fixed axis.
            Vector3 up = dp * -1.0;
            if (!sleetStreakCameraCoords) {
                up = motion * -1.0;
            }
            up.Normalize();

            Vector3 in = camDir * -1;
            Vector3 right = up.Cross(in);
            right.Normalize();
            in = right.Cross(up);
            in.Normalize();

            rotateTranslate.elem[0][0] = right.x;
            rotateTranslate.elem[0][1] = right.y;
            rotateTranslate.elem[0][2] = right.z;
            rotateTranslate.elem[1][0] = up.x;
            rotateTranslate.elem[1][1] = up.y;
            rotateTranslate.elem[1][2] = up.z;
            rotateTranslate.elem[2][0] = in.x;
            rotateTranslate.elem[2][1] = in.y;
            rotateTranslate.elem[2][2] = in.z;

            rotateTranslate.Transpose();
        }

        if (vertexBuffer && indexBuffer) {
            // Do physics.
            UpdateParticles(motion, dp, particleIdx, particleIdx + nDrops[i]);

            // Make streaks of proper width and length using a scaling matrix
            Vector3 vScale(dropDiameter * 0.001 * sleetWidthMultiplier, streakLength,
                           dropDiameter * 0.001 * sleetWidthMultiplier);

            Vector4 worldMinSize(0,0,0,0);

            if (minParticlePixels > 0) {
                // Enforce minimum screen size at the near clip plane
                // Compute desired minimum size in NDC
                int x, y, w, h;
                ren->GetViewport(x, y, w, h);
                double nearClipNDC = ren->GetIsDirectX() ? 0.0 : -1.0;
                Vector4 minSize = Vector4(minParticlePixels / ((double)w * 0.5), minParticlePixels / ((double)h * 0.5),
                                          nearClipNDC, 1.0);
                // Deproject to world size
                worldMinSize = invProj * minSize;
            }

            Matrix4 scale;
            scale.elem[0][0] = vScale.x > worldMinSize.x ? vScale.x : worldMinSize.x;
            scale.elem[1][1] = vScale.y > worldMinSize.y ? vScale.y : worldMinSize.y;
            scale.elem[2][2] = 1.0;

            double halfStreak = -dy * 0.5;

            int dropsPerOscillation = (int)((float)nDrops[i] / (float)NUM_SLEET_TEXTURE_OSCILLATIONS);
            int oscCount = 0;
            int oscNum = 0;
            ren->EnableTexture(sleetTextures[oscNum], 0);

            if (precipShader) {
                float outputScale = Atmosphere::GetCurrentAtmosphere()->GetOutputScale();
                Renderer::GetInstance()->SetConstantVector(precipShader, "sl_outputScale", Vector3((double)outputScale, 0.0, 0.0));
                ren->SetConstantVector4(precipShader, "sl_lightingColor", Vector4(sleetColor.r, sleetColor.g, sleetColor.b, sleetColor.a));
                ren->BindShader(precipShader, VERTEX_PROGRAM);
            }

            // Set active buffers
            ren->SetVertexBuffer(vertexBuffer->GetHandle(), false);
            ren->SetIndexBuffer(indexBuffer->GetHandle());

            // Now, draw each particle.
            for (int j = particleIdx; j < particleIdx + nDrops[i]; j++) {
                if (numStreaksRendered > maxRenderedParticles)
                    break;

                const Vector3& v = particles[j].pos;

                // Cycle through the available images for different types of drop oscillation patterns.
                if (oscCount > dropsPerOscillation) {
                    oscNum++;
                    if (oscNum < NUM_SLEET_TEXTURE_OSCILLATIONS) {
                        ren->EnableTexture(sleetTextures[oscNum], 0);
                        if (precipShader) {
                            ren->SetConstantVector4(precipShader, "sl_lightingColor", Vector4(sleetColor.r, sleetColor.g, sleetColor.b, sleetColor.a));
                        } else {
                            ren->SetCurrentColor(sleetColor);
                        }
                    }
                    oscCount = 0;
                }

                oscCount++;

                // You might think doing a cull test against every single particle
                // would kill performance, but not doing this kills it a LOT more.
                bool cull = false;
                for (int i = 0; i < 5; i++) { // we purposely ignore the far clip plane
                    const Plane& p = adjustedFrustum.GetPlane(i);
                    const Vector3& N = p.GetNormal();
                    double origin = N.Dot(v) + p.GetDistance();
                    if (origin < -halfStreak) { // The "effective radius" is the streak length / 2.
                        cull = true;
                        break;
                    }
                }

                // Finally, draw the particle.
                if (!cull) {
                    rotateTranslate.elem[0][3] = v.x;
                    rotateTranslate.elem[1][3] = v.y;
                    rotateTranslate.elem[2][3] = v.z;

                    if (precipShader) {
                        Matrix4 mvp = proj * (savedModelview * rotateTranslate * scale);
                        ren->SetConstantMatrix(precipShader, "sl_modelViewProj", mvp);
                    } else {
                        ren->SetModelviewMatrix(savedModelview * rotateTranslate * scale);
                    }

                    ren->DrawStripDirect(indexBuffer->GetHandle(), 0, indexBuffer->GetNumIndices(), vertexBuffer->GetNumVertices());

                    numStreaksRendered++;
                }
            }

            particleIdx += nDrops[i];

        }
    }

    // Clean up our mess.
    ren->EnableDepthWrites(true);
    ren->EnableDepthReads(true);
    ren->EnableLighting(false);
    ren->EnableTexture2D(true);
    //ren->EnableBackfaceCulling(true);
    ren->UnbindShader(VERTEX_PROGRAM);

    previousCamPos = currentCamPos;

    ren->SetDepthRange(nearDepth, farDepth);

    ren->SetProjectionMatrix(savedProj);
    ren->SetModelviewMatrix(savedModelview);
}

void Sleet::UpdateParticles(const Vector3& motion, const Vector3& dp, int start, int end)
{
    Vector3 camPos = Atmosphere::GetCurrentAtmosphere()->GetCamPos();
    Vector3 center = camPos - dp;
#ifndef CUBE_VOLUME
    double  farClipSquared = farClip * farClip;
    double  nearClipSquared = nearClip * nearClip;
#endif

    // Enable OpenMP in your language settings to parallelize the particle physics update.
    // In practice, it doesn't really do much good on a dual core.
#ifdef _OPENMP
    #pragma omp parallel for \
    shared(start, end, camPos, center)
#endif
    for (int i = start; i < end; i++) {
#ifdef CUBE_VOLUME
        if ( fabs(particles[i].pos.x - camPos.x) > farClip ||
                fabs(particles[i].pos.y - camPos.y) > farClip ||
                fabs(particles[i].pos.z - camPos.z) > farClip )
#else
        double sqLen = (particles[i].pos - camPos).SquaredLength();
        if ( sqLen > farClipSquared || sqLen < nearClipSquared )
#endif
        {
            particles[i].InitializeParticlePosition(center, i);
        } else {
            particles[i].pos = particles[i].pos + motion;
        }
    } /* End parallel for loop */
}

