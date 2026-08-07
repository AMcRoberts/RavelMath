// ravel/proof/constant_last_letter_forces_depth1_coincidence.hpp
//
// The exact dual of constant_factor_forces_depth1_coincidence.hpp
// (Finding 17), found while proving the missing foundation stone for
// Finding 37: every one of Finding 37's 10 genuine non-unit Pisot
// substitutions in Barge's (2016) class ("injective on first letters,
// constant on last letter") resolved strong coincidence at depth 1 --
// but that was only ever OBSERVED, not proved, for that class. This
// closes it, by the SUFFIX half of `pair_has_coincidence`'s check
// (definition 2.1's condition is P(p1)=P(p2) OR P(s1)=P(s2) -- Finding
// 17 used the prefix half; this uses the suffix half).
//
// THEOREM (general, no Pisot/irreducibility/injective-first-letters
// hypothesis needed at all -- strictly weaker than Barge's own
// hypothesis, which additionally needs injective first letters for
// HIS theorem's different conclusion, pure discrete spectrum; this
// theorem only needs the LAST-letter condition, for the narrower
// claim of depth-1 coincidence): if sigma(i) and sigma(j) end with the
// same letter c, the pair (i,j) exhibits a coincidence at k=1,
// unconditionally, via the LAST occurrence of c in each word.
//
// PROOF. Read `adelic::pair_has_coincidence`'s suffix loop directly:
// for word w1, at its FINAL position (the last letter, i=w1.size()-1,
// necessarily w1[i]=c under this hypothesis), `running` has counted
// every letter's occurrences strictly BEFORE this position, so
// `suffix = total1 - running - e_c` computes exactly "how many letters
// remain AFTER this position" -- which is the ZERO vector, since this
// IS the last position. That zero vector is inserted into
// `suffix_set1[c]`. When w2 is scanned and ITS final position is
// reached (w2's last letter is also c, by hypothesis), the identical
// computation gives the zero vector again, and
// `suffix_set1[c].count(suffix)` finds the one just inserted from w1
// -- a hit, unconditionally, regardless of anything about either word
// before its last position. This is definitional, exactly mirroring
// Finding 17's prefix argument with "first" and "prefix" replaced by
// "last" and "suffix" throughout.
//
// CONSEQUENCE: "constant last letter" (every alphabet letter's image
// ends with the same letter) is ALSO a sufficient condition for
// depth-1 strong coincidence, independent of and complementary to
// Finding 17's "constant first letter" condition -- the two overlap
// only for length-1 images. Combined, they cover every substitution in
// Barge's (2016) structural class (which requires constant last letter
// as half its hypothesis) for the coincidence half of the tiling
// argument, closing exactly the gap Finding 37 left open: strong
// coincidence for that whole class is now PROVED, not merely observed
// on 10 examples.

#pragma once

#include <array>
#include <cstddef>
#include <string>
#include <vector>

namespace ravel::proof {

struct ConstantLastLetterCoincidenceCertificate {
    bool has_constant_last_letter{};   // every image shares the same last letter
    long long constant_letter{-1};     // that shared letter, if has_constant_last_letter
    std::string note;
};

// Checks the PREMISE directly from the substitution's images, exactly
// mirroring check_constant_factor_forces_depth1's own structure.
template <std::size_t d>
inline ConstantLastLetterCoincidenceCertificate check_constant_last_letter_forces_depth1(
    const std::array<std::vector<long long>, d>& images) {
    ConstantLastLetterCoincidenceCertificate out;
    for (const auto& img : images) {
        if (img.empty()) { out.note = "an image is empty; premise does not apply"; return out; }
    }
    long long c0 = images[0].back();
    out.has_constant_last_letter = true;
    for (std::size_t i = 1; i < d; ++i) {
        if (images[i].back() != c0) { out.has_constant_last_letter = false; break; }
    }
    if (out.has_constant_last_letter) out.constant_letter = c0;

    out.note = out.has_constant_last_letter
        ? "every image shares last letter " + std::to_string(c0)
          + " -- ALL pairs resolve strong coincidence at depth 1, unconditionally, "
            "via the suffix half of the coincidence check"
        : "images do not all share a last letter -- this theorem does not "
          "predict depth-1 resolution (coincidence may still hold, at higher "
          "depth or via the prefix half, by a different route)";
    return out;
}

}  // namespace ravel::proof
