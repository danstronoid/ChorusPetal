/*
  ==============================================================================

    File: chorus_processor.h
    Author: Daniel Schwartz
    Description: A processor class to manage the controls and audio callback.

  ==============================================================================
*/

#pragma once

#ifndef CHORUS_PROCESSOR_H
#define CHORUS_PROCESSOR_H

#include "daisy_petal.h"

#include "dsp/chorus.h"
#include "dsp/math_helpers.h"
#include "dsp/smooth_value.h"

// Processor class to manage the controls and the audio callback.
class ChorusProcessor
{
public:
    // Processor needs a reference to the Daisy hardware object.
    ChorusProcessor(daisy::DaisyPetal &hw);
    ~ChorusProcessor() {}

    // Initialize the state of the processor.
    // Must be called after hardware has been initialized.
    void Init();

    // Process all analog and digital controls.
    void ProcessControls();

    // The audio callback function to process incoming audio
    // Note: this needs to be passed to a C-style library callback
    // It will need some kind of wrapper
    void AudioCallback(daisy::AudioHandle::InputBuffer in,
                       daisy::AudioHandle::OutputBuffer out,
                       size_t size);

    // Update the leds.
    void UpdateLeds();

private:
    // Reference to the daisy hardware.
    daisy::DaisyPetal &hw_;

    // The chorus audio processor.
    dingus_dsp::Chorus chorus_;

    // Parameters

    // Delay time of chorus.
    dingus_dsp::SmoothValue<float> delay_time_{};

    // Depth of chorus.
    dingus_dsp::SmoothValue<float> depth_{};

    // Engage: effect is on if true.
    bool engage_{true};

    // Mix of wet/dry signal.
    float mix_{0.5f};

    // Master output level.
    float level_{1.f};

    // Number of active voices.
    size_t encoder_pos_{1};
};

#endif