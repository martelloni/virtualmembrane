/**
 * @file Geometries.hpp
 * @author Andrea Martelloni (a.martelloni@qmul.ac.uk)
 * @brief 
 * @version 0.1
 * @date 2020-06-30
 * 
 * @copyright Copyright (c) 2020
 * 
 */

#ifndef __GEOMETRIES_HPP__
#define __GEOMETRIES_HPP__

#include <cmath>
#include <functional>
#include "Triangular2DMesh.hpp"


namespace Geometries {


std::function<bool (float, float)> CircularMembrane(
    float radius
);


}

#endif  // __GEOMETRIES_HPP__
