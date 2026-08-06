// ravel/proof/property_f_correct_cycle_criterion.hpp
//
// SUPERSEDED / KEPT AS A DOCUMENTED FAILED ATTEMPT -- do not use this
// function. It gives a FALSE "property F FAILS" verdict on Fibonacci
// (sigma(0)=01, sigma(1)=0), contradicting Rauzy's classical result.
// Root cause: enumerating "simple cycles" directly in the small
// letter-level prefix automaton OVER-GENERATES -- it counts cycles
// that are algebraically self-consistent (satisfy the fixed-point
// equation and the geometric bound M) but are never actually reached
// by any genuine walk starting from the zero frontier U. Confirmed
// directly: the real, trusted `check_property_f` BFS-from-U closure
// for Fibonacci has exactly 8 nodes and NEVER includes the gamma=beta
// value this header's cycle enumeration finds on the 0->1->0 cycle,
// even though that value satisfies both the algebraic recursion and
// the bound.
//
// THE ACTUAL FIX (much smaller, and does not require abandoning the
// BFS-from-U closure at all) lives directly in
// `include/adelic/coincidence_and_property_f.hpp`'s `check_property_f`
// -- see its 2026-08-06 CORRECTION comment. The BFS-from-U graph was
// never wrong; only the verdict extracted from it was (it required a
// cycle to touch BOTH zero and nonzero nodes; the correct requirement,
// given property_f_unconditional.hpp's proof that mixed cycles are
// impossible, is simply "touches any nonzero node at all"). That one-
// line change reproduces every historical ESTABLISHED verdict this
// project has on record exactly (same node counts), including
// Fibonacci at 8 nodes -- see tests/property_f_correct_verdict_test.cpp.
//
// This file is kept, not deleted, as a record of the failed attempt
// and why it failed -- per this project's own discipline of leaving
// falsified hypotheses visible rather than erasing the trail.
//
// ORIGINAL (now-superseded) header comment follows:
//
// A CORRECTION, found by reading the primary source directly
// (Minervino-Thuswaldner, "The geometry of non-unit Pisot
// substitutions," Ann. Inst. Fourier 64 (2014), Definition 9.3 /
// Definition 9.6 / Proposition 9.7 / Lemma 9.8) against what this
// project's `adelic::check_property_f` actually checks.
//
// WHAT THE PAPER ACTUALLY SAYS (verbatim structure, not paraphrase):
// property (F) holds iff the iterates of T_ext^{-1} starting from
// U = {(0,a) : a in alphabet} eventually cover ALL of Gamma (the
// full "self-replicating translation set", an independently-defined,
// invariant-under-T_ext^{-1} Delone set -- Definition 9.3). The
// zero-expansion graph G^(0) (Definition 9.6) has as its nodes EVERY
// (gamma,a) in Gamma with ||gamma||<=M -- NOT just nodes reachable
// from U -- subject to the requirement that every node supports an
// INFINITE walk. Lemma 9.8's proof shows: property (F) FAILS iff
// there exists a cycle in G^(0) whose nodes are ALL nonzero (i.e. a
// cycle that never touches a (0,a)-type node at all) -- constructed
// via a BACKWARD/predecessor chain from a hypothetical uncovered
// point, using Gamma's own self-invariance (every element of Gamma
// has a predecessor also in Gamma), forced to cycle by pigeonhole
// since Gamma has only finitely many bounded-norm elements.
//
// WHAT THIS CODEBASE'S check_property_f ACTUALLY CHECKS: it builds
// ONLY the forward BFS closure starting from U (never anything not
// reachable from U), then flags a violation only for a cycle
// containing BOTH a zero AND a nonzero node ("mixed"). This differs
// from the paper's actual criterion in two ways: (1) it can never see
// any structure not reachable from U at all, so it cannot detect
// Gamma having an entirely separate, U-unreachable region; (2) even
// restricted to what it does see, "mixed cycle" is not the paper's
// failure condition -- an ALL-NONZERO cycle is. Given
// property_f_unconditional.hpp already proves NO mixed cycle can ever
// occur (Perron-positivity), the existing check was verifying a
// condition that is unconditionally true and therefore never actually
// tests anything -- see the diary entry accompanying this header for
// the full account of finding this by reading the primary source.
//
// THE CORRECT, TRACTABLE REFORMULATION (derived here, not from the
// paper directly, though consistent with it -- see the accompanying
// diary entry for the derivation): property (F) holds if and only if
// EVERY SIMPLE CYCLE in the small, purely combinatorial prefix
// automaton has periodic translation value EXACTLY ZERO. Proof
// sketch: (a) a nonzero periodic gamma on some simple cycle is
// directly a nonzero-only cycle in G^(0) -- failure, by Lemma 9.8;
// (b) if EVERY simple cycle has periodic gamma = 0, then since the
// recursion gamma_{k+1} = beta^{-1}(gamma_k + delta(p_k)) maps 0 to 0
// along any simple cycle, it maps 0 to 0 along ANY closed walk built
// by concatenating simple cycles (composition of maps that each fix
// 0 still fixes 0) -- and since a bi-infinite walk in a finite graph
// is eventually periodic in its "deep past" (entering some simple
// cycle), every element of Gamma traces back through a genuine
// gamma=0 point, i.e. is reachable from U. This sidesteps ever having
// to construct Gamma independently or run an unbounded/budgeted BFS
// at all -- it is a small, finite, exact enumeration over the
// automaton's own combinatorial cycles.
//
// This is checked below against the SAME 8 historical stress-test
// cases used for property_f_unconditional.hpp, to see whether any
// verdict actually changes under the corrected criterion.

#pragma once

#include <algorithm>
#include <cstddef>
#include <functional>
#include <set>
#include <string>
#include <vector>

#include "math/bezout.hpp"
#include "math/qbeta.hpp"
#include "adelic/coincidence_and_property_f.hpp"
#include "adelic/prefix_automaton.hpp"

namespace ravel::proof {

struct PropertyFCycleCriterionResult {
    bool holds{};
    std::size_t simple_cycles_checked{};
    std::vector<std::vector<long long>> nonzero_cycle_letters;  // witnesses, if any
    std::string note;
};

namespace property_f_cycle_detail {

inline mathlib::QElem qpow(const mathlib::QElem& base, std::size_t k, const mathlib::QBetaRing& R) {
    mathlib::QElem r = R.one();
    for (std::size_t i = 0; i < k; ++i) r = R.mul(r, base);
    return r;
}

}  // namespace property_f_cycle_detail

// Enumerates every simple cycle in the gamma-graph (edge a->b, using
// automaton edge source=b,target=a,prefix=p, exactly matching
// T_ext^{-1}'s own direction), computes each one's exact periodic
// gamma value in Q(beta), and checks it against zero.
template <std::size_t d>
inline PropertyFCycleCriterionResult check_property_f_via_cycle_criterion(
    const adelic::PrefixAutomaton<d>& automaton,
    const std::function<bool(const mathlib::QElem&)>& extra_bound = nullptr) {
    using namespace property_f_cycle_detail;
    const mathlib::QBetaRing& R = automaton.ring;
    PropertyFCycleCriterionResult out;

    mathlib::QBetaInverseResult inv = mathlib::invert_in_qbeta(R.beta_k(1), R);
    if (!inv.invertible) throw std::runtime_error("check_property_f_via_cycle_criterion: beta not invertible");
    mathlib::QElem inv_beta = inv.inverse;

    // Same bound M as check_property_f: ||gamma|| <= M at the
    // SECONDARY (non-dominant) places is what actually admits a node
    // into Gamma -- a cycle's periodic gamma being abstractly nonzero
    // is necessary but NOT sufficient for it to be a genuine G^(0)
    // node/violation; it must also satisfy this geometric bound (and,
    // for non-unit substitutions, the supplied p-adic extra_bound).
    std::vector<std::complex<double>> all_roots = adelic::find_roots_durand_kerner(R.charpoly());
    std::sort(all_roots.begin(), all_roots.end(),
              [](const std::complex<double>& a, const std::complex<double>& b) { return std::abs(a) > std::abs(b); });
    std::vector<std::complex<double>> secondary_roots(all_roots.begin() + 1, all_roots.end());
    double max_secondary_modulus = 0.0;
    for (const auto& z : secondary_roots) max_secondary_modulus = std::max(max_secondary_modulus, std::abs(z));
    if (max_secondary_modulus >= 1.0) {
        throw std::runtime_error("check_property_f_via_cycle_criterion: secondary root modulus >= 1");
    }
    double bound_M = 0.0;
    for (const auto& delta_p : automaton.digit_set) bound_M = std::max(bound_M, adelic::archimedean_norm(delta_p, secondary_roots));
    bound_M /= (1.0 - max_secondary_modulus);
    bound_M *= 1.05;

    // gamma-graph adjacency: for letter a, list of (b, prefix-index)
    // pairs such that there's a T_ext^{-1} edge a -> b via that prefix.
    std::vector<std::vector<std::pair<long long, std::size_t>>> gadj(d);
    for (std::size_t b = 0; b < d; ++b) {
        for (const auto& [a, prefix] : automaton.by_source[b]) {
            auto it = std::find(automaton.distinct_prefixes.begin(), automaton.distinct_prefixes.end(), prefix);
            std::size_t idx = static_cast<std::size_t>(it - automaton.distinct_prefixes.begin());
            gadj[static_cast<std::size_t>(a)].push_back({static_cast<long long>(b), idx});
        }
    }

    mathlib::QElem zero = R.from_int(0);
    std::vector<long long> path;
    std::vector<std::size_t> prefix_idx_path;
    std::vector<bool> in_path(d, false);

    // DFS enumerating simple cycles starting (and only starting) at
    // the lexicographically smallest letter in each cycle, to avoid
    // reporting the same cycle once per rotation.
    std::function<void(long long, long long)> dfs = [&](long long start, long long cur) {
        for (const auto& [nxt, pidx] : gadj[static_cast<std::size_t>(cur)]) {
            if (nxt == start) {
                // Closes a simple cycle: the edge sequence is
                // prefix_idx_path (path.size() edges already recorded)
                // PLUS this closing edge's own pidx (start->...->cur->start)
                // -- L = path.size() + 1 total, not path.size().
                ++out.simple_cycles_checked;
                std::vector<std::size_t> full_prefixes = prefix_idx_path;
                full_prefixes.push_back(pidx);
                std::size_t L = full_prefixes.size();
                mathlib::QElem S = zero;
                for (std::size_t i = 0; i < L; ++i) {
                    mathlib::QElem term = R.mul(qpow(inv_beta, L - i, R), automaton.digit_set[full_prefixes[i]]);
                    S = R.add(S, term);
                }
                mathlib::QElem denom = R.sub(R.one(), qpow(inv_beta, L, R));
                mathlib::QBetaInverseResult dinv = mathlib::invert_in_qbeta(denom, R);
                if (!dinv.invertible) {
                    throw std::runtime_error("check_property_f_via_cycle_criterion: "
                                              "1 - beta^{-L} not invertible (unexpected: beta^L=1 would mean "
                                              "beta is a root of unity, impossible for a Pisot number)");
                }
                mathlib::QElem gamma_c = R.mul(S, dinv.inverse);
                if (!(gamma_c == zero)) {
                    double norm = adelic::archimedean_norm(gamma_c, secondary_roots);
                    bool within_bound = norm < bound_M && (!extra_bound || extra_bound(gamma_c));
                    if (within_bound) {
                        std::vector<long long> witness = path;
                        witness.push_back(nxt);
                        out.nonzero_cycle_letters.push_back(witness);
                    }
                }
                continue;
            }
            if (nxt < start) continue;  // only start from the min-letter of each cycle
            if (in_path[static_cast<std::size_t>(nxt)]) continue;
            path.push_back(nxt);
            prefix_idx_path.push_back(pidx);
            in_path[static_cast<std::size_t>(nxt)] = true;
            dfs(start, nxt);
            in_path[static_cast<std::size_t>(nxt)] = false;
            path.pop_back();
            prefix_idx_path.pop_back();
        }
    };

    for (long long s = 0; s < static_cast<long long>(d); ++s) {
        path.clear();
        prefix_idx_path.clear();
        std::fill(in_path.begin(), in_path.end(), false);
        dfs(s, s);
    }

    out.holds = out.nonzero_cycle_letters.empty();
    out.note = out.holds
        ? "every simple cycle in the prefix automaton has periodic gamma = 0"
        : "found a simple cycle with nonzero periodic gamma -- property (F) genuinely FAILS";
    return out;
}

}  // namespace ravel::proof
