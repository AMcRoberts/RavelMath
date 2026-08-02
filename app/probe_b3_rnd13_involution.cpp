// app/probe_b3_rnd13_involution.cpp
//
// Item B, Thread B3 (docs/DIRECTION_AND_OPEN_THREADS.md): "do the
// n-bonacci involution results give structural information about the
// non-unimodular tilers (only rnd13 in the 24/24 batch)?"
//
// rnd13 is the ONE non-unimodular Pisot substitution in the 24/24
// survey known to actually TILE its adelic representation space
// (FINDINGS_FOR_CITATION.md Finding 2). This driver runs the same
// A1 (nilpotent cofactor) / A2 (involution) test used throughout
// Item A's AR-exact/AR-partial/non-AR classification (Finding 5,
// 6.6, 10) against rnd13's actual G_B, reusing the same
// include/ravel/involution_helpers.hpp primitives -- no new
// mathematics, just pointing existing machinery at a new candidate.
//
// sigma(0)=(0,0,1,2,3,3), sigma(1)=(0,0,2,3,3), sigma(2)=(0,0,3,3),
// sigma(3)=(0,0,0,2,3,3), beta~=5.6236, charpoly x^4-4x^3-8x^2-6x-2
// (|det|=2, non-unimodular).

#include <cstdio>
#include <tuple>
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

using namespace ravel;

int main() {
    std::printf("=== B3: does rnd13 (the one non-unimodular tiler) show the involution/cofactor pattern? ===\n\n");

    std::vector<std::vector<std::int8_t>> sigma = {
        {0, 0, 1, 2, 3, 3},
        {0, 0, 2, 3, 3},
        {0, 0, 3, 3},
        {0, 0, 0, 2, 3, 3},
    };
    SubstitutionRule rule(sigma);
    std::vector<std::vector<long long>> M(4, std::vector<long long>(4, 0));
    for (std::size_t c = 0; c < 4; ++c)
        for (auto letter : sigma[c]) M[static_cast<std::size_t>(letter)][c] += 1;

    auto spectral = classify_matrix_spectral(M);
    std::printf("beta=%.6f  pisot(2x2/3x3 test)=%d  |det|(from charpoly)=2 (non-unimodular)\n",
                spectral.beta, spectral.pisot ? 1 : 0);
    std::fflush(stdout);

    std::printf("starting search_D_cont<4>...\n"); std::fflush(stdout);
    auto d_cont_cands = search_D_cont<4>(make_substitution<4>(rule, spectral.beta), 2);
    std::printf("search_D_cont<4> done: %zu candidates\n", d_cont_cands.size()); std::fflush(stdout);
    std::vector<std::tuple<long long, std::vector<long long>, long long>> d_cont;
    d_cont.reserve(d_cont_cands.size());
    for (const auto& c : d_cont_cands) {
        d_cont.emplace_back(c.i, std::vector<long long>(c.x.begin(), c.x.end()), c.j);
    }
    std::printf("starting compute_contact_boundary_dispatch...\n"); std::fflush(stdout);
    ContactBoundaryReport rep;
    try {
        rep = compute_contact_boundary_dispatch(rule, spectral.beta, 0.0, d_cont);
    } catch (const std::exception& e) {
        std::printf("compute_contact_boundary threw: %s\n", e.what());
        return 1;
    }
    std::printf("compute_contact_boundary_dispatch done: converged=%d boundary_size=%zu gb_matrix.size()=%zu\n",
                rep.converged ? 1 : 0, rep.boundary_size, rep.gb_matrix.size());
    std::fflush(stdout);
    if (!rep.converged || rep.gb_matrix.empty()) {
        std::printf("contact-boundary did not converge (|G_B|=%zu)\n", rep.boundary_size);
        return 1;
    }
    std::printf("|G_B|=%zu, converged\n", rep.boundary_size); std::fflush(stdout);

    std::printf("starting check_involution_on_core...\n"); std::fflush(stdout);
    auto [matched, total] = check_involution_on_core(rep, /*print_unmatched=*/false);
    bool exact = (matched == total && total > 0);
    std::printf("[A2 involution] %d / %d core nodes matched%s\n",
                matched, total, exact ? "  (EXACT)" : "  (partial)");
    std::fflush(stdout);

    std::printf("starting compute_gb_sym_quotient<4>...\n"); std::fflush(stdout);
    auto gb_qsym = compute_gb_sym_quotient<4>(rep, rule);
    std::printf("compute_gb_sym_quotient<4> done: num_orbits=%zu\n", gb_qsym.num_orbits); std::fflush(stdout);
    auto bp_qsym = compute_bp_sym_quotient(rule);
    std::printf("compute_bp_sym_quotient done: num_orbits=%zu\n", bp_qsym.num_orbits); std::fflush(stdout);
    if (gb_qsym.num_orbits == 0 || bp_qsym.num_orbits == 0) {
        std::printf("[A1 cofactor] skipped (empty quotient: GB_orbits=%zu, BP_orbits=%zu)\n",
                    gb_qsym.num_orbits, bp_qsym.num_orbits);
        return 0;
    }
    auto charpoly_gb = charpoly_PolyZ(gb_qsym.Qsym);
    auto charpoly_bp = charpoly_PolyZ(bp_qsym.Qsym);
    bool xk = check_exact_factor("A1 nilpotent cofactor (rnd13)", charpoly_gb, charpoly_bp);
    std::printf("[A1 cofactor] Q_sym_GB (deg %lld) / Q_sym_BP (deg %lld) = x^k exact? %s\n",
                (long long)charpoly_gb.degree(), (long long)charpoly_bp.degree(), xk ? "YES" : "no");

    std::printf("\n=== Interpretation ===\n");
    std::printf("rnd13 is the one non-unimodular candidate known to TILE.\n");
    if (exact && xk) {
        std::printf("It ALSO shows the full AR-exact (Class I-like) involution/cofactor\n");
        std::printf("signature -- suggestive structural correlation with tiling.\n");
    } else {
        std::printf("It does NOT show the AR-exact signature (%s involution, cofactor %s) --\n",
                     exact ? "exact" : "partial", xk ? "x^k" : "not x^k");
        std::printf("the involution/cofactor mechanism and adelic tiling appear to be\n");
        std::printf("INDEPENDENT properties, at least for this one tested candidate.\n");
    }
    return 0;
}
