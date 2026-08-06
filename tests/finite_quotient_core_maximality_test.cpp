#include <cstdio>
#include <vector>

#include "ravel/proof/finite_quotient_core_maximality.hpp"

int main() {
    using Matrix = std::vector<std::vector<long long>>;
    // Branching core, a terminal 3-cycle, and a subordinate one-state loop.
    const Matrix core{{1,1},{1,0}};
    const Matrix terminal{{0,1,0},{0,0,1},{1,0,0}};
    const Matrix subordinate{{1}};
    const auto proof = ravel::proof::derive_finite_quotient_core_maximality(
        core, {terminal, subordinate}, 64);
    if (!proof.core_spectrally_maximal || proof.evidence.size() != 3 ||
        proof.terminal_components != 2) {
        std::fprintf(stderr, "finite quotient core maximality FAIL\n");
        return 1;
    }
    // A competitor with larger growth must not be certified.
    const Matrix larger{{2}};
    const auto obstruction = ravel::proof::derive_finite_quotient_core_maximality(
        Matrix{{1}}, {larger}, 16);
    if (obstruction.core_spectrally_maximal ||
        obstruction.evidence.back().mechanism !=
            ravel::proof::QuotientDominanceMechanism::Unproved) {
        std::fprintf(stderr, "finite quotient obstruction FAIL\n");
        return 1;
    }
    std::printf("finite quotient core maximality PASS terminal=%zu injection=%zu renewal=%zu paired=%zu\n",
                proof.terminal_components, proof.path_injection_components,
                proof.renewal_components, proof.paired_matrix_components);
    return 0;
}
