/**
 * @file DetectHit.hpp
 * @author Andrea Martelloni (a.martelloni@qmul.ac.uk)
 * @brief 
 * @version 0.1
 * @date 2020-05-14
 * 
 * @copyright Copyright (c) 2020
 * 
 */

#ifndef __DETECT_HIT_HPP__
#define __DETECT_HIT_HPP__


class DetectHit {

 public:
    DetectHit();
    void Reset();
    void SetThresh(float thresh);
    void SetHPFCoef(float b0, float b1, float a1);
    void SetLPFCoef(float b0, float b1, float a1);
    struct CurrentState {
        bool hit_detected;
        float value;
    };
    CurrentState ProcessSample(float input);

 protected:
    struct FiltCoef_ {
        float b0;
        float b1;
        float a1;
    };
    struct FiltHist_ {
        float x;
        float y;
    };
    FiltCoef_ hpf_;
    FiltCoef_ lpf_;
    FiltHist_ hpf_z_;
    FiltHist_ lpf_z_;
    float thresh_;

    __attribute__((always_inline))
    static float FiltEngine_(FiltCoef_ &c, FiltHist_ &z, float x) {
        z.y = c.b0 * x + c.b1 * z.x - c.a1 * z.y;
        z.x = x;
        return z.y;
    }

    __attribute__((always_inline))
    static float FiltEngineBypass_(FiltHist_ &z, float x) {
        z.x = z.y = x;
        return z.y;
    }
};


#endif  // __DETECT_HIT_HPP__
