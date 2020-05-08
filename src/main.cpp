#if !defined(PYTHON_WRAPPER)


#if defined(CATCH2_TEST)


#define CATCH_CONFIG_MAIN
#include "catch.hpp"


// All tested classes are friends, yay!
#define private public


#include "Triangular2DMesh.hpp"


TEST_CASE( "Check dimensions", "[Triangular2DMesh]" ) {
    using mesh = Triangular2DMesh;

    mesh::Properties p { 21.3f, 19.98f, 5.f };
    size_t meshsize = mesh::GetMemSize(p);
    REQUIRE(meshsize > 0);
}


#endif  // defined(CATCH2_TEST)

#endif  // !defined(PYTHON_WRAPPER)
