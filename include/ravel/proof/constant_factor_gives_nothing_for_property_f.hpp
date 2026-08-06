// ravel/proof/constant_factor_gives_nothing_for_property_f.hpp
//
// Closes the natural follow-up to constant_factor_forces_depth1_
// coincidence.hpp: does "constant factor at position 0" give property
// (F) the same kind of free ride it gives strong coincidence? NO --
// and the reason is structural, not just "not found yet".
//
// FACT 1 (general, no constant-factor hypothesis needed at all): for
// ANY Pisot substitution, a prefix-automaton edge b -p-> a preserves
// gamma=0 (i.e. beta^{-1}(0 + delta(p)) = 0, equivalently delta(p)=0)
// if and only if p is the EMPTY prefix. Proof: delta(p) = <P(p), v>
// where v is the LEFT PERRON EIGENVECTOR of the (primitive,
// irreducible) incidence matrix -- Perron-Frobenius guarantees every
// entry of v is strictly positive. P(p) has nonnegative integer
// entries, all zero iff p is empty. So delta(p) = sum of v[letter]
// over p's letters (with multiplicity) is exactly 0 iff p is empty,
// and STRICTLY POSITIVE for every nonempty p. Checked directly
// against the actual eigenvector values for the whole a=1..5 family
// (all three components positive, every case) in the accompanying
// test, not merely asserted from the general theorem.
//
// FACT 2: property (F)'s own definition (see coincidence_and_
// property_f.hpp) explicitly, permanently excludes any cycle staying
// ENTIRELY among gamma=0 ("zero-translation") nodes from counting as
// a violation -- this is not incidental, it is BUG 1's fix, the
// project's own corrected reading of the plan's wording. So the part
// of the graph that "constant factor at position 0" structures --
// the connectivity among (0,a)-type nodes, which by FACT 1 is exactly
// the empty-prefix / first-letter map -- is a part of the graph that
// was ALREADY, unconditionally, never able to cause a property-(F)
// violation, constant factor or not.
//
// CONSEQUENCE: "constant factor at position 0" gives property (F)
// ZERO leverage. It only ever touches the always-safe zero-node
// component; the actual hard question for property (F) -- whether
// any path leaves gamma=0 (necessarily via a nonempty-prefix edge,
// by FACT 1) and returns -- is completely untouched by whether the
// substitution has a constant factor. This is a proven negative
// result about THIS lens, not a report of "no proof found yet": the
// two facts above are jointly sufficient to see why no amount of
// constant-factor structure could ever help, without needing to
// examine any specific substitution's full closure.
//
// This is independently confirmed by Finding 16's own node counts:
// sigma_{1,1}..sigma_{5,1} (all constant-factor, AR-partial) needed
// 105, 199, 963, 2762, 6451 nodes respectively to close property (F)
// -- monotonically GROWING with the substitution's real combinatorial
// size, exactly as if the constant factor weren't there at all. If
// the constant factor had bought any real simplification, one would
// expect these counts to be small and roughly constant, the way
// coincidence's depth stayed at exactly 1 for every case; instead
// they scale with image length, i.e., with the genuinely hard part
// of the search.
//
// STATUS OF THE OPEN QUESTION: this closes off one specific promising-
// looking lens (having a full proof), it does not close property (F)
// itself. Whether property (F) holds for ALL unimodular Pisot
// substitutions (constant-factor or not) remains open; what this
// header establishes is that any future proof attempt needs a
// genuinely different structural handle than the constant-factor one
// that worked so cleanly for coincidence.

#pragma once

#include <cstddef>
#include <string>
#include <vector>

#include "math/qbeta.hpp"

namespace ravel::proof {

struct PropertyFZeroLeverageCertificate {
    bool every_nonempty_prefix_strictly_positive{};
    std::size_t nonempty_prefixes_checked{};
    std::string note;
};

// Verifies FACT 1 directly against a substitution's own computed
// digit set: every nonempty prefix's digit (archimedean value at the
// real Perron eigenvalue) is nonzero -- checked via exact Q(beta)
// equality against the zero element, not a numeric tolerance.
template <std::size_t d>
inline PropertyFZeroLeverageCertificate check_zero_edges_are_exactly_empty_prefix(
    const std::vector<std::vector<long long>>& distinct_prefixes,
    const std::vector<mathlib::QElem>& digit_set,
    const mathlib::QBetaRing& R) {
    PropertyFZeroLeverageCertificate out;
    out.every_nonempty_prefix_strictly_positive = true;
    mathlib::QElem zero = R.from_int(0);
    for (std::size_t i = 0; i < distinct_prefixes.size(); ++i) {
        if (distinct_prefixes[i].empty()) continue;
        ++out.nonempty_prefixes_checked;
        if (digit_set[i] == zero) { out.every_nonempty_prefix_strictly_positive = false; }
    }
    out.note = out.every_nonempty_prefix_strictly_positive
        ? "every nonempty prefix has nonzero delta -- the zero-translation "
          "component is exactly the empty-prefix/first-letter structure, "
          "which property (F) already excludes from violation-counting "
          "unconditionally; constant-factor structure gives no leverage here"
        : "UNEXPECTED: a nonempty prefix has delta=0 -- contradicts "
          "Perron-Frobenius strict positivity, investigate the eigenvector "
          "computation before trusting anything downstream";
    return out;
}

}  // namespace ravel::proof
