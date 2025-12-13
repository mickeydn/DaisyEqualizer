// Kim Bjerge - 29. November 2025

#include "daisy_pod.h"
#include "daisysp.h"

#define SRV_MAX_DELAY_SAMPLES 48000
#define NUM_COMBS 4 
#define NUM_ALLP  2
#include "SchroederReverb.h"

SchroederReverb reverbLeft;
SchroederReverb reverbRight;

static float DSY_SDRAM_BSS bufferCompLeft[SRV_MAX_DELAY_SAMPLES*NUM_COMBS];
static float DSY_SDRAM_BSS bufferAllPassLeft[SRV_MAX_DELAY_SAMPLES*NUM_ALLP];
static float DSY_SDRAM_BSS bufferCompRight[SRV_MAX_DELAY_SAMPLES*NUM_COMBS];
static float DSY_SDRAM_BSS bufferAllPassRight[SRV_MAX_DELAY_SAMPLES*NUM_ALLP];

using namespace daisy;
using namespace daisysp;

DaisyPod hwPod;

#define NUM_COLORS 				5
static Color my_colors[NUM_COLORS];

float wetMix = 0.33f; // reverb mix gain
float wetMixPrev = 0.0f; 
float roomSize = 0.25f; // reverb room size
float roomSizePrev = 0.0f;
float damping = 0.3f; // damping
float dampingPrev = 0.0f;

bool bypass = true; // bypass flag

void AudioCallback(AudioHandle::InputBuffer in, AudioHandle::OutputBuffer out, size_t size)
{
	hwPod.ProcessAllControls();
	for (size_t i = 0; i < size; i++)
	{
		if (bypass) {
			out[0][i] = in[0][i];
			out[1][i] = in[1][i];
		} else {
			float sampleLeft = in[0][i];
			float sampleRight = in[1][i];
			out[0][i] = reverbLeft.Process(sampleLeft);
			out[1][i] = reverbRight.Process(sampleRight);
		}
	}
}

int main(void)
{
    my_colors[0].Init(Color::PresetColor::OFF);
    my_colors[1].Init(Color::PresetColor::GREEN);
    my_colors[2].Init(Color::PresetColor::BLUE);
	my_colors[3].Init(Color::PresetColor::RED);
    my_colors[4].Init(Color::PresetColor::WHITE);

	hwPod.Init();
	hwPod.SetAudioBlockSize(4); // number of samples handled per callback
	hwPod.SetAudioSampleRate(SaiHandle::Config::SampleRate::SAI_48KHZ);

	reverbLeft.setBuffers(bufferCompLeft, bufferAllPassLeft);
	reverbLeft.setSampleRate(hwPod.AudioSampleRate());
	reverbRight.setBuffers(bufferCompRight, bufferAllPassRight);
	reverbRight.setSampleRate(hwPod.AudioSampleRate());
	
	hwPod.StartAdc();
	hwPod.StartAudio(AudioCallback);

	while(1) 
	{
		hwPod.ProcessAnalogControls();
    	hwPod.ProcessDigitalControls();

	    wetMix = hwPod.knob1.Process();
		if (wetMix != wetMixPrev)
		{	
			reverbLeft.setWet(wetMix);
			reverbRight.setWet(wetMix);
			wetMixPrev = wetMix;
		}

	    roomSize = hwPod.knob2.Process();
		if (roomSize != roomSizePrev)
		{
			reverbLeft.setRoomSize(roomSize);
			reverbRight.setRoomSize(roomSize);
			roomSizePrev = roomSize;
		}

		//damping = hwPod.knob2.Process(); // damping
		if (damping != dampingPrev)
		{
			reverbLeft.setDamping(damping);
			reverbRight.setDamping(damping); 
			dampingPrev = damping;
		}

		        // using button1 turn EQ on/off
        if (hwPod.button1.RisingEdge()) {
			bypass = !bypass;
		}

		hwPod.ClearLeds();
		int BypassOff = bypass ? 0 : 4; 
		int Size = roomSize*5;
		hwPod.led1.SetColor(my_colors[BypassOff]);
		hwPod.led2.SetColor(my_colors[Size]);
		hwPod.UpdateLeds();
		
		//System::Delay(10);
	}
}

