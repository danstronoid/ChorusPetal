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

    // Initialize the terrarium leds
    led1_.Init(hw_.seed.GetPin(Terrarium::L_1), false);
    led2_.Init(hw_.seed.GetPin(Terrarium::L_2), false);

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

    /* // Reset the delay time to chorus value and ignore tap tempo
    if (hw_.switches[Terrarium::FS_2].Pressed() && hw_.switches[Terrarium::FS_1].RisingEdge()) {
        delay_time_.SetTargetValue(chorus_.GetMinDelay());
        prev_time_ = 0.f;
    } else {
        // Otherwise toggle the bypass state
        engage_ ^= hw_.switches[Terrarium::FS_1].RisingEdge();
    }

    // Set delay time using tap tempo
    if (hw_.switches[Terrarium::FS_2].RisingEdge()) {
        uint32_t curr_time = daisy::System::GetNow();
        uint32_t delay_ms = curr_time - prev_time_;
        prev_time_ = curr_time;

        if (delay_ms < (chorus_.GetMaxDelay() * 1000)) {
            delay_time_.SetTargetValue(delay_ms * 0.001f);
        }
    } */

    // This is a hack since I bought the wrong switches :/
    // Toggle the bypass state
    engage_ = hw_.switches[Terrarium::FS_1].Pressed();
    led1_.Set(engage_ ? 1.f : 0.f);

    float amt = chorus_.GetLfoValue() * 0.49f + 1.f;
    led2_.Set(amt);
    
    led1_.Update();
    led2_.Update();

    // Set delay time using tap tempo
    if (fs2_state_ != hw_.switches[Terrarium::FS_2].Pressed()) {
        fs2_state_ = hw_.switches[Terrarium::FS_2].Pressed();
        uint32_t curr_time = daisy::System::GetNow();
        uint32_t delay_ms = curr_time - prev_time_;
        prev_time_ = curr_time;

        if (delay_ms < (chorus_.GetMaxDelay() * 1000)) {
            delay_time_.SetTargetValue(delay_ms * 0.001f);
        }
    } 

    // Toggle the high pass filters
    hipass_engage_ = hw_.switches[Terrarium::S_4].Pressed();

    // Toggle between tri and sine waveforms
    if (tri_mode_ != hw_.switches[Terrarium::S_1].Pressed()) {
        tri_mode_ = hw_.switches[Terrarium::S_1].Pressed();

        if (tri_mode_) {
            chorus_.SetOscType(dingus_dsp::LfoType::STRI);
        } else {
            chorus_.SetOscType(dingus_dsp::LfoType::SINE);
        }
    }

    /* ----------------------------- Knob Behaviour ----------------------------- */

    // If the mix is sufficently small, round it to zero
    mix_ = hw_.knob[Terrarium::K_1].Process();
    if (mix_ < 0.001f) mix_ = 0.f;
    mixer_.SetMix(mix_);

    // The cut/boost filter gain scales as mix increases
    // Gaussian distribution to scale the cut/boost filters
    // The filters not active at 0% and 100% mix
    mix_scale_ = expf(-1.0f * (mix_ - 0.5) * (mix_ - 0.5) / 0.02f); 

    // Set the tone filter cutoff
    tone_ = dingus_dsp::QuadraticScale<float>(800.0f, 20000.0f, hw_.knob[Terrarium::K_2].Process());

    // Update all filters
    for (int i = 0; i < 2; i++) {
        cut_filters_[i].SetGain(mix_scale_ * FILTER_GAIN * -1.0f);
        boost_filters_[i].SetGain(mix_scale_ * FILTER_GAIN);
        tone_filters_[i].SetFreq(tone_);
    }

    // Update delay time only if the knob was moved
    float k_val = hw_.knob[Terrarium::K_3].Process();
    if (!dingus_dsp::CompareFloat(delay_knob_, k_val, 0.001f)) {
        delay_knob_ = k_val;
        delay_time_.SetTargetValue(chorus_.GetMinDelay() + delay_knob_ * chorus_.GetMaxDelay());
    }

    // Set the feedback level
    // Cut the level in half for quad mode
    float feedback_lvl = hw_.knob[Terrarium::K_4].Process();
    chorus_.SetFeedbackLevel(feedback_lvl);


    // Rate scales from .01Hz - 20Hz
    float rate_knob = hw_.knob[Terrarium::K_5].Process();
    chorus_.SetRate(dingus_dsp::QuadraticScale(0.1f, 10.0f, rate_knob));

    // Set the warp factor based on the switch state
    // warp_factor_ = (hw_.switches[5].Pressed()) ? 100.f : 1.f;
    // depth_.SetTargetValue(hw_.knob[5].Process() * warp_factor_);
    depth_.SetTargetValue(hw_.knob[Terrarium::K_6].Process());
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
    //hw_.ClearLeds();
    //led1_.Set(0.f);
    //led2_.Set(0.f);

    // Set bypass led
    /* if (hw_.switches[Terrarium::FS_1].Pressed()) {
        led1_.Set(1.f);
    } else {
        float amt = chorus_.GetLfoValue() * 0.49f + 1.f;
        if (engage_)
            led1_.Set(amt);
        else
            led1_.Set(0.f);
    }
    
    // Tap tempo led on while pressed
    if (hw_.switches[Terrarium::FS_2].Pressed())
    {
        led2_.Set(1.f);
    } else {
        led2_.Set(0.f);
    } */
}