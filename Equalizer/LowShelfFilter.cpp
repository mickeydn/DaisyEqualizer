#include "LowShelfFilter.h"
#include <math.h>

LowShelfFilter::LowShelfFilter() {}

LowShelfFilter::~LowShelfFilter() {}

void LowShelfFilter::Init() 
{
    for (int i = 0; i < NUM_LOW_SHELF_COEFFS; i++) 
    {
        a[i] = 0;
        b[i] = 0;
    }
    x1_ = x2_ = y1_ = y2_ = 0.0;
}

void LowShelfFilter::Init(float *a_coeffs, float *b_coeffs) 
{
     for (int i = 0; i < NUM_LOW_SHELF_COEFFS; i++) 
    {
        a[i] = a_coeffs[i];
        b[i] = b_coeffs[i];
    }
    x1_ = x2_ = y1_ = y2_ = 0.0;
}

float LowShelfFilter::Process(float input)
{
    float res = b[0]*input + b[1]*x1_ + b[2]*x2_ - a[1]*y1_ - a[2]*y2_;

    y2_ = y1_;
    y1_ = res;
    x2_ = x1_;
    x1_ = input;

    return res;
}

void LowShelfFilter::MakeLowShelf(const float fs,
                                  const float fc,
                                  const float Q,
                                  const float A)
{
    float omega = 2.0 * float_PI * (fc / fs);
    float cosW  = cosf(omega);
    float sinW  = sinf(omega);
    float alpha = sinW / (2.0 * Q);
    float sqrtA = sqrtf(A);

    b[0] =      A * ((A + 1) - (A - 1)*cosW + 2*sqrtA*alpha);
    b[1] =  2 * A * ((A - 1) - (A + 1)*cosW);
    b[2] =      A * ((A + 1) - (A - 1)*cosW - 2*sqrtA*alpha);
    a[0] =           (A + 1) + (A - 1)*cosW + 2*sqrtA*alpha;
    a[1] =     -2 * ((A - 1) + (A + 1)*cosW);
    a[2] =           (A + 1) + (A - 1)*cosW - 2*sqrtA*alpha;

    float a0 = a[0];
    for (int i = 0; i < NUM_LOW_SHELF_COEFFS; i++)
    {
        a[i] /= a0;
        b[i] /= a0;
    }
}
