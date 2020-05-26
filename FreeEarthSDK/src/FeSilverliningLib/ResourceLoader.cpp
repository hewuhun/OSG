// Copyright (c) 2010 Sundog Software, LLC. All rights reserved worldwide.

#include <FeSilverliningLib/MemAlloc.h>
#include <FeSilverliningLib/SilverLiningTypes.h>
#include <FeSilverliningLib/ResourceLoader.h>
#include <stdio.h>
#include <string.h>
#include <string>
#include <stdlib.h>
#include <algorithm>

#if (!defined(WIN32) && !defined(WIN64))
#include <dirent.h>
#include <string.h>
#else
#include <windows.h>
#endif

using namespace SilverLining;

ResourceLoader::~ResourceLoader()
{
    if (resourcePath) {
        SL_FREE(resourcePath);
        resourcePath = 0;
    }
}

void ResourceLoader::SetResourceDirPath(const char *path)
{
    if (resourcePath) SL_FREE(resourcePath);

    resourcePath = (char *)SL_MALLOC(strlen(path) + 1);
    strcpy(resourcePath, path);
}

// Enforce proper use of forward or back slashes depending on the OS, and strip duplicate slashes.
static SL_STRING ScrubPath(SL_STRING& path)
{
    SL_STRING newPath;

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

        prevCharWasSlash = (path[i] == '/' || path[i] == '\\');
    }

    return newPath;
}

bool ResourceLoader::LoadResource(const char *pathName, char *&data, unsigned int &dataLen, bool text)
{
    SL_STRING filePath(resourcePath);
    filePath += "/";
    filePath += pathName;

    data = 0;
    dataLen = 0;

    SL_STRING scrubbedPath = ScrubPath(filePath);


    FILE *f = fopen(scrubbedPath.c_str(), text ? "rt" : "rb");
    if (f) {
        fseek(f, 0, SEEK_END);
        dataLen = ftell(f);
        rewind(f);

        data = (char *)SL_MALLOC(text ? dataLen + 1 : dataLen);
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

    return false;
}

void ResourceLoader::FreeResource(char *data)
{
    if (data) {
        SL_FREE(data);
    }
}

bool ResourceLoader::GetFilesInDirectory(const char *pathName, SL_VECTOR(SL_STRING)& dirContents)
{
    SL_STRING dirPath(resourcePath);
    dirPath += "/";
    dirPath += pathName;
    dirPath += "/";

    dirContents.clear();

    SL_STRING scrubbedDir = ScrubPath(dirPath);

#if defined(WIN32) || defined(WIN64)
    // Windows-specific code below.
    WIN32_FIND_DATAA ffd;
    HANDLE hFind = INVALID_HANDLE_VALUE;

    SL_STRING dirSearch = scrubbedDir + "*.*";
    hFind = FindFirstFileA(dirSearch.c_str(), &ffd);
    if (hFind != INVALID_HANDLE_VALUE) {
        do {
            if (!(ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
                dirContents.push_back(SL_STRING(ffd.cFileName));
            }
        } while (FindNextFileA(hFind, &ffd) != 0);

        FindClose(hFind);

        std::sort(dirContents.begin(), dirContents.end());

        return true;
    } else {
        return false;
    }


#else

    DIR *dir = opendir(scrubbedDir.c_str());
    if (dir) {
        struct dirent *entry;
        do {
            entry = readdir(dir);
            if (entry) {
                if (entry->d_type != DT_DIR) {
                    dirContents.push_back(SL_STRING(entry->d_name));
                }
            }
        } while (entry);
        closedir(dir);

        std::sort(dirContents.begin(), dirContents.end());

        return true;
    } else {
        return false;
    }
#endif
}
