// app/probe_a5_extend_data.cpp
//
// Thread A5 (docs/DIRECTION_AND_OPEN_THREADS.md): extend
// FINDINGS_FOR_CITATION.md Finding 5's 10-candidate data set to test
// whether the Class I/II/III decomposition is exhaustive.
//
// Deliberately a SEPARATE driver from app/probe_a1_a2_unimodular.cpp
// rather than an edit to it -- that file's exact 10-candidate grid is
// cited by Finding 5's own numbers and should stay reproducible as-is.
// This file reuses the SAME underlying primitives from
// include/ravel/involution_helpers.hpp (check_involution_on_core,
// compute_gb_sym_quotient, compute_bp_sym_quotient, check_exact_factor)
// -- only the driver glue (ProbeRow/run_one/matrix_from_subst) is
// duplicated, not the mathematics.
//
// Extension strategy:
//   1. Wider sigma_{a,b} grid (a=0..15, plus more b>=2 non-unimodular
//      cases) -- same family, more data points.
//   2. A handful of genuinely random UNIMODULAR (det=+-1) 3-letter
//      Pisot matrices, NOT from the sigma_{a,b} family -- the real
//      test of exhaustiveness, since every one of Finding 5's original
//      10 candidates came from just two structural families
//      (n-bonacci and sigma_{a,b}).

#include <cstdio>
#include <random>
#include <string>
#include <vector>

#include "math/charpoly.hpp"
#include "ravel/balanced_pair.hpp"
#include "ravel/contact_boundary.hpp"
#include "ravel/core.hpp"
#include "ravel/corona.hpp"
#include "ravel/d_cont_check.hpp"
#include "ravel/graph_divisor.hpp"
#include "ravel/involution_helpers.hpp"
#include "ravel/spectral.hpp"
#include "ravel/substitution.hpp"
#include "ravel/survey.hpp"
#include "math/bigint.hpp"

using namespace ravel;

namespace {

std::vector<std::vector<std::int8_t>> sigma_ab_rule(int a, int b) {
    std::vector<std::vector<std::int8_t>> sigma(3);
    sigma[0].reserve(a + b + 1);
    for (int i = 0; i < a; ++i) sigma[0].push_back(0);
    for (int i = 0; i < b; ++i) sigma[0].push_back(1);
    sigma[0].push_back(2);
    sigma[1].reserve(a + 1);
    for (int i = 0; i < a; ++i) sigma[1].push_back(0);
    sigma[1].push_back(2);
    sigma[2] = {0};
    return sigma;
}

inline std::vector<std::vector<long long>>
matrix_from_subst(const std::vector<std::vector<std::int8_t>>& sigma) {
    std::size_t n = sigma.size();
    std::vector<std::vector<long long>> M(n, std::vector<long long>(n, 0));
    for (std::size_t c = 0; c < n; ++c) {
        for (auto letter : sigma[c]) {
            if (letter >= 0 && static_cast<std::size_t>(letter) < n) {
                M[static_cast<std::size_t>(letter)][c] += 1;
            }
        }
    }
    return M;
}

long long det3(const std::vector<std::vector<long long>>& M) {
    return M[0][0]*(M[1][1]*M[2][2]-M[1][2]*M[2][1])
         - M[0][1]*(M[1][0]*M[2][2]-M[1][2]*M[2][0])
         + M[0][2]*(M[1][0]*M[2][1]-M[1][1]*M[2][0]);
}

struct ProbeRow {
    std::string label;
    bool involution_exact = false;
    int  involution_matched = 0;
    int  involution_total = 0;
    bool cofactor_is_xk = false;
    long cofactor_k = -1;
    bool was_pisot = false;
    bool contact_boundary_converged = false;
    long gb_size = 0;
};

ProbeRow run_one(const std::string& label,
                 const std::vector<std::vector<std::int8_t>>& sigma,
                 double beta) {
    ProbeRow r;
    r.label = label;
    SubstitutionRule rule(sigma);

    std::vector<std::vector<long long>> M = matrix_from_subst(sigma);
    auto spectral = classify_matrix_spectral(M);
    r.was_pisot = spectral.pisot;
    if (!r.was_pisot) {
        std::printf("\n[%s] not Pisot (alpha=%.4f); skipping probes\n",
                    label.c_str(), spectral.beta);
        return r;
    }

    auto d_cont_cands = search_D_cont<3>(make_substitution<3>(rule, beta), 2);
    std::vector<std::tuple<long long, std::vector<long long>, long long>> d_cont;
    d_cont.reserve(d_cont_cands.size());
    for (const auto& c : d_cont_cands) {
        d_cont.emplace_back(c.i, std::vector<long long>(c.x.begin(), c.x.end()), c.j);
    }
    ContactBoundaryReport rep;
    try {
        rep = compute_contact_boundary_dispatch(rule, beta, 0.0, d_cont);
    } catch (const std::exception& e) {
        std::printf("\n[%s] compute_contact_boundary threw: %s\n", label.c_str(), e.what());
        return r;
    }
    r.contact_boundary_converged = rep.converged;
    r.gb_size = static_cast<long>(rep.boundary_size);
    if (!rep.converged || rep.gb_matrix.empty()) {
        std::printf("\n[%s] contact-boundary did not converge (|G_B|=%zu)\n",
                    label.c_str(), rep.boundary_size);
        return r;
    }

    auto [matched, total] = check_involution_on_core(rep, /*print_unmatched=*/false);
    r.involution_matched = matched;
    r.involution_total = total;
    r.involution_exact = (matched == total && total > 0);
    std::printf("\n[%s] beta=%.6f  |G_B|=%zu\n", label.c_str(), beta, rep.boundary_size);
    std::printf("  [A2 involution]  %d / %d core nodes matched%s\n",
                matched, total, r.involution_exact ? "  (EXACT)" : "  (partial)");

    auto gb_qsym = compute_gb_sym_quotient<3>(rep, rule);
    auto bp_qsym = compute_bp_sym_quotient(rule);
    if (gb_qsym.num_orbits == 0 || bp_qsym.num_orbits == 0) {
        std::printf("  [A1 nilpotent cofactor] skipped (empty quotient)\n");
        return r;
    }
    auto charpoly_gb_qsym = charpoly_PolyZ(gb_qsym.Qsym);
    auto charpoly_bp_qsym = charpoly_PolyZ(bp_qsym.Qsym);
    bool exact = check_exact_factor("A1 nilpotent cofactor (Q_sym_GB / Q_sym_BP)",
                                     charpoly_gb_qsym, charpoly_bp_qsym);
    r.cofactor_is_xk = exact;
    if (exact) {
        r.cofactor_k = static_cast<long>(charpoly_gb_qsym.degree())
                     - static_cast<long>(charpoly_bp_qsym.degree());
    }
    return r;
}

}  // namespace

int main() {
    std::printf("=== Thread A5: extending Finding 5's data set ===\n\n");
    std::fflush(stdout);

    std::vector<ProbeRow> rows;
    int n_probed = 0, n_pisot = 0, n_invol_exact = 0, n_cofactor_xk = 0;

    auto probe = [&](const std::string& label,
                      const std::vector<std::vector<std::int8_t>>& sigma) {
        auto M = matrix_from_subst(sigma);
        auto spectral = classify_matrix_spectral(M);
        if (!spectral.pisot) {
            std::printf("[%s] not Pisot (alpha=%.4f); skipping\n", label.c_str(), spectral.beta);
            return;
        }
        ProbeRow r = run_one(label, sigma, spectral.beta);
        rows.push_back(r);
        n_probed++;
        if (r.was_pisot) n_pisot++;
        if (r.involution_exact) n_invol_exact++;
        if (r.cofactor_is_xk) n_cofactor_xk++;
    };

    // ---- Wider sigma_{a,b} grid: a=6..15 (Finding 5 covered a=0..5) ----
    std::printf("--- Wider sigma_{a,1} grid (a=6..15) ---\n");
    for (int a = 6; a <= 15; ++a) {
        probe("sigma_{" + std::to_string(a) + ",1}", sigma_ab_rule(a, 1));
    }

    // ---- More non-unimodular b>=2 cases (Class III territory) ----
    std::printf("\n--- More sigma_{a,b>=2} cases ---\n");
    for (auto [a, b] : std::vector<std::pair<int,int>>{{0,3},{1,3},{3,2},{4,2},{0,4}}) {
        probe("sigma_{" + std::to_string(a) + "," + std::to_string(b) + "}", sigma_ab_rule(a, b));
    }

    // ---- Genuinely random UNIMODULAR 3-letter Pisot matrices, NOT
    // from the sigma_{a,b} family -- the actual exhaustiveness test. ----
    std::printf("\n--- Random unimodular (det=+-1) 3-letter Pisot, outside sigma_{a,b} ---\n");
    std::mt19937 rng(11);
    int found = 0, tried = 0;
    while (found < 8 && tried < 20000) {
        ++tried;
        std::vector<std::vector<long long>> M(3, std::vector<long long>(3, 0));
        std::uniform_int_distribution<long long> d(0, 4);
        for (auto& row : M) for (auto& v : row) v = d(rng);
        long long det = det3(M);
        if (det != 1 && det != -1) continue;
        auto sigma = matrix_to_subst(M);
        if (sigma.empty()) continue;
        auto spectral = classify_matrix_spectral(M);
        if (!spectral.pisot) continue;
        ++found;
        probe("random_unimod_" + std::to_string(found), sigma);
    }
    std::printf("(random search: %d found in %d trials)\n", found, tried);

    std::printf("\n=== Summary ===\n");
    std::printf("%-24s  %-9s  %-14s  %s\n", "candidate", "Pisot?", "A2 invol.", "A1 x^k");
    for (auto& r : rows) {
        std::printf("%-24s  %-9s  %-14s  %s\n",
                    r.label.c_str(),
                    r.was_pisot ? "yes" : "no",
                    r.contact_boundary_converged
                        ? (r.involution_exact ? "EXACT" : "partial") : "n/a",
                    r.cofactor_is_xk ? ("YES(k=" + std::to_string(r.cofactor_k) + ")").c_str() : "no");
    }
    std::printf("\nTotals: %d probed, %d Pisot, %d A2-exact, %d A1-x^k\n",
                n_probed, n_pisot, n_invol_exact, n_cofactor_xk);
    return 0;
}
