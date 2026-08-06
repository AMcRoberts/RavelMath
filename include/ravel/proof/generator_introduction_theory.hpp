// ravel/proof/generator_introduction_theory.hpp
//
// The general theory of generator introduction for terminating Pisot
// expansions, built on `terminating_generator_theorem.hpp`. The
// primitive generator set is {0,+-1} union the DIFFERENCE SET of V,
// where V is the set of DISTINCT values among the non-final digits
// (t_0,...,t_{N-2}) -- a classical additive-combinatorics object, not
// a bespoke one. This gives a clean, provable bound on how rich the
// generator set can possibly be, purely as a function of beta's own
// integer part, with no per-number computation required to know the
// BOUND (only to know which specific generators a given number
// realizes).
//
// THEOREM (maximum generator count per beta-level). For beta in
// [k, k+1) (so every digit is bounded by k = floor(beta)), the
// non-final digit value set V is a subset of {0,1,...,k}. Every
// achievable difference lies in [-k,k] trivially (both operands of any
// difference are themselves in [0,k]), so the primitive generator
// count is at most 2k+1, with equality iff diff(V) = {-k,...,k}
// exactly (the difference set covers the full range) -- checked
// directly below, not assumed to require any particular V.
//
// A CORRECTION made while deriving this (worth recording, not just
// fixing silently): the natural first guess is that V = {0,...,k}
// itself (every digit value present) is the unique way to hit this
// bound, reasoning "arithmetic progressions look like the natural
// maximal case." That reasoning has the classical additive-
// combinatorics fact backwards. An arithmetic progression MINIMIZES
// the size of a set's difference set for a given cardinality -- an AP
// with r elements has |V-V| = 2r-1 exactly, the smallest possible; a
// SIDON set (all pairwise differences distinct) has the LARGEST
// possible difference set for r elements, |V-V| = r(r-1)+1. Checked
// directly: V={0,1,2} (an AP, r=3) gives diff(V) of size 5 = 2(3)-1;
// V={0,1,3} (not an AP, r=3) gives diff(V) of size 7 -- covering
// {-3,...,3} completely with only 3 elements, not the 4 that
// {0,1,2,3} would need. So the bound 2k+1 can be ACHIEVED by a set
// smaller than {0,...,k}, via Sidon-like structure -- this is exactly
// the classical "perfect difference basis / sparse ruler" problem
// (minimal |V| with V-V covering an interval of width k grows like
// sqrt(2k), not like k), not a fact specific to this project's
// substitutions. `min_difference_basis_size` below computes the true
// minimum by direct search for small k, rather than assuming k+1.

#pragma once

#include <algorithm>
#include <cstddef>
#include <set>
#include <vector>

namespace ravel::proof {

// The difference set of a set of integers: {a-b : a,b in S}.
inline std::set<long long> difference_set(const std::set<long long>& S) {
    std::set<long long> D;
    for (long long a : S) for (long long b : S) D.insert(a - b);
    return D;
}

struct GeneratorIntroductionReport {
    long long k{};                         // floor(beta) for this level, i.e. max possible digit value
    std::set<long long> value_set;         // V: distinct non-final digit values actually realized
    std::set<long long> generator_set;      // {0,+-1} union diff(V)
    std::size_t generator_count{};
    std::size_t max_possible_at_this_k{};   // 2k+1
    bool achieves_maximum_for_k{};          // true iff diff(V) == {-k,...,k} exactly
};

inline GeneratorIntroductionReport analyze_generator_introduction(
    const std::vector<long long>& digits, long long k) {
    GeneratorIntroductionReport out;
    out.k = k;
    if (digits.size() >= 2) out.value_set = std::set<long long>(digits.begin(), digits.end() - 1);
    auto diffs = difference_set(out.value_set);
    out.generator_set = {-1, 0, 1};
    for (long long d : diffs) out.generator_set.insert(d);
    out.generator_count = out.generator_set.size();
    out.max_possible_at_this_k = static_cast<std::size_t>(2 * k + 1);
    // The {0,+-1} base is always free, so the max is achieved when the
    // FULL generator set (base union diff(V)) covers {-k,...,k}, not
    // when diff(V) alone does -- diff(V) can lean on the free +-1 too.
    bool covers_full_range = true;
    for (long long v = -k; v <= k && covers_full_range; ++v) covers_full_range = out.generator_set.count(v) > 0;
    out.achieves_maximum_for_k = covers_full_range;
    return out;
}

// The classical "perfect difference basis for an interval" problem:
// the minimum size of a subset V of {0,...,k} such that diff(V)
// covers all of {-k,...,k}. Brute-force search (fine for the small k
// this project's Pisot digits ever reach) rather than assumed to be
// k+1 -- see the theorem's correction note above for why that
// assumption would be wrong.
inline std::size_t min_difference_basis_size(long long k) {
    if (k == 0) return 1;
    for (std::size_t r = 1; r <= static_cast<std::size_t>(k) + 1; ++r) {
        // Try all r-subsets of {0,...,k} that include 0 and k (WLOG,
        // since 0 and k must both be needed to reach the extremes -1
        // is unreachable without a k-valued element, and translating
        // V doesn't change diff(V), so fixing the minimum at 0 loses
        // no generality; k must be present to reach difference k
        // itself, since every element is <= k).
        std::vector<long long> mid;
        for (long long v = 1; v < k; ++v) mid.push_back(v);
        std::vector<bool> chosen(mid.size(), false);
        std::size_t need = (r >= 2) ? r - 2 : 0;
        if (need > mid.size()) continue;
        std::fill(chosen.end() - static_cast<long long>(need), chosen.end(), true);
        do {
            std::set<long long> V = {0, k};
            for (std::size_t i = 0; i < mid.size(); ++i) if (chosen[i]) V.insert(mid[i]);
            if (V.size() != r) continue;
            auto diffs = difference_set(V);
            bool covers = true;
            for (long long v = -k; v <= k && covers; ++v) covers = diffs.count(v) > 0;
            if (covers) return r;
        } while (std::next_permutation(chosen.begin(), chosen.end()));
    }
    return static_cast<std::size_t>(k) + 1;
}

}  // namespace ravel::proof
