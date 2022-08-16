/*
  ==============================================================================

    File: chorus_processor.cpp
    Author: Daniel Schwartz
    Description: A processor class to manage the controls and audio callback.

  ==============================================================================
*/

#include "chorus_processor.h"

using namespace terrarium;

ChorusProcessor::ChorusProcessor(daisy::DaisyPetal &hw) : hw_(hw) {}

void ChorusProcessor::Init()
{
    float sample_rate = hw_.AudioSampleRate();

    led1_.Init(hw_.seed.GetPin(Terrarium::LED_1), false);
    led2_.Init(hw_.seed.GetPin(Terrarium::LED_2), false);

    chorus_.Init(sample_rate);
    chorus_.SetDelayRange(0.005f, 1.f, 0.005f);
    chorus_.SetNumVoices(1);

    // Set the inital state of the filters
    for (int i = 0; i < 2; i++) {
        cut_filters_[i].Init(sample_rate, bark::BiquadType::LowShelf);
        cut_filters_[i].SetParams(FILTER_CUTOFF, FILTER_Q, 0.0f);

        boost_filters_[i].Init(sample_rate, bark::BiquadType::LowShelf);
        boost_filters_[i].SetParams(FILTER_CUTOFF, FILTER_Q, 0.0f);

        hipass_filters_[i].Init(sample_rate, bark::BiquadType::HighPass);
        hipass_filters_[i].SetParams(110.f, FILTER_Q, 0.0f);

        tone_filters_[i].Init(sample_rate, bark::OnePoleType::LowPass);
    }

    mixer_.SetType(bark::MixType::Sqrt);

    // Initialize the smoothed paramters
    delay_time_.Init(0.025f, sample_rate);
    depth_.Init(0.025f, sample_rate);
}

/* -------------------------------------------------------------------------- */

void ChorusProcessor::ProcessSwitches()
{
    hw_.ProcessDigitalControls();

    // Reset the delay time to chorus value and ignore tap tempo
    if (hw_.switches[Terrarium::FOOTSWITCH_2].Pressed() && hw_.switches[Terrarium::FOOTSWITCH_1].RisingEdge()) {
        delay_time_.SetTargetValue(chorus_.GetMinDelay());
        prev_time_ = 0.f;
    } else {
        // Otherwise toggle the bypass state
        engage_fs1_ ^= hw_.switches[Terrarium::FOOTSWITCH_1].RisingEdge();
    }

    // Set delay time using tap tempo
    if (hw_.switches[Terrarium::FOOTSWITCH_2].RisingEdge()) {
        uint32_t curr_time = daisy::System::GetNow();
        uint32_t delay_ms = curr_time - prev_time_;
        prev_time_ = curr_time;

        if (delay_ms > 0 && delay_ms < (chorus_.GetMaxDelay() * 1000)) {
            delay_time_.SetTargetValue(delay_ms * 0.001f);
        }
    }

    // Toggle the high pass filters
    hipass_engage_ = hw_.switches[Terrarium::SWITCH_4].Pressed();

    // Toggle between tri and sine waveforms
    if (sine_mode_ != hw_.switches[Terrarium::SWITCH_1].Pressed()) {
        sine_mode_ = hw_.switches[Terrarium::SWITCH_1].Pressed();

        if (sine_mode_) {
            chorus_.SetOscType(bark::LfoType::SINE);
        } else {
            chorus_.SetOscType(bark::LfoType::STRI);
        }
    }
}

void ChorusProcessor::ProcessKnobs()
{
    hw_.ProcessAnalogControls();

    // Set the feedback level
    float feedback_lvl = hw_.knob[Terrarium::KNOB_4].Process();
    chorus_.SetFeedbackLevel(feedback_lvl);

    // Set the depth. Might be cool to increase this beyond 1.
    depth_.SetTargetValue(hw_.knob[Terrarium::KNOB_6].Process());

    // Update delay time only if the knob was moved
    float knob3 = hw_.knob[Terrarium::KNOB_3].Process();
    if (!bark::CompareFloat(delay_knob_, knob3, 0.001f)) {
        delay_knob_ = knob3;
        delay_time_.SetTargetValue(chorus_.GetMinDelay() + delay_knob_ * chorus_.GetMaxDelay());
    }

    // If the mix is sufficently small, round it to zero
    mix_ = hw_.knob[Terrarium::KNOB_1].Process();
    if (mix_ < 0.001f) mix_ = 0.f;
    mixer_.SetMix(mix_);

    // The cut/boost filter gain scales as mix increases
    // Gaussian distribution to scale the cut/boost filters
    // The filters not active at 0% and 100% mix
    mix_scale_ = expf(-1.0f * (mix_ - 0.5) * (mix_ - 0.5) / 0.02f); 

    // Set the tone filter cutoff
    lofi_ = hw_.knob[Terrarium::KNOB_2].Process();
    float tone = bark::QuadraticScale<float>(800.0f, 20000.0f, 1.f - lofi_);

    // Update all filters
    for (int i = 0; i < 2; i++) {
        cut_filters_[i].SetGain(mix_scale_ * FILTER_GAIN * -1.0f);
        boost_filters_[i].SetGain(mix_scale_ * FILTER_GAIN);
        tone_filters_[i].SetFreq(tone);
    }

    // Rate scales from .01Hz - 20Hz
    float rate_knob = hw_.knob[Terrarium::KNOB_5].Process();
    chorus_.SetRate(bark::QuadraticScale(0.1f, 10.0f, rate_knob));
}

/* -------------------------------------------------------------------------- */

void ChorusProcessor::AudioCallback(daisy::AudioHandle::InputBuffer in,
                                    daisy::AudioHandle::OutputBuffer out,
                                    size_t size)
{
	ProcessKnobs();

    // Set bypass led
    // This is being updated here for faster pwm
    if (engage_fs1_) {
        float amt = (chorus_.GetLfoValue() + 1.f) * 0.4f + 0.2f;
        led1_.Set(amt);
    } else {
        led1_.Set(0.f);
    }

    led1_.Update();

    float wet_l, wet_r, dry_l, dry_r = 0.0f;

    for (size_t i = 0; i < size; i++) {
        chorus_.SetDelayTimeInSec(delay_time_.GetNextValue());
        chorus_.SetDepth(depth_.GetNextValue());

        wet_l = cut_filters_[0].Process(in[0][i]);
        wet_r = cut_filters_[1].Process(in[0][i]);

        wet_l = chorus_.Process(wet_l, 0);
        wet_r = chorus_.Process(wet_r, 1);

        wet_l = tone_filters_[0].Process(wet_l);
        wet_r = tone_filters_[1].Process(wet_r);

        if (hipass_engage_)
        {
            wet_l = hipass_filters_[0].Process(wet_l);
            wet_r = hipass_filters_[1].Process(wet_r);
        }

        dry_l = boost_filters_[0].Process(in[0][i]);
        dry_r = boost_filters_[1].Process(in[1][i]);

        if (engage_fs1_) {
            out[0][i] = mixer_.Process(dry_l, bark::SoftClip::Sinusoidal(wet_l - wet_r * mix_scale_, CLIP_THRESH - lofi_ * 0.2f));
            out[1][i] = mixer_.Process(dry_r, bark::SoftClip::Sinusoidal(wet_r - wet_l * mix_scale_, CLIP_THRESH - lofi_ * 0.2f));
        } else {
            out[0][i] = in[0][i];
            out[1][i] = in[1][i];
        }
    }
}

/* -------------------------------------------------------------------------- */

void ChorusProcessor::UpdateLeds()
{    
    // The tap tempo led blinks to indicate the delay value
    if (delay_counter_ < 0.006f) {
        delay_counter_ = delay_time_.GetValue();
        led2_.Set(1.f);
    } else {
        delay_counter_ -= 0.006f;
        led2_.Set(0.f);
    }

    led2_.Update();
}