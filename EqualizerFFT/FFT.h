/*
 * Simple iterative radix-2 FFT class (float)
 * - Precomputes twiddle factors for the current FFT size
 * - In-place complex FFT (real[] and imag[] arrays provided by caller)
 * - Simple real-FFT wrapper: runs complex FFT on real input (imag zeros) and returns full complex spectrum
 *
 * Usage:
 *   FFT fft;
 *   fft.init(1024);                  // must be power of two
 *   // prepare arrays
 *   float real[1024], imag[1024];
 *   // fill real[], set imag[] = 0
 *   fft.fft(real, imag, false);      // forward FFT
 *
 * Note: For maximum throughput on STM32, consider using CMSIS-DSP if available.
 */
// Kim Bjerge - 29. November 2025
#pragma once
#include <cstdint>
#include <cmath>
#include <cstring> // memset, memcpy
#include <cassert>

class FFT {
public:
    FFT() : N(0), log2N(0), twiddles(nullptr), bitrev(nullptr) {}
    ~FFT() { freeBuffers(); }

    // Initialize for given size (must be power of two)
    // returns true on success
    bool init(uint32_t size) 
    {
        if (!isPowerOfTwo(size)) return false;
        freeBuffers();

        N = size;
        log2N = computeLog2(N);

        // allocate tables
        // twiddles: pairs of (cos, sin) for k=0..N/2-1 stored as [2*k], [2*k+1]
        twiddles = new float[(N/2) * 2];
        // bit reversal table
        bitrev = new uint32_t[N];

        if (!twiddles || !bitrev) {
            freeBuffers();
            return false;
        }

        computeTwiddles();
        computeBitReversal();
        return true;
    }

    // In-place complex FFT
    // real[], imag[] must be arrays of length N (set imag==0 for pure real input)
    // inverse=false => forward FFT (sign = -1); inverse=true => inverse FFT (sign = +1) and scales output by 1/N
    void fft(float* real, float* imag, bool inverse = false) 
    {
        assert(real && imag && N > 0);

        // 1) bit reversal reorder
        for (uint32_t i = 0; i < N; ++i) {
            uint32_t j = bitrev[i];
            if (j > i) {
                float tmp = real[i]; real[i] = real[j]; real[j] = tmp;
                tmp = imag[i]; imag[i] = imag[j]; imag[j] = tmp;
            }
        }

        // 2) iterative Cooley-Tukey
        // sign: -1 for forward, +1 for inverse (we precomputed twiddles for sign=-1)
        //const float sign = inverse ? 1.0f : -1.0f;

        uint32_t halfSize = 1;
        for (uint32_t stage = 1; stage <= (uint32_t)log2N; ++stage) {
            uint32_t step = halfSize << 1;               // current FFT length
            // twiddle step: index increment for twiddle table
            uint32_t twStep = N / step;                  // how many twiddle indices between successive butterflies
            for (uint32_t i = 0; i < N; i += step) {
                uint32_t twIndex = 0;
                for (uint32_t j = 0; j < halfSize; ++j) {
                    // get twiddle (cos, sin) for index twIndex
                    float wr = twiddles[2 * twIndex + 0];
                    float wi = twiddles[2 * twIndex + 1];
                    // if inverse, conjugate the twiddle (flip sign of sin)
                    if (inverse) wi = -wi;

                    // butterfly: a = x[i+j], b = x[i+j+halfSize] * w
                    uint32_t idxA = i + j;
                    uint32_t idxB = i + j + halfSize;
                    float ar = real[idxA];
                    float ai = imag[idxA];
                    float br = real[idxB];
                    float bi = imag[idxB];

                    // complex multiply b * w
                    float tr = br * wr - bi * wi;
                    float ti = br * wi + bi * wr;

                    // combine
                    real[idxA] = ar + tr;
                    imag[idxA] = ai + ti;
                    real[idxB] = ar - tr;
                    imag[idxB] = ai - ti;

                    twIndex += twStep;
                }
            }
            halfSize = step;
        }

        // scale for inverse FFT
        if (inverse) {
            float invN = 1.0f / float(N);
            for (uint32_t i = 0; i < N; ++i) {
                real[i] *= invN;
                imag[i] *= invN;
            }
        }
    }

    // Simple real-FFT wrapper
    // inputReal length N -> outputComplexReal/Imag arrays length N/2+1 (DC..Nyquist)
    // This implementation performs an N-point complex FFT with imag=0 then returns k=0..N/2
    // Not the most optimized real-FFT, but simple and robust.
    void realFft(const float* inputReal, float* outReal, float* outImag) 
    {
        assert(inputReal && outReal && outImag && N > 0);
        // temporary buffers (caller may reuse real/imag arrays to avoid allocation)
        // Here we allocate on heap once per class; for real-time systems you can provide external buffers.
        float* r = new float[N];
        float* i = new float[N];
        for (uint32_t k = 0; k < N; ++k) { r[k] = inputReal[k]; i[k] = 0.0f; }

        fft(r, i, false);

        // copy k=0..N/2
        uint32_t half = N/2;
        for (uint32_t k = 0; k <= half; ++k) {
            outReal[k] = r[k];
            outImag[k] = i[k];
        }

        delete [] r;
        delete [] i;
    }

    // Free buffers
    void freeBuffers() 
    {
        if (twiddles) { delete [] twiddles; twiddles = nullptr; }
        if (bitrev)   { delete [] bitrev; bitrev = nullptr; }
        N = 0; log2N = 0;
    }

    // helpers
    uint32_t size() const { return N; }
    bool isInit() const { return N != 0; }

private:
    uint32_t N;
    int log2N;
    float* twiddles;   // length = N/2 * 2 (cos,sin)
    uint32_t* bitrev;  // length = N

    // compute twiddles for k=0..N/2-1 as cos(-2pi*k/N), sin(-2pi*k/N)
    void computeTwiddles() 
    {
        const float TWO_PI = 6.283185307179586476925286766559f;
        for (uint32_t k = 0; k < N/2; ++k) {
            float angle = -TWO_PI * float(k) / float(N); // negative for forward FFT convention
            twiddles[2*k + 0] = cosf(angle);
            twiddles[2*k + 1] = sinf(angle);
        }
    }

    // compute bit reversal for indices 0..N-1
    void computeBitReversal() 
    {
        for (uint32_t i = 0; i < (uint32_t)N; ++i) {
            bitrev[i] = bitReverse(i, log2N);
        }
    }

    static inline bool isPowerOfTwo(uint32_t x) 
    { 
        return x && !(x & (x - 1)); 
    }
    static inline int computeLog2(uint32_t x) 
    {
        int l = 0;
        while (x > 1) { x >>= 1; l++; }
        return l;
    }
    static inline uint32_t bitReverse(uint32_t x, int bits) 
    {
        uint32_t y = 0;
        for (int i = 0; i < bits; ++i) {
            y = (y << 1) | (x & 1u);
            x >>= 1;
        }
        return y;
    }
};
