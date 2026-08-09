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

#include <algorithm>
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

inline long long checked_exact_add(long long a, long long b) {
    long long value = 0;
    if (__builtin_add_overflow(a, b, &value))
        throw std::overflow_error("coincidence closure vector addition overflow");
    return value;
}

inline long long checked_exact_mul(long long a, long long b) {
    long long value = 0;
    if (__builtin_mul_overflow(a, b, &value))
        throw std::overflow_error("coincidence closure matrix product overflow");
    return value;
}

template <std::size_t d>
inline ExactVec<d> exact_vec_add(const ExactVec<d>& a, const ExactVec<d>& b) {
    ExactVec<d> r{};
    for (std::size_t i = 0; i < d; ++i) r[i] = checked_exact_add(a[i], b[i]);
    return r;
}

template <std::size_t d>
inline ExactVec<d> exact_matvec(const std::array<std::array<long long, d>, d>& M, const ExactVec<d>& v) {
    ExactVec<d> r{};
    for (std::size_t i = 0; i < d; ++i)
        for (std::size_t j = 0; j < d; ++j)
            r[i] = checked_exact_add(r[i], checked_exact_mul(M[i][j], v[j]));
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
        std::set<long long> seen;
        while (!is_junction(letter)) {
            if (!seen.insert(letter).second)
                throw std::invalid_argument(
                    "build_junction_graph: deterministic non-junction cycle has no branching target");
            chain.push_back(letter);
            if (images[static_cast<std::size_t>(letter)].empty())
                throw std::invalid_argument(
                    "build_junction_graph: empty non-junction image has no deterministic target");
            if (images[static_cast<std::size_t>(letter)].size() != 1)
                throw std::invalid_argument(
                    "build_junction_graph: a non-junction letter must have image length exactly 1");
            letter = images[static_cast<std::size_t>(letter)][0];
            ++steps;
        }
        chain.push_back(letter);  // the junction itself, chain.back()
        return {chain, steps};
    };
    bool has_junction = false;
    for (std::size_t letter = 0; letter < d; ++letter)
        has_junction = has_junction || is_junction(static_cast<long long>(letter));
    if (!has_junction)
        throw std::invalid_argument(
            "build_junction_graph: substitution has no branching letter");
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
                    for (std::size_t k = 0; k < d; ++k)
                        s = checked_exact_add(s, checked_exact_mul(M_[i][k], prev[k][j]));
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

// A finite, exact prefix-half classifier for strong coincidence.  It uses
// the deduplicated landmark closure rather than materializing sigma^K(b), so
// its resource bound is on distinct (terminal, vector) outcomes, not on the
// exponentially growing word length.  The result is deliberately scoped to
// the PREFIX half: an unresolved pair may still have a suffix coincidence,
// and therefore is reported as inconclusive rather than as a failure.
struct PrefixClosureCoincidenceResult {
    bool holds = false;
    bool inconclusive = false;
    long long depth_reached = 0;
    long long unresolved_pairs = 0;
    std::vector<long long> pair_resolution_depths;
};

// Full finite strong-coincidence result obtained by running the exact
// closure on both sigma and its letterwise-reversed substitution.  A prefix
// coincidence for rev(sigma) is a suffix coincidence for sigma, because
// rev(sigma^k(b)) = rev(sigma)^k(b).  Thus this combines both halves without
// materializing either exponentially growing word family.
struct ClosureStrongCoincidenceResult {
    bool holds = false;
    bool inconclusive = false;
    long long depth_reached = 0;
    long long unresolved_pairs = 0;
    std::vector<long long> pair_resolution_depths;
};

template <std::size_t d>
inline std::array<std::vector<long long>, d> reverse_substitution_images(
    const std::array<std::vector<long long>, d>& images) {
    auto reversed = images;
    for (auto& image : reversed) std::reverse(image.begin(), image.end());
    return reversed;
}

template <std::size_t d>
inline PrefixClosureCoincidenceResult check_prefix_coincidence_closure(
    const std::array<std::vector<long long>, d>& images,
    const std::array<std::array<long long, d>, d>& matrix,
    long long max_depth = 20,
    std::size_t outcome_budget = 1'000'000) {
    std::vector<std::pair<long long, long long>> pairs;
    for (long long a = 0; a < static_cast<long long>(d); ++a)
        for (long long b = a + 1; b < static_cast<long long>(d); ++b)
            pairs.emplace_back(a, b);
    PrefixClosureCoincidenceResult result;
    result.pair_resolution_depths.assign(pairs.size(), -1);
    if (pairs.empty()) {
        result.holds = true;
        return result;
    }
    if (max_depth < 1) {
        result.inconclusive = true;
        result.unresolved_pairs = static_cast<long long>(pairs.size());
        return result;
    }

    const auto edges = build_junction_graph<d>(images);
    CoincidenceClosure<d> closure(edges, matrix);
    std::vector<std::pair<long long, long long>> active = pairs;
    std::vector<std::size_t> active_slots(pairs.size());
    for (std::size_t i = 0; i < active_slots.size(); ++i) active_slots[i] = i;

    auto outcomes_from_start = [&](long long start, long long depth) {
        OutcomeSet<d> outcomes;
        long long letter = start;
        long long remaining = depth;
        while (remaining > 0 && images[static_cast<std::size_t>(letter)].size() == 1) {
            letter = images[static_cast<std::size_t>(letter)][0];
            --remaining;
        }
        if (remaining == 0) {
            outcomes.insert({letter, ExactVec<d>{}});
            return outcomes;
        }
        if (images[static_cast<std::size_t>(letter)].size() < 2)
            throw std::invalid_argument(
                "check_prefix_coincidence_closure: empty-image or invalid deterministic chain");
        auto const& reachable = closure.reachable(letter, remaining);
        outcomes.insert(reachable.begin(), reachable.end());
        return outcomes;
    };

    for (long long depth = 1; depth <= max_depth; ++depth) {
        std::map<long long, OutcomeSet<d>> by_start;
        std::size_t total_outcomes = 0;
        for (const auto& pair : active) {
            for (long long start : {pair.first, pair.second}) {
                if (!by_start.count(start)) {
                    by_start.emplace(start, outcomes_from_start(start, depth));
                    total_outcomes += by_start.at(start).size();
                }
            }
        }
        if (total_outcomes > outcome_budget) {
            result.depth_reached = depth;
            result.inconclusive = true;
            result.unresolved_pairs = static_cast<long long>(active.size());
            return result;
        }
        std::vector<std::pair<long long, long long>> still_active;
        std::vector<std::size_t> still_slots;
        for (std::size_t i = 0; i < active.size(); ++i) {
            const auto& left = by_start.at(active[i].first);
            const auto& right = by_start.at(active[i].second);
            bool coincides = false;
            for (const auto& state : left) {
                if (right.count(state)) { coincides = true; break; }
            }
            if (coincides) {
                result.pair_resolution_depths[active_slots[i]] = depth;
            } else {
                still_active.push_back(active[i]);
                still_slots.push_back(active_slots[i]);
            }
        }
        active = std::move(still_active);
        active_slots = std::move(still_slots);
        result.depth_reached = depth;
        if (active.empty()) {
            result.holds = true;
            return result;
        }
    }
    result.inconclusive = true;
    result.unresolved_pairs = static_cast<long long>(active.size());
    return result;
}

template <std::size_t d>
inline ClosureStrongCoincidenceResult check_strong_coincidence_closure(
    const std::array<std::vector<long long>, d>& images,
    const std::array<std::array<long long, d>, d>& matrix,
    long long max_depth = 20,
    std::size_t outcome_budget = 1'000'000) {
    const auto prefix = check_prefix_coincidence_closure<d>(
        images, matrix, max_depth, outcome_budget);
    const auto suffix = check_prefix_coincidence_closure<d>(
        reverse_substitution_images<d>(images), matrix, max_depth, outcome_budget);
    ClosureStrongCoincidenceResult result;
    result.pair_resolution_depths.resize(prefix.pair_resolution_depths.size(), -1);
    for (std::size_t i = 0; i < result.pair_resolution_depths.size(); ++i) {
        const long long p = prefix.pair_resolution_depths[i];
        const long long s = suffix.pair_resolution_depths[i];
        if (p > 0 && s > 0) result.pair_resolution_depths[i] = std::min(p, s);
        else if (p > 0) result.pair_resolution_depths[i] = p;
        else if (s > 0) result.pair_resolution_depths[i] = s;
    }
    result.depth_reached = std::max(prefix.depth_reached, suffix.depth_reached);
    for (long long depth : result.pair_resolution_depths)
        if (depth < 0) ++result.unresolved_pairs;
    result.holds = result.unresolved_pairs == 0;
    result.inconclusive = !result.holds;
    return result;
}

}  // namespace ravel::proof
