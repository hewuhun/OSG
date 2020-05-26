// Copyright 2006-2015 Sundog Software, LLC. All rights reserved worldwide.

#include <FeSilverliningLib/StratusCloud.h>
#include <FeSilverliningLib/TGALoader.h>
#include <FeSilverliningLib/Configuration.h>
#include <FeSilverliningLib/Billboard.h>
#include <FeSilverliningLib/Metaball.h>
#include <FeSilverliningLib/CloudLayer.h>
#include <stdlib.h>
#include <assert.h>
#include <FeSilverliningLib/mie.h>

using namespace SilverLining;
using namespace std;


StratusCloud::StratusCloud(CloudLayer *layer) : Cloud(layer), fadeFalloff(5.0)
{
    stratusShader = Renderer::GetInstance()->LoadShaderFromFile("Shaders/Stratiform.cg", VERTEX_PROGRAM);

    doBrokenVisibility = false;
    Configuration::GetBoolValue("stratus-handle-broken-visibility", doBrokenVisibility);

    lightScale = 1.0f;
    Configuration::GetFloatValue("stratus-deck-light-scale", lightScale);

    bulginess = 0.6;
    Configuration::GetDoubleValue("stratus-cloud-bulgy-exponent", bulginess);

    skyColorScale = Vector3(1.0, 1.0, 1.0);
    Configuration::GetDoubleValue("stratus-sky-color-scale-r", skyColorScale.x);
    Configuration::GetDoubleValue("stratus-sky-color-scale-g", skyColorScale.y);
    Configuration::GetDoubleValue("stratus-sky-color-scale-b", skyColorScale.z);

    groundColorScale = Vector3(1.0, 1.0, 1.0);
    Configuration::GetDoubleValue("stratus-ground-color-scale-r", groundColorScale.x);
    Configuration::GetDoubleValue("stratus-ground-color-scale-g", groundColorScale.y);
    Configuration::GetDoubleValue("stratus-ground-color-scale-b", groundColorScale.z);

    for (int i = 0; i < NUM_SECTIONS; i++) {
        ibTop[i] = 0;
        ibBottom[i] = 0;
        textures[i] = 0;
        scudMap[i] = 0;
        vb[i] = 0;
    }

    mieTexture = 0;
}

void StratusCloud::ReloadShaders()
{
    Renderer *ren = Renderer::GetInstance();
    if (ren && stratusShader) {
        ren->DeleteShader(stratusShader);
        stratusShader = ren->LoadShaderFromFile("Shaders/Stratiform.cg", VERTEX_PROGRAM);
    }
}

static void clamp(double& t)
{
    if (t > 1.0) t = 1.0;
    if (t < 0) t = 0;
}

#define fastPow pow
/*
double fastPow(double a, double b) {
    //return pow(a,b);
    union {
        double d;
        int x[2];
    } u = { a };

    u.x[1] = (int)(b * (u.x[1] - 1072632447) + 1072632447);
    u.x[0] = 0;
    return u.d;
}
*/

void StratusCloud::CreateTexture(double density, double edgeTexSizeX, double edgeTexSizeY)
{
    TGALoader tga;

    double scudThickness;
    Configuration::GetDoubleValue("stratus-scud-thickness", scudThickness);
    scudThickness *= Atmosphere::GetUnitScale();

    double maxOpticalThickness;
    Configuration::GetDoubleValue("stratus-max-optical-thickness", maxOpticalThickness);
    maxOpticalThickness *= Atmosphere::GetUnitScale();

    double cloudThickness = GetParentCloudLayer()->GetThickness();

    if (tga.Load("stratus.tga")) {
        int texSize = tga.GetWidth() * tga.GetHeight() * 2;

        double tx = (double)(tga.GetWidth() - 1);
        double ty = (double)(tga.GetHeight() - 1);

        scudMapW = tga.GetWidth();
        scudMapH = tga.GetHeight();

        double stratusEdgeFalloff = 4.0;
        Configuration::GetDoubleValue("stratus-edge-falloff", stratusEdgeFalloff);

        double curvedDensity = fastPow(density, 0.6);

        int numTextures = GetParentCloudLayer()->GetIsInfinite() ? 1 : NUM_SECTIONS;

        for (int texNum = 0; texNum < numTextures; texNum++) {
            scudMap[texNum] = SL_NEW unsigned char[scudMapW * scudMapH];

            unsigned char *noise = tga.GetPixels();
            unsigned char *pixels = SL_NEW unsigned char[texSize];
            unsigned char *p = pixels;

            bool repeatU = true, repeatV = true;

            for (int y = 0; y < tga.GetHeight(); y++) {
                for (int x = 0; x < tga.GetWidth(); x++) {
                    unsigned char src = *noise++;

                    //unsigned char alpha;

                    double blend = 1.0;

                    if (!GetParentCloudLayer()->GetIsInfinite()) {
                        double dx = (double)x;
                        double dy = (double)y;

                        double xx = 1.0, yy = 1.0;

                        if (texNum == LEFT || texNum == TOPLEFT || texNum == BOTTOMLEFT) {
                            double t = 1.0 - dx / tx / edgeTexSizeX;
                            clamp(t);
                            xx = 1.0 - fastPow(t, stratusEdgeFalloff);
                            repeatU = false;
                        } else if (texNum == RIGHT || texNum == TOPRIGHT || texNum == BOTTOMRIGHT) {
                            double t = dx / tx / edgeTexSizeX - (1.0 - edgeTexSizeX) * (1.0 / edgeTexSizeX);
                            clamp(t);
                            xx = 1.0 - fastPow(t, stratusEdgeFalloff);
                            repeatU = false;
                        }

                        if (texNum == TOP || texNum == TOPLEFT || texNum == TOPRIGHT) {
                            double t = 1.0 - dy / ty / edgeTexSizeY;
                            clamp(t);
                            yy = 1.0 - fastPow(t, stratusEdgeFalloff);
                            repeatV = false;
                        } else if (texNum == BOTTOM || texNum == BOTTOMLEFT || texNum == BOTTOMRIGHT) {
                            double t = dy / ty / edgeTexSizeY - (1.0 - edgeTexSizeY) * (1.0 / edgeTexSizeY);
                            clamp(t);
                            yy = 1.0 - fastPow(t, stratusEdgeFalloff);
                            repeatV = false;
                        }

                        blend = xx * yy;
                    }

                    double adjustedDensity = curvedDensity * blend;

                    double nSrc = (double)src / 255.0;
                    double nClamped = nSrc;

                    if (adjustedDensity >= 1.0) {
                        if (scudThickness < cloudThickness) {
                            double scud = scudThickness * nSrc;
                            double baseThickness = cloudThickness - scudThickness;
                            double thickness = baseThickness + scud;
                            nSrc = thickness / cloudThickness;
                            if (nSrc < 0) nSrc = 0;
                            if (nSrc > 1.0) nSrc = 1.0;

                            double clampedThickness = cloudThickness > maxOpticalThickness ? maxOpticalThickness : cloudThickness;
                            baseThickness = clampedThickness - scudThickness;
                            thickness = baseThickness + scud;
                            nClamped = thickness / clampedThickness;
                            if (nClamped < 0) nClamped = 0;
                            if (nClamped > 1.0) nClamped = 1.0;

                        }
                    } else {
                        if (blend < 1.0 && density >= 1.0) {
                            double scud = scudThickness * nSrc;
                            double baseThickness = cloudThickness - scudThickness;
                            double thickness = baseThickness + scud;
                            nSrc = thickness / cloudThickness;
                            nSrc -= (1.0 - adjustedDensity);
                            if (nSrc < 0) nSrc = 0;
                            if (nSrc > 1.0) nSrc = 1.0;

                            double clampedThickness = cloudThickness > maxOpticalThickness ? maxOpticalThickness : cloudThickness;
                            baseThickness = clampedThickness - scudThickness;
                            thickness = baseThickness + scud;
                            nClamped = thickness / clampedThickness;
                            nClamped -= (1.0 - adjustedDensity);
                            if (nClamped < 0) nClamped = 0;
                            if (nClamped > 1.0) nClamped = 1.0;
                        } else {
                            nSrc -= (1.0 - adjustedDensity);
                            if (nSrc < 0) nSrc = 0;
                            nSrc /= adjustedDensity;
                            nClamped = nSrc;
                        }
                    }


                    nSrc = fastPow(nSrc, bulginess);
                    nClamped = fastPow(nClamped, bulginess);

                    unsigned char dst = (unsigned char)(nSrc * 255.0);
                    *p++ = dst;
                    scudMap[texNum][y*scudMapH + x] = dst;
                    dst = (unsigned char)(nClamped * 255.0);
                    *p++ = dst;
                }
            }

            Renderer *ren = Renderer::GetInstance();
            ren->LoadTexture(pixels, tga.GetWidth(), tga.GetHeight(), &textures[texNum], repeatU, repeatV);
            SL_DELETE[] pixels;
        }
    }
}

Vector3 StratusCloud::GetSortPosition() const
{
    double x, y, z;
    GetParentCloudLayer()->GetSortPosition(x, y, z);
    return Vector3(x, y, z);
}

static double DistanceToPlane(const Vector3& from, const Vector3& to, double D, const Matrix4& mv)
{
    const double farFarAway = 1000000;

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

    //return (Pi - from).Length();
    return ((mv * Pi).z);
}

double StratusCloud::GetDistance(const Vector3& from, const Vector3& to, const Matrix4& mv, bool rightHanded) const
{
    // Find ray / plane intersection point.
    Vector3 xformPos = GetWorldPosition() * Renderer::GetInstance()->GetInverseBasis3x3();
    double D = xformPos.y;

    // No distance if we're in the cloud
    Vector3 xformFrom = from * Renderer::GetInstance()->GetInverseBasis3x3();
    if (xformFrom.y > xformPos.y && xformFrom.y < (xformPos.y + GetParentCloudLayer()->GetThickness())) {
        return 0;
    }

    double bottomDist = DistanceToPlane(from, to, D, mv);
    double topDist = DistanceToPlane(from, to, D + GetParentCloudLayer()->GetThickness(), mv);

    if (rightHanded) {
        bottomDist *= -1.0;
        topDist *= -1.0;
    }
    return topDist < bottomDist ? topDist : bottomDist;
}


float StratusCloud::LookupScud(const Vector3& pos)
{
    double scudThickness;
    Configuration::GetDoubleValue("stratus-scud-thickness", scudThickness);
    scudThickness *= Atmosphere::GetUnitScale();

    Matrix3 basis = GetParentCloudLayer()->GetLocalBasis();
    Matrix3 invBasis = basis.Transpose();

    Vector3 xformpos = pos * invBasis;
    Vector3 xformCloudPos = GetWorldPosition() * invBasis;

    double layerBottom = GetParentCloudLayer()->GetBaseAltitudeGeocentric();
    double scudBottom = layerBottom - scudThickness;
    double cloudBottom = scudBottom + scudThickness;
    double cloudTop = cloudBottom + depth;
    double scudTop = cloudTop + scudThickness;

    if (xformpos.y < scudBottom || xformpos.y > scudTop) return 0;

    double layerX, layerZ;
    GetParentCloudLayer()->GetLayerPosition(layerX, layerZ);

    if (!GetParentCloudLayer()->GetIsInfinite()) {
        if (xformpos.x < (layerX - GetParentCloudLayer()->GetBaseWidth() * 0.5)) return 0;
        if (xformpos.x > (layerX + GetParentCloudLayer()->GetBaseWidth() * 0.5)) return 0;
        if (xformpos.z > (layerZ + GetParentCloudLayer()->GetBaseLength() * 0.5)) return 0;
        if (xformpos.z < (layerZ - GetParentCloudLayer()->GetBaseLength() * 0.5)) return 0;
    }

    if (xformpos.y > cloudBottom && xformpos.y < cloudTop && textureDensity >= 1.0) return 1;

    float cloudDistance = 0;
    if (xformpos.y < cloudBottom) {
        cloudDistance = (float)((cloudBottom - xformpos.y) / (cloudBottom - scudBottom));
    }
    if (xformpos.y > cloudTop) {
        cloudDistance = (float)((xformpos.y - cloudTop) / (scudTop - cloudTop));
    }

    if (doBrokenVisibility && segmentWidth > 0 && segmentHeight > 0) {
        double interpX, interpY;

        if (GetParentCloudLayer()->GetIsInfinite()) {
            Vector3 center = xformCloudPos;
            interpX = fmod(xformpos.x + center.x, segmentWidth);
            interpY = fmod(xformpos.z + center.z, segmentHeight);
        } else {
            double posInCloudX = xformpos.x - (xformCloudPos.x - GetParentCloudLayer()->GetBaseWidth() * 0.5);
            double posInCloudY = xformpos.z - (xformCloudPos.z - GetParentCloudLayer()->GetBaseLength() * 0.5);

            interpX = fmod(posInCloudX, segmentWidth);
            interpY = fmod(posInCloudY, segmentHeight);
        }

        interpX /= segmentWidth;
        interpY /= segmentHeight;
        if (interpX < 0) interpX += 1.0;
        if (interpY < 0) interpY += 1.0;
        interpY = 1.0 - interpY;

        int mapX = (int)(interpX * (double)scudMapW);
        int mapY = (int)(interpY * (double)scudMapH);
        int mapIdx = mapY * scudMapH + mapX;

        unsigned char scud = 0;

        if (mapIdx < scudMapW * scudMapH) {
            scud = scudMap[TILABLE][mapIdx];
        }

        return ((float)scud / 255.0f) * (1.0f - cloudDistance);
    }

    return (1.0f - cloudDistance);
}

void StratusCloud::ShadeCloud()
{
    //Now done in shader
    /*
    for (int i = 0; i < NUM_SECTIONS; i++) {
        if (vb[i]) {
            vb[i]->LockBuffer();
            Vertex *verts = vb[i]->GetVertices();

            for (int j = 0; j < vb[i]->GetNumVertices(); j++) {
                float a = 1.0f;

                if (GetParentCloudLayer()->GetFadeTowardEdges() && GetParentCloudLayer()->GetIsInfinite()) {
                    Vector3 v;
                    v.x = verts[j].x;
                    v.y = verts[j].y;
                    v.z = verts[j].z;
                    float minDim = hw < hh ? hw : hh;
                    float len = ((float)v.Length() / minDim);
                    if (len > 1.0f) len = 1.0f;

                    a = exp(-fadeFalloff * len * len * len);
                }

                verts[j].SetColor(Color(1.0f, 1.0f, 1.0f, a));
            }

            vb[i]->UnlockBuffer();
        }
    }
    */
}

IndexBuffer *StratusCloud::CreateIndices(int w, int h, bool top)
{
    if (!(w > 0 && h > 0)) return 0;

    int offset = 0;
    if (top) offset = w * h;

    int nIndices = (h - 1) * (w * 2 + 2);

    IndexBuffer *ib = SL_NEW IndexBuffer(nIndices);
    if (ib && ib->LockBuffer()) {
        Index *indices = ib->GetIndices();

        int idx = 0;
        for (int strip = 0; strip < h - 1; strip++) {
            for (int col = 0; col < w; col++) {
                Index a = (Index)(strip * w + col);
                Index b = (Index)((strip + 1) * w + col);

                if (top) {
                    indices[idx++] = b + (Index)offset;
                    indices[idx++] = a + (Index)offset;
                } else {
                    indices[idx++] = a + (Index)offset;
                    indices[idx++] = b + (Index)offset;
                }
            }

            // Degenerate triangles to wrap back to next strip
            indices[idx] = indices[idx-1];
            ++idx;

            if (strip == h - 2) {
                // There is no next strip...
                indices[idx] = indices[idx - 1];
            } else {
                if (top) {
                    indices[idx] = (Index)((strip + 2) * w + offset);
                } else {
                    indices[idx] = (Index)((strip + 1) * w + offset);
                }
            }
            //assert(indices[idx] < w * h * 2);

            idx++;
        }

        ib->UnlockBuffer();

        return ib;
    }

    return NULL;
}

VertexBuffer *StratusCloud::CreateVertices(int x1, int x2, int y1, int y2,
        float u1, float u2, float v1, float v2)
{
    bool curve = true;
    Configuration::GetBoolValue("stratus-round-earth", curve);

    if (!parentCloudLayer->GetIsInfinite()) {
        curve = false;
    }

    double earthRadius = 6371000;
    Configuration::GetDoubleValue("earth-radius-meters-polar", earthRadius);
    earthRadius *= Atmosphere::GetUnitScale();

    double radius = earthRadius;

    if (parentCloudLayer->GetCurveTowardGround()) {
        double baseWidth = parentCloudLayer->GetBaseWidth();
        double baseLength = parentCloudLayer->GetBaseLength();
        double a = baseWidth < baseLength ? baseWidth : baseLength; //sqrt(baseWidth * baseWidth + baseLength * baseLength) / 2.0;
        double b = parentCloudLayer->GetBaseAltitude();
        radius = (a*a + b*b) / (2.0 * b);
        curve = true;
    }

    int w = x2 - x1;
    int h = y2 - y1;

    if (!(w > 0 && h > 0)) return NULL;

    float incTx = (u2 - u1) / (float)(w - 1);
    float incTy = (v2 - v1) / (float)(h - 1);

    int row, col;

    VertexBuffer *vb = SL_NEW VertexBuffer(w * h * 2);
    if (vb && vb->LockBuffer()) {
        Vertex *verts = vb->GetVertices();
        if (verts) {
            float tu, tv;
            // Bottom verts
            for (row = 0; row < h; row++) {
                for (col = 0; col < w; col++) {
                    int idx = row * w + col;
                    Vector3 v(-hw + (x1 + col) * incX, 0, hh - (y1 + row) * incY);

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

                    tu = u1 + col * incTx;
                    tv = v1 + row * incTy;
                    verts[idx].u = tu;
                    verts[idx].v = tv;

                    verts[idx].SetColor(Color(1.0,1.0,1.0,1.0));
                }
            }

            // Top verts
            for (row = 0; row < h; row++) {
                for (col = 0; col < w; col++) {
                    int idx = row * w + col;
                    idx += w * h;

                    Vector3 v(-hw + (x1 + col) * incX, depth, hh - (y1 + row) * incY);

                    if (curve) {
                        Vector3 planar(v.x, 0, v.z);
                        double dist = planar.Length();
                        if (dist < earthRadius) {
                            double displacement = radius - (sqrt(radius * radius - dist * dist));
                            v.y -= displacement;
                        }
                    }

                    verts[idx].x = (float)v.x;
                    verts[idx].y = (float)v.y;
                    verts[idx].z = (float)v.z;
                    verts[idx].w = 1.0f;

                    tu = u1 + col * incTx;
                    tv = v1 + row * incTy;
                    verts[idx].u = tu;
                    verts[idx].v = tv;

                    verts[idx].SetColor(Color(1.0,1.0,1.0,1.0));
                }
            }
        }

        vb->UnlockBuffer();
    }

    return vb;
}

void StratusCloud::CreateMieLookupTexture()
{
    int i;
    float *pixels = SL_NEW float[2048*3*2];
    memset((void*)pixels, 0, 2048*3*2*sizeof(float));

    float *p = pixels;

    for (i = 0; i < 1801; i++) {
        *p++ = (float)mieRed[i];
        *p++ = (float)mieGreen[i];
        *p++ = (float)mieBlue[i];
    }

    p = pixels + 2048*3;
    for (i = 0; i < 1801; i++) {
        *p++ = (float)mieRedConv[i];
        *p++ = (float)mieGreenConv[i];
        *p++ = (float)mieBlueConv[i];
    }

    // Shove other constants in columns 2000+
    float blut[10] = { 1.1796f, 1.1293f, 1.1382f, 1.0953f, 0.9808f, 0.9077f, 0.7987f, 0.6629f, 0.5042f, 0.3021f };
    float clut[10] = { 0.0138f, 0.0154f, 0.0131f, 0.0049f, 0.0012f, 0.0047f, 0.0207f, 0.0133f, 0.0280f, 0.0783f };
    float kclut[10] = { 0.0265f, 0.0262f, 0.0272f, 0.0294f, 0.0326f, 0.0379f, 0.0471f, 0.0616f, 0.0700f, 0.0700f };
    float tlut[10] = { 0.8389f, 0.8412f, 0.8334f, 0.8208f, 0.8010f, 0.7774f, 0.7506f, 0.7165f, 0.7149f, 0.1000f };
    float rlut[10] = { 0.0547f, 0.0547f, 0.0552f, 0.0564f, 0.0603f, 0.0705f, 0.0984f, 0.1700f, 0.3554f, 0.9500f };

    p = pixels + 2000 * 3;
    for (i = 0; i < 10; i++) {
        *p++ = blut[i];
        *p++ = clut[i];
        *p++ = kclut[i];
    }

    p = pixels + 2048 * 3 + 2000 * 3;
    for (i = 0; i < 10; i++) {
        *p++ = tlut[i];
        *p++ = rlut[i];
        *p++ = 0;
    }

    Renderer::GetInstance()->LoadFloatTextureRGB(pixels, 2048, 2, &mieTexture);

    SL_DELETE[] pixels;
}

static float findClosestDivisor(float target, int value)
{
    for (int candidate = value; candidate > 0; candidate--) {
        float divisor = (float)value / (float)candidate;
        if (divisor > target) {
            return divisor;
        }
    }

    return target;
}

void StratusCloud::Init(double w, double h, double thickness, double density)
{
    width = w;
    height = h;
    depth = thickness;
    textureDensity = density;

    segmentWidth = segmentHeight = 0;

    dim = 50;
    if (density >= 1.0) {
        Configuration::GetIntValue("stratus-grid-dimension-solid", dim);
    } else {
        Configuration::GetIntValue("stratus-grid-dimension", dim);
    }

    double stratusScaleFactor = 1.0;
    Configuration::GetDoubleValue("stratus-scale-factor", stratusScaleFactor);
    double scale = 200000.0 * stratusScaleFactor * Atmosphere::GetUnitScale();

    dim = (int)((dim / scale) * w);

    if (dim <= 1) return;

    hw = (float)w * 0.5f;
    hh = (float)h * 0.5f;

    const float texDensityX = 10.0f / (float)scale;
    const float texDensityY = 10.0f / (float)scale;
    float maxU = ceil((float)w * texDensityX);
    float maxV = ceil((float)h * texDensityY);

    // maxU, maxV must be even divisors of (dim - 1)
    maxU = findClosestDivisor(maxU, dim - 1);
    maxV = findClosestDivisor(maxV, dim - 1);

    segmentWidth = w / maxU;
    segmentHeight = h / maxV;

    //float texelX = 1.0f / scudMapW;
    //float texelY = 1.0f / scudMapH;

    incX = (float)w / (float)(dim - 1);
    incY = (float)h / (float)(dim - 1);
    float incTx = (float)(maxU / (dim - 1));
    float incTy = (float)(maxV / (dim - 1));

    edgeVertsX = int(1.0f / incTx) + 1;
    edgeVertsY = int(1.0f / incTy) + 1;

    if (GetParentCloudLayer()->GetIsInfinite()) edgeVertsX = edgeVertsY = 0;

    double stratusEdgeFalloff = 4.0;
    Configuration::GetDoubleValue("stratus-edge-falloff", stratusEdgeFalloff);

    Configuration::GetFloatValue("stratus-fade-falloff", fadeFalloff);

    const float edgeTexSizeX = incTx * (edgeVertsX - 1);
    const float edgeTexSizeY = incTy * (edgeVertsY - 1);

    CreateTexture(density, edgeTexSizeX, edgeTexSizeY);

    CreateMieLookupTexture();

    vb[TILABLE]      = CreateVertices(edgeVertsX - 1,            dim - edgeVertsX + 1,
                                      edgeVertsY - 1,            dim - edgeVertsY + 1,
                                      edgeTexSizeX,              incTx * (dim - edgeVertsX),
                                      edgeTexSizeY,              incTy * (dim - edgeVertsY));

    vb[LEFT]         = CreateVertices(0,                         edgeVertsX,
                                      edgeVertsY - 1,            dim - edgeVertsY + 1,
                                      0,                         edgeTexSizeX,
                                      incTy * (edgeVertsY - 1),  incTy * (dim - edgeVertsY));

    vb[RIGHT]        = CreateVertices(dim - edgeVertsX,          dim,
                                      edgeVertsY - 1,            dim - edgeVertsY + 1,
                                      1.0f - edgeTexSizeX,       1.0,
                                      incTy * (edgeVertsY - 1),  incTy * (dim - edgeVertsY));

    vb[TOP]          = CreateVertices(edgeVertsX - 1,            dim - edgeVertsX + 1,
                                      0,                         edgeVertsY,
                                      incTx * (edgeVertsX - 1),  incTx * (dim - edgeVertsX),
                                      0,                         edgeTexSizeY);

    vb[BOTTOM]       = CreateVertices(edgeVertsX - 1,            dim - edgeVertsX + 1,
                                      dim - edgeVertsY,          dim,
                                      incTx * (edgeVertsX - 1),  incTx * (dim - edgeVertsX),
                                      1.0f - edgeTexSizeY,       1.0);

    vb[TOPLEFT]      = CreateVertices(0,                         edgeVertsX,
                                      0,                         edgeVertsY,
                                      0,                         edgeTexSizeX,
                                      0,                         edgeTexSizeY);

    vb[TOPRIGHT]     = CreateVertices(dim - edgeVertsX,          dim,
                                      0,                         edgeVertsY,
                                      1.0f - edgeTexSizeX,       1.0,
                                      0,                         edgeTexSizeY);

    vb[BOTTOMLEFT]   = CreateVertices(0,                         edgeVertsX,
                                      dim - edgeVertsY,          dim,
                                      0,                         edgeTexSizeX,
                                      1.0f - edgeTexSizeY,       1.0);

    vb[BOTTOMRIGHT]  = CreateVertices(dim - edgeVertsX,          dim,
                                      dim - edgeVertsY,          dim,
                                      1.0f - edgeTexSizeX,       1.0,
                                      1.0f - edgeTexSizeY,       1.0);


    ibTop[TILABLE] = CreateIndices(dim - (edgeVertsX - 1) * 2, dim - (edgeVertsY - 1) * 2, true);
    ibTop[LEFT] = CreateIndices(edgeVertsX, dim - (edgeVertsY - 1) * 2, true);
    ibTop[RIGHT] = CreateIndices(edgeVertsX, dim - (edgeVertsY - 1) * 2, true);
    ibTop[TOP] = CreateIndices(dim - (edgeVertsX - 1) * 2, edgeVertsY, true);
    ibTop[BOTTOM] = CreateIndices(dim - (edgeVertsX - 1) * 2, edgeVertsY, true);
    ibTop[TOPLEFT] = CreateIndices(edgeVertsX, edgeVertsY, true);
    ibTop[TOPRIGHT] = CreateIndices(edgeVertsX, edgeVertsY, true);
    ibTop[BOTTOMLEFT] = CreateIndices(edgeVertsX, edgeVertsY, true);
    ibTop[BOTTOMRIGHT] = CreateIndices(edgeVertsX, edgeVertsY, true);

    ibBottom[TILABLE] = CreateIndices(dim - (edgeVertsX - 1) * 2, dim - (edgeVertsY - 1) * 2, false);
    ibBottom[LEFT] = CreateIndices(edgeVertsX, dim - (edgeVertsY - 1) * 2, false);
    ibBottom[RIGHT] = CreateIndices(edgeVertsX, dim - (edgeVertsY - 1) * 2, false);
    ibBottom[TOP] = CreateIndices(dim - (edgeVertsX - 1) * 2, edgeVertsY, false);
    ibBottom[BOTTOM] = CreateIndices(dim - (edgeVertsX - 1) * 2, edgeVertsY, false);
    ibBottom[TOPLEFT] = CreateIndices(edgeVertsX, edgeVertsY, false);
    ibBottom[TOPRIGHT] = CreateIndices(edgeVertsX, edgeVertsY, false);
    ibBottom[BOTTOMLEFT] = CreateIndices(edgeVertsX, edgeVertsY, false);
    ibBottom[BOTTOMRIGHT] = CreateIndices(edgeVertsX, edgeVertsY, false);

    ShadeCloud();
}

StratusCloud::~StratusCloud()
{
    Renderer *ren = Renderer::GetInstance();

    if (stratusShader) {
        ren->DeleteShader(stratusShader);
    }

    if (mieTexture) {
        ren->ReleaseTexture(mieTexture);
    }

    for (int i = 0; i < NUM_SECTIONS; i++) {
        if (vb[i]) {
            SL_DELETE vb[i];
        }

        if (textures[i]) {
            ren->ReleaseTexture(textures[i]);
        }

        if (ibTop[i]) {
            SL_DELETE ibTop[i];
        }

        if (ibBottom[i]) {
            SL_DELETE ibBottom[i];
        }

        if (scudMap[i]) {
            SL_DELETE[] scudMap[i];
        }
    }
}

bool StratusCloud::Cull(const Frustum& f)
{
    return false;
}

bool StratusCloud::Draw(int pass, const Vector3 &lightPos, const Vector3& lightDir,
                        const Color &lightColor, bool invalid, const Sky *sky)
{
    lightDirection = lightDir;
    sunColor = Vector3(lightColor.r, lightColor.g, lightColor.b);

    if (pass > 0) {
        Renderer::GetInstance()->SubmitBlendedObject(this);
    }

    return true;
}

void StratusCloud::DrawBlendedObject()
{
    Renderer *ren = Renderer::GetInstance();

    Matrix4 mv;
    Vector3 camPos = ren->GetCamPos();// Atmosphere::GetCurrentAtmosphere()->GetCamPos();
    camPos = camPos * ren->GetInverseBasis3x3();
    Vector3 center = GetWorldPosition() * ren->GetInverseBasis3x3();

    bool viewingFromEdge = false;
    double cloudHeight = 0;

    Color originalAmbient;
    Vector3 forcedAmbient;
    bool ambientOverride = false, doLighting;
    if (GetParentCloudLayer()->GetOverriddenCloudColor(forcedAmbient, doLighting)) {
        originalAmbient = Metaball::GetAmbientColor();
        Color forcedColor = Color(forcedAmbient.x, forcedAmbient.y, forcedAmbient.z);
        if (doLighting) {
            forcedColor = forcedColor * Color(sunColor.x, sunColor.y, sunColor.z);
        }
        Metaball::SetAmbientColor(forcedColor);
        Cloud::SetAmbientColor(forcedColor);
    }

    if (!GetParentCloudLayer()->GetIsInfinite()) {
        double layerX, layerZ;
        GetParentCloudLayer()->GetLayerPosition(layerX, layerZ);

        viewingFromEdge = ((camPos.x < (layerX - GetParentCloudLayer()->GetBaseWidth() * 0.5)) ||
                           (camPos.x > (layerX + GetParentCloudLayer()->GetBaseWidth() * 0.5)) ||
                           (camPos.z > (layerZ + GetParentCloudLayer()->GetBaseLength() * 0.5)) ||
                           (camPos.z < (layerZ - GetParentCloudLayer()->GetBaseLength() * 0.5)));
        cloudHeight = center.y;
    } else {
        cloudHeight = GetParentCloudLayer()->GetBaseAltitudeGeocentric();
    }

    bool drawTop = (camPos.y >= cloudHeight + depth);
    bool drawBottom = (camPos.y <= cloudHeight || (viewingFromEdge && camPos.y < cloudHeight + depth) );

    //bool drawTop = (camPos.y >= cloudHeight);
    //bool drawBottom = (camPos.y < cloudHeight + depth);

    if (!(drawTop || drawBottom)) return;

    ren->GetModelviewMatrix(&mv);

    bool alwaysWriteDepth = false;
    if (GetParentCloudLayer()->GetDensity() > 0.9) {
        Configuration::GetBoolValue("stratus-always-write-depth", alwaysWriteDepth);
    }

    ren->EnableBlending(SRCALPHA, INVSRCALPHA);
    ren->EnableDepthReads(true);
    ren->EnableDepthWrites(alwaysWriteDepth ? true : drawTop);
    ren->EnableTexture2D(true);
    ren->EnableLighting(false);
    ren->EnableBackfaceCulling(viewingFromEdge ? false : true);

    bool setFog = false;

    if (!ren->GetFogEnabled()) {
        ren->EnableFog(true);
        ren->ConfigureFog(Billboard::GetFogDensity(), 1, 100000, Billboard::GetFogColor());
        setFog = true;
    }

    if (stratusShader) {
        Vector4 fog;
        Color fogColor = Billboard::GetFogColor();
        fog.x = fogColor.r;
        fog.y = fogColor.g;
        fog.z = fogColor.b;
        fog.w = Billboard::GetFogDensity();
        ren->SetConstantVector4(stratusShader, "sl_fogColorAndDensity", fog);

        float outputScale = Atmosphere::GetCurrentAtmosphere()->GetOutputScale();
        Renderer::GetInstance()->SetConstantVector(stratusShader, "sl_outputScale", Vector3((double)outputScale, 0.0, 0.0));

        Vector4 cloudTint(lightScale, lightScale, lightScale, 1.0);
        Color forceColor;
        if (Billboard::GetForceConstantColor(forceColor))
            cloudTint = forceColor.ToVector4();
        ren->SetConstantVector4(stratusShader, "sl_cloudTint", cloudTint);

        ren->SetConstantVector4(stratusShader, "sl_lightDirection", lightDirection);
        ren->SetConstantVector4(stratusShader, "sl_sunColor", sunColor);

        Color ambientColor = Metaball::GetAmbientColor();
        ren->SetConstantVector4(stratusShader, "sl_skyColor", Vector3(ambientColor.r,
                                ambientColor.g, ambientColor.b) * skyColorScale);

        double fadeFactor = 0.0;
        if (GetParentCloudLayer()->GetIsInfinite() && GetParentCloudLayer()->GetFadeTowardEdges()) {
            fadeFactor = fadeFalloff;
        }
        ren->SetConstantVector4(stratusShader, "sl_layerSizeAndUnitScale", Vector4(
                                    width, height, Atmosphere::GetUnitScale(), fadeFactor));

        ren->SetConstantMatrix(stratusShader, "sl_invBasis", ren->GetInverseBasis4x4());

        Vector4 groundColor;
        double coverage = GetParentCloudLayer()->GetDensity();
        double xmit = exp(-(coverage*coverage));
        groundColor = Vector4(ambientColor.r * groundColorScale.x * xmit,
                              ambientColor.g * groundColorScale.y * xmit,
                              ambientColor.b * groundColorScale.z * xmit, 1.0);
        ren->SetConstantVector4(stratusShader, "sl_groundColor", groundColor);

    }

    Vector3 modelOffset;

    if (GetParentCloudLayer()->GetIsInfinite()) {
        // Translate to the Camera position
        Matrix4 xlate;

        Vector3 offset;
        camPos = Atmosphere::GetCurrentAtmosphere()->GetCamPos();
        camPos = camPos * ren->GetInverseBasis3x3();

        //offset.x = -fmod(camPos.x, segmentWidth) + fmod(center.x, segmentWidth);
        offset.y = GetParentCloudLayer()->GetBaseAltitudeGeocentric() - camPos.y;
        //offset.z = -fmod(camPos.z, segmentHeight) + fmod(center.z, segmentHeight);

        double dx = center.x - camPos.x;
        double dz = center.z - camPos.z;
        offset.x = fmod(dx, segmentWidth);
        offset.z = fmod(dz, segmentWidth);

        if (dx > 0) offset.x -= segmentWidth;
        if (dz > 0) offset.z -= segmentWidth;

        offset = offset * ren->GetBasis3x3();

        xlate.elem[0][3] = offset.x;
        xlate.elem[1][3] = offset.y;
        xlate.elem[2][3] = offset.z;

        Matrix4 view = mv;
        view.elem[0][3] = view.elem[1][3] = view.elem[2][3] = 0;
        ren->SetModelviewMatrix(view * xlate * ren->GetInverseBasis4x4());

        modelOffset = offset + ren->GetCamPos();
    } else {
        // Translate to the cloud position
        Matrix4 xlate;
        Vector3 worldPos = GetWorldPosition();
        xlate.elem[0][3] = worldPos.x;
        xlate.elem[1][3] = worldPos.y;
        xlate.elem[2][3] = worldPos.z;
        modelOffset = worldPos;
        Matrix4 localBasis = GetParentCloudLayer()->GetLocalBasis4();
        Matrix4 invBasis = localBasis.Inverse();
        ren->MultiplyModelviewMatrix(xlate * invBasis);
    }

    double scudThickness;
    Configuration::GetDoubleValue("stratus-scud-thickness", scudThickness);
    scudThickness *= Atmosphere::GetUnitScale();

    float extinctionFactor = 1.0f;
    Configuration::GetFloatValue("stratus-extinction-factor", extinctionFactor);

    bool flatTops = false;
    Configuration::GetBoolValue("stratus-flat-tops", flatTops);

    if (stratusShader) {
        ren->BindShader(stratusShader, VERTEX_PROGRAM);
        Vector3 localUp = GetParentCloudLayer()->GetLocalUpVector();
        Vector3 up = localUp * scudThickness;
        Vector4 dvc(up.x, up.y, up.z, 1.0);
        Vector4 vfade(fade * alpha, 1.0, GetParentCloudLayer()->GetDensity(), 0.0);//textureDensity == 1.0 ? 1.0 : 0.0, 0.0, 0.0);
        //if (GetParentCloudLayer()->GetIsInfinite()) {
        //    vfade.y = 0.0;
        //}
        Vector4 upThickness(localUp.x, localUp.y, localUp.z,
                            flatTops ? 0 : depth);
        ren->SetConstantVector4(stratusShader, "sl_displacementVectorAndContrast", dvc);
        ren->SetConstantVector4(stratusShader, "sl_fadeAndDisplacementFactor", vfade);
        ren->SetConstantVector4(stratusShader, "sl_upVectorAndThickness", upThickness);

        ren->SetConstantVector(stratusShader, "sl_extinctionFactor", Vector3(extinctionFactor, 0.0f, 0.0f));

        Matrix4 modelview, proj, modelviewProj;
        ren->GetModelviewMatrix(&modelview);
        ren->GetProjectionMatrix(&proj);
        modelviewProj = proj * modelview;
        ren->SetConstantMatrix(stratusShader, "sl_modelViewProj", modelviewProj);
        ren->SetConstantMatrix(stratusShader, "sl_modelView", modelview);
        ren->SetConstantVector(stratusShader, "sl_modelPos", modelOffset * ren->GetInverseBasis3x3());
        ren->SetConstantVector(stratusShader, "sl_cameraPos", ren->GetCamPos());

    }

    int i;
    for (i = 0; i < NUM_SECTIONS; i++) {
        if (textures[i] && vb[i] && ibTop[i]) {

            if (stratusShader) {
                ren->SetConstantVector4(stratusShader, "sl_layerThicknessAndIsTop", Vector4(depth,scudThickness,depth,1.0));
            }

            ren->EnableTexture(textures[i], 0);
            ren->EnableTexture(mieTexture, 1);
            ren->BindShader(stratusShader, VERTEX_PROGRAM);

            if (drawTop) {
                vb[i]->DrawIndexedStrip(*ibTop[i], true);
            }
        }
    }

    for (i = 0; i < NUM_SECTIONS; i++) {
        if (textures[i] && vb[i] && ibBottom[i]) {
            ren->EnableTexture(textures[i], 0);
            ren->EnableTexture(mieTexture, 1);
            ren->BindShader(stratusShader, VERTEX_PROGRAM);

            if (drawBottom) {
                if (stratusShader) {

                    ren->SetConstantVector4(stratusShader, "sl_layerThicknessAndIsTop", Vector4(depth,scudThickness,camPos.y > cloudHeight ? 1.0 : 0.0,0.0));

                    Vector3 localUp = GetParentCloudLayer()->GetLocalUpVector();
                    Vector4 v(localUp.x, localUp.y, localUp.z, 0);
                    ren->SetConstantVector4(stratusShader, "sl_upVectorAndThickness", v);
                    v.x = v.y = v.z = 0;
                    v.w = 1.0;
                    ren->SetConstantVector4(stratusShader, "sl_displacementVectorAndContrast", v);
                }

                vb[i]->DrawIndexedStrip(*ibBottom[i], true);
            }
        }
    }

    if (stratusShader) {
        ren->UnbindShader(VERTEX_PROGRAM);
    }

    ren->SetCurrentColor(Color(1,1,1));

    if (setFog) {
        ren->EnableFog(false);
    }

    //ren->EnableBackfaceCulling(true);

    ren->EnableDepthWrites(true);

    ren->SetModelviewMatrix(mv);
}

bool StratusCloud::Update(unsigned long now, bool forceUpdate)
{
    return false;
}

void StratusCloud::GetSize(double& pwidth, double& pdepth, double& pheight) const
{
    pwidth = width;
    pdepth = height;
    pheight = depth;
}
