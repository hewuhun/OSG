// Copyright (c) 2011-2015 Sundog Software LLC. All rights reserved worldwide.

#include <FeTriton/WakeManager.h>
#include <FeTriton/Configuration.h>
#include <FeTriton/ProjectedGrid.h>
#include <algorithm>

using namespace Triton;

double CircularWave::minAmplitude;
double CircularWave::maxAmplitude;
double CircularWave::wakeScale;
//double CircularWave::wakeFadeInTime;

double KelvinWake::minAmplitude;
double KelvinWake::maxAmplitude;
double KelvinWake::wakeScale;
double KelvinWake::gravity;
double KelvinWake::wakeFadeInTime;

float LeewardDampener::maxLeewardDampeningVelocity = 0.0f;
float LeewardDampener::scaledMaxLeewardDampeningVelocity = 0.0f;

double PropWashSegment::washScale;
double PropWashSegment::propWaveHeightFade;

#if defined(WIN32) || defined(WIN64)
inline double fastPow(double a, double b)
{
    union {
        double d;
        int x[2];
    } u = { a };
    u.x[1] = (int)(b * (u.x[1] - 1072632447) + 1072632447);
    u.x[0] = 0;

    return u.d;
}
#else
#define fastPow(a, b) pow(a, b)
#endif

WaterDisturbance::WaterDisturbance() : generator(), startDistanceTravelled(0)
{

}

double WaterDisturbance::GetDistanceFromGenerator() const
{
    if (generator) {
        double dist = (generator->GetDistanceTravelled() - startDistanceTravelled);
        if (dist < 0) dist = 0;
        return dist;
    } else {
        return 0;
    }
}

double WaterDisturbance::GetDistanceSquared(const Vector3& from) const
{
    return (position - from).SquaredLength();
}

LeewardDampener::LeewardDampener()
{
    classID = LEEWARD_DAMPENER;

    maxLeewardDampeningVelocity = 10.0f;
    Configuration::GetFloatValue("max-leeward-dampening-velocity", maxLeewardDampeningVelocity);
}

bool LeewardDampener::Update(double time, double worldUnits)
{
    scaledMaxLeewardDampeningVelocity = maxLeewardDampeningVelocity / (float)worldUnits;
    return true;
}

CircularWave::CircularWave()
{
    classID = CIRCULAR_WAVE;

    minAmplitude = 0.01;
    maxAmplitude = 1.5;

    Configuration::GetDoubleValue("wake-circular-wave-min-amplitude", minAmplitude);
    Configuration::GetDoubleValue("wake-circular-wave-max-amplitude", maxAmplitude);

    wakeScale = 1.0;
    Configuration::GetDoubleValue("wake-circular-wave-scale", wakeScale);

    //wakeFadeInTime = 0.5;
    //Configuration::GetDoubleValue("wake-fade-in-time", wakeFadeInTime);
}

bool CircularWave::Update(double time, double worldUnits)
{
    double elapsedTime = time - startTime;

    radius = elapsedTime * velocity + beamWidth;
    double unfadedAmplitude = (1.0 / fastPow(radius / worldUnits, waveDecay)) * (wavelength / 7.0) * wakeScale;

    if (unfadedAmplitude > maxAmplitude / worldUnits) unfadedAmplitude = maxAmplitude / worldUnits;

	/*if (elapsedTime > wakeFadeInTime) {
	amplitude = unfadedAmplitude;
	} else {
	amplitude = unfadedAmplitude * (elapsedTime / wakeFadeInTime);
	}*/

	if (elapsedTime > wakeFadeInTime) {
		unfadedAmplitude = 0;
	} else {
		unfadedAmplitude *= (1 - (elapsedTime / wakeFadeInTime));
	}

	amplitude = unfadedAmplitude;

    return (unfadedAmplitude > (minAmplitude * wakeScale) / worldUnits);
}

PropWashSegment::PropWashSegment(double maxLength, double pFadeTime) : wakeLength(maxLength), washWidth(0), fadeTime(pFadeTime)
{
    classID = PROP_WASH;

    washScale = 1.0;
    Configuration::GetDoubleValue("prop-wash-scale", washScale);

    propWaveHeightFade = 100.0;
    Configuration::GetDoubleValue("prop-wave-height-fade", propWaveHeightFade);
}

double PropWashSegment::GetDistanceFromGenerator() const
{
    if (generator) {
        double dist = (generator->GetDistanceTravelled() - startDistanceTravelled);// + deltaPos.Length() * 0.5;
        if (dist < 0) dist = 0;
        return dist;
    } else {
        return 0;
    }
}

bool PropWashSegment::Update(double time, double worldUnits)
{
    double distFromSource = GetDistanceFromGenerator();

    if (generator) {

        // From Zilman et al: Speed and Beam of a Ship from its Wake's SAR Images
        const double w0 = 4.0;
        const double x0 = 4.0;

        double x = distFromSource;
        if (x == 0) return true;

        const double a = 4.5;

        washWidth = (w0 / fastPow( (x0 * shipLength) / beamWidth, 1.0 / a)) * fastPow(beamWidth, (a - 1.0) / a);// * pow(x, 1.0 / a);
        washWidth *= washScale;

        double segmentLength = deltaPos.Length();
        double distanceAlphaStart = 1.0 - ((x + segmentLength) / wakeLength);
        if (distanceAlphaStart < 0) distanceAlphaStart = 0;
        if (distanceAlphaStart > 1.0) distanceAlphaStart = 1.0;

        double distanceAlphaEnd = 1.0 - (x / wakeLength);
        if (distanceAlphaEnd < 0) distanceAlphaEnd = 0;
        if (distanceAlphaEnd > 1.0) distanceAlphaEnd = 1.0;

        double timeAlphaStart = 1.0 - ((time - startTime) / fadeTime);
        if (timeAlphaStart < 0) timeAlphaStart = 0;
        if (timeAlphaStart > 1.0) timeAlphaStart = 1.0;

        double timeAlphaEnd = 1.0 - ((time - lastTime) / fadeTime);
        if (timeAlphaEnd < 0) timeAlphaEnd = 0;
        if (timeAlphaEnd > 1.0) timeAlphaEnd = 1.0;

        double roughnessAlpha = 1.0;
        const Ocean *ocean = generator->GetOcean();
        if (ocean) {
            const Environment *env = ocean->GetEnvironment();
            if (env) {
                float waveHeight = env->GetMaximumWaveHeight();
                roughnessAlpha = 1.0 - (waveHeight / (propWaveHeightFade / worldUnits));
                if (roughnessAlpha < 0) roughnessAlpha = 0;
                if (roughnessAlpha > 1.0) roughnessAlpha = 1.0;
            }
        }

        alphaStart = distanceAlphaStart * timeAlphaStart * roughnessAlpha;
        alphaEnd = distanceAlphaEnd * timeAlphaEnd * roughnessAlpha;

    } else {
        washWidth = 0;
        alphaStart = alphaEnd = 1.0;
    }

    return ((distFromSource < wakeLength) && (alphaStart > 0 || alphaEnd > 0));
}

KelvinWake::KelvinWake() : isExplicit(false)
{
    classID = KELVIN_WAKE;

    minAmplitude = 0.05;
    maxAmplitude = 1.5;

    foamAmount = 1.0f;
    Configuration::GetFloatValue("wake-kelvin-wave-foam-scale", foamAmount);

    Configuration::GetDoubleValue("wake-kelvin-wave-min-amplitude", minAmplitude);
    Configuration::GetDoubleValue("wake-kelvin-wave-max-amplitude", maxAmplitude);

    wakeScale = 1.0;
    Configuration::GetDoubleValue("wake-kelvin-wave-scale", wakeScale);

    gravity = 9.81;
    Configuration::GetDoubleValue("wave-gravity-force", gravity);

    wakeFadeInTime = 0.5;
    Configuration::GetDoubleValue("wake-fade-in-time", wakeFadeInTime);
}

bool KelvinWake::Update(double time, double worldUnits)
{
    if (isExplicit) return true;

    double elapsedTime = time - startTime;

    double distFromSource = GetDistanceFromGenerator();

    if (distFromSource == 0) return true;

    if (generator) {
        const double velFactor = 2.8286;
        velocity = generator->GetVelocity() / velFactor;
    }

    double wavelength = (TRITON_TWOPI * velocity * velocity) / (gravity / worldUnits);

    double unfadedAmplitude = (1.0 / fastPow(distFromSource / worldUnits, waveDecay)) * (wavelength / 7.0) * wakeScale;

    if (unfadedAmplitude > maxAmplitude / worldUnits) unfadedAmplitude = maxAmplitude / worldUnits;

    if (elapsedTime > wakeFadeInTime || wakeFadeInTime <= 0) {
        amplitude = unfadedAmplitude;
    } else {
        amplitude = unfadedAmplitude * (elapsedTime / wakeFadeInTime);
    }

    return (unfadedAmplitude > (minAmplitude * wakeScale) / worldUnits);
}

WakeManager::WakeManager(double worldUnits, ParticleSystemManager *psm)
{
    particleSystemManager = psm;

    environment = 0;
    maxCircularWaves = maxKelvinWakes = maxPropWashes = 0;

    rotorWashTexture = 0;
    tidalStreamTexture = 0;

    waveGenerationDistance = 0.5;
    Configuration::GetDoubleValue("wake-wave-generation-distance", waveGenerationDistance);

    sprayGenerationPeriod = 1.0;
    Configuration::GetDoubleValue("wake-spray-generation-period", sprayGenerationPeriod);

    wakeSprayElevation = 0;
    Configuration::GetDoubleValue("wake-spray-elevation", wakeSprayElevation);

    leewardDampening = false;
    Configuration::GetBoolValue("leeward-dampening", leewardDampening);

    maxKelvinDist = 1000.0;
    maxCircularDist = 1000.0;
    maxLeewardDampenerDist = 1000.0;
    Configuration::GetDoubleValue("max-kelvin-wake-distance", maxKelvinDist);
    Configuration::GetDoubleValue("max-circular-wave-distance", maxCircularDist);
    Configuration::GetDoubleValue("max-leeward-dampener-distance", maxLeewardDampenerDist);
    maxKelvinDist /= worldUnits;
    maxCircularDist /= worldUnits;
    maxLeewardDampenerDist /= worldUnits;

    unitScale = worldUnits;

    nextSpray = 0;

    numSprays = 10;
    Configuration::GetIntValue("wake-max-sprays", numSprays);

    cullSprays = true;
    Configuration::GetBoolValue("wake-cull-sprays", cullSprays);

    sprays = TRITON_NEW ParticleSystem*[numSprays];
    for (int i = 0; i < numSprays; i++) {
        sprays[i] = 0;
    }

    numParticles = 10;
    Configuration::GetIntValue("wake-spray-particles", numParticles);

    spraySize = 5.0;
    Configuration::GetDoubleValue("wake-spray-particle-size", spraySize);
    spraySize /= worldUnits;

    spraySizeVariation = 0.2;
    Configuration::GetDoubleValue("wake-spray-size-variation", spraySizeVariation);

    sprayPositionVariation = 2.0;
    Configuration::GetDoubleValue("wake-spray-position-variation", sprayPositionVariation);
    sprayPositionVariation /= worldUnits;

    sprayVelocityVariation = 0.2;
    Configuration::GetDoubleValue("wake-spray-velocity-variation", sprayVelocityVariation);

    sprayDirectionVariation = 0.2;
    Configuration::GetDoubleValue("wake-spray-direction-variation", sprayDirectionVariation);

    sprayTransparency = 0.5;
    Configuration::GetDoubleValue("wake-spray-transparency", sprayTransparency);

    Configuration::GetDoubleValue("wake-wash-length", wakeLength);
    wakeLength /= worldUnits;

    gravity = 9.81;
    Configuration::GetDoubleValue("wave-gravity-force", gravity);

    tidalStreamFoam = 0.1f;
    Configuration::GetFloatValue("tidal-stream-foam", tidalStreamFoam);
}

WakeManager::~WakeManager()
{
    TRITON_MAP(WakeGenerator*, KelvinWake*)::iterator explicitWavesIt;
    TRITON_MAP(TidalStreamWake*, KelvinWake*)::iterator explicitWavesTSWIt;
    TRITON_MAP(WakeGenerator*, LeewardDampener*)::iterator leewardDampenersIt;

    for (explicitWavesIt = explicitWaves.begin(); explicitWavesIt != explicitWaves.end(); explicitWavesIt++) {
        WakeGenerator *wg = explicitWavesIt->first;
        if (wg) {
            wg->WakeManagerDestroyed();
        }
    }

    for (explicitWavesTSWIt = explicitWavesTSW.begin(); explicitWavesTSWIt != explicitWavesTSW.end(); explicitWavesTSWIt++) {
        TidalStreamWake *tsw = explicitWavesTSWIt->first;
        if (tsw) {
            tsw->WakeManagerDestroyed();
        }
    }

    for (leewardDampenersIt = leewardDampeners.begin(); leewardDampenersIt != leewardDampeners.end(); leewardDampenersIt++) {
        WakeGenerator *wg = leewardDampenersIt->first;
        if (wg) {
            wg->WakeManagerDestroyed();
        }
    }

    TRITON_LIST(WaterDisturbance *)::iterator it;
    for (it = waves.begin(); it != waves.end(); it++) {
        TRITON_DELETE *it;
    }
    waves.clear();

    if (sprays) {
        for (int i = 0; i < numSprays; i++) {
            if (sprays[i]) {
                TRITON_DELETE sprays[i];
            }
        }
        TRITON_DELETE[] sprays;
    }

}

void WakeManager::AddSpray(double shipVelocity, const Vector3& position, double time, double positionVariation, double effectScale)
{
    NewSprayRequest sprayRequest;
    sprayRequest.velocity = shipVelocity;
    sprayRequest.hasDirection = false;
    sprayRequest.position = position;
    sprayRequest.time = time;
    sprayRequest.particleSize = spraySize;
    sprayRequest.transparency = (float)sprayTransparency;
    sprayRequest.posVariation = positionVariation == -1.0 ? sprayPositionVariation : positionVariation;
    sprayRequest.effectScale = effectScale;
    sprayRequests.push_back(sprayRequest);
}

void WakeManager::AddDirectionalSpray(double shipVelocity, const Vector3& position, const Vector3& direction, double time, double particleSize, float transparency, double posVariation)
{
    NewSprayRequest sprayRequest;
    sprayRequest.velocity = shipVelocity;
    sprayRequest.hasDirection = true;
    sprayRequest.position = position;
    sprayRequest.direction = direction;
    sprayRequest.time = time;
    sprayRequest.particleSize = particleSize == -1.0 ? spraySize : particleSize;
    sprayRequest.transparency = transparency == -1.0f ? (float)sprayTransparency : transparency;
    sprayRequest.posVariation = posVariation == -1.0 ? sprayPositionVariation : posVariation;
    sprayRequest.effectScale = 1.0;
    sprayRequests.push_back(sprayRequest);
}

void WakeManager::AddExplicitWave(WakeGenerator *generator, double size)
{
    KelvinWake *wave = TRITON_NEW KelvinWake();
    wave->generator = generator;
    wave->isExplicit = true;
    //wave->wavelength = size;
    //wave->radius = 0;
    waves.push_back(wave);

    explicitWaves[generator] = wave;
}

void WakeManager::UpdateExplicitWave(WakeGenerator *generator, const Vector3& position, const Vector3& direction, double amplitude, double wavelength)
{
    TRITON_MAP(WakeGenerator*, KelvinWake*)::iterator it;
    it = explicitWaves.find(generator);
    if (it != explicitWaves.end()) {
        KelvinWake *kw = it->second;

        kw->position = position - direction * wavelength;
        kw->amplitude = amplitude;
        kw->explicitOrigin = position;
    }
}

void WakeManager::AddExplicitWave(TidalStreamWake *generator, double size)
{
    KelvinWake *wave = TRITON_NEW KelvinWake();
    wave->tidalStream = generator;
    wave->isExplicit = true;
    wave->foamAmount = tidalStreamFoam;
    waves.push_back(wave);

    explicitWavesTSW[generator] = wave;
}

void WakeManager::UpdateExplicitWave(TidalStreamWake *generator, const Vector3& position, const Vector3& direction, double amplitude, double wavelength)
{
    TRITON_MAP(TidalStreamWake*, KelvinWake*)::iterator it;
    it = explicitWavesTSW.find(generator);
    if (it != explicitWavesTSW.end()) {
        KelvinWake *kw = it->second;

        kw->position = position - direction * wavelength;
        kw->amplitude = amplitude;
        kw->explicitOrigin = position;
    }
}

void WakeManager::AddLeewardDampener(WakeGenerator *generator)
{
    if (!leewardDampening) return;

    LeewardDampener *dampener = TRITON_NEW LeewardDampener();
    dampener->generator = generator;

    waves.push_back(dampener);

    leewardDampeners[generator] = dampener;
}

void WakeManager::UpdateLeewardDampener(WakeGenerator *generator, const Vector3& bowPos, const Vector3& sternPos, double velocity)
{
    if (!leewardDampening) return;

    TRITON_MAP(WakeGenerator *, LeewardDampener*)::iterator it;
    it = leewardDampeners.find(generator);
    if (it != leewardDampeners.end()) {
        LeewardDampener *dampener = it->second;

        dampener->position = (bowPos + sternPos) * 0.5;
        dampener->bowPos = bowPos;
        dampener->sternPos = sternPos;
        float velocityDampening = ((float)velocity / LeewardDampener::scaledMaxLeewardDampeningVelocity);
        if (velocityDampening < 0) velocityDampening = 0;
        if (velocityDampening > 1.0f) velocityDampening = 1.0f;
        dampener->velocityDampening = 1.0f - velocityDampening;
    }
}

void WakeManager::AddCircularWave(double shipVelocity, const Vector3& position, double time, double fadeTime,const Vector3& direction,
                                  double beamWidth, double decay, const WakeGenerator *generator)
{
    CircularWave *wave = TRITON_NEW CircularWave();
    wave->position = position;
    wave->startTime = time;
    if (generator) {
        wave->startDistanceTravelled = generator->GetDistanceTravelled();
    }

    const double wakeAngle = RADIANS(19.47);
    double velFactor = 1.0 / tan(wakeAngle);

    wave->velocity = shipVelocity / velFactor;
    wave->shipDirection = direction;
    wave->beamWidth = beamWidth;
    wave->generator = generator;
    wave->waveDecay = decay;
	wave->wakeFadeInTime = fadeTime;

    // Angle of the caustic is 38.94 degrees per Kelvin
    double theta = RADIANS(38.94);
    double cosTheta = cos(theta);
    wave->wavelength = (2.0 * TRITON_PI * shipVelocity * shipVelocity * cosTheta * cosTheta) / (gravity / unitScale);

    waves.push_back(wave);
}

void WakeManager::AddKelvinWake(double shipVelocity, const Vector3& position, double time, double decay, const WakeGenerator *generator)
{
    KelvinWake *wake = TRITON_NEW KelvinWake();
    wake->position = position;
    wake->startTime = time;
    if (generator) {
        wake->startDistanceTravelled = generator->GetDistanceTravelled();
    }

    //const double wakeAngle = RADIANS(19.47);
    //double velFactor = 1.0 / tan(wakeAngle);

    const double velFactor = 2.8286;

    wake->velocity = shipVelocity / velFactor;
    wake->generator = generator;
    wake->waveDecay = decay;

    waves.push_back(wake);
}

void WakeManager::AddPropWash(int wakeNumber, const Vector3& washPosition, const Vector3& deltaPos,
                              double length, double beamWidth, double time, double fadeTime, const WakeGenerator *generator)
{
    PropWashSegment *seg = TRITON_NEW PropWashSegment(wakeLength, fadeTime);
    seg->position = washPosition;
    if (generator) {
        seg->startDistanceTravelled = generator->GetDistanceTravelled();
    }

    seg->deltaPos = deltaPos;
    seg->beamWidth = beamWidth;
    seg->shipLength = length;
    seg->generator = generator;
    seg->number = wakeNumber;
    seg->fadeTime = fadeTime;
    seg->startTime = time;
    seg->lastTime = time;

    waves.push_back(seg);
}

void WakeManager::UpdatePropWash(int wakeNumber, const Vector3& newPropPosition, const Vector3& newDelta, double time, const WakeGenerator *generator)
{
    // Find the impulse
    TRITON_LIST(WaterDisturbance *)::iterator it;
    for (it = waves.begin(); it != waves.end(); it++) {

        if ( (*it)->GetClassID() == PROP_WASH) {
            PropWashSegment *seg = reinterpret_cast<PropWashSegment*>(*it);

            if ( seg->number == wakeNumber && seg->generator == generator ) {
                // Got it
                seg->position = newPropPosition;
                seg->deltaPos = newDelta;
                seg->lastTime = time;
                break;
            }
        }
    }
}

static Vector3 camPos;
bool WaterDisturbanceSortDistancePredicate(const WaterDisturbance * d1, const WaterDisturbance * d2)
{
    const WakeGenerator *w1, *w2;
    w1 = d1->generator;
    w2 = d2->generator;
    if (w1 == w2) {
        double d1ds = d1->GetDistanceSquared(camPos);
        double d2ds = d2->GetDistanceSquared(camPos);
        if (fabs(d1ds - d2ds) < 0.001) {
            if (d1->GetClassID() == PROP_WASH && d2->GetClassID() == PROP_WASH) {
                const PropWashSegment *d1ps, *d2ps;
                d1ps = reinterpret_cast<const PropWashSegment*>(d1);
                d2ps = reinterpret_cast<const PropWashSegment*>(d2);

                return d1ps->number < d2ps->number;
            } else if (d1->GetClassID() == CIRCULAR_WAVE && d2->GetClassID() == CIRCULAR_WAVE) {
                const CircularWave *d1cw, *d2cw;
                d1cw = reinterpret_cast<const CircularWave*>(d1);
                d2cw = reinterpret_cast<const CircularWave*>(d2);

                return d1cw->amplitude > d2cw->amplitude;
            }
            return false;
        } else {
            return d1ds < d2ds;
        }
    } else {
        double wd1 = 0, wd2 = 0;
        if (w1) {
            wd1 = (w1->GetPosition() - camPos).SquaredLength();
        } else {
            wd1 = d1->GetDistanceSquared(camPos);
        }
        if (w2) {
            wd2 = (w2->GetPosition() - camPos).SquaredLength();
        } else {
            wd2 = d2->GetDistanceSquared(camPos);
        }

        return wd1 < wd2;
    }

}

bool WaterDisturbanceSortNumberPredicate(const WaterDisturbance * d1, const WaterDisturbance * d2)
{
    // Sort first by WakeGenerator, then by number.
    const WakeGenerator *w1, *w2;
    w1 = d1->generator;
    w2 = d2->generator;

    if (w1 == w2) {
        if (d1->GetClassID() == PROP_WASH && d2->GetClassID() == PROP_WASH) {
            const PropWashSegment *d1ps, *d2ps;
            d1ps = reinterpret_cast<const PropWashSegment *>(d1);
            d2ps = reinterpret_cast<const PropWashSegment *>(d2);

            return d1ps->number < d2ps->number;
        }
        return false;
    } else {
        return w1 < w2;
    }
}

int WakeManager::ApplyLODs(const Vector3& camPos, const Environment *env, int lodBias)
{
    int numPropWashes = 0;

    // The waves vector should be sorted first by WakeGenerator, then by number here.
    //std::sort(waves.begin(), waves.end(), WaterDisturbanceSortNumberPredicate);
    waves.sort(WaterDisturbanceSortNumberPredicate);
    //printf("Num waves %d\n", waves.size());

    TRITON_LIST(WaterDisturbance *)::iterator it;
    for (it = visibleWaves.begin(); it != visibleWaves.end(); it++) {
        TRITON_DELETE (*it);
    }
    visibleWaves.clear();
    // visibleWaves.reserve(waves.size());

    const WakeGenerator *currentWakeGenerator = 0;
    Vector3 prevPropPos(0,0,0);
    double prevStartDistance = 0;
    int lod = 0;
    bool hasPreviousPos = false;

    double zoomFactor = 1.0;
    if (environment) zoomFactor = (double)environment->GetZoomLevel();
    double zoomedKelvin = maxKelvinDist * zoomFactor;
    double zoomedCircular = maxCircularDist * zoomFactor;
    double zoomedLeeward = maxLeewardDampenerDist * zoomFactor;

    double maxKelvinDistSq = zoomedKelvin * zoomedKelvin;
    double maxCircularDistSq = zoomedCircular * zoomedCircular;
    double maxLeewardDampenerDistSq = zoomedLeeward * zoomedLeeward;

    it = waves.begin();
    while (it != waves.end()) {
        const WakeGenerator *wg = (*it)->generator;
        if (wg && (*it)->GetClassID() == PROP_WASH) {

            PropWashSegment *seg = reinterpret_cast<PropWashSegment*>(*it);

            if (wg != currentWakeGenerator) {
                if (wg->GetLODDistance() > 0) {
                    double distance = (camPos - wg->GetPosition()).Length();
                    lod = (int)(floor(distance / wg->GetLODDistance()));
                } else {
                    lod = 0;
                }

                lod += lodBias;

                currentWakeGenerator = wg;
                hasPreviousPos = false;
            }

            // Always use the last & second from last points.
            bool mostRecentSegment = false;
            int maxSegment = wg->GetMaxWakeNumber();
            if ( seg->number == maxSegment || seg->number == (maxSegment - 1)) {
                mostRecentSegment = true;
            }

            if ( lod == 0 || !hasPreviousPos || mostRecentSegment || (seg->number % lod) == 0) {

                Vector3 delta = prevPropPos - seg->position;
                double halfDelta = delta.Length() * 0.5;
                double halfWashWidth = seg->washWidth * 0.5;
                double radius = halfDelta + halfWashWidth;
                Vector3 washCenter = seg->position + delta * 0.5;

                if (!env->CullSphere(washCenter, radius)) {

                    PropWashSegment *newWave = TRITON_NEW PropWashSegment(*seg);

                    if (hasPreviousPos) {
                        newWave->deltaPos = delta;
                    }

                    if (prevStartDistance > 0) {
                        newWave->startDistanceTravelled = prevStartDistance;
                    }

                    visibleWaves.push_back(newWave);
                    numPropWashes++;
                }

                prevPropPos = seg->position;

                prevStartDistance = 0;

                TRITON_LIST(WaterDisturbance *)::iterator it2 = it;
                it2++;
                while (it2 != waves.end()) {
                    bool isPropWash = (*it2)->GetClassID() == PROP_WASH;

                    if (isPropWash && (*it2)->generator == currentWakeGenerator) {
                        PropWashSegment *nextSeg = reinterpret_cast<PropWashSegment*>(*it2);
                        prevStartDistance = nextSeg->startDistanceTravelled;
                        break;
                    }
                    it2++;
                }

                hasPreviousPos = true;
            }
        } else {

            if ( (*it)->GetClassID() == KELVIN_WAKE) {
                KelvinWake *kw = reinterpret_cast<KelvinWake*>(*it);
                if (kw->GetDistanceSquared(camPos) < maxKelvinDistSq) {
                    visibleWaves.push_back(TRITON_NEW KelvinWake(*kw));
                }
            }

            if ( (*it)->GetClassID() == CIRCULAR_WAVE) {
                CircularWave *cw = reinterpret_cast<CircularWave*>(*it);
                if (environment->GetRenderer() == NO_RENDERER) {
                    visibleWaves.push_back(TRITON_NEW CircularWave(*cw));
                } else {
                    if (cw->GetDistanceSquared(camPos) < maxCircularDistSq) {
                        visibleWaves.push_back(TRITON_NEW CircularWave(*cw));
                    }
                }
            }

            if ((*it)->GetClassID() == LEEWARD_DAMPENER) {
                LeewardDampener *ld = reinterpret_cast<LeewardDampener*>(*it);
                if (ld->GetDistanceSquared(camPos) < maxLeewardDampenerDistSq) {
                    visibleWaves.push_back(TRITON_NEW LeewardDampener(*ld));
                }
            }
            //hasPreviousPos = false;
        }
        it++;
    }

    // Now sort by distance so closest wakes get generated first
    //std::sort(visibleWaves.begin(), visibleWaves.end(), WaterDisturbanceSortDistancePredicate);
    visibleWaves.sort(WaterDisturbanceSortDistancePredicate);
    return numPropWashes;
}

void WakeManager::Update(const Vector3& cameraPosition, double time, const Environment *env)
{
    camPos = cameraPosition;

    TRITON_VECTOR(NewSprayRequest)::iterator sit = sprayRequests.begin();
    while (sit != sprayRequests.end()) {
        NewSprayRequest request = *sit;
        if (sprays && sprays[nextSpray]) {
            Vector3 adjustedPosition = request.position;
            double offset = request.particleSize * request.effectScale * 0.5;
            adjustedPosition = adjustedPosition - (environment->GetUpVector() * offset);
            adjustedPosition = adjustedPosition + (environment->GetUpVector() * wakeSprayElevation);

            // Don't bother if it's not in view
            if (!env->CullSphere(adjustedPosition, request.particleSize) || !cullSprays) {
                sprays[nextSpray]->Start(request.time, request.hasDirection ? request.direction : environment->GetUpVector(),
                                         (float)sprayDirectionVariation, (float)request.velocity,
                                         (float)(request.velocity * sprayVelocityVariation), adjustedPosition, (float)(request.posVariation * request.effectScale),
                                         (float)(request.particleSize * request.effectScale), (float)(request.particleSize * request.effectScale * spraySizeVariation));

                sprays[nextSpray]->SetTransparency(request.transparency);

                nextSpray++;
                if (nextSpray == numSprays) nextSpray = 0;
            }
        }

        sit++;
    }
    sprayRequests.clear();

    //printf("Num waves: %d\n", waves.size());

    TRITON_LIST(WaterDisturbance *)::iterator it = waves.begin();
    while (it != waves.end()) {
        bool active = (*it)->Update(time, unitScale);
        if (!active) {
            TRITON_DELETE (*it);
            it = waves.erase(it);
        } else {
            it++;
        }
    }

    // Apply LOD's
    int numWashes = ApplyLODs(cameraPosition, env, 0);
    int lodBias = 1;
    int iteration = 0;
    const int maxIterations = 10;
    while (numWashes > maxPropWashes && iteration < maxIterations) { // Force LOD if we ran out.
        int newNumWashes = ApplyLODs(cameraPosition, env, lodBias++);
        if (newNumWashes >= numWashes) { // It is hopeless.
            break;
        }
        numWashes = newNumWashes;
        iteration++;
    }
}

void WakeManager::RemoveWakeGenerator(WakeGenerator *generator)
{
    TRITON_LIST(WaterDisturbance *)::iterator it = waves.begin();
    while (it != waves.end()) {
        if ( (*it)->generator == generator ) {
            TRITON_DELETE (*it);
            it = waves.erase(it);
        } else {
            it++;
        }
    }

    TRITON_MAP(WakeGenerator *, KelvinWake*)::iterator it2;
    it2 = explicitWaves.find(generator);
    if (it2 != explicitWaves.end()) {
        explicitWaves.erase(it2);
    }
}

void WakeManager::RemoveWakeGenerator(TidalStreamWake *generator)
{
    TRITON_LIST(WaterDisturbance *)::iterator it = waves.begin();
    while (it != waves.end()) {
        if ((*it)->GetClassID() == KELVIN_WAKE) {
            KelvinWake *kw = reinterpret_cast<KelvinWake*>((*it));
            if (kw->tidalStream == generator) {
                TRITON_DELETE(*it);
                it = waves.erase(it);
                continue;
            }
        }

        it++;
    }

    TRITON_MAP(TidalStreamWake *, KelvinWake*)::iterator it2;
    it2 = explicitWavesTSW.find(generator);
    if (it2 != explicitWavesTSW.end()) {
        explicitWavesTSW.erase(it2);
    }
}

void WakeManager::RemoveLeewardDampener(WakeGenerator *generator)
{
    if (!leewardDampening) return;

    TRITON_MAP(WakeGenerator *, LeewardDampener *)::iterator it2;
    it2 = leewardDampeners.find(generator);
    if (it2 != leewardDampeners.end()) {
        leewardDampeners.erase(it2);
    }
}

float WakeManager::GetHeight(const Vector3& position, const ProjectedGrid *grid, Vector3& normal)
{
    if (!grid) return 0.0f;

    float height = 0;
    Vector3 slope(0.0, 0.0, 0.0);
    TRITON_LIST(WaterDisturbance *)::iterator it;
    for (it = waves.begin(); it != waves.end(); it++) {
        if ( (*it)->GetClassID() == CIRCULAR_WAVE) {
            CircularWave *wave = reinterpret_cast<CircularWave *>(*it);
            if (wave) {
                Vector3 D = (position - wave->position);
                float dist = (float)(D.Length());
                float r = dist - (float)(wave->radius);
                if (wave->wavelength > 0 && dist > 0) {
                    if (fabs(r) < ((float)(wave->wavelength) * 0.5f)) {
                        float theta = (float)(TRITON_TWOPI / wave->wavelength) * r;

                        float sinTheta = sinf(theta);
                        float cosTheta = sqrtf(1.0f - sinTheta * sinTheta);

                        height += (float)(wave->amplitude) * cosTheta;

                        float derivative = (float)(wave->amplitude) * -sinTheta;
                        slope = slope + D * (derivative / dist);
                    }
                }
            }
        }
    }

    Vector3 sx(1.0, 0.0, slope.x);
    Vector3 sy(0.0, 1.0, slope.y);
    normal = sx.Cross(sy);
    normal.Normalize();

    normal = normal * grid->GetInverseBasis();

    return height;
}