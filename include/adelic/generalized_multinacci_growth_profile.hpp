// Structural growth diagnostics for the generalized-multinacci transport.
//
// This is deliberately a profile, not a Property-(F) proof.  It records the
// three parameters that control the size of the finite carry cloud before an
// exact graph search is attempted: the number of return-block channels, the
// degree of the coefficient lattice, and the weakest secondary-root gap.
#pragma once

#include <algorithm>
#include <complex>
#include <cstddef>
#include <cmath>
#include <limits>
#include <stdexcept>
#include <vector>

#include "adelic/coincidence_and_property_f.hpp"
#include "ravel/generalized_multinacci.hpp"

namespace adelic {

struct GeneralizedMultinacciGrowthProfile {
    std::size_t dimension = 0;
    std::size_t multiplicity = 0;
    std::size_t coefficient_dimension = 0;
    std::size_t return_channels = 0;
    double beta = 0.0;
    double secondary_modulus = 0.0;
    double inverse_secondary_gap = 0.0;
    double secondary_carry_radius = 0.0;
    // Floating-point resource proxies, not proof certificates.
    double maximum_coefficient_bound = 0.0;
    double coefficient_box_log10 = 0.0;
};

inline std::vector<std::vector<std::complex<double>>>
invert_complex_matrix(std::vector<std::vector<std::complex<double>>> a) {
    const std::size_t n = a.size();
    std::vector<std::vector<std::complex<double>>> aug(
        n, std::vector<std::complex<double>>(2 * n, {0.0, 0.0}));
    for (std::size_t i = 0; i < n; ++i) {
        if (a[i].size() != n) return {};
        for (std::size_t j = 0; j < n; ++j) aug[i][j] = a[i][j];
        aug[i][n + i] = {1.0, 0.0};
    }
    for (std::size_t column = 0; column < n; ++column) {
        std::size_t pivot = column;
        for (std::size_t row = column + 1; row < n; ++row)
            if (std::abs(aug[row][column]) > std::abs(aug[pivot][column]))
                pivot = row;
        if (std::abs(aug[pivot][column]) < 1e-13) return {};
        std::swap(aug[pivot], aug[column]);
        const auto divisor = aug[column][column];
        for (auto& entry : aug[column]) entry /= divisor;
        for (std::size_t row = 0; row < n; ++row) {
            if (row == column) continue;
            const auto factor = aug[row][column];
            for (std::size_t j = 0; j < 2 * n; ++j)
                aug[row][j] -= factor * aug[column][j];
        }
    }
    std::vector<std::vector<std::complex<double>>> inverse(
        n, std::vector<std::complex<double>>(n));
    for (std::size_t i = 0; i < n; ++i)
        for (std::size_t j = 0; j < n; ++j) inverse[i][j] = aug[i][n + j];
    return inverse;
}

inline GeneralizedMultinacciGrowthProfile
derive_generalized_multinacci_growth_profile(std::size_t dimension,
                                             std::size_t multiplicity) {
    if (dimension < 2 || multiplicity < 1)
        throw std::invalid_argument("growth profile requires d>=2 and m>=1");
    const auto polynomial =
        ravel::generalized_multinacci_polynomial(dimension, multiplicity);
    auto roots = find_roots_durand_kerner(polynomial);
    double dominant = 0.0;
    for (const auto& root : roots) dominant = std::max(dominant, std::abs(root));
    double secondary = 0.0;
    bool skipped_dominant = false;
    for (const auto& root : roots) {
        // The Perron root is simple in this nonnegative primitive family.
        // Skip exactly one root, rather than relying on a tolerance between
        // close secondary roots.
        if (!skipped_dominant && std::abs(root) > 0.5 * dominant) {
            skipped_dominant = true;
            continue;
        }
        secondary = std::max(secondary, std::abs(root));
    }
    const double gap_factor = 1.0 / (1.0 - secondary);
    std::vector<double> embedding_bounds;
    embedding_bounds.reserve(roots.size());
    embedding_bounds.push_back(
        multiplicity * dominant / (dominant - 1.0));
    for (const auto& root : roots) {
        if (std::abs(root) > 0.5 * dominant) continue;
        const double modulus = std::abs(root);
        embedding_bounds.push_back(
            multiplicity * modulus / std::max(1e-12, 1.0 - modulus));
    }
    std::vector<std::vector<std::complex<double>>> vandermonde(
        roots.size(), std::vector<std::complex<double>>(roots.size()));
    for (std::size_t i = 0; i < roots.size(); ++i) {
        std::complex<double> power{1.0, 0.0};
        for (std::size_t j = 0; j < roots.size(); ++j) {
            vandermonde[i][j] = power;
            power *= roots[i];
        }
    }
    const auto inverse_vandermonde = invert_complex_matrix(vandermonde);
    double maximum_coefficient_bound = 0.0;
    double coefficient_box_log10 = 0.0;
    if (inverse_vandermonde.empty() || embedding_bounds.size() != roots.size()) {
        coefficient_box_log10 = std::numeric_limits<double>::infinity();
    } else {
        for (std::size_t coefficient = 0; coefficient < roots.size(); ++coefficient) {
            double bound = 0.0;
            for (std::size_t embedding = 0; embedding < roots.size(); ++embedding)
                bound += std::abs(inverse_vandermonde[coefficient][embedding]) *
                         embedding_bounds[embedding];
            bound *= 1.10;  // numerical safety margin; this is a profile proxy
            maximum_coefficient_bound = std::max(maximum_coefficient_bound, bound);
            coefficient_box_log10 += std::log10(2.0 * bound + 1.0);
        }
    }
    return GeneralizedMultinacciGrowthProfile{
        dimension,
        multiplicity,
        dimension,
        (dimension - 1) * multiplicity + 1,
        ravel::generalized_multinacci_beta(dimension, multiplicity),
        secondary,
        gap_factor,
        static_cast<double>(multiplicity) * secondary * gap_factor,
        maximum_coefficient_bound,
        coefficient_box_log10,
    };
}

}  // namespace adelic
