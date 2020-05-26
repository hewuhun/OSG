// Copyright (c) 2009 Sundog Software, LLC. All rights reserved worldwide.

/** \file RandomNumberGenerator.h
   \brief A virtual base class for overriding SilverLining's random number generator in your application.
 */

#ifndef RANDOM_NUMBER_GENERATOR_H
#define RANDOM_NUMBER_GENERATOR_H

#include <FeSilverliningLib/MemAlloc.h>

#pragma pack(push)
#pragma pack(8)

namespace SilverLining {
/** Extend this class in order to use your own random number generator with SilverLining. Pass
    your implementation into Atmosphere::SetRandomNumberGenerator(). */
class RandomNumberGenerator
{
public:
/** Virtual destructor. */
    virtual ~RandomNumberGenerator() {
    };

/** Return a uniformly distributed random float between 0.0 and 1.0 */
    virtual float SILVERLINING_API UniformRandomFloat() const = 0;

/** Return a uniformly distributed random double between 0.0 and 1.0 */
    virtual double SILVERLINING_API UniformRandomDouble() const = 0;

/** Return a uniformly distributed random integer between the integers specified (inclusive.) */
    virtual unsigned int SILVERLINING_API UniformRandomIntRange(unsigned int start, unsigned int end) const = 0;
};
}

#pragma pack(pop)

#endif
