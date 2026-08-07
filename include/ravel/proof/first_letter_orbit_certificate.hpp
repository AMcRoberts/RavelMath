// ravel/proof/first_letter_orbit_certificate.hpp
//
// C++ certificate for Finding 39/41's GENERAL case (beyond the
// same-chain special case in zero_run_forces_bounded_coincidence.hpp):
// tracks only the FIRST LETTER of each letter's iterated substitution
// image, reducing "does this pair of letters eventually coincide" to
// orbit collision in the finite functional graph
// `firstLetterMap(a) := images[a][0]` -- covering mixed in-run/
// out-of-run and cross-run pairs the same-chain lemma cannot, exactly
// via `RavelGenerated.first_letter_orbit_collision_forces_coincidence`
// (lean/first_letter_orbit_coincidence.lean).
//
// Given a substitution's full image data and two letters i, j, this
// simulates `firstLetterMap` from both simultaneously (bounded by 2d,
// safely more than enough for a finite functional graph on d letters)
// and finds the smallest depth k with firstLetterMap^[k](i) ==
// firstLetterMap^[k](j) -- verified EXACTLY (integer letter indices,
// no floating point), not merely asserted to exist.

#pragma once

#include <array>
#include <cstddef>
#include <string>
#include <vector>

#include "math/proof_reflection.hpp"

namespace ravel::proof {

struct FirstLetterOrbitResult {
    bool found{};
    long long k{-1};
    std::string note;
};

template <std::size_t d>
inline FirstLetterOrbitResult certify_first_letter_orbit_collision(
    const std::array<std::vector<long long>, d>& images, long long i, long long j) {
    FirstLetterOrbitResult out;
    for (const auto& img : images) {
        if (img.empty()) { out.note = "an image is empty; premise does not apply"; return out; }
    }
    long long bound = 2 * static_cast<long long>(d) + 2;
    long long xi = i, xj = j;
    for (long long k = 0; k <= bound; ++k) {
        if (xi == xj) {
            out.found = true;
            out.k = k;
            out.note = "firstLetterMap^[" + std::to_string(k) + "](" + std::to_string(i)
                + ") = firstLetterMap^[" + std::to_string(k) + "](" + std::to_string(j)
                + ") = " + std::to_string(xi) + " -- verified exactly";
            if (mathlib::reflection::enabled()) {
                mathlib::reflection::FirstLetterOrbitCertificate node;
                node.d = static_cast<long long>(d);
                node.images.assign(images.begin(), images.end());
                node.i = i;
                node.j = j;
                node.k = k;
                node.description = "letters " + std::to_string(i) + " and " + std::to_string(j)
                    + " on a " + std::to_string(d) + "-letter alphabet";
                mathlib::reflection::record(mathlib::reflection::NodeKind::LemmaApplication, node);
            }
            return out;
        }
        xi = images[static_cast<std::size_t>(xi)][0];
        xj = images[static_cast<std::size_t>(xj)][0];
    }
    out.note = "no collision found within bound " + std::to_string(bound)
        + " -- should be impossible for this project's canonical family (every non-pass-through "
          "letter maps directly to letter 0 under firstLetterMap); not recorded.";
    return out;
}

}  // namespace ravel::proof
