#include "daisy_petal.h"
#include "daisysp.h"

#include "dingus_dsp/chorus.h"
#include "dingus_dsp/dingus_math.h"
#include "dingus_dsp/smooth_value.h"

using namespace daisy;
//using namespace daisysp;

// A wrapper for all of the effect parameters.
struct GlobalParameters 
{
	void Init(float sample_rate) {
		delay_time.Init(0.01f, sample_rate);
		depth.Init(0.01f, sample_rate);
	}

	bool effectOn { false };
	float mix { 0.5f };
	float level { 1.f };
	float feedback { 0.f };
	float rate { 1.f };
	size_t voices { 1 };
	dingus_dsp::SmoothValue<float> delay_time {};
	dingus_dsp::SmoothValue<float> depth {};
};

// Globals
DaisyPetal hw;
static dingus_dsp::Chorus g_chorus;
static GlobalParameters g_param;

void Controls() 
{
	hw.ProcessAllControls();
	g_param.mix = hw.knob[0].Process();
	g_param.level = hw.knob[1].Process();
	g_param.effectOn ^= hw.switches[0].RisingEdge();

	g_param.voices += hw.encoder.Increment();
	g_param.voices = dingus_dsp::Clamp(g_param.voices, (size_t) 1, g_chorus.GetMaxNumVoices());
	g_chorus.SetNumVoices(g_param.voices);

	g_param.feedback = hw.knob[3].Process();
	g_chorus.SetFeedbackLevel(g_param.feedback);
	g_param.rate = hw.knob[4].Process();
	g_chorus.SetRate(g_param.rate * g_param.rate * 20.f);

	// Parameter is latched and will only update if changed significantly.
	g_param.delay_time.SetTargetValue(hw.knob[2].Process());

	// Parameter is latched and will only update if changed significantly.
	g_param.depth.SetTargetValue(hw.knob[5].Process());
}

void AudioCallback(AudioHandle::InputBuffer in, 
				   AudioHandle::OutputBuffer out, 
				   size_t size)
{
	// Need to spread these calls out in a way to reduce noise.
	Controls();

	for (size_t i = 0; i < size; i++)
	{
		g_chorus.SetDelayTime(g_param.delay_time.GetNextValue());
		g_chorus.SetDepth(g_param.depth.GetNextValue());

		out[0][i] = in[0][i];
		out[1][i] = in[1][i];

		if (g_param.effectOn) 
		{
			float processed_l = g_chorus.Process(in[0][i], 0);
			float processed_r = g_chorus.Process(in[1][i], 1);

			out[0][i] = in[0][i] * (1.f - g_param.mix) + (processed_l - processed_r) * g_param.mix;
			out[1][i] = in[1][i] * (1.f - g_param.mix) + (processed_r - processed_l) * g_param.mix;

			out[0][i] *= g_param.level;
			out[1][i] *= g_param.level;
		} 
	}
}

int main(void)
{
	// Default initialized to 24-bit 48kHz with block size of 48
	hw.Init();
	//hw.SetAudioBlockSize(8);

	float sample_rate = hw.AudioSampleRate();
	g_chorus.Init(sample_rate);
	g_param.Init(sample_rate);

	hw.StartAdc();
	hw.StartAudio(AudioCallback);

	while(1) 
	{
		hw.DelayMs(6);
		hw.ClearLeds();

		hw.SetFootswitchLed((DaisyPetal::FootswitchLed) 0, (float) g_param.effectOn);

		for (int i = 0; i < g_param.voices; i++) {
			hw.SetRingLed((DaisyPetal::RingLed) i, 1.f, 0.f, 0.f);
		}

		hw.UpdateLeds();
	}
}
