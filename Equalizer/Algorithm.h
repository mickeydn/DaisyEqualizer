
#if !defined(ALGORITHM_INCLUDED_)
#define ALGORITHM_INCLUDED_

class Algorithm
{

public:
	Algorithm();
	virtual ~Algorithm();

	virtual float Process(float input) = 0;
	virtual void Init(void) = 0;

};
#endif // !defined(ALGORITHM_INCLUDED_)
