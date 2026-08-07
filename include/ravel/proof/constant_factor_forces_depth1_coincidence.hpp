// ravel/proof/constant_factor_forces_depth1_coincidence.hpp
//
// Explains, exactly (not just empirically), why Finding 5's AR-partial
// family (sigma_{a,1}, a=1..5) resolves strong coincidence at depth 1
// for EVERY pair, while the non-AR case (sigma_{0,1}) needs depth 13:
// AR-partial has a "constant factor at position 0" (every letter's
// image begins with the same fixed letter -- Finding 5's own table
// already recorded this as a yes/no column without connecting it to
// coincidence depth); non-AR does not.
//
// THEOREM (general, no Pisot/unimodularity hypothesis needed at all):
// if sigma(i) and sigma(j) begin with the same letter c, then the
// pair (i,j) exhibits a coincidence at k=1, unconditionally, via the
// FIRST occurrence of c in each word.
//
// PROOF. Read `adelic::pair_has_coincidence`'s own loop
// (`coincidence_and_property_f.hpp`): `running` starts at the zero
// vector and is only incremented AFTER recording the prefix set for
// the current letter. So processing w1's very first letter (w1[0]=c)
// inserts the zero vector into `prefix_set1[c]`. When w2 is scanned
// and its first letter (w2[0]=c) is reached, `running` is STILL the
// zero vector (nothing has been processed yet), so
// `prefix_set1[c].count(running)` finds the zero vector that was just
// inserted -- a hit, unconditionally, regardless of anything about
// either word beyond position 0. This is definitional, not a property
// of Pisot substitutions, beta-expansions, or any specific family --
// it holds for ANY two words over ANY alphabet sharing a first
// letter. Verified directly against the actual coincidence-detection
// function (not re-derived independently) in the accompanying test.
//
// CONSEQUENCE for the open general unimodular Pisot tiling question:
// "constant factor at position 0" (every alphabet letter's image
// begins with the same letter) is a SUFFICIENT condition for strong
// coincidence to hold trivially, at depth 1, for the WHOLE
// substitution (every pair, not just one) -- with no search needed at
// all. Any hypothetical counterexample to the classical Pisot tiling
// conjecture among unimodular Pisot substitutions can therefore only
// come from the non-constant-factor (Finding 5's "non-AR") class:
// strong coincidence is never the obstruction for constant-factor
// substitutions. This narrows, but does not close, the open case --
// property (F) is untouched by this argument, and non-constant-factor
// substitutions can still resolve strong coincidence by other routes
// (a shared LATER occurrence, or a matching SUFFIX abelianization
// instead of a prefix one) at higher depth, exactly as sigma_{0,1}
// does at depth 13.

#pragma once

#include <array>
#include <cstddef>
#include <string>
#include <vector>

#include "math/proof_reflection.hpp"

namespace ravel::proof {

struct ConstantFactorCoincidenceCertificate {
    bool has_constant_factor{};       // every image shares the same first letter
    long long constant_letter{-1};    // that shared letter, if has_constant_factor
    bool every_pair_shares_first_letter{}; // the strictly weaker, per-pair version
    std::string note;
};

// Checks the PREMISE directly from the substitution's images (not from
// a separately-run coincidence search) -- this is what makes it a
// certificate rather than a re-statement of the search's own output.
template <std::size_t d>
inline ConstantFactorCoincidenceCertificate check_constant_factor_forces_depth1(
    const std::array<std::vector<long long>, d>& images) {
    ConstantFactorCoincidenceCertificate out;
    for (const auto& img : images) {
        if (img.empty()) { out.note = "an image is empty; premise does not apply"; return out; }
    }
    long long c0 = images[0][0];
    out.has_constant_factor = true;
    for (std::size_t i = 1; i < d; ++i) {
        if (images[i][0] != c0) { out.has_constant_factor = false; break; }
    }
    if (out.has_constant_factor) out.constant_letter = c0;

    out.every_pair_shares_first_letter = true;
    for (std::size_t i = 0; i < d; ++i)
        for (std::size_t j = i + 1; j < d; ++j)
            if (images[i][0] != images[j][0]) out.every_pair_shares_first_letter = false;
    // (identical to has_constant_factor for the whole-alphabet case;
    // kept separate because a future caller might supply only a
    // subset of pairs, where the weaker per-pair fact is what matters)

    out.note = out.has_constant_factor
        ? "every image shares first letter " + std::to_string(c0)
          + " -- ALL pairs resolve strong coincidence at depth 1, unconditionally"
        : "images do not all share a first letter -- this theorem does not "
          "predict depth-1 resolution (coincidence may still hold, at higher "
          "depth, by a different route)";

    if (out.has_constant_factor) {
        mathlib::reflection::ConstantFirstLetterCertificate node;
        node.d = static_cast<long long>(d);
        node.constant_letter = c0;
        node.images.assign(images.begin(), images.end());
        node.description = "d=" + std::to_string(d) + " letters, constant first letter " + std::to_string(c0);
        mathlib::reflection::record(mathlib::reflection::NodeKind::LemmaApplication, node);
    }
    return out;
}

}  // namespace ravel::proof
