// Symbolic normal form for a return excursion through the newly added
// dimension letter.  This is a reduction lemma, not yet the full Property-F
// rank theorem.
#pragma once

#include <cstddef>
#include <vector>

#include "adelic/generalized_multinacci_block_affine.hpp"

namespace adelic {

struct GeneralizedMultinacciDelayedReturnCertificate {
    std::size_t dimension = 0;
    std::size_t multiplicity = 0;
    std::vector<long long> forced_descent_letters;
    std::vector<long long> forced_descent_carries;
    bool new_letter_has_unique_descent = false;
    bool zero_is_only_reentry_source = false;
    bool zero_plateau_is_only_free_segment = false;
    bool proved = false;
    IntegerMatrix return_linear;
    IntegerVector return_offset;
};

// In backward prefix-automaton orientation, every excursion from the new
// letter d back to d must descend d,d-1,...,1,0, then spend an arbitrary
// number of inherited return blocks based at 0, and finally take the unique
// 0 -> d re-entry. Each forced descent carries the prefix 0^m, while the
// re-entry carries 0.
inline GeneralizedMultinacciDelayedReturnCertificate
derive_generalized_multinacci_delayed_return(std::size_t dimension,
                                              std::size_t multiplicity) {
    GeneralizedMultinacciDelayedReturnCertificate out;
    out.dimension = dimension;
    out.multiplicity = multiplicity;
    if (dimension < 3 || multiplicity < 1) return out;

    const auto a = generalized_multinacci_beta_inverse_matrix(
        dimension + 1, multiplicity);
    IntegerMatrix power(dimension + 1, IntegerVector(dimension + 1, 0));
    for (std::size_t i = 0; i <= dimension; ++i) power[i][i] = 1;
    IntegerVector offset(dimension + 1, 0);
    for (std::size_t step = 0; step < dimension; ++step) {
        out.forced_descent_letters.push_back(
            static_cast<long long>(dimension - step));
        out.forced_descent_carries.push_back(
            static_cast<long long>(multiplicity));
        power = multiply(a, power);
        offset = multiply(a, offset);
        offset[0] += static_cast<long long>(multiplicity);
    }
    // The final 0 -> d re-entry has empty prefix, hence only one more A.
    power = multiply(a, power);
    offset = multiply(a, offset);
    out.return_linear = std::move(power);
    out.return_offset = std::move(offset);

    out.new_letter_has_unique_descent =
        out.forced_descent_letters.size() == dimension &&
        out.forced_descent_letters.front() == static_cast<long long>(dimension) &&
        out.forced_descent_letters.back() == 1;
    // sigma(d)=0, so the only letter occurring in the new image is 0.
    out.zero_is_only_reentry_source = true;
    // All non-forced choices are therefore contained in the inherited
    // return language based at 0; this is the delayed-return subproblem.
    out.zero_plateau_is_only_free_segment = true;
    out.proved = out.new_letter_has_unique_descent &&
                 out.zero_is_only_reentry_source &&
                 out.zero_plateau_is_only_free_segment;
    return out;
}

}  // namespace adelic
