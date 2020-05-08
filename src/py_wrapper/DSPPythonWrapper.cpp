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
#include <cassert>
// Include user libraries here
#include "Triangular2DMesh.hpp"

namespace p = boost::python;
namespace np = boost::python::numpy;


class Triangular2DMesh_py : public Triangular2DMesh {
 public:
    Triangular2DMesh_py(float x, float y, float spatial_res) :
        Triangular2DMesh(),
        mesh_mem_(new float[GetMemSize(Properties { x, y, spatial_res })]) {
        
        Init_(Properties { x, y, spatial_res }, mesh_mem_);
    }

    ~Triangular2DMesh_py() {
        delete mesh_mem_;
    }

    np::ndarray GetMeshCoordinates() {
        p::tuple sizetuple = p::make_tuple(pi_.c_size, pi_.k_size, 2);
        np::ndarray output = np::empty(sizetuple,
            np::dtype::get_builtin<float>());
        float *data = reinterpret_cast<float *>(output.get_data());
        // Resolve XY coordinates of every (c, k) point in the mesh
        FOREACH_MESH_POINT({
            float x;
            float y;
            CKtoXY_(c, k, x, y);
            data[(c * pi_.c_size + k) * 2] = x;
            data[(c * pi_.c_size + k) * 2 + 1] = y;
        });

        return output;
    }

    np::ndarray GetV(unsigned int index) {
        p::tuple sizetuple = p::make_tuple(pi_.c_size, pi_.k_size);
        np::ndarray output = np::empty(sizetuple,
            np::dtype::get_builtin<float>());
        // Copy V values across all (c, k) points
    }

 protected:

    float *mesh_mem_;
};

using namespace boost::python;

BOOST_PYTHON_MODULE(DSPPythonWrapper)
{
    // Boilerplate to initialise the runtime
    Py_Initialize();
    np::initialize();

    // Define functions here

    // Define classes here
    class_<Triangular2DMesh_py>("Triangular2DMesh",
        init<float, float, float>())
        .def("GetMeshCoordinates",
            &Triangular2DMesh_py::GetMeshCoordinates);
}

#endif
