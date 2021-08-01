#include "daisy_petal.h"
#include "ChorusProcessor.h"

static ChorusProcessor* processor_ptr;

// A wrapper to call the audio callback using the processor member function.
void AudioCallbackWrapper(daisy::AudioHandle::InputBuffer in, 
						  daisy::AudioHandle::OutputBuffer out, 
						  size_t size) 
{
	return processor_ptr->AudioCallback(in, out, size);
}

int main(void)
{
	daisy::DaisyPetal hw;
	hw.Init();

	ChorusProcessor processor(hw);
	processor.Init();

	hw.StartAdc();
	hw.StartAudio(AudioCallbackWrapper);

	while(1) 
	{
		processor.UpdateLeds();
	}
}
