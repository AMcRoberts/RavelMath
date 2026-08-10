// Exact spectral certificate for an induced return substitution.
//
// This keeps the cyclotomic-fibre observation machine-checkable: the
// incidence matrix is built from the live return images, and its exact
// characteristic polynomial is compared with the claimed Perron/cyclotomic
// factorization.  It is deliberately a certificate, not a numerical
// eigenvalue computation.
#pragma once

#include <stdexcept>
#include <vector>

#include "math/charpoly.hpp"
#include "math/poly_z.hpp"
#include "ravel/return_substitution.hpp"

namespace ravel {

struct ReturnCoreSpectralCertificate {
    std::vector<std::vector<long long>> incidence;
    mathlib::PolyZ characteristic;
    mathlib::PolyZ expected;
    mathlib::PolyZ expanding_factor;
    bool exact_factorization = false;
    bool holds = false;
};

inline std::vector<std::vector<long long>> return_incidence_matrix(
        const ReturnSubstitution& induced) {
    const std::size_t n = induced.words.size();
    std::vector<std::vector<long long>> matrix(
        n, std::vector<long long>(n, 0));
    for (std::size_t source = 0; source < n; ++source) {
        for (std::size_t destination : induced.derived_images[source]) {
            if (destination >= n) {
                throw std::runtime_error(
                    "return_incidence_matrix: image index out of range");
            }
            ++matrix[destination][source];
        }
    }
    return matrix;
}

inline ReturnCoreSpectralCertificate certify_return_core_spectrum(
        const ReturnSubstitution& induced,
        const mathlib::PolyZ& expected_expanding_factor,
        const mathlib::PolyZ& expected_cyclotomic_factor) {
    ReturnCoreSpectralCertificate result;
    result.incidence = return_incidence_matrix(induced);
    result.characteristic =
        mathlib::charpoly_faddeev_leverrier(result.incidence);
    result.expanding_factor = expected_expanding_factor;
    result.expected = expected_expanding_factor * expected_cyclotomic_factor;
    result.exact_factorization = result.characteristic == result.expected;
    result.holds = result.exact_factorization;
    return result;
}

}  // namespace ravel
