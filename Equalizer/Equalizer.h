#if !defined(EQUALIZER_INCLUDED_)
#define EQUALIZER_INCLUDED_

#include "IIRFilter.h"
#include "Algorithm.h"
#include "HighShelfFilter.h"
#include "LowShelfFilter.h"

#define NUM_EQ_BANDS 4 

#define MAX_FREQ		20000.0
#define MIN_FREQ        2.0
#define MAX_Q			20.0
#define MIN_Q        	1.0
#define MAX_GAIN 		5.0
#define MIN_GAIN 		0.1

enum PARAMETER {
	PM_GAIN = 0,
	PM_FREQ = 1,
	PM_Q = 2
};

enum BANDTYPE {
	BAND_LOW_SHELF,
	BAND_PEAK_EQ,
	BAND_HIGH_SHELF
};

static const BANDTYPE BAND_TYPES[NUM_EQ_BANDS] = {
    BAND_LOW_SHELF,
    BAND_PEAK_EQ,
    BAND_PEAK_EQ,
    BAND_HIGH_SHELF
};
class Equalizer : public Algorithm
{
public:

	Equalizer();
	virtual ~Equalizer();

	float Process(float input) override;
	void Init() override;
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
	Algorithm* m_filters[NUM_EQ_BANDS];
	IIRFilter m_IIRfilter[NUM_EQ_BANDS];
	HighShelfFilter m_HighShelfFilter;
	LowShelfFilter m_LowShelfFilter;
};

#endif // !defined(EQUALIZER_INCLUDED_)
