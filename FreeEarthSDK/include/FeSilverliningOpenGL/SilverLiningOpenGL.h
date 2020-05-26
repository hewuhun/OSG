﻿
// The following ifdef block is the standard way of creating macros which make exporting
// from a DLL simpler. All files within this DLL are compiled with the SILVERLININGOPENGL_EXPORTS
// symbol defined on the command line. this symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see
// SILVERLININGOPENGL_API functions as being imported from a DLL, wheras this DLL sees symbols
// defined with this macro as being exported.
#ifdef FESILVERLININGOPENGL_LIBRARY
#define SILVERLININGDLL_API extern "C" __declspec(dllexport)
#else
//#define SILVERLININGDLL_API extern "C" __declspec(dllimport)
#define SILVERLININGDLL_API extern "C"
#endif

#ifndef FESILVERLININGOPENGL_LIBRARY_STATIC
//#include "../public headers/SilverLiningDLLCommon.h"
#include <FeSilverliningLib/SilverLiningDLLCommon.h>
#else
//#include "SilverLiningDLLCommon.h"
#include <FeSilverliningLib/SilverLiningDLLCommon.h>
#endif



