// Copyright (c) 2012 Sundog Software LLC. All rights reserved worldwide.

#ifndef TRITON_DEFAULT_RANDOM_NUMBER_GENERATOR_H
#define TRITON_DEFAULT_RANDOM_NUMBER_GENERATOR_H

/** \file DefaultRandomNumberGenerator.h
    \brief An implementation of RandomNumberGenerator using stdlib rand().
*/
#include <FeTriton/RandomNumberGenerator.h>

#pragma pack(push)
#pragma pack(8)

namespace Triton
{
/** A default implementation of RandomNumberGenerator that uses the stdlib rand() function. */
class DefaultRandomNumberGenerator : public RandomNumberGenerator
{
public:
    DefaultRandomNumberGenerator() {}
    virtual ~DefaultRandomNumberGenerator() {}

    /** Return an evenly distributed random double-precision number within a given range.
        \param start The lowest value in the range
        \param end The highest value in the range
        \return An evenly distributed random number within the range.
    */
    virtual double TRITONAPI GetRandomDouble(double start, double end) const;

    /** Return an evenly distributed random integer within a given range.
        \param start The lowest value in the range
        \param end The highest value in the range
        \return An evenly distributed random number within the range.
    */
    virtual int TRITONAPI GetRandomInt(int start, int end) const;

    /** Seeds the random number generator with a given value, to ensure consistent
        results.
        \param seed A value used to seed the random number generator's sequence
                    of psuedo-random numbers.
    */
    virtual void TRITONAPI SetRandomSeed(unsigned int seed);
};
}

#pragma pack(pop)

#endif