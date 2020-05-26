// Copyright (c) 2011 Sundog Software LLC. All rights reserved worldwide.

#ifndef TRITON_COMPLEX_NUMBER_H
#define TRITON_COMPLEX_NUMBER_H

/** \file ComplexNumber.h
   \brief A complex number class with basic operators.
 */

#include <FeTriton/TritonCommon.h>
#include <math.h>

namespace Triton
{
/** A complex number class with basic operators. */
class ComplexNumber : public MemObject
{
public:
    /** Constructor; clears to 0 */
    ComplexNumber() : real(0), img(0) {}

    /** Constructor.
    \param r The real component to initialize the complex number with.
    \param i The imaginary component.
    */
    ComplexNumber(float r, float i) : real(r), img(i) {}

    /** Adds a complex number to this number and returns the result.
    \param c The complex number to add
    \return The sum of this ComplexNumber and the one passed in.
    */
    ComplexNumber operator + (const ComplexNumber& c) const {
        ComplexNumber tmp;
        tmp.real = real + c.real;
        tmp.img = img + c.img;
        return tmp;
    }

    /** Subtracts a complex number from this number and returns the result.
    \param c The complex number to subtract
    \return The difference between this ComplexNumber and the one pased in.
    */
    ComplexNumber operator - (const ComplexNumber& c) const {
        ComplexNumber tmp;
        tmp.real = real - c.real;
        tmp.img = img - c.img;
        return tmp;
    }

    /** Multiplies a complex number with this number and returns the result.
    \param c The complex number to multiply
    \return The product of this ComplexNumber and the one passed in.
    */
    ComplexNumber operator * (const ComplexNumber& c) const {
        ComplexNumber tmp;
        tmp.real = (real * c.real) - (img * c.img);
        tmp.img = (real * c.img) + (img * c.real);
        return tmp;
    }

    /** Divides this complex number by the one passed in and returns the result.
    \param c The complex number to divide by
    \return The result of the division
    */
    ComplexNumber operator / (const ComplexNumber& c) const {
        float div = (c.real * c.real) + (c.img * c.img);
        ComplexNumber tmp;
        tmp.real = (real * c.real) + (img * c.img);
        tmp.real /= div;
        tmp.img = (img * c.real) - (real * c.img);
        tmp.img /= div;
        return tmp;
    }

    /** Returns the complex conjugate of this ComplexNumber. */
    ComplexNumber getConjugate() const {
        ComplexNumber tmp;
        tmp.real = real;
        tmp.img = -img;
        return tmp;
    }

    float real, img;
};
}

#endif
