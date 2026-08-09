// ravel/proof/adjacent_swap_count_certificate.hpp
//
// Wires `ravel::certify_class_ii_adjacent_swap_count`/
// `ravel::certify_nbonacci_adjacent_swap_count` (already-existing,
// already-correct checks in `family_closed_forms.hpp` that independently
// construct the substitution, invoke `adjacent_swap_neighbors`, and compare
// the exact count) to the reflection pipeline: if the check passes, records
// the CONCRETE images the substitution actually produced, so the renderer
// can have Lean recompute the adjacent-unequal-site count from those exact
// images via `decide`, not merely restate the number.
//
// `lean/family_of_families_adjacent_swap_counts.lean`'s previous version
// checked a `Finset.univ`-cardinality placeholder unrelated to any actual
// substitution image -- archived (`lean/archive/`) as flat and superseded
// by this certificate + `render_adjacent_swap_count_instances`.

#pragma once

#include <string>
#include <vector>

#include "math/proof_reflection.hpp"
#include "ravel/family_closed_forms.hpp"

namespace ravel::proof {

inline void stage_class_ii_adjacent_swap_count(std::size_t a, std::size_t b,
                                                const std::string& description) {
    if (!ravel::certify_class_ii_adjacent_swap_count(a, b)) return;
    if (!mathlib::reflection::enabled()) return;
    const auto sigma = ravel::class_ii_sigma_ab(a, b);
    mathlib::reflection::AdjacentSwapCountCertificate node;
    node.family = "class_ii";
    node.a = static_cast<long long>(a);
    node.b = static_cast<long long>(b);
    for (const auto& word : sigma) {
        std::vector<long long> w;
        for (auto letter : word) w.push_back(static_cast<long long>(letter));
        node.images.push_back(std::move(w));
    }
    node.count = static_cast<long long>(ravel::class_ii_adjacent_swap_count_closed_form(a, b));
    node.description = description;
    mathlib::reflection::record(mathlib::reflection::NodeKind::LemmaApplication, node);
}

inline void stage_nbonacci_adjacent_swap_count(std::size_t n, const std::string& description) {
    if (!ravel::certify_nbonacci_adjacent_swap_count(n)) return;
    if (!mathlib::reflection::enabled()) return;
    const auto sigma = ravel::nbonacci_substitution(n);
    mathlib::reflection::AdjacentSwapCountCertificate node;
    node.family = "nbonacci";
    node.n = static_cast<long long>(n);
    for (const auto& word : sigma) {
        std::vector<long long> w;
        for (auto letter : word) w.push_back(static_cast<long long>(letter));
        node.images.push_back(std::move(w));
    }
    node.count = static_cast<long long>(ravel::nbonacci_adjacent_swap_count_closed_form(n));
    node.description = description;
    mathlib::reflection::record(mathlib::reflection::NodeKind::LemmaApplication, node);
}

}  // namespace ravel::proof
