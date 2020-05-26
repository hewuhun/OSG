// Copyright (c) 2013-2015 Sundog Software LLC. All rights reserved worldwide.

#include <FeSilverliningLib/EnvironmentMap.h>
#include <FeSilverliningLib/Atmosphere.h>
#include <FeSilverliningLib/Configuration.h>
#include <FeSilverliningLib/Renderer.h>
#include <FeSilverliningLib/Utils.h>

using namespace SilverLining;

bool EnvironmentMap::drawingEnvMap = false;

EnvironmentMap::EnvironmentMap(Atmosphere *atm) : atmosphere(atm), geocentric(false), lastFaceRendered(0), floatingPoint(false)
{
    cubeMapDim = 256;
    Configuration::GetIntValue("environment-map-size", cubeMapDim);

    enableStars = false;
    Configuration::GetBoolValue("environment-map-enable-stars", enableStars);

    neverDrawClouds = false;
    Configuration::GetBoolValue("environment-map-never-draw-clouds", neverDrawClouds);

    cubeMap = 0;
}

EnvironmentMap::~EnvironmentMap()
{
    if (cubeMap) {
        Renderer::GetInstance()->ReleaseRenderTextureCube(cubeMap);
    }
}

bool EnvironmentMap::Create(int facesToRender, bool pFloatingPoint, bool drawClouds, bool drawSunAndMoon)
{
    Renderer *ren = Renderer::GetInstance();

    if (cubeMap && floatingPoint != pFloatingPoint) {
        ren->ReleaseRenderTextureCube(cubeMap);
        cubeMap = 0;
    }

    floatingPoint = pFloatingPoint;

    if (cubeMap == 0) {
        ren->InitRenderTextureCube(cubeMapDim, cubeMapDim, &cubeMap, pFloatingPoint);
    }

    if (cubeMap && atmosphere) {
        drawingEnvMap = true;

        int savedX, savedY, savedW, savedH;

        double savedCamera[16], savedProjection[16];

        const double *cam = atmosphere->GetCameraMatrix();
        const double *proj = atmosphere->GetProjectionMatrix();
        for (int i = 0; i < 16; i++) {
            savedCamera[i] = cam[i];
            savedProjection[i] = proj[i];
        }

        bool atmosphereVP = true;
        if (!atmosphere->GetViewport(savedX, savedY, savedW, savedH)) {
            ren->GetViewport(savedX, savedY, savedW, savedH);
            atmosphereVP = false;
        }

        float nearDepth, farDepth;
        ren->GetDepthRange(nearDepth, farDepth);
        ren->SetDepthRange(0.0f, 1.0f);

        Vector3 from = Atmosphere::GetCurrentAtmosphere()->GetCamPos();

        ren->PushTransforms();

        double znear, zfar;
        ren->GetNearFarClip(znear, zfar);
        if (znear < 0) {
            znear = 10.0;
            zfar = 100000.0;
        }

        ren->SetProjectionMatrix(znear, zfar, HALFPI, HALFPI);

        int faceNum = lastFaceRendered;
        for (int face = 0; face < facesToRender; face++) {
            faceNum++;
            if (faceNum > 5) faceNum = 0;
            CubeFace cubeFace = (CubeFace)faceNum;
            lastFaceRendered = faceNum;

            if (ren->MakeRenderTextureCubeCurrent(cubeMap, true, cubeFace)) {
                if (atmosphereVP) {
                    atmosphere->SetViewport(0, 0, cubeMapDim, cubeMapDim);
                } else {
                    ren->SetViewport(0, 0, cubeMapDim, cubeMapDim);
                }

                Vector3 to, up;
                double offset = 100000.0;
                switch (cubeFace) {
                case POSX:
                    to = from + Vector3(offset, 0, 0);
                    up = Vector3(0, -1, 0);
                    break;

                case NEGX:
                    to = from + Vector3(-offset, 0, 0);
                    up = Vector3(0, -1, 0);
                    break;

                case POSY:
                    to = from + Vector3(0, offset, 0);
                    up = Vector3(0, 0, 1);
                    break;

                case NEGY:
                    to = from + Vector3(0, -offset, 0);
                    up = Vector3(0, 0, -1);
                    break;

                case POSZ:
                    to = from + Vector3(0, 0, offset);
                    up = Vector3(0, -1, 0);
                    break;

                case NEGZ:
                    to = from + Vector3(0, 0, -offset);
                    up = Vector3(0, -1, 0);
                    break;
                }

                ren->SetModelviewLookat(from, to, up);
                Matrix4 mv, proj;
                ren->GetModelviewMatrix(&mv);
                ren->GetProjectionMatrix(&proj);

                atmosphere->SetCameraMatrix(mv.ToArray());
                atmosphere->SetProjectionMatrix(proj.ToArray());

                atmosphere->DrawSky(true, geocentric, 0.0, enableStars, true, drawSunAndMoon);

                atmosphere->DrawObjects(drawClouds && !neverDrawClouds, false);

                ren->BindRenderTextureCube(cubeMap);
            }
        }

        ren->SetDepthRange(nearDepth, farDepth);

        if (atmosphereVP) {
            atmosphere->SetViewport(savedX, savedY, savedW, savedH);
        } else {
            ren->SetViewport(savedX, savedY, savedW, savedH);
        }

        ren->PopTransforms();

        atmosphere->SetCameraMatrix(savedCamera);
        atmosphere->SetProjectionMatrix(savedProjection);

        drawingEnvMap = false;
        return true;
    }

    return false;
}

void *EnvironmentMap::GetNativeTexture() const
{
    if (cubeMap) {
        TextureHandle texture;
        Renderer::GetInstance()->GetRenderTextureCubeTextureHandle(cubeMap, &texture);
        return Renderer::GetInstance()->GetNativeTexture(texture);
    }

    return 0;
}

