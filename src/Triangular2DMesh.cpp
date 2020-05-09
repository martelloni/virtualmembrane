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
    pi_.k_size = (pi_.x_size >> 1) + 1;
    pi_.total_size_ck = pi_.c_size * pi_.k_size;
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

