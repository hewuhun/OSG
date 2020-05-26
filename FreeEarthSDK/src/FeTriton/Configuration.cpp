// Copyright (c) 2004-2014  Sundog Software, LLC. All rights reserved worldwide.

#include <FeTriton/TritonCommon.h>
#include <FeTriton/ResourceLoader.h>
#include <FeTriton/Configuration.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sstream>
#include <locale>

using namespace Triton;
using namespace std;

Configuration::Settings Configuration::settings;
int Configuration::refCount = 0;

#define MAX_LINE 1024

static double LocaleSafeAtoF(const char *str)
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

ConfigEntry::ConfigEntry(const char *value)
{
    stringVal = 0;

    if (value) {
        unsigned int len = (unsigned int)strlen(value) + 1;
        stringVal = TRITON_NEW char[len];

#if (_MSC_VER > 1310)
        strcpy_s(stringVal, len, value);
#else
        strcpy(stringVal, value);
#endif
        intVal = atoi(value);
        floatVal = (float)LocaleSafeAtoF(value);
        doubleVal = LocaleSafeAtoF(value);
        boolVal = strcmp(value, "yes") == 0 || strcmp(value, "true") == 0;
    }
}

ConfigEntry::~ConfigEntry()
{
    if (stringVal) TRITON_DELETE[] stringVal;
}

bool Configuration::Initialize(ResourceLoader *rl)
{
    if (!rl) return false;

    refCount++;

    if (refCount > 1) return true;

    if (Configuration::Load("Triton.config", rl)) {
        return true;
    } else {
        Utils::DebugMsg("Could not load Triton.config from the given resources directory.");
        return false;
    }
}

void Configuration::Destroy()
{
    refCount--;

    if (refCount <= 0) {
        Clear();
    }
}

void Configuration::Clear()
{
    for (Settings::iterator it = settings.begin(); it != settings.end(); ++it) {
        // Delete the config entry object
        if (it->first) {
            TRITON_DELETE[] (char *)it->first;
        }

        if (it->second) {
            TRITON_DELETE it->second;
        }
    }

    settings.clear();
}

bool Configuration::Load(const TRITON_STRING& path, ResourceLoader *rl)
{
    if (!rl) return false;

    char *data;
    unsigned int len;

    char line[MAX_LINE], scratch[MAX_LINE];

    Clear();

    if (rl->LoadResource(path.c_str(), data, len, true)) {
        std::stringstream *stream = TRITON_NEW stringstream(string(data));

        while (stream->good()) {
            stream->getline(line, MAX_LINE);

            // Remove whitespace (anything outside of ascii 33-125)
            char *src = line;
            char *dst = scratch;
            char c;
            do {
                c = *src;

                // Preserve null terminator too
                if ((c > 32 && c < 126) || c == 0) {
                    *dst++ = c;
                }

                src++;

            } while (c);

            // Toss out blanks and comments
            if (strlen(scratch) == 0) {
                continue;
            }

            if (scratch[0] == '#') {
                continue;
            }

            // Appears to be a valid entry. Tokenize on the =
            char seps[] = "=\0";

#if (_MSC_VER > 1310)
            char *context = 0;
            char *skey = strtok_s(scratch, seps, &context);
            if (skey) {
                char *tok = strtok_s(NULL, seps, &context);
                if (tok)
                    Set(skey, tok);
            }
#else
            char *skey = strtok(scratch, seps);
            if (skey) {
                char * tok = strtok(NULL, seps);
                if (tok)
                    Set(skey,tok);
            }
#endif
        }

        TRITON_DELETE stream;

        rl->FreeResource(data);

        return true;
    }

    return false;
}

void Configuration::Set(const char * key, const char * val)
{
    if (!key || !val) return;

    ConfigEntry *entry = TRITON_NEW ConfigEntry(val);

    Settings::iterator it = settings.find(key);

    if (it != settings.end()) {
        // Delete the config entry object
        if (it->second) {
            TRITON_DELETE it->second;
        }

        it->second = entry;
    } else {
        unsigned int len = (unsigned int)strlen(key) + 1;
        char * keyCopy = TRITON_NEW char[len];

#if (_MSC_VER > 1310)
        strcpy_s(keyCopy, len, key);
#else
        strcpy(keyCopy, key);
#endif

        settings[keyCopy] = entry;
    }
}

ConfigEntry * Configuration::GetConfigEntry(const char *key)
{
    if (settings.empty()) {
        return NULL;
    }

    if (key) {
        Settings::iterator it = settings.find(key);

        if (it != settings.end()) {
            return it->second;
        }

        //Utils::DebugMsg("Failed to find the following setting in Triton.config:");
        //Utils::DebugMsg(key);
    } else {
        Utils::DebugMsg("Null configuration key received in Configuration::GetConfigEntry");
    }

    return NULL;
}

bool Configuration::GetStringValue(const char *key, const char *& val)
{
    ConfigEntry *ce = GetConfigEntry(key);

    if (ce) {
        val = ce->stringVal;
        return true;
    }

    return false;
}

bool Configuration::GetDoubleValue(const char * key, double& val)
{
    ConfigEntry *ce = GetConfigEntry(key);

    if (ce) {
        val = ce->doubleVal;
        return true;
    }

    return false;
}

bool Configuration::GetFloatValue(const char * key, float& val)
{
    ConfigEntry *ce = GetConfigEntry(key);

    if (ce) {
        val = ce->floatVal;
        return true;
    }

    return false;
}

bool Configuration::GetBoolValue(const char * key, bool& val)
{
    ConfigEntry *ce = GetConfigEntry(key);

    if (ce) {
        val = ce->boolVal;
        return true;
    }

    return false;
}

bool Configuration::GetIntValue(const char * key, int& val)
{
    ConfigEntry *ce = GetConfigEntry(key);

    if (ce) {
        val = ce->intVal;
        return true;
    }

    return false;
}

