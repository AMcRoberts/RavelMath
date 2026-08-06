#include <cassert>
#include <iostream>
#include "ravel/proof/symbolic_residual_induction.hpp"
int main() {
    using namespace ravel::proof;
    const auto proof = derive_residual_induction_proof(2);
    assert(proof.valid);
    assert(!proof.quotient_members.empty());
    const auto lean = render_residual_induction_lean(proof);
    assert(lean.find("every_word_has_exact_residual") != std::string::npos);
    std::cout << "symbolic residual induction proof PASS\n";
}
