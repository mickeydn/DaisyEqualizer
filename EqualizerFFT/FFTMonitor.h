#pragma once
#include "Window.h"

#define SAMPLE_TIME_US	(FFT_SIZE/(float)SAMPLE_RATE*1000000) // in us

//#define DISPLAY_FFT_TIMING # Define to show timing info instead of full spectrum

#ifdef USE_SDRAM
static float DSY_SDRAM_BSS sampleBuffer[FFT_SIZE];
static float DSY_SDRAM_BSS real[FFT_SIZE];
static float DSY_SDRAM_BSS imag[FFT_SIZE];
#endif

class FFTMonitor
{
	int fftIdx;
	bool newBuffer;
	uint32_t  start, end, dur;
    DaisyPod *pHWPod;
    FFT *pFFT;
	WindowFunction win;

#ifndef USE_SDRAM
	float sampleBuffer[FFT_SIZE];
	float real[FFT_SIZE];
	float imag[FFT_SIZE];
#endif
	float mag[FFT_SIZE/2 + 1];

public:
    FFTMonitor(DaisyPod *hwPod, FFT *fft) : fftIdx(0), newBuffer(false), 
	                                        win(FFT_SIZE, WindowFunction::RECTANGULAR)	
    {  
        pHWPod = hwPod;
        pFFT = fft;
    };

	void insertSample(float sample)
	{
		if (newBuffer)
			return; // skip samples until buffer is processed

		sampleBuffer[fftIdx++] = sample;
		if (fftIdx >= FFT_SIZE)
		{
			fftIdx = 0;
			newBuffer = true;
		}
	}
	
	void setWindowType(WindowFunction::WindowType type)
	{
		win.setWindow(type);
	}

	// Example: compute magnitude spectrum of 1024-sample buffer
	void processFrame(void) 
	{

		if (!newBuffer)
			return; // no new data

#ifdef DISPLAY_FFT_TIMING
		start = System::GetTick();
#endif
		// copy input, imag=0
		for (int i = 0; i < FFT_SIZE; ++i) 
		{ 
			real[i] = sampleBuffer[i]; 
			imag[i] = 0.0f; 
		}
		newBuffer = false; // ready for new samples while computing FFT

		// apply window function
		win.apply(real); 

		// forward FFT
		pFFT->fft(real, imag, false);

		// compute magnitude for bins 0..512
		for (int k = 0; k <= FFT_SIZE/2; ++k) {
			float r = real[k];
			float im = imag[k];
			mag[k] = sqrtf(r*r + im*im);
		}

#ifdef DISPLAY_FFT_TIMING
		end = System::GetTick();
		dur = (end - start) / 200; // us
		float sampleTime = SAMPLE_TIME_US;
		pHWPod->seed.PrintLine("FFT Magnitudes duration %u us (%.0f)", dur, sampleTime);
#else
		pHWPod->seed.PrintLine("#BEGIN");
		pHWPod->seed.PrintLine("Frequency,Amplitude");
		
		for (int k = 0; k <= FFT_SIZE/2; ++k)	
		{
			float freq = k * ((float)SAMPLE_RATE / FFT_SIZE);
			pHWPod->seed.PrintLine("%.1f,%.6f", freq, mag[k]);
		}
		pHWPod->seed.PrintLine("#END");
#endif
			
	}
};
