#pragma once
#ifndef CHORUS_H
#define CHORUS_H

#include "modulated_delay.h"
#include "multitap_delay.h"

#include <array>

namespace dingus_dsp
{

// A stereo chorus engine combining two modulated delay lines.
class Chorus
{
    public:
        Chorus() {}

        ~Chorus() {}

        // Initialize the state of the chorus.
        void Init(float sample_rate) {
            sample_rate_ = sample_rate;
            delaylines_[0].Init(sample_rate);
            delaylines_[1].Init(sample_rate, 333);
        }

        // Process a sample for one channel.
        float Process(float input, size_t channel) {
            float output = delaylines_[channel].Process(input);
            return output;
        }

        // Reset the state of the chorus.
        void Reset() {
            delaylines_[0].Reset();
            delaylines_[1].Reset();
        }

        // Returns the maximum delay time in seconds.
        float GetMaxDelay() {
            return static_cast<float>(delaylines_[0].GetMaxDelay()) / sample_rate_;
        }

        // Returns the maximum number of voices.
        size_t GetMaxNumVoices() {
            return delaylines_[0].GetMaxNumTaps();
        }

        // Set the delay time given a 0 to 1 parameter.
        // Scales the time to between 5ms to 25ms
        // The right delay line has a 2ms offset.
        void SetDelayTime(float delay_time) {
            delay_time = .005f + delay_time * 0.023f;
            delaylines_[0].SetDelayTime(delay_time);
            delaylines_[1].SetDelayTime(delay_time + 0.002f);
        }

        // Set the rate of modulation.
        void SetRate(float rate) {
            delaylines_[0].SetRate(rate);
            delaylines_[1].SetRate(rate);
        }

        // Set the depth of modulation.
        void SetDepth(float depth) {
            delaylines_[0].SetDepth(depth);
            delaylines_[1].SetDepth(depth * -1.f);
        }

        // Set the amount of feedback.
        void SetFeedbackLevel(float feedback_lvl) {
            delaylines_[0].SetFeedbackLevel(feedback_lvl);
            delaylines_[1].SetFeedbackLevel(feedback_lvl);
        }

        // Set the number of voices (taps).
        void SetNumVoices(size_t voices) {
            delaylines_[0].SetNumActiveTaps(voices);
            delaylines_[1].SetNumActiveTaps(voices);
        }

    private:
        // Stereo multitap delay lines.
        std::array<MultitapDelay<8, 48000>, 2> delaylines_;

        // The audio sample rate.
        float sample_rate_ {};
};

}

#endif