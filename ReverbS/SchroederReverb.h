// SchroederReverb.h
// Single-file Schroeder Reverb for real-time STM32 (float)
// (adapted for STM32) - example implementation
// Kim Bjerge - 29. November 2025

#pragma once
#include <cstring> // memset

// === Configuration ===
// Max delay buffer length in samples (tune for your application).
// At 48 kHz, 48000 = 1s which is plenty for Schroeder comb delays.
// Adjust down to reduce RAM.
#ifndef SRV_MAX_DELAY_SAMPLES
#define SRV_MAX_DELAY_SAMPLES 48000
#endif

// Internal number of comps filters and all-pass filters
#ifndef NUM_COMBS
#define NUM_COMBS 4 
#endif
#ifndef NUM_ALLP
#define NUM_ALLP  2
#endif

// === Internal comb filter ===
struct CombFilter
{
    //float buffer[SRV_MAX_DELAY_SAMPLES];
    float *buffer;
    int size = 0;
    int idx = 0;
    // simple one-pole lowpass in loop (damping)
    float filterStore = 0.0f;
    float damp1 = 0.0f; // a
    float damp2 = 0.0f; // 1-a

public:
    float feedback = 0.0f;

    void init(int delaySamples, float fb, float damping) 
    {
        size = delaySamples > 0 ? delaySamples : 1;
        idx = 0;
        feedback = fb;
        filterStore = 0.0f;
        setDamping(damping);
        memset(buffer, 0, sizeof(float) * size);
    }
    
    inline float clamp(float v, float minVal, float maxVal) 
    {
        v = (v > minVal) ? v : minVal;
        v = (v < maxVal) ? v : maxVal;
        return v;
    }

    void setDamping(float d) 
    {
        // damp1 = a, damp2 = 1-a
        damp1 = clamp(d, 0.0f, 1.0f);
        damp2 = 1.0f - damp1;
    }

    void reset(float *pBuffer) 
    {
        buffer = pBuffer;
        if (size > 0) memset(buffer, 0, sizeof(float)*size);
        idx = 0; filterStore = 0.0f;
    }

    float process(float in) 
    {
        // read delayed value
        float out = buffer[idx];
        // apply simple lowpass in feedback path
        filterStore = (out * damp2) + (filterStore * damp1);
        // compute feedback sample to write
        buffer[idx] = in + filterStore * feedback;
        // advance pointer
        if (++idx >= size) idx = 0;
        return out;
    }
};

// === Internal all-pass filter ===
struct AllpassFilter
{
    float *buffer;
    int size = 0;
    int idx = 0;
    float feedback = 0.5f; // typically around 0.5-0.7

public:

    void init(int delaySamples, float fb) 
    {
        size = delaySamples > 0 ? delaySamples : 1;
        idx = 0;
        feedback = fb;
        memset(buffer, 0, sizeof(float)*size);
    }

    void reset(float *pBuffer) 
    {
        buffer = pBuffer;
        if (size > 0) memset(buffer, 0, sizeof(float)*size);
        idx = 0;
    }

    float process(float in) 
    {
        float bufOut = buffer[idx];
        float out = -in + bufOut;
        buffer[idx] = in + (bufOut * feedback);
        if (++idx >= size) idx = 0;
        return out;
    }
};

// === Schroeder Reverb Class ===
class SchroederReverb 
{
public:
    // Constructor (sampleRate default 48000)
    SchroederReverb(float sampleRate = 48000.0f)
    : sr(sampleRate)
    {
        initDefaults();
        sr = sampleRate;
        //setSampleRate(sampleRate);
    }

    // Set sample rate (recomputes delay lengths)
    void setSampleRate(float sampleRate) 
    {
        sr = sampleRate;
        computeDelays();
        resetBuffers();
    }

    // Set high-level controls
    // roomSize: 0..1 (controls comb feedback)
    // damping: 0..1 (controls comb damping / lowpass in loop)
    // wet: 0..1 (wet mix)
    // dry: 0..1 (dry mix)
    void setRoomSize(float roomSize)    { roomSizeParam = clamp(roomSize,0.0f,1.0f); updateCombFeedbacks(); }
    void setDamping(float d)            { damping = clamp(d,0.0f,1.0f); updateCombDamping(); }
    void setWet(float w)                { wet = clamp(w,0.0f,1.0f); }

    // Process a single sample (mono). Call from ISR or process loop.
    // Returns processed sample (wet+dry mixed).
    inline float Process(float in) 
    {
        // sum outputs of comb filters
        float combSum = 0.0f;
        for (int i = 0; i < NUM_COMBS; ++i) {
            combSum += combs[i].process(in);
        }

        // put through series allpass filters
        float apOut = combSum;
        for (int i = 0; i < NUM_ALLP; ++i) {
            apOut = allpasses[i].process(apOut);
        }

        // mix wet/dry
        return (1.0 - wet) * in + wet * apOut;
    }

    // Process a block of samples in-place (mono)
    void processBlock(float* buf, uint32_t numSamples) 
    {
        for (uint32_t i = 0; i < numSamples; ++i) {
            buf[i] = Process(buf[i]);
        }
    }

    // Set external buffers in SDRAM for internal filters
    void setBuffers(float* bufComp, float* bufAllPass)
    {
        bufferComp = bufComp;
        bufferAllPass = bufAllPass;
        resetBuffers();
    }

    // Reset internal buffers and state
    void resetBuffers() 
    {
        for (int i = 0; i < NUM_COMBS; ++i) 
            combs[i].reset(&bufferComp[SRV_MAX_DELAY_SAMPLES*i]);
        for (int i = 0; i < NUM_ALLP; ++i) 
            allpasses[i].reset(&bufferAllPass[SRV_MAX_DELAY_SAMPLES*i]);
    }

private:
    // Internal filter counts
    //static constexpr int NUM_COMBS = 4;
    //static constexpr int NUM_ALLP  = 2;
    float *bufferComp;
    float *bufferAllPass;

    // Member instances
    CombFilter combs[NUM_COMBS];
    AllpassFilter allpasses[NUM_ALLP];

    // parameters
    float sr = 48000.0f;
    float roomSizeParam = 0.4f; // 0..1 -> maps to comb feedback
    float damping = 0.2f;       // lowpass in comb loop
    float wet = 0.33f;  

    // default delay times (ms) for combs and allpasses (a common Schroeder choice)
    float combDelMs[NUM_COMBS] = {50.0f, 56.0f, 61.0f, 68.0f};
    float allpassDelMs[NUM_ALLP] = {12.0f, 6.0f};

    // computed sample delays
    int combDelaySamples[NUM_COMBS];
    int allpassDelaySamples[NUM_ALLP];

    // Utility helpers
    inline float clamp(float v, float a, float b) 
    {
        if (v < a) return a;
        if (v > b) return b;
        return v;
    }

    void initDefaults() 
    {
        roomSizeParam = 0.4f;
        damping = 0.2f;
        wet = 0.33f;
    }

    // Map roomSize param -> comb feedback (tuned range)
    float roomSizeToFeedback(float rs) 
    {
        // feedback range from ~0.70 .. 0.985 (tunable)
        const float minFb = 0.70f;
        const float maxFb = 0.985f;
        return minFb + (maxFb - minFb) * clamp(rs, 0.0f, 1.0f);
    }

    void updateCombFeedbacks() 
    {
        float fb = roomSizeToFeedback(roomSizeParam);
        for (int i = 0; i < NUM_COMBS; ++i) combs[i].feedback = fb;
    }

    void updateCombDamping() 
    {
        for (int i = 0; i < NUM_COMBS; ++i) combs[i].setDamping(damping);
    }

    // compute integer delay sample counts from ms arrays
    void computeDelays() 
    {
        for (int i = 0; i < NUM_COMBS; ++i) {
            combDelaySamples[i] = (int)std::round((combDelMs[i] * sr) / 1000.0f);
            if (combDelaySamples[i] < 1) combDelaySamples[i] = 1;
            if (combDelaySamples[i] > SRV_MAX_DELAY_SAMPLES) combDelaySamples[i] = SRV_MAX_DELAY_SAMPLES;
        }
        for (int i = 0; i < NUM_ALLP; ++i) {
            allpassDelaySamples[i] = (int)std::round((allpassDelMs[i] * sr) / 1000.0f);
            if (allpassDelaySamples[i] < 1) allpassDelaySamples[i] = 1;
            if (allpassDelaySamples[i] > SRV_MAX_DELAY_SAMPLES) allpassDelaySamples[i] = SRV_MAX_DELAY_SAMPLES;
        }

        // initialize filter buffers with computed sizes and default params
        float fb = roomSizeToFeedback(roomSizeParam);
        for (int i = 0; i < NUM_COMBS; ++i) {
            combs[i].init(combDelaySamples[i], fb, damping);
        }
        // Allpass feedback often around 0.5 - 0.7
        allpasses[0].init(allpassDelaySamples[0], 0.5f);
        allpasses[1].init(allpassDelaySamples[1], 0.5f);
    }
};
