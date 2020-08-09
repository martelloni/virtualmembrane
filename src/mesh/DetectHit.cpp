#include "DetectHit.hpp"
#include <cmath>


DetectHit::DetectHit() :
    hpf_{ 1., 0., 0., },
    lpf_{ 1., 0., 0., },
    thresh_(0.5) {
    
    Reset();
}


void DetectHit::Reset() {
    lpf_z_ = { 0, 0 };
    hpf_z_ = { 0, 0 };
}


void DetectHit::SetThresh(float thresh) {
    thresh_ = thresh;
}


void DetectHit::SetHPFCoef(float b0, float b1, float a1) {
    hpf_.b0 = b0;
    hpf_.b1 = b1;
    hpf_.a1 = a1;
}


void DetectHit::SetLPFCoef(float b0, float b1, float a1) {
    lpf_.b0 = b0;
    lpf_.b1 = b1;
    lpf_.a1 = a1;
}


DetectHit::CurrentState DetectHit::ProcessSample(float input) {

    // Remove DC
    input = FiltEngine_(hpf_, hpf_z_, input);
    // Full-wave rectification
    input = std::abs(input);
    // Remove fizz
    input = FiltEngine_(lpf_, lpf_z_, input);

    return { true, input };
}