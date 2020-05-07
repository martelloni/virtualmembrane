/**
 * @file DSPPythonWrapper.cpp
 * @author Andrea Martelloni (a.martelloni@qmul.ac.uk)
 * @brief 
 * @version 0.1
 * @date 2020-04-30
 * 
 * @copyright Copyright (c) 2020
 * 
 */

#ifdef PYTHON_WRAPPER

#include <boost/python.hpp>
#include <boost/python/numpy.hpp>
// Include user libraries here

using namespace boost::python;
namespace np = boost::python::numpy;


BOOST_PYTHON_MODULE(DSPPythonWrapper)
{
    // Boilerplate to initialise the runtime
    Py_Initialize();
    np::initialize();

    // Define functions here

    // Define classes here
}

#endif
