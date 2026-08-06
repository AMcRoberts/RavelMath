#include <iostream>
#include <stdexcept>
#include "ravel/proof/plastic_three_generator_intertwiner.hpp"

int main() {
    const auto c = ravel::proof::derive_plastic_three_generator_intertwiner();
    if (!c.proved) throw std::runtime_error(c.obstruction);
    if (!c.every_boundary_edge_has_universal_witness)
        throw std::runtime_error("missing universal witness");
    if (!c.g0_intertwines || !c.gplus_intertwines || !c.gminus_intertwines)
        throw std::runtime_error("three-generator intertwiner failed");
    if (!c.finite_positive_grammar_ready)
        throw std::runtime_error("finite-positive-grammar bridge unavailable");
    if (c.generator_count != 3 || c.boundary_states != 101 || c.boundary_edges != 125)
        throw std::runtime_error("plastic benchmark changed unexpectedly");
    std::cout << "plastic three-generator intertwiner PASS\n"
              << "boundary_states=" << c.boundary_states
              << " universal_roles=" << c.universal_roles
              << " boundary_edges=" << c.boundary_edges
              << " universal_edges=" << c.universal_edges << "\n";
}
