// include/adelic/csy_finite_carry_automaton.hpp
//
// CSY Theorem 3 — finite Myhill-Nerode quotient automaton for
// Pisot numerations, closing the open seam flagged in finding A4 of
// docs/RECOVERY_AUDIT_2026-07-29.md.
//
// THEOREM 3 (Lot02 chapter 7 citation, restated in CSY 2026):
//   For any Pisot numeration U and any finite alphabet B ⊂ Z,
//   the set K_B := {w ∈ B* : [w]_U = 0} is regular — accepted by
//   a deterministic finite automaton over B.
//
// LEMMA 43 (CSY 2026):
//   Let U be a Pisot numeration, g a finite sequence, and set
//   B = {−‖g‖_∞, …, ‖g‖_∞}.  Let N = N(U, ‖g‖_∞) be the number
//   of states of the automaton constructed in Theorem 3.  Then
//   for any n ≥ N, if g_0 g_1 … g_n satisfies [g_0^n]_U = 0 and
//   g_{k+1} = g_k = 0 for k ≥ n, then g ∈ H_U, i.e. [g]_U = 0.
//
// What this header actually builds, and what it does NOT claim:
//
//   Phase 1 (reachability) is the exact same bounded frontier-only
//   BFS as CSYAutomaton in csy_carry_automaton.hpp: states are
//   (value, position) pairs, deduplicated by literal Q(β) equality,
//   explored up to `max_prefix_length` and pruned by `bound_bits`.
//   That reachability strategy is already tested and known not to
//   explode (see csy_carry_automaton_test.cpp, tens of thousands of
//   states at max_pos=16 without incident).
//
//   Phase 2 (minimization) runs Moore-style partition refinement
//   over that reachable graph: states are merged whenever they have
//   identical accept/reject status and, for every digit, transition
//   into the same class.  This is standard DFA minimization and is
//   polynomial in the number of *reachable* nodes -- it never
//   enumerates literal suffixes or re-expands already-processed
//   prefixes, which is what made the previous version of this file
//   exponential (unbounded prefix reprocessing) and made it OOM the
//   host machine at any depth beyond a handful of digits.
//
//   The result IS a genuine Myhill-Nerode quotient of the *bounded*
//   prefix language (words reachable within max_prefix_length and
//   bound_bits) -- but that is a claim about the explored budget,
//   not about the unbounded language K_B.  Whether the bounded
//   quotient's state count stabilizes as max_prefix_length grows
//   (which is what would let N stop depending on the exploration
//   budget and become the genuine CSY Theorem 3 bound) remains the
//   open question tracked in docs/RECOVERY_AUDIT_2026-07-29.md and
//   docs/READINGLIST.next.session.md.  Use `truncated` below to see
//   whether reachability actually closed before hitting the budget.
//
// Why the existing prefix-tree CSYAutomaton in
// include/adelic/csy_carry_automaton.hpp is NOT the same object:
//   That header records (digit_value, absolute_position) for each
//   visited state and reports truncation at `max_pos`, with no
//   merging at all -- its state count grows with position even when
//   two states would behave identically forever after.  The
//   automaton built here reuses that same reachable graph but then
//   collapses it to its minimal quotient, so its state count can
//   only be smaller, never larger.
//
// Reference (paper text on disk):
//   refs/arXiv_2606.30496_CartonSudberyYassawi2026_
//         Pisot-numerations-topological-groups.txt
//
// All arithmetic is exact in mathlib::QElem / mathlib::QBetaRing.

#pragma once

#include <algorithm>
#include <cstddef>
#include <limits>
#include <map>
#include <vector>

#include "adelic/csy_carry_automaton.hpp"

namespace adelic {

using mathlib::QElem;

// ===================================================================
// Finite carry automaton for {w ∈ B* : [w]_U = 0}, built as the
// Moore-minimized quotient of the bounded reachable (value,
// position) graph.  See file header for exactly what this proves
// and what it leaves open.
//
// Performance: minimize() is naive (whole-array) Moore refinement,
// not Hopcroft's O(n log n) worklist algorithm.  Measured behaviour
// (tribonacci, alphabet size 5, bound_bits=14): refinement_rounds
// tracks max_prefix_length + 1, not the reachable node count, so
// total cost is O(max_prefix_length * raw_node_count * |alphabet|).
// raw_node_count itself grows roughly geometrically in
// max_prefix_length until bound_bits pruning dominates. This is
// genuine necessary work for the chosen exploration budget, not a
// reprocessing bug like the one this file replaced -- but if a
// future caller wants much larger max_prefix_length, replacing
// minimize() with worklist-based (Hopcroft-style) refinement is the
// next real optimization, not a bug fix.
// ===================================================================
class CSYZeroAutomaton {
public:
    static constexpr std::size_t kSink = std::numeric_limits<std::size_t>::max();
    static constexpr long long kSinkClass = -1;

    PisotPoly poly;
    std::vector<long long> alphabet;       // sorted, distinct
    std::size_t bound_bits = 0;
    std::size_t max_prefix_length = 0;

    // True if the reachable-node BFS still had live states sitting
    // exactly at max_prefix_length when it stopped (mirroring
    // CSYAutomaton::closed_at_bound_): those states always have a
    // valid "extend by 0" successor, so stopping there is a real
    // truncation of the explored language, not a natural closure.
    bool truncated = false;

    std::size_t build(const PisotPoly& P, const std::vector<long long>& B,
                       std::size_t max_prefix_length_ = 16,
                       std::size_t bound_bits_ = (1U << 16)) {
        poly = P;
        alphabet = B;
        std::sort(alphabet.begin(), alphabet.end());
        alphabet.erase(std::unique(alphabet.begin(), alphabet.end()),
                       alphabet.end());
        max_prefix_length = max_prefix_length_;
        bound_bits = bound_bits_;

        reachability_bfs();
        minimize();
        return num_classes_;
    }

    bool accepts(const std::vector<long long>& w) const {
        long long c = start_class_;
        for (long long d : w) {
            if (c == kSinkClass) return false;
            auto it = std::lower_bound(alphabet.begin(), alphabet.end(), d);
            if (it == alphabet.end() || *it != d) return false;
            std::size_t di = static_cast<std::size_t>(it - alphabet.begin());
            c = class_transition_[static_cast<std::size_t>(c)][di];
        }
        return c != kSinkClass
            && class_accept_[static_cast<std::size_t>(c)];
    }

    // Minimized (Myhill-Nerode-quotient) state count -- the N used
    // for Lemma 43's bounded-correction length.
    std::size_t state_count() const { return num_classes_; }
    // Raw reachable (value, position) node count before
    // minimization, kept for diagnostics/comparison.
    std::size_t raw_node_count() const { return node_value_.size(); }
    std::size_t transition_count() const {
        std::size_t n = 0;
        for (const auto& row : class_transition_)
            for (long long t : row)
                if (t != kSinkClass) ++n;
        return n;
    }
    std::size_t max_visited_position = 0;
    // Number of Moore refinement rounds actually needed to reach
    // the fixpoint (diagnostic -- see minimize()).
    std::size_t refinement_rounds = 0;

private:
    // Reachable-graph storage (phase 1).
    std::map<AutoState, std::size_t> node_id_;
    std::vector<QElem> node_value_;
    std::vector<std::size_t> node_pos_;
    std::vector<std::vector<std::size_t>> node_transition_;  // [node][digit] -> node id or kSink

    // Minimized-DFA storage (phase 2).
    std::vector<std::vector<long long>> class_transition_;   // [class][digit] -> class id or kSinkClass
    std::vector<bool> class_accept_;
    std::size_t num_classes_ = 0;
    long long start_class_ = kSinkClass;

    std::size_t add_node(const AutoState& st) {
        auto it = node_id_.find(st);
        if (it != node_id_.end()) return it->second;
        std::size_t id = node_value_.size();
        node_id_.emplace(st, id);
        node_value_.push_back(st.v);
        node_pos_.push_back(st.pos);
        node_transition_.emplace_back(alphabet.size(), kSink);
        return id;
    }

    // Frontier-only BFS: each round expands only the nodes newly
    // discovered in the previous round.  This is the fix for the
    // original bug, which re-expanded the entire accumulated
    // history every round and blew up memory exponentially.
    void reachability_bfs() {
        node_id_.clear();
        node_value_.clear();
        node_pos_.clear();
        node_transition_.clear();
        max_visited_position = 0;
        truncated = false;

        const PisotContext& ctx = pisotContextFor(poly);
        AutoState init{ctx.ring.zero(), 0};
        add_node(init);
        std::vector<AutoState> frontier = {init};

        while (!frontier.empty()) {
            std::vector<AutoState> next;
            for (const auto& st : frontier) {
                const std::size_t src_id = node_id_[st];
                if (st.pos > max_visited_position)
                    max_visited_position = st.pos;
                if (st.pos >= max_prefix_length) {
                    // Zero is always a legal digit and preserves
                    // st.v, so this state always has a further
                    // successor; stopping here is truncation.
                    truncated = true;
                    continue;
                }
                QElem b = ctx.betaPowerMemoised(st.pos);
                for (std::size_t di = 0; di < alphabet.size(); ++di) {
                    QElem term = ctx.ring.mul(
                        ctx.ring.from_int(alphabet[di]), b);
                    QElem nv = ctx.ring.add(st.v, term);
                    if (!qBetaCoefficientsBounded(nv, bound_bits)) continue;
                    AutoState ns{nv, st.pos + 1};
                    const bool is_new = !node_id_.count(ns);
                    const std::size_t dst_id = add_node(ns);
                    node_transition_[src_id][di] = dst_id;
                    if (is_new) next.push_back(ns);
                }
            }
            frontier = std::move(next);
        }
    }

    // Moore partition refinement: repeatedly split classes whose
    // members disagree on (accept status, per-digit destination
    // class).  Monotonic in the number of classes, so a round that
    // does not increase the class count is a fixpoint; terminates
    // in at most raw_node_count() rounds.
    void minimize() {
        const std::size_t n = node_value_.size();
        std::vector<std::size_t> class_of(n);
        for (std::size_t i = 0; i < n; ++i)
            class_of[i] = node_value_[i].is_zero() ? 1 : 0;
        std::size_t num_classes = std::min<std::size_t>(2, n);
        // If every node happens to share one accept status, collapse
        // the unused initial class immediately.
        {
            bool any0 = false, any1 = false;
            for (std::size_t i = 0; i < n; ++i)
                (class_of[i] == 0 ? any0 : any1) = true;
            if (!any0 || !any1) {
                for (auto& c : class_of) c = 0;
                num_classes = n > 0 ? 1 : 0;
            }
        }

        refinement_rounds = 0;
        for (std::size_t round = 0; round <= n; ++round) {
            ++refinement_rounds;
            std::map<std::vector<long long>, std::size_t> sig_to_class;
            std::vector<std::size_t> new_class(n);
            for (std::size_t i = 0; i < n; ++i) {
                std::vector<long long> sig;
                sig.reserve(alphabet.size() + 1);
                sig.push_back(static_cast<long long>(class_of[i]));
                for (std::size_t di = 0; di < alphabet.size(); ++di) {
                    const std::size_t t = node_transition_[i][di];
                    sig.push_back(t == kSink
                        ? kSinkClass
                        : static_cast<long long>(class_of[t]));
                }
                auto it = sig_to_class.find(sig);
                if (it == sig_to_class.end()) {
                    const std::size_t c = sig_to_class.size();
                    sig_to_class.emplace(std::move(sig), c);
                    new_class[i] = c;
                } else {
                    new_class[i] = it->second;
                }
            }
            const std::size_t new_num_classes = sig_to_class.size();
            class_of = std::move(new_class);
            const bool fixpoint = (new_num_classes == num_classes);
            num_classes = new_num_classes;
            if (fixpoint) break;
        }

        num_classes_ = num_classes;
        class_transition_.assign(num_classes,
            std::vector<long long>(alphabet.size(), kSinkClass));
        class_accept_.assign(num_classes, false);
        std::vector<bool> filled(num_classes, false);
        for (std::size_t i = 0; i < n; ++i) {
            const std::size_t c = class_of[i];
            if (filled[c]) continue;
            filled[c] = true;
            class_accept_[c] = node_value_[i].is_zero();
            for (std::size_t di = 0; di < alphabet.size(); ++di) {
                const std::size_t t = node_transition_[i][di];
                class_transition_[c][di] = (t == kSink)
                    ? kSinkClass
                    : static_cast<long long>(class_of[t]);
            }
        }
        start_class_ = n == 0
            ? kSinkClass
            : static_cast<long long>(class_of[node_id_[AutoState{
                  pisotContextFor(poly).ring.zero(), 0}]]);
    }
};

// ===================================================================
// Bounded-correction bound N(U, ‖g‖∞) — Lemma 43 of CSY.
//
// The minimized state count from CSYZeroAutomaton is the
// a-posteriori upper bound on the bounded-correction length within
// the explored budget: the pumping lemma applied to the minimized
// DFA guarantees that any accepted word longer than state_count()
// letters revisits a class, so state_count() bounds the shortest
// non-pumpable witness at this exploration budget.
// ===================================================================
struct CSYBoundedCorrection {
    std::size_t state_count;       // minimized (Myhill-Nerode) count
    std::size_t raw_node_count;    // pre-minimization reachable count
    std::size_t transition_count;
    std::size_t max_visited_position;
    bool truncated;
    long long digit_bound;

    std::size_t N_upper_bound() const { return state_count; }
};

inline CSYBoundedCorrection csy_bounded_correction(
    const PisotPoly& P, const std::vector<long long>& B,
    std::size_t max_prefix_length = 16,
    std::size_t bound_bits = (1U << 16)) {
    CSYZeroAutomaton aut;
    aut.build(P, B, max_prefix_length, bound_bits);
    long long bound = 0;
    for (long long d : B) {
        if (std::abs(d) > bound) bound = std::abs(d);
    }
    return CSYBoundedCorrection{
        aut.state_count(),
        aut.raw_node_count(),
        aut.transition_count(),
        aut.max_visited_position,
        aut.truncated,
        bound,
    };
}

}  // namespace adelic
