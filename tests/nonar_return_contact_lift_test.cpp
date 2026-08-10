#include <algorithm>
#include <array>
#include <cstdio>
#include <functional>
#include <set>
#include <tuple>
#include <vector>

#include "ravel/contact_boundary.hpp"
#include "ravel/d_cont_check.hpp"
#include "ravel/marker_power_return_core.hpp"
#include "adelic/prefix_automaton.hpp"
#include "adelic/coincidence_and_property_f.hpp"
#include "adelic/property_f_role_digit_cocycle.hpp"
#include "adelic/return_phase_digit_cocycle.hpp"
#include "adelic/return_contact_digit_holonomy.hpp"
#include "math/charpoly.hpp"
#include "math/linalg_qbeta.hpp"
#include "ravel/return_contact_lift.hpp"
#include "ravel/return_history_factor.hpp"
#include "ravel/return_offset_fibre_certificate.hpp"
#include "ravel/return_offset_fibre_holonomy.hpp"
#include "ravel/return_contact_textile_certificate.hpp"
#include "ravel/spectral.hpp"
#include "ravel/substitution.hpp"
#include "ravel/survey.hpp"

using namespace ravel;

namespace {
int tests = 0;
int failures = 0;
void expect(bool value, const char* label) {
    ++tests;
    if (value) std::printf("  [ok]   %s\n", label);
    else { std::printf("  [FAIL] %s\n", label); ++failures; }
}

std::size_t largest_nonzero_recurrent_scc(const ReturnContactLift<3>& lift) {
    const std::size_t n = lift.states.size();
    std::vector<std::vector<std::size_t>> graph(n);
    for (const auto& edge : lift.edges) graph[edge.source].push_back(edge.destination);
    std::vector<int> index(n, -1), low(n, 0), stack;
    std::vector<bool> active(n, false);
    int next = 0;
    std::size_t largest = 0;
    std::function<void(std::size_t)> visit = [&](std::size_t v) {
        index[v] = low[v] = next++;
        stack.push_back(static_cast<int>(v));
        active[v] = true;
        for (std::size_t w : graph[v]) {
            if (index[w] < 0) {
                visit(w);
                low[v] = std::min(low[v], low[w]);
            } else if (active[w]) {
                low[v] = std::min(low[v], index[w]);
            }
        }
        if (low[v] != index[v]) return;
        std::vector<std::size_t> component;
        while (true) {
            const std::size_t w = static_cast<std::size_t>(stack.back());
            stack.pop_back();
            active[w] = false;
            component.push_back(w);
            if (w == v) break;
        }
        bool cyclic = component.size() > 1;
        if (!cyclic && !component.empty()) {
            const auto u = component.front();
            for (std::size_t w : graph[u]) if (w == u) cyclic = true;
        }
        if (!cyclic) return;
        for (std::size_t u : component) {
            const auto& x = lift.states[u].contact.x;
            bool nonzero = false;
            for (long long coordinate : x) if (coordinate != 0) nonzero = true;
            if (nonzero) largest = std::max(largest, component.size());
        }
    };
    for (std::size_t v = 0; v < n; ++v)
        if (index[v] < 0) visit(v);
    return largest;
}
}

int main() {
    const SubstitutionRule base({{1, 2}, {2}, {0}});
    const auto powered = marker_power_rule(base, 3);
    const auto core = analyze_marker_power_return_core(base, 0);
    expect(core.holds && core.power == 3,
           "non-AR base has a certified marker-power core");

    std::vector<std::vector<long long>> incidence(
        powered.alphabet_size(), std::vector<long long>(powered.alphabet_size(), 0));
    for (std::size_t source = 0; source < powered.alphabet_size(); ++source)
        for (auto letter : powered.image(source))
            ++incidence[static_cast<std::size_t>(letter)][source];
    const auto spectral = classify_matrix_spectral(incidence);
    const auto substitution = make_substitution<3>(powered, spectral.beta);
    std::vector<std::tuple<long long, std::vector<long long>, long long>> d_cont;
    for (const auto& candidate : search_D_cont<3>(substitution, 2))
        d_cont.emplace_back(candidate.i,
                            std::vector<long long>(candidate.x.begin(), candidate.x.end()),
                            candidate.j);
    const auto boundary = compute_contact_boundary_dispatch(
        powered, spectral.beta, spectral.b2, d_cont);
    expect(boundary.converged && !boundary.closure_stopped_early
               && !boundary.corona_capped,
           "powered non-AR contact boundary closes");
    expect(!boundary.boundary_nodes.empty(),
           "powered non-AR contact boundary is nonempty");

    std::vector<SNode<3>> bare;
    for (const auto& [i, x, j] : boundary.boundary_nodes) {
        SNode<3> node{};
        node.i = i;
        node.j = j;
        for (std::size_t k = 0; k < 3; ++k) node.x[k] = x[k];
        bare.push_back(node);
    }
    const auto phases = build_return_phase_system(powered, 0);
    std::vector<ReturnContactState<3>> seeds;
    for (const auto& node : bare) {
        const auto compatible = compatible_return_contact_seeds(node, phases);
        seeds.insert(seeds.end(), compatible.begin(), compatible.end());
    }
    ReturnContactLimits limits;
    limits.max_states = 1 << 18;
    limits.max_edges = 1 << 20;
    const auto lift = build_reachable_return_contact_lift(
        substitution, bare, phases, seeds, limits);
    expect(lift.complete(), "non-AR return/contact lift closes");
    expect(lift.projected_node_count == bare.size(),
           "lift projects onto every powered contact node");
    expect(!lift.edges.empty(), "non-AR return/contact lift has transport edges");
    const auto history = probe_return_history_factor(lift, phases);
    std::printf("return-history keys=%zu conflicts=%zu offset_variants=%zu offset_sensitive=%zu max_variants=%zu\n",
                history.history_keys, history.conflicting_keys,
                history.offset_variant_keys, history.offset_sensitive_keys,
                history.maximum_offset_variants);
    expect(history.conflicts_are_offset_driven
               && history.offset_sensitive_keys == history.offset_variant_keys,
           "every multi-offset history key has offset-sensitive transport");
    const auto fibre = derive_return_offset_fibre_certificate(lift, phases);
    std::printf("offset-fibre states=%zu edges=%zu cyclic_sccs=%zu largest=%zu "
                "class_sizes=%zu..%zu uniform=%d\n",
                fibre.states, fibre.edges, fibre.cyclic_sccs,
                fibre.largest_cyclic_scc, fibre.minimum_class_size,
                fibre.maximum_class_size, fibre.uniform_class_sizes ? 1 : 0);
    expect(fibre.finite && fibre.cyclic_sccs > 0,
           "offset fibre is finite and carries recurrent phase transport");
    expect(!fibre.uniform_class_sizes,
           "offset quotient is a relation, not a uniform permutation cover");
    const auto textile = derive_return_contact_textile_certificate(lift, phases);
    std::printf("textile pairs=%zu biregular=%zu permutation=%zu irregular=%zu "
                "branching=%d\n", textile.nonempty_pairs,
                textile.biregular_pairs, textile.permutation_pairs,
                textile.irregular_pairs, textile.has_branching_channels ? 1 : 0);
    expect(textile.finite && textile.nonempty_pairs > 0,
           "contact-aware quotient has a finite textile relation");
    expect(textile.irregular_pairs > 0 || textile.has_branching_channels,
           "contact-aware relation retains branching or irregular channels");
    const auto holonomy = derive_return_offset_fibre_holonomy(lift, phases);
    std::printf("holonomy recurrent_sccs=%zu gcd_one_sccs=%zu",
                holonomy.recurrent_sccs, holonomy.gcd_one_sccs);
    for (const auto& scc : holonomy.sccs)
        std::printf(" [nodes=%zu gcd=%lld max=%lld]", scc.nodes,
                    scc.gcd_cycle_residue, scc.max_abs_cycle_residue);
    std::printf("\n");
    expect(holonomy.finite && holonomy.recurrent_sccs > 0,
           "finite offset quotient has classified recurrent holonomy");
    expect(holonomy.recurrent_sccs == 1 && holonomy.sccs.front().gcd_cycle_residue == 3 &&
               !holonomy.sccs.front().coboundary,
           "non-AR transport has a genuine period-three twist");

    // The stronger Q(beta) test asks whether that integer transport survives
    // as a recurrent zero-expansion obstruction.  Build the live automaton
    // for the same powered rule, rather than reusing a hand-entered label set.
    std::vector<std::vector<long long>> powered_images(
        powered.alphabet_size(), std::vector<long long>{});
    std::vector<std::vector<long long>> powered_incidence(
        powered.alphabet_size(),
        std::vector<long long>(powered.alphabet_size(), 0));
    for (std::size_t source = 0; source < powered.alphabet_size(); ++source) {
        for (const auto letter : powered.image(source)) {
            powered_images[source].push_back(letter);
            ++powered_incidence[static_cast<std::size_t>(letter)][source];
        }
    }
    const auto charpoly = mathlib::charpoly_faddeev_leverrier(powered_incidence);
    const mathlib::QBetaRing ring(charpoly);
    const auto eigen = mathlib::left_eigenvector_via_qbeta_reduced_factor(
        powered_incidence, ring);
    std::array<std::vector<long long>, 3> automaton_images{};
    for (std::size_t source = 0; source < 3; ++source)
        automaton_images[source] = powered_images[source];
    const auto automaton = adelic::build_prefix_automaton<3>(
        automaton_images, eigen.v, ring);
    long long zero_beyond_frontier = -1;
    adelic::PropertyFGraph property_graph;
    const auto property_f = adelic::check_property_f<3>(
        automaton, 1'000'000, nullptr, nullptr, nullptr,
        &zero_beyond_frontier, &property_graph, true);
    std::printf("exact Property-F: holds=%d closed=%d nodes=%lld zero=%lld "
                "nonzero=%lld zero_beyond=%lld\n",
                property_f.holds ? 1 : 0, property_f.closure_reached ? 1 : 0,
                property_f.nodes_explored, property_f.zero_nodes,
                property_f.nonzero_nodes, zero_beyond_frontier);
    expect(property_f.holds && property_f.closure_reached &&
               zero_beyond_frontier == 0,
           "powered non-AR exact zero-expansion graph closes");
    const auto contact_holonomy = adelic::derive_return_contact_digit_holonomy<3>(
        automaton_images, lift, automaton, seeds);
    std::printf("contact Q(beta) holonomy: cyclic_sccs=%zu nontrivial=%zu\n",
                contact_holonomy.cyclic_sccs,
                contact_holonomy.nontrivial_holonomy_sccs);
    std::printf("  zero_seed_count=%zu zero_seed_reachable=%zu "
                "nontrivial_reachable=%zu reaches_nontrivial=%d "
                "ambiguous_left=%zu ambiguous_right=%zu single=%d\n",
                contact_holonomy.zero_seed_count,
                contact_holonomy.zero_seed_reachable_states,
                contact_holonomy.zero_seed_reachable_nontrivial_nodes,
                contact_holonomy.zero_seed_reaches_nontrivial_holonomy ? 1 : 0,
                contact_holonomy.left_frontier_ambiguous_states,
                contact_holonomy.right_frontier_ambiguous_states,
                contact_holonomy.single_valued_frontier_projection ? 1 : 0);
    for (const auto& scc : contact_holonomy.sccs)
        std::printf("  contact_scc nodes=%zu residual_edges=%zu left=%zu "
                    "right=%zu ranks=%zu/%zu/%zu zero=%zu nonzero=%zu coboundary=%d "
                    "left_cob=%d right_cob=%d\n", scc.nodes,
                    scc.residual_edges, scc.left_residual_edges,
                    scc.right_residual_edges, scc.residual_rank,
                    scc.left_residual_rank, scc.right_residual_rank,
                    scc.zero_contact_nodes,
                    scc.nonzero_contact_nodes, scc.coboundary ? 1 : 0,
                    scc.left_coboundary ? 1 : 0,
                    scc.right_coboundary ? 1 : 0);
    expect(contact_holonomy.exact && contact_holonomy.cyclic_sccs > 0,
           "full contact lift has classified Q(beta) recurrent cycles");
    bool full_rank_frontier_residual = false;
    bool all_difference_sccs_consistent = true;
    for (const auto& scc : contact_holonomy.sccs) {
        all_difference_sccs_consistent &= scc.coboundary;
        if ((scc.nodes == 52 || scc.nodes == 401) &&
            scc.residual_rank == 3 && scc.left_residual_rank == 3 &&
            scc.right_residual_rank == 3)
            full_rank_frontier_residual = true;
    }
    expect(contact_holonomy.nontrivial_holonomy_sccs == 0 &&
               all_difference_sccs_consistent && full_rank_frontier_residual,
           "affine difference holonomy is cohomologically consistent");
    expect(contact_holonomy.zero_seed_count > 0 &&
               contact_holonomy.zero_seed_reachable_states > 0 &&
               contact_holonomy.left_frontier_ambiguous_states > 0 &&
               contact_holonomy.right_frontier_ambiguous_states > 0,
           "zero-contact frontier reaches multivalued affine transport");
    expect(!contact_holonomy.single_valued_frontier_projection,
           "frontier-to-contact projection is genuinely multivalued");
    const auto digit_cocycle = adelic::derive_property_f_role_digit_cocycle<3>(
        automaton_images, automaton, 16, 1'000'000);
    std::printf("powered digit cocycle: zero_pairs=%zu missing=%zu "
                "recurrent_missing=%zu\n",
                digit_cocycle.zero_kernel_pairs,
                digit_cocycle.zero_kernel_missing_pairs,
                digit_cocycle.recurrent_zero_kernel_missing_pairs);
    // This deliberately fails the naïve bridge hypothesis: the coarse
    // letter-role cocycle forgets the return collar and misses recurrent zero
    // witnesses even though the full Property-F graph closes.
    expect(!digit_cocycle.proved &&
               digit_cocycle.recurrent_zero_kernel_missing_pairs > 0,
           "coarse role cocycle is correctly rejected as an incomplete bridge");
    const auto collar_cocycle = adelic::derive_return_phase_digit_cocycle<3>(
        automaton_images, phases, automaton, 16, 1'000'000);
    std::printf("return-collar cocycle: phase_states=%zu edges=%zu "
                "zero_pairs=%zu missing=%zu recurrent_missing=%zu\n",
                collar_cocycle.phase_states, collar_cocycle.edge_count,
                collar_cocycle.zero_kernel_pairs,
                collar_cocycle.zero_kernel_missing_pairs,
                collar_cocycle.recurrent_zero_kernel_missing_pairs);
    expect(collar_cocycle.cocycle_edges_exact &&
               !collar_cocycle.recurrent_zero_kernel_complete &&
               collar_cocycle.recurrent_zero_kernel_missing_pairs > 0,
           "phase collar alone is correctly rejected as an incomplete bridge");
    const std::size_t nonzero_scc = largest_nonzero_recurrent_scc(lift);
    expect(nonzero_scc > 0,
           "non-AR contact lift exposes a recurrent nonzero transport component");

    std::printf("%d tests run, %d failed. boundary=%zu phases=%zu lift=%zu edges=%zu\n",
                tests, failures, bare.size(), phases.states.size(),
                lift.states.size(), lift.edges.size());
    std::printf("largest_nonzero_contact_scc=%zu\n", nonzero_scc);
    return failures == 0 ? 0 : 1;
}
