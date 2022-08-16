#include "daisy_petal.h"
#include "chorus_processor.h"

// Must be in global or heap mem
static daisy::DaisyPetal hw;
static ChorusProcessor processor(hw);

// A wrapper to call the audio callback using the processor member function.
void AudioCallbackWrapper(daisy::AudioHandle::InputBuffer in,
						  daisy::AudioHandle::OutputBuffer out,
						  size_t size)
{
	processor.AudioCallback(in, out, size);
}

// Main entry point to process loop.
int main(void)
{
	hw.Init();
	hw.seed.SetAudioBlockSize(1);
	hw.seed.SetAudioSampleRate(daisy::SaiHandle::Config::SampleRate::SAI_48KHZ);

	processor.Init();

	hw.StartAdc();
	hw.StartAudio(AudioCallbackWrapper);

	while (1)
	{
		hw.DelayMs(6);
		
		processor.ProcessSwitches();
		processor.UpdateLeds();
	}
}
