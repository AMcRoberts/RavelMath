// ravel/proof/occurrence_length_sets.hpp
//
// The corrected, general (covers both terminating and eventually-
// periodic expansions) description of which letters are "rigid"
// (single parent-decomposition prefix length) versus "flexible"
// (more than one), and the provably-exact closed form this gives for
// the RAW defect set -- as opposed to `generator_set_from_digit_
// differences.hpp`'s formula, which is only exact for terminating
// expansions and only predicts the (harder) PRIMITIVE/post-domination
// count, not the raw one.
//
// Every letter m in a canonical beta-substitution (terminating,
// purely periodic, or eventually periodic) occurs as an image symbol
// at a set of prefix lengths occ(m):
//   - letter 0: pooled over every state c with digit[c] >= 1, at
//     every length 0..digit[c]-2 (the "0"-run positions strictly
//     before the trailing symbol) PLUS every length 0..digit[c]-1 for
//     states whose trailing symbol is ALSO 0 (only possible for the
//     purely-periodic/terminating last state, whose trailing "digit"
//     has no letter appended at all -- see
//     canonical_beta_substitution_from_digits). In practice this is
//     simplest to read directly off the constructed substitution
//     rather than re-derive from the digit sequence in the abstract;
//     this header does the former.
//   - the cycle-start letter p (ONLY when there is a genuine preperiod,
//     p = preperiod length > 0): occ(p) = {digit[p-1], digit[N-1]}
//     (its ordinary successor-position length, from state p-1, AND
//     the wraparound-closing length, from the last state) -- exactly
//     two elements (one, if they happen to coincide).
//   - every other nonzero letter m: occ(m) = {digit[m-1]}, a single
//     fixed value -- RIGID, no alternative, ever.
//
// The raw defect set (every difference b-a actually realized by some
// pair of parent decompositions, BEFORE checking which survive
// primitively) is then EXACTLY the set of pairwise differences
// {b-a : a in occ(i), b in occ(j), any letters i,j} -- this is exact
// by construction (it is simply what "defect" means, restated), not
// an approximation, and matches `raw_defect_classes` from
// `canonical_substitution_generator_collapse.hpp` exactly in every
// case checked (see tests/occurrence_length_sets_test.cpp).
//
// What this header does NOT give a closed form for: which of those
// raw defects survive as genuinely PRIMITIVE generators. That
// question is a reachability question in the role-transition graph
// (does some chain of unit-magnitude steps -- not just repeats of a
// single generator, ANY mix of {-1,0,+1} -- connect the same source
// and target role as the raw large-magnitude edge?), and despite a
// real attempt this session, no simpler description than actually
// checking that graph has been found. `generator_set_from_digit_
// differences.hpp`'s formula happens to answer it correctly for
// terminating expansions specifically (because there every nonzero
// letter is rigid, and a "both endpoints rigid" role provably has no
// other edge at all, so reachability trivially fails whenever the raw
// gap is >= 2); once a second letter becomes flexible (the eventually-
// periodic case), that short-circuit argument no longer applies.
//
// A natural next guess -- "the seam produces a genuine generator
// exactly when some rigid letter's value falls strictly outside the
// range spanned by the flexible junction letter's two values" -- was
// tested directly and falsified: x^4+0x^3-2x^2-2x+1 (preperiod 2,
// period 2, digits (2,1,2,0)) has rigid letters at value 2, strictly
// outside the junction letter's range [0,1], yet the raw defect-2
// class IS dominated (reduces to Q/R/S) -- decomposable via a route
// through letter 0's own unusually rich parent list (5 occurrences),
// which the naive range heuristic doesn't account for at all. So the
// determining factor is genuinely global role-graph connectivity, not
// a local numeric comparison on the letters directly involved in the
// large-defect edge. `canonical_substitution_generator_collapse.hpp`
// now implements the correct general check (full BFS reachability
// over {-1,0,+1}-labeled edges, not merely repeated application of one
// generator), validated in both directions (finds the known
// decomposition for reducible cases, fails to find one for the two
// confirmed genuine-generator witnesses) before being trusted.

#pragma once

#include <cstddef>
#include <set>
#include <vector>

namespace ravel::proof {

// occ[m] = set of prefix lengths at which letter m occurs as an image
// symbol anywhere in `sigma`.
inline std::vector<std::set<long long>> occurrence_length_sets(
    const std::vector<std::vector<long long>>& sigma) {
    const std::size_t n = sigma.size();
    std::vector<std::set<long long>> occ(n);
    for (std::size_t c = 0; c < n; ++c) {
        const auto& img = sigma[c];
        for (std::size_t k = 0; k < img.size(); ++k) {
            occ[static_cast<std::size_t>(img[k])].insert(static_cast<long long>(k));
        }
    }
    return occ;
}

// The provably-exact raw defect set: all differences b-a realized by
// some pair of occurrences of any two letters (including the same
// letter with itself, giving defect 0).
inline std::set<long long> raw_defect_set(const std::vector<std::vector<long long>>& sigma) {
    auto occ = occurrence_length_sets(sigma);
    std::set<long long> defects;
    for (const auto& oi : occ) for (const auto& oj : occ)
        for (long long a : oi) for (long long b : oj) defects.insert(b - a);
    return defects;
}

}  // namespace ravel::proof
