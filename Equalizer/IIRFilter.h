
#include "Algorithm.h"
#define float_PI 3.14159265359
#define NUM_IIR_COEFFS 3

class IIRFilter : public Algorithm {

	public:
		IIRFilter();
		~IIRFilter();

		void Init() override;
		void Init(float* a_coeffs, float* b_coeffs);
		
		float Process(float input) override;
		void makePeakEQ(const float fs, const float fc, const float Q, const float A);
		
	private:
		float a[NUM_IIR_COEFFS];
		float b[NUM_IIR_COEFFS];
		float x1_, x2_, y1_, y2_;
};
