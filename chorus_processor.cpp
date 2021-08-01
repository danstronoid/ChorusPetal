/*
  ==============================================================================

    File: chorus_processor.cpp
    Author: Daniel Schwartz
    Description: A processor class to manage the controls and audio callback.

  ==============================================================================
*/

#include "chorus_processor.h"

ChorusProcessor::ChorusProcessor(daisy::DaisyPetal &hw) : hw_(hw) {}

void ChorusProcessor::Init()
{
    float sample_rate = hw_.AudioSampleRate();
    chorus_.Init(sample_rate);
    delay_time_.Init(0.01f, sample_rate);
    depth_.Init(0.01f, sample_rate);
}

void ChorusProcessor::ProcessControls()
{
    hw_.ProcessAllControls();

    engage_ ^= hw_.switches[0].RisingEdge();

    mix_ = hw_.knob[0].Process();
    level_ = hw_.knob[1].Process();

    encoder_pos_ += hw_.encoder.Increment();
    encoder_pos_ = dingus_dsp::Clamp(encoder_pos_, size_t(1), chorus_.GetMaxNumVoices());
    chorus_.SetNumVoices(encoder_pos_);

    chorus_.SetFeedbackLevel(hw_.knob[3].Process());

    float rate_knob = hw_.knob[4].Process();
    chorus_.SetRate(rate_knob * rate_knob * 20.f);

    delay_time_.SetTargetValue(hw_.knob[2].Process());
    depth_.SetTargetValue(hw_.knob[5].Process());
}

void ChorusProcessor::AudioCallback(daisy::AudioHandle::InputBuffer in,
                                    daisy::AudioHandle::OutputBuffer out,
                                    size_t size)
{
    ProcessControls();

    for (size_t i = 0; i < size; i++)
    {
        chorus_.SetDelayTime(delay_time_.GetNextValue());
        chorus_.SetDepth(depth_.GetNextValue());

        out[0][i] = in[0][i];
        out[1][i] = in[1][i];

        if (engage_)
        {
            chorus_.Process(out[0][i], out[1][i]);

            out[0][i] = in[0][i] * (1.f - mix_) + out[0][i] * mix_;
            out[1][i] = in[1][i] * (1.f - mix_) + out[1][i] * mix_;

            out[0][i] *= level_;
            out[1][i] *= level_;
        }
    }
}

void ChorusProcessor::UpdateLeds()
{
    hw_.DelayMs(6);
    hw_.ClearLeds();

    // Set bypass led
    hw_.SetFootswitchLed((daisy::DaisyPetal::FootswitchLed)0,
                         static_cast<float>(engage_));

    // Set encoder ring leds for active voice count
    for (size_t i = 0; i < encoder_pos_; i++)
    {
        // Set led color to red: R, G, B = (1, 0, 0)
        hw_.SetRingLed((daisy::DaisyPetal::RingLed)i, 1.f, 0.f, 0.f);
    }

    hw_.UpdateLeds();
}