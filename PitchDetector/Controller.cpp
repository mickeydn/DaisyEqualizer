#include "Controller.h"

Controller::Controller(PitchDetector *pitchDetectorLeft,
                       PitchDetector *pitchDetectorRight,
                       DaisyPod      *daisyPod) : m_pitchDetectorLeft(pitchDetectorLeft),
												 m_pitchDetectorRight(pitchDetectorRight),
												 m_daisyPod(daisyPod),
												 bypass_(true),
												 currentAlgorithm_(ALG_YIN)
{
}

Controller::~Controller() {}

void Controller::Init() 
{
	colorYIN_.Init(Color::PresetColor::GREEN);
	colorACF_.Init(Color::PresetColor::BLUE);
	colorActive_.Init(Color::PresetColor::WHITE);
	colorOff_.Init(Color::PresetColor::OFF);

}

void Controller::printState(uint32_t dur, float sampleTimeNs) 
{
	PitchResult result = m_pitchDetectorLeft->getLastResult();

	const char* algName = (currentAlgorithm_ == ALG_YIN) ? "YIN" : "ACF";

	if (bypass_) {
		m_daisyPod->seed.PrintLine("Bypass enabled. No pitch detection.");
	}

	if (result.valid){
		m_daisyPod->seed.PrintLine("[%s] Pitch: %.2f Hz | Confidence: %.2f",
            algName,
            result.frequency,
            result.confidence
            );
	}
	else {
		m_daisyPod->seed.PrintLine("[%s] No pitch detected.",
			algName);
	}

}

void Controller::setBypass(bool bypass) {
	bypass_ = bypass;
	m_pitchDetectorLeft->setBypass(bypass);
	m_pitchDetectorRight->setBypass(bypass);
	
}

void Controller::updateLEDs() {
	// LED1 viser valgt algoritme
    m_daisyPod->led1.SetColor(currentAlgorithm_ == ALG_YIN ? colorYIN_ : colorACF_);

    // LED2 viser om detection er aktiv
    m_daisyPod->led2.SetColor(bypass_ ? colorOff_ : colorActive_);

    m_daisyPod->UpdateLeds();
}

void Controller::Process() 
{
	 m_daisyPod->ProcessAllControls();

    // SW1 er YIN
    if (m_daisyPod->button1.RisingEdge()) {
        if (bypass_) {
            setBypass(false);
            currentAlgorithm_ = ALG_YIN;
        } else if (currentAlgorithm_ != ALG_YIN) {
            currentAlgorithm_ = ALG_YIN;
        } else {
            
            setBypass(true);
        }
        m_pitchDetectorLeft->setAlgorithm(currentAlgorithm_);
        m_pitchDetectorRight->setAlgorithm(currentAlgorithm_);
    }

    // autororelation
    if (m_daisyPod->button2.RisingEdge()) {
        if (bypass_) {
            setBypass(false);
            currentAlgorithm_ = ALG_AUTOCORRELATION;
        } else if (currentAlgorithm_ != ALG_AUTOCORRELATION) {
            currentAlgorithm_ = ALG_AUTOCORRELATION;
        } else {
            
            setBypass(true);
        }
        m_pitchDetectorLeft->setAlgorithm(currentAlgorithm_);
        m_pitchDetectorRight->setAlgorithm(currentAlgorithm_);
    }

    updateLEDs();


}