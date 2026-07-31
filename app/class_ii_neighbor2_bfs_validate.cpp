// Multi-round backward-closure BFS built from the already-generic,
// already-trusted class_ii_neighbor_transition_weight, validated
// against the CENTER's known-correct 9->16->14 chain before being
// trusted on the neighbor. Small case first, per the project's own
// "the first few instances usually show the pattern" discipline.
//
// Direction, confirmed by tracing class_ii_neighbor_prefix_families:
// transition_weight(neighbor, a, source, target) substitutes TARGET
// and searches for SOURCE inside its image -- target is the parent,
// source the child. So a backward-closure step from a known node Y
// looks for new parents X with transition_weight(neighbor, a, Y, X) > 0.

#include <cstdio>
#include <set>
#include <vector>

#include "ravel/class_ii_neighbor_family.hpp"
#include "ravel/contact_boundary.hpp"
#include "ravel/d_cont_check.hpp"
#include "ravel/spectral.hpp"
#include "ravel/substitution.hpp"
#include "ravel/substitution_neighborhood.hpp"
#include "ravel/survey.hpp"

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
    for (std::size_t image = 0; image < 3; ++image) {
        for (const auto letter : substitution[image]) {
            ++matrix[static_cast<std::size_t>(letter)][image];
        }
    }
    return matrix;
}

// Brute-force one BFS step: for a known node `y`, find every candidate
// parent x (over a bounded search box) with transition_weight(y, x) > 0
// AND with x itself a valid restricted node -- is_valid_anode's exact
// gate (corona.hpp:319): in_H_sigma(x,j) AND (x != 0 OR i < j). Missing
// this gate is what made the first version of this driver blow past
// its node cap: class_ii_neighbor_transition_weight alone is raw,
// unrestricted prefix arithmetic (correct for counting edges within an
// already-known-valid state set, as it's used elsewhere in this
// codebase) -- it is not itself the restricted-graph membership test.
// Any accepted x that touches the search box boundary is a live signal
// that the box is too small and the closure below is not trustworthy.
bool g_bound_touched = false;

bool is_valid_restricted(
        const Substitution<3>& subst, const SNode<3>& n) {
    const bool all_zero =
        n.x[0] == 0 && n.x[1] == 0 && n.x[2] == 0;
    if (all_zero && !(n.i < n.j)) return false;
    IVec<3> xv{n.x[0], n.x[1], n.x[2]};
    return subst.in_H_sigma_exact(xv, static_cast<std::size_t>(n.j));
}

std::vector<SNode<3>> parents_of(
        std::size_t neighbor, long long a, const Substitution<3>& subst,
        const SNode<3>& y, long long x_bound, long long x0_bound) {
    std::vector<SNode<3>> result;
    for (long long pi = 0; pi < 3; ++pi) {
        for (long long pj = 0; pj < 3; ++pj) {
            for (long long x1 = -x_bound; x1 <= x_bound; ++x1) {
                for (long long x2 = -x_bound; x2 <= x_bound; ++x2) {
                    for (long long x0 = -x0_bound; x0 <= x0_bound; ++x0) {
                        const SNode<3> x{pi, {x0, x1, x2}, pj};
                        if (class_ii_neighbor_transition_weight(
                                    neighbor, a, y, x) > 0
                                && is_valid_restricted(subst, x)) {
                            result.push_back(x);
                            if (x0 == -x0_bound || x0 == x0_bound
                                    || x1 == -x_bound || x1 == x_bound
                                    || x2 == -x_bound || x2 == x_bound) {
                                g_bound_touched = true;
                            }
                        }
                    }
                }
            }
        }
    }
    return result;
}

std::set<SNode<3>> backward_closure_bfs(
        std::size_t neighbor, long long a, const Substitution<3>& subst,
        const std::vector<SNode<3>>& seeds,
        long long x_bound, long long x0_bound, std::size_t max_nodes) {
    std::set<SNode<3>> visited(seeds.begin(), seeds.end());
    std::vector<SNode<3>> frontier = seeds;
    while (!frontier.empty()) {
        std::vector<SNode<3>> next_frontier;
        for (const auto& y : frontier) {
            for (const auto& x :
                 parents_of(neighbor, a, subst, y, x_bound, x0_bound)) {
                if (visited.insert(x).second) {
                    if (visited.size() > max_nodes) {
                        std::printf(
                            "BFS_BLEW_PAST_CAP,%zu\n", max_nodes);
                        return visited;
                    }
                    next_frontier.push_back(x);
                }
            }
        }
        frontier = std::move(next_frontier);
    }
    return visited;
}

}  // namespace

int main(int argc, char** argv) {
    const long long a = argc > 1 ? std::atoll(argv[1]) : 3;
    const long long x_bound = argc > 2 ? std::atoll(argv[2]) : 4;
    const long long x0_bound = argc > 3 ? std::atoll(argv[3]) : 15;

    const auto center = class_ii(static_cast<std::size_t>(a));
    const auto spectral = classify_matrix_spectral(incidence(center));
    const SubstitutionRule rule(center);
    const auto subst = make_substitution<3>(rule, spectral.beta);
    const auto d_cont_candidates = search_D_cont<3>(subst, 2);

    std::vector<SNode<3>> seeds;
    for (const auto& c : d_cont_candidates) {
        seeds.push_back({c.i, c.x, c.j});
    }
    std::printf("SEEDS,%zu\n", seeds.size());

    const auto closure = backward_closure_bfs(
        3, a, subst, seeds, x_bound, x0_bound, 500);
    std::printf(
        "CENTER_BFS_CLOSURE,%lld,%zu,bound_touched=%s\n",
        a, closure.size(), g_bound_touched ? "YES_UNTRUSTWORTHY" : "no");
    for (const auto& node : closure) {
        std::printf(
            "  NODE,%lld,%lld,%lld,%lld,%lld,%lld\n",
            a, node.i, node.x[0], node.x[1], node.x[2], node.j);
    }
    return 0;
}
