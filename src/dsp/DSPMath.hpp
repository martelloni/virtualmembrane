#include <cmath>
#include <cstdint>

namespace DSP {

/**
 * @brief Faster implementation of common mathematical functions for DSP.
 * 
 * Library of fast functions with a generic eye on ARM/DSP optimisation.
 * Log->exp are all based on exploiting the linear approximation done by
 * the floating-point representation of a number.
 * 
 * Liberally taken from:
 * http://www.machinedlearnings.com/2011/06/fast-approximate-logarithm-exponential.html
 */
class Math {

 public:

#if !defined(__arm__)
    /**
     * @brief Log base 2 of exp(1)
     * 
     */
    static constexpr float kLog2OfE = std::log2(std::exp((double)1.));
    /**
     * @brief Log base 2 of 10
     * 
     */
    static constexpr float kLog2Of10 = std::log2(10.);
    /**
     * @brief Log base 2 of exp(1)
     * 
     */
    static constexpr float kOneOverLog2OfE = 1. / kLog2OfE;
    /**
     * @brief Log base 2 of 10
     * 
     */
    static constexpr float kOneOverLog2Of10 = 1. / kLog2Of10;
#else
	// ARM compiler does not (really) support constexpr
	static constexpr float kLog2OfE = 1.4426950408889634f;  //std::log2(std::exp((double)1.));
    static constexpr float kLog2Of10 = 3.321928094887362f; //std::log2(10.);
    static constexpr float kOneOverLog2OfE = 1. / kLog2OfE;
    static constexpr float kOneOverLog2Of10 = 1. / kLog2Of10;
#endif

    static inline float FastPow2(float p) {
        float clipp = (p < -126) ? -126.0f : p;
        union { uint32_t i; float f; } v = {
            static_cast<uint32_t>( (1 << 23) * (clipp + 126.94269504f) ) };
        return v.f;
    }

    static inline float FastLog2(float x) {
        union { float f; uint32_t i; } vx = { x };
        float y = vx.i;
        y *= 1.1920928955078125e-7f;
        return y - 126.94269504f;
    }

    static inline float FastExp(float p) {
        return FastPow2(p * kLog2OfE);
    };

    static inline float FastLn(float x) {
        return FastLog2(x) * kOneOverLog2OfE;
    }

    static inline float FastPow10(float p) {
        return FastPow2(p * kLog2Of10);
    };

    static inline float FastLog10(float x) {
        return FastLog2(x) * kOneOverLog2Of10;
    }

    static inline float LinTodB(float x) {
        return 20.f * FastLog10(x);
    }

    static inline float dBToLin(float x) {
        static constexpr float kOneOver20 = 1. / 20.;
        return FastPow10(x * kOneOver20);
    }

    /**
     * @brief Translate voltage to frequency with an
     * exponential law, taking 1V -> 110Hz as reference.
     * It does NOT use fast math because pitch needs
     * to be more precise than amplitude!
     * 
     * @param v Voltage
     * @return float Frequency
     */
    static inline float VoltsToFreq(float v) {
        return 110.f * std::pow(2, v);
    }
};

}
