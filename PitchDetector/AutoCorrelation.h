#if !defined(AUTOCORRELATION_INCLUDED_)
#define AUTOCORRELATION_INCLUDED_

#include "Algorithm.h"

#define ACF_BUFFER_SIZE 2048

class AutoCorrelation : public Algorithm {
public:
    AutoCorrelation();
    ~AutoCorrelation();

    void Init(int samplerate) override;
    PitchResult Process(const float* buffer, int bufferSize) override;

private:
    int sampleRate_;
    int minLag_;
    int maxLag_;

    float acf_[ACF_BUFFER_SIZE];    // autokorrelation

    void removeDC(float* buffer, int bufferSize);
    void computeACF(const float* buffer, int bufferSize);
    int findPeakLag();
};

#endif