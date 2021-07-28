#pragma once
#ifndef DINGUS_MATH_H
#define DINGUS_MATH_H

#include <cmath>
#include <random>

namespace dingus_dsp
{

// Generally useful constant values.
template<typename FloatType>
struct MathConstants
{
    static constexpr FloatType PI = static_cast<FloatType>(3.1415927410125732421875f);
    static constexpr FloatType TWOPI = static_cast<FloatType>(2 * 3.1415927410125732421875f);
};

// Given a value and limits lo and hi, clamps the value between the limits.
template <typename T>
inline T Clamp(T value, T lo, T hi) {
    if (value > hi) {
        return hi;
    } else if (value < lo) {
        return lo;
    } else {
        return value;
    }
}

// Applies exponential moving average algorithm given: 
// current value Y_t, previous ema value S_t-1, and a coefficient between 0-1
// Acts as a first order filter to smooth value changes.
template <typename T>
inline T ExpMovingAvg(T value, T prev_ema, float coef) {
    return coef * value + (1 - coef) * prev_ema;
}

// Simple way to compare two floats.
template <typename FloatType>
inline bool CompareFloat(FloatType a, FloatType b, float epsilon) {
    return fabs(a - b) < epsilon;
}

}

#endif