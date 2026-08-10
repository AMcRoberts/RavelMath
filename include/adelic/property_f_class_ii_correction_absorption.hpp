// Structural bridge from the finite Akiyama deviation window to the
// Property-(F) collar.
//
// This certificate deliberately keeps the two levels separate.  The phase
// identities explain why a nonzero deviation leaves the affine strip; the
// realized graph certificate checks that the only high invalid recurrence is
// the distinguished zero seed, while the collar grammar checks that a
// deviation which enters the collar cannot re-enter the tail.  Thus the
// remaining uniform theorem is visible as one named seam rather than being
// hidden inside a large graph search.
#pragma once

#include <array>
#include <cstddef>
#include <string>

#include "adelic/property_f_class_ii_akiyama_window_census.hpp"
#include "adelic/property_f_class_ii_collar_grammar.hpp"
#include "adelic/property_f_class_ii_phase_absorption.hpp"
#include "adelic/property_f_class_ii_phase_strip.hpp"
#include "adelic/property_f_class_ii_rank_spine.hpp"

namespace adelic {

struct PropertyFClassIICorrectionAbsorptionCertificate {
    static constexpr std::size_t collar_height = 5;

    std::size_t parameter_a = 0;
    std::size_t tail_sources = 0;
    std::size_t bounded_candidates = 0;
    std::size_t bounded_collar_edges = 0;
    // Raw bounded high edges, including the forced affine continuation.
    std::size_t bounded_high_edges = 0;
    std::size_t bounded_internal_high_edges = 0;
    std::size_t bounded_seed_internal_edges = 0;
    std::size_t malformed_edges = 0;
    std::size_t nonordinary_edges = 0;
    std::size_t invalid_phase_sources = 0;
    std::size_t phase_deviation_mismatches = 0;
    std::size_t unexpected_internal_high_edges = 0;
    std::size_t unexpected_external_high_edges = 0;
    std::size_t seed_node = 0;
    bool parameter_domain = false;
    bool rank_valid = false;
    bool spine_valid = false;
    bool window_valid = false;
    bool phase_absorption_valid = false;
    bool collar_absorption_valid = false;
    bool phase_deviation_valid = false;
    bool high_recurrence_is_seed_only = false;
    bool bounded_deviations_absorbed = false;
    bool valid = false;
};

inline PropertyFClassIICorrectionAbsorptionCertificate
derive_property_f_class_ii_correction_absorption(
        const PropertyFGraph& graph, std::size_t a) {
    PropertyFClassIICorrectionAbsorptionCertificate out;
    out.parameter_a = a;
    out.parameter_domain = a >= 7 &&
        a <= property_f_class_ii_spine_max_safe_a;
    if (!out.parameter_domain) return out;

    const auto rank = derive_property_f_escape_rank(graph);
    out.rank_valid = rank.valid;
    if (!rank.valid) return out;
    const auto spine = derive_property_f_class_ii_rank_spine(graph, a);
    out.spine_valid = spine.valid;
    if (!spine.valid) return out;

    const auto window = derive_property_f_class_ii_akiyama_window_census(
        graph, a);
    out.window_valid = window.valid;
    out.bounded_candidates = window.bounded_candidates;
    out.bounded_collar_edges = window.bounded_collar_edges;
    // The window census calls this the high count before separating its one
    // allowed internal recurrence; keep the raw value for diagnostics.
    out.bounded_high_edges = window.bounded_high_edges;
    out.bounded_internal_high_edges = window.bounded_internal_high_edges;
    out.malformed_edges = window.malformed_edges;
    out.nonordinary_edges = window.nonordinary_edges;

    const auto phase_absorption =
        derive_property_f_class_ii_phase_absorption(graph, a);
    out.phase_absorption_valid = phase_absorption.valid;
    const auto collar = derive_property_f_class_ii_collar_grammar(graph, a);
    out.collar_absorption_valid = collar.valid && collar.no_tail_reentry &&
        collar.maximum_target_height <=
            PropertyFClassIICollarGrammar::collar_height;

    // The zero state with letter 0 is the sole high phase-invalid seed in the
    // current grammar.  Locate it from the exact graph rather than relying
    // on construction order.
    out.seed_node = graph.nodes.size();
    for (std::size_t node = 0; node < graph.nodes.size(); ++node) {
        if (!graph.nodes[node].zero || graph.nodes[node].letter != 0)
            continue;
        if (rank.node_height[node] == rank.maximum_height) {
            out.seed_node = node;
            break;
        }
    }

    out.phase_deviation_valid = true;
    for (const auto source : spine.node_ids) {
        if (source >= graph.nodes.size() || rank.node_height[source] <=
            PropertyFClassIICorrectionAbsorptionCertificate::collar_height)
            continue;
        ++out.tail_sources;
        std::array<long long, 3> source_state{};
        if (!property_f_class_ii_node_integer_state(
                graph.nodes[source], source_state)) {
            ++out.malformed_edges;
            continue;
        }
        const bool seed = source == out.seed_node;
        const auto source_phase = property_f_class_ii_phase_pair_valid(
            source_state, static_cast<long long>(a));
        if (!seed && !source_phase) ++out.invalid_phase_sources;
        const auto forced = property_f_class_ii_phase_forced_digit(
            source_state, static_cast<long long>(a));
        for (std::size_t edge = 0;
             edge < graph.nodes[source].successors.size(); ++edge) {
            const auto raw_target = graph.nodes[source].successors[edge];
            if (raw_target < 0 || static_cast<std::size_t>(raw_target) >=
                graph.nodes.size() || edge >=
                graph.nodes[source].edge_digit_coefficients.size()) {
                ++out.malformed_edges;
                continue;
            }
            long long digit = 0;
            if (!property_f_class_ii_decode_integer_digit(
                    graph.nodes[source].edge_digit_coefficients[edge], digit)) {
                ++out.nonordinary_edges;
                continue;
            }
            const auto target = static_cast<std::size_t>(raw_target);
            std::array<long long, 3> target_state{};
            if (!property_f_class_ii_node_integer_state(
                    graph.nodes[target], target_state)) {
                ++out.malformed_edges;
                continue;
            }
            if (!property_f_class_ii_akiyama_eta_bound(
                    target_state[0] + target_state[2]))
                continue;
            const auto deviation = digit - forced;
            const bool target_phase = property_f_class_ii_phase_pair_valid(
                target_state, static_cast<long long>(a));
            // On a phase-valid source, d=0 is equivalent to remaining in the
            // two-state strip.  The seed is the one finite boundary exception.
            if (!seed && source_phase && target_phase != (deviation == 0))
                ++out.phase_deviation_mismatches;

            if (rank.node_height[target] <=
                PropertyFClassIICorrectionAbsorptionCertificate::collar_height)
                continue;
            // The zero-deviation edge is the affine spine continuation.  It
            // is intentionally not a correction case; only nonzero bounded
            // deviations can threaten to create a second high branch.
            if (deviation == 0)
                continue;
            const bool internal = graph.scc_labels[source] ==
                graph.scc_labels[target];
            if (internal) {
                if (source == out.seed_node && target == out.seed_node)
                    ++out.bounded_seed_internal_edges;
                else
                    ++out.unexpected_internal_high_edges;
            } else {
                ++out.unexpected_external_high_edges;
            }
        }
    }

    out.phase_deviation_valid = out.invalid_phase_sources == 0 &&
        out.phase_deviation_mismatches == 0;
    out.high_recurrence_is_seed_only =
        out.unexpected_internal_high_edges == 0 &&
        out.unexpected_external_high_edges == 0;
    out.bounded_deviations_absorbed =
        out.phase_deviation_valid && out.high_recurrence_is_seed_only &&
        out.bounded_seed_internal_edges <= 1 &&
        out.collar_absorption_valid;
    out.valid = out.parameter_domain && out.rank_valid && out.spine_valid &&
        out.window_valid && out.phase_absorption_valid &&
        out.collar_absorption_valid && out.invalid_phase_sources == 0 &&
        out.phase_deviation_mismatches == 0 && out.malformed_edges == 0 &&
        out.high_recurrence_is_seed_only && out.bounded_deviations_absorbed;
    return out;
}

}  // namespace adelic
