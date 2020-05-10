#if !defined(PYTHON_WRAPPER)


#if defined(CATCH2_TEST)


#define CATCH_CONFIG_MAIN
#include "catch.hpp"
#include <cmath>


// All tested classes are friends, yay!
#define private public
#define protected public


#include "Triangular2DMesh.hpp"


TEST_CASE( "Check dimensions", "[Triangular2DMesh]" ) {
    using mesh = Triangular2DMesh;

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
    unsigned int k_size = 12;
    unsigned int meshsize_ck = c_size * k_size;
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
    CHECK(m.pi_.k_size == k_size);
    CHECK(m.pi_.total_size_ck == meshsize_ck);

    // Test default source and pickup
    CHECK(m.source_.c <= c_size);
    CHECK(m.source_.k <= k_size - (m.source_.c & 0x1));
    CHECK(m.pickup_.c <= c_size);
    CHECK(m.pickup_.k <= k_size - (m.pickup_.c & 0x1));

    // Cleanup
    delete mem;
}


TEST_CASE( "Set source and pickup", "[Triangular2DMesh]" ) {

}


#endif  // defined(CATCH2_TEST)

#endif  // !defined(PYTHON_WRAPPER)
