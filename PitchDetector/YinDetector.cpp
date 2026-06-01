#include "YinDetector.h"
#include <math.h>



YinDetector::YinDetector() {}

YinDetector::~YinDetector() {}

void YinDetector::Init(int samplerate) {
    samplerate_ = samplerate;
    tauMin_ = samplerate_ / 600; //  periode for 1200 Hz
    tauMax_ = samplerate_ / 60; // periode for 60 Hz i samples
    threshold_ = 0.2f; // Threshold

    for (int i = 0; i < YIN_BUFFER_SIZE; i++) {
        d_[i] = 0.0f;
        d_norm_[i] = 0.0f; 
    }

}

void YinDetector::computeDifference(const float *buffer, int bufferSize) {
    for (int tau = 0; tau < tauMax_; tau++) {
        float sum = 0.0f;
        for (int i = 0; i < bufferSize - tau; i++) {
            float diff = buffer[i] - buffer[i + tau];
            sum += diff * diff;
        }
        d_[tau] = sum;
    }
}

void YinDetector::computeCMNDF() {
    d_norm_[0] = 1.0f;
    float cumsum = 0.0f;
    for (int tau = 1; tau < tauMax_; tau++) {
        cumsum += d_[tau];
        d_norm_[tau] = (cumsum > 0.0f) ? d_[tau] / (cumsum / tau) : 0.0f;
    }
}

int YinDetector::findTau()
{
    int tau = tauMin_ + 1; // starter ved tauMin + 1 så vi kan tjekke tau-1

    while (tau < tauMax_ - 1) {
        bool isDip = d_norm_[tau] < d_norm_[tau - 1] &&
                     d_norm_[tau] < threshold_;

        if (isDip) {
            // følg ned til lokalt minimum
            while (tau + 1 < tauMax_ && d_norm_[tau + 1] < d_norm_[tau]) {
                tau++;
            }
            return tau;
        }
        tau++;
    }
    return -1; // ingen pitch fundet
}

 float YinDetector::parabolicInterpolation(int tau){
        if (tau <= 1 || tau >= tauMax_ - 1) {
            return float(tau); // No interpolation possible
        }
        float a = d_norm_[tau - 1];
        float b = d_norm_[tau];
        float c = d_norm_[tau + 1];
        float denom = 2.0f * b - a - c;

        if (fabs(denom) < 1e-6f) {
            return float(tau); // Avoid division by zero
        }
        return float(tau) + 0.5f * (a - c) / denom;


  }


 PitchResult YinDetector::Process(const float *buffer, int bufferSize)
 {
    PitchResult result = {0.0f, 0.0f, false};

    computeDifference(buffer, bufferSize);
    computeCMNDF();

    int tau = findTau();

    if (tau == -1){
        return result;
    }
    
    float tauRefined = parabolicInterpolation(tau);

    result.frequency   = samplerate_ / tauRefined;
    result.confidence  = 1.0f - d_norm_[tau]; // høj confidence = lav d_norm
    result.valid       = true;

    return result;

 }
