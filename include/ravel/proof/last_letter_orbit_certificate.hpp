// ravel/proof/last_letter_orbit_certificate.hpp
//
// The exact dual of first_letter_orbit_certificate.hpp for the SUFFIX
// side of Finding 39/41: tracks the LAST letter of each letter's
// iterated substitution image (`lastLetterMap(a) := images[a].back()`),
// reducing suffix coincidence to orbit collision the same way the
// prefix side reduces to `firstLetterMap` orbit collision. Instantiates
// `RavelGenerated.last_letter_orbit_collision_forces_coincidence`
// (lean/last_letter_orbit_coincidence.lean).

#pragma once

#include <array>
#include <cstddef>
#include <string>
#include <vector>

#include "math/proof_reflection.hpp"

namespace ravel::proof {

struct LastLetterOrbitResult {
    bool found{};
    long long k{-1};
    std::string note;
};

template <std::size_t d>
inline LastLetterOrbitResult certify_last_letter_orbit_collision(
    const std::array<std::vector<long long>, d>& images, long long i, long long j) {
    LastLetterOrbitResult out;
    for (const auto& img : images) {
        if (img.empty()) { out.note = "an image is empty; premise does not apply"; return out; }
    }
    long long bound = 2 * static_cast<long long>(d) + 2;
    long long xi = i, xj = j;
    for (long long k = 0; k <= bound; ++k) {
        if (xi == xj) {
            out.found = true;
            out.k = k;
            out.note = "lastLetterMap^[" + std::to_string(k) + "](" + std::to_string(i)
                + ") = lastLetterMap^[" + std::to_string(k) + "](" + std::to_string(j)
                + ") = " + std::to_string(xi) + " -- verified exactly";
            if (mathlib::reflection::enabled()) {
                mathlib::reflection::LastLetterOrbitCertificate node;
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
        xi = images[static_cast<std::size_t>(xi)].back();
        xj = images[static_cast<std::size_t>(xj)].back();
    }
    out.note = "no collision found within bound " + std::to_string(bound) + "; not recorded.";
    return out;
}

}  // namespace ravel::proof
