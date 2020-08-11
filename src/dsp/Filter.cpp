/**
 * @file Filter.cpp
 * @author Andrea Martelloni (a.martelloni@qmul.ac.uk)
 * @brief 
 * @version 0.1
 * @date 2020-08-11
 * 
 * @copyright Copyright (c) 2020
 * 
 */

#include "Filter.hpp"

namespace DSP {


ARSmoother::ARSmoother(float alpha_attack, float alpha_release) :
    alpha_a_(alpha_attack),
    alpha_r_(alpha_release) {
    Reset();
}


void ARSmoother::Reset() {
    y_1_ = 0;
}


float ARSmoother::ProcessSample(float in) {
    float alpha = (in > y_1_) ? alpha_a_ : alpha_r_;
    y_1_ = in * alpha + (1.f - alpha) * y_1_;
    return y_1_;
}


}  // namespace DSP
