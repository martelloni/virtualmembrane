/**
 * @file WrapperUtils.hpp
 * @author Andrea Martelloni (a.martelloni@qmul.ac.uk)
 * @brief 
 * @version 0.1
 * @date 2020-04-30
 * 
 * @copyright Copyright (c) 2020
 * 
 */

#ifndef __WRAPPER_UTILS_HPP__
#define __WRAPPER_UTILS_HPP__

#ifdef PYTHON_WRAPPER

#include <boost/python.hpp>
#include <boost/python/numpy.hpp>
namespace p = boost::python;
namespace np = boost::python::numpy;


/**
 * @brief Utilities to use Boost.Python in a DSP use case with
 * more explanatory/less verbose syntax.
 * 
 */
class WrapperUtils {
 public:
    /**
     * @brief Size of the buffer, inferred by the dimensions of the
     * Numpy array.
     * Only 2-d arrays are supported for now, and they're
     * assumed to be interleaved, i.e. shape[0] = n_samples and
     * shape[1] = n_channels
     * 
     */
    struct BufferSize {
        unsigned int n_channels;
        unsigned int n_samples;
    };
    /**
     * @brief Check and return the size of a Numpy array.
     * Only 2-d arrays are supported for now, and they're
     * assumed to be interleaved, i.e. shape[0] = n_samples and
     * shape[1] = n_channels
     * 
     * @param buffer_main 
     * @return BufferSize 
     */
    static BufferSize CheckAndGetSize(np::ndarray &buffer_main);
    /**
     * @brief Check that two Numpy arrays are identical in shape,
     * and return their size.
     * Only 2-d arrays are supported for now, and they're
     * assumed to be interleaved, i.e. shape[0] = n_samples and
     * shape[1] = n_channels
     * 
     * @param buffer_main 
     * @param buffer_other 
     * @return BufferSize 
     */
    static BufferSize CheckAndGetSize(np::ndarray &buffer_main, np::ndarray &buffer_other);
    /**
     * @brief Check that a set of Numpy arrays are identical in shape,
     * and return their size.
     * Only 2-d arrays are supported for now, and they're
     * assumed to be interleaved, i.e. shape[0] = n_samples and
     * shape[1] = n_channels
     * 
     * @tparam Args List of other buffers to be compared against buffer_main
     * @param buffer_main 
     * @param buffer_other 
     * @param args 
     * @return BufferSize 
     */
    template<typename ... Args>
    static BufferSize CheckAndGetSize(np::ndarray &buffer_main, np::ndarray &buffer_other,
            Args ... args);
    /**
     * @brief Convert the Numpy array type to the desired native C++ type.
     * It creates and returns a copy of the Numpy array, or a reference
     * to the original array if the dtype of buffer is compatible with
     * the desired C++ type.
     * 
     * @tparam CPP_TYPE Type (e.g. float, int of a given precision...)
     * @param buffer 
     * @return np::ndarray 
     */
    template<typename CPP_TYPE>
    static np::ndarray CheckAndConvertType(np::ndarray &buffer);
    /**
     * @brief Allocates a buffer of a given size and type
     * as a Numpy array. Useful to return
     * processing results non-destructively.
     * 
     * @tparam CPP_TYPE 
     * @param size 
     * @return np::ndarray 
     */
    template<typename CPP_TYPE>
    static np::ndarray AllocateBuffer(BufferSize size);
    /**
     * @brief Get data from a Numpy array as a CPP_TYPE* pointer,
     * for use in processing.
     * 
     * @tparam CPP_TYPE 
     * @param buffer 
     * @return CPP_TYPE* 
     */
    template<typename CPP_TYPE>
    static inline CPP_TYPE* GetData(np::ndarray &buffer);
};


template<typename CPP_TYPE>
np::ndarray WrapperUtils::CheckAndConvertType(np::ndarray &buffer) {
    if (buffer.get_dtype() == np::dtype::get_builtin<CPP_TYPE>()) {
        return buffer;
    } else {
        return buffer.astype(np::dtype::get_builtin<CPP_TYPE>());
    }
}


template<typename CPP_TYPE>
np::ndarray WrapperUtils::AllocateBuffer(BufferSize size) {
    p::tuple sizetuple;
    if (1 == size.n_channels) {
        // 1-d array
        sizetuple = p::make_tuple(size.n_samples);
    } else {
        // Interleaved array
        sizetuple = p::make_tuple(size.n_samples, size.n_channels);
    }
    return np::zeros(sizetuple, np::dtype::get_builtin<CPP_TYPE>());
}


template<typename CPP_TYPE>
CPP_TYPE* WrapperUtils::GetData(np::ndarray &buffer) {
    return reinterpret_cast<CPP_TYPE *>(buffer.get_data());
}


#endif  // PYTHON_WRAPPER

#endif  // __WRAPPER_UTILS_HPP__
