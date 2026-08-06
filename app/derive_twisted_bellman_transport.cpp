#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>

#include "ravel/proof/twisted_bellman_transport.hpp"

int main(int argc, char** argv) {
    try {
        const std::string output = argc > 1
            ? argv[1]
            : "lean/generated/twisted_bellman_transport.lean";
        std::ofstream file(output);
        if (!file) throw std::runtime_error("cannot open Lean output");
        file << ravel::proof::render_twisted_bellman_transport_lean();
        std::cout << "TWISTED_BELLMAN_TRANSPORT_LEAN\n";
        std::cout << "output=" << output << "\n";
        std::cout << "result=EMITTED\n";
        return 0;
    } catch (const std::exception& error) {
        std::cerr << "ERROR: " << error.what() << "\n";
        return 1;
    }
}
