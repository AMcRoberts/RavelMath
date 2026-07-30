// test_bp_gb_divisor.cpp
//
// Runs the graph_divisor.hpp comparison between the balanced-pair
// automaton's noncoincidence transition graph and G_B's own
// adjacency graph, for two 4-letter cases:
//
//   1. Tetrabonacci (unimodular Pisot): bp_rho_nc == lambda(G_B)
//      holds numerically (both 1.6234 to 6 decimal places).
//      If the two graphs share a common coarsest-equitable-partition
//      quotient, that would be a STRUCTURAL reason for the equality,
//      not a coincidence -- this is the case the divisor hypothesis
//      predicts should succeed.
//
//   2. rnd13 (non-unimodular Pisot, |det|=2): bp_rho_nc (4.9007) !=
//      lambda(G_B) (5.48502) in plain R^{d-1}. The divisor hypothesis
//      predicts this should FAIL to find a common quotient in the
//      plain real-space G_B -- consistent with the working
//      hypothesis (docs/DIRECTION_AND_OPEN_THREADS.md follow-up)
//      that the real-space G_B is missing the p-adic factor of the
//      substitution's true adelic representation space K_sigma.
//
// This is a FIRST-CUT structural test, not a proof either way: a
// negative result on the coarsest-equitable-partition test does not
// rule out a subtler (non-equitable-partition) relationship between
// the two graphs; a positive result on Tetrabonacci is evidence for,
// not a proof of, the general divisor hypothesis.  See
// graph_divisor.hpp's header comment for exactly what is and is not
// established by a match/mismatch here.

#include <cstdio>
#include <vector>

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

namespace {

using namespace ravel;

// Mirrors app/contact_boundary_4x4.cpp's matrix_to_rule (sorted-by-
// letter canonical substitution from a 4x4 incidence matrix).
std::vector<std::vector<std::int8_t>> matrix_to_rule(const long long M[4][4]) {
    std::vector<std::vector<std::int8_t>> sigma(4);
    for (int c = 0; c < 4; ++c) {
        for (int letter = 0; letter < 4; ++letter) {
            for (long long k = 0; k < M[letter][c]; ++k) {
                sigma[c].push_back(static_cast<std::int8_t>(letter));
            }
        }
    }
    return sigma;
}

void run_case(const char* name, const long long M[4][4], double beta) {
    std::printf("\n=== %s (beta=%.6f) ===\n", name, beta);

    auto sigma = matrix_to_rule(M);
    SubstitutionRule rule(sigma);

    // --- Balanced-pair transition graph.
    auto bp = balanced_pair_transition_graph(rule);
    if (!bp.terminated) {
        std::printf("  balanced-pair BFS did not terminate within caps; skipping.\n");
        return;
    }
    std::printf("  BP transition graph: %zu noncoincidence nodes\n", bp.matrix.size());

    // --- G_B via the standard contact-boundary pipeline.
    std::vector<DCandidate<4>> d_cont_cands =
        search_D_cont<4>(make_substitution<4>(rule, beta), /*bound=*/2);
    std::vector<std::tuple<long long, std::vector<long long>, long long>> d_cont;
    for (const auto& c : d_cont_cands) {
        std::vector<long long> xvec(c.x.begin(), c.x.end());
        d_cont.emplace_back(c.i, std::move(xvec), c.j);
    }
    auto rep = compute_contact_boundary_dispatch(rule, beta, 0.0, d_cont);
    std::printf("  G_B: %zu nodes, bp_rho_nc=%.6f, lambda(G_B)=%.6f\n",
                rep.gb_matrix.size(), rep.bp_rho_nc, rep.boundary_eigenvalue);

    if (bp.matrix.empty() || rep.gb_matrix.empty()) {
        std::printf("  one of the two graphs is empty; skipping divisor comparison.\n");
        return;
    }

    // --- Divisor comparison.
    auto bp_graph = WeightedDigraph::from_dense(bp.matrix);
    auto gb_graph = WeightedDigraph::from_dense(rep.gb_matrix);
    auto cmp = compare_divisors(bp_graph, gb_graph);

    std::printf("  BP coarsest quotient:  %d classes\n", cmp.classes_a);
    std::printf("  G_B coarsest quotient: %d classes\n", cmp.classes_b);
    if (cmp.used_exact_charpoly) {
        std::printf("  quotient charpolys match (exact): %s\n", cmp.charpoly_match ? "YES" : "no");
    } else {
        std::printf("  quotients too large for exact charpoly (>120 classes); "
                    "using power-iteration dominant eigenvalue instead\n");
    }
    std::printf("  BP quotient dominant eigenvalue:  %.6f\n", cmp.dominant_eigenvalue_a);
    std::printf("  G_B quotient dominant eigenvalue: %.6f\n", cmp.dominant_eigenvalue_b);
    std::printf("  dominant eigenvalues match (numeric): %s\n",
                cmp.dominant_eigenvalue_match ? "YES" : "no");
    if (cmp.checked_perm) {
        std::printf("  quotients match up to permutation (brute-force, both <= 8 classes): %s\n",
                    cmp.quotient_permutation_match ? "YES" : "no");
    } else {
        std::printf("  permutation check skipped (a quotient has > 8 classes)\n");
    }

    bool structural_match = cmp.used_exact_charpoly ? cmp.charpoly_match : cmp.dominant_eigenvalue_match;
    if (structural_match) {
        std::printf("  => STRUCTURAL EVIDENCE for a common divisor: bp_rho_nc == lambda(G_B) "
                    "would follow from Perron-Frobenius, not need per-case checking.\n");
    } else {
        std::printf("  => NO common coarsest quotient found: the two graphs are not related "
                    "by a simple equitable-partition divisor in their CURRENT form -- consistent "
                    "with each graph's OWN internal symmetry being insufficient to explain the "
                    "eigenvalue relationship; a real common-divisor witness (if one exists) would "
                    "need an explicit cross-graph map, not just each graph's own automorphism orbits.\n");
    }
}

}  // namespace

int main() {
    // Tetrabonacci: sigma(0)=(0,1), sigma(1)=(0,2), sigma(2)=(0,3),
    // sigma(3)=(0).  M = [[1,1,1,1],[1,0,0,0],[0,1,0,0],[0,0,1,0]].
    static const long long M_tetra[4][4] = {
        {1, 1, 1, 1},
        {1, 0, 0, 0},
        {0, 1, 0, 0},
        {0, 0, 1, 0},
    };
    run_case("Tetrabonacci", M_tetra, 1.927561975482925);

    // rnd13: sigma(0)=(0,0,1,2,3,3), sigma(1)=(0,0,2,3,3),
    // sigma(2)=(0,0,3,3), sigma(3)=(0,0,0,2,3,3).
    // M = [[2,2,2,3],[1,0,0,0],[1,1,0,1],[2,2,2,2]].
    static const long long M_rnd13[4][4] = {
        {2, 2, 2, 3},
        {1, 0, 0, 0},
        {1, 1, 0, 1},
        {2, 2, 2, 2},
    };
    run_case("rnd13", M_rnd13, 5.623559);

    return 0;
}
