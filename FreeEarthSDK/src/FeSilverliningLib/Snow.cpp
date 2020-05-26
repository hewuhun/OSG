// Copyright (c) 2009-2015 Sundog Software LLC, all rights reserved worldwide

#include <FeSilverliningLib/Snow.h>
#include <FeSilverliningLib/VertexBuffer.h>
#include <FeSilverliningLib/Atmosphere.h>
#include <FeSilverliningLib/Configuration.h>
#include <FeSilverliningLib/Renderer.h>
#include <FeSilverliningLib/Utils.h>
#if defined(__INTEL_COMPILER)
#include <mathimf.h>
#else
#include <math.h>
#endif

//#define CUBE_VOLUME

#ifdef _OPENMP
#include <omp.h>
#endif

using namespace std;
using namespace SilverLining;

#define NUM_RADII 65536
static Vector3f randomOffsets[NUM_RADII];

#define RAND_RADIUS Atmosphere::GetCurrentAtmosphere()->GetRandomNumberGenerator()->UniformRandomIntRange(0, NUM_RADII - 1)

void SnowParticle::InitializeParticlePosition(const Vector3& camPos, int idxOffset)
{
    int index = RAND_RADIUS;
    index += idxOffset;
    index %= NUM_RADII;
    const Vector3f& offset = randomOffsets[index];
    mat.elem[0][3] = camPos.x + offset.x;
    mat.elem[1][3] = camPos.y + offset.y;
    mat.elem[2][3] = camPos.z + offset.z;
}

void SnowParticle::InitializeParticleRotation()
{
    const Vector3f& offset1 = randomOffsets[RAND_RADIUS];
    const Vector3f& offset2 = randomOffsets[RAND_RADIUS];
    Vector3 up (offset1.x, offset1.y, offset1.z);
    Vector3 right(offset2.x, offset2.y, offset2.z);
    right.Normalize();
    Vector3 in = right.Cross(up);
    in.Normalize();
    up = in.Cross(right);
    up.Normalize();

    mat.elem[0][0] = right.x;
    mat.elem[1][0] = right.y;
    mat.elem[2][0] = right.z;
    mat.elem[0][0] = up.x;
    mat.elem[1][1] = up.y;
    mat.elem[2][1] = up.z;
    mat.elem[0][2] = in.x;
    mat.elem[1][2] = in.y;
    mat.elem[2][2] = in.z;
}

Snow::Snow(const Atmosphere *atm)
{
    atmosphere = atm;

    isWet = false;
    indexBuffer = 0;
    vertexBuffer = 0;
    lastComputedIsWet = false;
    lastComputedIntensity = lastComputedFogDensity = -1;

    maxParticles = 100000;
    maxSceneParticles = 3000;
    maxIntensity = 3.0;
    farClip = 10.0;
    visibilityMultiplier = 1.0;
    lambda = 1.6;
    sizeMultiplier = 1.0;
    rotateParticles = true;
    nearClip = 0.1f * Atmosphere::GetUnitScale();
    minParticlePixels = 5.0;

    double r = 0.8, g = 0.8, b = 0.8;
    Configuration::GetIntValue("snow-max-particles", maxParticles);
    Configuration::GetIntValue("snow-max-rendered-particles", maxSceneParticles);
    Configuration::GetDoubleValue("snow-max-intensity", maxIntensity);
    Configuration::GetDoubleValue("snow-volume-radius", farClip);
    farClip *= Atmosphere::GetUnitScale();
    Configuration::GetDoubleValue("snow-visibility-multiplier", visibilityMultiplier);
    Configuration::GetDoubleValue("snow-lambda", lambda);
    Configuration::GetDoubleValue("snowflake-size-multiplier", sizeMultiplier);
    Configuration::GetBoolValue("snow-rotate-particles", rotateParticles);
    Configuration::GetDoubleValue("snow-minimum-pixels", minParticlePixels);

    Configuration::GetBoolValue("snow-use-depth-buffer", useDepthBuffer);
    wetSnowVelocity = 2.0;
    drySnowVelocity = 1.0;
    Configuration::GetDoubleValue("wet-snow-velocity", wetSnowVelocity);
    Configuration::GetDoubleValue("dry-snow-velocity", drySnowVelocity);

    Configuration::GetDoubleValue("snow-color-r", r);
    Configuration::GetDoubleValue("snow-color-g", g);
    Configuration::GetDoubleValue("snow-color-b", b);
    snowColor = Color(r, g, b);

    Configuration::GetDoubleValue("snow-near-clip", nearClip);
    nearClip *= Atmosphere::GetUnitScale();

    InitializeBuffers();
    InitializeParticles();

    Renderer::GetInstance()->LoadTextureFromFile("snow/Snowflake.TGA", &snowTexture, false, false);
}

Snow::~Snow()
{
    if (indexBuffer) SL_DELETE indexBuffer;
    if (vertexBuffer) SL_DELETE vertexBuffer;
    if (snowTexture) Renderer::GetInstance()->ReleaseTexture(snowTexture);
}

void Snow::SetIntensity(double R)
{
    if (R < 0) R = 0;
    if (R > maxIntensity) R = maxIntensity;

    intensity = R;

    if (isWet == lastComputedIsWet && R == lastComputedIntensity) {
        fogDensity = lastComputedFogDensity;
        return;
    }

    if (intensity == 0) {
        fogDensity = 0;
        return;
    }

    terminalVelocity = isWet ? wetSnowVelocity : drySnowVelocity; // m/s

    double C3 = isWet ? 0.072 : 0.017;
    double Vt = terminalVelocity * 100.0; // cm/s
    double intensityCmS = (intensity * 0.1) / 3600.0; // mm/hr -> cm/s

    // Reference: Rasmussen, Vivekanandan, Cole, Meyers, Masters, "The Estimation of Snowfall Rate Using Visibility",
    // 1999, Journal of Applied Meteorology, Vol. 38, pp 1542-1563

    double N0 = (6.0 * pow(lambda, 4.0) * intensityCmS) / (PI * C3 * 6.0 * Vt);

    double visibilityCm = (1.3 * C3 * Vt) / intensityCmS;

    visibility = visibilityCm * 0.01 * visibilityMultiplier * Atmosphere::GetUnitScale();
    terminalVelocity *= Atmosphere::GetUnitScale();

    // From Koschmeider equation
    fogDensity = (3.91 / visibility) / 1.3;

    double totalFlakes = 0;
    int i;

    for (i = 0; i < NUM_SNOWFLAKE_DIAMETERS; i++) {
        flakeDiameters[i] = SNOWFLAKE_MIN_DIAMETER + (i * SNOWFLAKE_DIAMETER_INCREMENT);
        double Dcm = flakeDiameters[i] * 0.1; // mm -> cm
        nFlakes[i] = N0 * exp(lambda * Dcm);
        totalFlakes += nFlakes[i];

        flakeDiameters[i] *= Atmosphere::GetUnitScale();
    }

    // Normalize and multiply by available particles.
    double totalParticles = (R / maxIntensity) * (double)maxParticles;
    for (i = 0; i < NUM_SNOWFLAKE_DIAMETERS; i++) {
        nFlakes[i] = (nFlakes[i] / totalFlakes) * totalParticles;
    }

    lastComputedIsWet = isWet;
    lastComputedIntensity = intensity;
    lastComputedFogDensity = fogDensity;
}

void Snow::Render(double dt, const Frustum& f, const Color& lightColor)
{
    if (intensity == 0) return;

    Renderer *ren = Renderer::GetInstance();

    // Clamp dt to something reasonable
    if (dt <= 0) return;

    if (intensity <= 0) return;

    if (dt > (1.0 / 20.0))
        dt = 1.0 / 20.0;

    Matrix4 savedProj;
    ren->GetProjectionMatrix(&savedProj);

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
    ren->SetCurrentColor(lightColor * snowColor);
    ren->EnableTexture(snowTexture, 0);

    Vector3 currentCamPos = Atmosphere::GetCurrentAtmosphere()->GetCamPos();

    double velocity, heading;
    atmosphere->GetConditions().GetWind(velocity, heading, atmosphere->GetConditions().GetLocation().GetAltitude());
    double windX, windZ;
    atmosphere->GetConditions().GetPrecipitationWind(windX, windZ);
    double dx = windX + sin(RADIANS(-heading)) * velocity;
    double dz = windZ + cos(RADIANS(-heading)) * velocity;

    int numFlakesRendered = 0;

    Matrix4 modelview;
    ren->GetModelviewMatrix(&modelview);
    // Vector pointing away from the eyepoint into the screen
    bool useNDC = true;
    Configuration::GetBoolValue("billboard-use-ndc", useNDC);
    Vector3 camDir;
    if (useNDC) {
        Vector3 ndcIn(0, 0, 1);
        // de-project to world space
        Matrix4 view = modelview;
        view.elem[0][3] = view.elem[1][3] = view.elem[2][3] = 0;
        Matrix4 mvp = savedProj * view;
        Matrix4 invMvp = mvp.Inverse();
        camDir = invMvp * ndcIn;
        camDir.Normalize();
    } else {
        camDir = Vector3(-modelview.elem[2][0], -modelview.elem[2][1], -modelview.elem[2][2]);
    }

    Configuration::GetDoubleValue("snowflake-size-multiplier", sizeMultiplier);
    Configuration::GetBoolValue("snow-rotate-particles", rotateParticles);

    for (int i = 0; i < NUM_SNOWFLAKE_DIAMETERS; i++) {
        double dy = -terminalVelocity * dt;

        Vector3 down = Vector3(0, dy, 0) * ren->GetBasis3x3();

        // Compute vector of raindrop's movement relative to the camera since the last frame
        Vector3 wind(Vector3(dx, 0, dz) * ren->GetBasis3x3() * 0.05 * dt * flakeDiameters[i]);

        Vector3 cameraMotion = (previousCamPos - currentCamPos);
        Vector3 particleMotion = down + wind;

        //Vector3 dp = (previousCamPos - currentCamPos) + particleMotion;

        Matrix4 billboard;

        if (!rotateParticles) {
            // Compute basis of motion vector, make a rotation matrix to properly orient
            // the flakes as a billboard using the motion vector as a fixed axis.
            Vector3 up = down * -1.0;
            up.Normalize();

            Vector3 in = camDir * -1;
            Vector3 right = up.Cross(in);
            right.Normalize();
            in = right.Cross(up);
            in.Normalize();

            billboard.elem[0][0] = right.x;
            billboard.elem[0][1] = right.y;
            billboard.elem[0][2] = right.z;
            billboard.elem[1][0] = up.x;
            billboard.elem[1][1] = up.y;
            billboard.elem[1][2] = up.z;
            billboard.elem[2][0] = in.x;
            billboard.elem[2][1] = in.y;
            billboard.elem[2][2] = in.z;

            billboard.Transpose();
        }

        Matrix4 modelview;
        ren->GetModelviewMatrix(&modelview);

        if (vertexBuffer && indexBuffer) {
            int startIdx = particleIdx;
            int endIdx = particleIdx + (int)nFlakes[i];

            // Do physics.
            UpdateParticles(particleMotion, cameraMotion, startIdx, endIdx);

            // Make streaks of proper width and length using a scaling matrix
            Matrix4 scale;
            double s = flakeDiameters[i] * 0.001 * sizeMultiplier;

            if (minParticlePixels > 0) {
                // Enforce minimum screen size at the near clip plane
                // Compute desired minimum size in NDC
                int x, y, w, h;
                ren->GetViewport(x, y, w, h);
                double nearClipNDC = ren->GetIsDirectX() ? 0.0 : -1.0;
                Vector4 minSize = Vector4(minParticlePixels / ((double)w * 0.5), minParticlePixels / ((double)h * 0.5),
                                          nearClipNDC, 1.0);
                // Deproject to world size
                Vector4 worldMinSize = invProj * minSize;

                double pointSize = worldMinSize.y; //worldMinSize.x > worldMinSize.y ? worldMinSize.x : worldMinSize.y;

                if (s < pointSize) s = pointSize;
            }

            scale.elem[0][0] = s;
            scale.elem[1][1] = s;
            scale.elem[2][2] = s;

            if (precipShader) {

                float outputScale = Atmosphere::GetCurrentAtmosphere()->GetOutputScale();
                Renderer::GetInstance()->SetConstantVector(precipShader, "sl_outputScale", Vector3((double)outputScale, 0.0, 0.0));

                ren->SetConstantVector4(precipShader, "sl_lightingColor", Vector4(lightColor.r * snowColor.r,
                                        lightColor.g * snowColor.g, lightColor.b * snowColor.b, lightColor.a * snowColor.a));
                ren->BindShader(precipShader, VERTEX_PROGRAM);
            }

            // Set the current buffers
            ren->SetVertexBuffer(vertexBuffer->GetHandle(), false);
            ren->SetIndexBuffer(indexBuffer->GetHandle());

            // Now, draw each particle.
            // Note - trying to parallelize this doesn't do any good, since you can't draw as you go,
            // and can't break out of the loop early. We tried, it actually hurt performance.
            for (int j = startIdx; j < endIdx; j++) {
                if (numFlakesRendered > maxSceneParticles)
                    break;

                Vector3 v (particles[j].mat.elem[0][3], particles[j].mat.elem[1][3], particles[j].mat.elem[2][3]);

                // You might think doing a cull test against every single particle
                // would kill performance, but not doing this kills it a LOT more.
                bool cull = false;

                for (int plane = 0; plane < 5; plane++) {
                    const Plane& p = adjustedFrustum.GetPlane(plane);
                    const Vector3& N = p.GetNormal();
                    double origin = N.Dot(v) + p.GetDistance();
                    if (origin < -s) {
                        cull = true;
                        break;
                    }
                }

                // Finally, draw the particle.
                if (!cull) {
                    if (!rotateParticles) {
                        for (int ii = 0; ii < 3; ii++) {
                            for (int jj = 0; jj < 3; jj++) {
                                particles[j].mat.elem[ii][jj] = billboard.elem[ii][jj];
                            }
                        }
                    }

                    if (precipShader) {
                        Matrix4 mvp = proj * (modelview * particles[j].mat * scale);
                        ren->SetConstantMatrix(precipShader, "sl_modelViewProj", mvp);
                    } else {
                        ren->SetModelviewMatrix(modelview * particles[j].mat * scale);
                    }

                    ren->DrawStripDirect(indexBuffer->GetHandle(), 0, indexBuffer->GetNumIndices(), vertexBuffer->GetNumVertices());

                    numFlakesRendered++;
                }
            }

            ren->UnsetVertexBuffer();
            ren->UnsetIndexBuffer();

            ren->UnbindShader(VERTEX_PROGRAM);

            particleIdx += (int)nFlakes[i];

        }

        ren->SetModelviewMatrix(modelview);
    }

    // Clean up our mess.
    ren->EnableDepthWrites(true);
    ren->EnableDepthReads(true);
    ren->EnableLighting(false);
    ren->EnableTexture2D(true);
    //ren->EnableBackfaceCulling(true);

    ren->SetDepthRange(nearDepth, farDepth);

    previousCamPos = currentCamPos;

    ren->SetProjectionMatrix(savedProj);
}

void Snow::InitializeBuffers()
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
                        particles.push_back(SnowParticle());
                    }

                    Color white(1, 1, 1, 1);

                    v[0].SetColor(white);
                    v[0].SetUV(0, 0);
                    v[0].x = -0.5;
                    v[0].y = 0.5;
                    v[0].z = 0;
                    v[0].w = 1.0f;
                    idx[0] = 0;

                    v[1].SetColor(white);
                    v[1].SetUV(1, 0);
                    v[1].x = 0.5;
                    v[1].y = 0.5;
                    v[1].z = 0;
                    v[1].w = 1.0f;
                    idx[1] = 1;

                    v[2].SetColor(white);
                    v[2].SetUV(0, 1);
                    v[2].x = -0.5;
                    v[2].y = -0.5;
                    v[2].z = 0;
                    v[2].w = 1.0f;
                    idx[2] = 2;

                    v[3].SetColor(white);
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

void Snow::InitializeParticles()
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

    SL_VECTOR(SnowParticle) ::iterator it;
    for (it = particles.begin(); it != particles.end(); it++) {
        if (rotateParticles) {
            (*it).InitializeParticleRotation();
        }
        (*it).InitializeParticlePosition(camPos, 0);

    }
}

void Snow::SetEffectRange(double fNear, double fFar, bool bUseDepthBuffer)
{
    if (fNear > 0 && fFar > 0 && fFar > fNear) {
        if (fNear != nearClip || fFar != farClip || bUseDepthBuffer != useDepthBuffer) {
            Precipitation::SetEffectRange(fNear, fFar, bUseDepthBuffer);
            InitializeParticles();
        }
    }
}

void Snow::UpdateParticles(const Vector3& flakeMotion, const Vector3& camMotion, int start, int end)
{
    const Vector3& camPos = Atmosphere::GetCurrentAtmosphere()->GetCamPos();
    Vector3 center = camPos - camMotion;
#ifndef CUBE_VOLUME
    double  farClipSquared = farClip * farClip;
    double  nearClipSquared = nearClip * nearClip;
#endif

    // Enable OpenMP in your language settings to parallelize the particle physics update.
    // In practice, it doesn't really do much good on a dual core.
#ifdef _OPENMP
    #pragma omp parallel for \
    shared(start, end, camPos, flakeMotion, center)
#endif
    for (int i = start; i < end; i++) {
#ifdef CUBE_VOLUME
        if ( fabs(particles[i].mat.elem[0][3] - camPos.x) > farClip ||
                fabs(particles[i].mat.elem[1][3] - camPos.y) > farClip ||
                fabs(particles[i].mat.elem[2][3] - camPos.z) > farClip )
#else
        Vector3 pos(particles[i].mat.elem[0][3], particles[i].mat.elem[1][3], particles[i].mat.elem[2][3]);
        double sqLen = (pos - camPos).SquaredLength();
        if ( sqLen > farClipSquared || sqLen < nearClipSquared )
#endif
        {
            particles[i].InitializeParticlePosition(center, i);
        } else {
            particles[i].mat.elem[0][3] += flakeMotion.x;
            particles[i].mat.elem[1][3] += flakeMotion.y;
            particles[i].mat.elem[2][3] += flakeMotion.z;
        }
    } /* End parallel for loop */
}

