﻿// Copyright 2006-2015 Sundog Software, LLC. All rights reserved worldwide.

#if defined(WIN32) || defined(WIN64)
#include <windows.h>
#else
#include <cstdio>
#include <unistd.h>
#include <dlfcn.h>
#endif

#include <FeTriton/LicenseManager.h>
#include <string>
#include <vector>
#include <time.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

using namespace std;
using namespace Triton;

static time_t startTime = 0;
static bool validated = false;

#ifdef DONGLE_SUPPORT

#define ValidateCode1 0xC121
#define ValidateCode2 0x8C25
#define ValidateCode3 0x2947
#define ClientIDCode1 0x1FF0
#define ClientIDCode2 0xF488
#define ReadCode1     0xBDFB
#define ReadCode2     0x394
#define ReadCode3     0x6B3E

/* COMMAND CODES */
#define TERMINATE      -1
#define KLCHECK         1

static unsigned long ReturnValue;

#if defined(WIN32) || defined(WIN64)
typedef unsigned long int (__stdcall *KFUNC_PROC)(int,int,int,int);
static unsigned ReturnValue1, ReturnValue2, Value;
typedef int kltype;
static HMODULE dll = 0;
#define RotateLeft(a, n) (((a >> (16 - n)) & 0xFFFF) | ((a & 0xFFFF) << n))
#else
typedef unsigned long (*KFUNC_PROC)(unsigned, unsigned, unsigned, unsigned);
typedef unsigned kltype;
static unsigned short ReturnValue1, ReturnValue2;
static void * dll = 0;

static unsigned RotateLeft(unsigned Target, int Counts)
/* This function rotates the bits in the Target left the number of positions
   identified by the argument Counts */
{
    int i;
    static unsigned LocalTarget, HighBit;

    LocalTarget = Target;
    for (i=0; i<Counts; i++) {
        HighBit = LocalTarget & 0X8000;
        LocalTarget = (LocalTarget << 1) + (HighBit >> 15);
    }
    LocalTarget = LocalTarget & 0XFFFF; /* For 32 bit integers */
    return (LocalTarget);
}

#endif

static KFUNC_PROC KFUNC = 0;

static bool LoadKeyLok(bool standalone)
{
#if defined(WIN32) || defined(WIN64)
    // First try local key DLL, then try network key DLL.
#ifdef WIN32
    if (!dll) dll = LoadLibraryA(standalone ? "kl2dll32.dll" : "nwkl2_32.dll");
#else
    if (!dll) dll = LoadLibraryA(standalone ? "kl2dll64.dll" : "nwkl2_64.dll");
#endif

    if (dll && !KFUNC) {
        KFUNC = (KFUNC_PROC) GetProcAddress(dll, "KFUNC");
    }

#else
    if (!standalone) return false;

#if defined(__LP64__) || defined(__LP64)
    if (!dll) dll = dlopen( "libkfunc64.so", RTLD_LAZY | RTLD_GLOBAL);
#else
    if (!dll) dll = dlopen("libkfunc32.so", RTLD_LAZY | RTLD_GLOBAL);
#endif
    if (dll && !KFUNC) {
        KFUNC = (KFUNC_PROC) dlsym( dll,  "KFUNC" );
    }
#endif

    return KFUNC != 0;
}

static void KTASK(kltype CommandCode, kltype Argument2, kltype Argument3, kltype Argument4)
{
    if (KFUNC) {
        ReturnValue =  KFUNC(CommandCode, Argument2, Argument3, Argument4);
        ReturnValue1 = (kltype)ReturnValue & 0XFFFF;
        ReturnValue2 = (kltype)(ReturnValue >> 16);
    }
}

static void FreeKeyLok()
{
#if defined(WIN32) || defined(WIN64)
    if (dll) FreeLibrary(dll);
#else
    if (dll) dlclose(dll);
#endif
    dll = 0;
    KFUNC = 0;
}

static bool CheckForDongle()
{
    bool found = false;
    // First try standalone
    if (LoadKeyLok(true)) {
        KTASK((kltype)(KLCHECK), ValidateCode1, ValidateCode2, ValidateCode3);
        KTASK(RotateLeft(ReturnValue1, (ReturnValue2 & 7)) ^ ReadCode3 ^ ReturnValue2,
              RotateLeft(ReturnValue2, (ReturnValue1 & 15)),
              ReturnValue1 ^ ReturnValue2, 0);

        found = ((ReturnValue1 == ClientIDCode1) && (ReturnValue2 == ClientIDCode2));
    }

    // Now try network
    if (!found) {
        if (LoadKeyLok(false)) {
            KTASK((kltype)(KLCHECK), ValidateCode1, ValidateCode2, ValidateCode3);
            KTASK(RotateLeft(ReturnValue1, (ReturnValue2 & 7)) ^ ReadCode3 ^ ReturnValue2,
                  RotateLeft(ReturnValue2, (ReturnValue1 & 15)),
                  ReturnValue1 ^ ReturnValue2, 0);

            found = ((ReturnValue1 == ClientIDCode1) && (ReturnValue2 == ClientIDCode2));
        }
    }

    return found;
}
#else //DONGLE_SUPPORT
static bool CheckForDongle()
{
    return false;
}
#endif //DONGLE_SUPPORT

void LicenseManager::Shutdown()
{
#ifdef DONGLE_SUPPORT
    FreeKeyLok();
#endif
}

bool LicenseManager::ValidateCode(const char *name, const char *code)
{
    if (name && code) {
        if (strlen(name) > 0 && strlen(code) > 0) {
            const char *decrypted = DecryptDongle(code);
            if (strcmp(name, decrypted) == 0) {
                validated = CheckForDongle();

                if (!validated) {
#if (defined(WIN32) || defined(WIN64)) && (_MSC_VER > 1310)
                    MessageBoxA(NULL, "Your license is node-locked and requires a USB dongle, but no valid "
                                "dongle was found. Please ensure your Sundog dongle is connected and its "
                                "drivers are installed. Contact sales@sundog-soft.com to purchase new licenses "
                                "if necessary. Application will terminate in 5 minutes.", "Unlicensed Software!", MB_OK);
#else
                    printf("Your license is node-locked and requires a USB dongle, but no valid "
                           "dongle was found. Please ensure your Sundog dongle is connected and its "
                           "drivers are installed. Contact sales@sundog-soft.com to purchase new licenses "
                           "if necessary. Application will terminate in 5 minutes.\n");
#endif
                    startTime = time(NULL);
                    return false;
                }

                return true;
            }


            decrypted = DecryptProd(code);
            if (strcmp(name, decrypted) == 0) {
                validated = true;
                return true;
            }

            decrypted = DecryptDevo(code);
            if (strcmp(name, decrypted) == 0) {
                static bool displayNotification = true;
                validated = true;

                if( displayNotification ) {
                    displayNotification = false;
#if (defined(WIN32) || defined(WIN64))
                    char buf[1024];

#if _MSC_VER >= 1400
                    sprintf_s(buf, 1024, "%s is using a development license for Triton. Please contact "
                              "sales@sundog-soft.com to purchase a production license prior to distributing your "
                              "application, and to remove this notice.", name);
#else
                    sprintf(buf, "%s is using a development license for Triton. Please contact "
                            "sales@sundog-soft.com to purchase a production license prior to distributing your "
                            "application, and to remove this notice.", name);
#endif

                    MessageBoxA(NULL, buf, "Development License", MB_OK);
#else
                    printf("%s is using a development license for Triton. Please contact "
                           "sales@sundog-soft.com to purchase a production license prior to distributing your "
                           "application, and to remove this notice.\n", name);
#endif
                }
                return true;
            }
        }
    }

    // No valid license at all:

    validated = false;


#if (defined(WIN32) || defined(WIN64))
    char buf[1024];
#if _MSC_VER >= 1400
    sprintf_s(buf, 1024, "Evaluation or invalid Triton license code specified for user \"%s\".\n"
              "Please contact sales@sundog-soft.com to purchase a Triton license,\n"
              "or visit www.sundog-soft.com.\n\n"
              "Need help with integration? Contact support@sundog-soft.com.\n\n"
              "Application will terminate in 15 minutes.", name);
#else
    sprintf(buf, "Evaluation or invalid Triton license code specified for user \"%s\".\n"
            "Please contact sales@sundog-soft.com to purchase a Triton license,\n"
            "or visit www.sundog-soft.com.\n\n"
            "Need help with integration? Contact support@sundog-soft.com.\n\n"
            "Application will terminate in 15 minutes.", name);
#endif
    MessageBoxA(NULL, buf, "Unlicensed or Evaluation Software!", MB_OK);
#else
    printf("Evaluation or invalid Triton license code specified for user \"%s\".\n"
           "Please contact sales@sundog-soft.com to purchase a Triton license,\n"
           "or visit www.sundog-soft.com.\n\n"
           "Need help with integration? Contact support@sundog-soft.com.\n\n"
           "Application will terminate in 15 minutes.\n", name);
#endif

    startTime = time(NULL);

    return false;
}

void LicenseManager::Heartbeat()
{
    if (!validated) {
        time_t now = time(NULL);
        if ((now - startTime) > (60 * 15)) {
            std::string msg("Per-run time limit reached for unlicensed / evaluation Triton, "
                            "or Environment::SetLicenseCode() was not called. "
                            "Please purchase a license from www.sundog-soft.com to remove this restriction.\n");
#ifdef _WIN32
            OutputDebugStringA(msg.c_str());
            MessageBoxA(NULL, msg.c_str(), "Triton Time Limit Reached", MB_OK);
            startTime = now;
#else
            printf("%s\n", msg.c_str());
            exit(0);
#endif

        }
    }
}

static char prodKey[] = "Elly Elephant was the sweetest elephant who ever lived. Every day when she awoke, "
                        "she told herself, 'Today I will make someone else's life better.' So Elly Elephant "
                        "gave money to friends. She did favors for relatives. And she helped all of her neighbors.";


static char devoKey[] = "One day Elly Elephant's house burned down. 'It is okay,' she said, 'because I have friends "
                        "and neighbors. They will help me as I have helped them.' But no one came. So Elly Elephant "
                        "gathered together all of her friends and relatives and neighbors. And pounced on their heads.";

static char dongleKey[] = "OpenGL is the only cross-platform graphics API that enables developers of software "
                          "for PC, workstation, and supercomputing hardware to create high-performance, visually-"
                          "compelling graphics software applications, in markets such as CAD, content creation, "
                          "energy, entertainment, game development, manufacturing, medical, and virtual reality.";

const char *LicenseManager::FromBase64(const char *in)
{
    static TRITON_STRING out;
    out.erase();

    int len = (int)strlen(in);

    for (int i = 0; i < len; i += 2) {
        char c1 = in[i];
        char c2 = in[i+1];
        char low, high;

        if (c1 <= '9') {
            high = (c1 - '0');
        } else {
            high = (c1 - 'a') + 10;
        }

        if (c2 <= '9') {
            low = (c2 - '0');
        } else {
            low = (c2 - 'a') + 10;
        }

        char c = (high << 4) | low;

        out += c;
    }

    return out.c_str();
}

const char *LicenseManager::DecryptProd(const char *code)
{
    static TRITON_STRING out;
    out.erase();

    const char *encrypted = FromBase64(code);
    int len = (int)strlen(code) / 2;
    for (int i = 0; i < len; i++) {
        out += encrypted[i] ^ prodKey[i];
    }

    return out.c_str();
}

const char *LicenseManager::DecryptDevo(const char *code)
{
    static TRITON_STRING out;
    out.erase();

    const char *encrypted = FromBase64(code);
    int len = (int)strlen(code) / 2;
    for (int i = 0; i < len; i++) {
        out += encrypted[i] ^ devoKey[i];
    }

    return out.c_str();
}

const char *LicenseManager::DecryptDongle(const char *code)
{
    static TRITON_STRING out;
    out.erase();

    const char *encrypted = FromBase64(code);
    int len = (int)strlen(code) / 2;
    for (int i = 0; i < len; i++) {
        out += encrypted[i] ^ dongleKey[i];
    }

    return out.c_str();
}
