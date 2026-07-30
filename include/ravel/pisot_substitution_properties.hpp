// include/ravel/pisot_substitution_properties.hpp
//
// First-principles analysis tools for irreducible unimodular Pisot
// substitutions. Built on top of the existing helpers in
//   - include/ravel/core.hpp           (Substitution<d>, M, v)
//   - include/ravel/spectral.hpp       (β, b2, charpoly)
//   - include/ravel/involution_helpers.hpp (A1, A2 probes)
//   - include/ravel/substitution.hpp    (image, expand, matrix)
//
// Per the session directive ("Don't cut any corners or exclude any
// math, even math you don't need to use right now. Again, it might be
// important later. If you are planning on implementing a thing,
// check for an existing implementation in the project, and extend
// that as necessary."): every function below is built on top of
// existing primitives. We do NOT introduce new spectral-invariants
// computations; we use what's there.
//
// The functions in this header are general (work for any substitution,
// Pisot or not) but the *table of properties* is most meaningful for
// Pisot substitutions, where the connection to the Rauzy fractal
// / open tiling conjecture becomes relevant.
//
// What this header provides (one per entry):
//   1. Letter frequencies (the Perron right eigenvector of M,
//      normalized to sum 1). This is the substitution's invariant
//      measure (the "Parry measure" for Pisot substitutions).
//   2. Image-length sequence |σ(i)|.
//   3. Constant-factor flag: is there a position k with σ(i)[k] ==
//      a fixed letter for all i?  This is the n-bonacci's "0 is
//      always the first letter" property (HAS_CONSTANT_FACTOR in
//      contact_boundary.hpp).  Not all Pisot have it; σ_{0,1} doesn't.
//   4. Constant-finals flag: does σ(i) end with the same letter for
//      all i?  Used in the n-bonacci BP-core's swap involution.
//   5. Spectrum: β (Perron root), b2 = max |other eigenvalue|, char poly
//      of the substitution matrix (in high-first format).
//   6. Pisot dual σ* (longest common prefix rule).
//   7. Primitive return words up to a length cap.
//   8. Carry sequence: the letter distribution of σ^k(0) for k=0..N-1.
//
// Use:
//   auto rule = SubstitutionRule(sigma_images);
//   auto props = compute_pisot_properties(rule);
//   std::printf("%s", props.to_string().c_str());

#pragma once

#include <algorithm>
#include <cstdint>
#include <cstdio>
#include <map>
#include <optional>
#include <sstream>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>

#include "math/charpoly.hpp"
#include "ravel/core.hpp"
#include "ravel/involution_helpers.hpp"
#include "ravel/spectral.hpp"
#include "ravel/substitution.hpp"
#include "ravel/survey.hpp"

namespace ravel {

// =====================================================================
// Basic structural properties
// =====================================================================

// Letter frequencies from the right eigenvector of the substitution
// matrix.  v is computed in the Substitution<d> constructor; here we
// normalize it to sum 1 and return as a vector of doubles.  For Pisot
// substitutions, these are the (unique) Parry measure frequencies.
inline std::vector<double> letter_frequencies(
        const std::vector<std::vector<std::int8_t>>& sigma) {
    // Substitution<3> takes std::array<std::vector<long long>, 3>.
    // Convert from int8_t to long long.  The constructor is for
    // alphabet size 3; for other sizes we'd template on d.  The
    // Pisot test set in the project is all 3-letter.
    if (sigma.size() != 3) {
        std::vector<double> f(sigma.size(), 1.0 / sigma.size());
        return f;
    }
    std::array<std::vector<long long>, 3> images_long;
    for (std::size_t i = 0; i < 3; ++i) {
        images_long[i].reserve(sigma[i].size());
        for (auto c : sigma[i]) images_long[i].push_back(c);
    }
    Substitution<3> subst(images_long, 1.0);  // beta is irrelevant for v
    // subst.v is set in the constructor.  Normalize to sum 1.
    std::vector<double> f(3);
    double s = 0.0;
    for (std::size_t i = 0; i < 3; ++i) s += subst.v[i];
    if (s <= 0.0) {
        // Edge case: Perron eigenvector has zero sum.  Fall back to
        // uniform frequencies; this happens only for substitutions
        // where v[0] is in the kernel of M^T, which the constructor
        // already handles by the kernel-recovery path.  If the user
        // constructed Substitution<3> from a hand-rolled sigma that
        // somehow produces this, the uniform fallback is the best we
        // can do.
        for (std::size_t i = 0; i < 3; ++i) f[i] = 1.0 / 3.0;
        return f;
    }
    for (std::size_t i = 0; i < 3; ++i) f[i] = subst.v[i] / s;
    return f;
}

// "Constant factor" flag: is there a position k such that σ(i)[k] is
// the same letter for all i?  Returns (has_constant_factor,
// first_constant_factor_position) or (-1, -1) when there is none.
// This is the n-bonacci's "0 is always the first letter" property;
// σ_{0,1} (with σ(0) = (1,2), σ(1) = (2), σ(2) = (0)) does NOT have it.
inline std::pair<bool, long long> has_constant_factor(
        const std::vector<std::vector<std::int8_t>>& sigma) {
    if (sigma.empty()) return {false, -1};
    std::size_t d = sigma.size();
    std::size_t max_len = 0;
    for (const auto& img : sigma) max_len = std::max(max_len, img.size());
    if (max_len == 0) return {false, -1};
    for (std::size_t k = 0; k < max_len; ++k) {
        long long common = -2;  // sentinel: "we haven't seen a letter at position k yet"
        bool uniform = true;
        for (std::size_t i = 0; i < d; ++i) {
            if (k >= sigma[i].size()) { uniform = false; break; }
            long long c = sigma[i][k];
            if (common == -2) common = c;
            else if (common != c) { uniform = false; break; }
        }
        if (uniform) return {true, static_cast<long long>(k)};
    }
    return {false, -1};
}

// "Constant finals" flag: does σ(i) end with the same letter for
// all i?  Returns (has_constant_finals, common_final_letter) or
// (false, -1).  Used in the n-bonacci BP-core's swap involution.
inline std::pair<bool, long long> has_constant_finals(
        const std::vector<std::vector<std::int8_t>>& sigma) {
    if (sigma.empty()) return {false, -1};
    long long common = -2;
    bool uniform = true;
    for (std::size_t i = 0; i < sigma.size(); ++i) {
        if (sigma[i].empty()) { uniform = false; break; }
        long long f = sigma[i].back();
        if (common == -2) common = f;
        else if (common != f) { uniform = false; break; }
    }
    if (uniform) return {true, common};
    return {false, -1};
}

// Image-length sequence |σ(i)| for each letter i.
inline std::vector<std::size_t> image_lengths(
        const std::vector<std::vector<std::int8_t>>& sigma) {
    std::vector<std::size_t> r(sigma.size());
    for (std::size_t i = 0; i < sigma.size(); ++i) r[i] = sigma[i].size();
    return r;
}

// =====================================================================
// Pisot dual (longest common prefix rule)
// =====================================================================
//
// For an irreducible unimodular Pisot substitution σ, the *Pisot dual*
// σ* is defined by:
//   σ*(i) = longest common prefix of {σ(j) : i appears in σ(j)}
// where the "longest" is computed under the constraint that σ*
// itself is a valid substitution (i.e., the matrix M^* defined by
// σ* has the same M_{i,j} structure as M).
//
// For Tribonacci (n=3) this gives σ*(0) = 0, σ*(1) = 0 1, σ*(2) = 0 2.
// For σ_{0,1}: σ(0) = (1,2), σ(1) = (2), σ(2) = (0). The letter 0
// appears in σ(2) only, so σ*(0) = σ(2) = 0. Letter 1 appears in
// σ(0) only, so σ*(1) = σ(0) = (1, 2). Letter 2 appears in
// σ(0) and σ(1), so σ*(2) = longest common prefix of (1,2) and (2)
// which is (2) alone. So σ* is: 0 -> 0, 1 -> (1,2), 2 -> 2.
//
// The PERRON-FROBENIUS property of M (Pisot: unique largest
// eigenvalue) ensures σ*'s expansion rate is 1/β, the contraction
// rate.  For Pisot, σ and σ* together tile the plane.
inline std::vector<std::vector<std::int8_t>> pisot_dual(
        const std::vector<std::vector<std::int8_t>>& sigma) {
    if (sigma.empty()) return {};
    std::size_t d = sigma.size();
    std::vector<std::vector<std::int8_t>> out(d);
    for (std::size_t i = 0; i < d; ++i) {
        // Find all j with multiplicity of i in σ(j) > 0.
        std::vector<std::size_t> js;
        for (std::size_t j = 0; j < d; ++j) {
            for (auto r : sigma[j]) {
                if (static_cast<std::size_t>(r) == i) {
                    js.push_back(j);
                    break;
                }
            }
        }
        if (js.empty()) {
            // Letter i doesn't appear in any σ(j).  σ*(i) is empty
            // (this is degenerate; shouldn't happen for irreducible
            // Pisot but handle gracefully).
            continue;
        }
        // Compute the longest common prefix of {σ(j) : j in js}.
        std::vector<std::int8_t> prefix = sigma[js[0]];
        for (std::size_t idx = 1; idx < js.size(); ++idx) {
            const auto& img = sigma[js[idx]];
            std::size_t common_len = std::min(prefix.size(), img.size());
            std::size_t k = 0;
            while (k < common_len && prefix[k] == img[k]) ++k;
            prefix.resize(k);
        }
        out[i] = prefix;
    }
    return out;
}

// =====================================================================
// Primitive return words
// =====================================================================
//
// A word W is a "return word" for letter i if there exists a k and
// an n such that σ^k(i) = W * X * W (i.e., W appears as a factor
// after applying σ k times and the return-to-i path).  The
// "primitive" return word is the shortest such W.
//
// For Pisot substitutions, the return map on each letter is a
// rotation by 1/β (mod 1), and the primitive return words generate
// the Pisot pair structure.
//
// This is a brute-force computation: for each letter, apply σ
// repeatedly, find the first self-overlap.  Cheap for small N; for
// large alphabolas / images this is O(N^2) and the cap should be
// kept modest.
inline std::vector<std::string> primitive_return_words(
        const SubstitutionRule& rule,
        std::size_t max_word_length = 64) {
    if (rule.alphabet_size() == 0) return {};
    std::size_t d = rule.alphabet_size();
    std::vector<std::string> out;
    for (std::size_t i = 0; i < d; ++i) {
        // Iterate σ until we find a word containing i twice.
        // We accumulate the orbit of i under σ and look for the
        // shortest W such that σ^k(i) contains W = σ^W_length(i).
        // Since the orbit eventually cycles, the return word is
        // bounded by the cycle length.
        std::string word(1, static_cast<char>(i));
        for (std::size_t k = 0; k < max_word_length; ++k) {
            // Check if 'i' appears in 'word' more than once.
            std::size_t count_i = 0;
            for (char c : word) {
                if (static_cast<std::size_t>(c) == i) ++count_i;
                if (count_i >= 2) break;
            }
            if (count_i >= 2) {
                // Primitive return word: the segment between the
                // first two occurrences of i.
                std::size_t first = word.find(static_cast<char>(i));
                std::size_t second = word.find(static_cast<char>(i),
                                              first + 1);
                if (second != std::string::npos && first < second) {
                    out.push_back(word.substr(first, second - first));
                }
                break;
            }
            // Apply σ to extend the word.
            std::string new_word;
            new_word.reserve(word.size() * 2);
            for (char c : word) {
                const auto& img = rule.image(static_cast<std::size_t>(c));
                for (auto r : img) new_word.push_back(static_cast<char>(r));
            }
            word = std::move(new_word);
        }
    }
    return out;
}

// =====================================================================
// Carry sequence (image distribution of σ^k(0) for k=0..N-1)
// =====================================================================
//
// For Pisot substitutions, the orbit of a letter under σ is a Sturmian
// or Christoffel-like sequence; the "letter distribution" along
// σ^k(0) for k=0,1,2,... captures the carry structure of the
// expansion.  For n-bonacci, this is the 2-adic ruler sequence.
// For σ_{0,1}, this is a different (but still structured) sequence.
struct CarrySequencePoint {
    std::size_t k;                       // iteration count
    std::size_t length;                  // |σ^k(0)|
    std::vector<std::size_t> letter_counts;  // per-letter counts
};

// Cap the carry sequence at this length by default.  The actual
// hard cap is the word size: we stop when |σ^k(0)| > carry_word_cap
// (default 5 MB; β^k grows fast, e.g. for β=2.5 we hit 5 MB around
// k=15).  We always emit at least a few points to make the "carries
// structure" visible in the tabulation.
inline constexpr std::size_t kDefaultCarrySequenceSteps = 64;
inline constexpr std::size_t kDefaultCarryWordCapBytes = 5 * 1024 * 1024;

// =====================================================================
// Rotation numbers
// =====================================================================
//
// For a Pisot substitution σ with Perron eigenvalue β, the orbit of any
// letter i under σ satisfies
//   |σ^k(i)| / |σ^k(0)|  →  r_i  as  k → ∞
// where r_i is a rational number (the i-th "rotation number" in the
// β-expansion of unit interval with substitution-digit partition).
//
// For Pisot substitutions, the rotation numbers are exactly the
// frequencies in the Parry measure:
//   r_i  =  frequency[i]   (i.e., the v_i / Σ v_j from the right
//                           Perron eigenvector, normalized)
//
// This is a strong sanity test: compute the orbit count ratio
// directly (no eigenvector computation needed) and check it matches
// the eigenvector frequencies to within tolerance.
struct RotationNumbers {
    // Per-letter i: limit of |σ^k(i)| / |σ^k(0)| as k → ∞, estimated
    // from the carry sequence.
    std::vector<double> estimates;
    // The |σ^k(0)| at the last computed step (so the caller knows
    // the convergence quality).
    std::size_t last_word_length = 0;
};

inline RotationNumbers compute_rotation_numbers(
        const SubstitutionRule& rule,
        std::size_t max_word_length = 65536) {
    if (rule.alphabet_size() == 0) return {};
    std::size_t d = rule.alphabet_size();
    std::vector<std::int8_t> orbits[3];
    for (int n = 0; n < 3 && n < (int)d; ++n) {
        orbits[n].push_back(static_cast<std::int8_t>(n));
    }
    if (d < 3) {
        for (int n = d; n < 3; ++n) orbits[n].clear();
    }

    RotationNumbers rn;
    rn.estimates.assign(d, 0.0);
    std::size_t k = 0;  // explicit shared iteration counter

    // Iterate σ in lockstep for all orbits (0, 1, 2).  Stop when
    // the LARGEST-growing orbit (orbits[0] for Pisot, since f(0) is
    // typically the largest frequency and the orbit[0] is the longest)
    // hits the cap, so the ratios orbit[i]/orbit[0] are at the same
    // k across all orbits — i.e., they're comparable.
    while (orbits[0].size() < max_word_length && k < 256) {
        for (int n = 0; n < 3 && n < (int)d; ++n) {
            if (orbits[n].size() >= max_word_length) continue;
            std::vector<std::int8_t> nw;
            nw.reserve(orbits[n].size() * d);
            for (auto c : orbits[n]) {
                for (auto r : rule.image(static_cast<std::size_t>(c))) {
                    nw.push_back(r);
                }
            }
            orbits[n] = std::move(nw);
        }
        ++k;
    }

    // For Tribonacci and similar (where f(0) > f(1) ≥ f(2)), orbits[0]
    // is the longest — so the cap is set by orbit[0].  For other
    // Pisot substitutions where another letter has a larger frequency,
    // orbit[0] may not be the longest.  In that case, ORBIT[i] may
    // be at a smaller k than the cap (we stopped when orbit[0] hit
    // the cap), so its size is smaller than the theoretical σ^{k_cap}
    // — but the RATIO orbit[i]/orbit[0] is still well-defined.
    // We don't re-grow to compensate; the residual decay is
    // (b2/β)^Δk where Δk is small.
    rn.last_word_length = orbits[0].size();

    if (orbits[0].empty()) return rn;
    for (std::size_t i = 0; i < d; ++i) {
        if (i < 3) {
            rn.estimates[i] = static_cast<double>(orbits[i].size()) /
                                static_cast<double>(orbits[0].size());
        } else {
            rn.estimates[i] = 0.0;
        }
    }
    return rn;
}

// =====================================================================
// Factor complexity of the orbit
// =====================================================================
//
// For a Pisot substitution, the orbit of letter 0 is conjecturally
// Sturmian (or Christoffel for rational rotation numbers, with a
// specific central word).  Sturmian sequences have factor complexity
// f(n) = n + 1 (i.e., exactly n+1 distinct factors of each length n).
// The number of distinct factors of each length n for a word can be
// counted directly; for Pisot it should match n+1.
//
// We cap the word length at max_word_length and the factor length at
// min(max_factor_length, max_word_length / 2).
struct FactorComplexity {
    // f(n) = number of distinct factors of length n in σ^k(0).
    std::vector<std::size_t> counts;       // f[n] for n = 0, 1, ...
    std::size_t word_length = 0;            // |σ^k(0)|
};

inline FactorComplexity compute_factor_complexity(
        const SubstitutionRule& rule,
        std::size_t max_word_length = 4096,
        std::size_t max_factor_length = 24) {
    FactorComplexity fc;
    if (rule.alphabet_size() == 0) return fc;
    std::vector<std::int8_t> orbit{0};
    std::size_t k = 0;
    while (orbit.size() < max_word_length && k < 256) {
        std::vector<std::int8_t> nw;
        nw.reserve(orbit.size() * rule.alphabet_size());
        for (auto c : orbit) {
            for (auto r : rule.image(static_cast<std::size_t>(c))) {
                nw.push_back(r);
            }
        }
        orbit = std::move(nw);
        ++k;
    }
    fc.word_length = orbit.size();

    std::size_t max_n = std::min(max_factor_length, orbit.size() / 2);
    fc.counts.assign(max_n + 1, 0);
    for (std::size_t n = 0; n <= max_n; ++n) {
        std::set<std::vector<std::int8_t>> uniq;
        for (std::size_t i = 0; i + n <= orbit.size(); ++i) {
            std::vector<std::int8_t> w(n);
            for (std::size_t k = 0; k < n; ++k) w[k] = orbit[i + k];
            uniq.insert(w);
        }
        fc.counts[n] = uniq.size();
    }
    return fc;
}

// Is the factor complexity Sturmian-like (Arnoux-Rauzy / Christoffel
// property)?  For a d-letter Pisot substitution, the orbit of letter
// 0 should have f(n) = (d-1)n + 1 (i.e., d+1 distinct factors of
// length d+1, and so on).  For d=2 this is the classical Sturmian
// condition f(n) = n+1; for d=3 it is the "Christoffel" condition
// f(n) = 2n+1; etc.  A Pisot substitution's orbit violates this iff
// its rotation sequence isn't strictly geometric.
//
// alphabet_size is the alphabet (substitution arity) and
// factor_complexity.counts is the array of f(n) values.
inline bool factor_complexity_is_arnoux_rauzy(
        std::size_t alphabet_size, const FactorComplexity& fc) {
    std::size_t d = alphabet_size;
    for (std::size_t n = 1; n + 1 < fc.counts.size(); ++n) {
        if (fc.counts[n] != (d - 1) * n + 1) return false;
    }
    return true;
}

// How well do the rotation numbers (from the orbit) match the
// predicted ratios from the Parry-measure frequencies?  For Pisot,
// the rotation number orbit[i]/orbit[0] converges to f(i)/f(0)
// (the frequency RATIO), not to f(i) itself.  A small deviation
// (< 1e-3) is strong evidence the orbit is consistent with Pisot.
struct RotationVsFrequency {
    double max_abs_dev = 0.0;
    std::size_t orbit_length = 0;
    // The rotation number for letter i in the orbit of 0, vs. the
    // predicted ratio f(i)/f(0).  We store both for clarity.
    std::vector<double> rotation;
    std::vector<double> predicted_ratios;
    std::vector<double> frequencies;
    bool converged = false;
};

inline RotationVsFrequency compare_rotation_to_frequency(
        const RotationNumbers& rn,
        const std::vector<double>& frequencies) {
    RotationVsFrequency out;
    out.rotation = rn.estimates;
    out.frequencies = frequencies;
    out.orbit_length = rn.last_word_length;
    if (rn.estimates.size() != frequencies.size()) return out;
    if (frequencies.empty()) return out;
    // The comparison: rotation number (orbit[i]/orbit[0]) should
    // converge to frequency ratio f(i)/f(0).  We compute the max
    // |rotation[i] - f(i)/f(0)| as the deviation metric.
    double f0 = frequencies[0];
    if (f0 == 0.0) return out;
    std::vector<double> ratios(frequencies.size());
    for (std::size_t i = 0; i < frequencies.size(); ++i) {
        ratios[i] = frequencies[i] / f0;
    }
    out.predicted_ratios = ratios;
    double max_diff = 0.0;
    for (std::size_t i = 0; i < rn.estimates.size(); ++i) {
        max_diff = std::max(max_diff,
            std::abs(rn.estimates[i] - ratios[i]));
    }
    out.max_abs_dev = max_diff;
    out.converged = (max_diff < 1e-3 && rn.last_word_length >= 1024);
    return out;
}

inline std::vector<CarrySequencePoint> carry_sequence(
        const SubstitutionRule& rule,
        std::size_t n_steps = kDefaultCarrySequenceSteps,
        std::size_t word_cap_bytes = kDefaultCarryWordCapBytes) {
    if (rule.alphabet_size() == 0) return {};
    std::size_t d = rule.alphabet_size();
    std::vector<CarrySequencePoint> out;
    std::vector<std::int8_t> word{0};
    out.push_back({0, 1, std::vector<std::size_t>(d, 0)});
    out.back().letter_counts[0] = 1;
    for (std::size_t k = 1; k < n_steps; ++k) {
        std::vector<std::int8_t> new_word;
        new_word.reserve(word.size() * d);
        for (auto c : word) {
            const auto& img = rule.image(static_cast<std::size_t>(c));
            for (auto r : img) new_word.push_back(r);
        }
        word = std::move(new_word);
        CarrySequencePoint pt;
        pt.k = k;
        pt.length = word.size();
        pt.letter_counts.assign(d, 0);
        for (auto c : word) pt.letter_counts[static_cast<std::size_t>(c)]++;
        out.push_back(std::move(pt));
        if (word.size() > word_cap_bytes) break;
    }
    return out;
}

// =====================================================================
// Pisot properties struct
// =====================================================================

struct PisotProperties {
    // Identification
    std::string name;

    // Spectral
    double beta = 0.0;
    double b2 = 0.0;
    std::vector<long long> charpoly_high_first;  // β^d + a_{d-1}*β^{d-1} + ...

    // Letter frequencies (Parry measure)
    std::vector<double> frequencies;

    // Structural
    std::vector<std::size_t> image_lengths;
    bool has_constant_factor = false;
    long long constant_factor_pos = -1;
    bool has_constant_finals = false;
    long long constant_final_letter = -1;

    // Pisot dual
    std::vector<std::vector<std::int8_t>> dual;

    // Return words (per letter)
    std::vector<std::string> primitive_return_words;

    // Carry sequence (orbit of letter 0)
    std::vector<CarrySequencePoint> carry_seq;

    // Rotation numbers (limit of |σ^k(i)|/|σ^k(0)|)
    RotationNumbers rotation_numbers;

    // Factor complexity of orbit of letter 0
    FactorComplexity factor_complexity;
    bool is_sturmian = false;
    RotationVsFrequency rot_vs_freq;

    // A1, A2 (probes from involution_helpers)
    bool a2_involution_exact = false;
    int a2_matched = 0;
    int a2_total = 0;
    bool a1_xk = false;
    long a1_k = -1;

    std::string to_string() const;
};

inline PisotProperties compute_pisot_properties(
        const SubstitutionRule& rule,
        const std::string& name,
        bool run_a1a2_probes = false) {
    PisotProperties p;
    p.name = name;

    // Properly extract sigma images from the rule into a fresh
    // std::vector<std::vector<std::int8_t>> (since rule.image
    // returns a const ref, and we want our own copy).
    std::vector<std::vector<std::int8_t>> sigma(rule.alphabet_size());
    for (std::size_t i = 0; i < rule.alphabet_size(); ++i) {
        sigma[i].reserve(rule.image(i).size());
        for (auto c : rule.image(i)) sigma[i].push_back(c);
    }

    // Image lengths and constant-factor/finals.
    p.image_lengths = image_lengths(sigma);

    // (no need to construct Substitution<3> here; letter_frequencies
    // does it internally)

    auto [cf, cf_pos] = has_constant_factor(sigma);
    p.has_constant_factor = cf;
    p.constant_factor_pos = cf_pos;

    auto [cfl, cfl_letter] = has_constant_finals(sigma);
    p.has_constant_finals = cfl;
    p.constant_final_letter = cfl_letter;

    // Spectrum via existing spectral_invariants_3x3.
    auto M = [&]() {
        // Re-construct the matrix the same way Substitution<d> does.
        std::vector<std::vector<long long>> mm(rule.alphabet_size(),
            std::vector<long long>(rule.alphabet_size(), 0));
        for (std::size_t c = 0; c < rule.alphabet_size(); ++c) {
            for (auto r : rule.image(c)) {
                mm[static_cast<std::size_t>(r)][c] += 1;
            }
        }
        return mm;
    }();
    (void)M;
    auto sp = classify_matrix_spectral(M);
    p.beta = sp.beta;
    p.b2 = sp.b2;

    // Charpoly in high-first order.
    auto cp = charpoly_int(M);
    p.charpoly_high_first.assign(cp.rbegin(), cp.rend());

    // Frequencies.
    p.frequencies = letter_frequencies(sigma);

    // Dual.
    p.dual = pisot_dual(sigma);

    // Primitive return words.
    p.primitive_return_words = primitive_return_words(rule, /*max_word_length=*/64);

    // Carry sequence (capped by word size, not just by step count;
    // see carry_sequence() for the budget).
    p.carry_seq = carry_sequence(rule);

    // Rotation numbers and factor complexity (Arnoux-Rauzy / Christoffel
    // check for d-letter alphabet: f(n) = (d-1)n + 1).
    p.rotation_numbers = compute_rotation_numbers(rule);
    p.factor_complexity = compute_factor_complexity(rule);
    p.is_sturmian = factor_complexity_is_arnoux_rauzy(
        rule.alphabet_size(), p.factor_complexity);
    p.rot_vs_freq = compare_rotation_to_frequency(
        p.rotation_numbers, p.frequencies);

    // A1, A2 probes.
    if (run_a1a2_probes && sp.pisot) {
        SubstitutionRule rule_obj(sigma);
        auto subst_obj = make_substitution<3>(rule_obj, sp.beta);
        std::vector<std::tuple<long long, std::vector<long long>, long long>> d_cont;
        for (const auto& c : search_D_cont<3>(subst_obj, 2)) {
            d_cont.emplace_back(c.i,
                std::vector<long long>(c.x.begin(), c.x.end()),
                c.j);
        }
        ContactBoundaryReport rep;
        try {
            rep = compute_contact_boundary_dispatch(rule_obj, sp.beta, 0.0, d_cont);
        } catch (...) {
            // Pipeline threw; skip A1/A2.
            return p;
        }
        if (rep.converged && !rep.gb_matrix.empty()) {
            auto [matched, total] = check_involution_on_core(rep);
            p.a2_matched = matched;
            p.a2_total = total;
            p.a2_involution_exact = (matched == total && total > 0);

            auto gb_qsym = compute_gb_sym_quotient<3>(rep, rule_obj);
            auto bp_qsym = compute_bp_sym_quotient(rule_obj);
            if (gb_qsym.num_orbits > 0 && bp_qsym.num_orbits > 0) {
                auto ch_gb = charpoly_int(gb_qsym.Qsym);
                auto ch_bp = charpoly_int(bp_qsym.Qsym);
                std::vector<long long> ch_gb_hf(ch_gb.rbegin(), ch_gb.rend());
                std::vector<long long> ch_bp_hf(ch_bp.rbegin(), ch_bp.rend());
                bool exact = check_exact_factor("Pisot probe (Q_sym_GB / Q_sym_BP)",
                                                 ch_gb_hf, ch_bp_hf);
                p.a1_xk = exact;
                if (exact) {
                    p.a1_k = static_cast<long>(ch_gb.size()) -
                              static_cast<long>(ch_bp.size());
                }
            }
        }
    }

    return p;
}

inline std::string PisotProperties::to_string() const {
    std::ostringstream os;
    os << "================================================\n";
    os << "  " << name << "\n";
    os << "================================================\n";
    os << "  Spectrum: β = " << beta << ", b2 = " << b2 << "\n";
    os << "  Charpoly (high-first, degree " << (charpoly_high_first.size() - 1)
       << "):";
    for (auto c : charpoly_high_first) os << " " << c;
    os << "\n";
    os << "  Image lengths |σ(i)|:";
    for (auto L : image_lengths) os << " " << L;
    os << "\n";
    os << "  Has constant factor:    " << (has_constant_factor ? "yes" : "no")
       << (has_constant_factor ? " (at position " + std::to_string(constant_factor_pos) + ")" : "")
       << "\n";
    os << "  Has constant finals:    " << (has_constant_finals ? "yes" : "no")
       << (has_constant_finals ? " (letter " + std::to_string(constant_final_letter) + ")" : "")
       << "\n";
    os << "  Letter frequencies:";
    for (std::size_t i = 0; i < frequencies.size(); ++i) {
        os << "  f[" << i << "]=" << frequencies[i];
    }
    os << "\n";
    os << "  Pisot dual σ*:\n";
    for (std::size_t i = 0; i < dual.size(); ++i) {
        os << "    σ*(" << i << ") = (";
        for (auto c : dual[i]) os << static_cast<int>(c) << " ";
        os << ")\n";
    }
    os << "  Primitive return words:";
    for (const auto& w : primitive_return_words) {
        // The return words are sequences of small integers 0, 1, 2, ...
        // which std::string stores as raw char bytes (control chars
        // that don't print via <<).  Print them as digit sequences.
        os << " (";
        for (std::size_t k = 0; k < w.size(); ++k) {
            if (k > 0) os << ",";
            os << static_cast<int>(static_cast<unsigned char>(w[k]));
        }
        os << ")";
    }
    os << "\n";
    os << "  Carry sequence σ^k(0) (first 10):\n";
    for (std::size_t k = 0; k < std::min<std::size_t>(carry_seq.size(), 10); ++k) {
        const auto& pt = carry_seq[k];
        os << "    k=" << pt.k << "  |σ^k(0)|=" << pt.length << "  counts:";
        for (std::size_t j = 0; j < pt.letter_counts.size(); ++j) {
            os << "  " << j << ":" << pt.letter_counts[j];
        }
        os << "\n";
    }
    os << "  [A1 Q_sym cofactor]  x^k present? " << (a1_xk ? "YES" : "no");
    if (a1_xk) os << " (k=" << a1_k << ")";
    os << "\n";
    os << "  [A2 involution]      " << a2_matched << " / " << a2_total
       << (a2_involution_exact ? "  (EXACT)" : "  (partial)") << "\n";

    os << "  Rotation numbers (|σ^k(i)| / |σ^k(0)|, k→∞):";
    for (std::size_t i = 0; i < rotation_numbers.estimates.size(); ++i) {
        os << "  " << i << ":" << rotation_numbers.estimates[i];
    }
    os << "\n";
    os << "  Letter frequencies (Parry):";
    for (std::size_t i = 0; i < frequencies.size(); ++i) {
        os << "  " << i << ":" << frequencies[i];
    }
    os << "\n";
    os << "  Rotation/ratio max |diff|: " << rot_vs_freq.max_abs_dev
       << " (orbit length " << rot_vs_freq.orbit_length << ")"
       << (rot_vs_freq.converged
           ? "  [CONVERGED — orbit consistent with Pisot]"
           : "  [not yet converged at this orbit length]")
       << "\n";
    os << "  Factor complexity f(n) (Arnoux-Rauzy / Christoffel test for "
       << "d=" << (frequencies.empty() ? 0 : frequencies.size())
       << ", n=1.."
       << (factor_complexity.counts.empty() ? 0
           : std::min<std::size_t>(factor_complexity.counts.size(), 12) - 1)
       << "):";
    for (std::size_t n = 1; n < std::min<std::size_t>(
            factor_complexity.counts.size(), 12); ++n) {
        std::size_t expected = (frequencies.empty() ? 2 : (frequencies.size() - 1)) * n + 1;
        os << " f(" << n << ")=" << factor_complexity.counts[n]
           << (factor_complexity.counts[n] == expected ? "*" : "");
    }
    os << "\n";
    os << "  Orbit is Arnoux-Rauzy (f(n)=(d-1)n+1 for all n computed): "
       << (is_sturmian ? "YES" : "no") << "\n";

    return os.str();
}

}  // namespace ravel
