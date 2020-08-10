#ifndef _FILTER_DESIGNER_HPP_
#define _FILTER_DESIGNER_HPP_

#include "Filter.hpp"

namespace DSP {

/**
 * @brief Filter designer class: functions to design filters on the fly
 * 
 */
class FilterDesigner {

 public:
    /**
     * @brief Resonant second-order lowpass filter
     * 
     * @param c Biquad coefficient pointer (in-place creation)
     * @param fs Sample rate
     * @param f_cut Cutoff frequency
     * @param q_factor Quality factor
     * @param gain Gain of filter (factor applied to B-coefficients)
     */
    static void ResonantLowpass(BiquadCoeffs *c, float fs,
            float f_cut, float q_factor, float gain);

    /**
     * @brief Resonant second-order highpass filter
     * 
     * @param c Biquad coefficient pointer (in-place creation)
     * @param fs Sample rate
     * @param f_cut Cutoff frequency
     * @param q_factor Quality factor
     * @param gain Gain of filter (factor applied to B-coefficients)
     */
    static void ResonantLowpass(BiquadCoeffs *c, float fs,
            float f_cut, float q_factor, float gain);

 private:
    /**
     * @brief Scale B and A array by A[0] and format into a Biquad
     * Coeffs reference
     * 
     * @param b B coefficients
     * @param a A coefficients
     * @param gain Gain of system
     * @param c Formatted coefficient memory
     */
    static void ScaleByA0(float *b, float *a, float gain,
            BiquadCoeffs *c);
};

}

#endif  // _FILTER_DESIGNER_HPP_
