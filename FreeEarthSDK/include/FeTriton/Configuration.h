﻿// Copyright (c) 2004-2015  Sundog Software, LLC. All rights reserved worldwide.

/**
    \file Configuration.h
    \brief A class that reads entries from a config file.
 */

#ifndef TRITON_CONFIGURATION_H
#define TRITON_CONFIGURATION_H

#include <string.h>

#include <FeTriton/MemAlloc.h>
#include <FeTriton/Export.h>

#if defined(WIN32) || defined(WIN64)
#pragma warning(disable: 4786)
#endif

#if ((defined(WIN32) || defined(WIN64)))

#if (_MSC_VER > 1310)

#if (_MSC_VER >= 1900)
#include <unordered_map>
#else
#include <hash_map>
#endif
#else
#include <map>
#endif
#else
#include <map>
#endif

namespace Triton
{

class ResourceLoader;

struct strltpred {
    bool operator()(const char* a, const char* b) const {
        return strcmp(a, b) < 0;
    }
};

class ConfigEntry : public MemObject
{
public:
    ConfigEntry(const char *value);

    ~ConfigEntry();

    char *stringVal;
    int intVal;
    float floatVal;
    double doubleVal;
    bool boolVal;
};

/** A class that reads entries from a configuration file. It expects that the
   config file is a plain text file where entries are of the form
   key = value
   with one entry per line.
   The # character is used to precede comment lines in the file. Empty lines
   are ignored.
 */
class FETRITON_EXPORT Configuration : public MemObject
{
public:
    /** Calls Load() for the file Triton.config in the current resource
       path. Must be called after the resource path has been set in the
       Environment object. */
    static bool Initialize(ResourceLoader *rl);

    /** Called by Environment instances on destruction or reloading.
       Each call dereferences the Configuration. Last call (reference counter == 0)
       clears configuration map and associated memory by invoking Clear(). */
    static void Destroy();

    /** Releases the underlying configuration map and all its associated memory. */
    static void Clear();

    /** Loads a config file given a full pathname, and parses its entries into
       a static STL map for fast lookups going forward. */
    static bool Load(const TRITON_STRING& path, ResourceLoader *rl);

    /** Returns a ConfigEntry object for the given key (or null if not found.) */
    static ConfigEntry *GetConfigEntry(const char *key);

    /** Looks up a value for a given key, and interprets it as a string. Returns
       a const pointer to the string, or null if they key was not found. */
    static bool GetStringValue(const char *key, const char *& val);

    /** Looks up a value for a given key, and converts it into an integer. Returns
       false if the key was not found. */
    static bool GetIntValue(const char * key, int& val);

    /** Looks up a value for a given key, and converts it to a boolean value. If
       the config entry is "yes" or "true", then the "val" parameter will be set to true.
       Otherwise, it will be set to false. Returns false if the key was not found. */
    static bool GetBoolValue(const char * key, bool& val);

    /** Looks up a value for a given key, and converts it into a double-precision
       number. Returns false if the key was not found. */
    static bool GetDoubleValue(const char * key, double& val);

    /** Looks up a value for a given key, and converts it into a single-precision
       number. Returns false if the key was not found. */
    static bool GetFloatValue(const char * key, float& val);

    /** Overwrites a given config key with the specified value. */
    static void Set(const char * key, const char * val);

private:

#if (defined(WIN32) || defined(WIN64))

#if (_MSC_VER > 1310)
#if (_MSC_VER >= 1900)

    template <class _Tp>
    struct my_equal_to : public std::binary_function<_Tp, _Tp, bool> {
        bool operator()(const _Tp& __x, const _Tp& __y) const {
            return strcmp(__x, __y) == 0;
        }
    };

    struct Hash_Func {
        //BKDR hash algorithm
        int operator()(const char * str)const {
            int seed = 131;//31  131 1313 13131131313 etc//
            int hash = 0;
            while (*str) {
                hash = (hash * seed) + (*str);
                str++;
            }

            return hash & (0x7FFFFFFF);
        }
    };

    typedef std::unordered_map<const char *, ConfigEntry *, Hash_Func, my_equal_to<const char*>,
            TritonAllocator< std::pair<const char *, ConfigEntry *> > > Settings;
#else // > 1900
    typedef stdext::hash_map<const char *, ConfigEntry *, stdext::hash_compare<const char *, strltpred>,
            TritonAllocator< std::pair<const char *, ConfigEntry *> > > Settings;
#endif // > 1900

#else // >1310
    typedef std::map<const char *, ConfigEntry *, strltpred> Settings;
#endif // > 1310
#else // windows
    typedef std::map<const char *, ConfigEntry *, strltpred> Settings;
#endif // windows

    static Settings settings;

    static int refCount;
};
}

#endif

