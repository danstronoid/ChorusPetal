/*
  ==============================================================================

    File: chorus_engine.h
    Author: Daniel Schwartz
    Description: A stereo chorus engine combining two modulated delay lines.

  ==============================================================================
*/

#pragma once
#ifndef CHORUS_H
#define CHORUS_H

#include "multitap_delay.h"

#include <array>

namespace dingus_dsp
{

    // A stereo chorus engine combining two modulated delay lines.
    class Chorus
    {
    public:
        Chorus() {}

        ~Chorus() {}

        // Initialize the state of the chorus.
        void Init(float sample_rate);

        // Process a sample for one channel.
        float Process(float input, size_t channel);

        // Process a stereo output.
        void Process(float* left, float* right);

        // Reset the state of the chorus.
        void Reset();

        // Returns the maximum delay time in seconds.
        float GetMaxDelay();

        // Returns the maximum number of voices.
        size_t GetMaxNumVoices();

        // Set the delay time given a 0 to 1 parameter.
        // Scales the time to between 5ms to 25ms
        // The right delay line has a 2ms offset.
        void SetDelayTime(float delay_time);

        // Set the rate of modulation.
        void SetRate(float rate);

        // Set the depth of modulation.
        void SetDepth(float depth);

        // Set the amount of feedback.
        void SetFeedbackLevel(float feedback_lvl);

        // Set the number of voices (taps).
        void SetNumVoices(size_t voices);

    private:
        // Stereo multitap delay lines.
        std::array<MultitapDelay<8, 48000>, 2> delaylines_;

        // The audio sample rate.
        float sample_rate_{};
    };

}

#endif