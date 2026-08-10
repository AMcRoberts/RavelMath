// Recurrent-SCC exhaustion, items 3 and 4 (docs/GLOBAL_CATALOGUE_
// OCCURRENCE_EXHAUSTION.md's "Recurrent exhaustion after layer
// equality" section):
//
//   3. at least one directed escape from every transient block toward
//      a later block;
//   4. absence of return paths from recurrent blocks to earlier
//      transient strata, most naturally through a monotone rank/round
//      function.
//
// Both need "earlier"/"later", which needs a round assigned to every
// state, not just the ones already indexed by round in the base/
// stable/penultimate/terminal/repeated dispatchers. This file builds
// that assignment directly from the trusted corona trace (the same
// algorithm2_trace machinery used throughout this investigation, kept
// in full this time instead of only its converged endpoint):
//
//   birth_round(state) = the smallest round r such that state is a
//   member of A_r (the round-r post-Red survivor set). Once a state
//   enters A_r it never leaves any later A_{r'>=r} (Red only prunes
//   within a round's own raw candidates before that round's survivors
//   are fixed), so this is well-defined without ambiguity.
//
// Step 1: check whether every recurrent block (as already verified
// against real Tarjan output in class_ii_neighbor2_recurrent_
// exhaustion_check.cpp) has a SINGLE birth round across all its
// members -- i.e. whether "rank" and "round" are the same ordering,
// which was flagged as the one genuine unknown in this file's scope.
// Step 2/3: group transient states by birth round and check items 3
// and 4 directly by scanning the real edges of the converged graph.

#include <cstdio>
#include <map>
#include <set>
#include <vector>

#include "ravel/class_ii_neighbor_family.hpp"
#include "ravel/contact_boundary.hpp"
#include "ravel/d_cont_check.hpp"
#include "ravel/spectral.hpp"
#include "ravel/substitution.hpp"
#include "ravel/substitution_neighborhood.hpp"
#include "ravel/survey.hpp"
#include "ravel/proof/finite_graph_correspondence.hpp"

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

void run(long long a) {
    const auto center = class_ii(static_cast<std::size_t>(a));
    const auto spectral = classify_matrix_spectral(incidence(center));
    const auto neighbors = adjacent_swap_neighbors(center);
    const auto& tau = neighbors[2].substitution;

    const Substitution<3> subst = make_substitution<3>(
        SubstitutionRule(tau), spectral.beta);
    const auto seeds = search_D_cont<3>(subst, 2);
    std::vector<ANode<3>> d_cont;
    for (const auto& s : seeds) d_cont.push_back(ANode<3>{s.i, s.x, s.j});
    const auto pre_contact = backward_closure<3>(subst, d_cont);
    const auto induced = induced_restricted_edges<3>(subst, pre_contact);
    const std::set<ANode<3>> pre_set(pre_contact.begin(), pre_contact.end());
    const auto reduced = red_anode<3>(pre_set, induced);
    std::set<SNode<3>> contact;
    for (const auto& n : reduced.first) contact.insert(to_simple<3>(n));

    const auto trace = algorithm2_trace<3>(
        subst, contact, CoronaConnectorPolicy::fixed_signed_contact,
        static_cast<int>(a) + 4, 0, CoronaEdgeArithmetic::exact_rational);

    // Birth round: first layer (1-indexed) containing the state.
    std::map<SNode<3>, int> birth_round;
    for (std::size_t li = 0; li < trace.layers.size(); ++li) {
        for (const auto& n : trace.layers[li].nodes) {
            birth_round.try_emplace(n, static_cast<int>(li) + 1);
        }
    }
    const auto& final_nodes = trace.layers.back().nodes;

    // Recurrent blocks: rank 0..a for neighbor 2.
    std::vector<std::set<SNode<3>>> recurrent_sets;
    std::vector<int> block_birth_round;      // -1 if inconsistent
    std::vector<std::set<int>> block_birth_rounds_seen;
    for (long long rank = 0; rank <= a; ++rank) {
        auto expected = class_ii_neighbor_recurrent_component_states(
            2, a, static_cast<std::size_t>(rank));
        std::set<int> rounds_seen;
        for (const auto& s : expected) {
            auto it = birth_round.find(s);
            if (it != birth_round.end()) rounds_seen.insert(it->second);
        }
        recurrent_sets.push_back(std::move(expected));
        block_birth_rounds_seen.push_back(rounds_seen);
        block_birth_round.push_back(
            rounds_seen.size() == 1 ? *rounds_seen.begin() : -1);
    }

    std::size_t clean_blocks = 0;
    for (std::size_t i = 0; i < block_birth_round.size(); ++i)
        if (block_birth_round[i] != -1) ++clean_blocks;

    std::printf(
        "STRATA,a=%lld,rounds=%zu,final=%zu,blocks=%zu,clean_birth_round=%zu/%zu\n",
        a, trace.layers.size(), final_nodes.size(), recurrent_sets.size(),
        clean_blocks, recurrent_sets.size());

    for (std::size_t i = 0; i < block_birth_round.size(); ++i) {
        if (block_birth_round[i] == -1) {
            std::printf(
                "  MESSY_BLOCK,a=%lld,rank=%zu,rounds_seen=%zu\n", a, i,
                block_birth_rounds_seen[i].size());
        } else {
            std::printf(
                "  BLOCK,a=%lld,rank=%zu,birth_round=%d\n", a, i,
                block_birth_round[i]);
        }
    }

    if (clean_blocks != recurrent_sets.size()) {
        std::printf(
            "  STOP,a=%lld,reason=inconsistent_block_birth_rounds -- "
            "items 3/4 scan skipped, needs a different stratification\n",
            a);
        return;
    }

    // Transient states, grouped by birth round.
    std::set<SNode<3>> recurrent_union;
    for (const auto& block : recurrent_sets)
        recurrent_union.insert(block.begin(), block.end());
    std::map<int, std::vector<SNode<3>>> transient_by_round;
    for (const auto& s : final_nodes) {
        if (recurrent_union.count(s)) continue;
        transient_by_round[birth_round.at(s)].push_back(s);
    }

    // Map each recurrent-union state to its block's birth round (for
    // item 4's "earlier than the block" comparison).
    std::map<SNode<3>, int> recurrent_state_block_round;
    for (std::size_t i = 0; i < recurrent_sets.size(); ++i)
        for (const auto& s : recurrent_sets[i])
            recurrent_state_block_round[s] = block_birth_round[i];

    // Real edges among final_nodes, from the trusted trace.
    const auto& edges = trace.layers.back().edges;

    // Replay the same trusted trace through the reusable stratification
    // contract. Recurrent blocks get their catalogue index; each transient
    // state is its own nonrecurrent component, since the contract checks the
    // birth-round/escape premises rather than deriving SCCs a second time.
    const std::vector<SNode<3>> final_order(
        final_nodes.begin(), final_nodes.end());
    std::map<SNode<3>, std::size_t> state_index;
    for (std::size_t i = 0; i < final_order.size(); ++i)
        state_index.emplace(final_order[i], i);
    std::map<SNode<3>, std::size_t> recurrent_block;
    for (std::size_t rank = 0; rank < recurrent_sets.size(); ++rank)
        for (const auto& state : recurrent_sets[rank])
            recurrent_block[state] = rank;
    std::vector<long long> component(final_order.size(), -1);
    std::size_t next_component = recurrent_sets.size();
    std::vector<bool> recurrent_component(next_component, true);
    std::vector<std::size_t> birth_round_vector(final_order.size());
    for (std::size_t i = 0; i < final_order.size(); ++i) {
        const auto& state = final_order[i];
        birth_round_vector[i] = static_cast<std::size_t>(birth_round.at(state));
        const auto recurrent_it = recurrent_block.find(state);
        if (recurrent_it != recurrent_block.end()) {
            component[i] = static_cast<long long>(recurrent_it->second);
        } else {
            component[i] = static_cast<long long>(next_component++);
            recurrent_component.push_back(false);
        }
    }
    std::vector<std::vector<std::size_t>> adjacency(final_order.size());
    for (const auto& [source, destination, lp, lq] : edges) {
        (void)lp; (void)lq;
        const auto source_it = state_index.find(source);
        const auto destination_it = state_index.find(destination);
        if (source_it != state_index.end() && destination_it != state_index.end())
            adjacency[source_it->second].push_back(destination_it->second);
    }
    const auto stratified =
        proof::derive_stratified_escape_certificate(
            adjacency, component, recurrent_component, birth_round_vector);
    std::printf(
        "  CONTRACT,a=%lld,valid=%d,groups=%zu,escaped=%zu,"
        "earlier_returns=%zu\n",
        a, stratified.valid ? 1 : 0, stratified.transient_groups,
        stratified.transient_groups_with_escape,
        stratified.recurrent_to_earlier_transient_edges);

    long long item4_violations = 0, item4_checked = 0;
    for (const auto& [source, dest, lp, lq] : edges) {
        (void)lp; (void)lq;
        auto rit = recurrent_state_block_round.find(source);
        if (rit == recurrent_state_block_round.end()) continue;  // source not recurrent
        auto bit = birth_round.find(dest);
        if (bit == birth_round.end()) continue;
        if (recurrent_union.count(dest)) continue;  // target recurrent, not item 4's concern
        ++item4_checked;
        if (bit->second < rit->second) ++item4_violations;
    }

    // Item 3: for every transient round-group, at least one edge that
    // escapes transience -- either to a strictly later round, or to a
    // recurrent block (even one born the same round: a transient
    // state feeding directly into the recurrent structure that forms
    // alongside it is exactly the "escape toward a later block" the
    // item wants, once "later" is read as "not transient forever"
    // rather than "strictly greater round index". A first version of
    // this check used the strict-round-only definition and found the
    // terminal-round transient group (born at round a) apparently
    // stuck with no escape; refining it here found those states DO
    // feed directly into the same-round recurrent block (6 edges,
    // 1 recurrent rank, at every tested a) -- the strict version was
    // simply too narrow, the same kind of correction as item 5's
    // "zero cross-block edges" overclaim earlier this session.
    std::vector<int> transient_rounds;
    for (const auto& [round, states] : transient_by_round)
        transient_rounds.push_back(round);
    std::map<int, bool> group_has_escape;
    for (int round : transient_rounds) group_has_escape[round] = false;
    for (const auto& [source, dest, lp, lq] : edges) {
        (void)lp; (void)lq;
        if (recurrent_union.count(source)) continue;
        auto sbit = birth_round.find(source);
        auto dbit = birth_round.find(dest);
        if (sbit == birth_round.end() || dbit == birth_round.end()) continue;
        const bool dest_recurrent = recurrent_union.count(dest) != 0;
        if (dest_recurrent) {
            group_has_escape[sbit->second] = true;
            continue;
        }
        if (dbit->second > sbit->second) group_has_escape[sbit->second] = true;
    }
    std::size_t groups_with_escape = 0;
    for (const auto& [round, has] : group_has_escape)
        if (has) ++groups_with_escape;

    std::printf(
        "  ITEM4,a=%lld,edges_from_recurrent=%lld,violations=%lld\n", a,
        item4_checked, item4_violations);
    std::printf(
        "  ITEM3,a=%lld,transient_groups=%zu,groups_with_escape=%zu\n", a,
        transient_rounds.size(), groups_with_escape);
    for (const auto& [round, has] : group_has_escape) {
        if (!has) {
            // Refine before treating this as a real gap: do these
            // states have edges into a recurrent block born the SAME
            // round (not "later" by the strict definition above, but
            // still an escape from transience into recurrence)?
            long long same_round_to_recurrent = 0;
            std::set<int> same_round_recurrent_ranks;
            for (const auto& [source, dest, lp, lq] : edges) {
                (void)lp; (void)lq;
                auto sbit = birth_round.find(source);
                if (sbit == birth_round.end() || sbit->second != round) continue;
                if (recurrent_union.count(source)) continue;
                if (!recurrent_union.count(dest)) continue;
                if (recurrent_state_block_round.at(dest) != round) continue;
                ++same_round_to_recurrent;
                for (std::size_t r = 0; r < recurrent_sets.size(); ++r)
                    if (recurrent_sets[r].count(dest))
                        same_round_recurrent_ranks.insert(static_cast<int>(r));
            }
            std::printf(
                "  NO_ESCAPE,a=%lld,round=%d,group_size=%zu,"
                "same_round_edges_to_recurrent=%lld,recurrent_ranks_hit=%zu\n",
                a, round, transient_by_round.at(round).size(),
                same_round_to_recurrent, same_round_recurrent_ranks.size());
        }
    }
}

}  // namespace

int main(int argc, char** argv) {
    const long long a_min = argc > 1 ? std::atoll(argv[1]) : 7;
    const long long a_max = argc > 2 ? std::atoll(argv[2]) : 10;
    for (long long a = a_min; a <= a_max; ++a) run(a);
    return 0;
}
