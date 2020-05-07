/**
 * @file WrapperUtils.cpp
 * @author Andrea Martelloni (a.martelloni@qmul.ac.uk)
 * @brief 
 * @version 0.1
 * @date 2020-04-30
 * 
 * @copyright Copyright (c) 2020
 * 
 */

#include "WrapperUtils.hpp"


#ifdef PYTHON_WRAPPER


WrapperUtils::BufferSize WrapperUtils::CheckAndGetSize(np::ndarray &buffer_main) {
    // Infer samples and channels from array shape.
    unsigned int n_channels;
    unsigned int n_samples;

    switch(buffer_main.get_nd()) {
        case 0: {
            // TODO raise exception
            return { 0, 0 };
        }
        case 1: {
            n_samples = buffer_main.shape(0);
            n_channels = 1;
            break;
        }
        case 2: {
            // Presume interleaved format (innermost dimension is frame size)
            n_samples = buffer_main.shape(0);
            n_channels = buffer_main.shape(1);
            break;
        }
        default: {
            // TODO raise exception
            return { 0, 0 };
        }
    }

    return BufferSize{ n_channels, n_samples };
}


WrapperUtils::BufferSize WrapperUtils::CheckAndGetSize(np::ndarray &buffer_main,
        np::ndarray &buffer_other) {
    assert(buffer_main.shape(0) == buffer_other.shape(0));
    if (buffer_main.get_nd() > 1) {
        assert(buffer_main.shape(1) == buffer_other.shape(1));
    }

    return CheckAndGetSize(buffer_main);
}


template<typename ... Args>
WrapperUtils::BufferSize WrapperUtils::CheckAndGetSize(np::ndarray &buffer_main,
        np::ndarray &buffer_other, Args ... args) {
    assert(buffer_main.shape(0) == buffer_other.shape(0));
    if (buffer_main.get_nd() > 1) {
        assert(buffer_main.shape(1) == buffer_other.shape(1));
    }

    return CheckAndGetSize(buffer_main, args...);
}


#endif  // PYTHON_WRAPPER
