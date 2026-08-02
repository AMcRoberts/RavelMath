// Batch check (2026-08-02) of every same_letter_H fact round 2's birth
// mechanism needs, extending the single worked example verified by
// hand (target i=0,x=(-2,2,0),j=0) to the FULL table of source-gate
// and destination-gate tests the round-1-to-2 corona step actually
// uses.
//
// Method: enumerate every (source, hop) witness pair reaching each of
// round 2's six new shell states (rank=a-1's four regular-shell states,
// rank=a-2's two special-shell states -- see
// app/class_ii_neighbor2_round2_birth_mechanism_check.cpp), collect the
// DISTINCT (x, j) pairs that get tested by same_letter_H along the way
// (both the source gate and the destination gate), then for each
// distinct pair sweep a huge range of `a` (7 through 200000, ~194
// log-spaced samples) using the real `same_letter_H`/`in_H_sigma`
// implementation directly -- no reimplementation, no rounding risk.
//
// Correction folded in after the first run (2026-08-02, see TODAY.md
// "Correction"): the first pass's header claimed margins "grow like a
// and 2a^2" for the worked example. That was checking a beta-SCALED
// quantity, not the actual (normalized, v[0]=1) margin the real
// implementation compares against. The true closed form, via the
// cubic: `b-c = a/beta + 1/beta^2` exactly (a sum of two strictly
// positive terms), giving margin `= 2(b-c)/b`, and `margin*a -> 2` as
// `a -> infinity`. The margin genuinely shrinks like O(1/a) -- real,
// not an artifact -- but never reaches zero, so the inequality holds
// for every a in the domain regardless. The fast (double-precision)
// path DOES spuriously fail once that shrinking margin drops below
// floating-point noise (observed starting around a=100000 in the first
// run); this version cross-checks any fast-path failure against
// `in_H_sigma_exact` (the project's existing bit-exact Sturm/Q(beta)
// path, built for exactly this class of near-boundary precision
// problem) before reporting a real failure.
//
// This is deliberately NOT yet a Lean proof: it is the numerical batch
// AM asked for ("put a batch together and run it"), extending the one
// hand-verified worked example's evidence from one case to the whole
// round-2 table. The diagnostic that matters is margin*a converging to
// a small positive constant (the provable shape), not margin staying
// large or growing.

#include <cmath>
#include <cstdio>
#include <set>
#include <string>
#include <vector>

#include "ravel/class_ii_neighbor_family.hpp"
#include "ravel/corona.hpp"
#include "ravel/pisot_substitution_properties.hpp"
#include "ravel/spectral.hpp"
#include "ravel/substitution.hpp"
#include "ravel/substitution_neighborhood.hpp"

using namespace ravel;

namespace {

using Matrix = std::vector<std::vector<long long>>;

FiniteSubstitution class_ii(std::size_t a) {
    FiniteSubstitution result(3);
    result[0].assign(a, 0);
    result[0].push_back(1);
    result[0].push_back(2);
    result[1].assign(a, 0);
    result[1].push_back(2);
    result[2] = {0};
    return result;
}

Matrix incidence(const FiniteSubstitution& substitution) {
    Matrix matrix(3, std::vector<long long>(3, 0));
    for (std::size_t image = 0; image < 3; ++image)
        for (const auto letter : substitution[image])
            ++matrix[static_cast<std::size_t>(letter)][image];
    return matrix;
}

Substitution<3> subst_for(long long a) {
    const auto center = class_ii(static_cast<std::size_t>(a));
    const auto spectral = classify_matrix_spectral(incidence(center));
    const auto neighbors = adjacent_swap_neighbors(center);
    const auto& tau = neighbors[2].substitution;
    SubstitutionRule rule(tau);
    return make_substitution<3>(rule, spectral.beta);
}

// Every distinct (x, j) pair that needs a same_letter_H test to
// reach round 2's six new shell states, collected at a=7 (the
// (x, j) pairs themselves are a-independent for round 2 -- checked
// separately by the round-a-independence tool).
std::set<std::pair<std::array<long long, 3>, long long>> collect_round2_tests() {
    const long long a = 7;
    const auto subst = subst_for(a);
    const auto seed = class_ii_neighbor2_signed_contact_set();

    std::set<SNode<3>> targets;
    for (auto& n : class_ii_neighbor_regular_shell_states(2, a, a - 1)) targets.insert(n);
    for (auto& n : class_ii_neighbor_special_shell_states(2, a, 1)) targets.insert(n);

    std::set<std::pair<std::array<long long, 3>, long long>> tests;
    for (const auto& target : targets) {
        for (const auto& source : seed) {
            if (!same_letter_H<3>(subst, source.x, static_cast<std::size_t>(source.j)))
                continue;
            for (const auto& hop : seed) {
                if (hop.i != source.j) continue;
                SNode<3> cand;
                cand.i = source.i;
                for (int k = 0; k < 3; ++k) cand.x[k] = source.x[k] + hop.x[k];
                cand.j = hop.j;
                if (cand.i == target.i && cand.x == target.x && cand.j == target.j) {
                    if (!same_letter_H<3>(subst, cand.x, static_cast<std::size_t>(cand.j)))
                        continue;
                    tests.insert({source.x, source.j});
                    tests.insert({cand.x, cand.j});
                }
            }
        }
    }
    return tests;
}

std::vector<long long> log_spaced_a_values() {
    std::vector<long long> out;
    for (long long a = 7; a <= 50; ++a) out.push_back(a);
    for (long long a = 60; a <= 500; a += 10) out.push_back(a);
    for (long long a = 600; a <= 5000; a += 100) out.push_back(a);
    for (long long a = 6000; a <= 50000; a += 1000) out.push_back(a);
    for (long long a = 60000; a <= 200000; a += 10000) out.push_back(a);
    return out;
}

}  // namespace

int main() {
    const auto tests = collect_round2_tests();
    std::printf("Collected %zu distinct (x,j) same_letter_H tests from round 2.\n\n",
                tests.size());

    const auto a_values = log_spaced_a_values();
    long long total_failures = 0;

    for (const auto& [x, j] : tests) {
        std::printf("test x=(%lld,%lld,%lld) j=%lld:\n", x[0], x[1], x[2], j);

        // Determine which branch passes at the smallest a, then track
        // whether the SAME branch keeps passing (and its margin trend)
        // across the whole sweep.
        bool first = true;
        bool baseline_direct = false;
        double prev_margin = 0.0, first_margin = 0.0, last_margin = 0.0;
        bool all_consistent = true;
        long long checked = 0;

        for (long long a : a_values) {
            const auto subst = subst_for(a);
            const bool direct = subst.in_H_sigma(x, static_cast<std::size_t>(j));
            std::array<long long, 3> negx{-x[0], -x[1], -x[2]};
            const bool mirror = subst.in_H_sigma(negx, static_cast<std::size_t>(j));
            const bool ok = direct || mirror;
            ++checked;

            if (!ok) {
                // The fast (double-precision) path can spuriously fail
                // once the true margin (which shrinks like O(1/a) for
                // these fixed, a-independent interior-regime states --
                // see TODAY.md 2026-08-02 "Correction") drops below
                // floating-point noise. Cross-check with the exact
                // (Sturm/Q(beta)) path before reporting a real failure.
                const bool direct_exact = subst.in_H_sigma_exact(x, static_cast<std::size_t>(j));
                const bool mirror_exact = subst.in_H_sigma_exact(negx, static_cast<std::size_t>(j));
                if (direct_exact || mirror_exact) {
                    std::printf(
                        "  a=%lld: fast path failed (precision artifact, confirmed by "
                        "exact path: direct=%d mirror=%d) -- not a real failure\n",
                        a, direct_exact, mirror_exact);
                    continue;
                }
                std::printf("  a=%lld: FAILS same_letter_H entirely, confirmed by exact "
                            "path too (real failure)\n", a);
                all_consistent = false;
                ++total_failures;
                continue;
            }
            if (first) {
                baseline_direct = direct;
                first = false;
            } else if (direct != baseline_direct && !(direct && mirror)) {
                // Branch flipped from the baseline and it isn't simply
                // "both now pass" -- flag it.
                std::printf("  a=%lld: BRANCH CHANGED from baseline (direct=%d mirror=%d)\n",
                            a, direct, mirror);
                all_consistent = false;
            }

            // Margin of the branch that matters (whichever passes),
            // using the same dot_v/bound the real implementation uses.
            const auto& use_x = baseline_direct ? x : negx;
            const double height = subst.dot_v(use_x);
            const double width = subst.dot_v(subst.tile_faces[static_cast<std::size_t>(j)]);
            const double lower_margin = height;          // height - 0
            const double upper_margin = width - height;  // width - height
            const double margin = std::min(lower_margin, upper_margin);
            if (a == a_values.front()) first_margin = margin;
            last_margin = margin;
            prev_margin = margin;
        }
        (void)prev_margin;

        // A margin shrinking like O(1/a) is the EXPECTED, provable shape
        // for these fixed-coordinate interior-regime tests (see TODAY.md
        // 2026-08-02 "Correction": b-c = a/beta + 1/beta^2 exactly, a sum
        // of two strictly positive terms, so margin=2(b-c)/b -> 0 but
        // never reaches it). Report margin*a_last as the real diagnostic:
        // it should converge to a small positive constant, not to zero.
        const double last_a = static_cast<double>(a_values.back());
        std::printf("  branch=%s checked=%lld consistent=%d "
                    "margin(first)=%.6f margin(last)=%.8f margin(last)*a(last)=%.4f\n",
                    baseline_direct ? "direct" : "mirror", checked,
                    all_consistent ? 1 : 0, first_margin, last_margin,
                    last_margin * last_a);
    }

    std::printf("\nTotal outright failures (neither branch, any a): %lld\n", total_failures);
    if (total_failures == 0)
        std::printf("Every round-2 same_letter_H test passes at every sampled a "
                    "from 7 to 200000.\n");
    return 0;
}
