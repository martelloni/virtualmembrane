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
#include <cassert>


void Triangular2DMesh::GetInternalProperties(
    Properties &p, Properties_internal_ &pi_) {

    // Calculate how many interleaved mesh projections each axis
    unsigned int x_size = std::ceil(2.f * p.x__mm / (p.spatial_res__mm));
    unsigned int y_size = std::ceil(2.f * p.y__mm / (std::sqrt(3.f) * p.spatial_res__mm));
    // Make sure X is odd, Y is even
    pi_.x_size = x_size + !(x_size & 0x1);
    pi_.y_size = y_size + (y_size & 0x1);
    pi_.total_size = pi_.x_size * pi_.y_size;
    // C and K are columns and rows respectively: rows are interleaved,
    // columns aren't.
    pi_.c_size = pi_.y_size;
    pi_.k_size_even = (pi_.x_size >> 1) + 1;
    pi_.k_size_odd = (pi_.x_size >> 1);
    pi_.n_even_k = pi_.c_size >> 1;  // C size is supposed to be even
    pi_.n_odd_k = pi_.c_size >> 1;
    pi_.total_size_ck = pi_.k_size_even * pi_.n_even_k
        + pi_.k_size_odd * pi_.n_odd_k;
}


size_t Triangular2DMesh::GetMemSize(
    Triangular2DMesh::Properties p) {

    Properties_internal_ pi;
    GetInternalProperties(p, pi);
    // Multiply by number of meshes needed
    unsigned int num_size_by_meshes = pi.total_size * kNMeshes;
    // Return number of bytes
    return num_size_by_meshes * sizeof(float);
}


Triangular2DMesh::Triangular2DMesh(
    Triangular2DMesh::Properties p, void *mem) {
    Init_(p, mem);
}



void Triangular2DMesh::Init_(Properties p, void *mem) {

    p_ = p;
    GetInternalProperties(p, pi_);
    // Stagger the mesh pointers:
    for (unsigned int n = 0; n < kNWaveguides; n++) {
        meshVCurrMem_[n] = static_cast<float *>(mem) + pi_.total_size * n;
        meshVHistMem_[n] = meshVCurrMem_[n] + 1;
    }
    // Junction mesh and mask mesh
    meshVJunc_ = meshVCurrMem_[kNWaveguides - 1] + pi_.total_size;
    mesh_mask_ = reinterpret_cast<uint32_t *>(meshVJunc_ + 1);
    // Apply default mask: all points inside the mesh
    ApplyMask([&](float x_, float y_) {
        return static_cast<bool>((x_ >= 0 && x_ < p_.x__mm) &&
            (y_ >= 0 && y_ < p_.y__mm));
    });
    // Apply initial state
    SetSource(p.x__mm * 0.5, p.y__mm * 0.5);
    SetPickup(0, 0);
    Reset();

}


void Triangular2DMesh::Reset() {

    // Reset mesh offset to initial position
    VCurr_ = meshVCurrMem_;
    VHist_ = meshVHistMem_;

    // Set all current and previous meshes to 0
    FOREACH_MESH_POINT({
        // Clear all meshes
        for (unsigned int n = 0; n < kNWaveguides; n++) {
            SetM_(meshVCurrMem_[n], c, k, 0.f);
            SetM_(meshVHistMem_[n], c, k, 0.f);
        }
        SetM_(meshVJunc_, c, k, 0.f);
    });
}


void Triangular2DMesh::SetSource(float x, float y) {
    
    CKCoords_ source = XYtoCK_(x, y);
    assert(source.c < pi_.c_size);
    assert(source.k < (source.c & 0x1) ? pi_.k_size_odd : pi_.k_size_even);
    // Get mask and assert it's in a point that exists and receives signal
    unsigned int source_mask = GetM_(mesh_mask_, source.c, source.k);
    assert(source_mask != 0);  // Is source point outside mesh mask?
    source_ = source;
}


void Triangular2DMesh::SetPickup(float x, float y) {
    
    CKCoords_ pickup = XYtoCK_(x, y);
    assert(pickup.c < pi_.c_size);
    assert(pickup.k < (pickup.c & 0x1) ? pi_.k_size_odd : pi_.k_size_even);
    // Get mask and assert it's in a point that exists and receives signal
    unsigned int pickup_mask = GetM_(mesh_mask_, pickup.c, pickup.k);
    assert(pickup_mask != 0);  // Is pickup point outside mesh mask?
    pickup_ = pickup;
}


float Triangular2DMesh::ProcessSample(bool input_present, float input) {

    float output = 0;

    // FOREACH_MESH_POINT expanded (fights with the X-Macro below)
    for (unsigned int c = 0; c < pi_.c_size; c++) {
        unsigned int column_is_even = !(c & 0x1);
        for (unsigned int k = 0;
            k < ((pi_.k_size_odd) + column_is_even);
            k++) {

        // Bit of X-Macro'ing: This will expand a macro that defines X for all
        // adjacent points. (thank you preprocessor!)
        #define ON_ALL_ADJACENT_POINTS    X(NE) X(E) X(SE) X(SW) X(W)

            // Get mask
            std::bitset<kNWaveguides> mask(GetM_(mesh_mask_, c, k));
            unsigned int n_junction_points = mask.count();
            if (n_junction_points == 0) {
                // No waveguides to see here, move along...
                continue;
            }

            // Scattering equation
            float scatter_coeff = 2 / n_junction_points;
            float scatter_sum = 0;
        #define ADD_TO_SCATTER_SUM(POINT)    \
            if (mask.test( k##POINT )) {     \
                scatter_sum += GetM_(VHist_[ k##POINT ], k##POINT##_C_K);    \
            }
        #define X    ADD_TO_SCATTER_SUM

            ON_ALL_ADJACENT_POINTS

        #undef X
        #undef ADD_TO_SCATTER_SUM
            scatter_sum *= scatter_coeff;
            SetM_(meshVJunc_, c, k, scatter_sum);

            // Junction output
            if (input_present && c == source_.c && k == source_.k) {
                // Source point - use input
            #define INJECT_SOURCE(POINT)    \
                if (mask.test( k##POINT )) {    \
                    SetM_(VCurr_[ k##POINT ], k##POINT##_C_K, input);    \
                }
            #define X    INJECT_SOURCE

                ON_ALL_ADJACENT_POINTS

            #undef X
            #undef INJECT_SOURCE
            } else {
                // Not a source point - use incoming waves
            #define COMPUTE_OUTGOING_WAVE(POINT)    \
                if (mask.test( k##POINT )) {    \
                    float junc_out = scatter_sum -    \
                        GetM_(VHist_[ k##POINT ], k##POINT##_C_K);    \
                    SetM_(VCurr_[ k##POINT ], k##POINT##_C_K, junc_out);    \
                }
            #define X    COMPUTE_OUTGOING_WAVE

                ON_ALL_ADJACENT_POINTS

            #undef X
            #undef COMPUTE_OUTGOING_WAVE
            }

            // If listener, store output
            if (c == pickup_.c && k == pickup_.k) {
                output = scatter_sum;
            }

        #undef ON_ALL_ADJACENT_POINTS
        }
    }

    // Swap buffers (current->history)
    float **tmp = VHist_;
    VHist_ = VCurr_;
    VCurr_ = tmp;

    return output;
}
