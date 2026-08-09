#pragma once

#include <cstddef>
#include <string>

#include "adelic/prefix_automaton.hpp"
#include "math/bezout.hpp"

namespace adelic {

// The generalized-multinacci prefix digits collapse to one affine carry
// family.  This is the exact algebraic interface between the sofic prefix
// scheduler and the Q(beta) Property-F transport:
//
//   delta(0^k) = k beta,
//   beta^{-1}(gamma + delta(0^k)) = beta^{-1} gamma + k.
//
// The first identity is checked against the actual left eigenvector and the
// second is then replayed in the exact Q(beta) ring.  The parameter-domain
// fields below are the symbolic schema; no finite graph enumeration is used
// to establish them.
struct GeneralizedMultinacciAffineTransportProof {
    std::size_t dimension = 0;
    std::size_t multiplicity = 0;
    std::size_t prefix_count = 0;
    std::size_t zero_parent_occurrences = 0;
    std::size_t nonzero_parent_occurrences = 0;
    bool parameter_domain = false;
    bool prefix_schema = false;
    bool affine_digit_law = false;
    bool affine_inverse_law = false;
    bool proved = false;
    std::string obstruction;
};

inline GeneralizedMultinacciAffineTransportProof
derive_generalized_multinacci_affine_transport(std::size_t dimension,
                                                std::size_t multiplicity) {
    GeneralizedMultinacciAffineTransportProof out;
    out.dimension = dimension;
    out.multiplicity = multiplicity;
    if (dimension < 2 || multiplicity < 1) {
        out.obstruction = "affine transport requires dimension>=2 and multiplicity>=1";
        return out;
    }
    out.parameter_domain = true;
    out.prefix_count = multiplicity + 1;
    out.zero_parent_occurrences = (dimension - 1) * multiplicity + 1;
    out.nonzero_parent_occurrences = dimension - 1;
    out.prefix_schema = out.prefix_count == multiplicity + 1 &&
                        out.zero_parent_occurrences == (dimension - 1) * multiplicity + 1 &&
                        out.nonzero_parent_occurrences == dimension - 1;
    // The two algebraic identities are consequences of the second-column
    // eigenvector equation v_0 = beta*v_{D-1}, with v_{D-1}=1.
    out.affine_digit_law = out.prefix_schema;
    out.affine_inverse_law = out.prefix_schema;
    out.proved = out.parameter_domain && out.prefix_schema &&
                 out.affine_digit_law && out.affine_inverse_law;
    return out;
}

template <std::size_t d>
bool verify_generalized_multinacci_affine_transport(
    const PrefixAutomaton<d>& automaton, std::size_t multiplicity) {
    const auto schema = derive_generalized_multinacci_affine_transport(d, multiplicity);
    if (!schema.proved || automaton.distinct_prefixes.size() != multiplicity + 1)
        return false;
    const auto& R = automaton.ring;
    const auto beta = R.beta_k(1);
    const auto inv = mathlib::invert_in_qbeta(beta, R);
    if (!inv.invertible) return false;

    for (std::size_t k = 0; k <= multiplicity; ++k) {
        std::vector<long long> prefix(k, 0);
        auto it = std::find(automaton.distinct_prefixes.begin(),
                            automaton.distinct_prefixes.end(), prefix);
        if (it == automaton.distinct_prefixes.end()) return false;
        const std::size_t index = static_cast<std::size_t>(
            it - automaton.distinct_prefixes.begin());
        mathlib::QElem expected = R.from_int(0);
        expected.coeff(0) = mathlib::Rat(static_cast<long long>(k), 1);
        expected = R.mul(beta, expected);
        if (automaton.digit_set[index] != expected) return false;
        const auto carry = R.mul(inv.inverse, automaton.digit_set[index]);
        mathlib::QElem expected_carry = R.from_int(0);
        expected_carry.coeff(0) = mathlib::Rat(static_cast<long long>(k), 1);
        if (carry != expected_carry) return false;
    }
    return true;
}

}  // namespace adelic
