#if !defined(PYTHON_WRAPPER)


#include <iostream>
#include "Triangular2DMesh.hpp"

int main(int argc, char **argv) {
    std::cout << "Testing the toolchain...\n";
    for (unsigned int n = 0; n < 15; n++) {
        std::cout << n << ", ";
    }
    std::cout << "\n";

    Triangular2DMesh::Properties properties {
        70.f,
        50.f,
        5.f,
    };
    char *meshmem = new char[Triangular2DMesh::GetMemSize(properties)];
    Triangular2DMesh mesh(properties, meshmem);
    
    return 0;
}

#endif  // !defined(PYTHON_WRAPPER)
