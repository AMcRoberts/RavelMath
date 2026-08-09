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
};

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
    return GeneralizedMultinacciGrowthProfile{
        dimension,
        multiplicity,
        dimension,
        (dimension - 1) * multiplicity + 1,
        ravel::generalized_multinacci_beta(dimension, multiplicity),
        secondary,
        gap_factor,
        static_cast<double>(multiplicity) * secondary * gap_factor,
    };
}

}  // namespace adelic
