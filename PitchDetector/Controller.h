#if !defined(CONTROLLER__INCLUDED_)
#define CONTROLLER__INCLUDED_

#include "daisy_pod.h"
#include "PitchDetector.h"

using namespace daisy;

class Controller
{

public:

	Controller(PitchDetector* pitchDetectorLeft, PitchDetector* pitchDetectorRight, DaisyPod* daisyPod);
	~Controller();

	void Init();
	void Process();
	void printState(uint32_t dur, float sampleTimeNs);
	void setBypass(bool bypass);


private:
	PitchDetector* m_pitchDetectorLeft;
	PitchDetector* m_pitchDetectorRight;
	DaisyPod* m_daisyPod;

	bool bypass_;
	PitchAlgorithmSelect currentAlgorithm_;
	Color colorYIN_;
    Color colorACF_;
    Color colorActive_;
    Color colorOff_;
	void updateLEDs();
};

#endif // !defined(CONTROLLER__INCLUDED_)
