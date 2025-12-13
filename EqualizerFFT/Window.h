// Kim Bjerge - 30. November 2025
#pragma once
#include <cmath>
#include <cstdint>

class WindowFunction
{
public:
    enum WindowType
    {
        RECTANGULAR = 0,
        HAMMING,
        HANNING,
        BLACKMAN,
        TRIANGULAR,
        WELCH
    };

    WindowFunction(uint16_t size, WindowType type)
        : N(size), winType(type)
    {
        coeffs = new float[N];
        computeCoefficients();
    }

    ~WindowFunction()
    {
        delete[] coeffs;
    }

    inline void apply(float* buffer)
    {
        for (uint16_t i = 0; i < N; i++)
            buffer[i] *= coeffs[i];
    }

    inline void setWindow(WindowType type)
    {
        winType = type;
        computeCoefficients();
    }

    inline WindowType getWindow() const { return winType; }

private:
    uint16_t N;
    WindowType winType;
    float* coeffs;

    void computeCoefficients()
    {
        switch (winType)
        {
            case RECTANGULAR: computeRectangular(); break;
            case HAMMING:     computeHamming();     break;
            case HANNING:     computeHanning();     break;
            case BLACKMAN:    computeBlackman();    break;
            case TRIANGULAR:  computeTriangular();  break;
            case WELCH:       computeWelch();       break;
        }
    }

    void computeRectangular()
    {
        for (uint16_t i = 0; i < N; i++)
            coeffs[i] = 1.0f;
    }

    void computeHamming()
    {
        for (uint16_t i = 0; i < N; i++)
            coeffs[i] = 0.53836f - 0.46164f * cosf(2.0f * M_PI * i / (float)(N - 1));
    }

    void computeHanning()
    {
        for (uint16_t i = 0; i < N; i++)
            coeffs[i] = 0.5f - 0.5f * cosf(2.0f * M_PI * i / (float)(N - 1));
    }

    void computeBlackman()
    {
        const float a0 = 0.42f;
        const float a1 = 0.5f;
        const float a2 = 0.08f;
        for (uint16_t i = 0; i < N; i++)
        {
            float w = 2.0f * M_PI * i / (float)(N - 1);
            coeffs[i] = a0 - a1 * cosf(w) + a2 * cosf(2.0f * w);
        }
    }

    void computeTriangular()
    {
        // Symmetric Bartlett window
        for (uint16_t i = 0; i < N; i++)
        {
            coeffs[i] = 1.0f - fabsf((i - (N - 1) / 2.0f) / (float)((N - 1) / 2.0f));
        }
    }

    void computeWelch()
    {
        // 1 - ((i - center)/center)^2
        float center = (N - 1) / 2.0f;
        for (uint16_t i = 0; i < N; i++)
        {
            float x = (i - center) / center;
            coeffs[i] = 1.0f - x * x;
        }
    }
};
