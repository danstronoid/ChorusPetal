#pragma once
#ifndef MULTITAP_DELAY_H
#define MULTITAP_DELAY_H

#include <array>
#include <cstdlib>

#include "dingus_math.h"
#include "delayline.h"

namespace dingus_dsp
{

template<size_t num_taps, size_t max_delay>
class MultitapDelay 
{
    public:
        MultitapDelay() {}
        ~MultitapDelay() {}

        // Sets the initial state of the delay given the sample rate.
        void Init(float sample_rate) {
            sample_rate_ = sample_rate;
            osc_.Init(sample_rate_);
            osc_.SetType(OscType::SINE);
            delay_taps_.fill(0);
            decay_ = 1.f / static_cast<float>(num_taps);

            // Set the random spread values
            srand(666);
            CalculateSpread();
        }

        // Writes the input sample to the delay buffer and returns the delayed output.
        float Process(float input) {
            float output = 0.f;
            float feedback = prev_output_ * feedback_lvl_;
            delay_line_.Write(input + feedback);

            float gain = 1.f;
            float mod_value = osc_.Process();

            // Only read and output from the active taps.
            for (size_t i = 0; i < active_taps_; i++) {
                float offset = delay_taps_[i] * mod_value * 0.5f;
                output += delay_line_.Read(delay_taps_[i] + offset) * gain;
                gain = gain = decay_;
            }

            prev_output_ = output;

            return output;
        }

        // Clears the buffer and resets the oscillator.
        void Reset() {
            delay_line_.Clear();
            osc_.Reset();
        }

        // Set the delay time for each tap based on the target delay.
        void SetDelayTime(float delay_time) {
            delay_time = delay_time * sample_rate_;

            // Make sure the delay time won't exceed the buffer size.
            delay_time = Clamp<float>(delay_time, 0.f, max_delay);

            float tap_spacing = delay_time / static_cast<float>(num_taps);
            float tap_time = tap_spacing;

            for (size_t i = 0; i < num_taps; i++) {
                float spread = tap_spread_[i] * tap_spacing * 0.1f;
                delay_taps_[i] = tap_time - spread;
                tap_time += tap_spacing;
            }
        }

        // Set the amount of feedback
        void SetFeedbackLevel(float feedback_lvl) {
            feedback_lvl_ = feedback_lvl;
        }

        // Sets the rate of the modulation.
        void SetRate(float rate) {
            osc_.SetFrequency(rate);
        }

        // Sets the depth of the modulation.
        void SetDepth(float depth) {
            osc_.SetAmplitude(Clamp<float>(depth, -1.f, 1.f));
        }

        // Sets the number of active taps to process.
        // Activates taps in increasing order of delay time.
        // Value must be between 1 and the total num_taps
        void SetNumActiveTaps(size_t active_taps) {
            active_taps_ = Clamp(active_taps, 1, num_taps);
        }

        // Returns the maximum delay time in samples.
        // Only allow for a maximum delay of half the buffer size.
        // This prevents a modulated time beyond the length of the buffer.
        size_t GetMaxDelay() { return max_delay / 2; }

        size_t GetMaxNumTaps() { return num_taps; }

    private:
        // The delay buffer.
        DelayLine<float, max_delay> delay_line_;

        // The modulation lfo.
        Oscillator osc_;

        // The delay time for each tap.
        std::array<float, num_taps> delay_taps_;

        // A random spread value calculated for each tap.
        std::array<float, num_taps> tap_spread_;

        // The amount by which gain is reduced for each tap.
        float decay_ {};

        // The amount of feedback.
        float feedback_lvl_ {};

        // The previous output value.
        float prev_output_ {};

        // The audio sample rate.
        float sample_rate_ {};

        // The number of active taps to output.
        size_t active_taps_ { num_taps };

        // Calculates a random spread value for each tap.
        // This is done at initialization and the value is seeded with a constant.
        // Thus the spread values will be random but persistent.
        void CalculateSpread() {
            for (size_t i = 0; i < num_taps; i++) {
                tap_spread_[i] = (static_cast<float>(rand()) / RAND_MAX);
            }
        }
};

}

#endif