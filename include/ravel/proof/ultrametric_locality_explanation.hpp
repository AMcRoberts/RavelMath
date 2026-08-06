// ravel/proof/ultrametric_locality_explanation.hpp
//
// WHY "locality" (in the sense of Part 3 of general_generator_theorem.hpp
// -- no fixed small set of "obviously relevant" letters suffices to
// predict primitivity) fails in the digit-VALUE (archimedean) metric
// but is expected to hold in the substitution's own dependency-graph
// (ultrametric / hub-distance) metric.
//
// Letter 0 occurs in EVERY image with digit[c] >= 1 (it is the
// universal filler symbol). Consequently letter 0 is at graph
// distance exactly 1 -- in the substitution's own parent/dependency
// graph, NOT in digit-value space -- from every letter that is the
// trailing symbol of such an image. For a substitution with several
// nonzero digits, this makes letter 0 simultaneously adjacent to most
// or all of the alphabet: a genuine HUB, close to everything
// structurally regardless of how far its "value" (it has none, being
// the filler, not a state with its own single defining digit) might
// seem from any other letter's value.
//
// This is confirmed to be exactly the rescue mechanism in the
// counterexample that refuted the "value range" hypothesis
// (x^4-2x^2-2x+1, see occurrence_length_sets.hpp): the extracted
// decomposition path for (2,1)->(3,0), defect +2, is
//   (2,1) -[+1]-> (1,0) -[-1]-> (0,0) -[+1]-> (2,0) -[+1]-> (3,0)
// and passes directly through role (0,0) -- letter 0 paired with
// itself -- even though letter 0's digit VALUE bears no numeric
// relationship to letters 1, 2, or 3's values. The naive archimedean
// hypothesis (compare digit values of the two endpoint letters) has
// no way to notice this, because letter 0 is not "close" to anything
// in that metric; it is close in the graph-distance metric instead.
//
// What this DOES explain: why a hub letter (0, or any letter with a
// large occurrence count) is always the right place to look for a
// rescue route, structurally, independent of value comparisons.
//
// What this does NOT give: a closed form. Letter 0's own achievable
// defect range (role (0,0)'s own difference set, from its pooled
// occurrence lengths) is frequently too small to bridge a large gap
// in a single step -- in the example above it is only {-1,0,1} -- so
// the actual rescue is a multi-step WALK combining the hub with other
// letters' contributions, not a single hub-provided jump. Ultrametric
// closeness explains WHERE to look; it does not by itself decide
// WHETHER the walk-sum succeeds, which remains the genuine graph-
// reachability question `general_generator_theorem.hpp` already
// answers exactly (if not in closed form).

#pragma once

#include <cstddef>
#include <map>
#include <set>
#include <vector>

namespace ravel::proof {

struct HubDistanceReport {
    std::size_t alphabet_size{};
    // graph_distance_from_zero[m] = shortest number of substitution
    // parent-hops from letter 0 to letter m (0 for m=0 itself, 1 for
    // any letter that is a trailing symbol of a digit>=1 image, etc.)
    std::vector<long long> graph_distance_from_zero;
    std::size_t letters_at_distance_one{};
    double fraction_at_distance_one{};
};

inline HubDistanceReport analyze_hub_distance(const std::vector<std::vector<long long>>& sigma) {
    HubDistanceReport out;
    const std::size_t n = sigma.size();
    out.alphabet_size = n;
    out.graph_distance_from_zero.assign(n, -1);
    out.graph_distance_from_zero[0] = 0;

    // Undirected adjacency via "co-occurrence in the same image": two
    // letters are graph-adjacent if they occur in the same sigma(c),
    // which is the natural notion of "structurally close" here (both
    // are reachable from state c in one substitution step).
    std::vector<std::set<std::size_t>> adj(n);
    for (std::size_t c = 0; c < n; ++c) {
        const auto& img = sigma[c];
        std::set<long long> letters_here(img.begin(), img.end());
        for (auto a : letters_here) for (auto b : letters_here) {
            if (a != b) adj[(std::size_t)a].insert((std::size_t)b);
        }
    }
    // BFS from letter 0.
    std::vector<std::size_t> queue{0};
    for (std::size_t qi = 0; qi < queue.size(); ++qi) {
        auto u = queue[qi];
        for (auto v : adj[u]) {
            if (out.graph_distance_from_zero[v] == -1) {
                out.graph_distance_from_zero[v] = out.graph_distance_from_zero[u] + 1;
                queue.push_back(v);
            }
        }
    }
    out.letters_at_distance_one = 0;
    for (std::size_t m = 1; m < n; ++m)
        if (out.graph_distance_from_zero[m] == 1) ++out.letters_at_distance_one;
    out.fraction_at_distance_one = n > 1 ? (double)out.letters_at_distance_one / (double)(n - 1) : 0.0;
    return out;
}

}  // namespace ravel::proof
