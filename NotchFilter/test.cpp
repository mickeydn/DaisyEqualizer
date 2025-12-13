#include <string.h>
#include <math.h>
//#include "daisy_seed.h"
#include "daisy_pod.h"
#include "daisysp.h"
#include "IIRFilter.h"

using namespace daisy;
using namespace daisysp;

// Use hwPod or hwSeed 
#define USE_HWPOD 

static DaisyPod hwPod; // Used for realtime audio and controls
static DaisySeed hwSeed; // Used for testing and logging without hwPod
static Oscillator osc; // Oscillator for testing

static uint32_t  start, end, dur;

static IIRFilter IIRFilterLeft;
static IIRFilter IIRFilterRight;

void AudioCallback(AudioHandle::InputBuffer in, AudioHandle::OutputBuffer out, size_t size)
{

	hwPod.ProcessAllControls();
	for (size_t i = 0; i < size; i++)
	{
	    start = System::GetTick();
		out[0][i] = IIRFilterLeft.Process(in[0][i]);
		out[1][i] = IIRFilterRight.Process(in[0][i]);
		end = System::GetTick();
		dur = (end - start) * 5; // ns
		//dur = (end - start) / 200; // us

		// Bypass filter
		//out[0][i] = in[0][i];
		//out[1][i] = in[1][i];
	}
}

int main(void)
{
	float b_coeff[3] = {1.0, -1.9895, 1.0};
	float a_coeff[3] = {1.0, -1.7905, 0.8100};
	IIRFilterLeft.Init(a_coeff, b_coeff);
	IIRFilterRight.Init(a_coeff, b_coeff);

#ifdef USE_HWPOD // Realtime audio with Daisy Pod and IIR Filter bypass toggle

	bool brightness1, brightness2;
    brightness1 = false;
    brightness2 = false;
	int counter = 0;

	hwPod.Init();
	hwPod.seed.StartLog();
	hwPod.SetAudioBlockSize(4); // number of samples handled per callback
	hwPod.SetAudioSampleRate(SaiHandle::Config::SampleRate::SAI_48KHZ);
	hwPod.StartAdc();
	hwPod.StartAudio(AudioCallback);

    while(1)
    {
        hwPod.ProcessDigitalControls();

        // using button1 as momentary switch for turning on/off led1
        brightness2 = hwPod.button2.Pressed();

        // using button2 as latching switch for toggling led2
        if (hwPod.button1.RisingEdge()) {
            brightness1 = !brightness1;
			IIRFilterLeft.SetBypass(!brightness1);
			IIRFilterRight.SetBypass(!brightness1);
		}
			
        // assign brightness levels to each led (R, G, B)
        hwPod.led1.Set(brightness1, brightness1, brightness1);
        hwPod.led2.Set(brightness2, brightness2, brightness2);
        hwPod.UpdateLeds();
		//hwPod.seed.PrintLine("Daisy Pod IIR Filter Example");

		if (counter % 1000 == 0) { // Print every second
			hwPod.seed.PrintLine("IIR filter duration = %u ns start = %u end = %u", dur, start, end);
		}
		counter++;
		System::Delay(1); // Wait 1 ms
    }

#else // Non-realtime test with Daisy Seed testing and logging
	float sample, res, sample_rate; 

    // Below code creates COM4 port and displays log messages
	// hwSeed and hwPod do not work well together
	hwSeed.Configure();
	hwSeed.Init();
	hwSeed.StartLog();
    System::Delay(5000); // Wait 5 second
	hwSeed.PrintLine("Daisy Pod IIR Filter Example");

    hwSeed.SetAudioBlockSize(4);
    sample_rate = hwSeed.AudioSampleRate();
    osc.Init(sample_rate);
	hwSeed.PrintLine("Sample rate %.6f", sample_rate);	

	// Set parameters for oscillator
    osc.SetWaveform(osc.WAVE_SIN);
    osc.SetFreq(785); // Notch frequency Hz
    osc.SetAmp(0.5);
	
	IIRFilterRight.SetBypass(false);
	while (1) {
		//sample = 0.5f;
		sample = osc.Process();
		start = System::GetTick();
		//res = IIRFilterLeft.Process(sample);
		res = IIRFilterRight.Process(sample);
		end = System::GetTick();
		dur = (end - start) * 5; // ps
		//dur = (end - start) / 200; // us
		//hwSeed.PrintLine("IIR filter duration = %u ns start = %u end = %u", dur, start, end);
		//hwSeed.PrintLine("Sample input %.4f and output %.4f", sample, res);
		hwSeed.PrintLine("%.6f,%.6f,", sample, res);
		System::Delay(100); // Wait 100 mseconds
	}

#endif

}
