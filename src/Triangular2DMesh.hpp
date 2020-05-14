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
#include <bitset>


/**
 * @brief Iterate over mesh points: when iterating over rows,
 * even rows have one fewer element.
 * 
 */
#define FOREACH_MESH_POINT(expr) \
    for (unsigned int c = 0; c < pi_.c_size; c++) { \
        __attribute__((unused)) unsigned int column_is_even = !(c & 0x1); \
        for (unsigned int k = 0; \
            k < ((pi_.k_size_odd) + column_is_even); \
            k++) { \
            expr } }

#define kNE_C_K    c-1, k+1 - column_is_even
#define kE_C_K    c, k+1
#define kSE_C_K    c+1, k+1 - column_is_even
#define kSW_C_K    c+1, k - column_is_even
#define kW_C_K    c, k-1
#define kNW_C_K    c-1, k - column_is_even

#define kNE_reciprocal    kSW
#define kE_reciprocal    kW
#define kSE_reciprocal    kNW
#define kSW_reciprocal    kNE
#define kW_reciprocal    kE
#define kNW_reciprocal    kSE

static_assert(sizeof(float) == sizeof(uint32_t),
    "Float needs to be exactly 32 bit for this code to work");


class Triangular2DMesh {

 public:

    struct Properties {
        float x__mm;
        float y__mm;
        float spatial_res__mm;
    };

    static size_t GetMemSize(Properties p);
    Triangular2DMesh(Properties p, void *mem);
    void Reset();
    template <typename MaskFnT>
    void ApplyMask(MaskFnT mask_fn);
    float ProcessSample(bool input_present, float input);
    void SetSource(float x, float y);
    void SetPickup(float x, float y);

 protected:

    enum WaveguideIndex_ {
        kNE,
        kE,
        kSE,
        kSW,
        kW,
        kNW,
        kNWaveguides,
    };
    static constexpr unsigned int kNVMeshes = kNWaveguides*2 + 1;  // + Junction
    static constexpr unsigned int kNMaskMeshes = 1;
    static constexpr unsigned int kNMeshes = kNVMeshes + kNMaskMeshes;
    static float kSqrt3Over2;
    struct Properties_internal_ {
        unsigned int x_size;
        unsigned int y_size;
        unsigned int total_size;
        unsigned int c_size;
        unsigned int k_size_even;
        unsigned int k_size_odd;
        unsigned int n_even_k;
        unsigned int n_odd_k;
        unsigned int total_size_ck;
    };
    struct CKCoords_ {
        unsigned int c;
        unsigned int k;
    };
    Properties p_;
    Properties_internal_ pi_;
    float *travelling_v_1_[kNWaveguides];
    float *travelling_v_2_[kNWaveguides];
    float *junc_v_;
    uint32_t *mesh_mask_;
    float ** v_curr_;
    float ** v_next_;
    CKCoords_ source_;
    CKCoords_ pickup_;

    Triangular2DMesh() {};

    void Init_(Properties p, void *mem);
    static void GetInternalProperties(Properties &p,
        Properties_internal_ &pi_);

    template<typename T_>
    __attribute__((always_inline)) T_ GetM_(T_ *v,
        unsigned int c, unsigned int k) {
        return v[c * pi_.x_size + 2 * k + (c & 0x1)];
    }

    template<typename T_>
    __attribute__((always_inline)) void SetM_(T_ *v,
        unsigned int c, unsigned int k, T_ value) {
        v[c * pi_.x_size + 2 * k + (c & 0x1)] = value;
    }

    __attribute__((always_inline)) void CKtoXY_(unsigned int c,
        unsigned int k, float &x, float &y) {
        y = static_cast<float>(c) * kSqrt3Over2 * p_.spatial_res__mm;
        x = static_cast<float>(k) * p_.spatial_res__mm +
            (p_.spatial_res__mm * 0.5f) * static_cast<float>(c & 0x1);
    }

    __attribute__((always_inline)) CKCoords_ XYtoCK_(float x, float y) {
        CKCoords_ out;
        out.c = y / (kSqrt3Over2 * p_.spatial_res__mm);
        out.k = (x - p_.spatial_res__mm * 0.5 * (out.c & 0x1)) / p_.spatial_res__mm;
        return out;
    }
};

template <typename MaskFnT>
void Triangular2DMesh::ApplyMask(MaskFnT mask_fn) {

    float x, y;

    FOREACH_MESH_POINT({
        CKtoXY_(c, k, x, y);
        // If point itself is outside the mask, then it shouldn't
        // receive any data
        std::bitset<kNWaveguides> result(0);
        if (mask_fn(x, y)) {
            // Otherwise, choose correct boundary function based on
            // how many points in hexagon lie within the mask
            // And build a bitmask from it.
            // Bit 0: (c-1, k+1) for odd cols, (c-1, k) for even cols (top right)
            CKtoXY_(kNE_C_K, x, y);
            result.set(kNE, mask_fn(x, y));
            // Bit 1: c, k+1 (right)
            CKtoXY_(kE_C_K, x, y);
            result.set(kE, mask_fn(x, y));
            // Bit 2: (c+1, k+1) for odd cols, (c+1, k) for even cols  (bottom right)
            CKtoXY_(kSE_C_K, x, y);
            result.set(kSE, mask_fn(x, y));
            // Bit 3: (c+1, k) for odd cols, (c+1, k-1) for even cols (bottom left)
            CKtoXY_(kSW_C_K, x, y);
            result.set(kSW, mask_fn(x, y));
            // Bit 4: c, k-1 (left)
            CKtoXY_(kW_C_K, x, y);
            result.set(kW, mask_fn(x, y));
            // Bit 5: (c-1, k) for odd cols, (c-1, k-1) for even cols (top left)
            CKtoXY_(kNW_C_K, x, y);
            result.set(kNW, mask_fn(x, y));
        }
        SetM_(mesh_mask_, c, k, static_cast<uint32_t>(result.to_ulong()));
    });
}


// Instantiation of templates for mesh access with short-hand functions
template float Triangular2DMesh::GetM_(float *v, unsigned int c, unsigned int k);
template void Triangular2DMesh::SetM_(float *v, unsigned int c, unsigned int k, float value);
template uint32_t Triangular2DMesh::GetM_(uint32_t *v,
    unsigned int c, unsigned int k);
template void Triangular2DMesh::SetM_(uint32_t *v, unsigned int c,
    unsigned int k, uint32_t value);


#endif
