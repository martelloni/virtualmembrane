#include "FilterDesigner.hpp"
#include <cmath>

namespace DSP {

void FilterDesigner::ResonantLowpass(BiquadCoeffs *c, float fs,
        float f_cut, float q_factor, float gain) {
    // Pre-calculate factors used more than once
    float omega = 2 * M_PI * f_cut;
    float omega_2 = omega * omega;
    float q = 1.f / q_factor;
    float omega_q = omega * q;
    // Calculate coefficients
    float b[3] {
        omega_2,  // b0
        2.f * omega_2,  // b1
        omega_2,  // b2
    };
    float a[3] = {
        2.f * fs * (2.f * fs + omega_q) + omega_2,  // a0
        -8.f * fs * fs + 2.f * omega_2,  // b1
        2.f * fs * (2.f * fs - omega_q) + omega_2,  // a2
    };
    ScaleByA0(b, a, gain, c);
}


void FilterDesigner::ResonantHighpass(BiquadCoeffs *c, float fs,
            float f_cut, float q_factor, float gain) {
    float omega = 2.f * M_PI * f_cut;
    float omega_2 = omega * omega;
    float q = 1.f / q_factor;
    float omega_q = omega * q;
    float bfactor_highpass = 4.f * fs * fs;

    float b[3] {1.f * bfactor_highpass,
        -2.f * bfactor_highpass,
        1.f * bfactor_highpass};

    float a[3] {
        2.f * fs * (2.f * fs + omega_q) + omega_2,
        -8.f * fs * fs + 2.f * omega_2,
        2.f * fs * (2.f * fs - omega_q) + omega_2};

    ScaleByA0(b, a, gain, c);
}


void FilterDesigner::ScaleByA0(float *b, float *a, float gain,
        BiquadCoeffs *c) {
    static const unsigned int kSOSLength = 3;
    // Pre-calculate scaling factors
    float coeff = 1.f / a[0];
    float coeff_times_gain = coeff * gain;
    // Apply scaling factors
    for (unsigned int n = 0; n < kSOSLength; n++) {
        b[n] *= coeff_times_gain;
        a[n] *= coeff;
    }
    // In-place onto coeff pointer
    c->b0 = b[0];
    c->b1 = b[1];
    c->b2 = b[2];
    c->a1 = a[1];
    c->a2 = a[2];
}

}
