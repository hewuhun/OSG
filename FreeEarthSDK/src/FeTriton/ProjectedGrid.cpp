// Copyright (c) 2011-2015 Sundog Software LLC. All rights reserved worldwide.

#include <FeTriton/ProjectedGrid.h>
#include <FeTriton/Configuration.h>
#include <FeTriton/Environment.h>
#include <FeTriton/WaterModel.h>
#include <FeTriton/OceanLightingModel.h>
#include <FeTriton/WakeManager.h>
#include <FeTriton/Matrix4.h>

using namespace Triton;

ProjectedGrid::ProjectedGrid(bool pEnableBreakingWaves) : environment(0), resolution(128), waterModelType(TESSENDORF), waterModel(0),
    lightingModel(0), antiAliasing(1.0), wireframe(false), wakeManager(0), aspectRatio(1.0), planarHeight(100.0),
    maxAllowableDisplacement(1.5), planarAdjust(10.0f), enableBreakingWaves(pEnableBreakingWaves), planarReflectionBlend(0.75f),
    depthOffset(0), horizonOffset(0.001), intersectionVectorLength(100000.0), intersectionVectorLengthAboveWater(100000.0),
    intersectionVectorLengthProjection(100000.0), doubleSided(false), alphaMask(false),
    Lx(512.0f), Ly(512.0f), meshSwitchAltitude(1000.0), meshSize(50000.0), useMesh(false),
    generateDepthTexture(false), depthTexture(0), depthTextureSize(512), gridClipFactor(4.0), numOctaves(3),
    doubleRefractionColor(0.3, 0.7, 0.6), doubleRefractionIntensity(0.0f), gamma(1.0f), godRayFade(1.0f), gridExpansionScale(1.3),
    nearZ(-1.0), farZ(1.0), reflectionScale(1.0f)
{
}

bool ProjectedGrid::Initialize(const Environment *env, WaterModelTypes type, unsigned int gridResolution, unsigned int pMeshGridResolution)
{
    if (!env) return false;

    environment = env;
    resolution = gridResolution;
    meshGridResolution = pMeshGridResolution;
    waterModelType = type;

    nearZ = -1.0, farZ = 1.0;
    if (environment->IsDirectX()) {
        Configuration::GetDoubleValue("directx-near-clip", nearZ);
        Configuration::GetDoubleValue("directx-far-clip", farZ);
    } else {
        Configuration::GetDoubleValue("opengl-near-clip", nearZ);
        Configuration::GetDoubleValue("opengl-far-clip", farZ);
    }

    Configuration::GetIntValue("depth-texture-size", depthTextureSize);

    Configuration::GetFloatValue("fft-grid-size-x", Lx);
    Configuration::GetFloatValue("fft-grid-size-y", Ly);
    Lx /= (float)env->GetWorldUnits();
    Ly /= (float)env->GetWorldUnits();

    Configuration::GetDoubleValue("mesh-size", meshSize);
    Configuration::GetBoolValue("use-mesh", useMesh);

    meshSize /= env->GetWorldUnits();

    Configuration::GetDoubleValue("grid-clip-factor", gridClipFactor);

    Configuration::GetBoolValue("alpha-mask", alphaMask);

    Configuration::GetBoolValue("grid-double-sided", doubleSided);

    Configuration::GetDoubleValue("grid-horizon-offset", horizonOffset);

    Configuration::GetDoubleValue("grid-anti-aliasing", antiAliasing);

    Configuration::GetFloatValue("grid-planar-height", planarHeight);
    planarHeight /= (float)env->GetWorldUnits();

    Configuration::GetFloatValue("planar-adjust", planarAdjust);
    planarAdjust /= (float)env->GetWorldUnits();

    Configuration::GetDoubleValue("max-allowable-displacement", maxAllowableDisplacement);

    Configuration::GetDoubleValue("grid-expansion-scale", gridExpansionScale);

    Configuration::GetDoubleValue("intersection-vector-length", intersectionVectorLength);
    Configuration::GetDoubleValue("intersection-vector-length-above-water", intersectionVectorLengthAboveWater);
    Configuration::GetDoubleValue("intersection-vector-length-projection", intersectionVectorLengthProjection);

    Configuration::GetIntValue("num-detail-octaves", numOctaves);

    double refractR = 0.3, refractG = 0.7, refractB = 0.6;
    Configuration::GetDoubleValue("double-refraction-r", refractR);
    Configuration::GetDoubleValue("double-refraction-g", refractG);
    Configuration::GetDoubleValue("double-refraction-b", refractB);
    doubleRefractionColor = Vector3(refractR, refractG, refractB);

    Configuration::GetFloatValue("double-refraction-intensity", doubleRefractionIntensity);

    CoordinateSystem cs = env->GetCoordinateSystem();

    if (cs == WGS84_ZUP || cs == WGS84_YUP) {
        double eqRad = 6378137.00, polRad = 6356752.3142;

        Configuration::GetDoubleValue("equatorial-earth-radius-meters", eqRad);
        Configuration::GetDoubleValue("polar-earth-radius-meters", polRad);

        eqRad /= env->GetWorldUnits();
        polRad /= env->GetWorldUnits();

        if (cs == WGS84_ZUP) {
            earthRadii = Vector3(eqRad, eqRad, polRad);
            north = Vector3(0, 0, 1);
        } else {
            earthRadii = Vector3(eqRad, polRad, eqRad);
            north = Vector3(0, 1, 0);
        }
    }

    if (cs == SPHERICAL_ZUP || cs == SPHERICAL_YUP) {
        double meanRad = 6371009.0;
        Configuration::GetDoubleValue("mean-earth-radius-meters", meanRad);
        meanRad /= env->GetWorldUnits();
        earthRadii = Vector3(meanRad, meanRad, meanRad);

        if (cs == SPHERICAL_ZUP) {
            north = Vector3(0, 0, 1);
        } else {
            north = Vector3(0, 1, 0);
        }
    }

    double m[9];
    if (cs == FLAT_ZUP || cs == SPHERICAL_ZUP || cs == WGS84_ZUP) {
        m[0] = 1.0;
        m[1] = 0.0;
        m[2] = 0.0;
        m[3] = 0.0;
        m[4] = 1.0;
        m[5] = 0.0;
        m[6] = 0.0;
        m[7] = 0.0;
        m[8] = 1.0;
    } else {
        m[0] = 1.0;
        m[1] =  0.0;
        m[2] = 0.0;
        m[3] = 0.0;
        m[4] =  0.0;
        m[5] = 1.0;
        m[6] = 0.0;
        m[7] = -1.0;
        m[8] = 0.0;
    }

    basis = Matrix3(m);
    invBasis = basis.Transpose();

    return true;
}

ProjectedGrid::~ProjectedGrid()
{
    if (waterModel) {
        TRITON_DELETE waterModel;
        waterModel = 0;
    }

    if (lightingModel) {
        TRITON_DELETE lightingModel;
        lightingModel = 0;
    }
}

void ProjectedGrid::ComputeDirections(const Vector3& camPos, Vector3& northPole)
{
    Vector3 up = camPos;
    up.Normalize();
    northPole = north;
    localEast = north.Cross(up);
    const double epsilon = 1E-10;
    if (localEast.SquaredLength() < epsilon) {
        // North or south pole, pick an arbitrary North direction.
        Vector3 fakeNorth(1, 0, 0);
        localEast = fakeNorth.Cross(up);
        northPole = fakeNorth;
    }
    localEast.Normalize();
    localNorth = up.Cross(localEast);
    localNorth.Normalize();
}

void ProjectedGrid::UpdateBasisMatrices()
{
    if (environment) {
        if (environment->IsGeocentric()) {
            //Vector3 zAxis = environment->GetUpVector();

            Vector3 camPos(environment->GetCameraPosition());

            // FIXME: This looks suspect.
            double A = camPos.x / (earthRadii.x * earthRadii.x);
            double B = camPos.y / (earthRadii.y * earthRadii.y);
            double C = camPos.z / (earthRadii.z * earthRadii.z);
            double distance = Vector3(A, B, C).Length();
            A /= distance;
            B /= distance;
            C /= distance;

            Vector3 zAxis(A, B, C);
            zAxis.Normalize();

            Vector3 xAxis = north.Cross(zAxis);
            xAxis.Normalize();
            Vector3 yAxis = zAxis.Cross(xAxis);
            yAxis.Normalize();

            basis = Matrix3(xAxis.x, yAxis.x, zAxis.x,
                            xAxis.y, yAxis.y, zAxis.y,
                            xAxis.z, yAxis.z, zAxis.z);

            invBasis = basis.Transpose();
        }
    }
}

float ProjectedGrid::GetChoppiness() const
{
    if (waterModel) {
        return waterModel->GetChoppiness();
    } else {
        return 0;
    }
}

void ProjectedGrid::SetChoppiness(float chop)
{
    if (waterModel) {
        waterModel->SetChoppiness(chop);
    }
}

float ProjectedGrid::GetDepth(Vector3& floorNormal) const
{
    if (waterModel) {
        return waterModel->GetDepth(floorNormal);
    } else {
        return 0;
    }
}

float ProjectedGrid::GetSlope(float& slopeHeading)
{
    if (waterModel) {
        return waterModel->GetSlope(slopeHeading);
    } else {
        slopeHeading = 0;
        return 0;
    }
}

void ProjectedGrid::SetDepth(float depth, const Vector3& floorNormal)
{
    float slope = 0.0f;
    float slopeHeading = 0.0f;

    Vector3 down = environment->GetUpVector() * -1.0;
    Vector3 vSlope = down - floorNormal * down.Dot(floorNormal);
    if (vSlope.SquaredLength() != 0) {
        vSlope.Normalize();

        Vector3 slopeZup = vSlope * invBasis;
        slopeHeading = (float)atan2(slopeZup.y, slopeZup.x);
    }

    if (waterModel) {
        waterModel->SetDepth(depth, floorNormal);
        waterModel->SetSlope(slope, slopeHeading);
    }
}

bool ProjectedGrid::GetIntersection(const Vector3& rayPosition, const Vector3& rayDirection, Vector3& intersectPoint) const
{
    if (!environment) return false;

    Vector3 dirVector = rayDirection * intersectionVectorLength;

    Vector4 p4(rayPosition);
    Vector4 d4(dirVector);
    Vector4 i4;

    bool hasIntersection = false;
    if (environment->IsGeocentric()) {
        hasIntersection = ProjectToEllipsoid(p4, d4, i4, 0);
    } else {
        hasIntersection = ProjectToPlane(p4, d4, i4, 0);
    }

    i4 = i4 * (1.0 / i4.w);
    intersectPoint = Vector3(i4.x, i4.y, i4.z);

    return hasIntersection;
}

bool ProjectedGrid::IsAboveWater()
{
    if (!environment) return false;

    Vector3 camPos = environment->GetCameraPosition();
    Vector4 camPos4(camPos);

    Vector3 down = environment->GetUpVector() * -intersectionVectorLengthAboveWater;
    Vector4 down4(down);

    bool intersect = false;
    Vector4 intersectPoint;
    if (environment->IsGeocentric()) {
        intersect = ProjectToEllipsoid(camPos4, down4, intersectPoint, 0);
    } else {
        intersect = ProjectToPlane(camPos4, down4, intersectPoint, 0);
    }

    return intersect;
}

bool ProjectedGrid::GetGridOffsetRoll(Vector3& offset, double& roll)
{
    // If not planar, punt
    if (!environment) return false;

    // Build up line segments of the far clip's edges
    struct LineSegment {
        Vector4 p0, p1;
    };

    Vector4 bottomLeft =  Vector4(-1.0, -1.0, 1.0, 1.0);
    Vector4 topLeft =     Vector4(-1.0,  1.0, 1.0, 1.0);
    Vector4 topRight =    Vector4( 1.0,  1.0, 1.0, 1.0);
    Vector4 bottomRight = Vector4( 1.0, -1.0, 1.0, 1.0);

    LineSegment edges[4];
    edges[0].p0 = bottomLeft;
    edges[0].p1 = topLeft;
    edges[1].p0 = topLeft;
    edges[1].p1 = topRight;
    edges[2].p0 = topRight;
    edges[2].p1 = bottomRight;
    edges[3].p0 = bottomRight;
    edges[3].p1 = bottomLeft;

    // Transform into world coordinates
    Matrix4 modelView(environment->GetCameraMatrix());
    Matrix4 projection(environment->GetProjectionMatrix());
    Matrix4 modelViewProj = modelView * projection;
    Matrix4 invModelViewProj = modelViewProj.InverseCramers();

    for (int i = 0; i < 4; i++) {
        edges[i].p0 = edges[i].p0 * invModelViewProj;
        edges[i].p1 = edges[i].p1 * invModelViewProj;
        edges[i].p0 = edges[i].p0 * (1.0 / edges[i].p0.w);
        edges[i].p1 = edges[i].p1 * (1.0 / edges[i].p1.w);
        edges[i].p0.w = edges[i].p1.w = 1.0;
    }

    // Intersect against the ocean plane
    Vector3 planeNormal = environment->GetUpVector();
    double planeDistance = environment->GetSeaLevel();

    if (environment->IsGeocentric()) {
        Vector3 camPos = environment->GetCameraPosition();
        Vector4 camPos4(camPos);

        Vector3 down = environment->GetUpVector() * -intersectionVectorLengthProjection;
        Vector4 down4(down);

        Vector4 intersectPoint;
        bool intersect = ProjectToEllipsoid(camPos4, down4, intersectPoint, 0);
        if (!intersect) {
            // Maybe we're underwater
            Vector3 up = environment->GetUpVector() * intersectionVectorLengthProjection;
            Vector4 up4(up);
            intersect = ProjectToEllipsoid(camPos4, up4, intersectPoint, 0);
            if (!intersect) {
                return false;
            }
        }
        planeDistance = Vector3(intersectPoint.x, intersectPoint.y, intersectPoint.z).Length();
    }

    TRITON_VECTOR(Vector3) intersections;
    for (int i = 0; i < 4; i++) {
        Vector3 p1 = Vector3(edges[i].p0.x, edges[i].p0.y, edges[i].p0.z);
        Vector3 p2 = Vector3(edges[i].p1.x, edges[i].p1.y, edges[i].p1.z);

        double numerator = planeDistance - p1.Dot(planeNormal);
        double denominator = (p2 - p1).Dot(planeNormal);
        if (denominator != 0.0) {
            double t = numerator / denominator;
            if (t >= 0.0 && t <= 1.0) {
                Vector3 intersection = p1 + (p2 - p1) * t;
                intersections.push_back(intersection);
            }
        }
    }

    // If there are two intersections, we have a horizon
    if (intersections.size() != 2) return false;

    // Transform back into normalized device coordinates
    Vector4 pt1 = Vector4(intersections[0].x, intersections[0].y, intersections[0].z, 1.0);
    Vector4 pt2 = Vector4(intersections[1].x, intersections[1].y, intersections[1].z, 1.0);
    pt1 = pt1 * modelViewProj;
    pt2 = pt2 * modelViewProj;
    pt1 = pt1 * (1.0 / pt1.w);
    pt2 = pt2 * (1.0 / pt2.w);

    // Compute the midpoint and the offset
    Vector3 midpoint = Vector3( (pt1.x + pt2.x) * 0.5, (pt1.y + pt2.y) * 0.5, 0.0 );

    // Compute the roll angle
    roll = atan2(pt2.y - pt1.y, pt2.x - pt1.x) * -1.0;

    // Nudge the offset up a bit to counter precision issues in the shader
    Vector3 midpointWorld = (intersections[0] + intersections[1]) * 0.5;
    Vector3 upABit = midpointWorld + environment->GetUpVector();
    Vector3 up = midpointWorld + upABit;
    Vector4 up4(up);
    up4 = up4 * modelViewProj;
    up4 = up4 * (1.0 / up4.w);
    Vector3 upNDC = Vector3(up4.x, up4.y, 0.0);
    Vector3 upDirNDC = upNDC - midpoint;
    upDirNDC.Normalize();

    offset = midpoint;
    offset = offset - upDirNDC * horizonOffset;

    return true;
}

Matrix4 ProjectedGrid::ComputeGridMatrix(double maxAmplitude)
{
    maxAmplitude *= gridClipFactor; // Just to be safe and account for choppiness

    Matrix4 identity;

    if (!environment) return identity;

    Configuration::GetDoubleValue("max-allowable-displacement", maxAllowableDisplacement);

    Vector4 corners[4];
    double maxDisplacement = 0;
    corners[0] = Vector4(-1, -1, 0, 1);
    corners[1] = Vector4( 1, -1, 0, 1);
    corners[2] = Vector4(-1,  1, 0, 1);
    corners[3] = Vector4( 1,  1, 0, 1);

    Matrix4 mv(environment->GetCameraMatrix());

    mv.elem[3][0] = mv.elem[3][1] = mv.elem[3][2] = 0.0;
    Vector3 camPos(environment->GetCameraPosition());
    Matrix4 proj(environment->GetProjectionMatrix());
    Matrix4 mvproj = mv * proj;
    Matrix4 invMvProj = mvproj.InverseCramers();

    bool foundIntersection = false;

    for (int i = 0; i < 4; i++) {
        Vector4 pNear = corners[i];
        pNear.z = nearZ;
        Vector4 pFar  = corners[i];
        pFar.z = farZ;

        pNear = pNear * invMvProj;
        pFar  = pFar * invMvProj;
        if (pNear.w == 0 || pFar.w == 0) continue;

        pNear = pNear * (1.0 / pNear.w);
        pFar = pFar * (1.0 / pFar.w);

        pNear = pNear + camPos;
        pFar = pFar + camPos;
        pNear.w = pFar.w = 1.0;

        Vector4 D = pFar - pNear;
        D.w = 1.0;

        Vector4 intersect1, intersect2;
        bool intersection1 = false, intersection2 = false;
        if (environment->IsGeocentric()) {
            intersection1 = ProjectToEllipsoid(pNear, D, intersect1, maxAmplitude);
            intersection2 = ProjectToEllipsoid(pNear, D, intersect2, -maxAmplitude);
        } else {
            intersection1 = ProjectToPlane(pNear, D, intersect1, maxAmplitude);
            intersection2 = ProjectToPlane(pNear, D, intersect2, -maxAmplitude);
        }

        if (intersection1 && !intersection2) {
            if (environment->IsGeocentric()) {
                intersection2 = ProjectToEllipsoid(pNear, D, intersect2, 0);
            } else {
                intersection2 = ProjectToPlane(pNear, D, intersect2, 0);
            }
        } else {
            if (intersection2 && !intersection1) {
                if (environment->IsGeocentric()) {
                    intersection1 = ProjectToEllipsoid(pNear, D, intersect1, 0);
                } else {
                    intersection2 = ProjectToPlane(pNear, D, intersect1, 0);
                }
            }
        }

        if (intersection1 && intersection2) {
            foundIntersection = true;

            intersect1 = intersect1 - camPos;
            intersect2 = intersect2 - camPos;

            intersect1 = intersect1 * mvproj;
            if (intersect1.w == 0) return identity;
            intersect1 = intersect1 * (1.0 / intersect1.w);
            intersect2 = intersect2 * mvproj;
            if (intersect2.w == 0) return identity;
            intersect2 = intersect2 * (1.0 / intersect2.w);

#define CLAMP(X) if (X < -1.0) X = -1.0; if (X > 1.0) X = 1.0;
            CLAMP(intersect1.x)
            CLAMP(intersect1.y)
            CLAMP(intersect2.x)
            CLAMP(intersect2.y)

            Vector3 diff = Vector3(intersect1.x, intersect1.y, 0) - Vector3(intersect2.x, intersect2.y, 0);
            double disp = diff.Length();
            if (disp > maxDisplacement) maxDisplacement = disp;
        }
    }

    if (!foundIntersection) maxDisplacement = maxAllowableDisplacement;

    if (maxDisplacement > maxAllowableDisplacement) maxDisplacement = maxAllowableDisplacement;

    Vector3 offset(0.0, 0.0, 0.0);
    double roll;
    if (!GetGridOffsetRoll(offset, roll)) {
        // rotate the grid to match the camera roll, to avoid artifacts from the grid tesselation not matching the ocean plane
        Vector3 right = Vector3(mv.elem[0][0], mv.elem[1][0], mv.elem[2][0]);
        Vector3 up = environment->GetUpVector();
        roll = asin(up.Dot(right));

        Vector3 camUp = Vector3(mv.elem[0][1], mv.elem[1][1], mv.elem[2][1]);
        if (camUp.Dot(up) < 0) roll *= -1.0;

        maxDisplacement = maxAllowableDisplacement;
    }

    Matrix4 rot;
    rot.elem[0][0] = cos(roll);
    rot.elem[0][1] = -sin(roll);
    rot.elem[1][0] = sin(roll);
    rot.elem[1][1] = cos(roll);

    // Expand the grid to account for the roll, and align it with the horizon line
    double maxOffset = 0.0;
    corners[0] = Vector4(1, 1, 0, 1);
    corners[1] = Vector4(-1, 1, 0, 1);
    corners[2] = Vector4(-1, -1, 0, 1);
    corners[3] = Vector4(1, -1, 0, 1);

    for (int i = 0; i < 4; i++) {
        Vector4 rotatedPoint = (corners[i] + Vector4(offset)) * rot;
        double dispX = fabs(rotatedPoint.x) - 1.0;
        double dispY = fabs(rotatedPoint.y) - 1.0;
        if (dispX > maxOffset) maxOffset = dispX;
        if (dispY > maxOffset) maxOffset = dispY;
    }

    float disp = (float)(1.0 + maxOffset * gridExpansionScale) + (float)maxDisplacement;

    Matrix4 scale;
    scale.elem[0][0] = disp;
    scale.elem[1][1] = disp;
    scale.elem[3][0] = offset.x;
    scale.elem[3][1] = offset.y;
    scale.elem[3][2] = offset.z;

    return rot * scale;
}

bool ProjectedGrid::ProjectToPlane(const Vector4& p0, const Vector4& D, Vector4& intersect, double zOffset) const
{
    if (!environment) return false;

    zOffset -= environment->GetSeaLevel();

    Vector4 plane;
    if (environment->GetCoordinateSystem() == FLAT_ZUP) {
        plane = Vector4(0, 0, 1, zOffset);
    } else {
        plane = Vector4(0, 1, 0, zOffset);
    }

    double t = -p0.Dot(plane) / D.Dot(plane);
    if (t > 0.0 && t < 1.0) {
        intersect = p0 + D * t;
        if (intersect.w == 0) return false;
        //intersect = intersect * (1.0 / intersect.w);
        intersect.w = 1.0;

        // Project it onto the base plane
        plane.w = -environment->GetSeaLevel();
        double dist = intersect.Dot(plane);
        intersect = intersect - plane * dist;
        intersect.w = 1.0;

        return true;
    } else {
        return false;
    }
}

bool ProjectedGrid::IntersectEllipsoidSegment(const Vector3& p0, const Vector3& D, Vector3& intersect, double zOffset) const
{
    Vector3 radii = earthRadii + (zOffset + environment->GetSeaLevel());

    if (radii.x == 0 || radii.y == 0 || radii.z == 0) return false;

    Vector3 oneOverRadii(1.0 / radii.x, 1.0 / radii.y, 1.0 / radii.z);

    Vector3 p0s = p0 * oneOverRadii;
    Vector3 Ds = D * oneOverRadii;

    double twop0v = 2.0 * p0s.Dot(Ds);
    double p02 = p0s.Dot(p0s);
    double v2 = Ds.Dot(Ds);

    double disc = twop0v * twop0v - (4.0 * v2)*(p02 - 1.0);
    if (disc > 0.0) {
        double discSqrt = sqrt(disc);
        double den = 2.0 * v2;
        double t = (-twop0v - discSqrt) / den;
        if (t < 0.0) {
            t = (-twop0v + discSqrt) / den;
        }
        intersect = p0s + Ds * t;
        intersect = intersect * radii;
        return (t <= 1.0 && t >= 0.0);
    } else {
        intersect = Vector3(0.0, 0.0, 0.0);
        return false;
    }
}

bool ProjectedGrid::IntersectEllipsoid(const Vector3& p0, const Vector3& D, Vector3& intersect, double zOffset) const
{
    Vector3 radii = earthRadii + (zOffset + environment->GetSeaLevel());

    if (radii.x == 0 || radii.y == 0 || radii.z == 0) return false;

    Vector3 oneOverRadii(1.0 / radii.x, 1.0 / radii.y, 1.0 / radii.z);
    Vector3 q = p0 * oneOverRadii;
    Vector3 bUnit = D * oneOverRadii;
    bUnit.Normalize();
    double wMagnitudeSquared = q.Dot(q) - 1.0;

    double t = -bUnit.Dot(q);
    double tSquared = t * t;

    if ((t >= 0.0) && (tSquared >= wMagnitudeSquared)) {
        double temp = t - sqrt(tSquared - wMagnitudeSquared);
        Vector3 r = (q + bUnit * temp);
        intersect = r * radii;
        return true;
    } else {
        return false;
    }
}

bool ProjectedGrid::ProjectToEllipsoid(const Vector4& p0, const Vector4& D, Vector4& intersect, double zOffset) const
{
    if (p0.w == 0 || D.w == 0) return false;

    Vector3 p03(p0.x / p0.w, p0.y / p0.w, p0.z / p0.w);
    Vector3 D3(D.x / D.w, D.y / D.w, D.z / D.w);

    if (zOffset == 0) {
        Vector3 intersection;
        if (IntersectEllipsoidSegment(p03, D3, intersection, 0)) {
            intersect = Vector4(intersection);
            return true;
        }
    } else {
        Vector3 baseIntersection, intersection;
        if (IntersectEllipsoidSegment(p03, D3, baseIntersection, 0)) {
            if (IntersectEllipsoidSegment(p03, D3, intersection, zOffset)) {
                // project to MSL
                Vector3 baseN = baseIntersection;
                baseN.Normalize();
                double baseD = -baseIntersection.Length();
                double distanceToPlane = intersection.Dot(baseN) + baseD;
                Vector3 projectedPt = intersection - baseN * distanceToPlane;
                intersect = Vector4(projectedPt);

                return true;
            }
        }
    }

    return false;
}

bool ProjectedGrid::GetFloorPlane(Vector3& floorPlanePoint, Vector3& floorNormal)
{
    floorNormal = environment->GetUpVector();
    const double *dCamPos = environment->GetCameraPosition();
    floorPlanePoint = Vector3(dCamPos) - environment->GetUpVector() * 1000.0;

    Vector3 surfacePoint;
    if (!GetIntersection(Vector3(dCamPos), environment->GetUpVector() * -1, surfacePoint)) {
        if (!GetIntersection(Vector3(dCamPos), environment->GetUpVector(), surfacePoint)) {
            return false;
        }
    }

    float depth = GetDepth(floorNormal);
    floorPlanePoint = surfacePoint + (environment->GetUpVector() * -depth);

    return true;
}

bool ProjectedGrid::SwitchToMesh()
{
    if (environment && useMesh) {
        double altitude = 0;
        Vector3 camPos(environment->GetCameraPosition());

        if (environment->IsGeocentric()) {

            Vector3 up = camPos;
            up.Normalize();

            Vector3 intersection;
            if (IntersectEllipsoid(camPos, up * -1.0, intersection, 0)) {
                altitude = (camPos - intersection).Length();
            }
        } else {
            Vector3 xPos = camPos * GetBasis();
            altitude = xPos.z;
        }

        Configuration::GetDoubleValue("mesh-switch-altitude", meshSwitchAltitude);
        meshSwitchAltitude /= environment->GetWorldUnits();

        return (altitude < (meshSwitchAltitude * environment->GetZoomLevel()));
    }

    return false;
}


static double inline Step(double x, double y)
{
    if (0 == y)
        return x;

    if (x < 0 && x > -1E-5) x = 0;

    return y * floor(x/y);
}

Matrix4 ProjectedGrid::ComputeMeshMatrix()
{
    // Translate to camera position / sea level, orient up
    Vector3 camPos(environment->GetCameraPosition());

    Vector3 origin;

    // Convert position to texture coords
    if (environment->IsGeocentric()) {

        Vector3 up = camPos;
        up.Normalize();
        Vector3 surfacePos;
        IntersectEllipsoid(camPos, up * -1.0, surfacePos, 0);
        origin = surfacePos - camPos;

        Vector3 north = GetNorth();
        up = environment->GetUpVector();
        Vector3 east = north.Cross(up);
        const double epsilon = 1E-10;
        if (east.SquaredLength() < epsilon) {
            // North or south pole, pick an arbitrary North direction.
            Vector3 fakeNorth(1, 0, 0);
            east = fakeNorth.Cross(up);
        }
        east.Normalize();
        north = up.Cross(east);
        north.Normalize();

#define BLOCKSIZE 100000.0
#define OFFSET     50000.0
        Vector3 refPt = camPos + Vector3(OFFSET, OFFSET, OFFSET);
        Vector3 localReference = Vector3(fmod(refPt.x, BLOCKSIZE), fmod(refPt.y, BLOCKSIZE), fmod(refPt.z, BLOCKSIZE));
        double camOffsetX = 0, camOffsetY = 0;

        camOffsetX = localReference.Dot(east);
        camOffsetY = localReference.Dot(north);

        double x = camPos.Dot(east) + camOffsetX;
        double y = camPos.Dot(north) + camOffsetY;
        double fx = fmod(x, (double)Lx);
        double fy = fmod(y, (double)Ly);

        Vector3 offsetZup(fx, fy, 0);
        Vector3 offset = offsetZup * GetInverseBasis();

        origin = origin - offset;

    } else {
        // Transform to z-is-up
        Vector3 xPos = camPos * GetBasis();
        double s = Step(xPos.x, (double)Lx);
        double t = Step(xPos.y, (double)Ly);
        Vector3 zUpPos;
        zUpPos.x = s;
        zUpPos.y = t;
        zUpPos.z = environment->GetSeaLevel();
        origin = zUpPos * GetInverseBasis();
    }

    Matrix4 translate;
    translate.elem[3][0] = origin.x;
    translate.elem[3][1] = origin.y;
    translate.elem[3][2] = origin.z;

    Matrix4 basis4;
    Matrix3 basis = GetBasis();
    for (int row = 0; row < 3; row++) {
        for (int col = 0; col < 3; col++) {
            basis4.elem[col][row] = basis.elem[row][col];
        }
    }

    return basis4 * translate;
}
