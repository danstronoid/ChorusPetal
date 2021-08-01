/*
  ==============================================================================

    File: smooth_value.h
    Author: Daniel Schwartz
    Description: SmoothValue will create a smooth linear transition from one 
    value to the next when it is set to a new value.

  ==============================================================================
*/

#pragma once
#ifndef SMOOTH_VALUE_H
#define SMOOTH_VALUE_H

#include "math_helpers.h"

namespace dingus_dsp
{

    // SmoothValue will create a smooth linear transition from one value to the next.
    template <typename FloatType>
    class SmoothValue
    {
    public:
        // Sets the inital value to 0.
        SmoothValue()
        {
            curr_ = 0;
            target_ = 0;
        }

        // Takes the initial value to use.
        SmoothValue(FloatType value)
        {
            curr_ = value;
            target_ = value;
        }

        ~SmoothValue() {}

        // Given the duration in seconds and audio sample rate, sets the smoothing time.
        void Init(float duration, float sample_rate)
        {
            sample_rate_ = sample_rate;
            duration_ = duration * sample_rate;
            counter_ = 0;
        }

        // Increment the value by one sample.
        void NextValue()
        {
            if (counter_ > 0)
            {
                curr_ += delta_;
                --counter_;
            }
        }

        // Get the current value and then increment it.
        FloatType GetNextValue()
        {
            FloatType prev = curr_;
            NextValue();
            return prev;
        }

        // Get the current value only.  Does not increment.
        FloatType GetValue()
        {
            return curr_;
        }

        // Get the current target value.  Does not increment.
        FloatType GetTarget()
        {
            return target_;
        }

        // Returns true if the current value has not reached the target.
        bool IsActive()
        {
            return (counter_ > 0);
        }

        // Set the target value.  Will not update if the old target
        // is within a difference of epsilon to the new target.
        void SetTargetValue(FloatType target, float epsilon = 0.0001f)
        {
            // Latch the target so it only updates if changed.
            if (!CompareFloat(target, target_, epsilon))
            {
                target_ = target;
                UpdateDelta();
            }
        }

        // Sets both the current and target value immediately with no transition.
        void SetValue(FloatType value)
        {
            curr_ = value;
            target_ = value;
            UpdateDelta();
        }

    private:
        // The current value.
        FloatType curr_{};

        // The target value.
        FloatType target_{};

        // The amount to increment.
        FloatType delta_{};

        // The duration in samples of the transition.
        float duration_{};

        // A counter for the number of times to increment.
        int counter_{};

        // The audio sample rate.
        float sample_rate_{};

        // Update the delta based on the difference between the current and target.
        // Resets the counter to the full duration.
        void UpdateDelta()
        {
            delta_ = (target_ - curr_) / duration_;
            counter_ = static_cast<int>(duration_);
        }
    };

}

#endif