// Copyright (c) 2010-2011 Sundog Software LLC, All rights reserved worldwide.

#include <FeTriton/MemAlloc.h>

#if defined(WIN32) || defined(WIN64)
#include <windows.h>
#else
#include <stdlib.h>
#endif

using namespace Triton;

//Allocator Allocator::sDefaultAllocator;
//Allocator *Allocator::sAllocator = &sDefaultAllocator;

Allocator::Allocator()
{
#if defined(WIN32) || defined(WIN64)
    heap = GetProcessHeap();
#endif
}

Allocator::~Allocator()
{

}

//static size_t total = 0;

void *Allocator::alloc(size_t bytes)
{
#if defined(WIN32) || defined(WIN64)
    //total += bytes;
    //printf("%d\n", total);
    return HeapAlloc(heap, 0, bytes);
#else
    return malloc(bytes);
#endif
}

void Allocator::dealloc(void *p)
{
#if defined(WIN32) || defined(WIN64)
    //total -= HeapSize(heap, 0, p);
    HeapFree(heap, 0, p);
#else
    free(p);
#endif
}

Allocator * TRITONAPI Triton::Allocator::GetAllocator()
{
	static Allocator sDefaultAllocator;
	static Allocator *sAllocator = &sDefaultAllocator;
	return sAllocator;
}

void TRITONAPI Triton::Allocator::SetAllocator( Allocator *a )
{
	if( a )
	{
		Allocator* ac = GetAllocator();
		ac = a;
	}
}
