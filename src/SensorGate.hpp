/**
 * @file SensorGate.hpp
 * @author Andrea Martelloni (a.martelloni@qmul.ac.uk)
 * @brief 
 * @version 0.1
 * @date 2020-07-09
 * 
 * @copyright Copyright (c) 2020
 * 
 */

#ifndef __SENSOR_GATE_HPP__
#define __SENSOR_GATE_HPP__

#if 0

class SensorGate {
 public:
    
    SensorGate();
    void SetAttack(float alpha);
    void SetRelease(float alpha);
    void SetThreshold(float thr);
    float ProcessSample(float in);

 protected:
    float attack_alpha_;
    float release_alpha_;
    float threshold_;
    float smoother_state_;
}

#endif // 0

#endif  // __SENSOR_GATE_HPP__
