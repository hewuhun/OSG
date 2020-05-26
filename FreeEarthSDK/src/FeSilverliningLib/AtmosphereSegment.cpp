// Copyright (c) 2007-2009 Sundog Software. All rights reserved worldwide.

#include <FeSilverliningLib/AtmosphereSegment.h>
#include <FeSilverliningLib/VertexBuffer.h>
#include <FeSilverliningLib/IndexBuffer.h>
#include <FeSilverliningLib/Renderer.h>
#include <FeSilverliningLib/Configuration.h>
#include <assert.h>

using namespace SilverLining;

AtmosphereSegment::AtmosphereSegment()  : Renderable(), worldPos(0,0,0), ib(0), vb(0)
{
    doShadow = true;

    Configuration::GetBoolValue("atmosphere-shadow-from-earth", doShadow);
}

void AtmosphereSegment::BuildGeometry(float startRadians, float endRadians, float r, float H0,
                                      float atmosphereHeight, int xResolution, int yResolution, float atmosphereBrightness)
{
    int numVerts = xResolution * yResolution;
    vb = SL_NEW VertexBuffer(numVerts);
    if (vb && vb->LockBuffer()) {
        Vertex *v = vb->GetVertices();
        if (v) {
            float radInc = (endRadians - startRadians) / (xResolution - 1);
            int idx = 0;
            for (int y = 0; y < yResolution; y++) {
                float ri;
                if (y == (yResolution - 1)) {
                    ri = atmosphereHeight + r;
                } else {
                    float pi = 1.0f - (float)y / ((float)yResolution - 1.0f);
                    ri = H0 * -logf(pi);
                    ri += r;
                }

                for (int x = 0; x < xResolution; x++) {
                    v[idx].x = cosf(startRadians + x * radInc); // * ri;
                    v[idx].y = sinf(startRadians + x * radInc); // * ri;
                    v[idx].z = 0;
                    v[idx].w = 1.0;
                    v[idx].u = startRadians + x * radInc;
                    v[idx].v = ri - r;
                    float p = expf(-((ri - r) / H0));
                    v[idx].t = p * atmosphereBrightness;

                    idx++;
                }
            }
        }

        vb->UnlockBuffer();
    }

    int numIndices = (xResolution * 2 + 2) * (yResolution - 1) - 2;
    ib = SL_NEW IndexBuffer(numIndices);
    if (ib && ib->LockBuffer()) {
        Index *index = ib->GetIndices();
        int v = 0;
        for (int y = 0; y < (yResolution - 1); y++) {
            for (int x = 0; x < xResolution; x++) {
                index[v++] = (Index)((y * xResolution) + x);
                index[v++] = (Index)(((y+1) * xResolution) + x);
            }

            if (y < yResolution - 2) {
                index[v++] = (Index)(((y + 2) * xResolution) - 1);
                index[v++] = (Index)(((y + 1) * xResolution));
            }
        }

        assert(v == numIndices);

        ib->UnlockBuffer();
    }
}

AtmosphereSegment::~AtmosphereSegment()
{
    if (ib) SL_DELETE ib;
    if (vb) SL_DELETE vb;
}

void AtmosphereSegment::ComputeVertices(const Vector3& sunPosNorm, const Matrix4& world, float fadeDist, float alpha, float radius)
{
    if (vb) {
        if (vb->LockBuffer()) {
            int numVerts = vb->GetNumVertices();
            Vertex *v = vb->GetVertices();
            if (v) {
                for (int i = 0; i < numVerts; i++) {
                    float p = v[i].t;
                    float fade;

                    if (doShadow) {
                        Vector4 pos(v[i].x, v[i].y, v[i].z, 1.0);
                        pos = pos * world;
                        Vector3 pos3(pos.x / pos.w, pos.y / pos.w, pos.z / pos.w);
                        pos3.Normalize();
                        double dot = pos3.Dot(sunPosNorm);
                        fade = alpha;
                        if (dot <= 0) {
                            fade = -(float)dot / fadeDist;
                            fade = 1.0f - fade;
                        }
                    } else {
                        fade = alpha;
                    }

                    Color c(Color(fade * .5f, fade * .5f, fade * 1.0f, 1.0f) * p);
                    c.ClampToUnitOrLess();

                    v[i].SetColor(c);

                    float r = v[i].v + radius;
                    v[i].x = r * cosf(v[i].u);
                    v[i].y = r * sinf(v[i].u);
                }
            }

            vb->UnlockBuffer();
        }
    }
}

bool AtmosphereSegment::Draw(int)
{
    if (ib && vb) {
        vb->DrawIndexedStrip(*ib);
    }

    return true;
}

bool AtmosphereSegment::Cull(const Frustum& f)
{
    return false;
}
