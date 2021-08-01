/*
  ==============================================================================

    File: chorus_engine.h
    Author: Daniel Schwartz
    Description: A stereo chorus engine combining two modulated delay lines.

  ==============================================================================
*/

#include "chorus_engine.h"

using namespace dingus_dsp;

void Chorus::Init(float sample_rate)
{
    sample_rate_ = sample_rate;
    delaylines_[0].Init(sample_rate);
    delaylines_[1].Init(sample_rate, 333);
}

float Chorus::Process(float input, size_t channel)
{
    float output = delaylines_[channel].Process(input);
    return output;
}

void Chorus::Process(float* left, float* right) 
{
    float processed_l = delaylines_[0].Process(*left);
    float processed_r = delaylines_[1].Process(*right);

    // Create stereo chorus effect by taking the difference of the channels.
    *left = processed_l - processed_r;
    *right = processed_r - processed_l;
}

void Chorus::Reset()
{
    delaylines_[0].Reset();
    delaylines_[1].Reset();
}

float Chorus::GetMaxDelay()
{
    return static_cast<float>(delaylines_[0].GetMaxDelay()) / sample_rate_;
}

size_t Chorus::GetMaxNumVoices()
{
    return delaylines_[0].GetMaxNumTaps();
}

void Chorus::SetDelayTime(float delay_time)
{
    // Scales the time to between 5ms to 25ms
    delay_time = .005f + delay_time * 0.023f;
    delaylines_[0].SetDelayTime(delay_time);

    // The right delay line has a 2ms offset.
    delaylines_[1].SetDelayTime(delay_time + 0.002f);
}

void Chorus::SetRate(float rate)
{
    delaylines_[0].SetRate(rate);
    delaylines_[1].SetRate(rate);
}

void Chorus::SetDepth(float depth)
{
    delaylines_[0].SetDepth(depth);

    // The invert the right lfo.
    delaylines_[1].SetDepth(depth * -1.f);
}

void Chorus::SetFeedbackLevel(float feedback_lvl)
{
    delaylines_[0].SetFeedbackLevel(feedback_lvl);
    delaylines_[1].SetFeedbackLevel(feedback_lvl);
}

void Chorus::SetNumVoices(size_t voices)
{
    delaylines_[0].SetNumActiveTaps(voices);
    delaylines_[1].SetNumActiveTaps(voices);
}