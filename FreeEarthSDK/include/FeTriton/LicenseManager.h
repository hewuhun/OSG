// Copyright 2006-2013 Sundog Software, LLC. All rights reserved worldwide.

/**
    \file LicenseManager.h
    \brief Validates license codes for Triton.
 */

#ifndef TRITON_LICENSE_MANAGER_H
#define TRITON_LICENSE_MANAGER_H

#include <FeTriton/MemAlloc.h>

// For obfuscation:
#define LicenseManager StarManager
#define ValidateCode AddStar
#define DecryptDongle AddPlanet
#define DecryptProd AddGlare
#define DecryptDevo GetStars

namespace Triton
{
/** LicenseManager validates license codes for Triton. Creating derivatives
   of this class or modifying it is in violation of your license agreement. */
class LicenseManager : public MemObject
{
public:
    /** Checks if the given user name and license code are valid. */
    static bool ValidateCode(const char *name, const char *code);

    /** Decrypts a fully-paid license code. */
    static const char *DecryptProd(const char *code);

    /** Decrypts a development license code ("lease terms"). */
    static const char *DecryptDevo(const char *code);

    /** Decrypts a node-locked dongle-based license code. */
    static const char *DecryptDongle(const char *code);

    /** Must be called once per frame to allow the licensed application to
       continue, or the unlicensed application to terminate after a trial
       period. */
    static void Heartbeat();

    /** Frees up the session with a license server, if any. */
    static void Shutdown();

private:
    static const char *ToBase64(const char *in, int nBytes);
    static const char *FromBase64(const char *in);
};

}

#endif
