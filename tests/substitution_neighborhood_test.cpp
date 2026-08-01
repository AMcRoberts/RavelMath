#include <array>
#include <cstddef>
#include <cstdio>
#include <set>

#include "ravel/class_ii_neighbor_family.hpp"
#include "ravel/class_ii_neighbor2_pruning.hpp"
#include "ravel/substitution_neighborhood.hpp"

using namespace ravel;

namespace {

FiniteSubstitution class_ii(std::size_t a) {
    FiniteSubstitution result(3);
    result[0].assign(a, 0);
    result[0].push_back(1);
    result[0].push_back(2);
    result[1].assign(a, 0);
    result[1].push_back(2);
    result[2] = {0};
    return result;
}

}  // namespace

int main() {
    bool ok = true;
    const auto contact_envelope =
        class_ii_contact_backward_envelope_certificate();
    ok = ok && contact_envelope.exact();
    std::printf(
        "  center contact: symbolic backward envelope"
        " cases=%zu unresolved=%zu restricted=%d exact=%d\n",
        contact_envelope.bounded_window_cases,
        contact_envelope.unresolved_window_cases,
        contact_envelope.restricted_nodes_exact ? 1 : 0,
        contact_envelope.exact() ? 1 : 0);
    for (long long a = 7; a <= 128; ++a) {
        std::array<std::size_t, 5> phase_counts{};
        for (long long round = 1; round <= a + 1; ++round) {
            const auto phase =
                class_ii_neighbor2_global_round_phase(a, round);
            ++phase_counts[static_cast<std::size_t>(phase)];
        }
        ok = ok
            && phase_counts[0] == 4
            && phase_counts[1] == static_cast<std::size_t>(a - 6)
            && phase_counts[2] == 1
            && phase_counts[3] == 1
            && phase_counts[4] == 1;
    }
    std::size_t named_nonbase_catalogues = 0;
    for (long long a = 7; a <= 32; ++a) {
        for (long long round = 5; round <= a + 1; ++round) {
            const auto before =
                class_ii_neighbor2_nonbase_pre_red_catalogue(a, round);
            const auto ranks =
                class_ii_neighbor2_nonbase_pruning_ranks(a, round);
            const auto after =
                class_ii_neighbor2_nonbase_post_red_catalogue(a, round);
            std::set<SNode<3>> removed;
            for (const auto& rank : ranks) {
                for (const auto& node : rank) {
                    ok = ok && before.count(node) == 1;
                    ok = ok && removed.insert(node).second;
                }
            }
            auto reconstructed = after;
            reconstructed.insert(removed.begin(), removed.end());
            ok = ok
                && !before.empty()
                && reconstructed == before;
            ++named_nonbase_catalogues;
        }
    }
    bool base_catalogue_rejected = false;
    try {
        (void)class_ii_neighbor2_nonbase_pre_red_catalogue(7, 4);
    } catch (const std::domain_error&) {
        base_catalogue_rejected = true;
    }
    ok = ok && base_catalogue_rejected;
    std::printf(
        "  neighbor 2: global round partition"
        " base=4 stable=a-6 endpoint=1+1+1 exact through a=128"
        " named-nonbase=%zu base-rejected=%d\n",
        named_nonbase_catalogues,
        base_catalogue_rejected ? 1 : 0);

    // ClassIINeighbor2GlobalInductionCertificate: reports the base
    // premise ledger for rounds 1..4 and, for round >= 5, defers
    // entirely to the already-checked nonbase dispatcher above (no
    // recomputation). As of 2026-07-30 rounds 1-3 are missing reverse
    // inclusion and round 4 is missing the bridge to the stable
    // theorem; round 1 is always the first missing premise. This test
    // intentionally fails loudly if a future session marks a base
    // round `closed` without also updating this assertion.
    bool induction_ledger_ok = true;
    {
        const auto premises1 = class_ii_neighbor2_base_round_premises(1);
        const auto premises2 = class_ii_neighbor2_base_round_premises(2);
        const auto premises3 = class_ii_neighbor2_base_round_premises(3);
        const auto premises4 = class_ii_neighbor2_base_round_premises(4);
        induction_ledger_ok = induction_ledger_ok
            && premises1.center_contact_base_closed
            && premises1.positive_witnesses_and_bridges_closed
            && premises1.window_validity_closed
            && premises1.status
                == ClassIINeighbor2InductionStatus::open_reverse_inclusion
            && premises2.status
                == ClassIINeighbor2InductionStatus::open_reverse_inclusion
            && premises3.status
                == ClassIINeighbor2InductionStatus::open_reverse_inclusion
            && premises4.status
                == ClassIINeighbor2InductionStatus::open_bridge;
        bool premises_reject_out_of_range = false;
        try {
            (void)class_ii_neighbor2_base_round_premises(5);
        } catch (const std::domain_error&) {
            premises_reject_out_of_range = true;
        }
        induction_ledger_ok =
            induction_ledger_ok && premises_reject_out_of_range;
    }
    for (long long a = 7; a <= 32; ++a) {
        for (long long round = 1; round <= 4; ++round) {
            const auto cert =
                class_ii_neighbor2_global_induction_certificate(a, round);
            induction_ledger_ok = induction_ledger_ok
                && cert.is_base_round
                && cert.a == a && cert.round == round
                && cert.pre_red_size == 0 && cert.post_red_size == 0;
        }
        for (long long round = 5; round <= a + 1; ++round) {
            const auto cert =
                class_ii_neighbor2_global_induction_certificate(a, round);
            induction_ledger_ok = induction_ledger_ok
                && !cert.is_base_round
                && cert.status == ClassIINeighbor2InductionStatus::closed
                && cert.pre_red_size
                    == class_ii_neighbor2_nonbase_pre_red_catalogue(
                           a, round).size()
                && cert.post_red_size
                    == class_ii_neighbor2_nonbase_post_red_catalogue(
                           a, round).size();
        }
        const auto missing = class_ii_neighbor2_first_missing_premise(a);
        induction_ledger_ok = induction_ledger_ok
            && missing.round == 1
            && missing.status
                == ClassIINeighbor2InductionStatus::open_reverse_inclusion;
    }
    ok = ok && induction_ledger_ok;
    std::printf(
        "  neighbor 2: global induction certificate"
        " base-ledger=1,2,3=open_reverse_inclusion 4=open_bridge"
        " first-missing=round1 through a=32 ok=%d\n",
        induction_ledger_ok ? 1 : 0);
    for (std::size_t a = 1; a <= 128; ++a) {
        const auto center = class_ii(a);
        const auto neighbors = adjacent_swap_neighbors(center);

        // 0^a12 has two unequal adjacencies and 0^a2 has one.
        ok = ok && neighbors.size() == 3;
        for (const auto& neighbor : neighbors) {
            ok = ok
                && same_incidence_matrix(center, neighbor.substitution)
                && neighbor.image < center.size()
                && neighbor.position + 1 < center[neighbor.image].size();
        }
    }

    // Invalid letters are rejected instead of indexing outside the matrix.
    ok = ok && parikh_columns(FiniteSubstitution{{0, 2}, {0}}).empty();
    ok = ok
        && class_ii_neighbor_dominant_core_states(0).size() == 15
        && class_ii_neighbor_dominant_core_states(1).size() == 17
        && class_ii_neighbor_dominant_core_states(2).size() == 39
        && class_ii_neighbor_affine_edges(0).size() == 41
        && class_ii_neighbor_affine_edges(1).size() == 34
        && class_ii_neighbor_affine_edges(2).size() == 91;
    ok = ok
        && class_ii_neighbor2_fixed_extension_states().size() == 24
        && class_ii_neighbor2_initial_extension_states().size() == 22
        && class_ii_neighbor2_signed_contact_set().size() == 50
        && class_ii_neighbor2_second_extension_states().size() == 25;
    // Proven 2026-08-01 (app/class_ii_neighbor2_round1_union_hypothesis.cpp):
    // the Round-1 target decomposes exactly as plus_minus_C(sigma_a) union
    // E_1 -- class_ii_contact_set(), its mirror, and E_1 are all
    // parameter-free, so this is a single universal set-equality check,
    // not a per-a sweep. Enrolled here for permanent regression coverage.
    {
        const auto contact = class_ii_contact_set();
        const auto e1 = class_ii_neighbor2_initial_extension_states();
        std::set<SNode<3>> decomposed;
        for (const auto& s : contact) decomposed.insert(s);
        for (const auto& s : contact) decomposed.insert(s.mirror());
        for (const auto& s : e1) decomposed.insert(s);
        ok = ok && decomposed == class_ii_neighbor2_signed_contact_set();
    }
    const auto initial_extension =
        class_ii_neighbor2_initial_extension_states();
    for (const auto& node : initial_extension) {
        ok = ok && initial_extension.count(node.mirror()) == 1;
    }
    const auto composition_certificate =
        class_ii_neighbor2_interior_composition_certificate();
    const auto pruning_grammar =
        class_ii_neighbor2_interior_pruning_grammar_certificate();
    const auto cell_exclusion =
        class_ii_neighbor2_cell_exclusion_certificate();
    const auto stable_rank1 =
        class_ii_neighbor2_stable_rank1_certificate();
    const auto stable_later_ranks =
        class_ii_neighbor2_stable_later_rank_certificate();
    const auto stable_composition =
        class_ii_neighbor2_stable_composition_certificate();
    const auto stable_affine_composition =
        class_ii_neighbor2_stable_affine_composition_certificate();
    const auto stable_window_exhaustion =
        class_ii_neighbor2_stable_window_exhaustion_certificate();
    const auto penultimate_partition =
        class_ii_neighbor2_penultimate_partition_certificate();
    const auto penultimate_rank1 =
        class_ii_neighbor2_penultimate_rank1_certificate();
    const auto penultimate_later_ranks =
        class_ii_neighbor2_penultimate_later_rank_certificate();
    const auto terminal_edit =
        class_ii_neighbor2_terminal_edit_certificate();
    const auto terminal_affine_composition =
        class_ii_neighbor2_terminal_affine_composition_certificate();
    const auto terminal_window_exhaustion =
        class_ii_neighbor2_terminal_window_exhaustion_certificate();
    const auto repeated_window_exhaustion =
        class_ii_neighbor2_repeated_window_exhaustion_certificate();
    const auto terminal_rank_edit =
        class_ii_neighbor2_terminal_rank_edit_shape_certificate();
    const auto terminal_rank1_endpoint =
        class_ii_neighbor2_terminal_rank1_endpoint_certificate();
    const auto terminal_inherited_rank1 =
        class_ii_neighbor2_terminal_inherited_rank1_certificate();
    const auto terminal_later_ranks =
        class_ii_neighbor2_terminal_later_rank_certificate();
    const auto fixed_red =
        class_ii_neighbor2_fixed_red_certificate();
    ok = ok
        && pruning_grammar.exact()
        && cell_exclusion.exact()
        && stable_rank1.exact()
        && stable_later_ranks.exact()
        && stable_composition.bounded_inclusion_exact
        && stable_affine_composition.exact()
        && stable_window_exhaustion.exact()
        && penultimate_partition.exact()
        && penultimate_rank1.exact()
        && penultimate_later_ranks.exact()
        && terminal_edit.exact()
        && terminal_affine_composition.exact()
        && terminal_window_exhaustion.exact()
        && repeated_window_exhaustion.exact()
        && terminal_rank_edit.exact()
        && terminal_rank1_endpoint.exact()
        && terminal_inherited_rank1.exact()
        && terminal_later_ranks.exact();
    ok = ok && fixed_red.exact();
    ok = ok
        && class_ii_pruning_linear_system_feasible({
            {0, 0, 1, 0}, {1, 0, -1, 0}})
        && !class_ii_pruning_linear_system_feasible({
            {0, 0, 1, 0}, {-1, 0, -1, 0}});
    std::printf(
        "  neighbor 2: universal growing-cell exclusion "
        "roles=%zu cell_pairs=%zu fixed_targets=%zu fixed_pairs=%zu "
        "prefix_branches=%zu unresolved=%zu exact=%d\n",
        cell_exclusion.role_count, cell_exclusion.pair_count,
        cell_exclusion.fixed_target_count,
        cell_exclusion.fixed_pair_count,
        cell_exclusion.prefix_branches,
        cell_exclusion.unresolved_branches,
        cell_exclusion.exact() ? 1 : 0);
    std::printf(
        "  neighbor 2: universal stable rank-one exclusion "
        "fixed=%zu frontier=%zu targets=%zu prefix_branches=%zu "
        "unresolved=%zu exact=%d\n",
        stable_rank1.fixed_source_count,
        stable_rank1.frontier_source_count,
        stable_rank1.target_role_count,
        stable_rank1.prefix_branches,
        stable_rank1.unresolved_branches,
        stable_rank1.exact() ? 1 : 0);
    std::printf(
        "  neighbor 2: universal stable later-rank exclusion "
        "rank2=%zu/%zu rank3=%zu/%zu prefix_branches=%zu "
        "predecessors=%d unresolved=%zu exact=%d\n",
        stable_later_ranks.rank2_sources,
        stable_later_ranks.rank2_targets,
        stable_later_ranks.rank3_sources,
        stable_later_ranks.rank3_targets,
        stable_later_ranks.prefix_branches,
        stable_later_ranks.predecessor_edges_exact ? 1 : 0,
        stable_later_ranks.unresolved_branches,
        stable_later_ranks.exact() ? 1 : 0);
    std::printf(
        "  neighbor 2: bounded full pre-Red inclusion "
        "layers=%zu witnesses=%zu exact=%d\n",
        stable_composition.layers_checked,
        stable_composition.witnesses_checked,
        stable_composition.bounded_inclusion_exact ? 1 : 0);
    std::printf(
        "  neighbor 2: universal stable affine composition "
        "fixed=%zu affine=%zu sources=%zu hops=%zu exact=%d\n",
        stable_affine_composition.fixed_targets,
        stable_affine_composition.affine_targets,
        stable_affine_composition.source_roles,
        stable_affine_composition.hop_roles,
        stable_affine_composition.exact() ? 1 : 0);
    std::printf(
        "  neighbor 2: universal stable window exhaustion "
        "compositions=%zu+%zu invalid=%zu+%zu shifted=%zu "
        "boundary=%zu unresolved=%zu valid=%d exact=%d\n",
        stable_window_exhaustion.fixed_compositions,
        stable_window_exhaustion.affine_compositions,
        stable_window_exhaustion.fixed_invalid,
        stable_window_exhaustion.affine_invalid,
        stable_window_exhaustion.affine_shifted,
        stable_window_exhaustion.boundary_instances,
        stable_window_exhaustion.unresolved,
        stable_window_exhaustion.target_window_validity_exact ? 1 : 0,
        stable_window_exhaustion.exact() ? 1 : 0);
    std::printf(
        "  neighbor 2: universal penultimate partition transport "
        "transferred=%zu promoted=%zu parameters=%zu exact=%d\n",
        penultimate_partition.transferred_states,
        penultimate_partition.promoted_states,
        penultimate_partition.parameters_checked,
        penultimate_partition.exact() ? 1 : 0);
    std::printf(
        "  neighbor 2: universal penultimate rank-one exclusion "
        "sources=%zu targets=%zu branches=%zu unresolved=%zu "
        "base=%d exact=%d\n",
        penultimate_rank1.affine_source_roles,
        penultimate_rank1.endpoint_target_roles,
        penultimate_rank1.prefix_branches,
        penultimate_rank1.unresolved_branches,
        penultimate_rank1.base_exact ? 1 : 0,
        penultimate_rank1.exact() ? 1 : 0);
    std::printf(
        "  neighbor 2: universal penultimate later-rank exclusion "
        "groups=%zu branches=%zu unresolved=%zu predecessors=%d "
        "base=%d exact=%d\n",
        penultimate_later_ranks.changed_pair_groups,
        penultimate_later_ranks.prefix_branches,
        penultimate_later_ranks.unresolved_branches,
        penultimate_later_ranks.predecessor_edges_exact ? 1 : 0,
        penultimate_later_ranks.base_exact ? 1 : 0,
        penultimate_later_ranks.exact() ? 1 : 0);
    std::printf(
        "  neighbor 2: terminal bounded affine edit "
        "removed=%zu added=%zu parameters=%zu exact=%d\n",
        terminal_edit.removed_roles,
        terminal_edit.added_roles,
        terminal_edit.validation_parameters,
        terminal_edit.exact() ? 1 : 0);
    std::printf(
        "  neighbor 2: universal terminal affine inclusion "
        "sources=%zu targets=%zu witnessed=%zu exact=%d\n",
        terminal_affine_composition.source_roles,
        terminal_affine_composition.target_roles,
        terminal_affine_composition.witnessed_targets,
        terminal_affine_composition.exact() ? 1 : 0);
    std::printf(
        "  neighbor 2: universal terminal window exhaustion "
        "sources=%zu compositions=%zu targets=%zu+%zu "
        "classified=%zu+%zu+%zu unresolved=%zu missing=%zu+%zu "
        "valid=%d exact=%d\n",
        terminal_window_exhaustion.source_roles,
        terminal_window_exhaustion.compositions,
        terminal_window_exhaustion.affine_target_roles,
        terminal_window_exhaustion.fixed_target_roles,
        terminal_window_exhaustion.matched,
        terminal_window_exhaustion.shifted,
        terminal_window_exhaustion.invalid,
        terminal_window_exhaustion.unresolved,
        terminal_window_exhaustion.missing_affine_targets,
        terminal_window_exhaustion.missing_fixed_targets,
        terminal_window_exhaustion.target_validity_exact ? 1 : 0,
        terminal_window_exhaustion.exact() ? 1 : 0);
    std::printf(
        "  neighbor 2: universal repeated window exhaustion "
        "sources=%zu compositions=%zu targets=%zu+%zu "
        "classified=%zu+%zu+%zu unresolved=%zu missing=%zu+%zu "
        "valid=%d exact=%d\n",
        repeated_window_exhaustion.source_roles,
        repeated_window_exhaustion.compositions,
        repeated_window_exhaustion.affine_target_roles,
        repeated_window_exhaustion.fixed_target_roles,
        repeated_window_exhaustion.matched,
        repeated_window_exhaustion.shifted,
        repeated_window_exhaustion.invalid,
        repeated_window_exhaustion.unresolved,
        repeated_window_exhaustion.missing_affine_targets,
        repeated_window_exhaustion.missing_fixed_targets,
        repeated_window_exhaustion.target_validity_exact ? 1 : 0,
        repeated_window_exhaustion.exact() ? 1 : 0);
    std::printf(
        "  neighbor 2: terminal rank-edit affine grammar "
        "removed=%zu added=%zu/%zu/%zu third=%d exact=%d\n",
        terminal_rank_edit.rank1_removed,
        terminal_rank_edit.rank1_added,
        terminal_rank_edit.rank2_added,
        terminal_rank_edit.rank3_added,
        terminal_rank_edit.third_parameter_exact ? 1 : 0,
        terminal_rank_edit.exact() ? 1 : 0);
    std::printf(
        "  neighbor 2: terminal rank-one bounded endpoint exclusion "
        "sources=%zu targets=%zu branches=%zu unresolved=%zu exact=%d\n",
        terminal_rank1_endpoint.source_roles,
        terminal_rank1_endpoint.target_roles,
        terminal_rank1_endpoint.prefix_branches,
        terminal_rank1_endpoint.unresolved_branches,
        terminal_rank1_endpoint.exact() ? 1 : 0);
    std::printf(
        "  neighbor 2: universal terminal inherited rank-one exclusion "
        "sources=%zu+%zu+%zu targets=%zu branches=%zu "
        "unresolved=%zu third=%d exact=%d\n",
        terminal_inherited_rank1.fixed_sources,
        terminal_inherited_rank1.indexed_cell_roles,
        terminal_inherited_rank1.endpoint_sources,
        terminal_inherited_rank1.target_roles,
        terminal_inherited_rank1.prefix_branches,
        terminal_inherited_rank1.unresolved_branches,
        terminal_inherited_rank1.third_parameter_grammar_exact ? 1 : 0,
        terminal_inherited_rank1.exact() ? 1 : 0);
    std::printf(
        "  neighbor 2: universal terminal later-rank exclusion "
        "sources=%zu+%zu targets=%zu/%zu branches=%zu "
        "predecessors=%zu unresolved=%zu exact=%d\n",
        terminal_later_ranks.new_rank2_sources,
        terminal_later_ranks.new_rank3_sources,
        terminal_later_ranks.post_rank1_new_targets,
        terminal_later_ranks.post_rank2_new_targets,
        terminal_later_ranks.prefix_branches,
        terminal_later_ranks.predecessor_edges,
        terminal_later_ranks.unresolved_branches,
        terminal_later_ranks.exact() ? 1 : 0);
    std::printf(
        "  neighbor 2: universal repeated Red exclusion "
        "rank1=%zu+%zu+%zu later=%zu/%zu/%zu/%zu/%zu "
        "branches=%zu predecessors=%zu unresolved=%zu base=%d exact=%d\n",
        fixed_red.fixed_rank1_sources,
        fixed_red.indexed_rank1_roles,
        fixed_red.endpoint_rank1_sources,
        fixed_red.later_rank_sources[0],
        fixed_red.later_rank_sources[1],
        fixed_red.later_rank_sources[2],
        fixed_red.later_rank_sources[3],
        fixed_red.later_rank_sources[4],
        fixed_red.prefix_branches,
        fixed_red.predecessor_edges,
        fixed_red.unresolved_branches,
        fixed_red.base_exact ? 1 : 0,
        fixed_red.exact() ? 1 : 0);
    ok = ok
        && composition_certificate.exact
        && composition_certificate.fixed_witnesses == 24
        && composition_certificate.affine_witnesses == 1;
    const auto center_interface =
        class_ii_neighbor2_center_interface_certificate();
    ok = ok && center_interface.exact();
    std::printf(
        "  neighbor 2: center interface contact-base=%d partition=%d "
        "signed-base=%d witnesses=%zu+%zu exact=%d\n",
        center_interface.contact_base_exact ? 1 : 0,
        center_interface.exact_partition ? 1 : 0,
        center_interface.initial_in_signed_contact ? 1 : 0,
        center_interface.round2_witnesses,
        center_interface.round3_witnesses,
        center_interface.exact() ? 1 : 0);
    const auto center_red =
        class_ii_center_base_red_certificate();
    ok = ok && center_red.exact;
    std::printf(
        "  center base: universal Red successor certificate "
        "stable_from=%lld edges=%zu parameter_checks=%zu\n",
        center_red.stable_from,
        center_red.successor_edges,
        center_red.parameter_checks);
    for (long long a = 3; a <= 128; ++a) {
        ok = ok
            && class_ii_center_base_red_composition_witnesses(2, a)
                .size() == 12
            && class_ii_center_base_red_composition_witnesses(3, a)
                .size() == 9;
    }
    const auto terminal_composition =
        class_ii_neighbor2_terminal_composition_certificate();
    const auto interior_red_bridge =
        class_ii_neighbor2_interior_red_bridge_certificate();
    ok = ok && interior_red_bridge.exact();
    std::printf(
        "  neighbor 2: Red bridges fixed=%d second=%d tip=%d "
        "penultimate=%d terminal=%d closure=%d "
        "stable_from=%lld checks=%zu\n",
        interior_red_bridge.fixed_exact ? 1 : 0,
        interior_red_bridge.second_exact ? 1 : 0,
        interior_red_bridge.tip_exact ? 1 : 0,
        interior_red_bridge.penultimate_exact ? 1 : 0,
        interior_red_bridge.terminal_exact ? 1 : 0,
        interior_red_bridge.closure_exact ? 1 : 0,
        interior_red_bridge.stable_from,
        interior_red_bridge.fixed_edge_checks);
    std::printf(
        "  neighbor 2: terminal sampled-witness alignment "
        "aligned=%d fixed=%zu affine=%zu\n",
        terminal_composition.exact ? 1 : 0,
        terminal_composition.fixed_targets,
        terminal_composition.affine_targets);
    ok = ok
        && terminal_composition.exact
        && terminal_composition.fixed_targets == 24
        && terminal_composition.affine_targets == 6;
    for (long long a = 4; a <= 128; ++a) {
        ok = ok
            && class_ii_neighbor2_interior_extension_states(
                static_cast<std::size_t>(a - 2)).size() == 25
            && class_ii_neighbor2_penultimate_extension_states(a).size() == 26
            && class_ii_neighbor2_center_extension(a).size() == 30;
        const auto witnesses =
            class_ii_neighbor2_interior_composition_witnesses(
                static_cast<std::size_t>(a));
        ok = ok && witnesses.size() == 25;
        if (a >= 5) {
            const auto round = static_cast<std::size_t>(a - 2);
            const auto red_bridges =
                class_ii_neighbor2_interior_red_bridges(round);
            std::set<SNode<3>> bridge_sources;
            for (const auto& bridge : red_bridges) {
                bridge_sources.insert(bridge.source);
                ok = ok
                    && class_ii_neighbor_transition_weight(
                        2, a, bridge.source, bridge.target) > 0;
            }
            ok = ok
                && bridge_sources
                    == class_ii_neighbor2_interior_extension_states(round)
                && class_ii_interior_shell(round)
                    .count(red_bridges.back().target) == 1;
        }
        const auto terminal_witnesses =
            class_ii_neighbor2_terminal_composition_witnesses(a);
        ok = ok && terminal_witnesses.size() == 30;
        if (a >= 5) {
            const auto penultimate_witnesses =
                class_ii_neighbor2_penultimate_composition_witnesses(a);
            ok = ok && penultimate_witnesses.size() == 26;
        }
    }
    for (std::size_t neighbor = 0; neighbor < 3; ++neighbor) {
        const auto certificate =
            class_ii_neighbor_affine_certificate(neighbor);
        ok = ok
            && certificate.exact
            && certificate.polynomial_exact
            && certificate.stable_from >= 3;
        std::printf(
            "  neighbor %zu: all-parameter affine certificate "
            "stable_from=%lld checked=%zu polynomial_points=%zu\n",
            neighbor, certificate.stable_from,
            certificate.matrices_checked,
            certificate.polynomial_points_checked);
        const auto shell_certificate =
            class_ii_neighbor_regular_shell_certificate(neighbor);
        ok = ok
            && shell_certificate.exact
            && shell_certificate.polynomial_exact;
        std::printf(
            "  neighbor %zu: universal regular-shell certificate "
            "cells=%zu checked=%zu polynomial_points=%zu\n",
            neighbor, shell_certificate.cells_checked,
            shell_certificate.matrices_checked,
            shell_certificate.polynomial_points_checked);
        const auto special_certificate =
            class_ii_neighbor_special_shell_certificate(neighbor);
        ok = ok && special_certificate.exact;
        std::printf(
            "  neighbor %zu: universal special-shell certificate "
            "families=%zu stable_from=%lld checked=%zu\n",
            neighbor, special_certificate.families_checked,
            special_certificate.stable_from,
            special_certificate.matrices_checked);
        const auto irreducibility =
            class_ii_neighbor_irreducibility_certificate(neighbor);
        ok = ok && irreducibility.exact();
        std::printf(
            "  neighbor %zu: universal catalogue irreducibility "
            "core=%d regular=%d special=%d monotone=%d\n",
            neighbor, irreducibility.core_irreducible ? 1 : 0,
            irreducibility.regular_shell_irreducible ? 1 : 0,
            irreducibility.special_shell_irreducible ? 1 : 0,
            irreducibility.monotone_support ? 1 : 0);
        for (long long a = 3; a <= 128; ++a) {
            const auto matrix =
                class_ii_neighbor_affine_matrix(neighbor, a);
            const auto compressed =
                class_ii_neighbor_compressed_matrix(neighbor, a);
            ok = ok
                && matrix.size()
                    == class_ii_neighbor_dominant_core_states(neighbor).size()
                && compressed == matrix;
        }
    }

    std::printf(
        "substitution_neighborhood_test: %s "
        "(three fixed-light neighbors through a=128; bounded prefix "
        "counts have exact finite-crossing certificates for affine "
        "core matrices 15,17,39 and all indexed regular shells; "
        "neighbor-2 interior correction has 24 fixed plus one affine "
        "raw-corona witness)\n",
        ok ? "PASS" : "FAIL");
    return ok ? 0 : 1;
}
