#include <cmath>
#include "daisy_pod.h"
#include "daisysp.h"

using namespace daisy;
using namespace daisysp;

#define SAMPLE_RATE 48000

//#define DISPLAY_FFT_TIMING # Define to show timing info instead of full spectrum
#define USE_SDRAM // Must be used if FFT_SIZE is larger than 4096
//#define FFT_SIZE 32768  // Max size for STM32H7 with 16MB SDRAM	
#define FFT_SIZE 8192	
//#define FFT_SIZE 4096	
//#define FFT_SIZE 2048	
//#define FFT_SIZE 1024	
//#define FFT_SIZE 64	
#include "FFT.h"
#include "FFTMonitor.h"

DaisyPod hwPod;

FFT fft;
FFTMonitor fftMonitor(&hwPod, &fft);

void AudioCallback(AudioHandle::InputBuffer in, AudioHandle::OutputBuffer out, size_t size)
{
	hwPod.ProcessAllControls();
	for (size_t i = 0; i < size; i++)
	{
		float sample = in[0][i]; // mono input
		fftMonitor.insertSample(sample);
		out[0][i] = sample;
		out[1][i] = in[1][i];
	}
}

int main(void)
{
	fft.init(FFT_SIZE);
	//fftMonitor.setWindowType(WindowFunction::WELCH);
	//fftMonitor.setWindowType(WindowFunction::BLACKMAN);
	fftMonitor.setWindowType(WindowFunction::HANNING);

	hwPod.Init();
	hwPod.SetAudioBlockSize(4); // number of samples handled per callback
	hwPod.SetAudioSampleRate(SaiHandle::Config::SampleRate::SAI_48KHZ);
	hwPod.seed.StartLog();
	hwPod.StartAdc();
	hwPod.StartAudio(AudioCallback);

	while(1) {
		fftMonitor.processFrame();
		System::Delay(1000); // avoid busy loop
	}

	fft.freeBuffers();	
}
