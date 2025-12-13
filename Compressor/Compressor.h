// Compressor.h
// Simple feed-forward compressor for real-time STM32 audio
// example implementation
// Kim Bjerge - 29. November 2025
/*

Compressor features:
- Peak / RMS style envelope detector (squared RMS with exponential smoothing)
- Attack and release time constants (ms) → converted to smoothing coefficients
- Threshold (dB), ratio, knee width (dB) — supports soft knee
- Makeup gain (dB) and output gain

Detector choice (RMS vs Peak): RMS (squared average + sqrt) yields smoother behavior; 
peak reacts faster. The class supports both via setDetectorRms(). 
You can also implement a true moving-window RMS for more stable metering.

Look-ahead: For transparent limiting you may want a small look-ahead implemented 
with a short FIFO (adds latency).

Stereo: For stereo material, run a single detector on the max(L,R) level and apply 
the same gain to both channels to avoid stereo imbalance.

Fixed-point / CMSIS: For MCUs without FPU, consider converting powf, log10f, sqrtf 
usage to CMSIS-DSP or table approximations. 
The dbToLin/linToDb functions use powf/log10f—these cost CPU; 
you can instead approximate using fast exp/log or operate directly on linear domain 
using ratio formula.

Performance: Avoid calling linToDb() if you redesign to compute gain using linear 
domain—this reduces expensive log10f/powf calls. 
Many compressors use a linear detector + precomputed lookup tables for gain curve.

*/
#pragma once
#include <cstdint>
#include <cmath>
#include <cstring>

class Compressor {
public:
    Compressor(float sampleRate = 48000.0f)
    : sr(sampleRate)
    {
        setSampleRate(sr);
        // reasonable defaults
        setThresholdDb(-24.0f);
        setRatio(4.0f);
        setAttackMs(10.0f);
        setReleaseMs(100.0f);
        setKneeDb(0.0f);        // 0 = hard knee
        setMakeupDb(0.0f);
        setDetectorRms(true);   // use RMS detector by default
        reset();
    }

    // ---------- configuration ----------
    void setSampleRate(float sampleRate) 
    {
        sr = sampleRate;
        updateTimeConstants();
    }
    void setThresholdDb(float db) { thresholdDb = db; }
    void setRatio(float r) { ratio = (r <= 1.0f ? 1.0f : r); } // ratio >= 1
    void setAttackMs(float ms) { attackMs = (ms < 0.01f ? 0.01f : ms); updateTimeConstants(); }
    void setReleaseMs(float ms) { releaseMs = (ms < 0.01f ? 0.01f : ms); updateTimeConstants(); }
    void setKneeDb(float db) { kneeDb = (db < 0.0f ? 0.0f : db); } // >= 0
    void setMakeupDb(float db) { makeupDb = db; makeupLin = dbToLin(db); }
    void setDetectorRms(bool useRms) { detectorRms = useRms; } // if false -> peak detector

    // Optional: set minimum detector level floor to avoid log(0)
    void setMinLevel(float minL) { minLevel = (minL>0.0f?minL:1e-12f); }

    // ---------- processing ----------
    // Process single sample (mono)
    inline float Process(float in) 
    {
        // 1) compute detector level (RMS or peak)
        float rect = in * in; // squared signal (for RMS style)
        if (!detectorRms) rect = fabsf(in); // peak detector uses abs

        // smooth detector: separate attack and release smoothing
        if (rect > env) {
            // attack
            env += (rect - env) * attackCoeff;
        } else {
            // release
            env += (rect - env) * releaseCoeff;
        }

        // convert env to dB (for RMS use sqrt)
        float levelLin = detectorRms ? sqrtf(env) : env;
        float levelDb = linToDb(levelLin + minLevel); // protect against zero

        // 2) compute required gain in dB using soft/hard knee and ratio
        float gainDb = computeGainDb(levelDb);

        // 3) apply makeup and linearize
        float gainLin = dbToLin(gainDb) * makeupLin;

        // 4) output
        return in * gainLin;
    }

    // Process buffer in-place
    void processBlock(float* buf, uint32_t numSamples) 
    {
        for (uint32_t i = 0; i < numSamples; ++i) buf[i] = Process(buf[i]);
    }

    // Reset detector and state
    void reset() 
    {
        env = 0.0f;
        makeupLin = dbToLin(makeupDb);
    }

private:
    // ---------- internal parameters ----------
    float sr = 48000.0f;
    float thresholdDb = -24.0f; // dBFS
    float ratio = 4.0f;         // >=1
    float attackMs = 10.0f;
    float releaseMs = 100.0f;
    float kneeDb = 0.0f;
    float makeupDb = 0.0f;
    float makeupLin = 1.0f;
    bool detectorRms = true;    // RMS (true) or peak (false)
    float minLevel = 1e-12f;    // avoid log(0)

    // envelope/detector state
    float env = 0.0f;           // smoothed rectified (squared) value or abs value
    float attackCoeff = 0.0f;
    float releaseCoeff = 0.0f;

    // ---------- helpers ----------
    static inline float dbToLin(float db) 
    {
        return powf(10.0f, db * 0.05f); // 20*log10 -> factor 0.05
    }
    static inline float linToDb(float lin) 
    {
        // protect against zero or negative
        return 20.0f * log10f(lin);
    }
    static inline float clampf(float v, float a, float b) 
    {
        if (v < a) return a;
        if (v > b) return b;
        return v;
    }

    // Compute attack/release smoothing coefficients for one-pole smoothing
    void updateTimeConstants() 
    {
        // convert ms to seconds
        float atSec = attackMs * 0.001f;
        float rtSec = releaseMs * 0.001f;
        // alpha = 1 - exp(-1/(tau * fs))
        // use this formulation to compute step smoothing toward input: env += (x - env) * alpha
        attackCoeff = 1.0f - expf(-1.0f / (atSec * sr + 1e-12f));
        releaseCoeff = 1.0f - expf(-1.0f / (rtSec * sr + 1e-12f));
        // clamp to (0,1)
        attackCoeff = clampf(attackCoeff, 0.000001f, 0.999999f);
        releaseCoeff = clampf(releaseCoeff, 0.000001f, 0.999999f);
    }

    // Compute gain (in dB) that should be applied to the signal given current levelDb
    // The returned gainDb is typically <= 0 (i.e., attenuation). For make-up we multiply later.
    float computeGainDb(float levelDb) 
    {
        // Soft knee handling:
        // For level below threshold - knee/2 -> no compression (gainDb = 0)
        // For level above threshold + knee/2 -> apply hard ratio
        // Inside knee -> smooth quadratic interpolation (as in common compressors)
        float thr = thresholdDb;
        float halfK = kneeDb * 0.5f;

        float gainDb = 0.0f;

        if (kneeDb > 0.0f) {
            // soft knee region boundaries
            float x = levelDb - thr;
            if (x <= -halfK) {
                gainDb = 0.0f; // below knee
            } else if (x > halfK) {
                // above knee: classic ratio conversion
                float exceeded = levelDb - thr;
                float compressed = thr + exceeded / ratio;
                gainDb = compressed - levelDb; // negative or zero
            } else {
                // inside soft knee => apply smooth transition (quadratic)
                // following CSS-style formula: see many compressor implementations
                // compute local point in knee range -halfK..halfK mapped to 0..1
                float t = (x + halfK) / (kneeDb); // 0..1
                // desired output level if fully compressed: thr + exceeded/ratio
                float exceeded = levelDb - thr;
                float compFull = thr + exceeded / ratio;
                float linearOut = levelDb; // no compression baseline
                // interpolate between no compression and full compression using smoothstep-like curve
                float out = linearOut * (1.0f - t) + compFull * t;
                // but to make curve smoother we use t*t*(3-2*t) (smoothstep) optionally
                float s = t * t * (3.0f - 2.0f * t);
                out = linearOut * (1.0f - s) + compFull * s;
                gainDb = out - levelDb;
            }
        } else {
            // hard knee
            if (levelDb <= thr) {
                gainDb = 0.0f;
            } else {
                float exceeded = levelDb - thr;
                float compressed = thr + exceeded / ratio;
                gainDb = compressed - levelDb;
            }
        }
        // clamp gainDb to reasonable range
        gainDb = clampf(gainDb, -60.0f, 12.0f);
        return gainDb;
    }
};
