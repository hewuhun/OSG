// Copyright 2006-2013 Sundog Software. All rights reserved worldwide.

#include <FeSilverliningLib/CirrusCloud.h>
#include <FeSilverliningLib/Configuration.h>
#include <FeSilverliningLib/Metaball.h>
#include <FeSilverliningLib/CloudLayer.h>
#include <FeSilverliningLib/Configuration.h>
#include <FeSilverliningLib/Billboard.h>
#include <FeSilverliningLib/Sky.h>
#include <FeSilverliningLib/CrepuscularRays.h>
#include <FeSilverliningLib/ShadowMap.h>
#include <stdlib.h>

using namespace SilverLining;
using namespace std;

CirrusCloud::CirrusCloud(CloudLayer *layer) : Cloud(layer), texture(0), vb(0), ib(0), fadeFalloff(5.0)
{
}

void CirrusCloud::Init(double w, double h)
{
    width = w;
    height = h;

    extinction = 0.5f;
    albedo = 0.9f;
    Configuration::GetFloatValue("cirrus-extinction", extinction);
    Configuration::GetFloatValue("cirrus-albedo", albedo);
    Configuration::GetFloatValue("cirrus-fade-falloff", fadeFalloff);

    Renderer *ren = Renderer::GetInstance();
    char tgaPath[1024];
    int whichCirrus = Atmosphere::GetCurrentAtmosphere()->GetRandomNumberGenerator()->UniformRandomIntRange(0, 1);
    switch (whichCirrus) {
    case 0:
        sprintf(tgaPath, "cirrus.tga");
        break;

    case 1:
        sprintf(tgaPath, "cirrus2.tga");
        break;
    };

    bool hires = false;
    Configuration::GetBoolValue("cirrus-hires", hires);
    if (hires) {
        sprintf(tgaPath, "cirrus-hires.tga");
    }

    ren->LoadTextureFromFile(tgaPath, &texture, false, true);

    gridDim = 10;
    Configuration::GetIntValue("cirrus-grid-dimension", gridDim);
    bool curve = true;
    Configuration::GetBoolValue("cirrus-round-earth", curve);
    double earthRadius = 6371000;
    Configuration::GetDoubleValue("earth-radius-meters-polar", earthRadius);
    earthRadius *= Atmosphere::GetUnitScale();

    double radius = earthRadius;

    if (parentCloudLayer->GetCurveTowardGround()) {
        double baseWidth = parentCloudLayer->GetBaseWidth();
        double baseLength = parentCloudLayer->GetBaseLength();
        double a = sqrt(baseWidth * baseWidth + baseLength * baseLength) / 2.0;
        double b = parentCloudLayer->GetBaseAltitude();
        radius = (a*a + b*b) / (2.0 * b);
        curve = true;
    }

    vb = SL_NEW VertexBuffer(gridDim * gridDim);
    if (vb && vb->LockBuffer()) {
        Vertex *verts = vb->GetVertices();

        if (verts) {
            float hw = (float)w * 0.5f;
            float hh = (float)h * 0.5f;

            float incX = (float)w / (float)(gridDim - 1);
            float incY = (float)h / (float)(gridDim - 1);
            float incT = 1.0f / (float)(gridDim - 1);

            for (int row = 0; row < gridDim; row++) {
                for (int col = 0; col < gridDim; col++) {
                    int idx = row * gridDim + col;
                    Vector3 v(-hw + col * incX, 0, hh - row * incY);
                    if (curve) {
                        double dist = v.Length();
                        if (dist < radius) {
                            double displacement = radius - (sqrt(radius * radius - dist * dist));
                            v.y = -displacement;
                        }
                    }

                    verts[idx].x = (float)v.x;
                    verts[idx].y = (float)v.y;
                    verts[idx].z = (float)v.z;
                    verts[idx].w = 1.0f;
                    verts[idx].SetUV(col * incT, row * incT);
                    verts[idx].SetColor(Color(1.0,1.0,1.0));
                }
            }
        }
        vb->UnlockBuffer();
    }

    numVerts = gridDim * gridDim;

    nIndices = (gridDim - 1) * (gridDim * 2 + 2);

    ib = SL_NEW IndexBuffer(nIndices);
    if (ib && ib->LockBuffer()) {
        Index *indices = ib->GetIndices();
        int idx = 0;
        for (int strip = 0; strip < gridDim - 1; strip++) {
            for (int col = 0; col < gridDim; col++) {
                Index a = (Index)(strip * gridDim + col);
                Index b = (Index)((strip + 1) * gridDim + col);

                indices[idx++] = a;
                indices[idx++] = b;
            }

            // Degenerate triangles to wrap back to next strip
            indices[idx] = indices[idx-1];
            ++idx;
            indices[idx] = (Index)((strip+1) * gridDim);

            idx++;
        }
        ib->UnlockBuffer();
    }

    hdrShader = Renderer::GetInstance()->LoadShaderFromFile("Shaders/CirrusHDR.cg", VERTEX_PROGRAM);
}

void CirrusCloud::ReloadShaders()
{
    Renderer *ren = Renderer::GetInstance();
    if (ren) {
        ren->DeleteShader(hdrShader);
        hdrShader = ren->LoadShaderFromFile("Shaders/CirrusHDR.cg", VERTEX_PROGRAM);
    }
}

CirrusCloud::~CirrusCloud()
{
    Renderer *ren = Renderer::GetInstance();

    if (texture) {
        ren->ReleaseTexture(texture);
    }

    if (vb) {
        SL_DELETE vb;
    }

    if (ib) {
        SL_DELETE ib;
    }
}

void CirrusCloud::ApplyColor(const Color& c)
{
    if (appliedColor != c) {
        appliedColor = c;
        bool fade = GetParentCloudLayer()->GetFadeTowardEdges() && GetParentCloudLayer()->GetIsInfinite();

        float halfGrid = (float)gridDim * 0.5f;

        if (vb && vb->LockBuffer()) {
            Vertex *verts = vb->GetVertices();
            if (verts) {
                for (int row = 0; row < gridDim; row++) {
                    for (int col = 0; col < gridDim; col++) {
                        Color vertColor = c;
                        if (fade) {
                            float dx = (row - halfGrid) / halfGrid;
                            float dy = (col - halfGrid) / halfGrid;
                            float distFromCenter = sqrt(dx * dx + dy * dy);
                            vertColor.a *= exp(-fadeFalloff * distFromCenter * distFromCenter * distFromCenter);
                        }

                        int idx = row * gridDim + col;
                        verts[idx].SetColor(vertColor);
                    }
                }
            }
            vb->UnlockBuffer();
        }
    }
}

bool CirrusCloud::Cull(const Frustum& f)
{

    return false;
}

bool CirrusCloud::Draw(int pass, const Vector3 &lightPos, const Vector3& lightDir,
                       const Color &lightColor, bool invalid, const Sky *sky)
{
    if (pass == 0) {
        Color nLightColor;
        nLightColor = lightColor * extinction //* (float)(lightDir.Dot(GetParentCloudLayer()->GetLocalUpVector()))
                      + Metaball::GetAmbientColor() * albedo;
        nLightColor.ClampToUnitOrLess();
        cloudColor = nLightColor;
    } else {
        Vector3 dir = GetWorldPosition() - Atmosphere::GetCurrentAtmosphere()->GetCamPos();
        dir.Normalize();
        dir = dir * Renderer::GetInstance()->GetInverseBasis3x3();
        if (dir.y < 0) dir.y = 0;
        fogColor = sky->SkyColorAt(dir);

        Renderer::GetInstance()->SubmitBlendedObject(this);
    }

    return true;
}

void CirrusCloud::DrawBlendedObject()
{
    // Don't cast rays from cirrus
    Atmosphere *atm = Atmosphere::GetCurrentAtmosphere();
    if (atm) {
        const CrepuscularRays *rays = atm->GetCrepuscularRays();
        if (rays && rays->IsRendering()) return;
        const ShadowMap *sm = atm->GetShadowMapObject();
        if (sm && sm->IsRendering()) return;
    }

    Renderer *ren = Renderer::GetInstance();

    cloudColor.a = (float)fade * (float)alpha;
    ApplyColor(cloudColor);

    Vector3 camPos = Atmosphere::GetCurrentAtmosphere()->GetCamPos();
    camPos = camPos * ren->GetInverseBasis3x3();
    double centerY = GetParentCloudLayer()->GetBaseAltitudeGeocentric();

    double layerX, layerZ;
    GetParentCloudLayer()->GetLayerPosition(layerX, layerZ);

    bool viewingFromEdge = ((camPos.x < (layerX - GetParentCloudLayer()->GetBaseWidth() * 0.5)) ||
                            (camPos.x > (layerX + GetParentCloudLayer()->GetBaseWidth() * 0.5)) ||
                            (camPos.z > (layerZ + GetParentCloudLayer()->GetBaseLength() * 0.5)) ||
                            (camPos.z < (layerZ - GetParentCloudLayer()->GetBaseLength() * 0.5)));

    bool drawTop = (camPos.y >= (centerY + GetParentCloudLayer()->GetThickness()));
    bool drawBottom = ((camPos.y <= centerY) || viewingFromEdge);

    if (!(drawTop || drawBottom)) return;

    ren->PushTransforms();
    //ren->PushAllState();

    bool fogOn = ren->GetFogEnabled();
    if (!fogOn) {
        ren->EnableFog(true);
        ren->ConfigureFog(Billboard::GetFogDensity(), 1, 100000, fogColor);
    }

    // Set the blend and z buffer modes
#ifdef ANDROID
    ren->EnableBlending(SRCALPHA, ONE);
#else
    ren->EnableBlending(SRCALPHA, INVSRCALPHA);
#endif
    ren->EnableDepthReads(true);
    ren->EnableDepthWrites(false);
    ren->EnableTexture2D(true);
    ren->EnableLighting(false);
    ren->EnableBackfaceCulling(false);

    Matrix4 mv;
    ren->GetModelviewMatrix(&mv);

    Matrix4 xlate;
    if (GetParentCloudLayer()->GetIsInfinite()) {
        double deltaY = centerY;
        if (drawTop) deltaY += GetParentCloudLayer()->GetThickness();
        deltaY -= camPos.y;
        Vector3 offset = GetParentCloudLayer()->GetLocalUpVector() * deltaY;

        Matrix4 orientation = mv;
        orientation.elem[0][3] = orientation.elem[1][3] = orientation.elem[2][3] = 0;

        xlate.elem[0][3] = offset.x;
        xlate.elem[1][3] = offset.y;
        xlate.elem[2][3] = offset.z;
        xlate.elem[3][3] = 1;

        ren->SetModelviewMatrix(orientation * xlate * ren->GetInverseBasis4x4());
    } else {
        // Translate to the cloud position
        Vector3 offset(0, 0, 0);
        if (drawTop) {
            offset = GetParentCloudLayer()->GetLocalUpVector() * GetParentCloudLayer()->GetThickness();
        }

        Vector3 pos = GetWorldPosition() + offset;
        xlate.elem[0][3] = pos.x;
        xlate.elem[1][3] = pos.y;
        xlate.elem[2][3] = pos.z;
        ren->MultiplyModelviewMatrix(xlate * ren->GetInverseBasis4x4());
    }

    ren->EnableTexture(texture, 0);

    if (hdrShader) {
        float outputScale = Atmosphere::GetCurrentAtmosphere()->GetOutputScale();
        ren->SetConstantVector(hdrShader, "sl_outputScale", Vector3((double)outputScale, 0.0, 0.0));
        Matrix4 modelview, proj;
        ren->GetModelviewMatrix(&modelview);
        ren->SetConstantMatrix(hdrShader, "sl_modelView", modelview);
        ren->GetProjectionMatrix(&proj);
        Matrix4 mvp = proj * modelview;
        ren->SetConstantMatrix(hdrShader, "sl_modelViewProj", mvp);
        Vector4 fog(fogColor.r, fogColor.g, fogColor.b,
                    Billboard::GetFogDensity());
        ren->SetConstantVector4(hdrShader, "sl_fogColorAndDensity", fog);
        ren->SetConstantVector4(hdrShader, "sl_lightingColor", Vector4(cloudColor.r, cloudColor.g, cloudColor.b, cloudColor.a));
        ren->BindShader(hdrShader, VERTEX_PROGRAM);
    }

    vb->DrawIndexedStrip(*ib, true);

    ren->UnbindShader(VERTEX_PROGRAM);

    ren->SetModelviewMatrix(mv);
    ren->DisableBlending();
    //ren->EnableDepthWrites(true);

    if (!fogOn) {
        ren->EnableFog(false);
    }

    //ren->PopAllState();
    ren->PopTransforms();
}

bool CirrusCloud::Update(unsigned long now, bool forceUpdate)
{
    return false;
}

Vector3 CirrusCloud::GetSortPosition() const
{
    double x, y, z;
    GetParentCloudLayer()->GetSortPosition(x, y, z);
    return Vector3(x, y, z);
}

double CirrusCloud::GetDistance(const Vector3& from, const Vector3& to, const Matrix4& mv, bool rightHanded) const
{
    const double farFarAway = 1000000;

    // Find ray / plane intersection point.
    Vector3 xformPos = GetWorldPosition() * Renderer::GetInstance()->GetInverseBasis3x3();
    double D = xformPos.y;

    Vector3 v = to - from;
    Vector3 Rd = v;
    Rd.Normalize();

    Vector3 R0 = from;
    Vector3 Pn = Vector3(0, -1, 0) * Renderer::GetInstance()->GetBasis3x3();

    const double epsilon = 2.4E-7;

    double Vd = Pn.Dot(Rd);
    if (Vd >= -epsilon) {
        // ray points away from plane, try the other plane normal
        Pn = Vector3(0, 1, 0) * Renderer::GetInstance()->GetBasis3x3();
        Vd = Pn.Dot(Rd);
        D = -D;
    }

    if (Vd >= -epsilon) return farFarAway;

    double V0 = -(Pn.Dot(R0) + D);
    double t = V0 / Vd;

    if (t < 0) return farFarAway; // does not intersect

    Vector3 Pi(R0.x + Rd.x * t, R0.y + Rd.y * t, R0.z + Rd.z * t);

    double dist = (((mv * Pi).z));

    return rightHanded ? -dist : dist;
}

void CirrusCloud::GetSize(double& pwidth, double& pdepth, double& pheight) const
{
    pwidth = width;
    pdepth = height;
    pheight = GetParentCloudLayer()->GetThickness();
}
