#pragma once

#include <cstddef>
#include <map>
#include <string>
#include <vector>

#include "ravel/proof/generalized_multinacci_prefix_phase.hpp"
#include "ravel/proof/generalized_multinacci_signed_renewal_twist.hpp"
#include "math/proof_reflection.hpp"

namespace ravel::proof {

/** Symbolic general-m transport law for the unimodular generalized-multinacci family.
 *
 * For sigma(i)=0^m(i+1), parent-prefix cuts are p,q in {0,...,m}.  The
 * signed cut defect delta=q-p factors canonically into
 *
 *   delta=0 : Q,
 *   delta!=0: R^|delta| with side voltage sign(delta).
 *
 * Hence the universal unsigned scheduler is
 *
 *   S_m(Q,R)=(m+1)Q + sum_{d=1}^m 2(m+1-d) R^d.
 *
 * Boundary admissibility may delete channels, but cannot create a different
 * primitive generator or a roof longer than m.
 */
struct GeneralizedMultinacciGeneralMProof {
    std::size_t multiplicity = 0;
    std::size_t prefix_positions = 0;
    std::size_t ordered_channels = 0;
    std::map<std::size_t,std::size_t> unsigned_scheduler_coefficients;
    std::vector<std::vector<PrefixPrimitiveGenerator>> macro_words;
    bool exact_channel_count = false;
    bool exact_triangular_scheduler = false;
    bool every_macro_is_q_or_r_power = false;
    bool side_voltage_is_sign_of_defect = false;
    bool roof_is_uniformly_bounded_by_m = false;
    bool specializes_to_nbonacci = false;
    bool proved = false;
    std::string obstruction;
};

inline GeneralizedMultinacciGeneralMProof
 derive_generalized_multinacci_general_m(std::size_t multiplicity) {
    GeneralizedMultinacciGeneralMProof out;
    out.multiplicity = multiplicity;
    if (multiplicity == 0) {
        out.obstruction = "general-m theorem requires m>=1";
        return out;
    }
    const auto phase = derive_generalized_multinacci_prefix_phase(multiplicity);
    if (!phase.proved) {
        out.obstruction = "general-m theorem requires the exact prefix-phase certificate";
        return out;
    }
    out.prefix_positions = phase.prefix_state_count;
    out.ordered_channels = phase.channel_count;
    out.macro_words = phase.macro_as_primitive_word;
    for (const auto& c : phase.channels)
        ++out.unsigned_scheduler_coefficients[c.macro_generator];

    out.exact_channel_count =
        out.prefix_positions == multiplicity + 1 &&
        out.ordered_channels == (multiplicity + 1) * (multiplicity + 1);
    out.exact_triangular_scheduler =
        out.unsigned_scheduler_coefficients[0] == multiplicity + 1;
    for (std::size_t d=1; d<=multiplicity; ++d)
        out.exact_triangular_scheduler &=
            out.unsigned_scheduler_coefficients[d] == 2 * (multiplicity + 1 - d);

    out.every_macro_is_q_or_r_power =
        out.macro_words.size() == multiplicity + 1 &&
        out.macro_words[0] == std::vector<PrefixPrimitiveGenerator>{
            PrefixPrimitiveGenerator::balanced_q};
    for (std::size_t d=1; d<=multiplicity; ++d)
        out.every_macro_is_q_or_r_power &=
            out.macro_words[d] == std::vector<PrefixPrimitiveGenerator>(
                d, PrefixPrimitiveGenerator::residual_r);

    out.side_voltage_is_sign_of_defect = phase.sign_retained_as_twist;
    out.roof_is_uniformly_bounded_by_m = true;
    for (const auto& c : phase.channels)
        out.roof_is_uniformly_bounded_by_m &= c.macro_generator <= multiplicity;
    out.specializes_to_nbonacci = phase.nbonacci_specialization_exact;
    out.proved = out.exact_channel_count &&
                 out.exact_triangular_scheduler &&
                 out.every_macro_is_q_or_r_power &&
                 out.side_voltage_is_sign_of_defect &&
                 out.roof_is_uniformly_bounded_by_m &&
                 out.specializes_to_nbonacci;
    if (!out.proved) out.obstruction = "general-m symbolic scheduler certificate failed";
    return out;
}

template <std::size_t d>
bool general_m_concrete_twist_refines_symbolic_scheduler(
    const GeneralizedMultinacciSignedRenewalTwistProof& concrete,
    const GeneralizedMultinacciGeneralMProof& symbolic) {
    if (!concrete.proved || !symbolic.proved) return false;
    if (concrete.dimension != d || concrete.multiplicity != symbolic.multiplicity)
        return false;
    if (concrete.maximum_return_time > symbolic.multiplicity) return false;
    for (const auto& edge : concrete.renewal_edges) {
        if (edge.return_time > symbolic.multiplicity) return false;
        if (edge.signed_voltage == 0) {
            if (edge.primitive_word != std::vector<std::size_t>{0}) return false;
        } else {
            if (edge.primitive_word !=
                std::vector<std::size_t>(edge.return_time,1)) return false;
        }
    }
    return true;
}

// Stages a `GeneralizedMultinacciGeneralMReflectionCertificate` for one
// multiplicity m -- gates on `proof.proved`, which independently rechecks
// (via derive_generalized_multinacci_general_m above) that the observed
// scheduler coefficients exactly match the closed form m+1 / 2(m+1-d)
// before this function is ever reached.
inline void stage_generalized_multinacci_general_m(
        const GeneralizedMultinacciGeneralMProof& proof,
        const std::string& description) {
    if (!proof.proved) return;
    if (!mathlib::reflection::enabled()) return;
    mathlib::reflection::GeneralizedMultinacciGeneralMReflectionCertificate node;
    node.multiplicity = static_cast<long long>(proof.multiplicity);
    node.description = description;
    mathlib::reflection::record(mathlib::reflection::NodeKind::LemmaApplication, node);
}

} // namespace ravel::proof
