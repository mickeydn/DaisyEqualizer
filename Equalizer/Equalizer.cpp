///////////////////////////////////////////////////////////
//  Equalizer.cpp
//  Implementation of the Class Equalizer
//  Created on:      06-aug-2014 09:31:01
//  Modified:        28-feb-2018 09:32:00
//  Modified:        27-nov-2025 20:01:00
//  Original author: kbe
///////////////////////////////////////////////////////////
#include "Equalizer.h"

#include <stdio.h>
#include <string.h>
#include <stdfix.h>

Equalizer::Equalizer()
{

	filterOn_ = true;
}

Equalizer::~Equalizer()
{

}

float Equalizer::Process(float input)
{
	if (!filterOn_) return input;

    float signal = input;
    for (int i = 0; i < NUM_EQ_BANDS; i++) {
        signal = m_filters[i]->Process(signal);  
    }
    return signal;
}

void Equalizer::Init(int sampleRate)
{
	sampleRate_ = sampleRate;

	// KBE changed to update filters
	// Setting default peak bands center frequency
	m_EQParams[0].fc = 100.0;
	m_EQParams[0].gain = 5.0;
	m_EQParams[1].fc = 500.0;
	m_EQParams[1].gain = 2.0; 
	m_EQParams[2].fc = 2000.0;
	m_EQParams[2].gain = 0.5; 
	m_EQParams[3].fc = 8000.0;
	m_EQParams[3].gain = 2.0; 

	for (short band = 0; band < NUM_EQ_BANDS; band++)
	{
		m_IIRfilter[band].Init();
	    //m_EQParams[band].gain = 1.0; // Default gain
		m_EQParams[band].Q = 8.0; // Default Q
		m_filters[band] = &m_IIRfilter[band];
		updateEQParameters(band);
	}
}

float Equalizer::incParameter(short band, PARAMETER param, float delta)
{
	// TODO change code to increment parameter for IIR filters (See slides) - call updateEQParamters
    //m_filterOn = true; // For test only
    setParameter(band, param, m_EQParams[band].gain + delta);
	return m_EQParams[band].gain;
}

float Equalizer::decParameter(short band, PARAMETER param, float delta)
{
	// TODO change code to decrement parameter for IIR filters - call updateEQParamters
    //m_filterOn = false; // For test only
    setParameter(band, param, m_EQParams[band].gain - delta);
	return m_EQParams[band].gain;
}

void Equalizer::setParameter(short band, PARAMETER param, float value)
{
    bool update = false;

	if (band < NUM_EQ_BANDS)
	{
		switch (param)
		{
			case PM_GAIN:
				if (m_EQParams[band].gain != value) {
					m_EQParams[band].gain = value;
					update = true;
				}
			  break;
			case PM_FREQ:
				if (m_EQParams[band].fc != value) {
					m_EQParams[band].fc = value;
					update = true;
				}
			  break;
			case PM_Q:
				if (m_EQParams[band].Q != value) {
					m_EQParams[band].Q = value;
					update = true;
				}
			  break;
		}
		if (update) updateEQParameters(band);
	}
}

void Equalizer::updateEQParameters(short band)
{
	// Parameter limitations
	if (m_EQParams[band].gain > MAX_GAIN)
		m_EQParams[band].gain = MAX_GAIN;
	if (m_EQParams[band].gain < MIN_GAIN)
		m_EQParams[band].gain = MIN_GAIN;
	if (m_EQParams[band].fc > MAX_FREQ)
		m_EQParams[band].fc = MAX_FREQ;
	if (m_EQParams[band].fc < MIN_FREQ)
		m_EQParams[band].fc = MIN_FREQ;
	if (m_EQParams[band].Q > MAX_Q)
		m_EQParams[band].Q = MAX_Q;
	if (m_EQParams[band].Q < MIN_Q)
		m_EQParams[band].Q = MIN_Q;

	// Call creating peaking filter for IIR filter band
	m_IIRfilter[band].makePeakEQ(sampleRate_,
							      m_EQParams[band].fc,
								  m_EQParams[band].Q,
								  m_EQParams[band].gain);

}

float Equalizer::getParameter(short band, PARAMETER param) 
{
		switch (param) 
		{
			case PM_GAIN:
				return m_EQParams[band].gain;
			case PM_FREQ:
				return m_EQParams[band].fc;
			case PM_Q:
				return m_EQParams[band].Q;
			default:
				return 0.0;
		}
	}