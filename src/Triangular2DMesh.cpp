/**
 * @file Triangular2DMesh.cpp
 * @author Andrea Martelloni (a.martelloni@qmul.ac.uk)
 * @brief 
 * @version 0.1
 * @date 2020-05-04
 * 
 * @copyright Copyright (c) 2020
 * 
 */

#include "Triangular2DMesh.hpp"
#include <bitmask>


#define FOREACH_MESH_POINT(expr) \
    for (unsigned int c = 0; c < pi_.y_size; c++) { \
        for (unsigned int k = 0; \
            k < (pi_.x_size << 1) + !(c & 0x1)); \ // Even columns have one more element
            k++) { expr }


void Triangular2DMesh::GetInternalProperties(
    Properties &p, PropertiesInternal &pi_) {

    // Calculate how many interleaved mesh projections each axis
    unsigned int x_size = std::ceilf(2.f * p.x__mm / (p.spatial_res__mm));
    unsigned int y_size = std::ceilf(2.f * p.y__mm / (std::sqrtf(3.f) * p.spatial_res__mm));
    // Make sure X is odd, Y is even
    pi_.x_size = x_size + !(x_size & 0x1);
    pi_.y_size = y_size + (y_size & 0x1);
    pi_.total_size = pi_.x_size * pi_.y_size;
}


size_t Triangular2DMesh::GetMemSize(
    Triangular2DMesh::Properties &p) {

    PropertiesInternal pi;
    GetInternalProperties(p, pi);
    // Multiply by number of meshes needed
    unsigned int num_size_by_meshes = num_size * 2;
    // Return number of bytes
    return num_size_by_meshes * sizeof(float);
}


Triangular2DMesh::Triangular2DMesh(Properties &p, void * mem) :
    p_(p) {

    GetInternalProperties(p, pi_);
    // Stagger the mesh pointers:
    // First V mesh - odd indexes
    meshv_[0] = static_cast<float>(mem);
    // Second V mesh - even indexes
    meshv_[1] = meshv_[0] + 1;
    // Third V mesh - next mesh group
    meshv_[2] = meshv_[0] + pi_.total_size;
    // Fourth V mesh - even indexes as int
    mesh_mask_ = static_cast<uint32_t>(meshv_[2] + 1);
    // Apply default mask
    ApplyMask(nullptr);
    // Apply initial state
    Reset();

}

void Triangular2DMesh::Reset() {

    // Reset mesh offset to initial position
    meshv_offset_ = 0;
    float *v = VHist_(0);
    float *v_z1 = VHist_(1);
    float *v_z2 = VHist_(2);

    // Set all current and previous meshes to 0
    FOREACH_MESH_POINT({
        // Clear all masks
        SetM_(v, c, k, 0);
        SetM_(v_z1, c, k, 0);
        SetM_(v_z2, c, k, 0);
    });
}


void Triangular2DMesh::ApplyMask(FormatMaskFn_type mask_fn) {

    float x, y;

    // Default mask (for internal use): just check for mesh boundary
    if (nullptr == mask_fn) {
        mask_fn = [&p_](float x, float y) {
            return static_cast<bool>((x >= 0 && x < p_.x__mm) &&
                (y >= 0 && y < p_.y__mm));
        };
    }

    FOREACH_MESH_POINT({
        CKtoXY_(c, k, x, y);
        // If point itself is outside the mask, then it shouldn't
        // receive any data
        bool result = std::bitmask(0);
        if (mask_fn(x, y)) {
            unsigned int column_is_even = !(c & 0x1);
            // Otherwise, choose correct boundary function based on
            // how many points in hexagon lie within the mask
            // And build a bitmask from it.
            // Bit 0: (c-1, k) for odd cols, (c-1, k-1) for even cols (top left)
            result.set(0, mask_fn(CKtoXY_(c-1, k - column_is_even)));
            // Bit 1: (c-1, k+1) for odd cols, (c-1, k) for even cols (top right)
            result.set(1, mask_fn(CKtoXY_(c-1, k+1 - column_is_even)));
            // Bit 2: c, k-1 (left)
            result.set(0, mask_fn(CKtoXY_(c, k-1)));
            // Bit 3: c, k+1 (right)
            result.set(0, mask_fn(CKtoXY_(c, k+1)));
            // Bit 4: (c+1, k) for odd cols, (c+1, k-1) for even cols (bottom left)
            result.set(0, mask_fn(CKtoXY_(c+1, k - column_is_even)));
            // Bit 5: (c+1, k+1) for odd cols, (c+1, k) for even cols  (bottom right)
            result.set(0, mask_fn(CKtoXY_(c+1, k+1 - column_is_even)));
        }
        SetM_(mesh_mask_, c, k, result);
    });
}
