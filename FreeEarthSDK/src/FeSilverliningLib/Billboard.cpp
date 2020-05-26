// Copyright (c) 2004-2014  Sundog Software, LLC. All rights reserved worldwide.

#include <FeSilverliningLib/Atmosphere.h>
#include <FeSilverliningLib/Billboard.h>
#include <FeSilverliningLib/Configuration.h>
#include <FeSilverliningLib/Utils.h>
#include <assert.h>
#include <vector>
#include <algorithm>

using namespace SilverLining;
using namespace std;

// Optimal number of vertices per vertex buffer object. May vary depending on your
// target hardware platform.
static const int vbSize = 10000;

SL_MAP( Atmosphere *, SL_LIST(SharedVertexBuffer *) ) Billboard::vbStackMap;
Matrix4 Billboard::overrideBillboard;
bool Billboard::overridingMatrix = false;
bool Billboard::forceConstantColor = false;
Color Billboard::forceColor;
SL_MAP(Atmosphere *, ShaderHandle) Billboard::billboardShaderMap;
SL_VECTOR(BillboardQuad) Billboard::batchQuads;
static Matrix4 billboardMatrix;
Color Billboard::fog;
double Billboard::fogDensity;
unsigned long Billboard::fadeStartTime = 0;
bool Billboard::spinEnabled = true;
double Billboard::verticalGradient = 0.0;
float Billboard::softness = 0.0f;
float Billboard::fogColorBoost = 1.0f;

void Billboard::Destroy()
{
    vbStackMap.clear();
    batchQuads.clear();
    billboardShaderMap.clear();
}

QuadBatch::~QuadBatch()
{
    if (ib) {
        SL_DELETE ib;
    }
}

// th changed ( use Index here instead of unsigned short, then we can quickly change datatype )
QuadBatch::QuadBatch(const SL_VECTOR(Index)& indices, VertexBuffer *pvb,
                     const SL_VECTOR(Color)& colors)
{
    vb = pvb;
    ib = 0;

    if (vb) {
        int nIndices = (int)indices.size();

        if (nIndices > 0) {
            ib = SL_NEW IndexBuffer(nIndices);
            if (ib && ib->LockBuffer()) {
                if (vb->LockBuffer()) {
                    // th changed ( use Index here instead of unsigned short, then we can quickly change datatype )
                    Index *idx = ib->GetIndices();

                    if (idx) {
                        Vertex *v = vb->GetVertices();
                        if (v) {
                            for (int i = 0; i < nIndices; i++) {
                                idx[i] = indices[i];
                                v[indices[i]].SetColor(colors[i]);
                            }
                        }
                    }

                    vb->UnlockBuffer();
                }

                ib->UnlockBuffer();
            }
        }
    }
}

void QuadBatch::Draw()
{
    Renderer *ren = Renderer::GetInstance();

    if (vb && ib) {
        ren->DrawStrip(vb->GetHandle(), ib->GetHandle(), 0, ib->GetNumIndices(), vb->GetNumVertices(),
                       true);
    }
}

QuadBatches::~QuadBatches()
{
    SL_VECTOR(QuadBatch *) ::iterator it;
    for (it = batches.begin(); it != batches.end(); it++) {
        SL_DELETE (*it);
    }

    batches.clear();
}

void QuadBatches::Draw(TextureHandle tex, double fade)
{
    ShaderHandle billboardShader = Billboard::GetShaderHandle();

    if (billboardShader) {
        Renderer *ren = Renderer::GetInstance();

        ren->EnableBackfaceCulling(false);

        ren->PushModelviewTransform();

        Billboard::SetBillboardMatrix();

        ren->EnableTexture(tex, 0);

        ren->BindShader(billboardShader, VERTEX_PROGRAM);
        Matrix4 modelview, proj, modelviewProj;
        ren->GetModelviewMatrix(&modelview);

        Billboard::UnsetShaderColor();

        ren->GetProjectionMatrix(&proj);
        modelviewProj = proj * modelview;

        float outputScale = Atmosphere::GetCurrentAtmosphere()->GetOutputScale();
        ren->SetConstantVector(billboardShader, "sl_outputScale", Vector3((double)outputScale, 0.0, 0.0));

        ren->SetConstantVector(billboardShader, "sl_noSpin", Billboard::spinEnabled ? Vector3(0, Billboard::verticalGradient, Billboard::softness) : Vector3(1.0, Billboard::verticalGradient, Billboard::softness));

        ren->SetConstantMatrix(billboardShader, "sl_modelView", modelview);

        ren->SetConstantMatrix(billboardShader, "sl_modelViewProj", modelviewProj);

        ren->SetConstantMatrix(billboardShader, "sl_billboard", billboardMatrix);

        ren->SetConstantMatrix(billboardShader, "sl_basis", ren->GetBasis4x4());

        ren->SetConstantVector(billboardShader, "sl_flipTCoordsAndFogDensity", ren->GetType() == Atmosphere::OPENGL ?
                               Vector3(1,Billboard::fogDensity,1) : Vector3(-1,Billboard::fogDensity,-1));

        Vector3 fogColor(1.0, 0, 0);
        fogColor.x = Billboard::fog.r;
        fogColor.y = Billboard::fog.g;
        fogColor.z = Billboard::fog.b;

        ren->SetConstantVector(billboardShader, "sl_fogColor", fogColor);

        unsigned long millis = Atmosphere::GetCurrentAtmosphere()->GetConditions()->GetMillisecondTimer()->GetMilliseconds();
        float t = (float)millis * 0.001f;
        float fadeT0 = (float)Billboard::GetFadeStartTime() * 0.001f;
        Vector4 fadeV(fade, Billboard::fogColorBoost, fadeT0, t);

        ren->SetConstantVector4(billboardShader, "sl_fade", fadeV);

        bool savedFog = ren->GetFogEnabled();
        ren->EnableFog(false);

        SL_VECTOR(QuadBatch *) ::iterator it;
        for (it = batches.begin(); it != batches.end(); it++) {
            (*it)->Draw();
        }

        ren->EnableFog(savedFog);

        ren->UnbindShader(VERTEX_PROGRAM);

        ren->PopModelviewTransform();
        //ren->EnableBackfaceCulling(true);
    }
}

// th changed ( use Index here instead of unsigned short, then we can quickly change datatype )
QuadBatch *Billboard::DrawQuadBatch(const SL_VECTOR(Index)& indices, VertexBuffer *vb,
                                    const SL_VECTOR(Color)& colors)
{
    QuadBatch *qb = SL_NEW QuadBatch(indices, vb, colors);
    qb->Draw();

    return qb;
}

QuadBatches *Billboard::DrawQuadBatches()
{
    Renderer *ren = Renderer::GetInstance();
    ren->SetCurrentColor(Color(1,1,1,1));

    // Sort the quads by vertex buffer.
    //sort(batchQuads.begin(), batchQuads.end());

    SL_VECTOR(BillboardQuad) ::iterator quadIt;
    // th changed (avoid new/delete)
    static SL_VECTOR(Index) indices;
    static SL_VECTOR(Color) colors;
    indices.clear();
    colors.clear();

    VertexBuffer *lastVb = 0;

    QuadBatches *qbs = SL_NEW QuadBatches();

    // Stitch quads together with degenerate tris.
    for (quadIt = batchQuads.begin(); quadIt != batchQuads.end(); quadIt++) {
        if ((*quadIt).vb != lastVb) {
            if (lastVb != 0) {
                // Vertex buffer changed, draw everything so far and start over.
                QuadBatch *qb = DrawQuadBatch(indices, lastVb, colors);
                qbs->Add(qb);
            }

            lastVb = (*quadIt).vb;
            indices.clear();
            colors.clear();
        }

        indices.push_back((Index)((*quadIt).index + 3));
        indices.push_back((Index)((*quadIt).index + 3));
        indices.push_back((Index)((*quadIt).index + 2));
        indices.push_back((Index)((*quadIt).index + 0));
        indices.push_back((Index)((*quadIt).index + 1));
        indices.push_back((Index)((*quadIt).index + 1));

        for (int i = 0; i < 6; i++) {
            colors.push_back((*quadIt).color);
        }
    }

    // Draw last batch.
    QuadBatch *qb = DrawQuadBatch(indices, lastVb, colors);
    qbs->Add(qb);

    indices.clear();
    colors.clear();

    return qbs;
}

SharedVertexBuffer::SharedVertexBuffer() : vb(0), quadIndices(0), nextAvail(0), numReferences(0)
{
}

SharedVertexBuffer::SharedVertexBuffer(const SharedVertexBuffer& svb)
{
    vb = svb.vb;
    numReferences = svb.numReferences;
    quadIndices = svb.quadIndices;
    nextAvail = svb.nextAvail;
}

SharedVertexBuffer::~SharedVertexBuffer()
{
    Clear();
}

void SharedVertexBuffer::Clear()
{
    if (vb) {
        SL_DELETE vb;
    }

    vb = 0;

    if (quadIndices) {
        SL_DELETE quadIndices;
    }

    quadIndices = 0;
}

void SharedVertexBuffer::AddReference()
{
    numReferences++;
}

void SharedVertexBuffer::RemoveReference()
{
    numReferences--;

    if (numReferences <= 0) {
        Clear();
        Billboard::vbStackMap[Atmosphere::GetCurrentAtmosphere()].remove(this);
        SL_DELETE this;
    }
}

bool SharedVertexBuffer::Reserve(int nVerts, unsigned int& vertIdx)
{
    if (!vb) {
        vb = SL_NEW VertexBuffer(vbSize);
    }

    if (!quadIndices && !Renderer::GetInstance()->HasQuads()) {
        assert(vbSize % 4 == 0);
        quadIndices = SL_NEW IndexBuffer(vbSize);
        if (quadIndices && quadIndices->LockBuffer()) {
            // th changed ( use Index here instead of unsigned short, then we can quickly change datatype )
            Index *indices = quadIndices->GetIndices();
            if (indices) {
                for (int i = 0; i < vbSize; i += 4) {
                    indices[0] = (Index)(i+3);
                    indices[1] = (Index)(i+0);
                    indices[2] = (Index)(i+2);
                    indices[3] = (Index)(i+1);

                    indices += 4;
                }
            } else {
#ifdef _DEBUG
                printf("Couldn't get index buffer pointer.\n");
#endif
                return false;
            }
        } else {
#ifdef _DEBUG
            printf("Couldn't lock index buffer.\n");
#endif
            return false;
        }

        quadIndices->UnlockBuffer();
    }

    if ((int)nextAvail + nVerts < vbSize) {
        vertIdx = nextAvail;
        nextAvail += nVerts;
        return true;
    } else {
        return false;
    }
}

void Billboard::OverrideBillboardMatrix(const Matrix4& mat)
{
    overrideBillboard = mat;
    overridingMatrix = true;
}

void Billboard::RestoreBillboardMatrix()
{
    overridingMatrix = false;
}

void Billboard::ReloadShaders()
{
    Renderer *ren = Renderer::GetInstance();
    if (ren) {
        ren->DeleteShader(billboardShaderMap[Atmosphere::GetCurrentAtmosphere()]);
        SetupShaders();
    }
}

void Billboard::SetupShaders()
{
    billboardShaderMap[Atmosphere::GetCurrentAtmosphere()] =
        Renderer::GetInstance()->LoadShaderFromFile("Shaders/Billboard.cg", VERTEX_PROGRAM);

    Configuration::GetFloatValue("billboard-fog-color-boost", fogColorBoost);
}

ShaderHandle Billboard::GetShader()
{
    return billboardShaderMap[Atmosphere::GetCurrentAtmosphere()];
}

Billboard::Billboard() : sharedVb(0), centerPos(0, 0, 0), billboardColor(1, 1, 1, 1), radius(0), fadeRate(0), atlasIndex(0), rotateSpeed(0)
{
}

void Billboard::Initialize(bool rotate, const float diameter, const float spinRate)
{
    sharedVb = 0;
    const float epsilon = 1E-9f;

    if (rotate) {
        rotateAngle = Atmosphere::GetCurrentAtmosphere()->GetRandomNumberGenerator()->UniformRandomFloat() * 2.0f * (float)PI;
        rotateSpeed = Atmosphere::GetCurrentAtmosphere()->GetRandomNumberGenerator()->UniformRandomFloat() * spinRate * 2.0f - spinRate;
        if (rotateAngle == 0.0f) rotateAngle = epsilon;
        if (rotateSpeed == 0.0f) rotateSpeed = epsilon;
    } else {
        rotateAngle = epsilon;
        rotateSpeed = epsilon;
    }

    VertexBuffer *vb;
    IndexBuffer *ib;
    GetVerts(4, vbIdx, &vb, &ib, &sharedVb);

    if (vb) {
        Vertex vert[4];

        vert[0].x = -0.5;
        vert[0].y = 0.5;
        vert[0].z = 0;
        vert[0].w = 0;
        vert[0].SetUV(0, 1);
        vert[0].SetColor(Color(1,1,1));

        vert[1].x = -0.5;
        vert[1].y = -0.5;
        vert[1].z = 0;
        vert[1].w = 0;
        vert[1].SetUV(0, 0);
        vert[1].SetColor(Color(1,1,1));

        vert[2].x = 0.5;
        vert[2].y = -0.5;
        vert[2].z = 0;
        vert[2].w = 0;
        vert[2].SetUV(1, 0);
        vert[2].SetColor(Color(1,1,1));

        vert[3].x = 0.5;
        vert[3].y = 0.5;
        vert[3].z = 0;
        vert[3].w = 0;
        vert[3].SetUV(1, 1);
        vert[3].SetColor(Color(1,1,1));

        for (int i = 0; i < 4; i++) {
            float x = cosf(rotateAngle) * vert[i].x - sinf(rotateAngle) * vert[i].y;
            float y = sinf(rotateAngle) * vert[i].x + cosf(rotateAngle) * vert[i].y;
            vert[i].x = x;
            vert[i].y = y;
        }

        if (diameter > 0) {
            radius = (float)diameter * 0.5f;

            if (billboardShaderMap[Atmosphere::GetCurrentAtmosphere()]) {
                // Use instancing technique; stuff world pos in x,y,z
                // point size in abs(u), direction in v,t sign, rotation in abs(t)
                // rotation speed in abs(v), rotation direction in u sign
                for (int i = 0; i < 4; i++) {
                    vert[i].x = (float)centerPos.x;
                    vert[i].y = (float)centerPos.y;
                    vert[i].z = (float)centerPos.z;
                    vert[i].w = (float)atlasIndex;
                    vert[i].u = radius * rotateSpeed > 1.0f ? 1.0f : -1.0f;
                    vert[i].s = 0;

                    switch (i) {
                    case 0:
                        vert[i].v = -fabs(rotateSpeed);
                        vert[i].t = rotateAngle;
                        break;

                    case 1:
                        vert[i].v = -fabs(rotateSpeed);
                        vert[i].t = -rotateAngle;
                        break;

                    case 2:
                        vert[i].v = fabs(rotateSpeed);
                        vert[i].t = -rotateAngle;
                        break;

                    case 3:
                        vert[i].v = fabs(rotateSpeed);
                        vert[i].t = rotateAngle;
                        break;
                    }
                }
            } else {
                vert[0].x = -radius;
                vert[0].y = radius;

                vert[1].x = -radius;
                vert[1].y = -radius;

                vert[2].x = radius;
                vert[2].y = -radius;

                vert[3].x = radius;
                vert[3].y = radius;

                for (int i = 0; i < 4; i++) {
                    float x = cosf(rotateAngle) * vert[i].x - sinf(rotateAngle) * vert[i].y;
                    float y = sinf(rotateAngle) * vert[i].x + cosf(rotateAngle) * vert[i].y;
                    vert[i].x = x;
                    vert[i].y = y;
                }
            }
        }

        vb->Update(vbIdx, vert, 4);
    }
}

Billboard::~Billboard()
{
    if (sharedVb) {
        sharedVb->RemoveReference();
    }
}

void Billboard::GetVerts(int nVerts, unsigned int& vbIdx, VertexBuffer **pvb, IndexBuffer **pib, SharedVertexBuffer **sib)
{
    SL_LIST(SharedVertexBuffer *)& vbStack = vbStackMap[Atmosphere::GetCurrentAtmosphere()];
    if (vbStack.empty()) {
        SharedVertexBuffer *svb = SL_NEW SharedVertexBuffer();
        vbStack.push_back(svb);
    }

    SharedVertexBuffer *svb = vbStack.back();
    if (svb->Reserve(nVerts, vbIdx)) {
        *pvb = svb->GetVertexBuffer();
        *pib = svb->GetIndexBuffer();
    } else {
        SharedVertexBuffer *newSvb = SL_NEW SharedVertexBuffer();
        vbStack.push_back(newSvb);
        svb = vbStack.back();
        if (!svb->Reserve(nVerts, vbIdx)) {
            *pvb = 0;
            *pib = 0;
        } else {
            *pvb = svb->GetVertexBuffer();
            *pib = svb->GetIndexBuffer();
        }
    }

    *sib = vbStack.back();
    (*sib)->AddReference();
}

void Billboard::UpdateQuadVertices(bool changeSize, double pointSize, bool rotate, float fadeRate)
{
    if (!sharedVb) return;

    VertexBuffer *vb = sharedVb->GetVertexBuffer();

    if (!vb) return;

    Vertex vert[4];

    float half = (float)pointSize * 0.5f;

    if (billboardShaderMap[Atmosphere::GetCurrentAtmosphere()]) {
        // Use instancing technique; stuff world pos in x,y,z
        // point size in abs(u), relative pos in signs of v,t, rotation in abs(t),
        // rotation speed in abs(v), rotation direction in sign(u)
        for (int i = 0; i < 4; i++) {
            vert[i].x = (float)centerPos.x;
            vert[i].y = (float)centerPos.y;
            vert[i].z = (float)centerPos.z;
            vert[i].w = (float)atlasIndex;
            vert[i].s = fadeRate;

            if (changeSize) {
                radius = half;
                vert[i].u = half;
            } else {
                vert[i].u = radius;
            }

            if (rotateSpeed < 0.0f) vert[i].u *= -1.0f;

            switch (i) {
            case 0:
                vert[i].v = -fabs(rotateSpeed);
                vert[i].t = rotateAngle;
                break;

            case 1:
                vert[i].v = -fabs(rotateSpeed);
                vert[i].t = -rotateAngle;
                break;

            case 2:
                vert[i].v = fabs(rotateSpeed);
                vert[i].t = -rotateAngle;
                break;

            case 3:
                vert[i].v = fabs(rotateSpeed);
                vert[i].t = rotateAngle;
                break;
            }

            vert[i].SetColor(Color(1,1,1));
        }

    } else {
        vb->Get(vbIdx, vert, 4);

        if (changeSize) {
            vert[0].x = -half;
            vert[0].y = half;

            vert[1].x = -half;
            vert[1].y = -half;

            vert[2].x = half;
            vert[2].y = -half;

            vert[3].x = half;
            vert[3].y = half;

            for (int i = 0; i < 4; i++) {
                if (rotate) {
                    float x = cosf(rotateAngle) * vert[i].x - sinf(rotateAngle) * vert[i].y;
                    float y = sinf(rotateAngle) * vert[i].x + cosf(rotateAngle) * vert[i].y;
                    vert[i].x = x;
                    vert[i].y = y;
                }
            }
        }
    }

    vb->Update(vbIdx, vert, 4);
}

void Billboard::SetScreenSize(double diameter)
{
    Renderer *renderer = Renderer::GetInstance();

    Vector3f dist = Atmosphere::GetCurrentAtmosphere()->GetCamPos();
    dist = centerPos - dist;
    double z = dist.Length();

    double fovV;
    renderer->GetFOV(fovV);
    int x, y, w, h;
    renderer->GetViewport(x, y, w, h);

    double fovH = fovV * ((double)w / (double)h);

    double e = 1.0 / tan(fovH / 2.0);

    double pointSize = (diameter * z) / (e * w);

    UpdateQuadVertices(true, pointSize);
}

void Billboard::SetWorldSize(double diameter)
{
    UpdateQuadVertices(true, diameter);
}

void Billboard::SetAngularSize(double degrees)
{
    Matrix4 mv;

    Vector3f dist = centerPos;

    double r = dist.Length();
    double pointSize = r * tan(RADIANS(degrees * 0.5)) * 2.0;

    UpdateQuadVertices(true, pointSize, false);
}

void Billboard::SetWorldPosition(const Vector3f& pos)
{
    if (sharedVb) {
        centerPos = pos;

        if (billboardShaderMap[Atmosphere::GetCurrentAtmosphere()]) {
            UpdateQuadVertices(false, 0);
        }
    }
}

void Billboard::SetColor(const Color& col)
{
    billboardColor = col;
}

Color Billboard::GetColor() const
{
    return billboardColor;
}

void Billboard::SetFadeRate(float pFadeRate)
{
    if (fadeRate != pFadeRate) {
        if (sharedVb) {
            if (billboardShaderMap[Atmosphere::GetCurrentAtmosphere()]) {
                UpdateQuadVertices(false, 0, true, pFadeRate);
                fadeRate = pFadeRate;
            }
        }
    }
}

bool Billboard::Cull(const Frustum& f)
{
    return false;
}

void Billboard::SetBillboardMatrix()
{
    Renderer *ren = Renderer::GetInstance();
    billboardMatrix = overridingMatrix ? overrideBillboard : ren->GetBillboardMatrix();
}

void Billboard::UnsetShaderColor()
{
    Renderer *ren = Renderer::GetInstance();

    Vector4 vbillboardColor;
    vbillboardColor.x = vbillboardColor.y = vbillboardColor.z = 1.0;
    vbillboardColor.w = 0;
    if (forceConstantColor)
        vbillboardColor = forceColor.ToVector4();
    ren->SetConstantVector4(billboardShaderMap[Atmosphere::GetCurrentAtmosphere()], "sl_billboardColor", vbillboardColor);
}

void Billboard::SetShaderColor(const Color& c)
{
    Renderer *ren = Renderer::GetInstance();

    Vector4 vbillboardColor;
    vbillboardColor.x = c.r;
    vbillboardColor.y = c.g;
    vbillboardColor.z = c.b;
    vbillboardColor.w = c.a;

    ren->SetConstantVector4(billboardShaderMap[Atmosphere::GetCurrentAtmosphere()], "sl_billboardColor", vbillboardColor);
}

static bool savedFog = false;

void Billboard::SetupBillboardShader(double fade, bool objectSpace)
{
    SetBillboardMatrix();

    ShaderHandle billboardShader = billboardShaderMap[Atmosphere::GetCurrentAtmosphere()];
    if (billboardShader) {
        Renderer *ren = Renderer::GetInstance();
        ren->BindShader(billboardShader, VERTEX_PROGRAM);
        Matrix4 modelview, proj, modelviewProj;
        ren->GetModelviewMatrix(&modelview);

        ren->GetProjectionMatrix(&proj);
        modelviewProj = proj * modelview;

        UnsetShaderColor();

        float outputScale = Atmosphere::GetCurrentAtmosphere()->GetOutputScale();
        ren->SetConstantVector(billboardShader, "sl_outputScale", Vector3((double)outputScale, 0.0, 0.0));

        ren->SetConstantVector(billboardShader, "sl_noSpin", spinEnabled ? Vector3(0, verticalGradient, softness) : Vector3(1.0, verticalGradient, softness));

        ren->SetConstantMatrix(billboardShader, "sl_modelView", modelview);

        ren->SetConstantMatrix(billboardShader, "sl_modelViewProj", modelviewProj);

        ren->SetConstantMatrix(billboardShader, "sl_billboard", billboardMatrix);

        if (objectSpace) {
            ren->SetConstantMatrix(billboardShader, "sl_basis", ren->GetBasis4x4());
        } else {
            Matrix4 identity;
            ren->SetConstantMatrix(billboardShader, "sl_basis", identity);
        }

        ren->SetConstantVector(billboardShader, "sl_flipTCoordsAndFogDensity", ren->GetType() == Atmosphere::OPENGL ?
                               Vector3(1,fogDensity,1) : Vector3(-1,fogDensity,-1));

        Vector3 fogColor;
        fogColor.x = fog.r;
        fogColor.y = fog.g;
        fogColor.z = fog.b;

        ren->SetConstantVector(billboardShader, "sl_fogColor", fogColor);

        unsigned long millis = Atmosphere::GetCurrentAtmosphere()->GetConditions()->GetMillisecondTimer()->GetMilliseconds();
        float t = (float)((float)millis * 0.001f);
        float fadeT0 = (float)fadeStartTime * 0.001f;
        ren->SetConstantVector4(billboardShader, "sl_fade", Vector4(fade, fogColorBoost, fadeT0, t));

        savedFog = ren->GetFogEnabled();
        ren->EnableFog(false);
    }
}

void Billboard::TeardownBillboardShader()
{
    if (billboardShaderMap[Atmosphere::GetCurrentAtmosphere()]) {
        Renderer *ren = Renderer::GetInstance();
        ren->UnbindShader(VERTEX_PROGRAM);
        ren->EnableFog(savedFog);
    }
}

bool Billboard::StartBatchDraw(TextureHandle tex, double fade, bool objectSpace)
{
    Renderer *ren = Renderer::GetInstance();
    if (ren) {
        ren->EnableBackfaceCulling(false);

        if (ren->PushModelviewTransform()) {
            batchQuads.clear();

            ren->EnableTexture(tex, 0);

            SetupBillboardShader(fade, objectSpace);

            return true;
        }

    }

    return false;
}

QuadBatches *Billboard::EndBatchDraw()
{
    Renderer *ren = Renderer::GetInstance();

    ren->EnableBackfaceCulling(false);

    QuadBatches *qbs = 0;

    if (billboardShaderMap[Atmosphere::GetCurrentAtmosphere()]) {
        qbs = DrawQuadBatches();
        TeardownBillboardShader();
    }

    //ren->EnableBackfaceCulling(true);

    if (ren->PopModelviewTransform()) {
        if (ren->DisableTexture(0)) {
            if (ren->DisableBlending()) {
                return qbs;
            }
        }
    }

    return qbs;
}

bool Billboard::DrawInBatch(bool drawImmediately)
{
    Renderer *ren = Renderer::GetInstance();
    if (!sharedVb) return false;

    VertexBuffer *vb = sharedVb->GetVertexBuffer();
    IndexBuffer *ib = sharedVb->GetIndexBuffer();

    if (billboardShaderMap[Atmosphere::GetCurrentAtmosphere()]) {
        if (drawImmediately) {
            SetShaderColor(billboardColor);

            if (ib) {
                ren->DrawStrip(vb->GetHandle(), ib->GetHandle(), vbIdx, 4, vb->GetNumVertices(), true);
            } else {
                ren->DrawQuads(vb->GetHandle(), 4, vbIdx, false);
            }

            UnsetShaderColor();
        } else {
            // Defer actual drawing until batch is done, so we can do it all at once
            BillboardQuad bq(vbIdx, vb, billboardColor);
            batchQuads.push_back(bq);
        }
    } else {
        Matrix4 xlateMatrix;
        xlateMatrix.elem[0][3] = centerPos.x;
        xlateMatrix.elem[1][3] = centerPos.y;
        xlateMatrix.elem[2][3] = centerPos.z;

        ren->SetCurrentColor(billboardColor);
        ren->PushModelviewTransform();

        ren->MultiplyModelviewMatrix(xlateMatrix * billboardMatrix);

        if (ib) {
            ren->DrawStrip(vb->GetHandle(), ib->GetHandle(), vbIdx, 4, vb->GetNumVertices(), false);
        } else {
            ren->DrawQuads(vb->GetHandle(), 4, vbIdx, false);
        }

        ren->PopModelviewTransform();
    }

    return true;
}

bool Billboard::Draw(TextureHandle tex, int pass, double fade, bool objectSpace)
{
    if (!sharedVb) return false;

    VertexBuffer *vb = sharedVb->GetVertexBuffer();
    IndexBuffer *ib = sharedVb->GetIndexBuffer();

    Renderer *renderer = Renderer::GetInstance();

    // for debugging:
    // renderer->DisableBlending();

    renderer->EnableBackfaceCulling(false);
    renderer->PushModelviewTransform();

    renderer->EnableTexture(tex, 0);

    bool useShader = billboardShaderMap[Atmosphere::GetCurrentAtmosphere()] != 0;

    if (useShader) {
        SetupBillboardShader(fade, objectSpace);
        SetShaderColor(billboardColor);
    } else {
        renderer->SetCurrentColor(billboardColor);

        Matrix4 xlateMatrix;
        xlateMatrix.elem[0][3] = centerPos.x;
        xlateMatrix.elem[1][3] = centerPos.y;
        xlateMatrix.elem[2][3] = centerPos.z;

        Matrix4 m = overridingMatrix ? overrideBillboard : renderer->GetBillboardMatrix();
        renderer->MultiplyModelviewMatrix(xlateMatrix * m);
    }

    if (ib) {
        renderer->DrawStrip(vb->GetHandle(), ib->GetHandle(), vbIdx, 4, vb->GetNumVertices(),
                            useShader);
    } else {
        renderer->DrawQuads(vb->GetHandle(), 4, vbIdx, useShader);
    }

    if (useShader) {
        TeardownBillboardShader();
        UnsetShaderColor();
    }

    renderer->PopModelviewTransform();

    renderer->DisableTexture(0);
    //renderer->EnableBackfaceCulling(true);

    return true;
}

void Billboard::RemoveAtmosphere(Atmosphere *atm)
{
    SL_MAP( Atmosphere *, SL_LIST(SharedVertexBuffer *) ) ::iterator iter = vbStackMap.find(atm);
    if (iter != vbStackMap.end()) {
        vbStackMap.erase(iter);
    }

    SL_MAP(Atmosphere *, ShaderHandle) ::iterator iter2 = billboardShaderMap.find(atm);
    if (iter2 != billboardShaderMap.end()) {
        billboardShaderMap.erase(iter2);
    }
}
