// Copyright (c) 2011-2014 Sundog Software, LLC. All rights reserved worldwide.

#include <FeTriton/Environment.h>
#include <FeTriton/LicenseManager.h>
#include <FeTriton/Configuration.h>
#include <FeTriton/DefaultRandomNumberGenerator.h>
#include <FeTriton/Matrix4.h>
#include <FeTriton/TritonOpenGL.h>
#include <FeTriton/Frustum.h>
#include <FeTriton/Ocean.h>
#include <algorithm>
#include <math.h>
#include <FeUtils/logger/LoggerDef.h>

using namespace Triton;

class Triton::EnvironmentImp
{
public:
	EnvironmentImp()
	{
	}
	~EnvironmentImp()
	{
		windFetches.clear();
		oceans.clear();
		swells.clear();
	}
	TRITON_VECTOR(WindFetch) windFetches;

	TRITON_VECTOR(Ocean *) oceans;

	TRITON_VECTOR(SwellDescription) swells;
};

BreakingWavesParameters::BreakingWavesParameters()
{
    steepness = 0.01f;
    steepnessVariance = 0.0001f;
    wavelength = 2000.0f;
    wavelengthVariance = 1000.0f;
    amplitude = 3.0f;
    surgeDepth = 8.0f;
    waveDirection = Vector3(1.0, 0.0, 0.0);
    autoWaveDirection = false;
    depthFalloff = 5.0f;

    ComputeDerivedValues();
}

void BreakingWavesParameters::ComputeDerivedValues()
{
    float g = 9.81f;
    Configuration::GetFloatValue("wave-gravity-force", g);
    if (wavelength > 0 && amplitude > 0) {
        float k = (1.0f / (wavelength * amplitude)) * steepness;
        kexp = expf(k);
        speed = sqrtf((g * wavelength) / (float)TRITON_TWOPI);
        phaseConstant = ((speed * (float)TRITON_TWOPI) / wavelength);
    } else {
        kexp = 1.0f;
        speed = 0.0f;
        phaseConstant = 0.0f;
    }
}

Environment::Environment() : units(1.0), envMap(0), heightMap(0), device(NULL), seaLevel(0), aboveWaterVisibility(1E9),
    belowWaterVisibility(1E5), planarReflectionMap(0), frustum(0), hdr(false), useOpenMP(false), confusedSeas(false),
    doHeightMapCopy(true), breakingWaveMap(0), zoomLevel(1.0f), sunIntensity(1.0f),
    userDefinedVertString(0), userDefinedFragString(0), depthMap(0), hasBlendDepth(false), blendDepth(100.0),
    userHeightCB(0), heightMapRange(1.0f), heightMapOffset(0.0f),environmentImp(NULL),camHeight(0.0),sunDistance(0.0),m_opExtension(NULL)
{
    defaultRandomNumberGenerator = TRITON_NEW DefaultRandomNumberGenerator();
    randomNumberGenerator = defaultRandomNumberGenerator;
    envMapMatrix = Matrix3::Identity;
    heightMapMatrix = Matrix4::Identity;
    breakingWaveMapMatrix = Matrix4::Identity;
    planarReflectionMapMatrix = Matrix3::Identity;

    for (int i = 0; i < 4; i++) viewport[i] = 0;

	environmentImp = TRITON_NEW EnvironmentImp();
}

void Environment::SetViewport(int x, int y, int w, int h)
{
    viewport[0] = x;
    viewport[1] = y;
    viewport[2] = w;
    viewport[3] = h;
}

bool Environment::GetViewport(int& x, int& y, int& w, int& h) const
{
    x = viewport[0];
    y = viewport[1];
    w = viewport[2];
    h = viewport[3];

    return (w > 0 && h > 0);
}

void Environment::RegisterOcean(Ocean *ocean)
{
	if(environmentImp != NULL)
       environmentImp->oceans.push_back(ocean);
}

void Environment::UnregisterOcean(Ocean *ocean)
{
	if(environmentImp != NULL)
        environmentImp->oceans.erase(std::remove(environmentImp->oceans.begin(), environmentImp->oceans.end(), ocean), environmentImp->oceans.end());
}

void Environment::SetRandomNumberGenerator(RandomNumberGenerator *rng)
{
    if (!rng) return;

    randomNumberGenerator = rng;
}

void Environment::SetHeightMap(TextureHandle pHeightMap, const Matrix4& worldToTextureCoords)
{
	if(environmentImp != NULL)
	{
		heightMap = pHeightMap;
		heightMapMatrix = worldToTextureCoords;

		if (doHeightMapCopy) {
			TRITON_VECTOR(Ocean *)::iterator it;
			for (it = environmentImp->oceans.begin(); it != environmentImp->oceans.end(); it++) {
				(*it)->UpdateHeightMap();
			}
		}
	}
}

void Environment::SetDepthMap(TextureHandle pDepthMap)
{
    depthMap = pDepthMap;
}

void Environment::SetBreakingWaveMap(TextureHandle pBreakingWaveMap, const Matrix4& worldToTextureCoords)
{
    breakingWaveMap = pBreakingWaveMap;
    breakingWaveMapMatrix = worldToTextureCoords;
}

EnvironmentError Environment::Initialize(CoordinateSystem cs, Renderer ren, ResourceLoader *rl, void *pDevice, bool pHdr)
{
    coordinateSystem = cs;
    renderer = ren;
    resourceLoader = rl;
    device = pDevice;
    hdr = pHdr;

    frustum = TRITON_NEW Frustum();

    if (rl) {
        if (!Configuration::Initialize(rl)) {
            Utils::DebugMsg("Triton.config failed to load - did you set the correct path to the resources folder?");
            return NO_CONFIG_FOUND;
        }
    } else {
        Utils::DebugMsg("Null ResourceLoader passed into the Environment constructor.");
        return NULL_RESOURCE_LOADER;
    }

    renderer = ren;
    if (IsOpenGL()) {
        OpenGLExtensionManager::LoadGLExtensions();
    } else if (IsDirectX()) {
        if (!device) {
            Utils::DebugMsg("No device was passed into the Environment constructor, and you're using a DirectX renderer.");
            return NO_DEVICE;
        }
    }

    float fogAboveR, fogAboveG, fogAboveB, visibilityAbove;
    float fogBelowR, fogBelowG, fogBelowB, visibilityBelow;

    Configuration::GetFloatValue("default-fog-color-above-r", fogAboveR);
    Configuration::GetFloatValue("default-fog-color-above-g", fogAboveG);
    Configuration::GetFloatValue("default-fog-color-above-b", fogAboveB);
    Configuration::GetFloatValue("default-fog-color-below-r", fogBelowR);
    Configuration::GetFloatValue("default-fog-color-below-g", fogBelowG);
    Configuration::GetFloatValue("default-fog-color-below-b", fogBelowB);
    Configuration::GetFloatValue("default-visibility-above", visibilityAbove);
    Configuration::GetFloatValue("default-visibility-below", visibilityBelow);

    aboveWaterFogColor = Vector3(fogAboveR, fogAboveG, fogAboveB);
    aboveWaterVisibility = visibilityAbove / GetWorldUnits();
    belowWaterFogColor = Vector3(fogBelowR, fogBelowG, fogBelowB);
    belowWaterVisibility = visibilityBelow / GetWorldUnits();

    Configuration::GetBoolValue("do-height-map-copy", doHeightMapCopy);

    douglasShortWL = 50.0f;
    douglasAvgWL = 100.0f;
    douglasLongWL = 200.0f;
    douglasLowHeight = 1.0f;
    douglasModerateHeight = 3.0f;
    douglasHighHeight = 4.5f;
    Configuration::GetFloatValue("douglas-short-wavelength", douglasShortWL);
    Configuration::GetFloatValue("douglas-average-wavelength", douglasAvgWL);
    Configuration::GetFloatValue("douglas-long-wavelength", douglasLongWL);
    Configuration::GetFloatValue("douglas-low-wave-height", douglasLowHeight);
    Configuration::GetFloatValue("douglas-moderate-wave-height", douglasModerateHeight);
    Configuration::GetFloatValue("douglas-high-wave-height", douglasHighHeight);
    douglasShortWL /= (float)GetWorldUnits();
    douglasAvgWL /= (float)GetWorldUnits();
    douglasLongWL /= (float)GetWorldUnits();
    douglasLowHeight /= (float)GetWorldUnits();
    douglasModerateHeight /= (float)GetWorldUnits();
    douglasHighHeight /= (float)GetWorldUnits();

	double eqRad,polRad;
	Configuration::GetDoubleValue("equatorial-earth-radius-meters", eqRad);
	Configuration::GetDoubleValue("polar-earth-radius-meters", polRad);
    double innerRadius = std::min<double>(eqRad, polRad);;
	sunDistance = innerRadius * 12000.0f;
	
    return SUCCEEDED;
}

Environment::~Environment()
{
    //windFetches.clear();
    Configuration::Destroy();
    LicenseManager::Shutdown();

	if(environmentImp != NULL)
	{
		TRITON_VECTOR(Ocean *)::iterator it;
		for (it = environmentImp->oceans.begin(); it != environmentImp->oceans.end(); it++) {
			Ocean *ocean = *it;
			ocean->EnvironmentDeleted();
		}
	}
    
    if (frustum) {
        TRITON_DELETE frustum;
    }

    if (defaultRandomNumberGenerator) {
        TRITON_DELETE defaultRandomNumberGenerator;
    }

    if (userDefinedVertString) {
        TRITON_FREE(userDefinedVertString);
    }

    if (userDefinedFragString) {
        TRITON_FREE(userDefinedFragString);
    }

	if(environmentImp != NULL)
	{
		TRITON_DELETE environmentImp;
		environmentImp = NULL;
	}
}

void Environment::SetLicenseCode(const char *userName, const char *registrationCode)
{
    LicenseManager::ValidateCode(userName, registrationCode);
}

void Environment::SimulateSeaState(double B, double dir, bool leftHanded)
{
    if (dir > TRITON_TWOPI || dir < -TRITON_TWOPI) {
        Utils::DebugMsg("You passed in a wind direction greater than 2PI to Environment::SimulateSeaState. "
                        "Technically this is OK, but are you sure you're using radians instead of degrees here?");
    }

    double speed = 0.836 * pow(B, 1.5);

    // Even sea state 0 has some movement.
    if (speed < 0.1) speed = 0.1;

    ClearWindFetches();
    WindFetch f;
    f.SetWind(speed / units, dir);
    AddWindFetch(f, leftHanded);
}

void Environment::AddWindFetch(const WindFetch& fetch, bool leftHanded)
{
	if(environmentImp != NULL)
	{
		WindFetch fetchCopy = fetch;
		if (leftHanded) {
			fetchCopy.AdjustForLeftHanded();
		}
		environmentImp->windFetches.push_back(fetchCopy);
	}
}

void Environment::ClearWindFetches()
{   
	if(environmentImp != NULL)
    {
        environmentImp->windFetches.clear();
    }
}

float Environment::GetMaximumWaveHeight() const
{
    double windSpeed, windDirection, fetchLength;
    GetWind(Vector3(camPos), windSpeed, windDirection, fetchLength);

    float windWaveHeight = 0;

    // From the Beaufort scale chart:

    if (windSpeed < 0.2) {
        windWaveHeight = 0.0f;
    } else if (windSpeed < 1.5) {
        windWaveHeight = 0.1f;
    } else if (windSpeed < 3.3) {
        windWaveHeight = 0.3f;
    } else if (windSpeed < 5.4) {
        windWaveHeight = 1.0f;
    } else if (windSpeed < 7.9) {
        windWaveHeight = 1.5f;
    } else if (windSpeed < 10.7) {
        windWaveHeight = 2.5f;
    } else if (windSpeed < 13.8) {
        windWaveHeight = 4.0f;
    } else if (windSpeed < 17.1) {
        windWaveHeight = 5.5f;
    } else if (windSpeed < 20.7) {
        windWaveHeight = 7.5f;
    } else {
        windWaveHeight = 10.0f;
    }

    TRITON_VECTOR(SwellDescription)::const_iterator it;
    float maxSwellHeight = 0;

	if(environmentImp != NULL)
	{
		for (it = environmentImp->swells.begin(); it != environmentImp->swells.end(); it++) {
			if ((*it).height > maxSwellHeight) {
				maxSwellHeight = (*it).height;
			}
		}
	} 

    if (maxSwellHeight > windWaveHeight) {
        return maxSwellHeight;
    } else {
        return windWaveHeight;
    }
}

void Environment::SetUserDefinedVertString( const char *userString )
{
    if (userDefinedVertString) TRITON_FREE(userDefinedVertString);

    userDefinedVertString = (char *)TRITON_MALLOC(strlen(userString) + 1);

#ifdef _WIN32
#if _MSC_VER < 1400
    strncpy(userDefinedVertString, userString, strlen(userString) + 1);
#else
    strcpy_s(userDefinedVertString, strlen(userString) + 1, userString);
#endif
#else
    strncpy(userDefinedVertString, userString, strlen(userString) + 1);
#endif
}

void Environment::SetUserDefinedFragString( const char *userString )
{
    if (userDefinedFragString) TRITON_FREE(userDefinedFragString);

    userDefinedFragString = (char *)TRITON_MALLOC(strlen(userString) + 1);

#ifdef _WIN32
#if _MSC_VER < 1400
    strncpy(userDefinedFragString, userString, strlen(userString) + 1);
#else
    strcpy_s(userDefinedFragString, strlen(userString) + 1, userString);
#endif
#else
    strncpy(userDefinedFragString, userString, strlen(userString) + 1);
#endif
}

void Environment::GetWind(const Vector3& pos, double& windSpeed, double& windDirection, double& fetchLength) const
{
    double speedX = 0, speedY = 0;
    double maxFetchLength = 0;

    TRITON_VECTOR(WindFetch)::const_iterator it;

	if(environmentImp != NULL)
	{
		for (it = environmentImp->windFetches.begin(); it != environmentImp->windFetches.end(); it++) {
			double thisSpeed, thisDirection, thisFetchLength;

			(*it).GetWindAtLocation(pos, thisSpeed, thisDirection, thisFetchLength);

			if (thisSpeed > 0) {
				speedX += thisSpeed * sin(thisDirection);
				speedY += thisSpeed * cos(thisDirection);
			}

			if (thisFetchLength > maxFetchLength) {
				maxFetchLength = thisFetchLength;
			}
		}
	}

    windSpeed = sqrt(speedX *speedX + speedY * speedY);

    if (windSpeed > 0) {
        // atan2 gives anticlockwise direction from X axis
        // We want clockwise from Y axis
        windDirection = atan2(speedY, speedX);
        windDirection -= TRITON_PI * 0.5;
        windDirection = TRITON_TWOPI - windDirection;

        if (windDirection > TRITON_PI) windDirection -= TRITON_TWOPI;
        if (windDirection < -TRITON_PI) windDirection += TRITON_TWOPI;
    } else {
        windDirection = 0;
    }

    fetchLength = maxFetchLength;
}

float Environment::adjustDirectionForLeftHanded(float windDirRadians)
{
    float theta = (0.5f * TRITON_PIF) - windDirRadians;
    float x = cosf(theta);
    float y = -sinf(theta);
    return (0.5f * TRITON_PIF) - atan2f(y, x);
}

void Environment::AddSwell(float waveLength, float waveHeight, float direction, float phase, bool leftHanded)
{
    if (direction > TRITON_TWOPI || direction < -TRITON_TWOPI) {
        Utils::DebugMsg("You passed in a wind direction greater than 2PI to Environment::AddSwell. "
                        "Technically this is OK, but are you sure you're using radians instead of degrees here?");
    }

    if (phase > TRITON_TWOPI || phase < -TRITON_TWOPI) {
        Utils::DebugMsg("You passed in a phase offset greater than 2PI to Environment::AddSwell. "
                        "Technically this is OK, but are you sure you're using radians instead of degrees here?");
    }

    if (waveHeight < 0) {
        Utils::DebugMsg("You passed in a negative wave height to Environment::AddSwell. Discarding.");
        return;
    }

    if (waveLength < 0) {
        Utils::DebugMsg("Swell ignored; negative wavelength passed in.");
        return;
    }

    if (leftHanded) {
        direction = adjustDirectionForLeftHanded(direction);
    }

    SwellDescription swell;
    swell.wavelength = waveLength;
    swell.height = waveHeight;
    swell.direction = direction;
    swell.phase = phase;

	if(environmentImp != NULL)
	{
		environmentImp->swells.push_back(swell);

		confusedSeas = false;

		TRITON_VECTOR(Ocean *)::iterator it;
		for (it = environmentImp->oceans.begin(); it != environmentImp->oceans.end(); it++) {
			(*it)->NotifySwellsChanged();
			(*it)->SetConfused(false);
		}
	}
}

void Environment::ClearSwells()
{
	if(environmentImp != NULL)
	{
		environmentImp->swells.clear();
		confusedSeas = false;

		TRITON_VECTOR(Ocean *)::iterator it;
		for (it = environmentImp->oceans.begin(); it != environmentImp->oceans.end(); it++) {
			(*it)->NotifySwellsChanged();
			(*it)->SetConfused(false);
		}
	}
}

const TRITON_VECTOR(SwellDescription)& Environment::GetSwells() const
{
	if(environmentImp != NULL)
	{
		return environmentImp->swells;
	}
	else
	{
		static TRITON_VECTOR(SwellDescription) empty;
		return empty;
	}       
}

void Environment::SetDouglasSeaScale(int seaState, float windWaveDirection, int swellState, float swellDirection, bool leftHanded)
{
    if (windWaveDirection > TRITON_TWOPI || windWaveDirection < -TRITON_TWOPI ||
            swellDirection > TRITON_TWOPI || swellDirection < -TRITON_TWOPI) {
        Utils::DebugMsg("You passed in a wind direction greater than 2PI to Environment::SetDouglasSeaScale. "
                        "Technically this is OK, but are you sure you're using radians instead of degrees here?");
    }

    if (seaState < 0 || seaState > 9) {
        Utils::DebugMsg("Invalid sea state; valid values are 0-9.");
        return;
    }

    if (swellState < 0 || swellState > 9) {
        Utils::DebugMsg("Invalid swell state; valid values are 0-9.");
        return;
    }

    ClearWindFetches();
    ClearSwells();

    SimulateSeaState((double)seaState, (double)windWaveDirection, leftHanded);

    if (swellState == 0) return; // No swell

    float wavelength = 0, height = 0;
    confusedSeas = false;

    switch (swellState) {
    case 1: // very low
        wavelength = douglasShortWL;
        height = douglasLowHeight;
        break;

    case 2: // low
        wavelength = douglasLongWL;
        height = douglasLowHeight;
        break;

    case 3: // light
        wavelength = douglasShortWL;
        height = douglasModerateHeight;
        break;

    case 4: // moderate
        wavelength = douglasAvgWL;
        height = douglasModerateHeight;
        break;

    case 5: // moderate rough
        wavelength = douglasLongWL;
        height = douglasModerateHeight;
        break;

    case 6: // rough
        wavelength = douglasShortWL;
        height = douglasHighHeight;
        break;

    case 7: // high
        wavelength = douglasAvgWL;
        height = douglasHighHeight;
        break;

    case 8: // very high
        wavelength = douglasLongWL;
        height = douglasHighHeight;
        break;

    case 9: // confused
        confusedSeas = true;
        break;
    }

    if (height > 0) {
        AddSwell(wavelength, height, swellDirection);
    }

	if(environmentImp != NULL)
	{
		TRITON_VECTOR(Ocean *)::iterator it;
		for (it = environmentImp->oceans.begin(); it != environmentImp->oceans.end(); it++) {
			(*it)->SetConfused(confusedSeas);
		}
	}
    
}


void Environment::SetDirectionalLight(const Vector3& d, const Vector3& c)
{
    if (c.x > 1.0 || c.y > 1.0 || c.z > 1.0 || c.x < 0 || c.y < 0 || c.z < 0) {
        Utils::DebugMsg("You passed in a color to Environment::SetDirectionalLight "
                        "with a component outside the range of 0-1.");
    }

    lightDirection = d;
    lightColor = c;

    lightDirection.Normalize();
}

void Environment::SetCameraMatrix(const double *m)
{
    if (!m) {
        Utils::DebugMsg("Null matrix received in Environment::SetCameraMatrix.");
        return;
    }

    if (memcmp(m, modelview, 16 * sizeof(double)) == 0) {
        return;
    }

    for (int i = 0; i < 16; i++) {
        modelview[i] = m[i];
    }

    Matrix4 mv(m);
    Matrix4 invMV = mv.InverseCramers();
    camPos[0] = invMV.elem[3][0];
    camPos[1] = invMV.elem[3][1];
    camPos[2] = invMV.elem[3][2];

    ExtractFrustum();
}

void Environment::SetProjectionMatrix(const double *p)
{
    if (!p) {
        Utils::DebugMsg("Null matrix received in Environment::SetProjectionMatrix.");
        return;
    }

    if (memcmp(p, projection, 16 * sizeof(double)) == 0) {
        return;
    }

    for (int i = 0; i < 16; i++) {
        projection[i] = p[i];
    }

    ExtractFrustum();
}

void Environment::ExtractFrustum()
{
    if (!frustum) return;

    Matrix4 m;
    Matrix4 mv(modelview);
    Matrix4 proj(projection);

    m = mv * proj;
    m.Transpose();

    Vector3 m0 = m.GetRow(0);
    Vector3 m1 = m.GetRow(1);
    Vector3 m2 = m.GetRow(2);
    Vector3 m3 = m.GetRow(3);

    Vector3 n;
    double d;

    n = m3 + m0;
    d = m(3, 3) + m(0, 3);
    Plane left(n, d);

    n = m3 - m0;
    d = m(3, 3) - m(0, 3);
    Plane right(n, d);

    n = m3 + m1;
    d = m(3, 3) + m(1, 3);
    Plane bottom(n, d);

    n = m3 - m1;
    d = m(3, 3) - m(1, 3);
    Plane top(n, d);

    n = m3 + m2;
    d = m(3, 3) + m(2, 3);
    Plane pnear(n, d);

    n = m3 - m2;
    d = m(3, 3) - m(2, 3);
    Plane pfar(n, d);

    pnear.Normalize();
    pfar.Normalize();
    left.Normalize();
    right.Normalize();
    top.Normalize();
    bottom.Normalize();

    frustum->SetPlane(Frustum::PNEAR, pnear);
    frustum->SetPlane(Frustum::PBACK, pfar);
    frustum->SetPlane(Frustum::PLEFT, left);
    frustum->SetPlane(Frustum::PRIGHT, right);
    frustum->SetPlane(Frustum::PTOP, top);
    frustum->SetPlane(Frustum::PBOTTOM, bottom);
}

bool Environment::CullSphere(const Vector3& position, double radius) const
{
    if (!frustum) return false;

    int nPlanes = frustum->GetNumCullingPlanes();

    for (int i = 0; i < nPlanes; i++) {
        const Plane& p = frustum->GetPlane(i);
        double origin = (p.GetNormal()).Dot(position) + p.GetDistance();

        if (origin < -radius) {
            return true;
        }

        if (fabs(origin) < radius) {
            return false;
        }
    }

    return false;
}

bool Environment::CullOrientedBoundingBox(const OrientedBoundingBox& obb) const
{
    if (!frustum) return false;

    for (int i = 0; i < frustum->GetNumCullingPlanes(); i++) {

        const Plane& p = frustum->GetPlane(i);

        Vector3 R = obb.axes[0] * obb.halfDistances[0];
        Vector3 S = obb.axes[1] * obb.halfDistances[1];
        Vector3 T = obb.axes[2] * obb.halfDistances[2];

        double rDotN = R.Dot(p.GetNormal());
        double sDotN = S.Dot(p.GetNormal());
        double tDotN = T.Dot(p.GetNormal());
        double rEff = (fabs(rDotN) + fabs(sDotN) + fabs(tDotN));

        Vector4 L(p.GetNormal().x, p.GetNormal().y, p.GetNormal().z, p.GetDistance());

        double lDotQ = L.Dot(Vector4(obb.center));

        if (lDotQ <= -rEff) {
            return true;
        }
    }

    return false;
}

Vector3 Environment::GetUpVector() const
{
    switch (coordinateSystem) {
    case FLAT_ZUP:
        return Vector3(0, 0, 1);

    case FLAT_YUP:
        return Vector3(0, 1, 0);

    default:
        Vector3 cam(camPos[0], camPos[1], camPos[2]);
        cam.Normalize();
        return cam;
    }
}

Vector3 Environment::GetRightVector() const
{
    switch (coordinateSystem) {
    case SPHERICAL_ZUP:
    case WGS84_ZUP: {
        Vector3 up = GetUpVector();
        Vector3 north = Vector3(0, 0, 1);
        Vector3 east = north.Cross(up);
        east.Normalize();
        return east;
    }
    break;

    case SPHERICAL_YUP:
    case WGS84_YUP: {
        Vector3 up = GetUpVector();
        Vector3 north = Vector3(0, 1, 0);
        Vector3 east = north.Cross(up);
        east.Normalize();
        return east;
    }
    break;

    default:
        return Vector3(1, 0, 0);
        break;
    }
}

void Environment::SetConfigOption(const char *key, const char *value)
{
    Configuration::Set(key, value);
}

const char *Environment::GetConfigOption(const char *key)
{
    const char *value;
    if (Configuration::GetStringValue(key, value)) {
        return value;
    } else {
        return NULL;
    }
}


TRITON_VECTOR(Ocean *) Triton::Environment::getOceans() const
{
	if(environmentImp != NULL)
		return environmentImp->oceans;
	else
	{
		static TRITON_VECTOR(Ocean *) empty;
		return empty;
	}
}
