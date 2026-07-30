// hyperplane_progression.cpp
//
// Hyperplane-first view of the Pisot polytope H_sigma.
//
// For each candidate 3-letter Pisot substitution, enumerate the
// polytope
//
//   P_j := {x in Z^3 : 0 <= <x, v> < v_j}
//
// for j = 0, 1, 2, where v is the right Perron eigenvector of the
// incidence matrix M.  Tabulate, per substitution:
//
//   - the Perron root beta and the eigenvector (v_0, v_1, v_2)
//     (normalized v_0 = 1);
//   - the count of integer points x in P_j, for each j, in a
//     bounding box [-bound, bound]^3;
//   - the minimum and maximum of <x, v> attained in each P_j;
//   - the "width order" ratios v_j / v_{j+1} that drive the
//     stepped-hyperplane geometry.
//
// This is the "shape in the hyperplane" before any contact-boundary
// filter.  Comparing these counts across substitutions exposes the
// discrete parameter shifts that change the qualitative outcome
// family (Pisot property, dominant core size, contact graph order).

#include <array>
#include <cstdio>
#include <cstdint>
#include <vector>
#include <cmath>
#include <set>

#include "ravel/core.hpp"
#include "ravel/d_cont_check.hpp"
#include "ravel/spectral.hpp"
#include "ravel/survey.hpp"
#include "ravel/substitution.hpp"
#include "ravel/contact_boundary.hpp"

using namespace ravel;

namespace {

using Sigma = std::vector<std::vector<std::int8_t>>;

// Substitution constructors for the candidate family.
Sigma sigma_ab(int a, int b) {
    Sigma s(3);
    for (int i = 0; i < a; ++i) s[0].push_back(0);
    for (int i = 0; i < b; ++i) s[0].push_back(1);
    s[0].push_back(2);
    for (int i = 0; i < a; ++i) s[1].push_back(0);
    s[1].push_back(2);
    s[2] = {0};
    return s;
}

Sigma n_bonacci(int n) {
    Sigma s(n);
    for (int i = 0; i + 1 < n; ++i) s[i] = {0, static_cast<std::int8_t>(i + 1)};
    s[n - 1] = {0};
    return s;
}

Sigma sigma1_3l() { return {{0, 0, 1}, {0, 2}, {0}}; }
Sigma sigma2_3l() { return {{0, 0, 1}, {0, 0, 2}, {0}}; }

// Build a Substitution from a sigma table.
using Matrix = std::vector<std::vector<long long>>;
Matrix incidence(const Sigma& s) {
    Matrix M(3, std::vector<long long>(3, 0));
    for (std::size_t j = 0; j < 3; ++j) {
        for (auto letter : s[j])
            ++M[static_cast<std::size_t>(letter)][j];
    }
    return M;
}
Substitution<3> build_subst(const Sigma& s) {
    SubstitutionRule rule(s);
    const auto spectral = classify_matrix_spectral(incidence(s));
    return make_substitution<3>(rule, spectral.beta);
}

// Tabulate the hyperplane polytope for a single substitution.
struct PolytopeStats {
    long long count_per_color[3];
    double min_height_per_color[3];
    double max_height_per_color[3];
    std::set<std::array<long long, 3>> sample_points;
};

PolytopeStats scan_polytope(const Substitution<3>& subst, long long bound) {
    PolytopeStats s{};
    for (int j = 0; j < 3; ++j) {
        s.count_per_color[j] = 0;
        s.min_height_per_color[j] = 1e18;
        s.max_height_per_color[j] = -1e18;
    }
    for (long long x0 = -bound; x0 <= bound; ++x0) {
        for (long long x1 = -bound; x1 <= bound; ++x1) {
            for (long long x2 = -bound; x2 <= bound; ++x2) {
                std::array<long long, 3> x{x0, x1, x2};
                for (std::size_t j = 0; j < 3; ++j) {
                    if (!subst.in_H_sigma_exact(
                            x, j)) continue;
                    double h = x[0] * subst.v[0] + x[1] * subst.v[1]
                        + x[2] * subst.v[2];
                    s.count_per_color[j]++;
                    if (h < s.min_height_per_color[j])
                        s.min_height_per_color[j] = h;
                    if (h > s.max_height_per_color[j])
                        s.max_height_per_color[j] = h;
                    if (s.sample_points.size() < 8)
                        s.sample_points.insert(x);
                }
            }
        }
    }
    return s;
}

void report(const char* name, const Substitution<3>& subst, long long bound) {
    std::printf("=== %s ===\n", name);
    std::printf("  beta = %.6f\n", subst.beta);
    std::printf("  v = (%.6f, %.6f, %.6f)\n",
                subst.v[0], subst.v[1], subst.v[2]);
    const double r_01 = subst.v[0] / subst.v[1];
    const double r_12 = subst.v[1] / subst.v[2];
    std::printf("  width ratios v_0/v_1 = %.6f, v_1/v_2 = %.6f\n",
                r_01, r_12);
    // Balanced-pivot deviation: zero iff σ_{a,1} = σ_{1,1}.
    // (Kernel-checked in lean/class_ii_balanced_pivot.lean.)
    std::printf("  balanced-pivot deviation |r_01 - r_12| = %.2e\n",
                std::abs(r_01 - r_12));
    auto stats = scan_polytope(subst, bound);
    std::printf("  bound = %lld, |P_0| = %lld, |P_1| = %lld, |P_2| = %lld\n",
                bound, stats.count_per_color[0],
                stats.count_per_color[1], stats.count_per_color[2]);
    std::printf("  height ranges:  P_0 [%.4f, %.4f]  P_1 [%.4f, %.4f]  P_2 [%.4f, %.4f]\n",
                stats.min_height_per_color[0], stats.max_height_per_color[0],
                stats.min_height_per_color[1], stats.max_height_per_color[1],
                stats.min_height_per_color[2], stats.max_height_per_color[2]);
    // Print first few sample points per color.
    std::printf("  sample integer points in P_0: ");
    int shown = 0;
    for (const auto& x : stats.sample_points) {
        std::array<long long, 3> ax = x;
        if (subst.in_H_sigma_exact(ax, 0)) {
            std::printf("(%lld,%lld,%lld) ", x[0], x[1], x[2]);
            if (++shown >= 5) break;
        }
    }
    std::printf("\n\n");
}

}  // namespace

int main() {
    // The progression: simple (Tribonacci), then sigma_{a,1} family,
    // then a parameter-shift sibling sigma_{1,2}, then the two
    // singular 3-letter counterexamples sigma_1 and sigma_2.
    report("Tribonacci n=3 (control)", build_subst(n_bonacci(3)), 3);

    for (int a = 0; a <= 5; ++a) {
        char name[64];
        std::snprintf(name, sizeof(name), "sigma_{%d,1}", a);
        report(name, build_subst(sigma_ab(a, 1)), 3);
    }

    report("sigma_{1,2}", build_subst(sigma_ab(1, 2)), 3);
    report("sigma_1 (3L)", build_subst(sigma1_3l()), 3);
    report("sigma_2 (3L)", build_subst(sigma2_3l()), 3);

    // The "simple-to-complex" extension: a progression that
    // starts from the Fibonacci-like sigma_{0,1} and adds b, the
    // trailing-1 count, and finally the third-letter corrections
    // that produce sigma_1 and sigma_2.
    report("--- simple-to-complex progression ---", build_subst(sigma_ab(0, 1)), 3);
    report("sigma_{0,2}  -- b-shift sibling", build_subst(sigma_ab(0, 2)), 3);
    report("sigma_{2,0}  -- empty b family", build_subst(sigma_ab(2, 0)), 3);

    return 0;
}