/**
 * @file Triangular2DMesh.hpp
 * @author Andrea Martelloni (a.martelloni@qmul.ac.uk)
 * @brief 
 * @version 0.1
 * @date 2020-05-04
 * 
 * @copyright Copyright (c) 2020
 * 
 */

#ifndef __TRIANGULAR_2D_MESH_HPP__
#define __TRIANGULAR_2D_MESH_HPP__


#include <cstddef>
#include <cstdint>
#include <cmath>

static_assert(sizeof(float) == sizeof(uint32_t),
    "Float needs to be exactly 32 bit for this code to work");


class Triangular2DMesh {

 public:

    struct Properties {
        float x__mm;
        float y__mm;
        float spatial_res__mm;
    };
    typedef bool (*FormatMaskFn_type)(float, float);

    static size_t GetMemSize(Properties &p);
    Triangular2DMesh(Properties &p, void *mem);
    void Reset();
    void ApplyMask(FormatMaskFn_type mask_fn);
    void Process(float *input, float *output, unsigned int n_samples);

 protected:

    constexpr float kSqrt3Over2 = std::sqrtf(3.f) / 2.f;
    struct Properties_internal_ {
        unsigned int x_size;
        unsigned int y_size;
        unsigned int total_size;
    };
    Properties p_;
    Properties_internal_ pi_;
    float *meshv_[3];
    unsigned int meshv_offset_;
    uint32_t *mesh_mask_;

    static void GetInternalProperties(Properties &p,
        PropertiesInternal &pi_);
    template<typename T_>
    static __attribute__((always_inline)) T_ GetM_(T_ *v,
        unsigned int c, unsigned int k) {
        return v[c][2 * k + (c & 0x1)];
    }
    template<typename T_>
    static __attribute__((always_inline)) void SetM_(T_ *v,
        unsigned int c, unsigned int k, T_ value) {
        v[c][2 * k + (c & 0x1)] = value;
    }
    template float GetM_(float *v, unsigned int c, unsigned int k);
    template void SetM_(float *v, unsigned int c, unsigned int k, float value);
    template uint32_t GetM_(uint32_t *v,
        unsigned int c, unsigned int k);
    template void SetM_(uint32_t *v, unsigned int c,
        unsigned int k, uint32_t value);
    __attribute__((always_inline)) float *VHist_(unsigned int z) {
        unsigned int index = (meshv_offset_ + z);
        while (index >= 3) {
            index -= 3;
        }
        return meshv_[index];
    }
    __attribute__((always_inline)) void CKtoXY_(unsigned int c,
        unsigned int k, float &x, float &y) {
        y = static_cast<float>(c) * kSqrt3Over2 * p_.spatial_res__mm;
        x = static_cast<float>(k) * p_.spatial_res__mm +
            (p_.spatial_res__mm) * static_cast<float(c & 0x1); 
    }
};


#endif
