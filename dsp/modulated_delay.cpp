/*
  ==============================================================================

    File: modulated_delay.cpp
    Author: Daniel Schwartz
    Description: A modulated delay line that combines a simple delay line 
    with a modulation source.

  ==============================================================================
*/

#include "modulated_delay.h"

using namespace dingus_dsp;

void ModulatedDelay::Init(float sample_rate)
{
    sample_rate_ = sample_rate;
    osc_.Init(sample_rate);

    // Set the initial osc to a 1Hz tri wave.
    osc_.SetType(OscType::SINE);
    osc_.SetAmplitude(1.f);
    osc_.SetFrequency(1.f);

    // Set the initial delay time to 10ms
    SetDelayTime(0.001f);
}

float ModulatedDelay::Process(float input)
{
    delay_line_.Write(input);

    // Modulated time ranges from 0.5dt to 1.5dt
    float mod_delay = delay_time_ + (delay_time_ * 0.5) * osc_.Process();
    float output = delay_line_.Read(mod_delay);

    return output;
}

void ModulatedDelay::Reset()
{
    delay_line_.Clear();
    osc_.Reset();
}