// graph_divisor_test.cpp
//
// Self-test for graph_divisor.hpp: coarsest equitable partition,
// quotient matrix construction, and the divisor-comparison test used
// by app/test_bp_gb_divisor.cpp to probe whether the balanced-pair
// automaton and G_B share a common equitable-partition structure.

#include "ravel/graph_divisor.hpp"

#include <cstdio>
#include <limits>
#include <stdexcept>
#include <vector>

using namespace ravel;

static int total_tests = 0;
static int failed = 0;

#define CHECK(expr, label) do {                                       \
    ++total_tests;                                                    \
    if (!(expr)) { std::printf("  [FAIL] %s\n", label); ++failed; }    \
    else         { std::printf("  [ok]   %s\n", label); }              \
} while (0)

int main() {
    std::printf("graph_divisor self-tests:\n");

    // --- 1. Directed 3-cycle: fully symmetric, so the coarsest
    // equitable partition must collapse all 3 nodes into ONE class
    // (every node has identical out/in structure by symmetry), and
    // the 1x1 quotient must just be [3] (each node has one out-edge
    // of weight 1, so summed appropriately the quotient reflects the
    // cycle's total weight structure: quotient[0][0] = 1, since a
    // representative's out-weight to its OWN class -- which is now
    // everyone -- is exactly its one outgoing edge of weight 1).
    {
        WeightedDigraph g(3);
        g.add_edge(0, 1, 1);
        g.add_edge(1, 2, 1);
        g.add_edge(2, 0, 1);
        auto p = coarsest_equitable_partition(g);
        CHECK(p.num_classes == 1, "directed 3-cycle collapses to 1 class");
        CHECK(is_equitable(g, p), "directed 3-cycle partition is genuinely equitable");
        auto Q = quotient_matrix(g, p);
        CHECK(Q.size() == 1 && Q[0][0] == 1, "directed 3-cycle quotient is [[1]]");
    }

    // --- 2. Two disjoint directed 3-cycles vs one directed 6-cycle:
    // both are vertex-transitive with out-degree 1, in-degree 1,
    // uniform weight 1 -- both must collapse to the SAME 1x1
    // quotient [[1]], even though the underlying graphs have
    // different sizes and different numbers of connected components.
    // This is exactly the situation the divisor test is meant to
    // recognize: structurally different graphs, same canonical
    // quotient, hence (by Perron-Frobenius) same dominant eigenvalue
    // (both have Perron root 1, trivially, since these are pure
    // permutation matrices).
    {
        WeightedDigraph a(6);  // two disjoint 3-cycles
        a.add_edge(0, 1, 1); a.add_edge(1, 2, 1); a.add_edge(2, 0, 1);
        a.add_edge(3, 4, 1); a.add_edge(4, 5, 1); a.add_edge(5, 3, 1);

        WeightedDigraph b(6);  // one 6-cycle
        for (int i = 0; i < 6; ++i) b.add_edge(i, (i + 1) % 6, 1);

        auto cmp = compare_divisors(a, b);
        CHECK(cmp.classes_a == 1 && cmp.classes_b == 1,
              "two 3-cycles and one 6-cycle both collapse to 1 class");
        CHECK(cmp.charpoly_match,
              "two 3-cycles and one 6-cycle have identical [[1]] quotients");
    }

    // --- 3. A graph with a genuine orbit structure: a 4-node "star"
    // (center -> 3 leaves, weight 1 each; no other edges) should
    // refine to exactly 2 classes: {center} and {leaf1,leaf2,leaf3}
    // (the leaves are interchangeable: identical in/out structure).
    {
        WeightedDigraph g(4);
        g.add_edge(0, 1, 1);
        g.add_edge(0, 2, 1);
        g.add_edge(0, 3, 1);
        auto p = coarsest_equitable_partition(g);
        CHECK(p.num_classes == 2, "star graph refines to 2 classes (center, leaves)");
        CHECK(is_equitable(g, p), "star graph partition is genuinely equitable");
        CHECK(p.color[1] == p.color[2] && p.color[2] == p.color[3],
              "star graph's three leaves land in the same class");
        CHECK(p.color[0] != p.color[1], "star graph's center is its own class");
    }

    // --- 4. A graph where refinement must go past round 1: a
    // directed path 0->1->2->3 with no other edges.  Every node has
    // a DIFFERENT (out-histogram, in-histogram) once colors start
    // propagating (node 0 has in-degree 0, node 3 has out-degree 0,
    // and 1 vs 2 differ once 0 and 3 are distinguished), so the
    // coarsest equitable partition should end up with 4 singleton
    // classes.
    {
        WeightedDigraph g(4);
        g.add_edge(0, 1, 1);
        g.add_edge(1, 2, 1);
        g.add_edge(2, 3, 1);
        auto p = coarsest_equitable_partition(g);
        CHECK(p.num_classes == 4, "directed path on 4 nodes refines to 4 singleton classes");
        CHECK(is_equitable(g, p), "directed path partition is (trivially) equitable");
    }

    // --- 5. from_dense round-trips a dense adjacency/multiplicity
    // matrix (the format G_B and the BP transition matrix are stored
    // in) into the same structure add_edge would build directly.
    {
        std::vector<std::vector<long long>> M = {{0, 2, 0}, {0, 0, 3}, {1, 0, 0}};
        auto g = WeightedDigraph::from_dense(M);
        auto p = coarsest_equitable_partition(g);
        CHECK(is_equitable(g, p), "from_dense-built graph's coarsest partition is equitable");
        auto Q = quotient_matrix(g, p);
        // Weighted 3-cycle 0->1(w2)->2(w3)->0(w1): vertex-transitive
        // under the weighted sense only if weights matched, which
        // they don't here (2,3,1 are all different) -- so this should
        // land on 3 singleton classes and the quotient should just BE
        // M itself (up to the class-id-to-vertex-id identity map,
        // since coarsest_equitable_partition assigns class ids in an
        // order determined by refinement, not necessarily 0,1,2 in
        // vertex order -- so compare via charpoly instead of entries).
        auto cp_direct = charpoly_int(M);
        auto cp_quot = charpoly_int(Q);
        CHECK(cp_direct == cp_quot,
              "from_dense weighted 3-cycle: quotient charpoly matches original matrix charpoly");

        CHECK(polyval_int_is_zero({1, 0, -1}, 1),
              "arbitrary-precision Horner zero test recognizes x^2-1 at x=1");
        bool coefficient_overflow_rejected = false;
        try {
            const long long m = std::numeric_limits<long long>::max();
            (void)charpoly_int({{m, 0}, {0, m}});
        } catch (const std::overflow_error&) {
            coefficient_overflow_rejected = true;
        }
        CHECK(coefficient_overflow_rejected,
              "legacy charpoly API rejects a true coefficient larger than long long");

        // Migration path (Q4 of docs/RECOVERY_AUDIT_2026-07-29.md):
        // charpoly_PolyZ returns mathlib::PolyZ directly with no
        // long long intermediate.  polyZ_to_long_long_vec re-enters
        // the legacy surface only at the boundary the caller
        // controls.
        auto cp_PolyZ = charpoly_PolyZ(M);
        auto cp_PolyZ_quot = charpoly_PolyZ(Q);
        CHECK(cp_PolyZ == cp_PolyZ_quot,
              "charpoly_PolyZ equality agrees with charpoly_int on a "
              "matrix whose coefficients fit in long long");
        auto cp_legacy = polyZ_to_long_long_vec(cp_PolyZ);
        CHECK(cp_legacy == cp_direct,
              "polyZ_to_long_long_vec agrees with charpoly_int on a "
              "matrix whose coefficients fit in long long");

        // On a matrix whose coefficients do exceed long long,
        // charpoly_PolyZ succeeds and polyZ_to_long_long_vec
        // throws overflow_error.  The legacy charpoly_int path
        // also throws, but charpoly_PolyZ preserves the exact
        // integer polynomial in a PolyZ so callers can compare
        // without overflow weakening.
        const long long m = std::numeric_limits<long long>::max();
        auto cp_overflow_PolyZ = charpoly_PolyZ({{m, 0}, {0, m}});
        bool polyz_overflow_rejected_on_legacy = false;
        try {
            (void)polyZ_to_long_long_vec(cp_overflow_PolyZ);
        } catch (const std::overflow_error&) {
            polyz_overflow_rejected_on_legacy = true;
        }
        CHECK(polyz_overflow_rejected_on_legacy,
              "polyZ_to_long_long_vec reports overflow rather than "
              "silently wrapping, even when charpoly_PolyZ succeeds");
    }

    // --- 6. tarjan_scc / extract_recurrent_core: a graph with one
    // nontrivial recurrent cycle {0,1} and several transient feeder
    // nodes that funnel into it but are never revisited -- structurally
    // the same shape Fibonacci's actual G_B has (a 2-cycle recurrent
    // core plus 4 transient nodes; see docs/FINDINGS_FOR_CITATION.md
    // Finding 1).  Feeders: 2->0, 3->1, 4->2, 5->0 (5 is a
    // second feeder into the cycle, not part of it).
    {
        WeightedDigraph g(6);
        g.add_edge(0, 1, 1);
        g.add_edge(1, 0, 1);  // the recurrent 2-cycle {0,1}
        g.add_edge(2, 0, 1);  // transient feeders, never revisited
        g.add_edge(3, 1, 1);
        g.add_edge(4, 2, 1);
        g.add_edge(5, 0, 1);

        auto sccs = tarjan_scc(g);
        std::size_t n_recurrent = 0;
        for (auto& c : sccs) if (is_recurrent_scc(g, c)) ++n_recurrent;
        CHECK(n_recurrent == 1, "exactly one recurrent SCC found (the {0,1} cycle)");

        auto [core, orig_idx] = extract_recurrent_core(g);
        CHECK(core.n == 2, "recurrent core has exactly 2 nodes (matching Fibonacci's G_B shape)");
        CHECK(orig_idx.size() == 2, "orig_idx maps back to exactly 2 original node indices");
        std::vector<std::size_t> sorted_idx = orig_idx;
        std::sort(sorted_idx.begin(), sorted_idx.end());
        CHECK(sorted_idx == std::vector<std::size_t>({0, 1}),
              "recurrent core's original indices are exactly {0,1}, the true cycle");

        // The core itself must be a genuine 2-cycle (each node has
        // exactly one out-edge, to the other node).
        bool core_is_cycle = true;
        for (std::size_t u = 0; u < core.n; ++u) {
            if (core.out_adj[u].size() != 1) core_is_cycle = false;
        }
        CHECK(core_is_cycle, "extracted core is itself a clean 2-cycle, transients excluded");
    }

    // --- 7. A graph with NO nontrivial recurrent SCC at all (a pure
    // DAG) -- extract_recurrent_core must return an empty core, not
    // silently pick a transient node.
    {
        WeightedDigraph g(4);
        g.add_edge(0, 1, 1);
        g.add_edge(1, 2, 1);
        g.add_edge(2, 3, 1);
        auto [core, orig_idx] = extract_recurrent_core(g);
        CHECK(core.n == 0, "pure DAG has an empty recurrent core");
        CHECK(orig_idx.empty(), "pure DAG's core has no original-index mapping");
    }

    // --- 8. extract_dominant_recurrent_core: a graph with TWO
    // recurrent SCCs where the LARGER-by-node-count one has the
    // SMALLER Perron root -- the exact shape of the Hexanacci (n=6
    // n-bonacci) counterexample (see docs/RESEARCH_STATUS.md for how it
    // was found)
    // (app/gb_bp_involution_general_n.cpp), at a small testable scale.
    // SCC A: a 5-node simple cycle (any simple cycle has Perron root
    // exactly 1, regardless of size). SCC B: a 2-node complete
    // bidirectional graph with edge weight 3 each way (adjacency
    // [[0,3],[3,0]], eigenvalues +-3, Perron root 3). B is smaller but
    // dominant; extract_recurrent_core (largest BY SIZE) would wrongly
    // return A; extract_dominant_recurrent_core must return B.
    {
        WeightedDigraph g(7);
        // SCC A: 0->1->2->3->4->0, a 5-cycle, Perron root 1.
        g.add_edge(0, 1, 1);
        g.add_edge(1, 2, 1);
        g.add_edge(2, 3, 1);
        g.add_edge(3, 4, 1);
        g.add_edge(4, 0, 1);
        // SCC B: nodes 5,6, complete bidirectional, weight 3, Perron root 3.
        g.add_edge(5, 6, 3);
        g.add_edge(6, 5, 3);

        auto [wrong_core, wrong_idx] = extract_recurrent_core(g);
        CHECK(wrong_core.n == 5,
              "extract_recurrent_core picks the LARGER-by-size SCC (5 nodes) -- the wrong one here");

        auto [dom_core, dom_idx] = extract_dominant_recurrent_core(g);
        CHECK(dom_core.n == 2,
              "extract_dominant_recurrent_core picks the ACTUALLY DOMINANT SCC (2 nodes, Perron root 3)");
        std::vector<std::size_t> sorted_dom_idx = dom_idx;
        std::sort(sorted_dom_idx.begin(), sorted_dom_idx.end());
        CHECK(sorted_dom_idx == std::vector<std::size_t>({5, 6}),
              "dominant core's original indices are exactly {5,6}, the true dominant SCC");

        double dom_eigenvalue = dominant_eigenvalue_estimate_sparse(dom_core, 500, 1e-12);
        CHECK(std::abs(dom_eigenvalue - 3.0) < 1e-6,
              "dominant core's own Perron root is ~3, matching SCC B's known eigenvalue");
    }

    // --- 9. Public graph APIs reject malformed dimensions and
    // partitions instead of indexing out of bounds or manufacturing a
    // quotient from an invalid precondition.
    {
        bool threw = false;
        try {
            (void)WeightedDigraph::from_dense({{1, 0}, {1}});
        } catch (const std::invalid_argument&) {
            threw = true;
        }
        CHECK(threw, "from_dense rejects a ragged matrix");

        WeightedDigraph g(2);
        threw = false;
        try {
            g.add_edge(0, 2);
        } catch (const std::out_of_range&) {
            threw = true;
        }
        CHECK(threw, "add_edge rejects an out-of-range vertex");

        threw = false;
        try {
            (void)coarsest_equitable_partition(g, {0});
        } catch (const std::invalid_argument&) {
            threw = true;
        }
        CHECK(threw, "color refinement rejects a size-mismatched coloring");

        g.add_edge(0, 1);
        EquitablePartition bad{{0, 0}, 1};
        CHECK(!is_equitable(g, bad),
              "is_equitable rejects a structurally unequal class");
        threw = false;
        try {
            (void)quotient_matrix(g, bad);
        } catch (const std::invalid_argument&) {
            threw = true;
        }
        CHECK(threw, "quotient_matrix rejects a non-equitable partition");
    }

    std::printf("\n%d tests run, %d failed.\n", total_tests, failed);
    return failed == 0 ? 0 : 1;
}
