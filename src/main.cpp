#if !defined(PYTHON_WRAPPER)


#if defined(CATCH2_TEST)


#define CATCH_CONFIG_MAIN
#include "catch.hpp"
#include <cmath>


// All tested classes are friends, yay!
#define private public
#define protected public


#include "Triangular2DMesh.hpp"
using mesh = Triangular2DMesh;


TEST_CASE( "Check dimensions", "[Triangular2DMesh]" ) {

    // Mesh with these properties:
    mesh::Properties p {
        54.9f,  // mm width
        27.5f,  // mm height
        5.f };  // mm resolution
    // Expected properties
    unsigned int meshsize_x = 23;
    unsigned int meshsize_y = 8;
    unsigned int meshsize = meshsize_x * meshsize_y;
    unsigned int c_size = 8;
    unsigned int k_size_even = 12;
    unsigned int k_size_odd = 11;
    unsigned int meshsize_ck = c_size * k_size_even - (c_size >> 1);
    size_t expected_memsize = sizeof(float) * meshsize * 14;
    
    // Test important static properties
    size_t memsize = mesh::GetMemSize(p);
    REQUIRE(memsize == expected_memsize);

    // Test creation of mesh
    char *mem = new char[memsize];
    mesh m(p, mem);
    CHECK(m.pi_.x_size == meshsize_x);
    CHECK(m.pi_.y_size == meshsize_y);
    CHECK(m.pi_.total_size == meshsize);
    CHECK(m.pi_.c_size == c_size);
    CHECK(m.pi_.k_size_even == k_size_even);
    CHECK(m.pi_.k_size_odd == k_size_odd);
    CHECK(m.pi_.total_size_ck == meshsize_ck);

    // Test default source and pickup
    CHECK(m.source_.c <= c_size);
    CHECK(m.source_.k <= k_size_even - (m.source_.c & 0x1));
    CHECK(m.pickup_.c <= c_size);
    CHECK(m.pickup_.k <= k_size_even - (m.pickup_.c & 0x1));

    // Cleanup
    delete mem;
}


TEST_CASE( "Set source and pickup", "[Triangular2DMesh]" ) {

    mesh::Properties p {
        54.9f,  // mm width
        27.5f,  // mm height
        5.f };  // mm resolution
    char *mem = new char[mesh::GetMemSize(p)];
    mesh m(p, mem);

    // Set pretty arbitrary source and pickup points
    m.SetSource(23.5, 19.1);
    m.SetPickup(0.18, 1.2);

    CHECK(m.source_.c == 4);
    CHECK(m.source_.k == 4);
    CHECK(m.pickup_.c == 0);
    CHECK(m.pickup_.k == 0);

    // Cleanup
    delete mem;
}


TEST_CASE(  "Process Gaussian impulse", "[Triangular2DMesh]" ) {

    // Impulse imported from PythonValidation.ipynb
    unsigned int impulse_length = 21;
    float impulse[impulse_length] = {
        0.01137871167519538, 0.026633939280853638, 0.057003245030845216, 0.11155412024665476, 
        0.19961515645360556, 0.32660533403633385, 0.488624112788967, 0.6684183720941176, 
        0.8360720857424218, 0.956226842945176, 1.0, 0.956226842945176, 
        0.8360720857424218, 0.6684183720941176, 0.488624112788967, 0.32660533403633385, 
        0.19961515645360556, 0.11155412024665476, 0.057003245030845216, 0.026633939280853638, 
        0.01137871167519538
    };
    float result[impulse_length];
    // Mesh with these properties:
    mesh::Properties p {
        54.9f,  // mm width
        27.5f,  // mm height
        5.f };  // mm resolution
    char *mem = new char[mesh::GetMemSize(p)];
    mesh m(p, mem);

    for (unsigned int n = 0; n < impulse_length; n++) {
        result[n] = m.ProcessSample(true, impulse[n]);
        CHECK(!std::isnan(result[n]));
    }

    // Cleanup
    delete mem;
}

#endif  // defined(CATCH2_TEST)

#endif  // !defined(PYTHON_WRAPPER)
