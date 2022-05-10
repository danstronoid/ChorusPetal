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
#include "../../DingusDSP/source/dingus_dsp.h"

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

    // Stereo low shelf cut
    std::array<dingus_dsp::BiquadFilter, 2> cut_filters_;

    // Stereo low shelf boost
    std::array<dingus_dsp::BiquadFilter, 2> boost_filters_;

    // Stereo high pass filters
    std::array<dingus_dsp::BiquadFilter, 2> hipass_filters_;

    // Stereo lowpass tone filter
    std::array<dingus_dsp::OnePoleFilter, 2> tone_filters_;

    // Wet/dry mixer
    dingus_dsp::Mixer mixer_;

    // Parameters

    // Maximum boost/cut fitler gain
    static constexpr float FILTER_GAIN = 3.0f;

    // Fixed boost/cut filter cutoff
    static constexpr float FILTER_CUTOFF = 220.0f;

    // Fixed boost/cut filter Q
    static constexpr float FILTER_Q = 0.7071f;

    // Threshold of hardclipping
    static constexpr float CLIP_THRESH = 1.2f;

    // Delay time of chorus.
    dingus_dsp::SmoothValue<float> delay_time_{};

    // Depth of chorus.
    dingus_dsp::SmoothValue<float> depth_{};

    // Previous system time. Used for tap tempo.
    uint32_t prev_time_{};

    // Use for led delay indicator
    float delay_counter_{};

    // Store delay knob value so tap can bypass knob
    // Set this negative to force an initialization
    float delay_knob_{ -1.f };

    // Engage: effect is on if true.
    bool engage_{true};

    // True if tri chorus mode is on
    bool tri_mode_{false};

    // True if high pass filter is on
    bool hipass_engage_{false};

    // Mix of wet/dry signal.
    float mix_{0.5f};

    // Scale the filter gain based on mix
    float mix_scale_{};

    // Master output level.
    float level_{1.f};

    // Lofi control
    float lofi_{};
};

#endif