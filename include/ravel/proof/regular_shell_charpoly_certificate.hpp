// ravel/proof/regular_shell_charpoly_certificate.hpp
//
// Wires docs/FAMILY_OF_FAMILIES.md's displayed regular-shell closed forms
// for the three Class-II adjacent-swap neighbors to the reflection
// pipeline: computes the CONCRETE compressed matrix for
// (neighbor, a, t) via class_ii_neighbor_compressed_matrix_for_states,
// its exact integer characteristic polynomial via
// charpoly_faddeev_leverrier (self-checked internally via Cayley-
// Hamilton), and independently verifies it equals the expected closed
// form -- only then records the certificate, so the renderer instantiates
// lean/class_ii_neighbor_dominance.lean's neighbor{0,1,2}_shell_below_*
// theorems at this CONCRETE (a, t), not merely restate the closed form.
//
// lean/class_ii_neighbor_dominance.lean was previously flat: zero C++
// consumer anywhere. The naive first guess (that "lambda" in those
// theorems is the FIXED dominant core's own Perron root) is WRONG --
// verified empirically: the dominant core's own charpoly (degree 15/17/39)
// does not reduce to the simple quadratic those theorems' hypotheses
// expect. docs/FAMILY_OF_FAMILIES.md's "indexed shell spectra" section
// (own text, not re-derived) identifies the correct object: the growing
// REGULAR SHELL component indexed by parameter t (or k for neighbor 2),
// whose charpoly this file verifies exactly.

#pragma once

#include <string>

#include "math/charpoly.hpp"
#include "math/proof_reflection.hpp"
#include "ravel/class_ii_neighbor_family.hpp"

namespace ravel::proof {

// Self-validating check: computes the concrete compressed matrix and its
// exact charpoly, and independently verifies it equals the closed form
// docs/FAMILY_OF_FAMILIES.md displays -- not merely trusting the state
// count or assuming the formula holds.
inline bool certify_regular_shell_charpoly(long long neighbor, long long a, long long t) {
    if (neighbor < 0 || neighbor > 2) return false;
    if (a < 3) return false;
    const long long min_t = (neighbor == 2) ? 2 : 1;
    const long long max_t = (neighbor == 0) ? a - 3 : (neighbor == 1) ? a - 2 : a - 1;
    if (t < min_t || t > max_t) return false;

    const auto states = class_ii_neighbor_regular_shell_states(
        static_cast<std::size_t>(neighbor), a, t);
    const auto mat = class_ii_neighbor_compressed_matrix_for_states(
        static_cast<std::size_t>(neighbor), a, states);
    const auto poly = mathlib::charpoly_faddeev_leverrier(mat);

    // Expected closed form (low-to-high coefficients), per neighbor:
    //   0: x^6 * (x^4 - ((t+2)^2-2) x^2 + 1)  -- degree 10, states.size()==10
    //   1: x^3 * (x^2 - t(t+2))               -- degree 5,  states.size()==5
    //   2: x^2 * (x^2 - k^2)                  -- degree 4,  states.size()==4
    std::vector<long long> expected(states.size() + 1, 0);
    if (neighbor == 0) {
        if (states.size() != 10) return false;
        const long long c = (t + 2) * (t + 2) - 2;
        expected[6] = 1;
        expected[8] = -c;
        expected[10] = 1;
    } else if (neighbor == 1) {
        if (states.size() != 5) return false;
        expected[3] = -(t * (t + 2));
        expected[5] = 1;
    } else {
        if (states.size() != 4) return false;
        expected[2] = -(t * t);
        expected[4] = 1;
    }
    if (poly.coeffs_.size() != expected.size()) return false;
    for (std::size_t i = 0; i < expected.size(); ++i) {
        long long actual = 0;
        if (i < poly.coeffs_.size()) {
            // BigInt -> long long, safe at these coefficient magnitudes.
            actual = std::stoll(mathlib::str(poly.coeffs_[i]));
        }
        if (actual != expected[i]) return false;
    }
    return true;
}

inline void stage_regular_shell_charpoly(long long neighbor, long long a, long long t,
                                          const std::string& description) {
    if (!certify_regular_shell_charpoly(neighbor, a, t)) return;
    if (!mathlib::reflection::enabled()) return;
    mathlib::reflection::RegularShellCharpolyCertificate node;
    node.neighbor = neighbor;
    node.a = a;
    node.t = t;
    node.description = description;
    mathlib::reflection::record(mathlib::reflection::NodeKind::LemmaApplication, node);
}

}  // namespace ravel::proof
