// sweep_mismatches_property_f.cpp
//
// Step 1: sanity-check check_property_f's unit-case branch against
// Tribonacci (classical, independently known pure discrete spectrum
// -- Rauzy 1982) -- a SECOND cross-check beyond the Fibonacci one
// already in coincidence_and_property_f.hpp's own STATUS section,
// before trusting the sigma1/sigma2 result (check_sigma12_property_f.cpp)
// any further.
//
// Step 2: generate a fresh batch of random unimodular irreducible
// 3-letter Pisot substitutions (include/ravel/survey.hpp's own
// seeded generator -- reproducible, not the untracked ad-hoc survey
// from an earlier session that produced the "~1%-180%" mismatch
// figure in docs/RESEARCH_STATUS.md), find which ones have
// lambda(G_B) != BP-rho_nc (thread A's central-conjecture mismatch,
// via the same run_pipeline<3> machinery gb_bp_involution_general_n.cpp
// uses for sigma1/sigma2), then run strong coincidence + property (F)
// on every mismatch found -- looking for an INCONCLUSIVE or FAILS
// result, which sigma1/sigma2 did NOT produce (both resolved
// trivially). All charpoly/M_transpose inputs are derived
// programmatically (mathlib::charpoly_faddeev_leverrier + matrix
// transpose) rather than hand-computed, to avoid the by-hand
// arithmetic risk in check_sigma12_property_f.cpp.
//
// Build: g++ -std=c++20 -O2 -Iinclude -Imath/include -Imath/include/mini-gmp \
//        app/sweep_mismatches_property_f.cpp math/out/libmath.a -o sweep_mismatches_property_f

#include <array>
#include <cstdio>
#include <vector>

#include "math/charpoly.hpp"
#include "math/qbeta.hpp"
#include "math/linalg_qbeta.hpp"
#include "math/poly_z.hpp"
#include "adelic/dedekind_factorization.hpp"
#include "adelic/ideal_arithmetic.hpp"
#include "adelic/prefix_automaton.hpp"
#include "adelic/coincidence_and_property_f.hpp"
#include "ravel/survey.hpp"
#include "ravel/substitution.hpp"
#include "ravel/contact_boundary.hpp"

using namespace ravel;

namespace {

// Identical logic to app/classify_adelic_tiling.cpp's classify<d>(),
// but taking the charpoly as an already-built PolyZ and M_transpose
// as a plain matrix, both derived programmatically by the caller
// rather than typed in by hand.
template <std::size_t d>
void classify(const char* name,
              const std::array<std::vector<long long>, d>& images,
              const mathlib::PolyZ& charpoly,
              const std::vector<std::vector<long long>>& M_transpose) {
    printf("  -- classify(%s) --\n", name);
    mathlib::QBetaRing R(charpoly);
    printf("     charpoly: %s\n", mathlib::str(R.charpoly()).c_str());

    std::vector<std::vector<long long>> Mt = M_transpose;
    auto eig = mathlib::right_eigenvector_via_qbeta(Mt, R);
    if (!eig.ok) {
        printf("     FAILED to compute left Perron eigenvector; cannot proceed.\n");
        return;
    }
    auto automaton = adelic::build_prefix_automaton<d>(images, eig.v, R);

    auto coin = adelic::check_strong_coincidence<d>(images);
    printf("     Strong coincidence: %s (depth %lld%s)\n",
           coin.holds ? "HOLDS" : (coin.inconclusive ? "INCONCLUSIVE" : "FAILS"),
           coin.depth_reached, coin.inconclusive ? " [cutoff]" : "");

    adelic::PropertyFResult propf = adelic::check_property_f<d>(automaton, 300000);
    printf("     Property (F): %s (%lld nodes%s)\n",
           propf.holds ? "HOLDS" : (propf.inconclusive ? "INCONCLUSIVE" : "FAILS"),
           propf.nodes_explored, propf.inconclusive ? " [budget exhausted]" : "");

    printf("     Verdict: %s\n",
           (coin.holds && propf.holds) ? "PISOT CONJECTURE ESTABLISHED (via Canterini-Siegel + genuine tiling)"
           : (coin.inconclusive || propf.inconclusive) ? "INCONCLUSIVE -- genuinely open with this check"
           : "a check FAILED -- would need individual scrutiny (possible counterexample)");
}

std::vector<std::vector<long long>> transpose(const std::vector<std::vector<long long>>& M) {
    std::size_t n = M.size();
    std::vector<std::vector<long long>> T(n, std::vector<long long>(n, 0));
    for (std::size_t i = 0; i < n; ++i)
        for (std::size_t j = 0; j < n; ++j)
            T[j][i] = M[i][j];
    return T;
}

std::array<std::vector<long long>, 3> to_ll_images(const std::vector<std::vector<std::int8_t>>& sigma) {
    std::array<std::vector<long long>, 3> out;
    for (std::size_t i = 0; i < 3; ++i)
        for (auto c : sigma[i]) out[i].push_back(static_cast<long long>(c));
    return out;
}

// Identical to app/gb_bp_involution_general_n.cpp's own run_pipeline<d>().
template <std::size_t d>
ContactBoundaryReport run_pipeline(const SubstitutionRule& rule, double beta) {
    auto d_cont_cands = search_D_cont<d>(make_substitution<d>(rule, beta), 2);
    std::vector<std::tuple<long long, std::vector<long long>, long long>> d_cont;
    for (const auto& c : d_cont_cands) {
        std::vector<long long> xv(c.x.begin(), c.x.end());
        d_cont.emplace_back(c.i, std::move(xv), c.j);
    }
    return compute_contact_boundary_dispatch(rule, beta, 0.0, d_cont);
}

}  // namespace

int main() {
    // -----------------------------------------------------------------
    // Step 1: Tribonacci sanity check.
    // -----------------------------------------------------------------
    printf("=== Step 1: Tribonacci sanity check (known pure discrete spectrum) ===\n");
    {
        std::vector<std::vector<long long>> M = {
            {1, 1, 1},
            {1, 0, 0},
            {0, 1, 0},
        };
        auto charpoly = mathlib::charpoly_faddeev_leverrier(M);
        std::array<std::vector<long long>, 3> images = {
            std::vector<long long>{0, 1},
            std::vector<long long>{0, 2},
            std::vector<long long>{0},
        };
        classify<3>("Tribonacci", images, charpoly, transpose(M));
    }
    printf("\n");

    // -----------------------------------------------------------------
    // Step 2: fresh random unimodular 3-letter Pisot sweep.
    // -----------------------------------------------------------------
    printf("=== Step 2: fresh random unimodular 3-letter Pisot sweep ===\n");
    auto candidates = wide_random_pisot_survey(
        /*target=*/40, /*K_max=*/3, /*seed=*/11,
        /*max_pairs_per_certify=*/8000, /*max_len_per_certify=*/60000,
        /*max_pairs_for_rho=*/8000, /*max_len_for_rho=*/60000,
        /*max_trials=*/20000, /*alphabet_size=*/3);
    printf("Generated %zu Pisot 3-letter candidates.\n", candidates.size());

    int n_unimodular = 0, n_certified = 0, n_mismatch = 0;
    for (auto& inst : candidates) {
        long long det = inst.M[0][0] * (inst.M[1][1] * inst.M[2][2] - inst.M[1][2] * inst.M[2][1])
                       - inst.M[0][1] * (inst.M[1][0] * inst.M[2][2] - inst.M[1][2] * inst.M[2][0])
                       + inst.M[0][2] * (inst.M[1][0] * inst.M[2][1] - inst.M[1][1] * inst.M[2][0]);
        if (std::llabs(det) != 1) continue;
        n_unimodular++;
        if (!inst.certified) continue;
        n_certified++;

        SubstitutionRule rule(inst.sigma);
        ContactBoundaryReport rep = run_pipeline<3>(rule, inst.beta);
        double ratio = rep.boundary_eigenvalue / rep.bp_rho_nc;
        bool mismatch = std::abs(ratio - 1.0) >= 1e-4;
        printf("\n%s: BP-rho_nc=%.6f lambda(G_B)=%.6f ratio=%.6f (%s)\n",
               inst.name.c_str(), rep.bp_rho_nc, rep.boundary_eigenvalue, ratio,
               mismatch ? "MISMATCH" : "match");
        if (!mismatch) continue;
        n_mismatch++;

        auto charpoly = mathlib::charpoly_faddeev_leverrier(inst.M);
        auto images = to_ll_images(inst.sigma);
        classify<3>(inst.name.c_str(), images, charpoly, transpose(inst.M));
    }
    printf("\n=== Summary: %d unimodular, %d certified, %d mismatches found and classified ===\n",
           n_unimodular, n_certified, n_mismatch);
    return 0;
}
