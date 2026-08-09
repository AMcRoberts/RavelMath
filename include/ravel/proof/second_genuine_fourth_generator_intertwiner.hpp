#pragma once

#include "ravel/proof/contact_boundary_generator_intertwiner.hpp"

namespace ravel::proof {

// The second known interior-zero witness: digits (2,0,1,0,2), hence
// 0->001, 1->2, 2->03, 3->4, 4->00.  The generic certificate keeps the
// boundary/role construction identical to the first witness while allowing
// the five-letter canonical substitution and its 25 role states.
inline ContactBoundaryGeneratorIntertwinerCertificate<5>
derive_second_genuine_fourth_generator_intertwiner() {
    SubstitutionRule rule(std::vector<std::vector<std::int8_t>>{
        {0, 0, 1}, {2}, {0, 3}, {4}, {0, 0}});
    constexpr double beta = 2.269530842081144;
    constexpr double beta_conjugate_modulus = 0.9387435225216725;
    ContactBoundaryLimits limits;
    limits.closure_cap = 100000;
    limits.corona_cap = 600000;
    limits.max_corona_rounds = 14;
    limits.retain_boundary_matrix = false;
    return derive_contact_boundary_generator_intertwiner<5>(
        rule, beta, beta_conjugate_modulus, 2, limits);
}

}  // namespace ravel::proof
