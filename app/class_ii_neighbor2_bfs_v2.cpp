// Second attempt at a neighbor-parametrized backward-closure BFS,
// this time a direct, small replica of the TRUSTED corona.hpp
// backward_edges formula (M x' = x + l(p1) - l(q1) for type 1, with
// the i'/j' roles swapped and M negated for type 2), built from
// class_ii_neighbor_image_segments (verified against tau_a's actual
// images already) instead of class_ii_neighbor_transition_weight
// (which the previous attempt showed does not reproduce
// backward_edges's relation, even after fixing an argument-order bug
// and adding the window-validity gate -- likely a different, corona-
// stage relation, not raw ambient-graph backward branches).
//
// Validated against the CENTER's known 9->16 chain before being
// trusted on the neighbor -- small case first.

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

// Abelianization of the prefix up to (not including) a given FLATTENED
// position -- not a segment index. A position can fall strictly inside
// a segment (this is the common case: the 0^a / 0^(a-1) runs are single
// segments of length > 1), so full segments before the position
// contribute their whole length, and the segment containing the
// position contributes only the partial count up to it. Passing a
// segment index here instead of a flattened position was the actual
// bug: it silently agreed with the correct answer only when every
// segment has length 1, and was wrong for any position inside the
// 0^a run -- exactly the part of the computation that varies with a.
std::array<long long, 3> abelianize_prefix(
        std::size_t neighbor, std::size_t parent,
        std::size_t upto_position, long long a) {
    std::array<long long, 3> result{0, 0, 0};
    const auto segments = class_ii_neighbor_image_segments(neighbor, parent);
    long long remaining = static_cast<long long>(upto_position);
    for (const auto& seg : segments) {
        const long long length = seg.length_intercept + seg.length_slope * a;
        if (remaining >= length) {
            result[seg.letter] += length;
            remaining -= length;
        } else {
            result[seg.letter] += remaining;
            remaining = 0;
            break;
        }
    }
    return result;
}

// Every (position, letter) pair directly from image_segments, expanded
// one unit at a time within each segment (segments can have length > 1
// only for the 0^a / 0^(a-1) runs, always letter 0, so this is cheap
// and exact -- no need to expand the literal word).
std::vector<std::pair<std::size_t, long long>> letter_positions(
        std::size_t neighbor, std::size_t parent, long long a) {
    std::vector<std::pair<std::size_t, long long>> out;
    const auto segments = class_ii_neighbor_image_segments(neighbor, parent);
    std::size_t pos = 0;
    for (const auto& seg : segments) {
        const long long length = seg.length_intercept + seg.length_slope * a;
        for (long long k = 0; k < length; ++k) {
            out.emplace_back(pos, static_cast<long long>(seg.letter));
            ++pos;
        }
    }
    return out;
}

std::array<long long, 3> incidence_action(
        long long a, const std::array<long long, 3>& x) {
    return {a * x[0] + a * x[1] + x[2], x[0], x[0] + x[1]};
}

bool is_valid_restricted(
        const Substitution<3>& subst, const SNode<3>& n) {
    const bool all_zero = n.x[0] == 0 && n.x[1] == 0 && n.x[2] == 0;
    if (all_zero && !(n.i < n.j)) return false;
    IVec<3> xv{n.x[0], n.x[1], n.x[2]};
    return subst.in_H_sigma_exact(xv, static_cast<std::size_t>(n.j));
}

std::vector<SNode<3>> backward_edges_replica(
        std::size_t neighbor, long long a, const Substitution<3>& subst,
        const SNode<3>& node) {
    std::vector<SNode<3>> out;
    const auto Mxp = incidence_action(a, node.x);

    // Type 1: decompose sigma(node.i) for the "i" side, sigma(node.j)
    // for the "j" side; cand.x = M*node.x + l(prefix_i) - l(prefix_j).
    {
        const auto pos_i = letter_positions(
            neighbor, static_cast<std::size_t>(node.i), a);
        const auto pos_j = letter_positions(
            neighbor, static_cast<std::size_t>(node.j), a);
        for (std::size_t pi = 0; pi < pos_i.size(); ++pi) {
            const auto lp1 = abelianize_prefix(
                neighbor, static_cast<std::size_t>(node.i), pi, a);
            const long long letter_i = pos_i[pi].second;
            for (std::size_t pj = 0; pj < pos_j.size(); ++pj) {
                const auto lq1 = abelianize_prefix(
                    neighbor, static_cast<std::size_t>(node.j), pj, a);
                const long long letter_j = pos_j[pj].second;
                SNode<3> cand;
                cand.i = letter_i;
                cand.j = letter_j;
                for (std::size_t k = 0; k < 3; ++k)
                    cand.x[k] = Mxp[k] + lp1[k] - lq1[k];
                if (is_valid_restricted(subst, cand)) out.push_back(cand);
            }
        }
    }
    // Type 2: roles of i'/j' swapped, M negated.
    {
        const auto pos_j = letter_positions(
            neighbor, static_cast<std::size_t>(node.j), a);
        const auto pos_i = letter_positions(
            neighbor, static_cast<std::size_t>(node.i), a);
        for (std::size_t pi = 0; pi < pos_j.size(); ++pi) {
            const auto lp1 = abelianize_prefix(
                neighbor, static_cast<std::size_t>(node.j), pi, a);
            const long long letter_i = pos_j[pi].second;
            for (std::size_t pj = 0; pj < pos_i.size(); ++pj) {
                const auto lq1 = abelianize_prefix(
                    neighbor, static_cast<std::size_t>(node.i), pj, a);
                const long long letter_j = pos_i[pj].second;
                SNode<3> cand;
                cand.i = letter_i;
                cand.j = letter_j;
                for (std::size_t k = 0; k < 3; ++k)
                    cand.x[k] = -Mxp[k] + lp1[k] - lq1[k];
                if (is_valid_restricted(subst, cand)) out.push_back(cand);
            }
        }
    }
    return out;
}

std::set<SNode<3>> closure_bfs(
        std::size_t neighbor, long long a, const Substitution<3>& subst,
        const std::vector<SNode<3>>& seeds, std::size_t max_nodes) {
    std::set<SNode<3>> visited(seeds.begin(), seeds.end());
    std::vector<SNode<3>> frontier = seeds;
    while (!frontier.empty()) {
        std::vector<SNode<3>> next_frontier;
        for (const auto& y : frontier) {
            for (const auto& x :
                 backward_edges_replica(neighbor, a, subst, y)) {
                if (visited.insert(x).second) {
                    if (visited.size() > max_nodes) {
                        std::printf("BLEW_PAST_CAP,%zu\n", max_nodes);
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
    const std::size_t neighbor = argc > 2 ? std::atoll(argv[2]) : 3;
    const char* label = argc > 3 ? argv[3] : "CENTER";

    const auto center = class_ii(static_cast<std::size_t>(a));
    const auto spectral = classify_matrix_spectral(incidence(center));
    FiniteSubstitution target = center;
    if (neighbor != 3) {
        const auto neighbors = adjacent_swap_neighbors(center);
        target = neighbors[neighbor].substitution;
    }
    const SubstitutionRule rule(target);
    const auto subst = make_substitution<3>(rule, spectral.beta);
    const auto d_cont_candidates = search_D_cont<3>(subst, 2);

    std::vector<SNode<3>> seeds;
    for (const auto& c : d_cont_candidates) seeds.push_back({c.i, c.x, c.j});
    std::printf("SEEDS,%s,%zu\n", label, seeds.size());

    const auto closure = closure_bfs(neighbor, a, subst, seeds, 200);
    std::printf("BFS_V2_CLOSURE,%s,%lld,%zu\n", label, a, closure.size());
    for (const auto& node : closure) {
        std::printf(
            "  NODE,%s,%lld,%lld,%lld,%lld,%lld,%lld\n",
            label, a, node.i, node.x[0], node.x[1], node.x[2], node.j);
    }
    return 0;
}
