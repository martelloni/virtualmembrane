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
    for (unsigned int c = 0; c < pi_.y_size; c++) { \
        for (unsigned int k = 0; \
            k < ((pi_.x_size << 1) + !(c & 0x1)); \
            k++) { expr } }

static_assert(sizeof(float) == sizeof(uint32_t),
    "Float needs to be exactly 32 bit for this code to work");


class Triangular2DMesh {

 public:

    struct Properties {
        float x__mm;
        float y__mm;
        float spatial_res__mm;
    };

    static size_t GetMemSize(Properties &p);
    Triangular2DMesh(Properties &p, void *mem);
    void Reset();
    template <typename MaskFnT>
    void ApplyMask(MaskFnT mask_fn);
    void Process(float *input, float *output, unsigned int n_samples);

 protected:

    static constexpr float kSqrt3Over2 = std::sqrt(3.f) / 2.f;
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
            (p_.spatial_res__mm) * static_cast<float>(c & 0x1); 
    }
};

template <typename MaskFnT>
void Triangular2DMesh::ApplyMask(MaskFnT mask_fn) {

    float x, y;

    FOREACH_MESH_POINT({
        CKtoXY_(c, k, x, y);
        // If point itself is outside the mask, then it shouldn't
        // receive any data
        std::bitset<6> result(0);
        if (mask_fn(x, y)) {
            unsigned int column_is_even = !(c & 0x1);
            // Otherwise, choose correct boundary function based on
            // how many points in hexagon lie within the mask
            // And build a bitmask from it.
            // Bit 0: (c-1, k) for odd cols, (c-1, k-1) for even cols (top left)
            CKtoXY_(c-1, k - column_is_even, x, y);
            result.set(0, mask_fn(x, y));
            // Bit 1: (c-1, k+1) for odd cols, (c-1, k) for even cols (top right)
            CKtoXY_(c-1, k+1 - column_is_even, x, y);
            result.set(1, mask_fn(x, y));
            // Bit 2: c, k-1 (left)
            CKtoXY_(c, k-1, x, y);
            result.set(2, mask_fn(x, y));
            // Bit 3: c, k+1 (right)
            CKtoXY_(c, k+1, x, y);
            result.set(3, mask_fn(x, y));
            // Bit 4: (c+1, k) for odd cols, (c+1, k-1) for even cols (bottom left)
            CKtoXY_(c+1, k - column_is_even, x, y);
            result.set(4, mask_fn(x, y));
            // Bit 5: (c+1, k+1) for odd cols, (c+1, k) for even cols  (bottom right)
            CKtoXY_(c+1, k+1 - column_is_even, x, y);
            result.set(5, mask_fn(x, y));
        }
        SetM_(mesh_mask_, c, k, result);
    });
}


// Instantiation of templates for mesh access with short-hand functions
template float Triangular2DMesh::GetM_(float *v, unsigned int c, unsigned int k);
template void Triangular2DMesh::SetM_(float *v, unsigned int c, unsigned int k, float value);
template uint32_t Triangular2DMesh::GetM_(uint32_t *v,
    unsigned int c, unsigned int k);
template void Triangular2DMesh::SetM_(uint32_t *v, unsigned int c,
    unsigned int k, uint32_t value);


#undef FOREACH_MESH_POINT

#endif
