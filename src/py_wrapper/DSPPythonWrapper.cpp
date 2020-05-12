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
        
        p::tuple sizetuple = p::make_tuple(pi_.total_size_ck, 2);
        np::ndarray output = np::empty(sizetuple,
            np::dtype::get_builtin<float>());
        float *data = reinterpret_cast<float *>(output.get_data());

        // Resolve XY coordinates of every (c, k) point in the mesh
        unsigned int flat_n = 0;
        FOREACH_MESH_POINT({
            float x;
            float y;
            CKtoXY_(c, k, x, y);
            data[flat_n++] = x;
            data[flat_n++] = y;
        });

        return output;
    }

    np::ndarray GetV() {

        p::tuple sizetuple = p::make_tuple(pi_.total_size_ck);
        np::ndarray output = np::empty(sizetuple,
            np::dtype::get_builtin<float>());
        float *data = reinterpret_cast<float *>(output.get_data());
        // Copy V values across all (c, k) points
        unsigned int flat_n = 0;
        FOREACH_MESH_POINT({
            data[flat_n++] = GetM_(meshVJunc_, c, k);
        });

        return output;
    }

    np::ndarray GetMask() {

        p::tuple sizetuple = p::make_tuple(pi_.total_size_ck, 6);
        np::ndarray output = np::empty(sizetuple,
            np::dtype::get_builtin<bool>());
        bool *data = reinterpret_cast<bool *>(output.get_data());
        // Copy V values across all (c, k) points
        unsigned int flat_n = 0;
        FOREACH_MESH_POINT({
            unsigned int maskval = GetM_(mesh_mask_, c, k);
            std::bitset<kNWaveguides> mask(maskval);
            for (unsigned int n = 0; n < kNWaveguides; n++)
                data[flat_n++] = mask.test(n);
        });

        return output;
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
    using mesh = Triangular2DMesh_py;
    class_<mesh>("Triangular2DMesh",
        init<float, float, float>())
        .def("GetMeshCoordinates",
            &mesh::GetMeshCoordinates)
        .def("GetV", &mesh::GetV)
        .def("GetMask", &mesh::GetMask)
        .def("SetSource", &mesh::SetSource)
        .def("SetPickup", &mesh::SetPickup)
        .def("ProcessSample", &mesh::ProcessSample);
}

#endif
