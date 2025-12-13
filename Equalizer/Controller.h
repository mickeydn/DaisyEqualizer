///////////////////////////////////////////////////////////
//  Controller.h
//  Implementation of the Class Controller
//  Created on:      06-aug-2014 23:31:01
//  Modified:        27-nov-2025 20:01:00
//  Original author: kbe
///////////////////////////////////////////////////////////

#if !defined(CONTROLLER__INCLUDED_)
#define CONTROLLER__INCLUDED_

#include "daisy_pod.h"
#include "Equalizer.h"

using namespace daisy;

#define INC_PARAM_VALUE 0
#define DEC_PARAM_VALUE 1
#define SEL_PARAM 2
#define SEL_BAND 3

class Controller
{

public:

	Controller(Equalizer *eqLeft, Equalizer *eqRight, DaisyPod *daisyPod);
	virtual ~Controller();

	float incBand();
	float incParam();
	float incParamValue();
	float decParamValue();
	void setBypass(bool bypass);
	float adjust(short select);
	void printState(uint32_t duration, float sampleTimeNs);

protected:

	short m_band;
	Equalizer *m_eqLeft;
	Equalizer *m_eqRight;
	DaisyPod *m_daisyPod;
};

#endif // !defined(CONTROLLER__INCLUDED_)
