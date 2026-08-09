#pragma once

#include <array>
#include "ravel/proof/contact_boundary_generator_intertwiner.hpp"

// Concrete contact-boundary version of first_genuine_fourth_generator.hpp's
// abstract-role finding for x^3-2x^2-2 (substitution 0->001, 1->2, 2->00,
// beta~2.35930, non-unit). This is the check that caught the quartic
// family's spurious fourth colour before it was believed: does the extra
// generator survive as a genuine edge in the REAL contact-boundary graph,
// under the same displacement-forgetting parent-role intertwiner used for
// plastic and supergolden, or was it an artifact of the abstract
// universal-role catalogue alone?
//
// Five generators this time, not three: neutral (0), +-1, +-2.

namespace ravel::proof {

struct FirstGenuineFourthGeneratorIntertwinerCertificate {
    std::size_t generator_count = 5;
    std::size_t boundary_states = 0;
    std::size_t universal_roles = 9;  // 3x3 role states
    long long boundary_edges = 0;
    long long universal_edges = 0;
    NonnegativeMatrix intertwiner;
    // index 0..4 maps to defect -2,-1,0,+1,+2
    std::array<NonnegativeMatrix,5> boundary_generators;
    std::array<NonnegativeMatrix,5> universal_generators;
    bool parent_catalogue_complete = false;
    bool every_boundary_edge_has_universal_witness = false;
    bool base_role_projection_exact = false;
    std::array<bool,5> generator_intertwines{};
    bool simultaneous_five_generator_intertwiner = false;
    bool boundary_realizes_defect_plus2 = false;   // does the CONCRETE graph even contain a +2 edge?
    bool boundary_realizes_defect_minus2 = false;
    bool finite_positive_grammar_ready = false;
    bool proved = false;
    std::string obstruction;
};

inline FirstGenuineFourthGeneratorIntertwinerCertificate
 derive_first_genuine_fourth_generator_intertwiner() {
    FirstGenuineFourthGeneratorIntertwinerCertificate out;

    // 0->001, 1->2, 2->00
    SubstitutionRule rule(std::vector<std::vector<std::int8_t>>{{0,0,1},{2},{0,0}});
    constexpr double beta = 2.3593040859717767;
    constexpr double b2 = 0.92071037690446744;
    ContactBoundaryLimits lim;
    lim.closure_cap = 40000;
    lim.corona_cap = 200000;
    lim.max_corona_rounds = 14;
    // This certificate consumes only the exact boundary graph.  Retaining
    // the dense boundary adjacency/spectral matrix would add O(|B|^2)
    // storage without contributing to the intertwiner or grammar checks.
    // Keep the graph-only path enabled so larger fourth-generator candidates
    // do not silently inherit the dense-matrix memory cost.
    lim.retain_boundary_matrix = false;
    const auto generic = derive_contact_boundary_generator_intertwiner<3>(
        rule, beta, b2, 2, lim);
    out.boundary_states = generic.boundary_states;
    out.boundary_edges = generic.boundary_edges;
    out.universal_edges = generic.universal_edges;
    out.intertwiner = generic.intertwiner;
    for (std::size_t g = 0; g < 5; ++g) {
        out.boundary_generators[g] = generic.boundary_generators[g];
        out.universal_generators[g] = generic.universal_generators[g];
        out.generator_intertwines[g] = generic.generator_intertwines[g];
    }
    out.parent_catalogue_complete = generic.parent_catalogue_complete;
    out.every_boundary_edge_has_universal_witness = generic.every_boundary_edge_has_universal_witness;
    out.base_role_projection_exact = generic.base_role_projection_exact;
    out.simultaneous_five_generator_intertwiner = generic.simultaneous_intertwiner;
    out.boundary_realizes_defect_minus2 = generic.boundary_realizes_defect[0];
    out.boundary_realizes_defect_plus2 = generic.boundary_realizes_defect[4];
    out.finite_positive_grammar_ready = generic.finite_positive_grammar_ready;
    out.proved = generic.proved;
    out.obstruction = generic.obstruction;
    if (!out.proved && out.obstruction.empty())
        out.obstruction = "x^3-2x^2-2 five-generator comparison invariant failed";
    return out;
}

} // namespace ravel::proof
