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

        hipass_filters_[i].Init(sample_rate, dingus_dsp::BiquadType::HighPass);
        hipass_filters_[i].SetParams(110.f, FILTER_Q, 0.0f);

        tone_filters_[i].Init(sample_rate, dingus_dsp::OnePoleType::LowPass);
    }

    mixer_.SetType(dingus_dsp::MixType::Sqrt);

    // Initialize the smoothed paramters
    delay_time_.Init(0.025f, sample_rate);
    depth_.Init(0.025f, sample_rate);
}

/* -------------------------------------------------------------------------- */

void ChorusProcessor::ProcessControls()
{
    hw_.ProcessAllControls();

    /* ---------------------------- Switch Behaviour ---------------------------- */

    // Reset the delay time to chorus value and ignore tap tempo
    if (hw_.switches[1].Pressed() && hw_.switches[0].RisingEdge()) {
        delay_time_.SetTargetValue(chorus_.GetMinDelay());
        prev_time_ = 0.f;
    } else {
        // Otherwise toggle the bypass state
        engage_ ^= hw_.switches[0].RisingEdge();
    }

    // Set delay time using tap tempo
    if (hw_.switches[1].RisingEdge()) {
        uint32_t curr_time = daisy::System::GetNow();
        uint32_t delay_ms = curr_time - prev_time_;
        prev_time_ = curr_time;

        if (delay_ms < (chorus_.GetMaxDelay() * 1000)) {
            delay_time_.SetTargetValue(delay_ms * 0.001f);
        }
    }

    // Toggle the high pass filters
    hipass_engage_ = hw_.switches[5].Pressed();

    // Toggle between tri and sine waveforms
    if (tri_mode_ != hw_.switches[4].Pressed()) {
        tri_mode_ = hw_.switches[4].Pressed();

        if (tri_mode_) {
            chorus_.SetOscType(dingus_dsp::LfoType::STRI);
        } else {
            chorus_.SetOscType(dingus_dsp::LfoType::SINE);
        }
    }

    /* ----------------------------- Knob Behaviour ----------------------------- */
    
    // Set the feedback level
    // Cut the level in half for quad mode
    float feedback_lvl = hw_.knob[3].Process();
    chorus_.SetFeedbackLevel(feedback_lvl);

    // Set the warp factor based on the switch state
    // warp_factor_ = (hw_.switches[5].Pressed()) ? 100.f : 1.f;
    // depth_.SetTargetValue(hw_.knob[5].Process() * warp_factor_);
    depth_.SetTargetValue(hw_.knob[5].Process());

    // Update delay time only if the knob was moved
    float knob_2 = hw_.knob[2].Process();
    if (!dingus_dsp::CompareFloat(delay_knob_, knob_2, 0.001f)) {
        delay_knob_ = knob_2;
        delay_time_.SetTargetValue(chorus_.GetMinDelay() + delay_knob_ * chorus_.GetMaxDelay());
    }

    // If the mix is sufficently small, round it to zero
    mix_ = hw_.knob[0].Process();
    if (mix_ < 0.001f) mix_ = 0.f;
    mixer_.SetMix(mix_);

    // The cut/boost filter gain scales as mix increases
    // Gaussian distribution to scale the cut/boost filters
    // The filters not active at 0% and 100% mix
    mix_scale_ = expf(-1.0f * (mix_ - 0.5) * (mix_ - 0.5) / 0.02f); 

    // Set the tone filter cutoff
    tone_ = dingus_dsp::QuadraticScale<float>(800.0f, 20000.0f, hw_.knob[1].Process());

    // Update all filters
    for (int i = 0; i < 2; i++) {
        cut_filters_[i].SetGain(mix_scale_ * FILTER_GAIN * -1.0f);
        boost_filters_[i].SetGain(mix_scale_ * FILTER_GAIN);
        tone_filters_[i].SetFreq(tone_);
    }

    // Rate scales from .01Hz - 20Hz
    float rate_knob = hw_.knob[4].Process();
    chorus_.SetRate(dingus_dsp::QuadraticScale(0.1f, 10.0f, rate_knob));
}

/* -------------------------------------------------------------------------- */

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

        if (hipass_engage_)
        {
            wet_l = hipass_filters_[0].Process(wet_l);
            wet_r = hipass_filters_[1].Process(wet_r);
        }

        dry_l = boost_filters_[0].Process(in[0][i]);
        dry_r = boost_filters_[1].Process(in[1][i]);

        if (engage_) {
            out[0][i] = mixer_.Process(dry_l, dingus_dsp::SoftClip::Sinusoidal(wet_l - wet_r * mix_scale_, CLIP_THRESH));
            out[1][i] = mixer_.Process(dry_r, dingus_dsp::SoftClip::Sinusoidal(wet_r - wet_l * mix_scale_, CLIP_THRESH));
        } else {
            out[0][i] = in[0][i];
            out[1][i] = in[1][i];
        }
    }
}

/* -------------------------------------------------------------------------- */

void ChorusProcessor::UpdateLeds()
{
    hw_.DelayMs(6);
    hw_.ClearLeds();

    // Set bypass led
    if (hw_.switches[0].Pressed()) {
        hw_.SetFootswitchLed((daisy::DaisyPetal::FootswitchLed) 0, 1.f);
    } else {
        float amt = chorus_.GetLfoValue() * 0.49f + 1.f;
        hw_.SetFootswitchLed((daisy::DaisyPetal::FootswitchLed) 0,
                            static_cast<float>(engage_) * amt);
    }
    

    // Tap tempo led on while pressed
    hw_.SetFootswitchLed((daisy::DaisyPetal::FootswitchLed) 1,
                         static_cast<float>(hw_.switches[1].Pressed()));

    hw_.UpdateLeds();
}