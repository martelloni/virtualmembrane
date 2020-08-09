/**
 * @file Geometries.cpp
 * @author Andrea Martelloni (a.martelloni@qmul.ac.uk)
 * @brief 
 * @version 0.1
 * @date 2020-06-30
 * 
 * @copyright Copyright (c) 2020
 * 
 */


#include "Geometries.hpp"


namespace Geometries {


std::function<bool (float, float)> CircularMembrane(
    float radius
) {
    return [=](float x, float y) {
        float z = std::pow((x - radius), 2) +
            std::pow((y - radius), 2) -
            radius * radius;
        return static_cast<bool>(z <= 0);
    };
}


}
