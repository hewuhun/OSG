// Copyright (c) 2004-2013  Sundog Software, LLC All rights reserved worldwide.

#ifndef DYNAMICLIBRARY_H
#define DYNAMICLIBRARY_H

#include "TritonCommon.h"
#include <string>

namespace Triton
{

class DynamicLibrary
{
private:
    typedef void (DynamicLibrary::*bool_type)() const;
    void this_type_does_not_support_comparisons() const {}

public:

    DynamicLibrary();
    ~DynamicLibrary();

    typedef void* Handle;
    typedef void* ProcAddress;

    // safe bool, http://en.wikibooks.org/wiki/More_C%2B%2B_Idioms/Safe_bool
    operator bool_type() const {
        return Valid() ? &DynamicLibrary::this_type_does_not_support_comparisons : 0;
    }

    bool Valid() const {
        return handle!=0;
    }

    bool Open(const TRITON_STRING& libraryName);

    void Close();

    Handle      GetHandle() const {
        return handle;
    }

    ProcAddress GetProcAddress(const TRITON_STRING& procName) const;

    TRITON_STRING GetErrorMessage() const;

protected:

    Handle     handle;
    int refCount;

};

}

#endif
