#include "daisy_pod.h"
#include "daisysp.h"
#include "Compressor.h"

Compressor compressorLeft;
Compressor compressorRight;

using namespace daisy;
using namespace daisysp;

DaisyPod hwPod;

bool bypass = true; // bypass flag

#define NUM_COLORS 				5
static Color my_colors[NUM_COLORS];

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
			out[0][i] = compressorLeft.Process(sampleLeft);
			out[1][i] = compressorRight.Process(sampleRight);
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

	compressorLeft.setSampleRate(hwPod.AudioSampleRate());
	compressorRight.setSampleRate(hwPod.AudioSampleRate());
	compressorLeft.setThresholdDb(-18.0f);
	compressorRight.setThresholdDb(-18.0f);
	compressorLeft.setRatio(3.0f);
	compressorRight.setRatio(3.0f);
	compressorLeft.setReleaseMs(150.0f);
	compressorRight.setReleaseMs(150.0f);
	compressorLeft.setAttackMs(8.0f);
	compressorRight.setAttackMs(8.0f);
	compressorLeft.setKneeDb(3.0f);
	compressorRight.setKneeDb(3.0f);
	compressorLeft.setMakeupDb(2.0f);
	compressorRight.setMakeupDb(2.0f);

	hwPod.StartAdc();
	hwPod.StartAudio(AudioCallback);

	while(1) 
	{
		hwPod.ProcessAnalogControls();
    	hwPod.ProcessDigitalControls();

		/*
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
		*/

		        // using button1 turn EQ on/off
        if (hwPod.button1.RisingEdge()) {
			bypass = !bypass;
		}

		hwPod.ClearLeds();
		int BypassOff = bypass ? 0 : 4; 
		//int Size = roomSize*5;
		hwPod.led1.SetColor(my_colors[BypassOff]);
		//hwPod.led2.SetColor(my_colors[Size]);
		hwPod.UpdateLeds();
		
		//System::Delay(10);
	}
}