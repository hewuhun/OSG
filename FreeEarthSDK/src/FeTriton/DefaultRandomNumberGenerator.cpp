// Copyright (c) 2012 Sundog Software LLC. All rights reserved worldwide.

#include <FeTriton/DefaultRandomNumberGenerator.h>

using namespace Triton;

double DefaultRandomNumberGenerator::GetRandomDouble(double start, double end) const
{
    if (end < start) {
        double tmp = end;
        end = start;
        start = tmp;
    }

    double range = end - start;
    double rnd = (double)rand() / (double)RAND_MAX;

    return start + rnd * range;
}

int DefaultRandomNumberGenerator::GetRandomInt(int start, int end) const
{
    if (end < start) {
        int tmp = end;
        end = start;
        start = tmp;
    }

    int range = end - start;

    return start + rand() % range;
}

void DefaultRandomNumberGenerator::SetRandomSeed(unsigned int seed)
{
    srand(seed);
}
