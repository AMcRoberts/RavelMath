// adelic_boundary_spectral_radius.cpp
//
// Tests the working hypothesis from the follow-up to
// docs/FINDINGS_FOR_CITATION.md Finding 1: that lambda(G_B) > beta
// for 10/11 clean non-unimodular 4-letter candidates is an artifact
// of computing G_B in R^{d-1} alone, when the substitution's true
// contracting space (for a non-unit Pisot substitution) is the full
// adelic K_sigma = R^{d-1} x K_p (Minervino-Thuswaldner).  If so, the
// "zero-expansion graph" that check_property_f already builds --
// which lives in the COMBINED archimedean + p-adic bounded region --
// is the correct adelic-extended analogue of G_B, and ITS dominant
// eigenvalue should compare more favorably to bp_rho_nc than the
// plain real-space lambda(G_B) does.
//
// This is a first-cut experiment, not a proof: the zero-expansion
// graph was built for a DIFFERENT purpose (deciding property (F): is
// there a mixed zero/nonzero cycle) and its dominant eigenvalue may
// or may not be the mathematically "correct" quantity to compare
// against bp_rho_nc -- that identification itself is part of what
// this experiment is testing, not a known equivalence.
//
// Run for rnd13, the only candidate whose full data (images, Q(beta)
// ring, incidence matrix) survived in this project's tree; the
// other 10 candidates from CB_4x4_QBETA_RESULTS.md were never
// persisted to disk (only their summary statistics survived into
// the archived markdown), so they cannot be re-tested without either
// regenerating an equivalent fresh batch (see
// app/adelic_boundary_spectral_radius_survey.cpp, a natural
// follow-up not yet built) or recovering the originals.

#include <cstdio>
#include <vector>

#include "adelic/coincidence_and_property_f.hpp"
#include "adelic/prefix_automaton.hpp"
#include "math/linalg_qbeta.hpp"
#include "math/qbeta.hpp"
#include "ravel/ambient_graph.hpp"
#include "ravel/balanced_pair.hpp"
#include "ravel/contact_boundary.hpp"
#include "ravel/core.hpp"
#include "ravel/corona.hpp"
#include "ravel/d_cont_check.hpp"
#include "ravel/faces.hpp"
#include "ravel/graph_divisor.hpp"
#include "ravel/substitution.hpp"
#include "ravel/survey.hpp"

using namespace adelic;
using namespace ravel;

int main() {
    constexpr std::size_t d = 4;

    // rnd13: sigma(0)=(0,0,1,2,3,3), sigma(1)=(0,0,2,3,3),
    // sigma(2)=(0,0,3,3), sigma(3)=(0,0,0,2,3,3).  charpoly
    // x^4-4x^3-8x^2-6x-2, (2) = p^4 (single totally ramified prime).
    std::array<std::vector<long long>, d> images = {
        std::vector<long long>{0, 0, 1, 2, 3, 3},
        std::vector<long long>{0, 0, 2, 3, 3},
        std::vector<long long>{0, 0, 3, 3},
        std::vector<long long>{0, 0, 0, 2, 3, 3},
    };
    mathlib::QBetaRing R = mathlib::QBetaRing::from_low_first({-4, -8, -6, -2});
    std::vector<std::vector<long long>> Mt = {
        {2, 1, 1, 2},
        {2, 0, 1, 2},
        {2, 0, 0, 2},
        {3, 0, 1, 2},
    };
    auto eig = mathlib::right_eigenvector_via_qbeta(Mt, R);
    if (!eig.ok) {
        std::fprintf(stderr, "eigenvector computation failed\n");
        return 1;
    }
    auto automaton = build_prefix_automaton<d>(images, eig.v, R);
    std::printf("rnd13: digit set size = %zu\n", automaton.digit_set.size());

    auto padic_bound = make_totally_ramified_padic_bound(2, 4, R.charpoly());
    std::vector<std::vector<long long>> adelic_adj_sparse;  // node -> successor ids (repeats = multiplicity)
    auto propf = check_property_f<d>(automaton, 100000, padic_bound, &adelic_adj_sparse);
    std::printf("property (F): holds=%d inconclusive=%d nodes=%lld\n",
                propf.holds, propf.inconclusive, propf.nodes_explored);

    if (adelic_adj_sparse.empty()) {
        std::fprintf(stderr, "no adjacency captured (budget exceeded?); aborting.\n");
        return 1;
    }

    // Build a sparse WeightedDigraph directly from the adjacency
    // LIST (NOT via WeightedDigraph::from_dense, which would need an
    // n x n matrix -- infeasible at ~33000 nodes; a dense matrix that
    // size is tens of gigabytes, confirmed by an OOM kill on the
    // first attempt at this experiment).
    WeightedDigraph adelic_graph(adelic_adj_sparse.size());
    for (std::size_t u = 0; u < adelic_adj_sparse.size(); ++u) {
        for (long long w : adelic_adj_sparse[u]) {
            adelic_graph.add_edge(u, static_cast<std::size_t>(w), 1);
        }
    }
    double lambda_adelic = dominant_eigenvalue_estimate_sparse(adelic_graph, 8000, 1e-12);
    std::printf("lambda(adelic zero-expansion graph) ~= %.6f  (n=%zu nodes)\n",
                lambda_adelic, adelic_graph.n);

    // Plain real-space G_B, via the ordinary contact-boundary
    // pipeline (docs/FINDINGS_FOR_CITATION.md Finding 1's own
    // number: 5.48502).
    long long M[4][4] = {
        {2, 2, 2, 3},
        {1, 0, 0, 0},
        {1, 1, 0, 1},
        {2, 2, 2, 2},
    };
    std::vector<std::vector<std::int8_t>> sigma(4);
    for (int c = 0; c < 4; ++c)
        for (int letter = 0; letter < 4; ++letter)
            for (long long k = 0; k < M[letter][c]; ++k)
                sigma[c].push_back(static_cast<std::int8_t>(letter));
    SubstitutionRule rule(sigma);
    double beta = 5.623559;

    auto d_cont_cands = search_D_cont<4>(make_substitution<4>(rule, beta), 2);
    std::vector<std::tuple<long long, std::vector<long long>, long long>> d_cont;
    for (const auto& c : d_cont_cands) {
        std::vector<long long> xvec(c.x.begin(), c.x.end());
        d_cont.emplace_back(c.i, std::move(xvec), c.j);
    }
    auto rep = compute_contact_boundary_dispatch(rule, beta, 0.0, d_cont);

    auto bp = balanced_pair_transition_graph(rule);

    std::printf("\n--- Summary for rnd13 (beta = %.6f) ---\n", beta);
    std::printf("  BP-rho_nc                                 : %.6f\n", rep.bp_rho_nc);
    std::printf("  lambda(G_B), plain real-space R^{d-1}      : %.6f  (ratio to beta: %.4f)\n",
                rep.boundary_eigenvalue, rep.boundary_eigenvalue / beta);
    std::printf("  lambda(adelic zero-expansion graph, K_sigma): %.6f  (ratio to beta: %.4f)\n",
                lambda_adelic, lambda_adelic / beta);
    std::printf("  beta                                       : %.6f\n", beta);

    double gap_plain = std::abs(rep.boundary_eigenvalue - rep.bp_rho_nc);
    double gap_adelic = std::abs(lambda_adelic - rep.bp_rho_nc);
    std::printf("\n  |lambda(G_B, plain) - BP-rho_nc|  = %.6f\n", gap_plain);
    std::printf("  |lambda(adelic)     - BP-rho_nc|  = %.6f\n", gap_adelic);
    if (gap_adelic < gap_plain) {
        std::printf("  => the adelic-extended graph's eigenvalue is CLOSER to BP-rho_nc "
                    "than the plain real-space G_B's is -- consistent with the missing-\n"
                    "     p-adic-factor hypothesis, though not proof of it (a numeric fact "
                    "about ONE candidate, not a general result).\n");
    } else {
        std::printf("  => the adelic-extended graph's eigenvalue is NOT closer to BP-rho_nc; "
                    "the missing-p-adic-factor hypothesis is NOT supported by this candidate.\n");
    }

    (void)bp;  // captured for completeness/future divisor comparison; not used further here
    return 0;
}
