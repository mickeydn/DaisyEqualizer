#include "PitchDetector.h"

PitchDetector::PitchDetector() :samplerate_(0),bypass_(false), selectedAlgorithm_(ALG_YIN), bufferIndex_(0)
{
    lastResult_ = {0.0f, 0.0f, false};
}

PitchDetector::~PitchDetector() {}


void PitchDetector::Init(int samplerate) 
{
    samplerate_ = samplerate;
    m_yin.Init(samplerate);
    m_autocorr.Init(samplerate);
    m_activeAlgorithm = &m_yin; // Default to Yin algorithm

    for (int i = 0; i < PITCH_BUFFER_SIZE; i++)
        buffer_[i] = 0.0f;
}

void PitchDetector::setAlgorithm(PitchAlgorithmSelect alg) 
{
    selectedAlgorithm_ = alg;
    switch (selectedAlgorithm_) {
        case ALG_YIN:
            m_activeAlgorithm = &m_yin;
            break;
        case ALG_AUTOCORRELATION:
            m_activeAlgorithm = &m_autocorr;
            break;
    }
}

PitchAlgorithmSelect PitchDetector::getAlgorithm()
{ return selectedAlgorithm_; }

PitchResult PitchDetector::getLastResult()
{ return lastResult_; }

void PitchDetector::setBypass(bool bypass) 
{
    bypass_ = bypass;
}

PitchResult PitchDetector::Process(float input)
{ 
    if (bypass_) {
        lastResult_ = {0.0f, 0.0f, false};
        return lastResult_;
    }
    buffer_[bufferIndex_++] = input;

    if (bufferIndex_ >= PITCH_BUFFER_SIZE) {
        lastResult_ = m_activeAlgorithm->Process(buffer_, PITCH_BUFFER_SIZE);
        bufferIndex_ = 0; // Reset buffer index for next block
    }

    return lastResult_;
}