#include <string.h>
#include <math.h>
#include "daisy_pod.h"
#include "daisysp.h"
#include "Equalizer.h"
#include "Controller.h"

using namespace daisy;
using namespace daisysp;

// Use hwPod or hwSeed (Realtime or testing)
#define USE_HWPOD 

#define SAMPLE_RATE 		48000 // Set to 48000 or 96000
#define SAMPLE_BUFFER_SIZE 		8
#define SAMPLE_TIME_NS		(SAMPLE_BUFFER_SIZE/(float)SAMPLE_RATE*1000000000) // in ns
#define NUM_COLORS 				7

static Color my_colors[NUM_COLORS];
static DaisyPod hwPod; // Used for realtime audio and controls
static uint32_t  start, end, dur;

static Equalizer equalizerLeft;
static Equalizer equalizerRight;
static Controller controller(&equalizerLeft, &equalizerRight, &hwPod);

void AudioCallback(AudioHandle::InputBuffer in, AudioHandle::OutputBuffer out, size_t size)
{
	start = System::GetTick();

	for (size_t i = 0; i < size; i++)
	{
		out[0][i] = equalizerLeft.Process(in[0][i]);
		out[1][i] = equalizerRight.Process(in[0][i]);

	}

	end = System::GetTick();
	dur = (end - start) * 5; // ns

}

#ifndef USE_HWPOD

static Oscillator osc; // Oscillator for testing
static DaisySeed hwSeed; // Used for testing and logging without hwPod

void algoTester(void)
{
	// Placeholder for non-realtime testing of algorithms
	float sample, res, sample_rate; 

    // Below code creates COM4 port and displays log messages
	// hwSeed and hwPod do not work well together
	hwSeed.Configure();
	hwSeed.Init();
	hwSeed.StartLog();
    System::Delay(5000); // Wait 5 second
	hwSeed.PrintLine("Daisy Pod IIR Filter Example");

    hwSeed.SetAudioBlockSize(SAMPLE_BUFFER_SIZE);
    sample_rate = hwSeed.AudioSampleRate();
    osc.Init(sample_rate);
	hwSeed.PrintLine("Sample rate %.6f", sample_rate);	

	// Set parameters for oscillator
    osc.SetWaveform(osc.WAVE_SIN);
    osc.SetFreq(785); // Notch frequency Hz
    osc.SetAmp(0.5);
	
	equalizerLeft.setBypass(false);
	while (1) {
		//sample = 0.5f;
		sample = osc.Process();
		start = System::GetTick();
		res = equalizerLeft.Process(sample);
		end = System::GetTick();
		dur = (end - start) * 5; // ps
		//dur = (end - start) / 200; // us
		//hwSeed.PrintLine("IIR filter duration = %u ns start = %u end = %u", dur, start, end);
		//hwSeed.PrintLine("Sample input %.4f and output %.4f", sample, res);
		hwSeed.PrintLine("%.6f,%.6f", sample, res);
		System::Delay(100); // Wait 100 mseconds
	}

}

#endif

int main(void)
{
	equalizerLeft.Init(SAMPLE_RATE);
	equalizerLeft.setBypass(true);
	equalizerRight.Init(SAMPLE_RATE);
	equalizerRight.setBypass(true);

#ifdef USE_HWPOD // Realtime audio with Daisy Pod and IIR Filter bypass toggle

	int32_t  inc;
	bool eqOn = false;
	int counter = 0;
	int band = 0;

	my_colors[0].Init(Color::PresetColor::RED);
    my_colors[1].Init(Color::PresetColor::GREEN);
    my_colors[2].Init(Color::PresetColor::BLUE);
    my_colors[3].Init(Color::PresetColor::WHITE);
	my_colors[4].Init(Color::PresetColor::PURPLE);
	my_colors[5].Init(Color::PresetColor::GOLD);
    my_colors[6].Init(Color::PresetColor::OFF);

	hwPod.Init();
	hwPod.seed.StartLog();
	hwPod.SetAudioBlockSize(SAMPLE_BUFFER_SIZE); // number of samples handled per callback
	
	if (SAMPLE_RATE == 48000)
		hwPod.SetAudioSampleRate(SaiHandle::Config::SampleRate::SAI_48KHZ);
	else if (SAMPLE_RATE == 96000)
		hwPod.SetAudioSampleRate(SaiHandle::Config::SampleRate::SAI_96KHZ);
	else
		hwPod.seed.PrintLine("Unsupported sample rate %d", SAMPLE_RATE);		

	hwPod.StartAdc();
	hwPod.StartAudio(AudioCallback);

    while(1)
    {
    	hwPod.ProcessAllControls(); 
	
		inc = hwPod.encoder.Increment();

		if (inc < 0) {
			controller.adjust(DEC_PARAM_VALUE);
		}
		else if (inc > 0) {
			controller.adjust(INC_PARAM_VALUE);
		}
	
        // using button1 turn EQ on/off
        if (hwPod.button1.RisingEdge()) {
			eqOn = !eqOn;
			controller.setBypass(!eqOn);
		}

       // using button2 select eq band
 		if (hwPod.button2.RisingEdge()) {
			band = (int)controller.adjust(SEL_BAND);
			hwPod.seed.PrintLine("Equalizer band %d                                    ", band);
			counter = 1;
		}
		
		hwPod.ClearLeds();
		int EqOn = eqOn ? 3 : 6; // LED White if on, Off if bypass
		hwPod.led1.SetColor(my_colors[EqOn]);
		hwPod.led2.SetColor(my_colors[band]);
		hwPod.UpdateLeds();
		
		if (counter % 100000 == 0) { // Print every ~0.2 seconds
			//hwPod.seed.PrintLine("Daisy Pod IIR Filter Example");
			//hwPod.seed.PrintLine("Equalizer duration = %u ns start = %u end = %u", dur, start, end);
			controller.printState(dur, SAMPLE_TIME_NS);
		}
		counter++;
		//System::Delay(1); // Wait 1 ms
    }

#else // Non-realtime test with Daisy Seed testing and logging

	algoTester();

#endif

}
