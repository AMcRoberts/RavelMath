#include <cstdio>
#include <vector>
#include "ravel/proof/recurrent_family_exhaustion.hpp"

int main() {
    using namespace ravel::proof;
    std::vector<RecurrentFamilyWitness> witnesses{
        {0, 108, 1, 1, RecurrentFamilyKind::predicted_core,
         true, true, true, false, true, "formula-defined core"},
        {1, 221, 2, 2, RecurrentFamilyKind::transported_competitor,
         true, true, true, false, true, "adjacent twisted renewal"},
        {2, 80, 2, 2, RecurrentFamilyKind::transported_competitor,
         true, true, true, false, true, "adjacent twisted renewal"},
        {3, 6, 3, 3, RecurrentFamilyKind::terminal_permutation,
         true, true, true, false, true, "permutation component has rho=1"}
    };
    const auto recurrent_count = witnesses.size();
    const auto proof = derive_recurrent_family_exhaustion(
        6, recurrent_count, true, true, std::move(witnesses));
    std::printf("%s", render_recurrent_family_exhaustion_report(proof).c_str());
    return proof.proved ? 0 : 1;
}
