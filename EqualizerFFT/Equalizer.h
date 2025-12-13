///////////////////////////////////////////////////////////
//  Equalizer.h
//  Implementation of the Class Equalizer
//  Created on:      06-aug-2014 09:31:01
//  Modified:        28-feb-2018 09:32:00
//  Modified:        27-nov-2025 20:01:00
//  Original author: kbe
///////////////////////////////////////////////////////////

#if !defined(EQUALIZER_INCLUDED_)
#define EQUALIZER_INCLUDED_

#include "IIRFilter.h"

#define NUM_EQ_BANDS 4 

#define MAX_FREQ		20000.0
#define MIN_FREQ        2.0
#define MAX_Q			20.0
#define MIN_Q        	1.0
#define MAX_GAIN 		6.0
#define MIN_GAIN 		0.1

enum PARAMETER {
	PM_GAIN = 0,
	PM_FREQ = 1,
	PM_Q = 2
};

class Equalizer //: public Algorithm
{

public:

	Equalizer();
	virtual ~Equalizer();

	float Process(float input);
	void Init(int SampleRate);

	void setParameter(short band, PARAMETER param, float value);
	float incParameter(short band, PARAMETER param, float delta);
	float decParameter(short band, PARAMETER param, float delta);
	float getParameter(short band, PARAMETER param);
	void setBypass(bool val) { filterOn_ = !val; };

protected:
	void updateEQParameters(short band);

	int sampleRate_;
	bool filterOn_;

	typedef struct _EQParams
	{
		float gain;
		float fc;
		float Q;
	} EQParams;

	EQParams m_EQParams[NUM_EQ_BANDS];
	IIRFilter m_IIRfilter[NUM_EQ_BANDS];
};

#endif // !defined(EQUALIZER_INCLUDED_)
