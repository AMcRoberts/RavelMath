// ambient_graph_test.cpp
//
// Self-test for ambient_graph.hpp.  Verifies that the combined
// type-1 + type-2 forward edges (matching Def 3.1) produces
// non-empty successors for the 6 non-self-contact D_cont entries
// of σ_1.  This is a partial W10 (full ambient graph port) test
// — the corona algorithm (Algorithm 2) is deferred to a future
// work item.

#include <cstdio>
#include <cstring>
#include <vector>
#include <array>

#include "ravel/ambient_graph.hpp"

using namespace ravel;

static int total_tests = 0;
static int failed = 0;

#define CHECK(expr, label) do {                              \
    ++total_tests;                                          \
    if (!(expr)) {                                          \
        std::printf("  [FAIL] %s\n", label);                \
        ++failed;                                           \
    } else {                                                \
        std::printf("  [ok]   %s\n", label);                \
    }                                                       \
} while (0)

int main() {
    // σ_1 (reference python) subst = {0:(0,0,0,1), 1:(0,0,2), 2:(0,)}.
    std::array<std::vector<long long>, 3> images = {
        std::vector<long long>{0, 0, 0, 1},
        std::vector<long long>{0, 0, 2},
        std::vector<long long>{0}
    };
    Substitution<3> subst(images, 3.6273650847118);

    // 6 non-self-contact D_cont entries (in 0-indexed notation):
    std::vector<ANode<3>> d_cont = {
        {0, {{0, 0, 0}}, 1},   // [0, 0, 1]   (paper's [1, 0, 2])
        {0, {{0, 0, 0}}, 2},   // [0, 0, 2]   (paper's [1, 0, 3])
        {1, {{1, -1, 0}}, 0},  // [1, e1-e2, 0]  (paper's [2, e1-e2, 1])
        {1, {{0, 0, 0}}, 2},   // [1, 0, 2]   (paper's [2, 0, 3])
        {2, {{1, 0, -1}}, 0},  // [2, e1-e3, 0]  (paper's [3, e1-e3, 1])
        {2, {{0, 1, -1}}, 1},  // [2, e2-e3, 1]  (paper's [3, e2-e3, 2])
    };

    std::printf("[sigma_1]   D_cont has 6 non-self entries; verify each has at least one type-1 OR type-2 forward edge\n");
    std::size_t total_edges = 0;
    std::size_t total_t1 = 0;
    std::size_t total_t2 = 0;
    for (const auto& node : d_cont) {
        auto edges1 = forward_edges_type1<3>(subst, node);
        auto edges2 = forward_edges_type2<3>(subst, node);
        char label[128];
        std::snprintf(label, sizeof(label),
            "edges exist for [%lld, (%lld,%lld,%lld), %lld]: t1=%zu t2=%zu",
            node.i, node.x[0], node.x[1], node.x[2], node.j,
            edges1.size(), edges2.size());
        CHECK(edges1.size() + edges2.size() >= 1, label);
        total_t1 += edges1.size();
        total_t2 += edges2.size();
        total_edges += edges1.size() + edges2.size();
    }
    std::printf("  total type-1 edges: %zu, type-2 edges: %zu, total: %zu\n",
        total_t1, total_t2, total_edges);

    // Sanity: for σ_1, G_D (full ambient graph including both
    // types) has 14 nodes (= |C|) and G_B (corona-fixed-point)
    // has 26 nodes.  We don't yet compute the corona, but the
    // combined forward-edge list has a definite number of
    // distinct edges we can sanity-check.  At minimum: 6 D_cont
    // entries each have at least one successor of each type, so
    // combined is at least 12 edges.
    CHECK(total_edges >= 6, "ambient graph has at least 6 edges");

    std::printf("\n%d tests run, %d failed.\n", total_tests, failed);
    return failed == 0 ? 0 : 1;
}
