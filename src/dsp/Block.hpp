#ifndef _BLOCK_HPP_
#define _BLOCK_HPP_


#define __LOOP_THROUGH_BLOCK(statement)    \
    unsigned int n = n_channels * n_samples; while (n-- > 0) { statement; }


namespace DSP {

/**
 * @brief Stateless block processing functions (add arrays, scale by gain, copy
 * contents...)
 * 
 */
class Block {

 public:

    /**
     * @brief Typedef for function pointer that processes a float sample.
     * 
     */
    typedef float (*pBlockProcessFn)(float);

    /**
     * @brief Copy array from source to destination.
     * 
     * @param dest 
     * @param src 
     * @param n_channels 
     * @param n_samples 
     */
    static void Copy(float *dest, float *src, unsigned int n_channels, unsigned int n_samples);
    /**
     * @brief Zero-out dirty array (for reuse)
     * 
     * @param buffer 
     * @param n_channels 
     * @param n_samples 
     */
    static void Zeros(float *buffer, unsigned int n_channels, unsigned int n_samples);
    /**
     * @brief Scale array by gain
     * 
     * @param buffer 
     * @param gain 
     * @param n_channels 
     * @param n_samples 
     */
    static void Gain(float *buffer, float gain, unsigned int n_channels, unsigned int n_samples);
    /**
     * @brief In-place accumulate array with values of accum array
     * 
     * @param buffer 
     * @param accum 
     * @param n_channels 
     * @param n_samples 
     */
    static void Accum(float *buffer, float *accum, unsigned int n_channels, unsigned int n_samples);
    /**
     * @brief In-place accumulate array with values of accum array (by subtraction)
     * 
     * @param buffer 
     * @param accum 
     * @param n_channels 
     * @param n_samples 
     */
    static void Subtract(float *buffer, float *accum, unsigned int n_channels, unsigned int n_samples);
    /**
     * @brief Accumulate and scale by gain into buffer (in-place)
     * 
     * @param buffer 
     * @param accum 
     * @param gain 
     * @param n_channels 
     * @param n_samples 
     */
    static void AccumGain(float *buffer, float *accum, float gain, unsigned int n_channels, unsigned int n_samples);
    /**
     * @brief Mix buffer with buffer2 back into buffer
     * 
     * @param buffer 
     * @param buffer2 
     * @param gain1 
     * @param gain2 
     * @param n_channels 
     * @param n_samples 
     */
    static void Mix(float *buffer, float *buffer2, float gain1, float gain2, unsigned int n_channels, unsigned int n_samples);
    /**
     * @brief Apply function fn to array element-wise
     * 
     * @param buffer 
     * @param fn 
     * @param n_channels 
     * @param n_samples 
     */
    template<typename FN_T>
    static void ApplyFn(float *buffer, FN_T fn,
        unsigned int n_channels,
        unsigned int n_samples) {
        __LOOP_THROUGH_BLOCK(*buffer = fn(*buffer); buffer++);
    }
};

}

#endif  // _BLOCK_HPP_