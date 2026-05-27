#if !defined(ALGORITHM_INCLUDED_)
#define ALGORITHM_INCLUDED_

struct PitchResult{
	float frequency;
	float confidence;
	float valid;
};


class Algorithm
{

public:
	Algorithm();
	virtual ~Algorithm();

	virtual PitchResult Process(const float* buffer, int bufferSize) = 0;
	virtual void Init(int samplerate) = 0;

};
#endif // !defined(ALGORITHM_INCLUDED_)
