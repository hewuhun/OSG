// Copyright (c) 2012-2013 Sundog Software LLC. All rights reserved worldwide.

#include <FeSilverliningLib/CloudBackdrop.h>
#include <FeSilverliningLib/Renderer.h>
#include <FeSilverliningLib/Configuration.h>
#include <FeSilverliningLib/Utils.h>
#include <FeSilverliningLib/CloudLayer.h>
#include <FeSilverliningLib/Cloud.h>

using namespace SilverLining;

CloudBackdrop::CloudBackdrop(double pDirection, double pAngularSize, double pDistance)
{
    vb = 0;
    ib = 0;
    direction = pDirection;
    angularSize = pAngularSize;
    distance = pDistance;
    initialized = false;
    rendered = false;
    renderTexture[0] = renderTexture[1] = 0;
    transitionStartTime = 0;
    currentTexture = 0;
    blend = 0;
    numClouds = 0;
    firstFrame = true;

    horizontalThreshold = 0.15;
    Configuration::GetDoubleValue("cloud-backdrop-horizontal-threshold", horizontalThreshold);
    verticalThreshold = 0.01;
    Configuration::GetDoubleValue("cloud-backdrop-vertical-threshold", verticalThreshold);

    transitionLength = 1.0;
    Configuration::GetFloatValue("cloud-backdrop-texture-transition-time", transitionLength);

    // Figure out dimensions of the backdrop
    backdropAngularHeight = 45.0;
    Configuration::GetDoubleValue("cloud-backdrop-angular-height", backdropAngularHeight);
    backdropAngularHeight = RADIANS(backdropAngularHeight);

    width = pDistance * tan(pAngularSize * 0.5) * 2.0;
    height = pDistance * tan(backdropAngularHeight * 0.5) * 2.0;

    radius = sqrt(width * width + height * height) * 0.5;

    relativePosition = Vector3(distance * sin(direction), 0, distance * cos(direction));
}

CloudBackdrop::~CloudBackdrop()
{
    if (vb) SL_DELETE vb;
    if (ib) SL_DELETE ib;


    Renderer *ren = Renderer::GetInstance();
    if (ren) {
        if (renderTexture[0]) {
            ren->ReleaseRenderTexture(renderTexture[0]);
        }
        if (renderTexture[1]) {
            ren->ReleaseRenderTexture(renderTexture[1]);
        }
    }
}

bool CloudBackdrop::Initialize(ShaderHandle pShader)
{
    shader = pShader;
    initialized = SetupGeometry();

    Renderer *ren = Renderer::GetInstance();
    imposterDimX = 512;
    imposterDimY = 512;
    Configuration::GetIntValue("cloud-backdrop-texture-dimension-x", imposterDimX);
    Configuration::GetIntValue("cloud-backdrop-texture-dimension-y", imposterDimY);
    if (!ren->InitRenderTexture(imposterDimX, imposterDimY, &renderTexture[0])) {
        initialized = false;
    }
    if (!ren->InitRenderTexture(imposterDimX, imposterDimY, &renderTexture[1])) {
        initialized = false;
    }

    worldPosition = Atmosphere::GetCurrentAtmosphere()->GetCamPos() + relativePosition * ren->GetBasis3x3();

    return initialized;
}

bool CloudBackdrop::SetupGeometry()
{
    vb = SL_NEW VertexBuffer(4);
    if (vb->LockBuffer()) {
        float fHalfWidth = (float)width * 0.5f;
        float fHalfHeight = (float)height * 0.5f;

        bool DX = Renderer::GetInstance()->GetIsDirectX();

        Vertex *verts = vb->GetVertices();
        if (verts) {
            verts[0].x = -fHalfWidth;
            verts[0].y = -fHalfHeight;
            verts[0].z = 0;
            verts[0].w = 1.0f;
            verts[0].SetUV(1.0f, DX ? 1.0f : 0.0f);
            verts[0].SetColor(Color(1,1,1));

            verts[1].x = fHalfWidth;
            verts[1].y = -fHalfHeight;
            verts[1].z = 0;
            verts[1].w = 1.0f;
            verts[1].SetUV(0.0f, DX ? 1.0f : 0.0f);
            verts[1].SetColor(Color(1,1,1));

            verts[2].x = fHalfWidth;
            verts[2].y = fHalfHeight;
            verts[2].z = 0;
            verts[2].w = 1.0f;
            verts[2].SetUV(0.0f, DX ? 0.0f : 1.0f);
            verts[2].SetColor(Color(1,1,1));

            verts[3].x = -fHalfWidth;
            verts[3].y = fHalfHeight;
            verts[3].z = 0;
            verts[3].w = 1.0f;
            verts[3].SetUV(1.0f, DX ? 0.0f : 1.0f);
            verts[3].SetColor(Color(1,1,1));
        }
        vb->UnlockBuffer();
    } else {
        return false;
    }

    ib = SL_NEW IndexBuffer(4);
    if (ib->LockBuffer()) {
        Index *indices = ib->GetIndices();
        indices[0] = 1;
        indices[1] = 2;
        indices[2] = 0;
        indices[3] = 3;
        ib->UnlockBuffer();
    } else {
        return false;
    }

    return true;
}

bool CloudBackdrop::Update(const Frustum& cameraFrustum, const SL_VECTOR(CloudLayer *)& cloudLayers, unsigned long now, const Sky& sky,
                           const Vector3& lightDir, const Color& lightColor, bool forceUpdate)
{
    if (!initialized) return false;

    Renderer *ren = Renderer::GetInstance();
    if (!ren) return false;
    Vector3 camPos = Atmosphere::GetCurrentAtmosphere()->GetCamPos();
    Vector3 camPosYUp = camPos * ren->GetInverseBasis3x3();

    double yThreshold = distance * verticalThreshold;
    double xThreshold = distance * horizontalThreshold;
    Vector3 diff = camPosYUp - lastUpdatePosition;
    double yDiff = fabs(diff.y);
    double xDiff = sqrt(diff.x * diff.x + diff.z * diff.z);

    if (!forceUpdate) {
        if (!(xDiff > xThreshold || yDiff > yThreshold || lightDir != lastLightDir || lightColor != lastLightColor)) {
            return false;
        }
    }

    lastUpdatePosition = camPosYUp;
    lastLightColor = lightColor;
    lastLightDir = lightDir;

    worldPosition = Atmosphere::GetCurrentAtmosphere()->GetCamPos() + relativePosition * ren->GetBasis3x3();

    Matrix4 savedProj;
    ren->GetProjectionMatrix(&savedProj);

    ren->PushTransforms();

    Frustum f;
    Plane pfar = cameraFrustum.GetPlane(Frustum::PBACK);
    double farClip = pfar.GetDistance();
    if (farClip < distance) {
        farClip = distance;
    }

    Vector3 Up = ren->GetUpVector();
    ren->SetModelviewLookat(Atmosphere::GetCurrentAtmosphere()->GetCamPos(), GetWorldPosition(), Up);

    ren->SetProjectionMatrix(distance, farClip * 1.2, angularSize, backdropAngularHeight);

    ren->ExtractViewFrustum(f);
    if (cameraFrustum.GetNumCullingPlanes() == 5) {
        f.EnableFarClipCulling(false);
    }

    // hack the billboard matrix
    Matrix4 billboard;

    Matrix4 modelview;
    ren->GetModelviewMatrix(&modelview);
    Vector3 Normal(modelview.elem[2][0], modelview.elem[2][1], modelview.elem[2][2]);
    Vector3 Right = Up.Cross(Normal);
    Up = Normal.Cross(Right);

    if (ren->GetIsRightHanded()) {
        billboard = Matrix4(Right.x, Up.x, Normal.x, 0,
                            Right.y, Up.y, Normal.y, 0,
                            Right.z, Up.z, Normal.z, 0,
                            0,   0,    0,      1);
    } else {
        billboard = Matrix4(Right.x, Up.x, -Normal.x, 0,
                            Right.y, Up.y, -Normal.y, 0,
                            Right.z, Up.z, -Normal.z, 0,
                            0,   0,    0,       1);
    }

    Billboard::OverrideBillboardMatrix(billboard);

    // Ensure default depth range so our projection matrix works properly
    float nearDepth, farDepth;
    ren->GetDepthRange(nearDepth, farDepth);
    ren->SetDepthRange(0.0f, 1.0f);

    numClouds = 0;
    SL_VECTOR(CloudLayer *)::const_iterator it;
    for (it = cloudLayers.begin(); it != cloudLayers.end(); it++) {
        CloudLayer *cl = *it;

        if (!cl->IsRenderable()) continue;
        if (cl->Cull(f)) continue;

        CloudTypes type = cl->GetType();
        if (type != CUMULUS_MEDIOCRIS && type != CUMULUS_CONGESTUS && type != CUMULUS_CONGESTUS_HI_RES) continue;

        Vector3 lightPos(0, 1, 0);

        Vector3 anchor;

        if (cl->GetCloudWrapping()) {
            double layerX, layerZ;
            cl->GetLayerPosition(layerX, layerZ);
            anchor = Vector3(layerX, cl->GetBaseAltitudeGeocentric(), layerZ);
        } else {
            Vector3 camPos = Atmosphere::GetCurrentAtmosphere()->GetCamPos();
            anchor = camPos * Renderer::GetInstance()->GetInverseBasis3x3();
        }

        const SL_VECTOR(Cloud *) clouds = cl->GetClouds();
        SL_VECTOR(Cloud *)::const_iterator cit;
        for (cit = clouds.begin(); cit != clouds.end(); cit++) {
            Cloud *cloud = *cit;

            double wrapFade = cl->ComputeWrapFade(cloud, anchor);
            cloud->SetFade(cl->GetFade());
            cloud->SetAlpha(cl->GetAlpha() * wrapFade);

            if (!CullCloudInside(f, cloud)) {
                cloud->Draw(1, lightPos, lightDir, lightColor, false, &sky);
                numClouds++;
            }
        }
    }

    if (numClouds > 0) {
        bool ok = true;

        if (firstFrame) {
            ok = DrawClouds(0) && DrawClouds(1);
            firstFrame = false;
        } else {
            ok = DrawClouds(currentTexture);
        }

        ren->ClearBlendedObjects();

        currentTexture = currentTexture == 0 ? 1 : 0;
        transitionStartTime = getMilliseconds();

        if (!ok) {
            ren->PopTransforms();
            Billboard::RestoreBillboardMatrix();
            return false;
        }
    }

    ren->ClearBlendedObjects();

    Billboard::RestoreBillboardMatrix();

    ren->SetDepthRange(nearDepth, farDepth);

    ren->DisableBlending();

    ren->PopTransforms();

    ren->SetProjectionMatrix(savedProj);

    rendered = true;

    return true;
}

bool CloudBackdrop::DrawClouds(int texNum)
{
    Renderer *ren = Renderer::GetInstance();
    if (!ren) return false;

    int savedX, savedY, savedW, savedH;
    ren->GetViewport(savedX, savedY, savedW, savedH);

    if (ren->MakeRenderTextureCurrent(renderTexture[texNum], true)) {
        ren->SetViewport(0, 0, imposterDimX, imposterDimY);

        ren->SortAndDrawBlendedObjects(false, false);

        ren->BindRenderTexture(renderTexture[texNum]);

        ren->SetViewport(savedX, savedY, savedW, savedH);

        return true;
    }

    return false;
}
bool CloudBackdrop::CullCloudInside(const Frustum& f, const Cloud * cloud)
{
    double w, h, d;
    cloud->GetSize(w, d, h);
    w *= 0.5;
    d *= 0.5;

    Vector3 center = cloud->GetWorldPosition();

    for (int i = 0; i < f.GetNumCullingPlanes(); i++) {
        const Plane& p = f.GetPlane(i);
        const Vector3& N = p.GetNormal();
        double distance = N.Dot(center) + p.GetDistance();

        double RDotN = w * N.x;
        double SDotN = h * N.y;
        double TDotN = d * N.z;

        double rEff = fabs( RDotN ) + fabs( SDotN ) + fabs( TDotN );

        // Cloud is entirely outside the plane
        if (distance < -rEff) {
            return true;
        }

        // Cull if we are only partially within the frustum; we don't want
        // half a cloud in the backdrop
        if (fabs(distance) < rEff) {
            return (i == Frustum::PNEAR ? true : false);
        }
    }

    return false;
}

bool CloudBackdrop::Draw(int)
{
    if (!initialized || !rendered) return false;

    if (numClouds == 0) return true;

    Renderer *ren = Renderer::GetInstance();
    if (ren) {
        worldPosition = Atmosphere::GetCurrentAtmosphere()->GetCamPos() + relativePosition * ren->GetBasis3x3();
    }

    unsigned long now = getMilliseconds();
    blend = ((float)(now - transitionStartTime) * 0.001f) / transitionLength;
    if (blend > 1.0f) blend = 1.0f;
    if (currentTexture == 1) { // 0 is the one that was just updated
        blend = 1.0f - blend;
    }

    Renderer::GetInstance()->SubmitBlendedObject(this);
    return true;
}

const Vector3& CloudBackdrop::GetWorldPosition() const
{
    return worldPosition;
}

void CloudBackdrop::DrawBlendedObject()
{
    if (numClouds == 0) return;

    Renderer *ren = Renderer::GetInstance();
    if (ren) {
        ren->PushTransforms();

        Matrix4 modelView;
        ren->GetModelviewMatrix(&modelView);
        // Blow away translation
        for (int row = 0; row < 3; row++) modelView.elem[row][3] = 0.0;

        Vector3 pos = relativePosition;
        Matrix4 translate;
        translate.elem[0][3] = pos.x;
        translate.elem[1][3] = pos.y;
        translate.elem[2][3] = pos.z;

        double theta = direction;
        double cosTheta = cos(theta);
        double sinTheta = sin(theta);

        Matrix4 rotate;

        rotate.elem[0][0] = cosTheta;
        rotate.elem[0][2] = sinTheta;

        rotate.elem[2][0] = -sinTheta;
        rotate.elem[2][2] = cosTheta;

        Matrix4 object = ren->GetInverseBasis4x4() * translate * rotate;
        modelView = modelView * object;

        ren->SetModelviewMatrix(modelView * object);

        ren->EnableBackfaceCulling(false);
        ren->EnableLighting(false);
        ren->EnableTexture2D(true);

        ren->EnableBlending(ONE, INVSRCALPHA);
        ren->EnableDepthReads(true);
        ren->EnableDepthWrites(false);

        TextureHandle tex0, tex1;
        ren->GetRenderTextureTextureHandle(renderTexture[0], &tex0);
        ren->GetRenderTextureTextureHandle(renderTexture[1], &tex1);
        ren->EnableTexture(tex0, 0);
        ren->EnableTexture(tex1, 1);

        ren->BindShader(shader, VERTEX_PROGRAM);
        ren->SetConstantVector(shader, "sl_textureBlend", Vector3(blend, blend, blend));
        Matrix4 mvp;
        Matrix4 projection;
        ren->GetProjectionMatrix(&projection);
        mvp = projection * modelView;
        ren->SetConstantMatrix(shader, "sl_modelViewProj", mvp);

        //ren->DisableBlending();

        ren->DrawStrip(vb->GetHandle(), ib->GetHandle(), 0, 4, 4, false);
        ren->UnbindShader(VERTEX_PROGRAM);

        ren->DisableTexture(0);
        ren->DisableTexture(1);

        //ren->EnableDepthWrites(true);

        ren->PopTransforms();
    }
}

bool CloudBackdrop::Cull(const Frustum& f)
{
    Vector3 center = GetWorldPosition();

    for (int i = 0; i < f.GetNumCullingPlanes(); i++) {
        const Plane& p = f.GetPlane(i);
        const Vector3& N = p.GetNormal();

        // find the distance to this plane
        double distance = N.Dot(center) + p.GetDistance();

        // if this distance is < -sphere.radius, we are outside
        if(distance < -radius)
            return true;

        // else if the distance is between +- radius, then we intersect
        if(fabs(distance) < radius)
            return false;
    }

    return false;
}
