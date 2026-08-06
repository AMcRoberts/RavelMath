#pragma once

#include <cstddef>
#include <cstdlib>
#include <map>
#include <string>
#include <utility>
#include <vector>

#include "ravel/proof/generalized_multinacci_boundary_word_lift.hpp"
#include "ravel/proof/positive_word_graph_expansion.hpp"

namespace ravel::proof {

/** The generalized-multinacci prefix deformation is a signed renewal twist.
 *
 * A macro boundary channel has signed prefix defect delta.  Its positive
 * transport word is Q when delta=0 and R^|delta| otherwise.  The magnitude is
 * therefore a roof/return-time cocycle, while sign(delta) is an independent
 * Z/2 side voltage.  Subdivision of every R^d macro channel into d primitive
 * R edges produces a genuine Q/R graph whose first-return compression is the
 * original contact-boundary graph.
 */
struct SignedRenewalMacroEdge {
    std::size_t source = 0;
    std::size_t target = 0;
    long long signed_voltage = 0; // -1,0,+1
    std::size_t return_time = 1;
    std::vector<std::size_t> primitive_word; // 0=Q, 1=R
};

struct GeneralizedMultinacciSignedRenewalTwistProof {
    std::size_t dimension = 0;
    std::size_t multiplicity = 0;
    std::size_t base_states = 0;
    std::size_t macro_edges = 0;
    std::size_t expanded_states = 0;
    std::size_t positive_voltage_edges = 0;
    std::size_t negative_voltage_edges = 0;
    std::size_t zero_voltage_edges = 0;
    std::size_t maximum_return_time = 0;
    std::map<std::size_t,std::size_t> return_time_histogram;
    std::vector<SignedRenewalMacroEdge> renewal_edges;
    PositiveWordGraphExpansionProof expansion;
    bool defect_factorization_exact = false;
    bool roof_bounded_by_prefix_multiplicity = false;
    bool sign_is_independent_z2_voltage = false;
    bool primitive_alphabet_is_exactly_qr = false;
    bool first_return_recovers_boundary = false;
    bool no_new_primitive_generator = false;
    bool proved = false;
    std::string obstruction;
};

template <std::size_t d>
GeneralizedMultinacciSignedRenewalTwistProof
 derive_generalized_multinacci_signed_renewal_twist(
    const MultinacciBoundaryWordLiftProof<d>& lift) {
    GeneralizedMultinacciSignedRenewalTwistProof out;
    out.dimension = d;
    out.multiplicity = lift.multiplicity;
    out.base_states = lift.projected_adjacency.size();
    if (!lift.proved) {
        out.obstruction = "signed renewal twist requires a proved boundary word lift";
        return out;
    }

    out.defect_factorization_exact = true;
    out.sign_is_independent_z2_voltage = true;
    out.primitive_alphabet_is_exactly_qr = true;
    std::vector<PositiveWordGraphEdge> macro;
    macro.reserve(lift.edges.size());
    out.renewal_edges.reserve(lift.edges.size());

    for (const auto& e : lift.edges) {
        SignedRenewalMacroEdge r;
        r.source = e.source;
        r.target = e.target;
        r.signed_voltage = (e.signed_prefix_defect > 0) -
                           (e.signed_prefix_defect < 0);
        const auto magnitude = static_cast<std::size_t>(
            std::llabs(e.signed_prefix_defect));
        r.return_time = magnitude == 0 ? 1 : magnitude;
        for (auto g : e.word)
            r.primitive_word.push_back(
                g == PrefixPrimitiveGenerator::balanced_q ? 0u : 1u);

        if (magnitude == 0) {
            ++out.zero_voltage_edges;
            out.defect_factorization_exact &=
                r.primitive_word == std::vector<std::size_t>{0};
            out.sign_is_independent_z2_voltage &= r.signed_voltage == 0;
        } else {
            if (r.signed_voltage > 0) ++out.positive_voltage_edges;
            else ++out.negative_voltage_edges;
            out.defect_factorization_exact &=
                r.primitive_word == std::vector<std::size_t>(magnitude,1);
            out.sign_is_independent_z2_voltage &=
                r.signed_voltage == 1 || r.signed_voltage == -1;
        }
        out.defect_factorization_exact &= r.return_time == r.primitive_word.size();
        for (auto generator : r.primitive_word)
            out.primitive_alphabet_is_exactly_qr &= generator <= 1;
        out.maximum_return_time = std::max(out.maximum_return_time,r.return_time);
        ++out.return_time_histogram[r.return_time];
        macro.push_back({r.source,r.target,r.primitive_word});
        out.renewal_edges.push_back(std::move(r));
    }
    out.macro_edges = out.renewal_edges.size();
    out.roof_bounded_by_prefix_multiplicity =
        out.maximum_return_time <= std::max<std::size_t>(1,out.multiplicity);

    out.expansion = derive_positive_word_graph_expansion(
        out.base_states,2,macro);
    out.expanded_states = out.expansion.expanded_node_count;
    out.first_return_recovers_boundary = out.expansion.proved &&
        out.expansion.reconstructed_macro_adjacency == lift.projected_adjacency;
    out.no_new_primitive_generator = out.primitive_alphabet_is_exactly_qr &&
        out.expansion.generator_adjacency.size() == 2;

    out.proved = out.defect_factorization_exact &&
                 out.roof_bounded_by_prefix_multiplicity &&
                 out.sign_is_independent_z2_voltage &&
                 out.primitive_alphabet_is_exactly_qr &&
                 out.first_return_recovers_boundary &&
                 out.no_new_primitive_generator;
    if (!out.proved && out.obstruction.empty())
        out.obstruction = "signed renewal/voltage factorization failed";
    return out;
}

} // namespace ravel::proof
