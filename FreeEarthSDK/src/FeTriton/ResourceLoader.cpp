// Copyright (c) 2010-2013 Sundog Software, LLC. All rights reserved worldwide.

#include <FeTriton/MemAlloc.h>
#include <FeTriton/ResourceLoader.h>
#include <stdio.h>
#include <string.h>
#include <string>
#include <stdlib.h>

#ifdef _WIN32
#ifndef _WIN32_WINNT        // Allow use of features specific to Windows NT 4 or later.
#define _WIN32_WINNT 0x0502 // Change this to the appropriate value to target Windows 98 and Windows 2000 or later.
#endif
#include <windows.h>
#endif

using namespace Triton;

ResourceLoader::ResourceLoader(const char *path, bool useAddDllDirectory)
{
    if (!path) {
        Utils::DebugMsg("Null path passed into ResourceLoader constructor");
    }

    resourcePath = 0;
    SetResourceDirPath(path, useAddDllDirectory);
}

ResourceLoader::~ResourceLoader()
{
    if (resourcePath) {
        TRITON_FREE(resourcePath);
    }
}

#ifdef _WIN32
// Enforce proper use of forward or back slashes depending on the OS, and strip duplicate slashes.
static TRITON_STRING ScrubPath(TRITON_STRING& path)
{
    TRITON_STRING newPath;

    bool prevCharWasSlash = false;

    for (unsigned int i = 0; i < path.length(); i++) {
#if defined(WIN32) || defined(WIN64)
        if (path[i] == '/') {
            if (!prevCharWasSlash) {
                newPath += '\\';
            }
        }
#else
        if (path[i] == '\\') {
            if (!prevCharWasSlash) {
                newPath += '/';
            }
        }
#endif
        else {
            if (path[i] == '/' || path[i] == '\\') {
                if (!prevCharWasSlash) {
                    newPath += path[i];
                }
            } else {
                newPath += path[i];
            }
        }

#if defined(WIN32) || defined(WIN64)
        // Allow first thing to be a double-slash, indicating a server path
        prevCharWasSlash = i > 0 && (path[i] == '/' || path[i] == '\\');
#else
        prevCharWasSlash = (path[i] == '/' || path[i] == '\\');
#endif
    }

    return newPath;
}

#endif

static std::string ScrubPath(std::string& path)
{
	std::string newPath;

	bool prevCharWasSlash = false;

	for (unsigned int i = 0; i < path.length(); i++) {
#if defined(WIN32) || defined(WIN64)
		if (path[i] == '/') {
			if (!prevCharWasSlash) {
				newPath += '\\';
			}
		}
#else
		if (path[i] == '\\') {
			if (!prevCharWasSlash) {
				newPath += '/';
			}
		}
#endif
		else {
			if (path[i] == '/' || path[i] == '\\') {
				if (!prevCharWasSlash) {
					newPath += path[i];
				}
			} else {
				newPath += path[i];
			}
		}

#if defined(WIN32) || defined(WIN64)
		// Allow first thing to be a double-slash, indicating a server path
		prevCharWasSlash = i > 0 && (path[i] == '/' || path[i] == '\\');
#else
		prevCharWasSlash = (path[i] == '/' || path[i] == '\\');
#endif
	}

	return newPath;
}

void ResourceLoader::SetResourceDirPath(const char *pPath, bool useAddDllDirectory)
{
    // Correct forward / backward slashes, add trailing slash if necessary.
    TRITON_STRING tmpPath = pPath;
    tmpPath += "/";
    TRITON_STRING scrubbedPath = ScrubPath(tmpPath);
    const char *path = scrubbedPath.c_str();

    if (resourcePath) TRITON_FREE(resourcePath);

    resourcePath = (char *)TRITON_MALLOC(strlen(path) + 1);

#ifdef _WIN32
#if _MSC_VER < 1400
    strncpy(resourcePath, path, strlen(path) + 1);
#else
    strcpy_s(resourcePath, strlen(path) + 1, path);
#endif
#else
    strncpy(resourcePath, path, strlen(path) + 1);
#endif

    // Extend the DLL search path
    TRITON_STRING dllPath = path;
#ifdef WIN64
    dllPath += "\\dll64\\";
#else
    dllPath += "\\dll\\";
#endif

#ifdef _WIN32
    // Convert to full path
    char fullPath[1024];
    _fullpath(fullPath, dllPath.c_str(), 1024);

#if (_MSC_VER > 1310)
    if (useAddDllDirectory) {
        // Use AddDllDirectory if possible
        typedef HANDLE DLL_DIRECTORY_COOKIE;
        typedef DLL_DIRECTORY_COOKIE (WINAPI *ADD_DLL_PROC)(PCWSTR);
        typedef BOOL (WINAPI *SET_DEFAULT_PROC)(DWORD);
        HMODULE kernel32 = GetModuleHandle(TEXT("kernel32.dll"));
        if (kernel32) {
            ADD_DLL_PROC lpfnAdllDllDirectory = (ADD_DLL_PROC)GetProcAddress(kernel32, "AddDllDirectory");
            SET_DEFAULT_PROC lpfnSetDefaultDllDirectories = (SET_DEFAULT_PROC)GetProcAddress(kernel32, "SetDefaultDllDirectories");

            if (lpfnSetDefaultDllDirectories) {
                lpfnSetDefaultDllDirectories(0x00001000); // LOAD_LIBRARY_SEARCH_DEFAULT_DIRS
            }

            if (lpfnAdllDllDirectory) {
                size_t origsize = strlen(fullPath) + 1;
                const size_t newsize = 1024;
                size_t convertedChars = 0;
                wchar_t wcstring[newsize];
                mbstowcs_s(&convertedChars, wcstring, origsize, fullPath, _TRUNCATE);
                if (((ADD_DLL_PROC)lpfnAdllDllDirectory)(wcstring) != 0) {
                    return;
                }
            }
        }
    }
#endif

    SetDllDirectoryA(fullPath);

#endif
}

bool ResourceLoader::LoadResource(const char *pathName, char *&data, unsigned int &dataLen, bool text)
{
    TRITON_STRING filePath(resourcePath);
    filePath += "/";
    filePath += pathName;

    data = 0;
    dataLen = 0;

    TRITON_STRING scrubbedPath = ScrubPath(filePath);

    FILE *f = 0;
#ifdef _WIN32
#if _MSC_VER >= 1400
    fopen_s(&f, scrubbedPath.c_str(), text ? "rt" : "rb");
#else
    f = fopen(scrubbedPath.c_str(), text ? "rt" : "rb");
#endif
#else
    f = fopen(scrubbedPath.c_str(), text ? "rt" : "rb");
#endif

    if (f) {

        fseek(f, 0, SEEK_END);
        dataLen = ftell(f);
        rewind(f);

        data = (char *)TRITON_MALLOC(text ? dataLen + 1 : dataLen);
        if (data) {
            dataLen = (unsigned int)fread(data, 1, dataLen, f);
            fclose(f);
            if (text) {
                data[dataLen] = '\0';
                dataLen++;
            }
            return true;
        }
    }

    Utils::DebugMsg("Failed to load the following resource:");
    Utils::DebugMsg(scrubbedPath.c_str());

    return false;
}

void ResourceLoader::FreeResource(char *data)
{
    if (data) {
        TRITON_FREE(data);
    }
}

std::string TRITONAPI Triton::ResourceLoader::GetFilePath( const char *fileName )
{
	std::string filePath(resourcePath);
	filePath += "/";
	filePath += fileName;

	std::string scrubbedPath = ScrubPath(filePath);

	return scrubbedPath;
}
