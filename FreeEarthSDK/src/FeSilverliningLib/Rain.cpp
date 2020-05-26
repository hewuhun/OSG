// Copyright (c) 2009-2015 Sundog Software LLC, All rights reserved worldwide.

#include <FeSilverliningLib/Rain.h>
#include <FeSilverliningLib/ResourceLoader.h>
#include <FeSilverliningLib/VertexBuffer.h>
#include <FeSilverliningLib/Renderer.h>
#include <FeSilverliningLib/Matrix4.h>
#include <FeSilverliningLib/Configuration.h>
#include <FeSilverliningLib/Metaball.h>
#include <FeSilverliningLib/Utils.h>
#include <string.h>
#include <stdlib.h>
#include <sstream>

//#define CUBE_VOLUME

#ifdef _OPENMP
#include <omp.h>
#endif

using namespace SilverLining;
using namespace std;

#define NUM_RADII 65536
static Vector3f randomOffsets[NUM_RADII];

Rain::Rain(const Atmosphere *atm)
{
    lastComputedIntensity = lastComputedFogDensity = -1;
    atmosphere = atm;
    maxParticles = 100000;
    maxIntensity = 30.0;
    rainAlphaThreshold = 0.01f;
    rainWidthMultiplier = 1.0f;
    maxRenderedParticles = 3000;
    visibilityMultiplier = 1.0;
    visibility = 0;
    fogDensity = 0;
    rainConstantAlpha = 0;
    rainStreakFrameTime = 0;
    rainStreakCameraCoords = false;
    minParticlePixels = 5.0;
    rainVelocityFactor = 1.0;

    Configuration::GetBoolValue("rain-streak-camera-coords", rainStreakCameraCoords);
    Configuration::GetIntValue("rain-max-particles", maxParticles);
    Configuration::GetIntValue("rain-max-rendered-particles", maxRenderedParticles);
    Configuration::GetDoubleValue("rain-max-intensity", maxIntensity);
    Configuration::GetFloatValue("rain-alpha-threshold", rainAlphaThreshold);
    Configuration::GetFloatValue("rain-streak-width-multiplier", rainWidthMultiplier);
    Configuration::GetDoubleValue("rain-visibility-multiplier", visibilityMultiplier);
    Configuration::GetDoubleValue("rain-near-clip", nearClip);
    nearClip *= Atmosphere::GetUnitScale();
    Configuration::GetDoubleValue("rain-constant-alpha", rainConstantAlpha);
    Configuration::GetDoubleValue("rain-streak-frame-time", rainStreakFrameTime);
    Configuration::GetDoubleValue("rain-volume-radius", farClip);
    farClip *= (float)Atmosphere::GetUnitScale();
    Configuration::GetDoubleValue("rain-minimum-pixels", minParticlePixels);
    Configuration::GetBoolValue("rain-use-depth-buffer", useDepthBuffer);
    Configuration::GetDoubleValue("rain-velocity-factor", rainVelocityFactor);

    InitializeBuffers();
    InitializePositions();
    LoadStreakTextures();
    LoadStreakColors();
    ComputeOscillationTimes();
}

#define RAND_RADIUS Atmosphere::GetCurrentAtmosphere()->GetRandomNumberGenerator()->UniformRandomIntRange(0, NUM_RADII - 1)

void RainParticle::InitializeParticlePosition(const Vector3& camPos, int idxOffset)
{
    int index = RAND_RADIUS;
    index += idxOffset;
    index %= NUM_RADII;
    const Vector3f& offset = randomOffsets[index];

    pos.x = camPos.x + offset.x;
    pos.y = camPos.y + offset.y;
    pos.z = camPos.z + offset.z;
}

void Rain::ComputeOscillationTimes()
{
    const double kSurfaceTension = 0.0728; // N/m2
    const double kDensityWater = 1000.0; // kg/m3
    for (int i = 0; i < NUM_RAINDROP_DIAMETERS; i++) {
        double D = MIN_RAINDROP_DIAMETER + i * RAINDROP_DIAMETER_INCREMENT;
        D *= 0.001; // convert from mm to m

        // Oscillation frequency per [Garg Nayar 2006]
        double w2 = TWOPI * sqrt( (8.0 * kSurfaceTension) / (4.0 * PI * PI * kDensityWater * D * D * D) );
        oscillationTimes[i] = TWOPI / w2;
    }
}

void Rain::InitializePositions()
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
        } while (r2 < (nearClip * nearClip) || r2 > (farClip * farClip));

        randomOffsets[i] = Vector3f(x, y, z);
#endif
    }

    SL_VECTOR(RainParticle) ::iterator it;
    for (it = particles.begin(); it != particles.end(); it++) {
        (*it).InitializeParticlePosition(camPos, 0);

    }
}

void Rain::SetEffectRange(double fNear, double fFar, bool bUseDepthBuffer)
{
    if (fNear > 0 && fFar > 0 && fFar > fNear) {
        if (fNear != nearClip || fFar != farClip || bUseDepthBuffer != useDepthBuffer) {
            Precipitation::SetEffectRange(fNear, fFar, bUseDepthBuffer);
            InitializePositions();
        }
    }
}

void Rain::InitializeBuffers()
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
                        particles.push_back(RainParticle());
                    }

                    Color startColor(1, 1, 1, 1);
                    Color endColor(1, 1, 1, 1);

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

void Rain::LoadStreakTextures()
{
    Renderer *ren = Renderer::GetInstance();

    for (int i = 0; i < NUM_RAIN_TEXTURE_ANGLES; i++) {
        double camAngle = (double)i * (double)RAIN_TEXTURE_ANGLE_INCREMENT;
        camAngle = 90.0 - camAngle;
        camAngle = RADIANS(camAngle);
        angleThresholds[i] = cos(camAngle);

        for (int j = 0; j < NUM_RAIN_TEXTURE_OSCILLATIONS; j++) {
            rainTextures[i][j] = 0;

            char filePath[1024];
            sprintf(filePath, "rain/cv%d_osc%d.TGA", i * RAIN_TEXTURE_ANGLE_INCREMENT, j);

            rainTextures[i][j] = 0;
            ren->LoadTextureFromFile(filePath, &rainTextures[i][j], true, true);
        }
    }
}

void Rain::LoadStreakColors()
{
    char *data;
    unsigned int dataLen;
    if (Atmosphere::GetResourceLoader()->LoadResource("rain/normalized_env_max.txt", data, dataLen, true)) {
        std::stringstream *s = SL_NEW std::stringstream(string(data));
        for (int i = 0; i < NUM_RAIN_TEXTURE_ANGLES; i++) {
            char buf[1024];
            s->getline(buf, 1024);

            int osc = 0;
            char *pch = strtok(buf, " \t\n");
            while (pch != NULL) {
                double c = LocaleSafeAtoF(pch);
                if (osc < NUM_RAIN_TEXTURE_OSCILLATIONS) {
                    rainColors[i][osc++] = Color(c, c, c, 1.0);
                }
                pch = strtok(NULL, " \t\r\n");
            }
        }

        Atmosphere::GetResourceLoader()->FreeResource(data);
        SL_DELETE s;
    }
}

Rain::~Rain()
{
    if (indexBuffer) SL_DELETE indexBuffer;
    if (vertexBuffer) SL_DELETE vertexBuffer;

    for (int i = 0; i < NUM_RAIN_TEXTURE_ANGLES; i++) {
        for (int j = 0; j < NUM_RAIN_TEXTURE_OSCILLATIONS; j++) {
            if (rainTextures[i][j]) Renderer::GetInstance()->ReleaseTexture(rainTextures[i][j]);
        }
    }
}

void Rain::SetIntensity(double R)
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
    double Nd[NUM_RAINDROP_DIAMETERS];
    double NdTotal = 0;
    int i;

    for (i = 0; i < NUM_RAINDROP_DIAMETERS; i++) {
        double D = MIN_RAINDROP_DIAMETER + (double)i * RAINDROP_DIAMETER_INCREMENT;
        Nd[i] = N0 * exp(-A * D * 0.001);
        NdTotal += Nd[i];
        terminalVelocities[i] = pow( 3.778 * D, 0.67) * Atmosphere::GetUnitScale() * rainVelocityFactor;
        dropDiameters[i] = D * Atmosphere::GetUnitScale();
    }

    // Normalize and multiply by available particles.
    double totalParticles = (R / maxIntensity) * (double)maxParticles;
    for (i = 0; i < NUM_RAINDROP_DIAMETERS; i++) {
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

void Rain::Render(double dt, const Frustum& f, const Color& lightColor)
{
    if (intensity == 0) return;

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

    Matrix4 savedTexture;
    // Assume texture matrix is identity, to prevent stalling to get whatever it really is.
    //ren->GetTextureMatrix(&savedTexture);

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
    if (rainStreakFrameTime > 0) {
        streakWind = Vector3(Vector3(dx, 0, dz) * ren->GetBasis3x3() * 0.2 * rainStreakFrameTime);
    } else {
        streakWind = wind;
    }

    // In real life, rain streaks aren't that easy to see. If you want to fudge
    // how prominent the rain streaks are, we let you do that...
    double rainBrightness = 1.0;
    Configuration::GetDoubleValue("rain-streak-brightness", rainBrightness);

    Configuration::GetFloatValue("rain-streak-width-multiplier", rainWidthMultiplier);

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

    for (int i = 0; i < NUM_RAINDROP_DIAMETERS; i++) {
        // Compute terminal velocity of raindrop based on its diameter from Atlas and Ulbrich (1977)
        double v = terminalVelocities[i];
        double dropDiameter = dropDiameters[i];

        // Compute texture scale for v coord
        double vScale = dt / oscillationTimes[i];
        Matrix4 textureMatrix;
        textureMatrix.elem[1][1] = vScale;
        ren->SetTextureMatrix(textureMatrix);

        double dy = -v * dt;

        Vector3 down = Vector3(0, dy, 0) * ren->GetBasis3x3();
        Vector3 streakDown;
        if (rainStreakFrameTime > 0) {
            streakDown = Vector3(0, -v * rainStreakFrameTime, 0) * ren->GetBasis3x3();
        } else {
            streakDown = down;
        }

        Vector3 motion = down + wind;
        Vector3 streakMotion = streakDown + streakWind;

        // Compute vector of raindrop's movement relative to the camera since the last frame
        Vector3 dp = (previousCamPos - currentCamPos) + motion;
        Vector3 streakDp = (previousCamPos - currentCamPos) + streakMotion;
        if (!rainStreakCameraCoords) {
            streakDp = streakMotion;
        }

        Matrix4 rotateTranslate;

        double streakLength = streakDp.Length();
        int angleIndex = 0;
        double rainAlpha = 0.0;

        if (dp.SquaredLength() > 0) {
            // Select the proper camera angle texture index
            // Streak textures based on Garg, K., and Nayark, S.K, "Photorealistic Rendering of Rain Streaks",
            // Proceedings of ACM SIGGRAPH 2006, 996-1002

            Vector3 rainDir = down;
            rainDir.Normalize();
            camDir.Normalize();
            double cosT = fabs(camDir.Dot(rainDir));

            for (int k = 0; k < NUM_RAIN_TEXTURE_ANGLES; k++) {
                if (cosT > angleThresholds[k]) angleIndex = k;
            }

            // The streak textures are scaled to use the available dynamic range.
            // rainColors[] maps these back to their real intensities. We modulate
            // by the available light, and then compute alpha based on Garg, K., and
            // Nayar, S.K. 2005. When Does a Camera See Rain? ICCV, 1067-1074
            Color rainColor = lightColor * rainColors[angleIndex][0];

            if (rainConstantAlpha > 0) {
                rainAlpha = rainConstantAlpha;
            } else {
                rainAlpha = ((2.0 * dropDiameter * 0.001) / (down.Length())) * rainBrightness;
            }

            rainColor.a = (float)rainAlpha;

            ren->SetCurrentColor(rainColor);

            // Compute basis of motion vector, make a rotation matrix to properly orient
            // the streaks as a billboard using the motion vector as a fixed axis.
            Vector3 up = dp * -1.0;
            if (!rainStreakCameraCoords) {
                up = motion * -1.0;
            }

            up.Normalize();

            Vector3 in = camDir * -1;
            Vector3 right = up.Cross(in);
            right.Normalize();

            // Handle case where up == in
            if (right.SquaredLength() == 0) right = Vector3(1,0,0);

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

        if (vertexBuffer && indexBuffer && rainAlpha > rainAlphaThreshold) {
            // Do physics.
            UpdateParticles(motion, dp, particleIdx, particleIdx + nDrops[i]);

            // Make streaks of proper width and length using a scaling matrix
            Vector3 vScale(dropDiameter * 0.001 * rainWidthMultiplier, streakLength,
                           dropDiameter * 0.001 * rainWidthMultiplier);

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

            int dropsPerOscillation = (int)((float)nDrops[i] / (float)NUM_RAIN_TEXTURE_OSCILLATIONS);
            int oscCount = 0;
            int oscNum = 0;
            ren->EnableTexture(rainTextures[angleIndex][oscNum], 0);

            if (precipShader) {
                float outputScale = Atmosphere::GetCurrentAtmosphere()->GetOutputScale();
                Renderer::GetInstance()->SetConstantVector(precipShader, "sl_outputScale", Vector3((double)outputScale, 0.0, 0.0));
                ren->SetConstantVector4(precipShader, "sl_lightingColor", Vector4(rainColor.r, rainColor.g, rainColor.b, rainColor.a));
                ren->BindShader(precipShader, VERTEX_PROGRAM);
            }

            // Make the vertex and index buffer current.
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
                    if (oscNum < NUM_RAIN_TEXTURE_OSCILLATIONS) {
                        ren->EnableTexture(rainTextures[angleIndex][oscNum], 0);
                        rainColor = Color(lightColor * rainColors[angleIndex][oscNum]);
                        rainColor.a = (float)rainAlpha;

                        if (precipShader) {
                            ren->SetConstantVector4(precipShader, "sl_lightingColor", Vector4(rainColor.r, rainColor.g, rainColor.b, rainColor.a));
                        } else {
                            ren->SetCurrentColor(rainColor);
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

                    ren->DrawStripDirect(indexBuffer->GetHandle(), 0, indexBuffer->GetNumIndices(),
                                         vertexBuffer->GetNumVertices());

                    numStreaksRendered++;
                }
            }

            ren->UnsetIndexBuffer();
            ren->UnsetVertexBuffer();
            ren->UnbindShader(VERTEX_PROGRAM);

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
    ren->SetTextureMatrix(savedTexture);
}

void Rain::UpdateParticles(const Vector3& motion, const Vector3& dp, int start, int end)
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

