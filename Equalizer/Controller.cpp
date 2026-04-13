
#include "Controller.h"

Controller::Controller(Equalizer *eqLeft, Equalizer *eqRight, DaisyPod *daisyPod) : m_band(0)
{
	m_eqLeft = eqLeft;
	m_eqRight = eqRight;
	m_daisyPod = daisyPod; 
}

Controller::~Controller()
{

}

float Controller::incBand()
{
	// Selects the next EQ band
	m_band++;
	if (m_band >= NUM_EQ_BANDS)
		m_band = 0;

	return (float)m_band;
}

float Controller::incParam()
{
	// TODO Optional to insert code to select next parameter in algorithm
	return 0.0;
}

float Controller::incParamValue()
{
	// Increments parameter value for PM_GAIN
	m_eqLeft->incParameter(m_band, PM_GAIN, 0.1);
	float val = m_eqRight->incParameter(m_band, PM_GAIN, 0.1);
	return val;
}

float Controller::decParamValue()
{
	// Decrements parameter value for PM_GAIN
	m_eqLeft->decParameter(m_band, PM_GAIN, 0.1);
	float val = m_eqRight->decParameter(m_band, PM_GAIN, 0.1);
	return val;
}

float Controller::adjust(short select)
{
	float val = 0.0;

	switch (select) {
		case INC_PARAM_VALUE:
			val = incParamValue(); // Increment selected parameter value
			break;
		case DEC_PARAM_VALUE:
			val = decParamValue(); // Decrement selected parameter value
			break;
		case SEL_PARAM:
			val = incParam(); // Select parameter - optional not implemented
			break;
		case SEL_BAND:
			val = incBand(); // Select equalizer band to adjust
			break;
	}

	return val;
}

void Controller::setBypass(bool bypass) 
{
	m_eqLeft->setBypass(bypass);
	m_eqRight->setBypass(bypass);
}	

void Controller::printState(uint32_t duration, float sampleTimeNs) 
{
	float val, freq, q;
	val = m_eqLeft->getParameter(m_band, PM_GAIN);
	freq = m_eqLeft->getParameter(m_band, PM_FREQ);
	q = m_eqLeft->getParameter(m_band, PM_Q);
	m_daisyPod->seed.Print("  Band %d Freq %.0f Q %.0f Gain %.1f Duration %u ns (%.0f ns)        \r", m_band, freq, q, val, duration, sampleTimeNs);
}
