/*
  ==============================================================================

    File: oscillator.h
    Author: Daniel Schwartz
    Description: A basic oscillator implemented using per sample calculations.

  ==============================================================================
*/

#pragma once
#ifndef DINGUS_OSC_H
#define DINGUS_OSC_H

#include "dingus_math.h"

namespace dingus_dsp 
{

// The different oscillator waveforms available.
enum class OscType {
    SINE,
    TRI,
    SAW,
    PULSE,
    MAX
};

// A basic oscillator implemented using per sample calculations.
class Oscillator 
{
    public:
        Oscillator() {}
        ~Oscillator() {}

        // Initialize the oscillator for playback given the audio rate.
        void Init(float sample_rate);

        // Process a single sample.
        float Process();

        // Reset the phase of the oscillator.
        void Reset() {
            phase_ = 0; 
        }

        // Set the frequency of the oscillator.
        void SetFrequency(float freq) {
            freq_ = freq;
            UpdateDelta();
        }

        // Set the amplitude of the oscillator.
        void SetAmplitude(float amp) {
            amp_ = amp;
        }

        // Set the waveform type of the oscillator.
        void SetType(OscType type) {
            osc_type_ = (type < OscType::MAX) ? type : OscType::SINE;
        }

    private:
        // The type of oscillator waveform.
        OscType osc_type_ {OscType::SINE};

        // The oscillator amplitude.
        float amp_ {1};

        // The oscillator frequency.
        float freq_ {220};

        // The current oscillator phase.
        float phase_ {};

        // The delta to increment the phase.
        float delta_ {};

        // The audio sample rate.
        float sample_rate_ {};

        // Calculate the delta amount to increment the phase.
        void UpdateDelta();
};


}

#endif