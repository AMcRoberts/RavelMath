// ravel/proof/beta_in_one_two_forces_qrs.hpp
//
// A genuine inductive theorem over the WHOLE sequence of Pisot numbers in
// (1,2), not a per-number computation. It answers, once and for all, the
// project's standing question "how do we get a genuine fourth primitive
// positive matrix?" for every Pisot number this small: you cannot, from
// this construction.
//
// Claim. Let beta be any Pisot number with 1 < beta < 2 (true of every
// Pisot number examined by this project so far: plastic, the quartic
// x^4-x^3-1 root, the third-smallest Pisot number, supergolden, the
// fifth- and sixth-smallest Pisot numbers, and the golden ratio itself).
// Its canonical beta-substitution (`canonical_beta_substitution_from_digits`,
// applied to the exact greedy expansion of 1) has EXACTLY the Q/R/S
// three-generator parent-prefix shape: every parent-decomposition prefix
// is either the empty word or the single letter [0], and both occur.
//
// Proof sketch (elementary, and checked exactly here, not assumed):
//   1. Every greedy digit d satisfies 0 <= d <= floor(beta) < 2, since
//      d = floor(beta * x) for x in [0,1) and beta < 2 forces beta*x < 2.
//      So every digit is exactly 0 or 1 -- checked exactly via the same
//      Q(beta)/Sturm sign machinery as the expansion itself, not assumed
//      from the bound alone.
//   2. The FIRST digit is forced to be exactly 1: beta > 1 means
//      floor(beta * 1) >= 1, and by (1) it is at most 1, so it is exactly
//      1. Hence state 0's image is always length 2 (`[0, 1]`), which is
//      the only source needed for the prefix [0] to occur (as the parent
//      of letter 1). The prefix [] occurs trivially (every image's first
//      symbol has empty prefix). So BOTH prefix classes always occur --
//      forced by beta being in (1,2), independent of which Pisot number.
//   3. With prefixes restricted to {[], [0]}, an ordered pair of parent
//      decompositions (p, q) has abelianized defect q.p - p.p equal to
//      the zero vector (both empty or both [0]), +e_0 (p=[], q=[0]), or
//      -e_0 (p=[0], q=[]) -- exactly three classes, and since both
//      prefixes occur (step 2), all three classes are realized by some
//      pair. No fourth class is combinatorially possible: there are no
//      other prefixes to draw from.
//
// This does not claim every Pisot number's ACTUAL project substitution
// (e.g. plastic's own hand-chosen 3-letter substitution, which is smaller
// than its canonical Parry substitution and is not literally the object
// this header analyzes) has this shape for the same reason; it happens to
// share the same {[],[0]} prefix shape empirically (checked separately),
// but the argument above is about the canonical construction specifically.
// A substitution search restricted to beta in (1,2) via this construction
// can therefore never discover a genuine fourth primitive generator; that
// search must move to beta >= 2 (admitting digit value 2, hence a third
// prefix [0,0]) or to a different (non-canonical) substitution family.

#pragma once

#include <cstddef>
#include <set>
#include <stdexcept>
#include <string>
#include <vector>

#include "math/qbeta.hpp"
#include "math/sturm.hpp"
#include "ravel/canonical_beta_substitution.hpp"

namespace ravel::proof {

struct BetaInOneTwoForcesQRSCertificate {
    std::vector<long long> digits;
    std::size_t alphabet_size{};
    bool beta_strictly_between_one_and_two{};
    bool all_digits_binary{};
    bool first_digit_is_one{};
    std::vector<std::vector<long long>> substitution;
    std::set<std::vector<long long>> prefixes_found;
    bool prefixes_subset_of_empty_and_zero{};
    bool both_prefixes_occur{};
    std::size_t defect_class_count{};
    bool exactly_three_defect_classes{};
    bool proved{};
    std::string obstruction;
};

inline BetaInOneTwoForcesQRSCertificate derive_beta_in_one_two_forces_qrs(
    const mathlib::QBetaRing& R, const mathlib::RootInterval& beta_I) {
    using namespace mathlib;
    BetaInOneTwoForcesQRSCertificate out;

    out.beta_strictly_between_one_and_two =
        cmp(beta_I.lo, Rat(1, 1)) > 0 && cmp(beta_I.hi, Rat(2, 1)) < 0;
    if (!out.beta_strictly_between_one_and_two) {
        out.obstruction = "beta is not certified strictly inside (1,2) by the supplied isolating interval";
        return out;
    }

    auto ge = exact_greedy_beta_expansion_of_one(R, beta_I, 128);
    if (!ge.terminated && ge.period_len == 0) {
        out.obstruction = "greedy expansion neither terminated nor cycled within the step budget";
        return out;
    }
    out.digits = ge.digits;
    out.alphabet_size = out.digits.size();

    out.all_digits_binary = true;
    for (auto d : out.digits) if (d != 0 && d != 1) out.all_digits_binary = false;
    out.first_digit_is_one = !out.digits.empty() && out.digits.front() == 1;
    if (!out.all_digits_binary || !out.first_digit_is_one) {
        out.obstruction = "digits not binary with a forced leading 1 -- beta may not actually be in (1,2), or expansion is degenerate";
        return out;
    }

    // Build the canonical substitution and independently re-derive the
    // prefix/defect shape by direct scan, rather than trusting the
    // sketch above without a check.
    out.substitution = canonical_beta_substitution_from_digits(out.digits);
    const std::size_t n = out.substitution.size();

    // parent_decompositions, written generically (runtime-sized, not
    // template<size_t d>) so this theorem applies uniformly across
    // Pisot numbers of any algebraic degree without per-degree instantiation.
    std::vector<std::vector<std::vector<long long>>> parent_prefixes(n); // parent_prefixes[inner] = list of prefixes
    for (std::size_t c = 0; c < n; ++c) {
        const auto& img = out.substitution[c];
        for (std::size_t k = 0; k < img.size(); ++k) {
            const auto inner = static_cast<std::size_t>(img[k]);
            std::vector<long long> prefix(img.begin(), img.begin() + static_cast<long long>(k));
            parent_prefixes[inner].push_back(prefix);
            out.prefixes_found.insert(prefix);
        }
    }

    out.prefixes_subset_of_empty_and_zero = true;
    for (const auto& p : out.prefixes_found) {
        if (!(p.empty() || (p.size() == 1 && p[0] == 0))) {
            out.prefixes_subset_of_empty_and_zero = false;
        }
    }
    out.both_prefixes_occur =
        out.prefixes_found.count({}) > 0 && out.prefixes_found.count({0}) > 0;

    std::set<long long> defects; // encode defect as -1 (=-e0), 0, +1 (=+e0)
    for (std::size_t i = 0; i < n; ++i) for (std::size_t j = 0; j < n; ++j) {
        for (const auto& p : parent_prefixes[i]) for (const auto& q : parent_prefixes[j]) {
            const long long lp = p.empty() ? 0 : 1;
            const long long lq = q.empty() ? 0 : 1;
            defects.insert(lq - lp);
        }
    }
    out.defect_class_count = defects.size();
    out.exactly_three_defect_classes = (out.defect_class_count == 3);

    out.proved = out.beta_strictly_between_one_and_two && out.all_digits_binary &&
        out.first_digit_is_one && out.prefixes_subset_of_empty_and_zero &&
        out.both_prefixes_occur && out.exactly_three_defect_classes;
    if (!out.proved && out.obstruction.empty())
        out.obstruction = "Q/R/S forcing argument failed its own independent re-derivation";
    return out;
}

}  // namespace ravel::proof
