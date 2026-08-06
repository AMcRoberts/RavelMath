// ravel/proof/coincidence_closure.hpp
//
// Replaces raw word materialization (which scales with the
// exponentially-growing word length) with an exact, deduped closure
// computation over (terminal_letter, landmark_vector) states -- the
// same architectural pattern this project's corona.hpp already uses
// for contact-boundary closure (exact integer vectors, deduped via a
// set, grown to a fixed point or a cap), applied here to the
// coincidence question. Built directly on
// ravel/generic_memoized_dag_closure.hpp, the shared contract factored
// out of that pattern -- see that header's own comment for why corona
// and property-(F)'s closures are NOT retrofitted onto it (both have
// genuinely different shapes: cyclic state spaces, round-level pruning)
// while this one's (junction, remaining_depth) state space really is
// an acyclic DAG and fits the base honestly.
//
// Generalizes Finding 23's single-junction jump model
// (single_junction_coincidence_composition.hpp) to arbitrary
// substitutions with any number of junction letters (image length >= 2),
// and generalizes Finding 22's landmark-vector formula
// (coincidence_as_landmark_vector_cancellation.hpp) from full-vector
// abelianization to a MEMOIZED reachability closure, so depths far
// beyond what raw materialization can reach become tractable -- as
// long as the number of DISTINCT achievable vectors stays bounded,
// which is exactly the same finiteness bet corona.hpp's own closure
// makes for the contact-boundary graph.
//
// KEY FACT (this is what makes the DP well-founded): the weight
// M^{depth_here-1} applied to a landmark event depends ONLY on the
// REMAINING depth at the moment the choice is made, not on the total
// depth K the walk will eventually have. So reachable(junction,
// remaining_depth) can be computed purely bottom-up from
// remaining_depth=0, without needing to fix K in advance.
//
// A real bug caught and fixed while building this (not found by
// inspection -- by re-deriving the mid-chain-cutoff case carefully
// after noticing the first version silently mislabeled it): when a
// walk runs out of depth PARTWAY through a chosen edge's forced chain,
// the terminal letter is NOT that edge's eventual target junction --
// it is whichever letter the chain has reached at the exact point the
// budget ran out. The first version of this file used the edge's
// target junction directly in that case, which is wrong whenever the
// chain has length > 1 and the cutoff lands before its end. Fixed by
// storing the full chain on each edge and indexing into it directly.

#pragma once

#include <array>
#include <cstddef>
#include <map>
#include <set>
#include <stdexcept>
#include <vector>

#include "ravel/generic_memoized_dag_closure.hpp"

namespace ravel::proof {

template <std::size_t d>
using ExactVec = std::array<long long, d>;

template <std::size_t d>
inline ExactVec<d> exact_vec_add(const ExactVec<d>& a, const ExactVec<d>& b) {
    ExactVec<d> r{};
    for (std::size_t i = 0; i < d; ++i) r[i] = a[i] + b[i];
    return r;
}

template <std::size_t d>
inline ExactVec<d> exact_matvec(const std::array<std::array<long long, d>, d>& M, const ExactVec<d>& v) {
    ExactVec<d> r{};
    for (std::size_t i = 0; i < d; ++i)
        for (std::size_t j = 0; j < d; ++j) r[i] += M[i][j] * v[j];
    return r;
}

// One edge in the general (possibly multi-junction) jump graph: from a
// junction letter, choosing child index `child_index` consumes
// `jump_size` levels, contributes `landmark` (the abelianization of the
// siblings before that child), and arrives at `to_junction` (which may
// equal `from_junction`, or be a different junction).
template <std::size_t d>
struct JunctionEdge {
    long long from_junction, to_junction, jump_size, child_index;
    ExactVec<d> landmark;
    // chain[0] = the chosen child itself; chain.back() == to_junction.
    // Length == jump_size - 1 (excludes the choice step itself, which
    // is accounted for by `jump_size`, but includes every forced letter
    // visited afterward). Needed to resolve mid-chain-cutoff terminals
    // exactly -- see the header comment above.
    std::vector<long long> chain;
};

// Builds the full multi-junction graph directly from the substitution's
// images -- no assumption of a single junction letter. A letter is a
// junction iff its image has length >= 2. Non-junction letters must
// form deterministic chains (each image length exactly 1) eventually
// reaching some junction letter; this is asserted, not silently assumed.
template <std::size_t d>
inline std::vector<JunctionEdge<d>> build_junction_graph(const std::array<std::vector<long long>, d>& images) {
    auto is_junction = [&](long long letter) { return images[static_cast<std::size_t>(letter)].size() >= 2; };
    auto follow_to_junction = [&](long long letter) -> std::pair<std::vector<long long>, long long> {
        std::vector<long long> chain;
        long long steps = 0;
        while (!is_junction(letter)) {
            chain.push_back(letter);
            if (images[static_cast<std::size_t>(letter)].size() != 1) {
                throw std::runtime_error("build_junction_graph: a non-junction letter must have image length exactly 1");
            }
            letter = images[static_cast<std::size_t>(letter)][0];
            ++steps;
            if (steps > 10000) throw std::runtime_error("build_junction_graph: chain never reaches a junction (not irreducible?)");
        }
        chain.push_back(letter);  // the junction itself, chain.back()
        return {chain, steps};
    };
    std::vector<JunctionEdge<d>> edges;
    for (long long j = 0; j < static_cast<long long>(d); ++j) {
        if (!is_junction(j)) continue;
        const auto& img = images[static_cast<std::size_t>(j)];
        ExactVec<d> prefix{};
        for (std::size_t c = 0; c < img.size(); ++c) {
            long long child = img[c];
            auto [chain, steps] = follow_to_junction(child);
            edges.push_back({j, chain.back(), 1 + steps, static_cast<long long>(c), prefix, chain});
            prefix[static_cast<std::size_t>(child)] += 1;
        }
    }
    return edges;
}

// The per-state outcome set: (terminal_letter, exact accumulated vector)
// pairs achievable by a walk starting at a junction with some remaining
// depth budget.
template <std::size_t d>
using OutcomeSet = std::set<std::pair<long long, ExactVec<d>>>;

// Thin wrapper: builds the (junction, remaining_depth) DAG closure on
// top of MemoizedDagClosure, handling the M^{power}-weighting and the
// mid-chain-cutoff terminal-letter lookup.
template <std::size_t d>
class CoincidenceClosure {
public:
    CoincidenceClosure(std::vector<JunctionEdge<d>> edges, std::array<std::array<long long, d>, d> M)
        : edges_(std::move(edges)), M_(M),
          closure_([this](const std::pair<long long, long long>& state,
                           std::function<const OutcomeSet<d>&(const std::pair<long long, long long>&)> recurse) {
              return this->compute(state, recurse);
          }) {}

    const OutcomeSet<d>& reachable(long long junction, long long remaining_depth) {
        return closure_.get({junction, remaining_depth});
    }

private:
    OutcomeSet<d> compute(const std::pair<long long, long long>& state,
                           const std::function<const OutcomeSet<d>&(const std::pair<long long, long long>&)>& recurse) {
        auto [junction, remaining_depth] = state;
        OutcomeSet<d> result;
        if (remaining_depth <= 0) {
            result.insert({junction, ExactVec<d>{}});
            return result;
        }
        for (const auto& e : edges_) {
            if (e.from_junction != junction) continue;
            ExactVec<d> weighted = exact_matvec<d>(power_cache(remaining_depth - 1), e.landmark);
            if (e.jump_size <= remaining_depth) {
                for (const auto& [term, subvec] : recurse({e.to_junction, remaining_depth - e.jump_size})) {
                    result.insert({term, exact_vec_add<d>(weighted, subvec)});
                }
            } else {
                // Mid-chain cutoff: the choice (jump_size-1-th step is
                // never reached) still commits its landmark, but the
                // walk stops at chain[remaining_depth - 1], not at
                // e.to_junction. remaining_depth-1 is a valid index into
                // e.chain because e.jump_size > remaining_depth implies
                // remaining_depth - 1 < e.jump_size - 1 == e.chain.size() - 1...
                // (chain.size() == steps+1 == jump_size, so index
                // remaining_depth-1 < jump_size-1 < chain.size()).
                long long idx = remaining_depth - 1;
                long long terminal = e.chain[static_cast<std::size_t>(idx)];
                result.insert({terminal, weighted});
            }
        }
        return result;
    }

    const std::array<std::array<long long, d>, d>& power_cache(long long power) {
        auto it = power_memo_.find(power);
        if (it != power_memo_.end()) return it->second;
        std::array<std::array<long long, d>, d> result{};
        if (power == 0) {
            for (std::size_t i = 0; i < d; ++i) result[i][i] = 1;
        } else {
            const auto& prev = power_cache(power - 1);
            for (std::size_t i = 0; i < d; ++i)
                for (std::size_t j = 0; j < d; ++j) {
                    long long s = 0;
                    for (std::size_t k = 0; k < d; ++k) s += M_[i][k] * prev[k][j];
                    result[i][j] = s;
                }
        }
        return power_memo_[power] = result;
    }

    std::vector<JunctionEdge<d>> edges_;
    std::array<std::array<long long, d>, d> M_{};
    std::map<long long, std::array<std::array<long long, d>, d>> power_memo_;
    ravel::MemoizedDagClosure<std::pair<long long, long long>, OutcomeSet<d>> closure_;
};

}  // namespace ravel::proof
