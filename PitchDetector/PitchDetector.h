#if !defined(PITCH_DETECTOR_INCLUDED_)
#define PITCH_DETECTOR_INCLUDED_
#include "Algorithm.h"
#include "YinDetector.h"
#include "AutoCorrelation.h"

#define PITCH_BUFFER_SIZE 2048

typedef enum {
    ALG_YIN,
    ALG_AUTOCORRELATION,
} PitchAlgorithmSelect;

class PitchDetector
{   

public:
    PitchDetector();
    ~PitchDetector();

    PitchResult Process(float input);
    void Init(int samplerate);
    void setAlgorithm(PitchAlgorithmSelect alg);
    PitchAlgorithmSelect getAlgorithm();
    PitchResult getLastResult();
    void setBypass(bool bypass);

private:
    int samplerate_;
    bool bypass_;
    PitchAlgorithmSelect selectedAlgorithm_;

    float buffer_[PITCH_BUFFER_SIZE];
    int bufferIndex_;

    PitchResult lastResult_;

    YinDetector m_yin;
    AutoCorrelation m_autocorr;
    Algorithm* m_activeAlgorithm;
};









#endif // !defined(PITCH_DETECTOR_INCLUDED_)