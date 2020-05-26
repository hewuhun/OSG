// Copyright (c) 2012 Sundog Software LLC. All rights reserved worldwide.

#ifndef TRITON_MUTEX_H
#define TRITON_MUTEX_H

#if (defined(WIN32) || defined(WIN64))
#include <Windows.h>
#define WINDOWS_THREADS
#else
#include <pthread.h>
#endif

namespace Triton
{
class Mutex
{
#ifdef WINDOWS_THREADS
public:
    Mutex() {
        mutex = NULL;
        mutex = CreateMutex(NULL, FALSE, NULL);
    }

    ~Mutex() {
        if (mutex) CloseHandle(mutex);
    }

    void Lock() {
        if (mutex) WaitForSingleObject(mutex, INFINITE);
    }

    void Unlock() {
        if (mutex) ReleaseMutex(mutex);
    }

private:
    HANDLE mutex;

#else
public:
    Mutex() {
//#ifndef __APPLE__
//        mutex = PTHREAD_MUTEX_INITIALIZER;
//#endif
        hasMutex = pthread_mutex_init(&mutex, NULL) == 0;
    }

    ~Mutex() {
        if (hasMutex) pthread_mutex_destroy(&mutex);
    }

    void Lock() {
        if (hasMutex) pthread_mutex_lock(&mutex);
    }

    void Unlock() {
        if (hasMutex) pthread_mutex_unlock(&mutex);
    }

private:
    pthread_mutex_t mutex;
    bool hasMutex;
#endif
};
}

#endif
