#ifndef _FILTER_HPP_
#define _FILTER_HPP_

#include <cassert>


namespace DSP {
    
/**
 * @brief Coefficients in a 5-number format (a0 ignored)
 * 
 */
struct BiquadCoeffs {
    float b0;
    float b1;
    float b2;
    float a1;
    float a2;
};


/**
 * @brief Biquad (or second-order section)
 * 
 * @tparam n_channels Number of channels to be processed, assumed
 * to be immutable for the life of the instance.
 */
template <unsigned int n_channels>
class Biquad {
 
 public:

    /**
     * @brief Memory of one section of the filter
     * 
     */
    struct State {
        float z1[n_channels];
        float z2[n_channels];
    };

    /**
     * @brief Construct a new Biquad object
     * 
     * @param n_sections Number of second-order sections
     * @param c Array of Coefficients. Allocate and set externally.
     * @param s Array of State (filter memory). Allocate externally.
     */
    Biquad(unsigned int n_sections, BiquadCoeffs *c, State *s);
    /**
     * @brief Reset memory of filter.
     * 
     */
    void Reset();
    /**
     * @brief Process a single frame of data.
     * 
     * @param x Audio frame (has to be n_channels long)
     */
    void ProcessFrame(float *x);
    /**
     * @brief Process buffer of frames.
     * 
     * @param buffer Audio buffer
     * @param n_samples Number of frames in audio buffer
     */
    void ProcessBuffer(float *buffer, unsigned int n_samples);
    /**
     * @brief Set the Coefficients object (deep copy)
     * 
     * @param sos_index Index of the second order section
     * @param c New coefficients
     */
    void SetCoefficients(unsigned int sos_index, BiquadCoeffs &c);

 private:
    
    /**
     * @brief DF2 kernel - perform filtering of all channels at once
     * 
     */
    static __attribute__((always_inline)) void DF2_(
            BiquadCoeffs *c, State *s, float *x) {
        for (unsigned int ch = 0; ch < n_channels; ch++) {
            float w = x[ch] - c->a1 * s->z1[ch] - c->a2 * s->z2[ch];
            x[ch] = c->b0 * w + c->b1 * s->z1[ch] + c->b2 * s->z2[ch];
            s->z2[ch] = s->z1[ch];
            s->z1[ch] = w;
        }
    }
    
    const unsigned int n_sections_;
    BiquadCoeffs *c_;
    State *s_;

};

template<unsigned int n_channels>
Biquad<n_channels>::Biquad(unsigned int n_sections, BiquadCoeffs *c, State *s) :
        n_sections_(n_sections),
        c_(c),
        s_(s) {
    Reset();
}


template<unsigned int n_channels>
void Biquad<n_channels>::Reset() {
    // Clear state manually
    for (unsigned int n = 0; n < n_sections_; n++) {
        for (unsigned int c = 0; c < n_channels; c++) {
            s_[n].z1[c] = 0;
            s_[n].z2[c] = 0;
        }
    }
}

template<unsigned int n_channels>
void Biquad<n_channels>::ProcessFrame(float *x) {
    BiquadCoeffs *c = c_;
    State *s = s_;
    for (unsigned int n = 0; n < n_sections_; n++) {
        // Direct form 2 kernel
        DF2_(c, s, x);
        c++;
        s++;
    }
}

template<unsigned int n_channels>
void Biquad<n_channels>::ProcessBuffer(float *buffer, unsigned int n_samples) {
    BiquadCoeffs *c = c_;
    State *s = s_;
    // Loop through SOS first, then samples (cache coefficients)
    for (unsigned int n = 0; n < n_sections_; n++) {
        float *buffer_head = buffer;
        for (unsigned int k = 0; k < n_samples; k++) {
            // Direct form 2 kernel
            DF2_(c, s, buffer_head);
            buffer_head += n_channels;
        }
        c++;
        s++;
    }
}


template<unsigned int n_channels>
void Biquad<n_channels>::SetCoefficients(
        unsigned int sos_index, BiquadCoeffs &c) {
    assert(sos_index < n_sections_);
    c_[sos_index] = c;
}


class ARSmoother {
 public:
    ARSmoother(float alpha_attack, float alpha_release);
    float ProcessSample(float in);
    void Reset();

 protected:
    float alpha_a_;
    float alpha_r_;
    float y_1_;
};


}  // namespace DSP

#endif  // _FILTER_HPP_
