#if !defined(YIN_DETECTOR_INCLUDED_)
#define YIN_DETECTOR_INCLUDED_

#include "Algorithm.h"

#define YIN_BUFFER_SIZE 2048

class YinDetector : public Algorithm
{

public:
    YinDetector();
    ~YinDetector();

    PitchResult Process(const float* buffer, int bufferSize) override;
    void Init(int samplerate) override;


private:
    int samplerate_;
    int tauMax_;
    int tauMin_;
    float threshold_;

    float d_[YIN_BUFFER_SIZE];
    float d_norm_[YIN_BUFFER_SIZE];

    void computeDifference(const float* buffer, int bufferSize);
    void computeCMNDF();
    int findTau();
    float parabolicInterpolation(int tau);

};


#endif // !defined(YIN_DETECTOR_INCLUDED_)