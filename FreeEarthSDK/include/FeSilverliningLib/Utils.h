// Copyright (c) 2004-2014  Sundog Software, LLC. All rights reserved worldwide.

/**
    \file Utils.h
    \brief General-purpose math and time macros and functions.
 */
#ifndef UTILS_H
#define UTILS_H

#include <FeSilverliningLib/Atmosphere.h>
#include <FeSilverliningLib/Billboard.h>

#if defined(WIN32) || defined(WIN64)
#include <windows.h>
#else
#include <sys/timeb.h>
#endif
#if defined(__INTEL_COMPILER)
#include <mathimf.h>
#else
#include <math.h>
#endif
#include <stdlib.h>

#include <sstream>
#include <locale>

// Convert watts/m2 to cd/m2
// wm2 * 683 lux / 1wm2 * 1 nit / 3.14 lux
#define NITS(irradiance) (irradiance * 683.0 / 3.14)

#define PI  3.14159265
#define TWOPI (PI * 2.0)
#define HALFPI (PI * 0.5)

#define RADIANS(x) ((x) * (PI / 180.0))
#define DEGREES(x) ((x) * (180.0 / PI))

#define METERS(x)   ((x) * 0.3048)
#define FEET(x)     ((x) * 3.2808)

#define SLMAX(a,b)            (((a) > (b)) ? (a) : (b))
#define SLMIN(a,b)            (((a) < (b)) ? (a) : (b))

/** A safe version of arc-cosine that protects from precision errors on the parameter. */
inline double myAcos(double x)
{
    return ((-1.0 < x) ? ((x < +1.0) ? acos(x) : acos(+1.0)) : acos(-1.0));
}

/** Returns a pseudo-random double from 0-1.0 with uniform distribution. */
inline double UniformRandom()
{
    SilverLining::Atmosphere *atm = SilverLining::Atmosphere::GetCurrentAtmosphere();
    if (atm)
        return atm->GetRandomNumberGenerator()->UniformRandomDouble();
    else
        return 0;
}

/** Returns a psuedo-random double from 0 - 1.0 with a normal distribution. */
inline double NormalRandom(double variance, double mean)
{
    double stddev = sqrt(variance);
    double S = 2.0;
    double U1, U2, V1, V2;

    while (S > 1.0)
    {
        U1 = UniformRandom();
        U2 = UniformRandom();

        V1 = 2.0 * U1 - 1;
        V2 = 2.0 * U2 - 1;
        S = V1*V1 + V2*V2;
    }

    double normalRandom = sqrt((-2.0 * log(S)) / S) * V1;
    return normalRandom * stddev + mean;
}

/** Returns the number of milliseconds elapsed. The time elapsed from may vary depending
   on the system, so this should only be used for relative time measurements. */
inline unsigned long getMilliseconds()
{
#if defined(WIN32) || defined(WIN64)
    return timeGetTime();
#else
    struct timeb tp;
    ftime(&tp);
    static time_t startTime = 0;
    if (startTime == 0) startTime = tp.time;
    return (((tp.time - startTime) * 1000) + tp.millitm);
#endif
}

/** Returns a very rough estimate (only one iteration) of the square root of a number.
   But it's really fast. */
inline double FastSquareRoot(double number) {
#if defined(WIN32) || defined(WIN64)
    long i;
    float x, y;
    const float f = 1.5F;

    float fnumber = (float)number;

    x = fnumber * 0.5F;
    y  = fnumber;
    i  = *( long * ) &y;
    i  = 0x5f3759df - ( i >> 1 );
    y  = *( float * ) &i;
    y  = y * ( f - ( x * y * y ) );
    // y  = y * ( f - ( x * y * y ) );
    return number * y;
#else
    // Sadly, GCC's strict aliasing rules don't allow these shenanigans.
    return sqrt(number);
#endif
}

inline double LocaleSafeAtoF(const char *str)
{
#if (_MSC_VER >= 1400)
    _locale_t l = _create_locale(LC_NUMERIC, "C");
    double result = _atof_l(str, l);
    _free_locale(l);
    return result;
#else
    double result;
    std::istringstream s(str);
    std::locale l("C");
    s.imbue(l);
    s >> result;
    return result;
#endif
}

/** 3D Perlin noise function. */
float Noise3(float vec[3]);

#endif
