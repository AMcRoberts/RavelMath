// corona_test.cpp
//
// Self-test for the contact boundary logic (corona + Red + Algorithm 2
// of arXiv:2511.16442).  This is the COMPLETION of W10: type-1 +
// type-2 forward edges (done) + the corona iteration (this file).
//
// Verifies the σ_1 results:
//   * |C| = 14  (the contact set is 14 nodes)
//   * Algorithm 2 converges to |G_B| = 26 (the boundary graph)
//   * G_B = Ghat_C (the boundary graph coincides with the contact
//     graph for σ_1; the paper's own claim)
//
// The differential-testing harness (forward<->backward) for the
// simple ambient graph (Def 3.5) is also tested: the type-1 forward
// targets and the independent type-1 backward targets must be exactly
// transpose.

#include <cstdio>
#include <cstring>
#include <vector>
#include <array>
#include <set>
#include <map>

#include "ravel/core.hpp"
#include "ravel/faces.hpp"
#include "ravel/ambient_graph.hpp"
#include "ravel/corona.hpp"

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

// Convert an ANode<d> to an SNode<d> (the corona uses SNode).
template <std::size_t d>
SNode<d> to_snode(const ANode<d>& n) {
    SNode<d> r;
    r.i = n.i;
    r.x = n.x;
    r.j = n.j;
    return r;
}

int main() {
    // σ_1 (reference python) subst = {0:(0,0,0,1), 1:(0,0,2), 2:(0,)}.
    std::array<std::vector<long long>, 3> images = {
        std::vector<long long>{0, 0, 0, 1},
        std::vector<long long>{0, 0, 2},
        std::vector<long long>{0}
    };
    Substitution<3> subst(images, 3.6273650847118);

    // The 6 non-self-contact D_cont entries (in 0-indexed notation).
    // Each ANode is a simple struct: {long long i; std::array<long long, d> x; long long j;}.
    std::vector<ANode<3>> d_cont_an;
    d_cont_an.push_back({0, {0, 0, 0}, 1});   // [0, 0, 1]   (paper's [1, 0, 2])
    d_cont_an.push_back({0, {0, 0, 0}, 2});   // [0, 0, 2]   (paper's [1, 0, 3])
    d_cont_an.push_back({1, {1, -1, 0}, 0});  // [1, e1-e2, 0]  (paper's [2, e1-e2, 1])
    d_cont_an.push_back({1, {0, 0, 0}, 2});   // [1, 0, 2]   (paper's [2, 0, 3])
    d_cont_an.push_back({2, {1, 0, -1}, 0});  // [2, e1-e3, 0]  (paper's [3, e1-e3, 1])
    d_cont_an.push_back({2, {0, 1, -1}, 1});  // [2, e2-e3, 1]  (paper's [3, e2-e3, 2])

    // Convert to SNode for the corona-side tests.
    std::vector<SNode<3>> d_cont;
    for (const auto& a : d_cont_an) d_cont.push_back(to_snode<3>(a));

    // ---- differential test: simple ambient graph forward <-> backward ----
    {
        std::printf("[simple_ambient]   forward<->backward differential test (Def 3.5)\n");
        int n_checked = 0;
        int n_fail = 0;
        for (const auto& node : d_cont) {
            auto fwd = simple_forward_targets<3>(subst, node);
            for (const auto& [dest, pq] : fwd) {
                ++n_checked;
                auto back = simple_backward_targets<3>(subst, dest);
                bool found = false;
                for (const auto& [src, _] : back) {
                    if (src.i == node.i && src.x == node.x && src.j == node.j) {
                        found = true;
                        break;
                    }
                }
                if (!found) ++n_fail;
            }
        }
        std::printf("  forward<->backward: %d/%d OK\n", n_checked - n_fail, n_checked);
        CHECK(n_fail == 0, "all forward<->backward transpose checks pass");
    }

    // ---- build the contact set C (backward closure + Red) ----
    std::printf("\n[contact_set]   build C from D_cont (backward closure + Red)\n");
    auto Gp_nodes = backward_closure<3>(subst, d_cont_an);
    bool labelled_backward_projection = true;
    for (const auto& destination : Gp_nodes) {
        const auto plain = backward_edges<3>(subst, destination);
        const auto labelled =
            backward_edge_witnesses<3>(subst, destination);
        const std::set<ANode<3>> plain_set(plain.begin(), plain.end());
        std::set<ANode<3>> labelled_set;
        for (const auto& witness : labelled)
            labelled_set.insert(witness.predecessor);
        labelled_backward_projection =
            labelled_backward_projection
            && plain_set.size() == labelled_set.size()
            && std::equal(
                plain_set.begin(), plain_set.end(),
                labelled_set.begin(),
                [](const auto& lhs, const auto& rhs) {
                    return !(lhs < rhs) && !(rhs < lhs);
                });
    }
    CHECK(labelled_backward_projection,
          "labelled backward witnesses project exactly to backward_edges");
    auto Gp_edges = induced_restricted_edges<3>(subst, Gp_nodes);
    // Red over the restricted graph (uses ANode, since D_cont is in D).
    std::set<ANode<3>> Gp_set(Gp_nodes.begin(), Gp_nodes.end());
    auto red_result = red_anode<3>(Gp_set, Gp_edges);
    auto& C_nodes_an = red_result.first;
    std::printf("  |C| = %zu (expect 14)\n", C_nodes_an.size());
    CHECK(C_nodes_an.size() == 14, "contact set C has 14 nodes for sigma_1");

    // Convert C to SNode (the corona uses SNode, not ANode).
    std::set<SNode<3>> C;
    for (const auto& a : C_nodes_an) C.insert(to_snode<3>(a));

    // ---- run Algorithm 2 (corona + Red iteration) ----
    std::printf("\n[algorithm_2]   run corona + Red fixed-point iteration\n");
    auto G_B = algorithm2<3>(subst, C);
    std::printf("  |G_B| = %zu (expect 26)\n", G_B.size());
    CHECK(G_B.size() == 26, "G_B has 26 nodes for sigma_1 (matches reference)");

    const auto fixed_trace = algorithm2_trace<3>(
        subst, C, CoronaConnectorPolicy::fixed_signed_contact);
    const auto evolving_trace = algorithm2_trace<3>(
        subst, C, CoronaConnectorPolicy::evolving_layer);
    const auto exact_trace = algorithm2_trace<3>(
        subst, C, CoronaConnectorPolicy::fixed_signed_contact,
        50, 0, CoronaEdgeArithmetic::exact_rational);
    const auto projected_all_trace = algorithm2_projected_trace<3>(
        subst, C, [](const SNode<3>&) { return true; },
        CoronaConnectorPolicy::fixed_signed_contact,
        50, 0, CoronaEdgeArithmetic::exact_rational);
    CHECK(fixed_trace.converged, "fixed-contact corona trace converges");
    CHECK(fixed_trace.final_nodes == G_B,
          "algorithm2 returns the fixed-contact trace result");
    CHECK(fixed_trace.final_nodes == evolving_trace.final_nodes,
          "sigma_1 fixed/evolving connector semantics agree");
    CHECK(fixed_trace.final_nodes == exact_trace.final_nodes,
          "sigma_1 rounded/exact edge arithmetic agrees");
    CHECK(projected_all_trace.converged,
          "unrestricted projected Algorithm 2 converges");
    CHECK(projected_all_trace.predicate_closed,
          "unrestricted projected Algorithm 2 has empty rejected boundary");
    CHECK(projected_all_trace.final_nodes == exact_trace.final_nodes,
          "projected Algorithm 2 agrees exactly with legacy materialization");
    CHECK(exact_trace.edge_arithmetic
              == CoronaEdgeArithmetic::exact_rational,
          "corona trace records exact edge arithmetic");

    CHECK(default_corona_execution_mode()
              == CoronaExecutionMode::projected_surface,
          "request-driven corona surface is the default mode");
    CoronaSurface<3> surface(subst, C);
    CoronaProjectionRequest<3> projection_request;
    projection_request.seeds.push_back(*fixed_trace.final_nodes.begin());
    projection_request.accept = [&](const SNode<3>& node) {
        return fixed_trace.final_nodes.count(node) != 0;
    };
    projection_request.operations =
        static_cast<unsigned>(CoronaSurfaceOperation::simple_forward) |
        static_cast<unsigned>(CoronaSurfaceOperation::simple_backward);
    projection_request.edge_arithmetic = CoronaEdgeArithmetic::exact_rational;
    const auto projected_image = surface.project(projection_request);
    CHECK(projected_image.complete(),
          "request-driven image certifies closure under requested operations");
    std::set<SNode<3>> expected_component;
    std::vector<SNode<3>> component_frontier{*fixed_trace.final_nodes.begin()};
    expected_component.insert(component_frontier.front());
    for (std::size_t head = 0; head < component_frontier.size(); ++head) {
        const auto current = component_frontier[head];
        for (const auto& [next, witness] :
             simple_forward_targets_exact<3>(subst, current)) {
            (void)witness;
            if (fixed_trace.final_nodes.count(next) != 0 &&
                expected_component.insert(next).second)
                component_frontier.push_back(next);
        }
        for (const auto& [next, witness] :
             simple_backward_targets<3>(subst, current)) {
            (void)witness;
            if (fixed_trace.final_nodes.count(next) != 0 &&
                expected_component.insert(next).second)
                component_frontier.push_back(next);
        }
    }
    CHECK(projected_image.nodes == expected_component,
          "one recurrent seed projects its complete closed boundary component");
    CHECK(!projected_image.edges.empty(),
          "request-driven image preserves induced transition witnesses");
    CHECK(fixed_trace.layers.size() == 3,
          "sigma_1 fixed-contact trace records initial, growth, fixed layers");
    bool red_traces_partition = true;
    bool red_ranks_are_ordered_sinks = true;
    for (std::size_t layer_index = 1;
         layer_index < exact_trace.layers.size(); ++layer_index) {
        const auto& layer = exact_trace.layers[layer_index];
        auto remaining = layer.pre_red_nodes;
        std::set<SNode<3>> reconstructed = layer.nodes;
        for (const auto& rank : layer.red_pruning_ranks) {
            for (const auto& node : rank)
                red_traces_partition =
                    red_traces_partition && reconstructed.insert(node).second;
            std::set<SNode<3>> has_out;
            for (const auto& source : remaining)
                for (const auto& [target, pq] :
                     simple_forward_targets_exact<3>(subst, source))
                    if (remaining.count(target) != 0) has_out.insert(source);
            for (const auto& node : rank)
                red_ranks_are_ordered_sinks =
                    red_ranks_are_ordered_sinks
                    && remaining.count(node) != 0
                    && has_out.count(node) == 0;
            for (const auto& node : remaining)
                if (rank.count(node) == 0)
                    red_ranks_are_ordered_sinks =
                        red_ranks_are_ordered_sinks
                        && has_out.count(node) != 0;
            for (const auto& node : rank) remaining.erase(node);
        }
        red_traces_partition =
            red_traces_partition
            && reconstructed == layer.pre_red_nodes
            && remaining == layer.nodes;
    }
    CHECK(red_traces_partition,
          "ranked Red trace partitions pre-Red nodes into drops and survivors");
    CHECK(red_ranks_are_ordered_sinks,
          "each ranked Red drop is exactly the current sink set");

    // The reference notes that for σ_1, the paper claims "boundary = contact
    // graph" in the sense that the corona iteration converges to a graph
    // that *grows from* the contact graph.  Concretely: |±C| = 28 (raw
    // union), and the corona+Red fixed point lands at |G_B| = 26.  This
    // is NOT a literal node-set equality: the corona expansion adds 2
    // new nodes (with negative components) that aren't in ±C, and the
    // Red step prunes 4 nodes from ±C that have no out-edges after the
    // expansion.  Verified against the Python reference directly: the
    // C++ and Python outputs are bit-identical on these counts.
    auto plus_minus_C = build_signed_contact_set<3>(C);
    std::printf("  |±C| (signed contact) = %zu (expect 28)\n", plus_minus_C.size());
    CHECK(plus_minus_C.size() == 28, "signed contact set ±C has 28 nodes");

    // Expected counts for σ_1:
    //   G_B ∩ ±C = 24 nodes (the bulk of both sets)
    //   G_B \ ±C = 2 nodes (corona expansion adds these)
    //   ±C \ G_B = 4 nodes (Red prunes these)
    std::set<SNode<3>> gb_minus_pmC;
    std::set<SNode<3>> pmC_minus_gb;
    for (const auto& n : G_B) if (plus_minus_C.count(n) == 0) gb_minus_pmC.insert(n);
    for (const auto& n : plus_minus_C) if (G_B.count(n) == 0) pmC_minus_gb.insert(n);
    std::printf("  G_B \\ ±C: %zu nodes (expect 2 -- corona expansion)\n", gb_minus_pmC.size());
    std::printf("  ±C \\ G_B: %zu nodes (expect 4 -- Red prunes these)\n", pmC_minus_gb.size());
    CHECK(gb_minus_pmC.size() == 2, "G_B has 2 nodes not in ±C (matches Python)");
    CHECK(pmC_minus_gb.size() == 4, "±C has 4 nodes not in G_B (matches Python)");

    // The exact set of nodes Python produces for σ_1.  If these change,
    // either (a) the C++ port drifted from Python, or (b) Python's
    // algorithm2 was modified; either way it's a regression to chase.
    std::set<SNode<3>> expected_gb_minus_pmC = {
        {1, {0, -1, 0}, 0},   // Python: (2, (0,-1,0), 1) in 1-indexed
        {2, {0, 0, -1}, 0},   // Python: (3, (0,0,-1), 1) in 1-indexed
    };
    std::set<SNode<3>> expected_pmC_minus_gb = {
        {0, {-1, 0, 1}, 1},   // Python: (1, (-1,0,1), 2) in 1-indexed
        {0, {-1, 0, 1}, 2},   // Python: (1, (-1,0,1), 3) in 1-indexed
        {0, {-1, 1, -1}, 1},  // Python: (1, (-1,1,-1), 2) in 1-indexed
        {1, {0, -1, 1}, 2},   // Python: (2, (0,-1,1), 3) in 1-indexed
    };
    CHECK(gb_minus_pmC == expected_gb_minus_pmC,
          "exact set of G_B \\ ±C nodes matches Python reference");
    CHECK(pmC_minus_gb == expected_pmC_minus_gb,
          "exact set of ±C \\ G_B nodes matches Python reference");

    std::printf("\n%d tests run, %d failed.\n", total_tests, failed);
    return failed == 0 ? 0 : 1;
}
