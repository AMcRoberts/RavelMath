// family_of_families_test.cpp
//
// First pilot for the "family of families" research organization
// (see docs/FAMILY_OF_FAMILIES.md):
//
//   1. Construct several σ_{a,b} substitutions at varying (a, b),
//      plus the n-bonacci control (n=3 Tribonacci).
//   2. Compute the adjacent-swap neighborhood for each.
//   3. Verify the incidence matrix is preserved across each swap
//      (this is the "same light, different shape" property).
//   4. Report a fixed-light fiber summary: how many distinct
//      substitutions share each Parikh column, and how many of
//      them are reachable by adjacent swaps (vs needing deeper
//      edits).
//
// This is the minimum-viable fingerprint comparison: at the
// matrix level, are different families distinguishable? At the
// word-order level, is the swap graph dense or sparse?

#include <cstdio>
#include <set>
#include <string>
#include <vector>

#include "ravel/substitution_neighborhood.hpp"

namespace {

using Sub = ravel::FiniteSubstitution;

Sub sigma_ab(int a, int b) {
    Sub s(3);
    for (int i = 0; i < a; ++i) s[0].push_back(0);
    for (int i = 0; i < b; ++i) s[0].push_back(1);
    s[0].push_back(2);
    for (int i = 0; i < a; ++i) s[1].push_back(0);
    s[1].push_back(2);
    s[2].push_back(0);
    return s;
}

Sub n_bonacci(int n) {
    Sub s(n);
    for (int i = 0; i + 1 < n; ++i) s[i] = {0, static_cast<std::int8_t>(i + 1)};
    s[n - 1] = {0};
    return s;
}

struct FamilyEntry {
    std::string name;
    Sub substitution;
    std::vector<std::vector<std::size_t>> parikh;
};

bool analyze_family(const FamilyEntry& f) {
    const auto neighbors = ravel::adjacent_swap_neighbors(f.substitution);
    // Deduplicate by Parikh columns.
    std::set<std::vector<std::vector<std::size_t>>> fiber;
    fiber.insert(f.parikh);
    for (const auto& n : neighbors) {
        fiber.insert(ravel::parikh_columns(n.substitution));
    }
    // All swap neighbors should be in the same fiber (same light).
    bool all_in_fiber = (fiber.size() == 1);
    std::printf(
        "  %-20s: |alphabet|=%-3zu  Parikh=[%zu %zu; %zu %zu; %zu %zu]  "
        "|N_1swap|=%-2zu  fiber=%s  distinct=%zu\n",
        f.name.c_str(),
        f.substitution.size(),
        // Just the first 2 columns of the first 3 rows for display
        f.parikh[0][0], f.parikh[0][1],
        f.parikh[1][0], f.parikh[1][1],
        f.parikh[2][0], f.parikh[2][1],
        neighbors.size(),
        all_in_fiber ? "all-1swap" : "broken",
        fiber.size());
    if (!all_in_fiber) {
        // Should not happen per the substitution_neighborhood.hpp
        // contract, but report if it does.
        std::printf("    WARNING: adjacent-swap broke incidence matrix\n");
    }
    return all_in_fiber;
}

}  // namespace

int main() {
    std::printf("Family of families pilot (adjacent-swap fiber summary)\n");
    std::printf("Each substitution is connected to its neighbors by "
                "single adjacent-swap moves (preserves Parikh).\n\n");
    std::vector<FamilyEntry> families;
    // Class-II family sigma_{a,1} for a wide range: verify the
    // 3-neighbor count is uniform for a >= 1.
    for (int a = 0; a <= 10; ++a) {
        FamilyEntry f;
        f.name = "sigma_{" + std::to_string(a) + ",1}";
        f.substitution = sigma_ab(a, 1);
        f.parikh = ravel::parikh_columns(f.substitution);
        families.push_back(std::move(f));
    }
    // Class-II family sigma_{a,2}: same structural expectation.
    for (int a = 0; a <= 6; ++a) {
        FamilyEntry f;
        f.name = "sigma_{" + std::to_string(a) + ",2}";
        f.substitution = sigma_ab(a, 2);
        f.parikh = ravel::parikh_columns(f.substitution);
        families.push_back(std::move(f));
    }
    // n-bonacci control: n = 3, 4, 5, 6 to see if the
    // |N_1swap| = 2 (Tribonacci) generalizes.
    for (int n = 3; n <= 6; ++n) {
        FamilyEntry f;
        f.name = std::to_string(n) + "-bonacci";
        f.substitution = n_bonacci(n);
        f.parikh = ravel::parikh_columns(f.substitution);
        families.push_back(std::move(f));
    }

    bool ok = true;
    for (const auto& f : families)
        ok = analyze_family(f) && ok;

    return ok ? 0 : 1;
}
