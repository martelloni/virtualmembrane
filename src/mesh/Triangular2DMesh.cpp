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


float Triangular2DMesh::kSqrt3Over2 = std::sqrt(3.f) / 2.f;


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
        // First one pointing at the first element
        travelling_v_1_[n] = reinterpret_cast<float *>(mem) + (n) * pi_.total_size;
        // Second one pointing at the second element
        travelling_v_2_[n] = travelling_v_1_[n] + 1;
    }
    // Junction mesh and mask mesh
    junc_v_ = travelling_v_1_[kNWaveguides - 1] + pi_.total_size;
    mesh_mask_ = reinterpret_cast<uint32_t *>(junc_v_ + 1);
    // Apply default mask: all points inside the mesh
    ApplyMask([&](float x_, float y_) {
        return static_cast<bool>((x_ >= 0 && x_ < p_.x__mm) &&
            (y_ >= 0 && y_ < p_.y__mm));
    });
    // Apply initial state
    SetSource(p.x__mm * 0.5, p.y__mm * 0.5);
    SetPickup(0, 0);
    SetAttenuation(0);
    Reset();
}


void Triangular2DMesh::Reset() {

    // Reset mesh offset to initial position
    v_curr_ = travelling_v_1_;
    v_next_ = travelling_v_2_;

    // Set all current and previous meshes to 0
    FOREACH_MESH_POINT({
        // Clear all meshes
        for (unsigned int n = 0; n < kNWaveguides; n++) {
            SetM_(travelling_v_1_[n], c, k, 0.f);
            SetM_(travelling_v_2_[n], c, k, 0.f);
        }
        SetM_(junc_v_, c, k, 0.f);
    });
}


void Triangular2DMesh::SetSource(float x, float y) {
    
    CKCoords_ source = XYtoCK_(x, y);
    assert(source.c < pi_.c_size);
    assert(source.k < (source.c & 0x1) ? pi_.k_size_odd : pi_.k_size_even);
    // Get mask and assert it's in a point that exists and receives signal
    __attribute__((unused))  // Bela compiler moans assertions aren't "use"
    unsigned int source_mask = GetM_(mesh_mask_, source.c, source.k);
    assert(source_mask != 0);  // Is source point outside mesh mask?
    source_ = source;
}


void Triangular2DMesh::SetPickup(float x, float y) {
    
    CKCoords_ pickup = XYtoCK_(x, y);
    assert(pickup.c < pi_.c_size);
    assert(pickup.k < (pickup.c & 0x1) ? pi_.k_size_odd : pi_.k_size_even);
    // Get mask and assert it's in a point that exists and receives signal
    __attribute__((unused))  // Bela compiler moans assertions aren't "used"
    unsigned int pickup_mask = GetM_(mesh_mask_, pickup.c, pickup.k);
    assert(pickup_mask != 0);  // Is pickup point outside mesh mask?
    pickup_ = pickup;
}


float Triangular2DMesh::ProcessSample(bool input_present, float input) {

    float output = 0;

// Bit of X-Macro'ing: This will expand a macro that defines X for all
// adjacent points. (thank you preprocessor!)
#define ON_ALL_ADJACENT_POINTS    X(NE) X(E) X(SE) X(SW) X(W) X(NW)

    // FOREACH_MESH_POINT expanded by hand (fights with the X-Macros below)
    for (unsigned int c = 0; c < pi_.c_size; c++) {
        unsigned int column_is_even = !(c & 0x1);
        for (unsigned int k = 0;
            k < ((pi_.k_size_odd) + column_is_even);
            k++) {

            // Get mask
            std::bitset<kNWaveguides> mask(GetM_(mesh_mask_, c, k));
            unsigned int n_junction_points = mask.count();
            if (n_junction_points == 0) {
                // No waveguides to see here, move along...
                continue;
            }

            // Point source
            float source_v = 0;
            float source_point_coef = 0;
            if (input_present && c == source_.c && k == source_.k) {
                // Source point - use input
            //#define INJECT_SOURCE(POINT)    \
            //    if (mask.test( k##POINT )) {    \
            //        float val = input;    \
            //        SetM_(v_curr_[ k##POINT ], c, k, val);    \
            //    }
            //#define X    INJECT_SOURCE

            //    ON_ALL_ADJACENT_POINTS

            //#undef X
            //#undef INJECT_SOURCE
                source_v = input;
                source_point_coef = 1.0f;
            }

            // Scattering equation
            float scatter_coeff = 2.f / (static_cast<float>(n_junction_points)
                + source_point_coef);
            float scatter_sum = 0;

        #define ADD_TO_SCATTER_SUM(POINT)    \
            if (mask.test( k##POINT )) {     \
                scatter_sum += GetM_(v_curr_[ k##POINT ], c, k);    \
            }
        #define X    ADD_TO_SCATTER_SUM

            ON_ALL_ADJACENT_POINTS

        #undef X
        #undef ADD_TO_SCATTER_SUM
            scatter_sum += source_v;
            scatter_sum *= scatter_coeff;
            scatter_sum *= alpha_;
            SetM_(junc_v_, c, k, scatter_sum);

            // Junction output (in-place replacement)
        #define COMPUTE_OUTGOING_WAVE(POINT)    \
            if (mask.test( k##POINT )) {    \
                float junc_out = scatter_sum -    \
                    GetM_(v_curr_[ k##POINT ], c, k);    \
                SetM_(v_curr_[ k##POINT ], c, k, junc_out);    \
            }
        #define X    COMPUTE_OUTGOING_WAVE

            ON_ALL_ADJACENT_POINTS

        #undef X
        #undef COMPUTE_OUTGOING_WAVE

            // Delay step
            // Junction output (in-place replacement)
        #define DELAY_STEP(POINT)    \
            if (mask.test( k##POINT )) {    \
                SetM_(v_next_[ k##POINT##_reciprocal ], k##POINT##_C_K, \
                    GetM_(v_curr_[ k##POINT ], c, k));    \
            }
        #define X    DELAY_STEP

            ON_ALL_ADJACENT_POINTS
        
        #undef X
        #undef DELAY_STEP

            // If listener, store output
            if (c == pickup_.c && k == pickup_.k) {
                output = scatter_sum;
            }

        }  // for k
    }  // for c

#undef ON_ALL_ADJACENT_POINTS

    // Swap buffers (next->current)
    float **tmp = v_next_;
    v_next_ = v_curr_;
    v_curr_ = tmp;

    return output;
}

void Triangular2DMesh::SetAttenuation(float mu) {
    assert(mu >= 0.f);
    assert(mu < 1.f);
    alpha_ = 1 - mu;
}
