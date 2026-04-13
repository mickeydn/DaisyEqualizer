#pragma once
#include "Algorithm.h"
#define float_PI 3.14159265359
#define NUM_LOW_SHELF_COEFFS 3

class LowShelfFilter : public Algorithm {

	public:
		LowShelfFilter();
		~LowShelfFilter();

		void Init() override;
		void Init(float* a_coeffs, float* b_coeffs);
		
		float Process(float input) override;
		void MakeLowShelf(const float fs, const float fc, const float Q, const float A);
		
	private:
		float a[NUM_LOW_SHELF_COEFFS];
		float b[NUM_LOW_SHELF_COEFFS];
		float x1_, x2_, y1_, y2_;
};
