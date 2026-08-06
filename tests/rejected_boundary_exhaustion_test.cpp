#include <cstdio>

#include "ravel/proof/rejected_boundary_exhaustion.hpp"

using namespace ravel;
using namespace ravel::proof;

int main() {
    WeightedDigraph terminal_graph(2);
    terminal_graph.add_edge(0, 1, 1);
    terminal_graph.add_edge(1, 0, 1);
    const auto terminal = derive_rejected_boundary_exhaustion(
        3, 1, terminal_graph, true, true, 2, 2,
        [](std::size_t) { return false; });
    if (!terminal.proved || terminal.terminal_components != 1) return 1;

    WeightedDigraph branched_graph(3);
    branched_graph.add_edge(0, 1, 1);
    branched_graph.add_edge(1, 0, 1);
    branched_graph.add_edge(0, 2, 1);
    branched_graph.add_edge(2, 0, 1);
    const auto known = derive_rejected_boundary_exhaustion(
        3, 1, branched_graph, true, true, 4, 4,
        [](std::size_t) { return true; });
    if (!known.proved || known.known_components != 1) return 2;

    const auto unknown = derive_rejected_boundary_exhaustion(
        3, 1, branched_graph, true, true, 4, 4,
        [](std::size_t) { return false; });
    if (unknown.proved || unknown.unknown_components != 1) return 3;

    WeightedDigraph dominance_graph(4);
    // Structurally selected core: complete two-state digraph, rho=2.
    dominance_graph.add_edge(0, 0, 1);
    dominance_graph.add_edge(0, 1, 1);
    dominance_graph.add_edge(1, 0, 1);
    dominance_graph.add_edge(1, 1, 1);
    // Unknown Fibonacci competitor, rho=phi<2 and not a permutation SCC.
    dominance_graph.add_edge(2, 2, 1);
    dominance_graph.add_edge(2, 3, 1);
    dominance_graph.add_edge(3, 2, 1);
    auto dominance_initial = derive_rejected_boundary_exhaustion(
        3, 1, dominance_graph, true, true, 7, 7,
        [](std::size_t vertex) { return vertex < 2; });
    if (dominance_initial.proved || dominance_initial.unknown_components != 1)
        return 5;
    ReplayedBoundaryGraph<3> replay;
    replay.nodes.resize(4);
    replay.graph = dominance_graph;
    replay.closure_complete = true;
    replay.replay_evidence_valid = true;
    const auto* core0 = &replay.nodes[0];
    const auto* core1 = &replay.nodes[1];
    const auto dominated = derive_replayed_boundary_dominance(
        dominance_initial, replay,
        [&](const SNode<3>& node) { return &node == core0 || &node == core1; },
        80);
    if (!dominated.proved || dominated.dominated_components != 1 ||
        dominated.unknown_components != 0)
        return 6;

    std::vector<std::vector<long long>> theta(7, std::vector<long long>(7, 0));
    theta[0][4] = 1; theta[1][0] = 1; theta[2][3] = 1;
    theta[3][5] = 1; theta[4][6] = 1; theta[5][1] = 1;
    theta[6][2] = 2;
    std::vector<std::vector<long long>> strong_core{{1,1},{1,1}};
    const auto block_power = derive_weighted_cycle_block_power(theta);
    if (!block_power.derived || !block_power.replayed ||
        block_power.period != 7 || block_power.multiplicity != 2 ||
        block_power.cycle_order.size() != 7)
        return 7;
    const auto renewal = derive_macro_renewal_dominance(theta, strong_core, 8, 16);
    if (!renewal.replayed || !renewal.block_power_derived ||
        !renewal.block_power_replayed || renewal.period != 7 ||
        renewal.multiplicity != 2)
        return 8;

    ReplayedBoundaryGraph<3> theta_replay;
    theta_replay.nodes.resize(7);
    theta_replay.graph = WeightedDigraph::from_dense(theta);
    theta_replay.closure_complete = true;
    theta_replay.replay_evidence_valid = true;
    const std::vector<std::size_t> theta_vertices{0,1,2,3,4,5,6};
    const auto state_cycle = derive_state_level_weighted_cycle(
        theta_replay, theta_vertices);
    if (!state_cycle.derived || !state_cycle.replayed ||
        state_cycle.period != 7 || state_cycle.multiplicity != 2 ||
        state_cycle.class_members.size() != 7 ||
        state_cycle.witnessed_state_edges.size() != 7)
        return 10;
    const auto state_renewal = derive_state_level_macro_renewal_dominance(
        theta_replay, theta_vertices, strong_core, 8, 16);
    if (!state_renewal.replayed || !state_renewal.block_power_derived ||
        !state_renewal.block_power_replayed || state_renewal.period != 7 ||
        state_renewal.multiplicity != 2)
        return 11;


    ReplayedBoundaryGraph<7> symbolic_replay;
    symbolic_replay.nodes.resize(7);
    symbolic_replay.graph = WeightedDigraph(7);
    symbolic_replay.closure_complete = true;
    symbolic_replay.replay_evidence_valid = true;
    for (std::size_t z = 0; z < 7; ++z) {
        auto& node = symbolic_replay.nodes[z];
        node.i = static_cast<long long>(z % 2);
        node.j = static_cast<long long>((z + 1) % 2);
        node.x.fill(0);
        long long sign = 1;
        for (std::size_t step = 1; step < 7; ++step) {
            node.x[(z + step) % 7] = sign;
            sign = -sign;
        }
        symbolic_replay.graph.add_edge(z, (z + 6) % 7, z == 2 ? 2 : 1);
    }
    const std::vector<std::size_t> symbolic_vertices{0,1,2,3,4,5,6};
    const auto symbolic = derive_symbolic_alternating_zero_cycle(
        symbolic_replay, symbolic_vertices);
    if (!symbolic.derived || !symbolic.descriptors_total ||
        !symbolic.phase_transport_replayed ||
        !symbolic.equitable_partition_replayed || symbolic.period != 7 ||
        symbolic.multiplicity != 2 || symbolic.branch_phase != 2)
        return 12;
    symbolic_replay.nodes[4].x[5] = -1;
    const auto malformed_symbolic = derive_symbolic_alternating_zero_cycle(
        symbolic_replay, symbolic_vertices);
    if (malformed_symbolic.derived) return 13;

    auto branching = theta;
    branching[0][1] = 1;
    const auto not_cycle = derive_weighted_cycle_block_power(branching);
    if (not_cycle.derived || not_cycle.replayed) return 9;

    std::array<std::vector<long long>,7> nbonacci_images{};
    for (std::size_t c = 0; c + 1 < 7; ++c)
        nbonacci_images[c] = {0, static_cast<long long>(c + 1)};
    nbonacci_images[6] = {0};
    Substitution<7> nbonacci7(nbonacci_images, 1.99);
    const auto schedule = derive_nbonacci_symbolic_label_schedule(nbonacci7);
    if (!schedule.derived || !schedule.candidate_family_generated ||
        !schedule.exact_prefix_graph_generated ||
        !schedule.recurrent_schedules_derived || !schedule.closed_form_schedules_derived ||
        !schedule.predicted_schedules_closed || !schedule.transient_label_rank_derived ||
        !schedule.direct_label_rank_formula_derived ||
        !schedule.endpoint_regions_derived || !schedule.transition_cases_derived ||
        !schedule.candidate_enumeration_avoided ||
        !schedule.scc_enumeration_avoided || !schedule.unique_branch_phase ||
        schedule.components.size() != 2 ||
        schedule.components.front().branch_phase != 2)
        return 14;
    for (const auto& component : schedule.components) {
        if (!component.renewal_grammar_derived || component.vertices.size() != 9 ||
            component.states.size() != 9 || component.phase_label_pairs.size() != 7 ||
            component.phase_weight.size() != 7)
            return 15;
    }
    ReplayedBoundaryGraph<7> predicted_component;
    predicted_component.nodes = schedule.components.front().states;
    predicted_component.graph = WeightedDigraph(predicted_component.nodes.size());
    predicted_component.closure_complete = true;
    predicted_component.replay_evidence_valid = true;
    std::vector<std::size_t> predicted_vertices(predicted_component.nodes.size());
    for (std::size_t k = 0; k < predicted_vertices.size(); ++k) predicted_vertices[k] = k;
    if (!replay_component_matches_symbolic_label_schedule(
            predicted_component, predicted_vertices, schedule))
        return 16;
    predicted_component.nodes.front().i = 6;
    if (replay_component_matches_symbolic_label_schedule(
            predicted_component, predicted_vertices, schedule))
        return 17;


    // The direct endpoint-label rank formula is dimension-parametric and
    // covers both odd renewal dimensions and even fully transient dimensions.
    const auto check_direct_formula = []<std::size_t D>() {
        std::array<std::vector<long long>,D> images{};
        for (std::size_t c = 0; c + 1 < D; ++c)
            images[c] = {0, static_cast<long long>(c + 1)};
        images[D - 1] = {0};
        const Substitution<D> substitution(images, 1.99);
        const auto proof = derive_nbonacci_symbolic_label_schedule(substitution);
        if (!proof.transient_label_rank_derived ||
            !proof.direct_label_rank_formula_derived ||
            !proof.endpoint_regions_derived || !proof.transition_cases_derived ||
            !proof.candidate_enumeration_avoided ||
            !proof.scc_enumeration_avoided || proof.maximum_transient_rank != 2)
            return false;
        if constexpr (D == 5 || D == 7)
            return proof.derived && proof.components.size() == 2;
        else
            return !proof.derived && proof.components.empty();
    };
    if (!check_direct_formula.template operator()<5>() ||
        !check_direct_formula.template operator()<6>() ||
        !check_direct_formula.template operator()<7>() ||
        !check_direct_formula.template operator()<8>() ||
        !check_direct_formula.template operator()<9>())
        return 18;

    // Forced-complement atom classification: no descriptor-pair search.
    // D=7 one-zero alternation necessarily uses two disjoint triples; D=5
    // admits the root/root cover.  D=9 is excluded by the support bound.
    for (const auto& component : schedule.components) {
        for (const auto& node : component.states) {
            const auto cert = derive_unit_grade_two_cancellation<7>(node.x);
            if (!cert.derived || !cert.forced_complement_derived ||
                cert.kind != GradeTwoAtomPairKind::TripleTriple ||
                cert.overlap_size != 0 || cert.cancellation_sites != 0)
                return 22;
        }
    }
    std::array<long long,5> d5{{0,-1,1,-1,1}};
    const auto d5_cert = derive_unit_grade_two_cancellation<5>(d5);
    if (!d5_cert.derived || !d5_cert.forced_complement_derived ||
        d5_cert.kind != GradeTwoAtomPairKind::RootRoot)
        return 23;
    std::array<long long,9> d9{{0,-1,1,-1,1,-1,1,-1,1}};
    const auto d9_cert = derive_unit_grade_two_cancellation<9>(d9);
    if (d9_cert.derived) return 24;

    // The universal rejected partition accepts an exact prederived renewal
    // schedule and rejects a non-permutation recurrent component outside it.
    ReplayedBoundaryGraph<7> partition_replay;
    partition_replay.nodes = schedule.components.front().states;
    partition_replay.graph = WeightedDigraph(partition_replay.nodes.size());
    partition_replay.closure_complete = true;
    partition_replay.replay_evidence_valid = true;
    partition_replay.atom_evidence_preserved = true;
    partition_replay.atom_witnesses.resize(partition_replay.nodes.size());
    for (std::size_t k = 0; k < partition_replay.nodes.size(); ++k) {
        const auto witness = derive_corona_atom_witness(partition_replay.nodes[k]);
        if (!witness) return 25;
        partition_replay.atom_witnesses[k] = *witness;
    }
    std::map<SNode<7>,std::size_t> partition_index;
    for (std::size_t k = 0; k < partition_replay.nodes.size(); ++k)
        partition_index.emplace(partition_replay.nodes[k], k);
    for (std::size_t u = 0; u < partition_replay.nodes.size(); ++u)
        for (const auto& [target,prefixes] :
             simple_forward_targets_exact<7>(nbonacci7, partition_replay.nodes[u])) {
            const auto it = partition_index.find(target);
            if (it == partition_index.end()) continue;
            partition_replay.graph.add_edge(u, it->second, 1);
            partition_replay.prefix_witnesses.push_back(
                {u,it->second,prefixes.first,prefixes.second});
        }
    const auto shape_invariant = derive_rejected_state_shape_invariant(
        partition_replay, [](const SNode<7>&) { return false; });
    if (!shape_invariant.derived ||
        !shape_invariant.request_atom_decomposition_derived ||
        !shape_invariant.catalogue_grade_replayed ||
        !shape_invariant.grade_bound_replayed ||
        !shape_invariant.quadratic_norm_defect_derived ||
        !shape_invariant.pure_rotation_derived ||
        !shape_invariant.unit_coordinates_derived ||
        !shape_invariant.grade_two_atom_decomposition_derived ||
        !shape_invariant.support_bound_derived ||
        !shape_invariant.renewal_dimension_bound_derived ||
        !shape_invariant.unit_grade_two_cancellation_classified ||
        !shape_invariant.forced_complement_classification_derived ||
        !shape_invariant.closed_overlap_cases_derived ||
        shape_invariant.maximum_grade_two_support != 6 ||
        shape_invariant.root_root_certificates +
                shape_invariant.root_triple_certificates +
                shape_invariant.triple_triple_certificates !=
            shape_invariant.checked_states ||
        !shape_invariant.unique_zero_derived ||
        !shape_invariant.cyclic_alternation_derived ||
        shape_invariant.checked_components != 1 ||
        shape_invariant.checked_states != 9)
        return 19;
    auto witness_loss_replay = partition_replay;
    witness_loss_replay.atom_witnesses.front().reset();
    const auto witness_loss_shape = derive_rejected_state_shape_invariant(
        witness_loss_replay, [](const SNode<7>&) { return false; });
    if (witness_loss_shape.derived ||
        witness_loss_shape.obstruction.find("lost its boundary atom witness") == std::string::npos)
        return 26;

    const auto symbolic_partition = derive_symbolic_rejected_boundary_partition(
        nbonacci7, partition_replay, [](const SNode<7>&) { return false; });
    if (!symbolic_partition.derived ||
        !symbolic_partition.shape_certificate_consumed ||
        !symbolic_partition.dimension_branch_classified ||
        symbolic_partition.alternating_zero_components != 1 ||
        symbolic_partition.unclassified_components != 0)
        return 20;
    partition_replay.nodes.front().x.fill(1);
    const auto bad_partition = derive_symbolic_rejected_boundary_partition(
        nbonacci7, partition_replay, [](const SNode<7>&) { return false; });
    if (bad_partition.derived || bad_partition.unclassified_components != 1)
        return 21;

    const auto invalid = derive_rejected_boundary_exhaustion(
        3, 1, terminal_graph, true, false, 2, 0,
        [](std::size_t) { return true; });
    if (invalid.proved || invalid.replay_evidence_valid) return 4;

    std::printf("rejected boundary exhaustion PASS\n");
    return 0;
}
