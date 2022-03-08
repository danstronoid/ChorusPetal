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
    chorus_.SetDelayRange(0.005f, 1.f, 0.005f);
    chorus_.SetNumVoices(1);

    // Set the inital state of the filters
    for (int i = 0; i < 2; i++) {
        cut_filters_[i].Init(sample_rate, dingus_dsp::BiquadType::LowShelf);
        cut_filters_[i].SetParams(FILTER_CUTOFF, FILTER_Q, 0.0f);

        boost_filters_[i].Init(sample_rate, dingus_dsp::BiquadType::LowShelf);
        boost_filters_[i].SetParams(FILTER_CUTOFF, FILTER_Q, 0.0f);

        tone_filters_[i].Init(sample_rate, dingus_dsp::OnePoleType::LowPass);
    }

    mixer_.SetType(dingus_dsp::MixType::Sqrt);

    // Initialize the smoothed paramters
    delay_time_.Init(0.025f, sample_rate);
    depth_.Init(0.025f, sample_rate);
}

void ChorusProcessor::ProcessControls()
{
    hw_.ProcessAllControls();

    // Toggle the bypass state
    if (!hw_.switches[1].Pressed()) {
        engage_ ^= hw_.switches[0].RisingEdge();
    }
    
    // Set the warp factor based on the switch state
    warp_latch_ ^= hw_.switches[1].Pressed() && hw_.switches[0].RisingEdge();
    warp_factor_ = (hw_.switches[1].Pressed() || warp_latch_) ? 100.f : 1.f;
    depth_.SetTargetValue(hw_.knob[5].Process() * warp_factor_);

    // Set delay time using tap tempo
    if (hw_.switches[1].RisingEdge()) {
        uint32_t curr_time = daisy::System::GetNow();
        uint32_t delay_ms = curr_time - prev_time_;
        prev_time_ = curr_time;

        if (delay_ms < (chorus_.GetMaxDelay() * 1000)) {
            delay_time_.SetTargetValue(delay_ms * 0.001f);
        }
    }

    // Update delay time only if the knob was moved
    float knob_2 = hw_.knob[2].Process();
    if (!dingus_dsp::CompareFloat(delay_knob_, knob_2, 0.001f)) {
        delay_knob_ = knob_2;
        delay_time_.SetTargetValue(chorus_.GetMinDelay() + delay_knob_ * chorus_.GetMaxDelay());
    }

    // The cut/boost filter gain scales as mix increases
    mix_ = hw_.knob[0].Process();
    mixer_.SetMix(mix_);

    // Gaussian distribution
    mix_scale_ = expf(-1.0f * (mix_ - 0.5) * (mix_ - 0.5) / 0.02f); 
    
    // level_ = hw_.knob[1].Process();
    tone_ = dingus_dsp::QuadraticScale<float>(800.0f, 20000.0f, hw_.knob[1].Process());

    for (int i = 0; i < 2; i++) {
        cut_filters_[i].SetGain(mix_scale_ * FILTER_GAIN * -1.0f);
        boost_filters_[i].SetGain(mix_scale_ * FILTER_GAIN);
        tone_filters_[i].SetFreq(tone_);
    }

    chorus_.SetFeedbackLevel(hw_.knob[3].Process());

    // Rate scales from .01Hz - 20Hz
    float rate_knob = hw_.knob[4].Process();
    chorus_.SetRate(dingus_dsp::QuadraticScale(0.1f, 10.0f, rate_knob));

}

void ChorusProcessor::AudioCallback(daisy::AudioHandle::InputBuffer in,
                                    daisy::AudioHandle::OutputBuffer out,
                                    size_t size)
{
    ProcessControls();

    float wet_l, wet_r, dry_l, dry_r = 0.0f;

    for (size_t i = 0; i < size; i++) {
        chorus_.SetDelayTimeInSec(delay_time_.GetNextValue());
        chorus_.SetDepth(depth_.GetNextValue());

        wet_l = cut_filters_[0].Process(in[0][i]);
        wet_r = cut_filters_[1].Process(in[1][i]);

        wet_l = chorus_.Process(wet_l, 0);
        wet_r = chorus_.Process(wet_r, 1);

        wet_l = tone_filters_[0].Process(wet_l);
        wet_r = tone_filters_[1].Process(wet_r);

        dry_l = boost_filters_[0].Process(in[0][i]);
        dry_r = boost_filters_[1].Process(in[1][i]);

        if (engage_) {
            out[0][i] = dingus_dsp::SoftClip::Sinusoidal(mixer_.Process(dry_l, wet_l - wet_r * mix_scale_), CLIP_THRESH);
            out[1][i] = dingus_dsp::SoftClip::Sinusoidal(mixer_.Process(dry_r, wet_r - wet_l * mix_scale_), CLIP_THRESH);
        } else {
            out[0][i] = in[0][i];
            out[1][i] = in[1][i];
        }
    }
}

void ChorusProcessor::UpdateLeds()
{
    hw_.DelayMs(6);
    hw_.ClearLeds();

    // Set bypass led
    hw_.SetFootswitchLed((daisy::DaisyPetal::FootswitchLed) 0,
                         static_cast<float>(engage_ || hw_.switches[0].Pressed()));


    // Warp light is toggled off when latch is enabled/disabled
    bool warp_light = (warp_latch_ || hw_.switches[1].Pressed()) && !hw_.switches[0].Pressed();

    // Warp led on while pressed
    hw_.SetFootswitchLed((daisy::DaisyPetal::FootswitchLed) 1,
                         static_cast<float>(warp_light));

    hw_.UpdateLeds();
}