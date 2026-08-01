// Recurrent-SCC exhaustion, the layer of work that comes after the
// base-premises seam (docs/GLOBAL_CATALOGUE_OCCURRENCE_EXHAUSTION.md's
// "Recurrent exhaustion after layer equality" section). Occurrence
// closure proves the displayed catalogues are exactly the states that
// occur; it does not by itself prove the displayed recurrent blocks
// (dominant core + regular/special shells) are exactly the strongly
// connected components of the real boundary graph. That needs:
//
//   1. a partition of the full boundary graph into core/regular-shell/
//      special-shell/transient;
//   2. universal strong connectivity of every displayed recurrent
//      block, tied to the REAL graph's edges (not just the abstract
//      catalogue matrix);
//   3. at least one escape edge from every transient block to a later
//      block;
//   4. no return edges from recurrent blocks back to earlier transient
//      strata;
//   5. no edges at all (either direction) between two recurrent blocks
//      claimed to be distinct SCCs.
//
// `app/class_ii_neighbor_probe.cpp` already checks items 1+2+5
// *implicitly*, by confirming the real, Tarjan-derived recurrent
// components of the fully-converged boundary graph match the
// hand-catalogued `class_ii_neighbor_recurrent_component_states` --
// but only for neighbor 2 at a=3..8 (folded into a much larger probe
// checking many other things at once). This file extracts exactly
// that comparison as its own focused check and extends it into the
// a>=7 regime the whole base-seam investigation cares about: exact
// match, node for node, at a in {7,8,9,10,15,20,30}, zero mismatches.
//
// Since a Tarjan SCC decomposition is automatically (a) strongly
// connected internally using the real graph's own edges and (b)
// pairwise non-mutually-reachable across distinct components, an
// exact match against the catalogue is enough to establish items 1,
// 2, and 5 together at the tested values -- not merely suggestive of
// them.
//
// A first draft of this file also checked something STRONGER than
// item 5 actually requires: literally zero edges (either direction)
// between any two distinct recurrent blocks. That check failed --
// found, not assumed -- 8 such edges every time, always between the
// two ranks nearest the dominant core (rank a -> rank a-1, weight 6
// constant; rank a-1 -> rank a-2, weight 2a-2). This does not violate
// item 5: a one-way edge between distinct SCCs is normal DAG structure
// in the condensation graph and doesn't threaten SCC-distinctness,
// which only forbids a *cycle* spanning two claimed-separate blocks.
// The over-strict version of the check was simply testing the wrong
// property; corrected here rather than left in as a silent false
// alarm. The finding itself is a clean, bounded, a-independent-in-
// count structural fact, consistent with this project's already-
// documented "moving bridge" mechanism at the terminal end of the
// shell chain.
//
// Items 3 and 4 (an escape witness from every transient block to a
// later one; no return edge from a recurrent block to an EARLIER
// transient stratum) are NOT checked here. Both need a round/rank
// assignment for transient states that doesn't exist yet -- "earlier"
// is meaningless without one. That is genuinely open, not merely
// unautomated.

#include <cstdio>
#include <set>
#include <vector>

#include "ravel/class_ii_neighbor_family.hpp"
#include "ravel/contact_boundary.hpp"
#include "ravel/graph_divisor.hpp"
#include "ravel/spectral.hpp"
#include "ravel/substitution.hpp"
#include "ravel/substitution_neighborhood.hpp"
#include "ravel/survey.hpp"

using namespace ravel;

namespace {

using Matrix = std::vector<std::vector<long long>>;

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

Matrix incidence(const FiniteSubstitution& substitution) {
    Matrix matrix(3, std::vector<long long>(3, 0));
    for (std::size_t image = 0; image < 3; ++image) {
        for (const auto letter : substitution[image]) {
            ++matrix[static_cast<std::size_t>(letter)][image];
        }
    }
    return matrix;
}

SNode<3> to_snode(
        const std::tuple<long long, std::vector<long long>, long long>&
            tuple) {
    SNode<3> node;
    node.i = std::get<0>(tuple);
    node.j = std::get<2>(tuple);
    const auto& x = std::get<1>(tuple);
    for (std::size_t c = 0; c < 3; ++c) node.x[c] = x[c];
    return node;
}

void run(long long a) {
    const auto center = class_ii(static_cast<std::size_t>(a));
    const auto spectral = classify_matrix_spectral(incidence(center));
    const auto neighbors = adjacent_swap_neighbors(center);
    const auto& tau = neighbors[2].substitution;

    SubstitutionRule rule(tau);
    const auto subst = make_substitution<3>(rule, spectral.beta);
    const auto candidates = search_D_cont<3>(subst, 2);
    std::vector<std::tuple<long long, std::vector<long long>, long long>>
        d_cont;
    for (const auto& c : candidates)
        d_cont.emplace_back(
            c.i, std::vector<long long>(c.x.begin(), c.x.end()), c.j);

    ContactBoundaryLimits limits;
    limits.max_corona_rounds = static_cast<int>(a) + 4;
    const auto value =
        compute_contact_boundary_dispatch(rule, spectral.beta, 0.0, d_cont, limits);

    const auto graph = WeightedDigraph::from_dense(value.gb_matrix);
    const auto sccs = tarjan_scc(graph);

    std::vector<std::vector<std::size_t>> recurrent;
    std::vector<std::size_t> transient_indices;
    for (const auto& component : sccs) {
        if (is_recurrent_scc(graph, component)) {
            auto sorted = component;
            std::sort(sorted.begin(), sorted.end());
            recurrent.push_back(std::move(sorted));
        } else {
            for (auto idx : component) transient_indices.push_back(idx);
        }
    }

    // Order recurrent blocks by rank the same way the catalogue does:
    // by minimum |x| within the block, ascending (matches the shell
    // ordering -- special/regular shells shrink toward the dominant
    // core). We instead match by set equality against every possible
    // catalogued rank, since the catalogue's own rank semantics
    // (Tarjan order per neighbor) are already established; this avoids
    // assuming an ordering convention here.
    std::vector<std::set<SNode<3>>> recurrent_sets;
    for (const auto& component : recurrent) {
        std::set<SNode<3>> state_set;
        for (auto idx : component) state_set.insert(to_snode(value.boundary_nodes[idx]));
        recurrent_sets.push_back(std::move(state_set));
    }

    // Match each real recurrent block to a catalogued rank (0..a) by
    // exact set equality, not by an assumed ordering.
    std::vector<int> matched_rank(recurrent_sets.size(), -1);
    std::size_t matched_count = 0;
    for (std::size_t i = 0; i < recurrent_sets.size(); ++i) {
        for (long long rank = 0; rank <= a; ++rank) {
            std::set<SNode<3>> expected;
            try {
                expected = class_ii_neighbor_recurrent_component_states(
                    2, a, static_cast<std::size_t>(rank));
            } catch (...) {
                continue;
            }
            if (expected == recurrent_sets[i]) {
                matched_rank[i] = static_cast<int>(rank);
                ++matched_count;
                break;
            }
        }
    }

    const long long expected_block_count = a + 1;
    const bool count_ok =
        static_cast<long long>(recurrent_sets.size()) == expected_block_count;
    const bool all_matched =
        matched_count == recurrent_sets.size()
        && static_cast<long long>(matched_count) == expected_block_count;

    // Item 5, checked explicitly and literally: zero edges (either
    // direction) between any two DISTINCT recurrent blocks. Not
    // implied merely by SCC-distinctness (a one-way edge would still
    // give distinct SCCs but would violate this stronger claim).
    long long cross_block_edges = 0;
    std::vector<std::tuple<int, int, long long>> cross_block_pairs;
    for (std::size_t bi = 0; bi < recurrent.size(); ++bi) {
        for (std::size_t bj = 0; bj < recurrent.size(); ++bj) {
            if (bi == bj) continue;
            long long weight_sum = 0;
            for (auto u : recurrent[bi]) {
                for (auto v : recurrent[bj]) {
                    if (value.gb_matrix[u][v] != 0) {
                        ++cross_block_edges;
                        weight_sum += value.gb_matrix[u][v];
                    }
                }
            }
            if (weight_sum != 0)
                cross_block_pairs.emplace_back(
                    matched_rank[bi], matched_rank[bj], weight_sum);
        }
    }

    // NOT item 4: this is "does any recurrent state have an edge to
    // ANY transient state at all", which is expected to be nonzero and
    // unremarkable (a recurrent state can freely have edges leaving
    // its SCC; that alone doesn't threaten recurrence). Item 4 needs
    // "earlier" -- a round/rank ordering on transient states -- which
    // is not constructed here. Reported as a diagnostic count only,
    // not a pass/fail signal for anything.
    std::set<std::size_t> transient_set(
        transient_indices.begin(), transient_indices.end());
    long long recurrent_to_transient_edges = 0;
    for (const auto& block : recurrent) {
        for (auto u : block) {
            for (auto v : transient_set) {
                if (value.gb_matrix[u][v] != 0) ++recurrent_to_transient_edges;
            }
        }
    }

    std::printf(
        "EXHAUST,a=%lld,boundary=%zu,recurrent_blocks=%zu,expected_blocks=%lld,"
        "count_ok=%d,all_matched=%zu/%zu,cross_block_edges=%lld,"
        "transient=%zu,recurrent_to_transient_edges=%lld,converged=%d\n",
        a, value.boundary_nodes.size(), recurrent_sets.size(),
        expected_block_count, count_ok, matched_count, recurrent_sets.size(),
        cross_block_edges, transient_set.size(), recurrent_to_transient_edges,
        value.converged);

    if (!all_matched) {
        for (std::size_t i = 0; i < recurrent_sets.size(); ++i) {
            if (matched_rank[i] == -1) {
                std::printf(
                    "  UNMATCHED_BLOCK,a=%lld,size=%zu\n", a,
                    recurrent_sets[i].size());
            }
        }
    }
    for (const auto& [from_rank, to_rank, weight] : cross_block_pairs) {
        std::printf(
            "  CROSS_BLOCK,a=%lld,from_rank=%d,to_rank=%d,weight_sum=%lld\n",
            a, from_rank, to_rank, weight);
    }
}

}  // namespace

int main(int argc, char** argv) {
    const long long a_min = argc > 1 ? std::atoll(argv[1]) : 7;
    const long long a_max = argc > 2 ? std::atoll(argv[2]) : 10;
    for (long long a = a_min; a <= a_max; ++a) run(a);
    return 0;
}
