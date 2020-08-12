#include "Block.hpp"

#define __USE_STDLIB    1

#if __USE_STDLIB
#include <cstring>
#endif

namespace DSP {

void Block::Copy(float *dest, float *src, unsigned int n_channels, unsigned int n_samples) {
#if __USE_STDLIB
    memcpy(dest, src, sizeof(float) * n_channels * n_samples);
#else
    __LOOP_THROUGH_BLOCK(*dest++ = *src++);
#endif
}


void Block::Zeros(float *buffer, unsigned int n_channels, unsigned int n_samples) {
#if __USE_STDLIB
    memset(buffer, 0, sizeof(float) * n_channels * n_samples);
#else
    __LOOP_THROUGH_BLOCK(*buffer++ = 0);
#endif
}


void Block::Gain(float *buffer, float gain, unsigned int n_channels, unsigned int n_samples) {
    __LOOP_THROUGH_BLOCK(*buffer++ *= gain);
}


void Block::Accum(float *buffer, float *accum, unsigned int n_channels, unsigned int n_samples) {
    __LOOP_THROUGH_BLOCK(*buffer++ += *accum++);
}


void Block::Subtract(float *buffer, float *accum, unsigned int n_channels, unsigned int n_samples) {
    __LOOP_THROUGH_BLOCK(*buffer++ -= *accum++);
}


void Block::AccumGain(float *buffer, float *accum, float gain, unsigned int n_channels, unsigned int n_samples) {
    __LOOP_THROUGH_BLOCK(*buffer++ += gain * (*accum++));
}


void Block::Mix(float *buffer, float *buffer2, float gain1, float gain2, unsigned int n_channels, unsigned int n_samples) {
    __LOOP_THROUGH_BLOCK(*buffer = (*buffer) * gain1 + (*buffer2++) * gain2; buffer++);
}

#undef __LOOP_THROUGH_BLOCK

}
