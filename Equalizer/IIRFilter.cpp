#include "IIRFilter.h"
#include <math.h>

        IIRFilter::IIRFilter() {}

        IIRFilter::~IIRFilter() {}

        void IIRFilter::Init() 
        {
            for (int i = 0; i < NUM_IIR_COEFFS; i++) 
                        {
                            a[i] = 0;
                            b[i] = 0;
                        }
                        a[0] = 1.0;
                        b[0] = 1.0;
                        x1_ = x2_ = y1_ = y2_ = 0.0;
        }

        void IIRFilter::Init(float* a_coeffs, float* b_coeffs) 
        {
            for (int i = 0; i < NUM_IIR_COEFFS; i++) 
			{
				a[i] = a_coeffs[i];
				b[i] = b_coeffs[i];
			}
			x1_ = x2_ = y1_ = y2_ = 0.0;
        }

        float IIRFilter::Process(float x)
        { 
            float res = b[0]*x + b[1]*x1_ + b[2]*x2_ - a[1]*y1_ - a[2]*y2_;

			y2_ = y1_;
			y1_ = res;
			x2_ = x1_;
			x1_ = x;

			return res;
        }

        void IIRFilter::makePeakEQ(const float fs,
                                   const float fc,
                                   const float Q,
                                   const float A)
        {
            float omega =  2.0 * float_PI * (fc / fs);
			float alpha = sinf (omega) / (2.0 * Q);

			b[0] = 1.0 + alpha * A;
			b[1] = -2.0 * cosf (omega);
			b[2] = 1.0 - alpha * A;
			a[0] = 1.0 + alpha / A;
			a[1] = -2.0 * cosf (omega);
			a[2] = 1.0 - alpha / A;

			// Normalize the coefficients
			float a0 = a[0];
			for (int i = 0; i < NUM_IIR_COEFFS; i++) {
				a[i] = a[i]/a0;
				b[i] = b[i]/a0;
			}
        }
