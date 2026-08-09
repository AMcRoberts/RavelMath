// include/adelic/coincidence_and_property_f.hpp
//
// §3.4 of docs/ADELIC_TILING_PLAN.md: the two finite checks that,
// together, upgrade the always-true "multiple tiling" fact
// (Minervino-Thuswaldner Thm 9.2) to a genuine simple tiling
// (Thm 9.9): the strong coincidence condition (§2.4) and geometric
// property (F) (§2.5).
//
// Topological meaning (Carton--Sudbery--Yassawi, arXiv:2606.30496):
// Property (F) is the "preserves zeros" condition that gives the
// Pisot numeration system U a topological group Z_U.  Per CSY Theorem
// 1.1, when U satisfies Property (F) and is unimodular, Z_U is
// continuously isomorphic to a torus.  The existing F-check
// machinery in this header is the EXACT finite check that determines
// whether the CSY theorem applies; the topological-group structure
// is a corollary.  See `tests/pisot_numeration_topology_test.cpp` for
// the concrete application to the Class-II family, and the
// `math/pisot_numeration_topology.hpp` extension that builds the
// Z_U group structure on top.
//
// Automaton meaning (Carton--Sudbery--Yassawi, §5 and refs [Mes98]):
// For a Pisot numeration U that preserves zeros, the carry of the
// addition `g + h` in Z_U admits an explicit deterministic automaton
// (after Messaoudi 1998).  CSY extend this: for U preserving zeros,
// the FINITE prefix-closed regular set of "expansions of 0" is accepted
// by a finite automaton over the alphabet, and the homomorphism to the
// torus factors through this automaton's state set.  The
// finite-crossing correction is now RESOLVED — see
// `include/adelic/csy_carry_automaton.hpp` for the literal CSY carry
// automaton (PisotPoly, three β^n routes — memoised recurrence,
// companion-matrix exponentiation, Newton-iterated numerical —
// plus a bounded LRU cache over the matrix route) and its
// `validate()` cross-reference against the direct `[w]_U = 0`
// arithmetic.  `tests/csy_carry_automaton_test.cpp` kernel-checks the
// agreement on Fibonacci / Tribonacci Pisot numerations: 1093 words
// of length ≤ 6 on Fibonacci c = 1, 364 words of length ≤ 5 on
// Tribonacci c = 1, 0 discrepancies in each case.  The naive
// Pisot-basis integer-value carry in
// `math/pisot_numeration_topology.hpp::group_add` is the homomorphic
// image of this automaton's exact carry, and the
// `π(a + b) = π(a) + π(b) mod 1` identity holds modulo a bounded
// correction that `csy_carry_automaton.hpp` makes explicit.
//
// ===================================================================
// STATUS (updated after a second pass; read before trusting
// check_property_f's output -- it is now trustworthy in the cases
// documented below, and honestly not-yet-implemented elsewhere):
// ===================================================================
// check_strong_coincidence is verified correct: it reproduces the
// paper's own stated result (HOLDS) on the worked example, on
// Fibonacci, and on rnd13 (resolved at depth 1).
//
// check_property_f had two real, sequentially-diagnosed bugs. Both
// are now understood and the first is fixed; the second has a
// working fix for an important special case (which happens to be
// exactly rnd13's own case) and an honestly-scoped gap for the
// general case.
//
// BUG 1 (FIXED): the SCC-based cycle check flagged ANY self-loop or
// cycle touching a zero-translation node as a property-(F)
// violation. But an empty-prefix automaton edge (sigma(c) starting
// with c itself -- extremely common, e.g. Fibonacci's sigma(0)="01")
// produces a self-loop (0,a)->(0,a) purely from delta(epsilon)=0;
// the plan's own wording allows exactly this ("the only cycle ...
// passing through (0,a)-type nodes is the trivial one"). Fixed by
// requiring a cycle/SCC to contain BOTH a zero node and a nonzero
// node before counting it as a genuine violation -- a cycle staying
// entirely among zero-translation nodes is exactly the allowed
// trivial case. Verified: Fibonacci now correctly reports
// `holds=true` with a clean 8-node graph, matching Rauzy's classical
// result, regardless of node budget (100 through 1,000,000 all agree).
//
// BUG 2 (fixed for one case, open for the general case): the
// original archimedean-only magnitude bound never closes for
// NON-UNIT substitutions (confirmed: the worked example and rnd13
// both ran past a 1,000,000-node budget with archimedean pruning
// alone and never stopped generating new nodes). Root cause,
// understood via the standard Pisot beta-expansion argument
// (Schmidt 1980 / Frougny-Solomyak-style): genuine finiteness of the
// translation set requires boundedness in the FULL adelic sense --
// the archimedean secondary places AND the p-adic factor(s) TOGETHER
// (matching the whole point of this project's adelic construction:
// a non-unit Pisot substitution's representation space K_sigma has a
// p-adic factor precisely because the archimedean factor alone isn't
// enough). Fix: `make_totally_ramified_padic_bound` builds an
// additional p-adic-integrality predicate (gamma must lie in
// O_{K_p}, using the QpTotallyRamified machinery from padic.hpp) and
// `check_property_f` now takes an optional `extra_bound` predicate,
// pruning a node only if it passes BOTH the archimedean bound AND
// (when supplied) this one.
//
// Applied to rnd13 (whose (2) = p^4 is exactly the single-prime,
// fully-ramified case QpTotallyRamified covers): the combined bound
// closes the search at a stable 33185 nodes -- confirmed
// budget-independent (identical node count at every budget from
// 100000 up to 1,000,000) and precision-independent (identical at
// p-adic working precision 15, 20, 30, 50, and 80) -- and reports
// **property (F) HOLDS for rnd13**. Combined with strong coincidence
// also holding (see above), this is the actual headline
// result: rnd13's adelic tiling classifier now resolves both of the
// plan's §2.6 conditions, cleanly, with both checks cross-validated
// for stability. See docs/RESEARCH_STATUS.md for the full trace.
//
// UPDATE: the general combined bound is implemented below. It handles
// partial ramification, multiple rational primes, multiple ideals above
// one prime, residue degree f>1, and (after the per-factor linear
// Hensel lift) multiple non-simple factors. The paper's worked example
// and the formerly blocked rndW3_5 case both close through the shared
// classifier. An archimedean-only control remains intentionally
// inconclusive in coincidence_and_property_f_test.cpp.
//

// ===================================================================
// Strong coincidence (§2.4)
// ===================================================================
// For every pair of letters (b1, b2): does there exist a power k and
// a letter a such that sigma^k(b1) = p1 a s1, sigma^k(b2) = p2 a s2
// (both contain a occurrence of letter a), with either
// P(p1) = P(p2) (prefix abelianizations equal) or P(s1) = P(s2)
// (suffix abelianizations equal)?
//
// This is finite PER DEPTH but not a priori bounded in k (§5 of the
// plan).  Implementation: materialize sigma^k(b) for each starting
// letter b, incrementing k, checking each still-unresolved pair at
// each depth, capped by both a max depth and a max total word
// length (word length grows like beta^k, so a length cap is the
// actual binding constraint well before the depth cap for
// fast-growing beta).  Per-depth-per-pair coincidence detection uses
// two hash sets (prefix and suffix abelianizations per letter) for
// O(n log n) rather than the naive O(n^2) all-pairs scan.
//
// ===================================================================
// Geometric property (F) (§2.5)
// ===================================================================
// Build the zero-expansion graph: nodes are (gamma, a) pairs
// reachable from U = {(0, a) : a in alphabet} by repeatedly applying
// T^{-1}_ext: (gamma', b) in T^{-1}_ext(gamma, a) iff there is a
// prefix-automaton edge b --p--> a with gamma' = beta^{-1}(gamma +
// delta(p)).  Property (F) holds iff the only cycle through a
// (0, a)-type node is the trivial one.
//
// IMPLEMENTATION NOTE / HONEST LIMITATION: the plan's bound M on
// ||gamma|| uses a non-Archimedean norm on the p-adic factor of the
// adelic space K_sigma; this implementation does not build that
// norm.  Instead it works with gamma values as EXACT Q(beta)
// elements (which faithfully determine the value in every
// completion, real or p-adic -- no information is lost, only the
// specific finiteness PROOF that ||gamma|| < M is not re-derived
// here) and bounds the search by a NODE BUDGET rather than a proven
// geometric radius.  If the BFS closes (no new nodes) within budget,
// that is a genuine, exact finite computation -- the same conclusion
// the plan's norm-bounded graph would reach, just discovered via
// "did the frontier stop growing" rather than "is every node inside
// a proven ball."  If the budget is exhausted first, the result is
// reported INCONCLUSIVE, per the project's "don't silently treat a
// cutoff as a negative result" convention (ADELIC_TILING_PLAN.md §5,
// CPP_DESIGN_PHILOSOPHY.md's own tolerance-vs-exact discipline).

#pragma once

#include <algorithm>
#include <cmath>
#include <complex>
#include <cstddef>
#include <functional>
#include <map>
#include <memory>
#include <queue>
#include <set>
#include <stdexcept>
#include <unordered_map>
#include <utility>
#include <vector>

#include "math/qbeta.hpp"
#include "math/bezout.hpp"
#include "math/bigfloat.hpp"
#include "math/ball.hpp"
#include "math/proof_reflection.hpp"
#include "adelic/prefix_automaton.hpp"
#include "adelic/padic.hpp"
#include "adelic/local_field.hpp"
#include "adelic/ideal_arithmetic.hpp"
#include "adelic/property_f_types.hpp"

namespace adelic {

// ===================================================================
// Strong coincidence
// ===================================================================

template <std::size_t d>
using AbelVec = std::array<long long, d>;

template <std::size_t d>
AbelVec<d> abelianize(const std::vector<long long>& word) {
    AbelVec<d> v{};
    for (auto letter : word) {
        if (letter < 0 || static_cast<std::size_t>(letter) >= d) {
            throw std::invalid_argument("abelianize: letter out of range");
        }
        v[static_cast<std::size_t>(letter)] += 1;
    }
    return v;
}

template <std::size_t d>
std::vector<long long> apply_substitution(
    const std::array<std::vector<long long>, d>& images,
    const std::vector<long long>& word) {
    std::vector<long long> result;
    for (auto letter : word) {
        const auto& img = images[static_cast<std::size_t>(letter)];
        result.insert(result.end(), img.begin(), img.end());
    }
    return result;
}

// Does the pair (w1, w2) exhibit a coincidence: a shared letter a
// with matching prefix OR matching suffix abelianization at some
// occurrence in each word?  O(n log n) via hash/tree sets keyed by
// abelianization vector, not the naive O(n^2) all-occurrence scan.
template <std::size_t d>
bool pair_has_coincidence(const std::vector<long long>& w1,
                           const std::vector<long long>& w2) {
    AbelVec<d> total1 = abelianize<d>(w1);
    AbelVec<d> total2 = abelianize<d>(w2);

    std::array<std::set<AbelVec<d>>, d> prefix_set1, suffix_set1;
    AbelVec<d> running{};
    for (std::size_t i = 0; i < w1.size(); ++i) {
        long long a = w1[i];
        prefix_set1[static_cast<std::size_t>(a)].insert(running);
        AbelVec<d> suffix;
        for (std::size_t k = 0; k < d; ++k) {
            suffix[k] = total1[k] - running[k] - (static_cast<long long>(k) == a ? 1 : 0);
        }
        suffix_set1[static_cast<std::size_t>(a)].insert(suffix);
        running[static_cast<std::size_t>(a)] += 1;
    }
    running = AbelVec<d>{};
    for (std::size_t i = 0; i < w2.size(); ++i) {
        long long a = w2[i];
        if (prefix_set1[static_cast<std::size_t>(a)].count(running)) return true;
        AbelVec<d> suffix;
        for (std::size_t k = 0; k < d; ++k) {
            suffix[k] = total2[k] - running[k] - (static_cast<long long>(k) == a ? 1 : 0);
        }
        if (suffix_set1[static_cast<std::size_t>(a)].count(suffix)) return true;
        running[static_cast<std::size_t>(a)] += 1;
    }
    return false;
}

struct StrongCoincidenceResult {
    bool holds;               // true iff EVERY letter pair resolved by max_depth
    bool inconclusive;        // true iff some pair did not resolve (depth or length cap hit)
    long long depth_reached;
    long long unresolved_pairs;  // count still unresolved at cutoff (0 if holds)
    // Pair order is lexicographic: (0,1), (0,2), ..., (d-2,d-1).
    // A value of -1 means that the pair remained unresolved at the finite
    // cutoff.  Keeping this profile in the core result makes the finite
    // classifier auditable instead of requiring a second independent search
    // to recover per-pair depths.
    std::vector<long long> pair_resolution_depths;
};

template <std::size_t d>
StrongCoincidenceResult check_strong_coincidence(
    const std::array<std::vector<long long>, d>& images,
    long long max_depth = 20,
    long long max_word_len = 5'000'000) {
    std::vector<std::pair<long long, long long>> pairs;
    for (long long b1 = 0; b1 < static_cast<long long>(d); ++b1) {
        for (long long b2 = b1 + 1; b2 < static_cast<long long>(d); ++b2) {
            pairs.push_back({b1, b2});
        }
    }
    if (pairs.empty()) {
        // d = 1: no pairs to check, vacuously holds.
        return {true, false, 0, 0, {}};
    }

    std::vector<long long> pair_resolution_depths(pairs.size(), -1);
    std::vector<std::size_t> active_pair_slots(pairs.size());
    for (std::size_t i = 0; i < active_pair_slots.size(); ++i) active_pair_slots[i] = i;

    std::array<std::vector<long long>, d> words;
    for (std::size_t b = 0; b < d; ++b) words[b] = images[b];  // sigma^1(b)

    long long depth = 1;
    for (;;) {
        std::vector<std::pair<long long, long long>> still_unresolved;
        std::vector<std::size_t> still_unresolved_slots;
        for (std::size_t pair_index = 0; pair_index < pairs.size(); ++pair_index) {
            const auto& pr = pairs[pair_index];
            if (!pair_has_coincidence<d>(words[static_cast<std::size_t>(pr.first)],
                                          words[static_cast<std::size_t>(pr.second)])) {
                still_unresolved.push_back(pr);
                still_unresolved_slots.push_back(active_pair_slots[pair_index]);
            } else {
                // `pairs` is compacted after every depth, so carry the
                // original profile slot alongside each unresolved pair.
                pair_resolution_depths[active_pair_slots[pair_index]] = depth;
            }
        }
        // Preserve the original pair slot while compacting the active list;
        // the parallel slot vector keeps the profile stable across depths.
        pairs = std::move(still_unresolved);
        active_pair_slots = std::move(still_unresolved_slots);
        if (pairs.empty()) {
            return {true, false, depth, 0, std::move(pair_resolution_depths)};
        }
        if (depth >= max_depth) {
            return {false, true, depth, static_cast<long long>(pairs.size()),
                    std::move(pair_resolution_depths)};
        }
        long long maxlen = 0;
        for (const auto& w : words) {
            maxlen = std::max<long long>(maxlen, static_cast<long long>(w.size()));
        }
        // Estimate next length by the largest single-letter image
        // length as a crude per-step growth factor; bail out before
        // actually allocating something huge.
        long long max_image_len = 0;
        for (const auto& im : images) {
            max_image_len = std::max<long long>(max_image_len, static_cast<long long>(im.size()));
        }
        if (max_image_len > 1 && maxlen > max_word_len / max_image_len) {
            return {false, true, depth, static_cast<long long>(pairs.size()),
                    std::move(pair_resolution_depths)};
        }
        std::array<std::vector<long long>, d> next;
        for (std::size_t b = 0; b < d; ++b) {
            next[b] = apply_substitution<d>(images, words[b]);
        }
        words = std::move(next);
        ++depth;
    }
}

// ===================================================================
// Geometric property (F)
// ===================================================================

// ===================================================================
// Complex root finder (Durand-Kerner / Weierstrass method).
// ===================================================================
//
// Self-contained, double-precision, no external dependency (per this
// project's "roll your own" convention) -- needed to find the
// SECONDARY (non-dominant) roots of the charpoly, which the
// archimedean-norm bound on the zero-expansion graph actually needs.
//
// Why this is needed (found by direct experiment): a
// naive unbounded BFS over exact Q(beta) elements, applying
// T^{-1}_ext with no magnitude check, never terminates -- not even
// on the toy worked example (ran past 300,000 distinct nodes with no
// sign of closing).  The reason: T^{-1}_ext is the INVERSE of the
// substitution's contracting action, hence itself EXPANDING at every
// archimedean place (the plan's own §0 says multiplication by beta,
// i.e. the FORWARD substitution direction, is the contraction; beta
// is being inverted here).  The theorem's actual claim is narrower
// and still correct: any genuine CYCLE back to a zero-translation
// node must stay within a bounded region (an unboundedly expanding
// trajectory can never return), so restricting the graph to
// ||gamma|| < M and pruning branches that leave the ball is enough
// to decide property (F) correctly, while the ever-growing branches
// (which an earlier, unbounded attempt was exploring pointlessly)
// get pruned instead of explored.
// ===================================================================
// High-precision complex arithmetic (BigFloat-based), used ONLY to
// harden find_roots_durand_kerner below -- see its own comment for
// why. Minimal: just enough complex add/sub/mul/div to run
// Durand-Kerner at arbitrary precision; not a general-purpose complex
// type.
// ===================================================================
struct BigComplex {
    mathlib::BigFloat re, im;
};

inline BigComplex bc_add(const BigComplex& a, const BigComplex& b, unsigned prec) {
    return {mathlib::bigfloat_add(a.re, b.re, prec), mathlib::bigfloat_add(a.im, b.im, prec)};
}
inline BigComplex bc_sub(const BigComplex& a, const BigComplex& b, unsigned prec) {
    return {mathlib::bigfloat_sub(a.re, b.re, prec), mathlib::bigfloat_sub(a.im, b.im, prec)};
}
inline BigComplex bc_mul(const BigComplex& a, const BigComplex& b, unsigned prec) {
    // (ar+i*ai)(br+i*bi) = (ar*br - ai*bi) + i*(ar*bi + ai*br)
    auto t1 = mathlib::bigfloat_mul(a.re, b.re, prec);
    auto t2 = mathlib::bigfloat_mul(a.im, b.im, prec);
    auto t3 = mathlib::bigfloat_mul(a.re, b.im, prec);
    auto t4 = mathlib::bigfloat_mul(a.im, b.re, prec);
    return {mathlib::bigfloat_sub(t1, t2, prec), mathlib::bigfloat_add(t3, t4, prec)};
}
inline BigComplex bc_div(const BigComplex& a, const BigComplex& b, unsigned prec) {
    // a/b = a * conj(b) / |b|^2
    auto br2 = mathlib::bigfloat_mul(b.re, b.re, prec);
    auto bi2 = mathlib::bigfloat_mul(b.im, b.im, prec);
    auto denom = mathlib::bigfloat_add(br2, bi2, prec);
    auto num_re = mathlib::bigfloat_add(mathlib::bigfloat_mul(a.re, b.re, prec),
                                         mathlib::bigfloat_mul(a.im, b.im, prec), prec);
    auto num_im = mathlib::bigfloat_sub(mathlib::bigfloat_mul(a.im, b.re, prec),
                                         mathlib::bigfloat_mul(a.re, b.im, prec), prec);
    return {mathlib::bigfloat_div(num_re, denom, prec), mathlib::bigfloat_div(num_im, denom, prec)};
}
// Squared modulus (avoids needing a BigFloat sqrt; every comparison
// this file needs -- "is |x| < 1e-300", sorting by modulus -- is
// monotonic in the square, so this is sufficient throughout).
inline mathlib::BigFloat bc_norm2(const BigComplex& a, unsigned prec) {
    return mathlib::bigfloat_add(mathlib::bigfloat_mul(a.re, a.re, prec),
                                  mathlib::bigfloat_mul(a.im, a.im, prec), prec);
}

// Reference implementation, kept for A/B comparison (see the STATUS
// section above): the ORIGINAL double-precision Durand-Kerner. Not
// used by default any more -- superseded by the BigFloat version
// below, which fixes the "secondary root modulus >= 1" false-positive
// precision failures found by stress-testing with fresh random
// non-unit substitutions and an "easy family" sweep (see
// docs/RESEARCH_STATUS.md and the conversation this fix came out of: over
// half of a batch of random non-unit Pisot candidates hit this
// exception, i.e. it was a systematic precision limitation, not a
// rare edge case).
inline std::vector<std::complex<double>> find_roots_durand_kerner_double(
    const mathlib::PolyZ& charpoly, int iterations = 500) {
    long long deg = charpoly.degree();
    std::vector<double> coeffs(static_cast<std::size_t>(deg + 1));
    for (long long i = 0; i <= deg; ++i) {
        coeffs[static_cast<std::size_t>(i)] = mpz_get_d(charpoly.coeff(static_cast<std::size_t>(i)).get());
    }
    double lc = coeffs[static_cast<std::size_t>(deg)];
    for (auto& c : coeffs) c /= lc;

    auto eval = [&](std::complex<double> z) {
        std::complex<double> r = 0;
        for (long long i = deg; i >= 0; --i) r = r * z + coeffs[static_cast<std::size_t>(i)];
        return r;
    };

    std::vector<std::complex<double>> roots(static_cast<std::size_t>(deg));
    std::complex<double> seed(0.4, 0.9);
    std::complex<double> p = 1;
    for (long long i = 0; i < deg; ++i) {
        roots[static_cast<std::size_t>(i)] = p;
        p *= seed;
    }
    for (int it = 0; it < iterations; ++it) {
        for (long long i = 0; i < deg; ++i) {
            std::complex<double> num = eval(roots[static_cast<std::size_t>(i)]);
            std::complex<double> den = 1;
            for (long long j = 0; j < deg; ++j) {
                if (j == i) continue;
                den *= (roots[static_cast<std::size_t>(i)] - roots[static_cast<std::size_t>(j)]);
            }
            if (std::abs(den) < 1e-300) continue;
            roots[static_cast<std::size_t>(i)] -= num / den;
        }
    }
    return roots;
}

// High-precision Durand-Kerner (Weierstrass method), BigFloat-based
// throughout the iteration -- converts to double only at the very
// end, once the roots have actually converged to the requested
// precision. Same signature/return type as the original
// double-precision version, so every downstream caller (the
// archimedean-norm bound, secondary-root sorting) is unaffected --
// this is a drop-in replacement, not an API change.
inline std::vector<std::complex<double>> find_roots_durand_kerner(
    const mathlib::PolyZ& charpoly, int iterations = 200, unsigned prec_bits = 200) {
    long long deg = charpoly.degree();
    std::vector<mathlib::BigFloat> coeffs(static_cast<std::size_t>(deg + 1));
    for (long long i = 0; i <= deg; ++i) {
        coeffs[static_cast<std::size_t>(i)] =
            mathlib::bigfloat_from_bigint(charpoly.coeff(static_cast<std::size_t>(i)));
    }
    mathlib::BigFloat lc = coeffs[static_cast<std::size_t>(deg)];
    for (auto& c : coeffs) c = mathlib::bigfloat_div(c, lc, prec_bits);

    auto eval = [&](const BigComplex& z) {
        BigComplex r{mathlib::BigFloat(0), mathlib::BigFloat(0)};
        for (long long i = deg; i >= 0; --i) {
            r = bc_mul(r, z, prec_bits);
            r.re = mathlib::bigfloat_add(r.re, coeffs[static_cast<std::size_t>(i)], prec_bits);
        }
        return r;
    };

    // Seed: same style as the double version (0.4, 0.9)^i), built
    // exactly as rationals via bigfloat_div rather than an imprecise
    // double literal, since this now runs at high precision.
    BigComplex seed{mathlib::bigfloat_div(mathlib::bigfloat_from_ll(4), mathlib::bigfloat_from_ll(10), prec_bits),
                    mathlib::bigfloat_div(mathlib::bigfloat_from_ll(9), mathlib::bigfloat_from_ll(10), prec_bits)};
    std::vector<BigComplex> roots(static_cast<std::size_t>(deg));
    BigComplex p{mathlib::bigfloat_from_ll(1), mathlib::bigfloat_from_ll(0)};
    for (long long i = 0; i < deg; ++i) {
        roots[static_cast<std::size_t>(i)] = p;
        p = bc_mul(p, seed, prec_bits);
    }

    // Threshold for "denominator too small to divide by" -- scaled to
    // precision rather than a fixed double constant (1e-300 doesn't
    // mean anything once we're not in double space): 2^{-prec_bits}.
    mathlib::BigFloat tiny_threshold(mathlib::BigInt(1), -static_cast<long>(prec_bits));

    for (int it = 0; it < iterations; ++it) {
        for (long long i = 0; i < deg; ++i) {
            BigComplex num = eval(roots[static_cast<std::size_t>(i)]);
            BigComplex den{mathlib::bigfloat_from_ll(1), mathlib::bigfloat_from_ll(0)};
            for (long long j = 0; j < deg; ++j) {
                if (j == i) continue;
                den = bc_mul(den, bc_sub(roots[static_cast<std::size_t>(i)], roots[static_cast<std::size_t>(j)], prec_bits), prec_bits);
            }
            if (mathlib::bigfloat_cmp(bc_norm2(den, prec_bits), tiny_threshold) < 0) continue;
            roots[static_cast<std::size_t>(i)] = bc_sub(roots[static_cast<std::size_t>(i)],
                                                         bc_div(num, den, prec_bits), prec_bits);
        }
    }

    std::vector<std::complex<double>> out(static_cast<std::size_t>(deg));
    for (long long i = 0; i < deg; ++i) {
        out[static_cast<std::size_t>(i)] = std::complex<double>(
            mathlib::bigfloat_to_double(roots[static_cast<std::size_t>(i)].re),
            mathlib::bigfloat_to_double(roots[static_cast<std::size_t>(i)].im));
    }
    return out;
}

// Evaluate a QElem (rational coefficients, power basis) at a complex
// number via Horner's method.
inline std::complex<double> eval_qelem_at(const mathlib::QElem& x, std::complex<double> z) {
    std::complex<double> r = 0;
    for (long long i = static_cast<long long>(x.coeffs_.size()) - 1; i >= 0; --i) {
        double c = mpq_get_d(x.coeffs_[static_cast<std::size_t>(i)].get());
        r = r * z + c;
    }
    return r;
}

// Archimedean norm of a QElem: max |value| over the SECONDARY
// (non-dominant) roots of the charpoly -- the roots array is assumed
// sorted so roots[0] is dominant (checked by the caller).
inline double archimedean_norm(const mathlib::QElem& x,
                                const std::vector<std::complex<double>>& secondary_roots) {
    double m = 0.0;
    for (const auto& z : secondary_roots) {
        m = std::max(m, std::abs(eval_qelem_at(x, z)));
    }
    return m;
}

// Build a p-adic integrality predicate for use as check_property_f's
// `extra_bound` argument.  Two layers:
//
//   1. make_local_field_padic_bound(p, e, f, charpoly, residue_a, ...):
//      the GENERAL CASE for arbitrary (e, f) above p.  Uses
//      local_field.hpp's QpLocalField (Ore's algorithm and the
//      cofactor approach for the local polynomial).  This is the
//      version that handles the paper's worked example
//      (sigma(1)=1113, sigma(2)=11, sigma(3)=2 at p=2, prime p_1
//      with e=2, f=1), and it covers every (e, f) shape including
//      f > 1 (verified by app/validate_f2_bound.cpp).
//
//   2. make_totally_ramified_padic_bound(p, n, charpoly, ...):
//      a CONVENIENCE WRAPPER for the totally-ramified special case
//      (single prime above p, e = n, f = 1).  Internally delegates
//      to make_local_field_padic_bound with the residue
//      reconstructed from the charpoly's mod-p factorization
//      (the unique root of the linear mod-p factor g(x) = x - a).
//      Previously this used QpTotallyRamified from padic.hpp
//      directly; unifying on the general path removes the
//      parallel machinery and makes every shape go through the
//      same validated integrality-predicate code.
inline std::function<bool(const mathlib::QElem&)> make_totally_ramified_padic_bound(
    long long p, long long n, const mathlib::PolyZ& charpoly, long long precision = 30) {
    // The mod-p factorization of the charpoly in the totally-ramified
    // case is a single linear factor (mult=n, deg=1), so the residue
    // a is the unique root of that factor mod p.  Find it via the
    // FpFactor data: the factor's constant term is g(0) = -a (mod p).
    long long residue_a = 0;
    {
        adelic::FpPoly f_p = adelic::reduce_z_to_fp(charpoly, p);
        auto factors = adelic::factor_fp(f_p);
        for (const auto& fac : factors) {
            if (fac.mult == n
                && static_cast<long long>(fac.g.c.size()) - 1 == 1
                && fac.g.c.size() >= 1) {
                residue_a = ((-fac.g.c[0]) % p + p) % p;
                break;
            }
        }
    }
    return adelic::make_local_field_padic_bound(p, n, 1, charpoly, residue_a, precision);
}

// General case: arbitrary (e, f) above p.  Requires the residue `a`
// of β at the prime p_k (the unique root of g_k(x) mod p; for f > 1
// cases this generalizes to the appropriate Teichmüller lift).
//
// (This is now just a thin re-export of make_local_field_padic_bound;
// kept for backward compatibility with callers that already pass
// through this name.  Use make_local_field_padic_bound directly in
// new code.)
inline std::function<bool(const mathlib::QElem&)> make_general_padic_bound(
    long long p, long long e, long long f,
    const mathlib::PolyZ& charpoly, long long residue_a,
    long long precision = 30) {
    return adelic::make_local_field_padic_bound(p, e, f, charpoly, residue_a, precision);
}

// ===================================================================
// Combined multi-prime / multi-ideal bound (property F, general case)
// ===================================================================
//
// The bound classify_adelic_tiling.cpp's own classify() function used
// (before this addition) only ever built a p-adic bound from the
// FIRST prime ideal encountered across the FIRST prime dividing det --
// silently wrong for any substitution with more than one ramified
// ideal, and the actual cause of a real crash surfaced by stress-
// testing with fresh random non-unit substitutions (see conversation:
// sweep_nonunit_property_f.cpp's rndW3_31, det=-6=2x3, segfaulted
// rather than erroring, since local_field.hpp's machinery was fed
// state for a shape it never expected -- multiple RATIONAL primes,
// each contributing its own ideal(s), none but the first considered).
//
// The correct bound is the AND of every individual ramified ideal's
// own integrality predicate (gamma must be integral at EVERY place
// simultaneously, not just one) -- this function builds exactly that,
// gathering every PrimeIdeal from every rational prime's own
// DedekindFactorization directly (PrimeIdeal::g is the actual
// irreducible factor polynomial, so the residue for f=1 ideals is
// read straight off it: residue = -g.coeff(0) mod p; no separate
// factor_fp() call needed).
//
// ===================================================================
// Combined multi-prime, multi-ideal bound (property F, general case)
// ===================================================================
//
// For every rational prime p in `primes_dividing_det` (the distinct
// rational primes dividing |det M|) and every prime ideal above p in
// O_K, build the local integrality predicate at that ideal.  The
// combined bound is the AND of all of these -- gamma is in the
// bounded region iff it is integral at EVERY ramified ideal of
// O_K (one predicate per ideal, regardless of how many distinct
// rational primes contribute).  The "trusted" flag in the returned
// pair is `true` when every per-ideal bound is itself complete
// (i.e., it is the genuine ring-of-integers predicate at that
// ideal, not a f==1-only approximation). The local-field implementation is
// complete for arbitrary (e, f) shapes that arise here, but the factorization
// is certified only when Dedekind's order test and the independent ideal
// lattice cross-check both pass. A non-maximal result is still useful for
// exploration, but the returned flag is false and callers must report it as
// inconclusive rather than as an established Property-F theorem.
inline std::pair<std::function<bool(const mathlib::QElem&)>, bool>
make_combined_padic_bound(const std::vector<long long>& primes_dividing_det,
                           const mathlib::PolyZ& charpoly,
                           long long precision = 30) {
    auto bounds = std::make_shared<std::vector<std::function<bool(const mathlib::QElem&)>>>();
    bool trusted = true;
    for (long long p : primes_dividing_det) {
        auto fac = adelic::factor_prime_in_qbeta(charpoly, p);
        // Dedekind's factorization is only a factorization in O_K when the
        // defining order is p-maximal and the independent ideal-lattice
        // identity agrees.  The local-field implementation can still build
        // a useful exploratory predicate in the other case, but it is not a
        // certified ring-of-integers bound and must not yield ESTABLISHED.
        if (!fac.maximal ||
            !adelic::cross_check_dedekind_factorization(fac, charpoly,
                                                        charpoly.degree())) {
            trusted = false;
        }
        for (const auto& pi : fac.prime_ideals) {
            long long residue_a = 0;
            if (pi.f == 1) {
                long long g0 = std::stoll(mathlib::str(pi.g.coeff(0)));
                residue_a = ((-g0) % p + p) % p;
            }
            // local_polynomial_cofactor's general branch (hit whenever
            // (e,f) != (1,1) and ef != n) identifies the requested
            // irreducible factor with its multiplicity modulo p and
            // Hensel-lifts it against the coprime product of every
            // other factor. Multiple non-simple ideals are therefore
            // supported; absent or ambiguous factor metadata is still
            // rejected explicitly rather than guessed.
            bounds->push_back(make_local_field_padic_bound(pi.p, pi.e, pi.f, charpoly, residue_a, precision));
        }
    }
    auto combined = [bounds](const mathlib::QElem& gamma) -> bool {
        for (auto& b : *bounds) if (!b(gamma)) return false;
        return true;
    };
    return {combined, trusted};
}

// ===================================================================
// Certified (exact rational, IBA/Ball-based) secondary-modulus bound
// ===================================================================
//
// find_roots_durand_kerner (BigFloat version, above) fixes the
// PRACTICAL problem: false rejections ("secondary root modulus >= 1")
// caused by insufficient double precision -- it turns former FALSE
// NEGATIVES into correct passes. But it is still fundamentally an
// approximation, however precise: it cannot itself distinguish "the
// true modulus is 0.9999999" from "the true modulus is 1.0000001" no
// matter how many bits are used, and a caller trusting it has no
// PROOF, just increasingly strong evidence. That is the different
// problem this section solves: a genuine certificate (a TRUE
// POSITIVE, not just an unrejected candidate) for the one case that
// covers essentially every substitution actually checked in this
// project so far -- a cubic charpoly (n=3) whose two secondary roots
// are a complex-conjugate pair (discriminant < 0, checked exactly).
//
// The idea needs no root-finding, complex arithmetic, or numerics of
// any kind: Vieta gives product(all roots) = (-1)^n * a_0 EXACTLY
// (a_0 the charpoly's integer constant term). For a conjugate pair
// (z, z̄), product(secondary roots) = |z|^2 (a real, nonnegative
// number) exactly equal to (-1)^n a_0 / beta. `certify_perron_bracket_exact`
// (already used elsewhere in this project for the Perron root itself)
// gives an EXACT rational bracket [lo, hi] for beta from the
// substitution's own nonnegative incidence matrix -- Collatz-Wielandt,
// no floating point anywhere. Combining: |z|^2 lies in the exact
// rational interval [(-1)^n a_0 / hi, (-1)^n a_0 / lo] (a_0's sign
// fixed, division monotonic), and a provable UPPER bound on |z|
// itself follows from a short exact-rational bisection for a
// rational s with s^2 >= that interval's upper endpoint (rounding
// AWAY from the true value, never toward it, so the result is a
// genuine proven upper bound, not merely a close approximation).
inline int cubic_discriminant_sign_exact(const mathlib::PolyZ& charpoly) {
    // charpoly must be a monic cubic: x^3 + b x^2 + c x + d.
    mathlib::BigInt b = charpoly.coeff(2);
    mathlib::BigInt c = charpoly.coeff(1);
    mathlib::BigInt d = charpoly.coeff(0);
    mathlib::BigInt b2, b3, c2, c3, d2, bc, t1, t2, t3, t4, t5, disc;
    mathlib::mul(b2, b, b);
    mathlib::mul(b3, b2, b);
    mathlib::mul(c2, c, c);
    mathlib::mul(c3, c2, c);
    mathlib::mul(d2, d, d);
    mathlib::mul(bc, b, c);
    mathlib::mul(t1, bc, d);
    mathlib::BigInt eighteen(18), four(4), twentyseven(27);
    mathlib::mul(t1, t1, eighteen);          // 18*b*c*d
    mathlib::mul(t2, b3, d);
    mathlib::mul(t2, t2, four);              // 4*b^3*d
    mathlib::mul(t3, b2, c2);                // b^2*c^2
    mathlib::mul(t4, c3, four);               // 4*c^3
    mathlib::mul(t5, d2, twentyseven);        // 27*d^2
    mathlib::add(disc, t1, t3);
    mathlib::sub(disc, disc, t2);
    mathlib::sub(disc, disc, t4);
    mathlib::sub(disc, disc, t5);
    return mathlib::sgn(disc);
}

// Returns {true, bound} if the certified route applies (n==3,
// discriminant < 0) with `bound` a PROVEN rational upper bound on the
// max secondary modulus; {false, -} otherwise (caller should fall
// back to the BigFloat estimate).
inline std::pair<bool, mathlib::Rat> certified_secondary_modulus_bound(
    const std::vector<std::vector<long long>>& incidence_matrix,
    const mathlib::PolyZ& charpoly) {
    if (charpoly.degree() != 3) return {false, mathlib::Rat(0)};
    if (cubic_discriminant_sign_exact(charpoly) >= 0) return {false, mathlib::Rat(0)};  // not a complex pair

    auto perron = mathlib::certify_perron_bracket_exact(incidence_matrix, 200, 1e-15);
    mathlib::Rat beta_lo = perron.bracket.lo;

    mathlib::BigInt a0 = charpoly.coeff(0);  // n=3 is odd, so (-1)^n = -1
    mathlib::BigInt a0_abs; mathlib::abs_(a0_abs, a0);
    mathlib::Rat a0_rat(a0_abs);
    // target = |a0| / beta_lo -- the SMALLER beta bound gives the
    // LARGER, i.e. safely conservative, upper bound on |z|^2.
    mathlib::Rat target(0);
    mathlib::div(target, a0_rat, beta_lo);

    // Exact rational bisection for the smallest-found s with
    // s^2 >= target (rounding away from the true sqrt: a proven
    // upper bound, never an underestimate).
    mathlib::Rat s_hi(1), two(2);
    mathlib::Rat s_hi_sq(0);
    mathlib::mul(s_hi_sq, s_hi, s_hi);
    while (mathlib::cmp(s_hi_sq, target) < 0) {
        mathlib::mul(s_hi, s_hi, two);
        mathlib::mul(s_hi_sq, s_hi, s_hi);
    }
    mathlib::Rat s_lo(0);
    for (int it = 0; it < 200; ++it) {
        mathlib::Rat mid(0), sum(0);
        mathlib::add(sum, s_lo, s_hi);
        mathlib::div(mid, sum, two);
        mathlib::Rat mid_sq(0);
        mathlib::mul(mid_sq, mid, mid);
        if (mathlib::cmp(mid_sq, target) >= 0) s_hi = mid; else s_lo = mid;
    }
    return {true, s_hi};  // s_hi^2 >= target by construction: a proven upper bound
}

template <std::size_t d>
PropertyFResult check_property_f(
    const PrefixAutomaton<d>& automaton,
    long long node_budget = 1'000'000,
    const std::function<bool(const mathlib::QElem&)>& extra_bound = nullptr,
    std::vector<std::vector<long long>>* out_adjacency = nullptr,
    const std::vector<std::vector<long long>>* incidence_matrix_for_certified_bound = nullptr,
    // Diagnostic only (default nullptr, no behavior change for existing
    // callers): if supplied, receives the count of zero-translation
    // nodes discovered BEYOND the initial `d`-sized starting frontier.
    // Per ravel/proof/property_f_unconditional.hpp's proof, this should
    // be exactly 0 for every Pisot substitution -- no nonzero-gamma
    // node can ever have an edge into a zero-gamma node. Exposed here
    // so that claim can be checked against the real, trusted
    // computation rather than a re-implemented copy.
    long long* out_zero_nodes_beyond_frontier = nullptr,
    PropertyFGraph* out_graph = nullptr) {
    const mathlib::QBetaRing& R = automaton.ring;
    mathlib::QElem beta = R.from_int(0);
    beta.coeff(1) = mathlib::Rat(1, 1);
    mathlib::QBetaInverseResult inv = mathlib::invert_in_qbeta(beta, R);
    if (!inv.invertible) {
        throw std::runtime_error("check_property_f: beta is not invertible in Q(beta) "
                                  "(should never happen for a genuine Pisot beta)");
    }
    mathlib::QElem inv_beta = inv.inverse;
    mathlib::QElem zero = R.from_int(0);
    std::string zero_key = qelem_key(zero);

    // Secondary (non-dominant) roots and the bound M, per §2.5:
    // M = max_{delta in D} ||delta|| / (1 - ||beta||), using the
    // archimedean norm at the secondary places (the dominant root is
    // excluded -- see the long comment above find_roots_durand_kerner
    // for why an UNBOUNDED search never terminates, even on the toy
    // worked example, without this pruning step).
    std::vector<std::complex<double>> all_roots = find_roots_durand_kerner(R.charpoly());
    std::sort(all_roots.begin(), all_roots.end(),
              [](const std::complex<double>& a, const std::complex<double>& b) {
                  return std::abs(a) > std::abs(b);
              });
    std::vector<std::complex<double>> secondary_roots(all_roots.begin() + 1, all_roots.end());
    double max_secondary_modulus = 0.0;
    for (const auto& z : secondary_roots) max_secondary_modulus = std::max(max_secondary_modulus, std::abs(z));

    // Prefer the CERTIFIED (exact rational, IBA/Ball-based) bound when
    // it applies (cubic charpoly, complex-conjugate secondary pair) --
    // a genuine proof, not merely a well-converged approximation. See
    // certified_secondary_modulus_bound's own comment for why this is
    // a different problem than the BigFloat fix above solves (true
    // positives / proof, vs. true negatives / not spuriously
    // rejecting good cases).
    bool used_certified_bound = false;
    if (incidence_matrix_for_certified_bound) {
        auto [applies, certified] = certified_secondary_modulus_bound(
            *incidence_matrix_for_certified_bound, R.charpoly());
        if (applies) {
            double certified_d = mpq_get_d(certified.get());
            // Sanity cross-check: the certified (proven) bound should
            // never be LESS than the numerically observed modulus --
            // if it were, that would indicate a bug in one of the two
            // independent computations, not something to silently
            // paper over.
            if (certified_d < max_secondary_modulus - 1e-9) {
                throw std::runtime_error("check_property_f: certified secondary-modulus bound "
                                          "is SMALLER than the numerically observed modulus -- "
                                          "a bug in one of the two independent computations, "
                                          "refusing to proceed with either.");
            }
            max_secondary_modulus = certified_d;
            used_certified_bound = true;
        }
    }
    (void)used_certified_bound;

    if (max_secondary_modulus >= 1.0) {
        // Shouldn't happen for a genuine Pisot beta (all conjugates
        // strictly inside the unit circle); if the root finder is
        // imprecise enough to trip this, refuse to proceed with a
        // meaningless bound rather than silently using a bad M.
        throw std::runtime_error("check_property_f: secondary root modulus >= 1 "
                                  "(unexpected for a Pisot beta; root-finder precision issue?)");
    }
    double bound_M = 0.0;
    for (const auto& delta_p : automaton.digit_set) {
        bound_M = std::max(bound_M, archimedean_norm(delta_p, secondary_roots));
    }
    bound_M /= (1.0 - max_secondary_modulus);
    // Small safety margin: the theorem's M is an exact sup; our root
    // values are double-precision approximations, so pad slightly to
    // avoid spuriously excluding a boundary node due to rounding.
    bound_M *= 1.05;

    // Node identity is an exact serialized Q(beta)-key plus letter.  The
    // traversal never needs ordering; an ordered tree made the million-node
    // quartic cases pay an avoidable O(log n) lookup and one tree allocation
    // per state.  A reserved hash table keeps the same identity semantics
    // while making the finite search's cost proportional to its actual graph.
    std::unordered_map<std::string, long long> node_id;
    node_id.max_load_factor(0.70f);
    node_id.reserve(1u << 20);
    std::vector<mathlib::QElem> node_gamma;
    std::vector<long long> node_letter;
    std::vector<std::vector<long long>> adj;  // adjacency: node -> successor node ids
    std::vector<std::vector<std::vector<std::pair<std::string, std::string>>>> edge_digits;
    std::vector<bool> is_zero_node;
    std::vector<bool> enqueued;

    long long zero_nodes_beyond_frontier = 0;
    long long boundary_edges = 0;
    auto get_or_create = [&](const mathlib::QElem& gamma, long long letter) -> long long {
        std::string key = qelem_key(gamma) + "|" + std::to_string(letter);
        auto it = node_id.find(key);
        if (it != node_id.end()) return it->second;
        long long id = static_cast<long long>(node_gamma.size());
        bool is_zero_now = (qelem_key(gamma) == zero_key);
        if (is_zero_now && id >= static_cast<long long>(d)) ++zero_nodes_beyond_frontier;
        node_id[key] = id;
        node_gamma.push_back(gamma);
        node_letter.push_back(letter);
        adj.push_back({});
        edge_digits.push_back({});
        is_zero_node.push_back(is_zero_now);
        enqueued.push_back(false);
        return id;
    };

    std::vector<long long> frontier;
    for (long long a = 0; a < static_cast<long long>(d); ++a) {
        long long id = get_or_create(zero, a);
        enqueued[static_cast<std::size_t>(id)] = true;
        frontier.push_back(id);
    }

    std::vector<long long> queue = frontier;
    std::size_t qi = 0;
    bool budget_exceeded = false;
    while (qi < queue.size()) {
        if (static_cast<long long>(node_gamma.size()) > node_budget) {
            budget_exceeded = true;
            break;
        }
        long long u = queue[qi++];
        const mathlib::QElem gamma = node_gamma[static_cast<std::size_t>(u)];  // copy: node_gamma can reallocate below
        long long a = node_letter[static_cast<std::size_t>(u)];
        // Edges: for every prefix-automaton edge b --p--> a, produce
        // (gamma', b) with gamma' = beta^{-1} (gamma + delta(p)).
        for (std::size_t b = 0; b < d; ++b) {
            for (const auto& target_prefix : automaton.by_source[b]) {
                if (target_prefix.first != a) continue;
                // Find delta(p) for this prefix via the automaton's
                // distinct_prefixes / digit_set parallel arrays.
                const auto& prefix = target_prefix.second;
                auto dp_it = std::find(automaton.distinct_prefixes.begin(),
                                        automaton.distinct_prefixes.end(), prefix);
                if (dp_it == automaton.distinct_prefixes.end()) continue;  // shouldn't happen
                std::size_t dp_idx = static_cast<std::size_t>(
                    dp_it - automaton.distinct_prefixes.begin());
                const mathlib::QElem& delta_p = automaton.digit_set[dp_idx];
                mathlib::QElem sum = R.add(gamma, delta_p);
                mathlib::QElem gamma_prime = R.mul(inv_beta, sum);
                // Prune: only continue expanding nodes within the
                // bounded region; nodes outside it still exist as
                // graph sinks (so edges into them are recorded, for
                // completeness) but are never expanded further --
                // per the argument in the comment above
                // find_roots_durand_kerner, no genuine cycle back to
                // zero can pass through an unboundedly-expanding
                // branch, so it's safe to stop following it.
                double norm = archimedean_norm(gamma_prime, secondary_roots);
                long long v = get_or_create(gamma_prime, static_cast<long long>(b));
                adj[static_cast<std::size_t>(u)].push_back(v);
                std::vector<std::pair<std::string, std::string>> digit_coefficients;
                for (const auto& coefficient : delta_p.coeffs_) {
                    digit_coefficients.emplace_back(mathlib::str(mathlib::num(coefficient)),
                                                    mathlib::str(mathlib::den(coefficient)));
                }
                edge_digits[static_cast<std::size_t>(u)].push_back(std::move(digit_coefficients));
                bool within_bound = norm < bound_M && (!extra_bound || extra_bound(gamma_prime));
                if (!within_bound) ++boundary_edges;
                if (within_bound && !enqueued[static_cast<std::size_t>(v)]) {
                    enqueued[static_cast<std::size_t>(v)] = true;
                    queue.push_back(v);
                }
            }
        }
        if (static_cast<long long>(node_gamma.size()) > node_budget) {
            budget_exceeded = true;
            break;
        }
    }

    // Populate the optional adjacency output regardless of the
    // eventual verdict below (a property-(F) violation, an
    // inconclusive budget exhaustion, or success all still produce a
    // genuine finite graph up to that point -- the graph itself is
    // the "zero-expansion graph" this project's docs refer to, i.e.
    // the finite combinatorial object living in the combined
    // archimedean + p-adic bounded region of K_sigma; its dominant
    // eigenvalue is the adelic-extended analogue of lambda(G_B) that
    // docs/DIRECTION_AND_OPEN_THREADS.md's follow-up experiment
    // needs, since the ordinary G_B (contact_boundary.hpp) only ever
    // sees the archimedean factor of K_sigma for a non-unit
    // substitution.
    //
    // Kept as a SPARSE adjacency list (node -> successor ids, with
    // repeats standing in for multiplicity), matching `adj`'s own
    // internal representation, NOT a dense n x n matrix: this graph
    // is typically small-out-degree but can have tens of thousands
    // of nodes (rnd13 closes at 33185), and a dense n^2 matrix at
    // that scale is tens of gigabytes -- confirmed by an out-of-
    // memory kill during an earlier attempt at this
    // exposure, which used exactly that dense format.  Callers that
    // want a ravel::WeightedDigraph should build it edge-by-edge
    // from this sparse form (see
    // app/adelic_boundary_spectral_radius.cpp).
    if (out_adjacency != nullptr) {
        *out_adjacency = adj;
    }
    if (out_graph != nullptr) {
        out_graph->characteristic_polynomial.clear();
        const auto& charpoly = R.charpoly();
        for (long long i = 0; i <= charpoly.degree(); ++i)
            out_graph->characteristic_polynomial.push_back(mathlib::str(charpoly.coeff(static_cast<std::size_t>(i))));
        out_graph->beta_inverse_matrix.clear();
        out_graph->beta_inverse_matrix.resize(R.degree());
        for (std::size_t row = 0; row < R.degree(); ++row) {
            out_graph->beta_inverse_matrix[row].resize(R.degree());
            for (std::size_t column = 0; column < R.degree(); ++column) {
                mathlib::QElem basis = R.from_int(0);
                basis.coeff(column) = mathlib::Rat(1, 1);
                const mathlib::QElem image = R.mul(inv_beta, basis);
                out_graph->beta_inverse_matrix[row][column] = {
                    mathlib::str(mathlib::num(image.coeff(row))),
                    mathlib::str(mathlib::den(image.coeff(row)))};
            }
        }
        out_graph->nodes.clear();
        out_graph->nodes.reserve(node_gamma.size());
        for (std::size_t i = 0; i < node_gamma.size(); ++i) {
            PropertyFGraphNode node;
            node.gamma_key = qelem_key(node_gamma[i]);
            for (const auto& coefficient : node_gamma[i].coeffs_) {
                node.gamma_coefficients.emplace_back(mathlib::str(mathlib::num(coefficient)),
                                                     mathlib::str(mathlib::den(coefficient)));
            }
            node.letter = node_letter[i];
            node.zero = is_zero_node[i];
            node.successors = adj[i];
            node.edge_digit_coefficients = edge_digits[i];
            out_graph->nodes.push_back(std::move(node));
        }
    }

    if (budget_exceeded) {
        if (out_zero_nodes_beyond_frontier) *out_zero_nodes_beyond_frontier = zero_nodes_beyond_frontier;
        PropertyFResult out;
        out.holds = false;
        out.inconclusive = true;
        out.closure_reached = false;
        out.archimedean_bound_applied = true;
        out.extra_bound_applied = static_cast<bool>(extra_bound);
        out.node_budget = node_budget;
        out.boundary_edges = boundary_edges;
        out.nodes_explored = static_cast<long long>(node_gamma.size());
        out.zero_nodes = static_cast<long long>(std::count(is_zero_node.begin(), is_zero_node.end(), true));
        out.nonzero_nodes = out.nodes_explored - out.zero_nodes;
        return out;
    }

    // Tarjan's SCC algorithm over the (now-closed, finite) graph.
    long long n = static_cast<long long>(node_gamma.size());
    std::vector<long long> index(static_cast<std::size_t>(n), -1), lowlink(static_cast<std::size_t>(n), -1);
    std::vector<bool> on_stack(static_cast<std::size_t>(n), false);
    std::vector<long long> stack;
    long long next_index = 0;
    std::vector<long long> scc_of(static_cast<std::size_t>(n), -1);
    long long scc_count = 0;

    // Iterative Tarjan (avoid deep recursion for large graphs).
    for (long long start = 0; start < n; ++start) {
        if (index[static_cast<std::size_t>(start)] != -1) continue;
        std::vector<std::pair<long long, std::size_t>> work;  // (node, child_iter)
        work.push_back({start, 0});
        index[static_cast<std::size_t>(start)] = lowlink[static_cast<std::size_t>(start)] = next_index++;
        stack.push_back(start);
        on_stack[static_cast<std::size_t>(start)] = true;
        while (!work.empty()) {
            auto& [v, ci] = work.back();
            if (ci < adj[static_cast<std::size_t>(v)].size()) {
                long long w = adj[static_cast<std::size_t>(v)][ci];
                ++ci;
                if (index[static_cast<std::size_t>(w)] == -1) {
                    index[static_cast<std::size_t>(w)] = lowlink[static_cast<std::size_t>(w)] = next_index++;
                    stack.push_back(w);
                    on_stack[static_cast<std::size_t>(w)] = true;
                    work.push_back({w, 0});
                } else if (on_stack[static_cast<std::size_t>(w)]) {
                    lowlink[static_cast<std::size_t>(v)] = std::min(lowlink[static_cast<std::size_t>(v)],
                                                                     index[static_cast<std::size_t>(w)]);
                }
            } else {
                if (lowlink[static_cast<std::size_t>(v)] == index[static_cast<std::size_t>(v)]) {
                    for (;;) {
                        long long w = stack.back();
                        stack.pop_back();
                        on_stack[static_cast<std::size_t>(w)] = false;
                        scc_of[static_cast<std::size_t>(w)] = scc_count;
                        if (w == v) break;
                    }
                    ++scc_count;
                }
                work.pop_back();
                if (!work.empty()) {
                    long long parent = work.back().first;
                    lowlink[static_cast<std::size_t>(parent)] =
                        std::min(lowlink[static_cast<std::size_t>(parent)], lowlink[static_cast<std::size_t>(v)]);
                }
            }
        }
    }

    // A cycle (SCC of size > 1, or a self-loop) is a genuine
    // property-(F) violation iff it MIXES zero-translation and
    // nonzero-translation nodes -- i.e. it's possible to leave a
    // zero node, pass through at least one nonzero node, and return.
    // A cycle that stays ENTIRELY among zero-translation nodes (e.g.
    // the empty-prefix self-loop (0,a)->(0,a) that exists whenever
    // some sigma(c) starts with c itself -- extremely common) is
    // exactly the "trivial" cycle the theorem's own wording allows
    // ("the only cycle ... passing through (0,a)-type nodes is the
    // trivial one") and must NOT be flagged.  A cycle that never
    // touches any zero node isn't "a cycle passing through
    // (0,a)-type nodes" at all, so it isn't covered by this check
    // either way.  (This replaces an earlier, incorrect version of
    // this function that flagged ANY self-loop/cycle touching a
    // zero node, which produced a confirmed false FAILS on the
    // classical Fibonacci substitution -- see the header's STATUS
    // section and docs/RESEARCH_STATUS.md for the diagnosis.)
    //
    // CORRECTION (2026-08-06): the "BOTH zero AND nonzero" requirement
    // just above was itself wrong, discovered by reading the primary
    // source directly (Minervino-Thuswaldner, Lemma 9.8) rather than
    // this project's own paraphrase of it. The paper's actual failure
    // condition is a cycle whose nodes are ALL nonzero (never
    // touching a zero-translation node at all) -- not "mixed". Given
    // `property_f_unconditional.hpp` separately proves (Lean-checked
    // core lemma, `lean/generated/property_f_zero_walk.lean`) that a
    // MIXED cycle can never occur for any Pisot substitution (Perron-
    // eigenvector positivity forces every edge into a zero node to
    // come only from another zero node), "not entirely zero" and "all
    // nonzero" are the SAME condition here -- so the fix is to drop
    // the `scc_has_zero` requirement below, not to also require it.
    // The original BUG1 fix corrected a real false positive (the
    // trivial empty-prefix self-loop) by adding a requirement in the
    // wrong direction; it should have REMOVED the zero-touching
    // requirement entirely instead of adding a nonzero-touching one
    // alongside it. Verified against Fibonacci (must still HOLD, and
    // does: its 8-node closure is a pure DAG among nonzero nodes, no
    // cycle at all outside the trivial zero self-loop) and re-run
    // against every historical ESTABLISHED case in
    // tests/property_f_correct_verdict_test.cpp before trusting this
    // change -- an attempted alternative fix (enumerating abstract
    // cycles in the small letter-level automaton directly, independent
    // of the BFS closure) was tried FIRST and gave a false FAILS on
    // Fibonacci, because it over-generates: it counts algebraically
    // self-consistent cycles that are never actually reached by any
    // genuine walk from the zero frontier. The BFS-from-U closure
    // itself was never wrong -- it faithfully implements the paper's
    // own T_ext^{-1} formula (eq. 13); only the verdict extracted from
    // it was.
    std::vector<long long> scc_size(static_cast<std::size_t>(scc_count), 0);
    std::vector<bool> scc_has_zero(static_cast<std::size_t>(scc_count), false);
    std::vector<bool> scc_has_nonzero(static_cast<std::size_t>(scc_count), false);
    for (long long v = 0; v < n; ++v) {
        long long s = scc_of[static_cast<std::size_t>(v)];
        scc_size[static_cast<std::size_t>(s)]++;
        if (is_zero_node[static_cast<std::size_t>(v)]) scc_has_zero[static_cast<std::size_t>(s)] = true;
        else scc_has_nonzero[static_cast<std::size_t>(s)] = true;
    }
    if (out_graph != nullptr) {
        out_graph->scc_labels = scc_of;
        out_graph->scc_sizes = scc_size;
    }
    std::vector<bool> scc_has_self_loop(static_cast<std::size_t>(scc_count), false);
    for (long long v = 0; v < n; ++v) {
        for (long long w : adj[static_cast<std::size_t>(v)]) {
            if (w == v) scc_has_self_loop[static_cast<std::size_t>(scc_of[static_cast<std::size_t>(v)])] = true;
        }
    }
    // NOTE (2026-08-07): this verdict logic relies on the
    // Lean-kernel-checked fact (property_f_unconditional.hpp,
    // lean/generated/property_f_zero_walk.lean's `zeroWalk_eq_zero_iff`)
    // that a mixed zero/nonzero SCC can never occur. An earlier version
    // of this file recorded a static `LemmaApplication` citation here
    // and had the renderer paste fixed Lean text keyed by that name --
    // the same flawed "citation" pattern corrected elsewhere this
    // session (see docs/REFLECTION_RETROFIT_PLAN.md's 2026-08-07
    // correction note). Removed rather than left in place: a genuine
    // per-instance connection needs concrete Q(beta)-valued delta data
    // and a reconstructed walk path threaded from THIS automaton's own
    // run, not a name match -- not yet built. Do not re-add a citation
    // here without that real data.

    long long nonzero_cycle_components = 0;
    bool has_nonzero_cycle = false;
    std::vector<long long> witness_nodes;
    std::vector<std::pair<long long, long long>> witness_edges;
    for (long long s = 0; s < scc_count; ++s) {
        bool is_cycle = scc_size[static_cast<std::size_t>(s)] > 1 || scc_has_self_loop[static_cast<std::size_t>(s)];
        if (!is_cycle) continue;
        if (scc_has_nonzero[static_cast<std::size_t>(s)]) {
            ++nonzero_cycle_components;
            has_nonzero_cycle = true;
            if (witness_nodes.empty()) {
                long long start = -1;
                for (long long v = 0; v < n; ++v) {
                    if (scc_of[static_cast<std::size_t>(v)] == s &&
                        !is_zero_node[static_cast<std::size_t>(v)]) {
                        start = v;
                        break;
                    }
                }
                if (start >= 0) {
                    for (const long long first : adj[static_cast<std::size_t>(start)]) {
                        if (scc_of[static_cast<std::size_t>(first)] != s) continue;
                        std::vector<long long> prev(static_cast<std::size_t>(n), -1);
                        prev[static_cast<std::size_t>(first)] = start;
                        std::queue<long long> pending;
                        pending.push(first);
                        while (!pending.empty() && prev[static_cast<std::size_t>(start)] == -1) {
                            const long long u = pending.front();
                            pending.pop();
                            for (const long long v : adj[static_cast<std::size_t>(u)]) {
                                if (scc_of[static_cast<std::size_t>(v)] != s ||
                                    prev[static_cast<std::size_t>(v)] != -1) continue;
                                prev[static_cast<std::size_t>(v)] = u;
                                pending.push(v);
                            }
                        }
                        if (prev[static_cast<std::size_t>(start)] == -1) continue;
                        std::vector<long long> back;
                        long long cur = start;
                        back.push_back(cur);
                        while (cur != first) {
                            cur = prev[static_cast<std::size_t>(cur)];
                            back.push_back(cur);
                        }
                        std::reverse(back.begin(), back.end());
                        witness_nodes.push_back(start);
                        witness_nodes.insert(witness_nodes.end(), back.begin(), back.end());
                        for (std::size_t i = 1; i < witness_nodes.size(); ++i)
                            witness_edges.emplace_back(witness_nodes[i - 1], witness_nodes[i]);
                        break;
                    }
                }
            }
        }
    }
    if (out_graph != nullptr) out_graph->nonzero_cycle_components = nonzero_cycle_components;
    if (out_zero_nodes_beyond_frontier) *out_zero_nodes_beyond_frontier = zero_nodes_beyond_frontier;
    PropertyFResult out;
    out.holds = !has_nonzero_cycle;
    out.inconclusive = false;
    out.closure_reached = true;
    out.archimedean_bound_applied = true;
    out.extra_bound_applied = static_cast<bool>(extra_bound);
    out.node_budget = node_budget;
    out.boundary_edges = boundary_edges;
    out.nodes_explored = n;
    out.zero_nodes = static_cast<long long>(std::count(is_zero_node.begin(), is_zero_node.end(), true));
    out.nonzero_nodes = n - out.zero_nodes;
    out.strongly_connected_components = scc_count;
    out.nonzero_cycle_components = nonzero_cycle_components;
    out.violation_cycle_nodes = std::move(witness_nodes);
    out.violation_cycle_edges = std::move(witness_edges);
    return out;
}

}  // namespace adelic
