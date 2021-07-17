/*
  ==============================================================================

    File: delayline.h
    Author: Daniel Schwartz
    Description: A simple fractional delay line using linear interpolation.

  ==============================================================================
*/

#pragma once
#ifndef DINGUS_DELAYLINE_H
#define DINGUS_DELAYLINE_H

#include <array>

namespace dingus_dsp
{

// Takes a float sample type and the max delay time (in samples) which 
// determines the size of the buffer.
template <typename SampleType, size_t max_delay>
class DelayLine 
{
    public:
        DelayLine() {
            Clear();
        }

        ~DelayLine() {}

        // Returns the buffer size.
        size_t Size() {
            return max_delay;
        }

        // Clears the contents of the buffer and resets the position.
        void Clear() {
            buffer_.fill(SampleType(0));
            pos_ = 0;
        }

        // Writes the given value to the buffer.
        inline void Write(SampleType value) {
            buffer_[pos_] = value;
            pos_ = (pos_ + max_delay - 1) % max_delay;
        }

        // Reads from the buffer given a delay time in samples.
        // Performs linear interpolation on fractional times.
        inline SampleType Read(SampleType delay) const {
            SampleType delay_pos = static_cast<SampleType>(pos_) + delay + SampleType(1);

            size_t a = static_cast<size_t>(delay_pos);
            size_t b = (a + 1);

            SampleType frac = delay_pos - static_cast<SampleType>(a);

            SampleType a_value = buffer_[a % max_delay];
            SampleType b_value = buffer_[b % max_delay];

            return a_value + frac * (b_value - a_value);
        }

    private:
        // The buffer to store the sample values.
        std::array<SampleType, max_delay> buffer_;

        // The current write position.
        size_t pos_;
};

}

#endif



