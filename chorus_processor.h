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
#include "terrarium.h"
#include "../../BarkLib/source/bark.h"

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

    // Switches should be processed at a slower rate to avoid extra triggers
    void ProcessSwitches();

    // Knobs should be processed at audio rate
    void ProcessKnobs();

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

    // Bypass indicator led
    daisy::Led led1_;

    // Tempo indicator led
    daisy::Led led2_;

    // The chorus audio processor.
    bark::Chorus chorus_;

    // Stereo low shelf cut
    std::array<bark::BiquadFilter, 2> cut_filters_;

    // Stereo low shelf boost
    std::array<bark::BiquadFilter, 2> boost_filters_;

    // Stereo high pass filters
    std::array<bark::BiquadFilter, 2> hipass_filters_;

    // Stereo lowpass tone filter
    std::array<bark::OnePoleFilter, 2> tone_filters_;

    // Wet/dry mixer
    bark::Mixer mixer_;

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
    bark::SmoothValue<float> delay_time_{};

    // Depth of chorus.
    bark::SmoothValue<float> depth_{};

    // Previous system time. Used for tap tempo.
    uint32_t prev_time_{};

    // Use for led delay indicator
    float delay_counter_{};

    // Store delay knob value so tap can bypass knob
    // Set this negative to force an initialization
    float delay_knob_{ -1.f };

    // Engage fs1: effect is on if true.
    bool engage_fs1_{true};

    // Engage fs2: maintain the state of fs2
    // This is a hack since I bought the wrong switches
    bool engage_fs2_{true};

    // True if tri chorus mode is on
    bool sine_mode_{false};

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