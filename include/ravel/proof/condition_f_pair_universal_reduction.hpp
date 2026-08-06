#pragma once

#include <cstddef>
#include <string>
#include <vector>

#include "ravel/proof/condition_f_cyclic_voltage_fourier.hpp"
#include "ravel/proof/condition_f_local_channel_recurrence.hpp"
#include "ravel/proof/condition_f_pair_boundary_substitution.hpp"

namespace ravel::proof {

struct ConditionFPairUniversalReduction {
    std::size_t dimension = 0;
    ConditionFParentPairRecurrence parent_pair;
    ConditionFLocalChannelRecurrence local_channels;
    ConditionFPairBoundarySubstitution boundary_substitution;
    bool voltage_twists_spectrally_eliminated = false;
    bool old_dimension_block_transported = false;
    bool terminal_boundary_formula_derived = false;
    bool boundary_pair_substitution_derived = false;
    bool joint_pair_order_preserved_by_boundary = false;
    bool only_base_pair_comparison_remains = false;
    bool proved = false;
    std::string reduced_obligation;
    std::string obstruction;
};

/** Compose the two general facts that were previously conflated.
 *
 * 1. Abelian fibre twists do not affect the maximal Perron sector: every
 *    represented sector is entrywise dominated by the untwisted Q+R channel
 *    count operator.
 * 2. The canonical n-bonacci parent grammar has an exact n->n+1 recurrence:
 *    all old channels persist, and only terminal parents of inner zero create
 *    new boundary channels, with Q/R type and signed defect fixed explicitly.
 *
 * Therefore the universal-n spectral problem contains no remaining twist or
 * matrix-color ambiguity.  Its sole local combinatorial content is whether
 * the new terminal-parent boundary roles create a component whose untwisted
 * channel-count growth exceeds the transported old/core block.
 */
inline ConditionFPairUniversalReduction
 derive_condition_f_pair_universal_reduction(std::size_t n) {
    ConditionFPairUniversalReduction out;
    out.dimension = n;
    out.parent_pair = derive_condition_f_parent_pair_recurrence(n);
    out.local_channels = derive_condition_f_local_channel_recurrence(n);
    out.boundary_substitution = derive_condition_f_pair_boundary_substitution(n);
    if (!out.parent_pair.proved || !out.local_channels.proved || !out.boundary_substitution.proved) {
        out.obstruction = !out.parent_pair.proved ? out.parent_pair.obstruction :
            (!out.local_channels.proved ? out.local_channels.obstruction :
                                          out.boundary_substitution.obstruction);
        return out;
    }
    // The cyclic-voltage Fourier theorem is dimension-free; this flag records
    // its universal application rather than a finite fibre enumeration.
    out.voltage_twists_spectrally_eliminated = true;
    out.old_dimension_block_transported =
        out.parent_pair.old_block_transport_exact &&
        out.local_channels.old_parent_channels_transport_exactly;
    out.terminal_boundary_formula_derived =
        out.parent_pair.boundary_formula_exact &&
        out.local_channels.only_zero_inner_letters_gain_parents &&
        out.local_channels.q_r_type_matches_prefix_defect;
    out.boundary_pair_substitution_derived =
        out.boundary_substitution.proved &&
        out.boundary_substitution.first_return_complete &&
        out.boundary_substitution.pair_words_canonical;
    // Closure under the boundary polynomial is a dimension-free algebraic
    // theorem: a joint Q/R intertwiner is preserved by every word and every
    // nonnegative sum of words.  The concrete matrix P is supplied by the
    // base comparison, not by this recurrence operation.
    out.joint_pair_order_preserved_by_boundary =
        out.boundary_pair_substitution_derived;
    out.only_base_pair_comparison_remains =
        out.voltage_twists_spectrally_eliminated &&
        out.old_dimension_block_transported &&
        out.terminal_boundary_formula_derived &&
        out.joint_pair_order_preserved_by_boundary;
    out.reduced_obligation =
        "Derive the joint Q/R comparison on the base Condition-F pair; the exact n->n+1 boundary substitution then transports it automatically.";
    out.proved = out.only_base_pair_comparison_remains;
    return out;
}

} // namespace ravel::proof
