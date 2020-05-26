// Copyright (c) 2010-2011 Sundog Software, LLC. All rights reserved worldwide.

#include <FeTriton/DynamicLibrary.h>

#if defined(WIN32) || defined(WIN64)
#include <Windows.h>
#else
#include <cstdio>
#include <unistd.h>
#include <dlfcn.h>
#endif

using namespace Triton;

DynamicLibrary::DynamicLibrary():
    handle(0), refCount(0)
{
}

DynamicLibrary::~DynamicLibrary()
{
    Close();
}

bool DynamicLibrary::Open(const TRITON_STRING& libraryName)
{
    refCount++;

#ifdef _WIN32
    handle = LoadLibrary(libraryName.c_str());
#else
    handle = dlopen( libraryName.c_str(), RTLD_LAZY | RTLD_GLOBAL);
#endif

    return Valid();
}

void DynamicLibrary::Close()
{
    refCount--;

    if (refCount <= 0 && handle) {
#ifdef _WIN32
        FreeLibrary((HMODULE)handle);
#else
        dlclose(handle);
#endif
        handle = 0;
        refCount = 0;
    }
}

DynamicLibrary::ProcAddress DynamicLibrary::GetProcAddress(const TRITON_STRING& procName) const
{
    if (!Valid()) return 0;

#ifdef _WIN32
    return (ProcAddress)( ::GetProcAddress( (HMODULE)handle, procName.c_str() ) );
#else
    return (ProcAddress)( dlsym( handle,  procName.c_str() ) );
#endif
}

TRITON_STRING DynamicLibrary::GetErrorMessage() const
{
    if (Valid()) return TRITON_STRING();

#ifdef _WIN32
    DWORD err = GetLastError();
    LPSTR buf;
    FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, NULL, err, NULL, (LPSTR)&buf, 0, NULL);
    return (buf!=0) ? TRITON_STRING(buf) : TRITON_STRING();
#else
    const char* error_str = dlerror();
    return (error_str!=0) ? TRITON_STRING(error_str) : TRITON_STRING();
#endif
}
