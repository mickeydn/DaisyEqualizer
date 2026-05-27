#include "AutoCorrelation.h"
#include <math.h>
#include <float.h>

//funktioner baseret på matlab kode til autokorrelation
// refactor nødvendigt for at simplificere og optimere

AutoCorrelation::AutoCorrelation() {}
AutoCorrelation::~AutoCorrelation() {}

void AutoCorrelation::Init(int samplerate)
{
    sampleRate_ = samplerate;
    minLag_     = samplerate / 1000;  // max 1000 Hz
    maxLag_     = samplerate / 60;    // min 60 Hz

    for (int i = 0; i < ACF_BUFFER_SIZE; i++)
        acf_[i] = 0.0f;
}

void AutoCorrelation::removeDC(float* buffer, int bufferSize)
{
    float mean = 0.0f;
    for (int i = 0; i < bufferSize; i++)
        mean += buffer[i];
    mean /= bufferSize;

    for (int i = 0; i < bufferSize; i++)
        buffer[i] -= mean;
}

void AutoCorrelation::computeACF(const float* buffer, int bufferSize)
{
    
    float energy = 0.0f;
    for (int i = 0; i < bufferSize; i++)
        energy += buffer[i] * buffer[i];

    if (energy < 1e-6f) {
        for (int i = 0; i < bufferSize; i++)
            acf_[i] = 0.0f;
        return;
    }

    for (int lag = 0; lag < bufferSize; lag++) {
        float sum = 0.0f;
        for (int j = 0; j < bufferSize - lag; j++)
            sum += buffer[j] * buffer[j + lag];
        acf_[lag] = sum / energy;
    }
}

int AutoCorrelation::findPeakLag()
{
    //nulstil under minimumsværdi
    int bestLag  = -1;
    float bestVal = -FLT_MAX;

    for (int lag = minLag_; lag < maxLag_; lag++) {
        if (acf_[lag] > bestVal) {
            bestVal = acf_[lag];
            bestLag = lag;
        }
    }

    return (bestVal > 0.0f) ? bestLag : -1;
}

PitchResult AutoCorrelation::Process(const float* buffer, int bufferSize)
{
    PitchResult result = {0.0f, 0.0f, false};

    // Lav kopi så vi ikke modificerer original buffer ved DC-fjernelse
    float localBuffer[ACF_BUFFER_SIZE];
    for (int i = 0; i < bufferSize; i++)
        localBuffer[i] = buffer[i];

    removeDC(localBuffer, bufferSize);
    computeACF(localBuffer, bufferSize);

    int lag = findPeakLag();

    if (lag == -1)
        return result;

    result.frequency  = (float)sampleRate_ / lag;
    result.confidence = acf_[lag];  // normaliseret acf peak = naturlig confidence
    result.valid      = true;

    return result;
}