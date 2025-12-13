// Kim Bjerge - 28. November 2025
// Daisy Pod IIR Equalizer Example
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
#define NUM_COLORS 				5

//#define DISPLAY_FFT_TIMING # Define to show timing info instead of full spectrum
#define USE_SDRAM // Must be used if FFT_SIZE is larger than 4096
//#define FFT_SIZE 32768  // Max size for STM32H7 with 16MB SDRAM	
//#define FFT_SIZE 8192	
#define FFT_SIZE 4096	
//#define FFT_SIZE 2048	
//#define FFT_SIZE 1024	
//#define FFT_SIZE 64	
#include "FFT.h"
#include "FFTMonitor.h"

static Color my_colors[NUM_COLORS];
static DaisyPod hwPod; // Used for realtime audio and controls
static uint32_t  start, end, dur;
static FFT fft;
static FFTMonitor fftMonitor(&hwPod, &fft);

static Equalizer equalizerLeft;
static Equalizer equalizerRight;
static Controller controller(&equalizerLeft, &equalizerRight, &hwPod);

void AudioCallback(AudioHandle::InputBuffer in, AudioHandle::OutputBuffer out, size_t size)
{
	start = System::GetTick();

	for (size_t i = 0; i < size; i++)
	{
		float sample = equalizerLeft.Process(in[0][i]); 
		fftMonitor.insertSample(sample);
		out[0][i] = sample;
		out[1][i] = equalizerRight.Process(in[0][i]);

		// Bypass filter
		//out[0][i] = in[0][i];
		//out[1][i] = in[1][i];
	}

	end = System::GetTick();
	dur = (end - start) * 5; // ns
	//dur = (end - start) / 200; // us
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
    my_colors[4].Init(Color::PresetColor::OFF);

	fft.init(FFT_SIZE);
	//fftMonitor.setWindowType(WindowFunction::WELCH);
	//fftMonitor.setWindowType(WindowFunction::BLACKMAN);
	fftMonitor.setWindowType(WindowFunction::HANNING);
	
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
	
		// Debounce the Encoder at a steady, fixed rate.
		//hwPod.encoder.Debounce();
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
		int EqOn = eqOn ? 3 : 4; // LED White if on, Off if bypass
		hwPod.led1.SetColor(my_colors[EqOn]);
		hwPod.led2.SetColor(my_colors[band]);
		hwPod.UpdateLeds();
		
		if (counter % 200000 == 0) { // Print every ~1 seconds
			//hwPod.seed.PrintLine("Daisy Pod IIR Filter Example");
			//hwPod.seed.PrintLine("Equalizer duration = %u ns start = %u end = %u", dur, start, end);
			
			//controller.printState(dur, SAMPLE_TIME_NS);
			controller.printParam();
			fftMonitor.processFrame();
		}
		counter++;
		//System::Delay(1); // Wait 1 ms
    }

#else // Non-realtime test with Daisy Seed testing and logging

	algoTester();

#endif

}
