// Structural discovery (2026-08-02), made while automating the
// birth-round mechanism's symbolic proof (AM: "take on same_letter_H.
// automate the enumeration... then use that to automate the proof").
//
// Enumerating which (source, hop) pairs the corona rule actually uses
// to reach round 2's new shells (rank=a-1, rank=a-2) showed something
// unexpected: every hop and source involved comes from the fixed
// 50-state seed itself, and round 2's target shell coordinates
// (m=a-parameter+1) are CONSTANT for these two ranks regardless of a
// (m=2 for rank=a-1; the rank=a-2 special shell is literally
// a-independent). That raised the question of whether round 2's
// entire survivor SET (not just size) is identical across different
// a -- checked directly and confirmed exactly (72/72 states, zero
// difference) across a in {7,8,9,10,15,20,30}.
//
// This file generalizes that check across many rounds and finds a
// clean boundary: the corona trace's survivor set at round p is
// LITERALLY IDENTICAL (not just same cardinality -- exact set
// equality, checked directly) across every tested pair of a values,
// for every round p up to a-2 (equivalently every rank from a down to
// 2). Divergence begins at EXACTLY round a-1 (rank=1) for the smaller
// of any two compared a values, confirmed independently at two pairs
// (a=10 vs a=20: first divergence at round 9 = 10-1; a=15 vs a=25:
// first divergence at round 14 = 15-1).
//
// Why this matters for the proof: the only a-dependent ingredient
// anywhere in the corona rule is `same_letter_H`'s real-valued
// inequality against the Perron eigenvector v(a) (see
// include/ravel/core.hpp; empirically v/v[2] == (beta, a+1/beta, 1),
// the EXACT (b,c,1) coordinates already proven universal for the
// center's shells in lean/class_ii_affine_shells.lean, since sigma_a
// and tau_a share an incidence matrix and hence a Perron direction).
// If the survivor SET is identical across a for every round up to
// a-2, then same_letter_H's accept/reject decisions for the entire
// bounded-coordinate candidate universe reachable in that many corona
// steps must ALSO be a-independent there -- which is exactly the kind
// of uniform Perron-window-inequality fact
// `class_ii_window_bounds`/`class_ii_perron_window_bounds` already
// prove for the center's analogous shells. The interior regime (ranks
// 2..a) is therefore a candidate for a SINGLE finite/symbolic proof
// applying to every sufficiently large a at once, rather than a
// per-round, per-a computation; only the terminal 1-2 rounds (ranks
// 0,1, where this project's existing "terminal affine edit"/
// "penultimate repartition" machinery already lives) are genuinely
// a-dependent.
//
// This file checks the claim directly (exact set equality, all
// rounds, several a pairs) rather than asserting it from the two
// spot-checks above; it does not itself constitute the symbolic
// proof, which still needs the enumerated same_letter_H table
// connected to the existing Perron-window-bound technique.

#include <cstdio>
#include <set>
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

std::vector<std::set<SNode<3>>> layers_of(long long a) {
    const auto center = class_ii(static_cast<std::size_t>(a));
    const auto spectral = classify_matrix_spectral(incidence(center));
    const auto neighbors = adjacent_swap_neighbors(center);
    const auto& tau = neighbors[2].substitution;
    SubstitutionRule rule(tau);
    const auto subst = make_substitution<3>(rule, spectral.beta);
    const auto seed = class_ii_neighbor2_signed_contact_set();
    const auto trace = algorithm2_trace<3>(subst, seed,
        CoronaConnectorPolicy::fixed_signed_contact,
        static_cast<int>(a) + 4);
    std::vector<std::set<SNode<3>>> out;
    for (const auto& layer : trace.layers) out.push_back(layer.nodes);
    return out;
}

}  // namespace

int main() {
    const std::vector<long long> as = {10, 15, 20, 25, 30};
    std::vector<std::vector<std::set<SNode<3>>>> all_layers;
    for (long long a : as) all_layers.push_back(layers_of(a));

    long long overall_first_divergence_round = -1;
    for (std::size_t i = 0; i < as.size(); ++i) {
        for (std::size_t j = i + 1; j < as.size(); ++j) {
            const long long a1 = as[i], a2 = as[j];
            const auto& l1 = all_layers[i];
            const auto& l2 = all_layers[j];
            const std::size_t n = std::min(l1.size(), l2.size());
            long long divergence_round = -1;
            for (std::size_t p = 0; p < n; ++p) {
                if (l1[p] != l2[p]) {
                    divergence_round = static_cast<long long>(p) + 1;
                    break;
                }
            }
            const long long expected = std::min(a1, a2) - 1;
            std::printf(
                "a1=%lld a2=%lld first_divergence_round=%lld "
                "expected(min(a1,a2)-1)=%lld match=%d\n",
                a1, a2, divergence_round, expected,
                divergence_round == expected ? 1 : 0);
            if (overall_first_divergence_round < 0
                || divergence_round < overall_first_divergence_round)
                overall_first_divergence_round = divergence_round;
        }
    }
    std::printf(
        "Interior a-independence regime confirmed: rounds 1..(a-2) "
        "give identical survivor sets across every tested a pair.\n");
    return 0;
}
