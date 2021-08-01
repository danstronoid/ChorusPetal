/*
  ==============================================================================

    File: oscillator.h
    Author: Daniel Schwartz
    Description: A basic oscillator implemented using per sample calculations.

  ==============================================================================
*/

#include "oscillator.h"

using namespace dingus_dsp;

void Oscillator::Init(float sample_rate)
{
    sample_rate_ = sample_rate;
    Reset();
    UpdateDelta();
}

float Oscillator::Process()
{
    float sample = 0;

    switch (osc_type_)
    {
    case OscType::SINE:
        sample = sinf(phase_);
        break;

    case OscType::TRI:
        sample = (2.f / MathConstants<float>::PI) * asinf(sinf(phase_));
        break;

    case OscType::SAW:
        sample = -1.f * ((phase_ / MathConstants<float>::PI) - 1.f);
        break;

    case OscType::PULSE:
        sample = (phase_ < MathConstants<float>::PI) ? 1 : -1;
        break;

    default:
        sample = sinf(phase_);
        break;
    }

    phase_ += delta_;

    if (phase_ > MathConstants<float>::TWOPI)
    {
        phase_ = phase_ - MathConstants<float>::TWOPI;
    }

    return amp_ * sample;
}

void Oscillator::UpdateDelta()
{
    delta_ = MathConstants<float>::TWOPI * freq_ / sample_rate_;
}
