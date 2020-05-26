// Copyright (c) 2009-2014 Sundog Software, LLC. All rights reserved worldwide.

#include <FeSilverliningLib/DefaultRandomNumberGenerator.h>
#include <stdlib.h>

using namespace SilverLining;

float DefaultRandomNumberGenerator::UniformRandomFloat() const
{
#ifdef MAC
    return (float)(random() & 0xFFFFFF) / (float)0xFFFFFF;
#else
    double drand = (double)rand() / (double)RAND_MAX;
    return (float)drand;
#endif
}

double DefaultRandomNumberGenerator::UniformRandomDouble() const
{
#ifdef MAC
    return (double)(random() & 0xFFFFFF) / (double)0xFFFFFF;
#else
    return (double)rand() / (double)RAND_MAX;
#endif
}

unsigned int DefaultRandomNumberGenerator::UniformRandomIntRange(unsigned int start, unsigned int end) const
{
    if (start > end) {
        int tmp = end;
        end = start;
        start = tmp;
    }

    int range = end - start;

#ifdef MAC
    return (random() % (range + 1)) + start;
#else
    return (rand() % (range + 1)) + start;
#endif
}
