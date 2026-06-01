#include <string.h>
#include <math.h>
#include "daisy_pod.h"
#include "daisysp.h"
#include "PitchDetector.h"
#include "Controller.h"

using namespace daisy;
using namespace daisysp;

#define USE_HWPOD

#define SAMPLE_RATE         48000
#define SAMPLE_BUFFER_SIZE  8
#define SAMPLE_TIME_NS      (SAMPLE_BUFFER_SIZE / (float)SAMPLE_RATE * 1000000000)

static DaisyPod hwPod;
static uint32_t start, end, dur;

static PitchDetector detectorLeft;
static PitchDetector detectorRight;
static Controller controller(&detectorLeft, &detectorRight, &hwPod);

void AudioCallback(AudioHandle::InputBuffer in, AudioHandle::OutputBuffer out, size_t size)
{
    start = System::GetTick();

    for (size_t i = 0; i < size; i++)
    {
        // Mikrofon/line-in signal processeres
        detectorLeft.Process(in[0][i]);
        detectorRight.Process(in[1][i]);

        // Pass-through så man kan høre input
        out[0][i] = in[0][i];
        out[1][i] = in[1][i];
    }

    end = System::GetTick();
    dur = (end - start) * 5; // ns
}


int main(void)
{
    detectorLeft.Init(SAMPLE_RATE);
    detectorLeft.setBypass(true);
    detectorRight.Init(SAMPLE_RATE);
    detectorRight.setBypass(true);

    int counter = 0;

    hwPod.Init();
    hwPod.seed.StartLog();
    hwPod.SetAudioBlockSize(SAMPLE_BUFFER_SIZE);

    if (SAMPLE_RATE == 48000)
        hwPod.SetAudioSampleRate(SaiHandle::Config::SampleRate::SAI_48KHZ);
    else if (SAMPLE_RATE == 96000)
        hwPod.SetAudioSampleRate(SaiHandle::Config::SampleRate::SAI_96KHZ);
    else
        hwPod.seed.PrintLine("Unsupported sample rate %d", SAMPLE_RATE);

    controller.Init();

    hwPod.StartAdc();
    hwPod.StartAudio(AudioCallback);

    while (1)
    {
        controller.Process();

        if (counter % 100000 == 0) {
            controller.printState(dur, SAMPLE_TIME_NS);
        }
        counter++;
    }

}