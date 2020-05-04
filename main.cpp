#include <iostream>


int main(int argc, char **argv) {
    std::cout << "Testing the toolchain...\n";
    for (unsigned int n = 0; n < 15; n++) {
        std::cout << n << ", ";
    }
    std::cout << "\n";
    return 0;
}
