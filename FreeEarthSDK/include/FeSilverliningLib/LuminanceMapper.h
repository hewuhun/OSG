// Copyright (c) 2004-2008  Sundog Software, LLC. All rights reserved worldwide.

/**
    \file LuminanceMapper.h
    \brief Manages tone-mapping for objects in the scene.
 */

#ifndef LUMINANCE_MAPPER_H
#define LUMINANCE_MAPPER_H

#include <FeSilverliningLib/MemAlloc.h>

namespace SilverLining
{
class Vector3;

/** Manages tone-mapping of high dynamic range images (such as the sky!) to
   values a computer display can reproduce. In addition to dynamic range
   compression, it also simulates scotopic vision by applying a blue-shift
   and loss of color information as lighting conditions degrade.

   See DURAND, F., AND DORSEY, J. 2000. Interactive tone mapping. In <i>Eurographics
   Workshop on Rendering</i>, 219?30 for some of the ideas behind this
   class.
 */
class LuminanceMapper : public MemObject
{
public:
/** Sets the modeled maximum luminance of the display, in "nits", or
   candela per square meter. */
    static void SetMaxDisplayLuminance(double nits);

    static void EnableToneMapping(bool enabled) {
        disableToneMapping = !enabled;
    }

/** Sets the log-average of the scene's luminance as perceived by both
   the eye's rods and cones, in nits. */
    static void SetSceneLogAvg(double rodNits, double coneNits);

/** Performs tone-mapping on an xyY color, where x and y are
   chromaticity and Y is luminance. The values passed in are modified
   by this method. Assumes that SetMaxDisplayLuminance() and
   SetSceneLogAvg() were previously called. */
    static void DurandMapper(double *x, double *y, double *Y);

/** Performs tone-mapping on a XYZ color. The color passed in is
   modified by this method. Assumes that SetMaxDisplayLuminance() and
   SetSceneLogAvg() were previously called. */
    static void DurandMapperXYZ(Vector3 *XYZ);

/** Returns the computed scale factors for mapping luminance for
   both the eye's rods and cones. Assumes SetMaxDisplayLuminance() and
   SetSceneLogAvg() were previously called. */
    static void GetLuminanceScales(double *rodSF, double *coneSF)
    {
        *rodSF = mR;
        *coneSF = mC;
    }

/** Retrieves the maximum display luminance previously set by
   SetMaxDisplayLuminance(). */
    static double GetMaxDisplayLuminance() {
        return Ldmax;
    }

/** Returns the luminance, in nits, that is mapped to the maximum
   luminance the display can represent. Luminances higher than this are displayed
   as white. Assumes SetMaxDisplayLuminance() and SetSceneLogAvg() were
   previously called. */
    static double GetBurnoutLuminance();

/** Retrieves the computed blend factor between rod and cone perception
   based on the current lighting conditions. Assumes SetSceneLogAvg() was
   previously called. */
    static double GetRodConeBlend() {
        return k;
    }

/** Retrieves the log-average rod and cone luminances in nits, as
   previously set by SetSceneLogAvg(). */
    static void GetSceneLogAvg(double *rodNits, double *coneNits)
    {
        *rodNits = LsavgR;
        *coneNits = LsavgC;
    }

private:
    static void ComputeScaleFactors();
    static double Ldmax, Lsavg;
    static double mR, mC, k, LsavgR, LsavgC;
    static bool disableToneMapping;
};
}

#endif
