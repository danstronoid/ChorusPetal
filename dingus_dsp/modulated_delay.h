/*
  ==============================================================================

    File: modulated_delay.h
    Author: Daniel Schwartz
    Description: A modulated delay line that combines a simple delay line 
    with a modulation source.

  ==============================================================================
*/

#pragma once
#ifndef MODULATED_DELAY_H
#define MODULATED_DELAY_H

#include "delayline.h"
#include "oscillator.h"
#include "dingus_math.h"

namespace dingus_dsp 
{

// A modulated delay line that combines a simple delay line with a modulation source.
class ModulatedDelay
{
    public:
        ModulatedDelay() {}
        ~ModulatedDelay() {}

        // This constant determines the size of the internal delay buffer.
        // Sample rate 48kHz: a buffer of size 4800 has a max delay of 100ms.
        static constexpr size_t MAX_DELAY {4800};

        // Sets the initial state of the modulated delay given the sample rate.
        void Init(float sample_rate);

        // Writes the input sample to the delay buffer and returns the delayed output.
        float Process(float input);

        // Clears the buffer and resets the modulation.
        void Reset();

        // Sets the delay time in seconds.
        void SetDelayTime(float delay_time) {
            delay_time = delay_time * sample_rate_;

            // Make sure the delay time won't exceed the buffer size.
            delay_time_ = Clamp<float>(delay_time, 0.f, MAX_DELAY);
        }

        // Sets the rate of the modulation.
        void SetRate(float rate) {
            osc_.SetFrequency(rate);
        }

        // Sets the depth of the modulation.
        void SetDepth(float depth) {
            osc_.SetAmplitude(Clamp<float>(depth, -1.f, 1.f));
        }

    private:
        // The delay line itself.
        // Note: buffer size is increased by a factor of 2
        // This allows for a modulated time that is twice MAX_DELAY
        DelayLine<float, 2 * MAX_DELAY> delay_line_;

        // The modulation oscillator.
        Oscillator osc_;        

        // The fractional delay time in samples.
        float delay_time_ {};

        // The audio sample rate.
        float sample_rate_ {};
};

}

#endif