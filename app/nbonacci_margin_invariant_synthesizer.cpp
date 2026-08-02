// nbonacci_margin_invariant_synthesizer.cpp
//
// Corona-independent exact synthesis of the candidate dominant n-bonacci
// margin catalogue.  Face labels alone are too coarse: combining every
// label transition with both extrema creates spurious paths.  The necessary
// refinement is the sparse displacement x.  We generate its parametric
// grammar, filter by the exact face window -v_i < <x,v> < v_j, reconstruct
// the affine update M x' = x + delta e_0, and audit the resulting graph.

#include <algorithm>
#include <cstdlib>
#include <cstring>
#include <cstdio>
#include <map>
#include <queue>
#include <set>
#include <stdexcept>
#include <tuple>
#include <vector>

#include "math/bezout.hpp"
#include "math/charpoly.hpp"
#include "math/linalg_qbeta.hpp"
#include "math/sturm.hpp"
#include "math/in_h_sigma.hpp"
#include "ravel/graph_divisor.hpp"
#include "ravel/nbonacci_margin_invariant.hpp"

namespace {

using namespace ravel::nbonacci_margin;

struct Node {
    LabelPair pair;
    std::vector<long long> x;
    auto operator<=>(const Node&) const = default;
};

struct ExactContext {
    mathlib::QBetaRing ring;
    mathlib::RootInterval beta_interval;
    mathlib::QBetaVec v;
};

std::vector<std::vector<long long>> nbonacci_matrix(std::size_t n) {
    std::vector<std::vector<long long>> matrix(
        n, std::vector<long long>(n, 0));
    for (std::size_t column = 0; column + 1 < n; ++column) {
        matrix[0][column] = 1;
        matrix[column + 1][column] = 1;
    }
    matrix[0][n - 1] = 1;
    return matrix;
}

ExactContext exact_context(std::size_t n) {
    const auto matrix = nbonacci_matrix(n);
    mathlib::QBetaRing ring(mathlib::charpoly_faddeev_leverrier(matrix));
    const auto interval = mathlib::isolate_beta(ring);
    auto eigen = mathlib::left_eigenvector_via_qbeta(matrix, ring);
    if (!eigen.ok || !mathlib::verify_left_eigenvector(eigen.v, matrix, ring))
        throw std::runtime_error("exact left eigenvector failed");
    const auto v0_inverse = mathlib::invert_in_qbeta(eigen.v[0], ring);
    if (!v0_inverse.invertible)
        throw std::runtime_error("v0 normalization failed");
    for (auto& coordinate : eigen.v)
        coordinate = ring.mul(coordinate, v0_inverse.inverse);
    if (!ring.sub(eigen.v[0], ring.one()).is_zero() ||
        !mathlib::verify_left_eigenvector(eigen.v, matrix, ring))
        throw std::runtime_error("normalized eigenvector failed");
    return {std::move(ring), interval, std::move(eigen.v)};
}

mathlib::QElem height(const ExactContext& context,
                      const std::vector<long long>& x) {
    return mathlib::dot_qbeta(x, context.v, context.ring);
}

bool in_open_face_window(const ExactContext& context, const Node& node) {
    const auto t = height(context, node.x);
    return mathlib::qbeta_sign(
               context.ring.add(t, context.v[node.pair.i]), context.ring,
               context.beta_interval) > 0 &&
           mathlib::qbeta_sign(
               context.ring.sub(context.v[node.pair.j], t), context.ring,
               context.beta_interval) > 0;
}

std::set<Node> synthesize_nodes(std::size_t n, const ExactContext& context) {
    std::set<Node> nodes;
    for (const LabelPair pair : label_pairs(n)) {
        for (const auto& x : displacement_catalogue(n)) {
            Node candidate{pair, x};
            if (in_open_face_window(context, candidate)) nodes.insert(candidate);
        }
    }
    return nodes;
}

std::set<Node> synthesize_predicted_core(std::size_t n) {
    std::set<Node> nodes;
    for (const LabelPair pair : label_pairs(n))
        for (const auto& x : displacement_catalogue(n))
            if (predicted_core_member(n, pair, x)) nodes.insert({pair, x});
    return nodes;
}

std::set<std::pair<Node, Node>> synthesize_edges(
    std::size_t n, const std::set<Node>& nodes) {
    std::set<std::pair<Node, Node>> edges;
    for (const Node& source : nodes) {
        const long long delta =
            (source.pair.j > 0 ? 1LL : 0LL) -
            (source.pair.i > 0 ? 1LL : 0LL);
        const auto descriptor = describe_displacement(source.x);
        if (!descriptor)
            throw std::runtime_error("catalogue node lacks descriptor");
        const auto next_descriptor = forward_descriptor(n, *descriptor, delta);
        if (!next_descriptor) continue;
        const auto xprime = displacement_from_descriptor(n, *next_descriptor);
        if (xprime != forward_displacement(source.x, delta))
            throw std::runtime_error("symbolic/concrete update mismatch");
        for (const std::size_t ip : parents(n, source.pair.i)) {
            for (const std::size_t jp : parents(n, source.pair.j)) {
                Node destination{{ip, jp}, xprime};
                if (nodes.count(destination)) edges.insert({source, destination});
            }
        }
    }
    return edges;
}

std::size_t reached(const Node& start,
                    const std::map<Node, std::vector<Node>>& adjacency) {
    std::set<Node> seen{start};
    std::queue<Node> pending;
    pending.push(start);
    while (!pending.empty()) {
        const Node source = pending.front();
        pending.pop();
        const auto it = adjacency.find(source);
        if (it == adjacency.end()) continue;
        for (const Node& destination : it->second) {
            if (seen.insert(destination).second) pending.push(destination);
        }
    }
    return seen.size();
}

ravel::WeightedDigraph induced_component(
    const ravel::WeightedDigraph& graph,
    const std::vector<std::size_t>& vertices) {
    std::map<std::size_t, std::size_t> local;
    for (std::size_t k = 0; k < vertices.size(); ++k) local[vertices[k]] = k;
    ravel::WeightedDigraph result(vertices.size());
    for (std::size_t k = 0; k < vertices.size(); ++k) {
        for (const auto& [destination, weight] : graph.out_adj[vertices[k]]) {
            const auto it = local.find(destination);
            if (it != local.end()) result.add_edge(k, it->second, weight);
        }
    }
    return result;
}

struct ComponentDiagnostic {
    std::size_t size = 0;
    int quotient_classes = 0;
    double perron_estimate = 0.0;
};

bool check(std::size_t n, bool dump_schema = false) {
    const ExactContext context = exact_context(n);
    const auto displacements = displacement_catalogue(n);
    const auto universe = synthesize_nodes(n, context);
    const auto universe_edges = synthesize_edges(n, universe);

    std::vector<Node> universe_nodes(universe.begin(), universe.end());
    std::map<Node, std::size_t> index;
    for (std::size_t k = 0; k < universe_nodes.size(); ++k)
        index[universe_nodes[k]] = k;
    ravel::WeightedDigraph graph(universe_nodes.size());
    for (const auto& [source, destination] : universe_edges)
        graph.add_edge(index.at(source), index.at(destination));
    const auto components = ravel::tarjan_scc(graph);
    std::vector<std::size_t> dominant_indices;
    std::vector<ComponentDiagnostic> component_diagnostics;
    std::size_t candidate_components = 0;
    for (const auto& component : components) {
        if (!ravel::is_recurrent_scc(graph, component)) continue;
        const auto component_graph = induced_component(graph, component);
        const auto partition =
            ravel::coarsest_equitable_partition(component_graph);
        ComponentDiagnostic diagnostic{
            component.size(), partition.num_classes,
            ravel::dominant_eigenvalue_estimate_sparse(component_graph, 1000)};
        component_diagnostics.push_back(std::move(diagnostic));
        if (component.size() == predicted_core_size(n)) {
            ++candidate_components;
            dominant_indices = component;
        }
    }
    std::sort(component_diagnostics.begin(), component_diagnostics.end(),
              [](const auto& lhs, const auto& rhs) {
                  return lhs.perron_estimate > rhs.perron_estimate;
              });
    std::set<Node> discovered_cyclic_core;
    for (const std::size_t k : dominant_indices)
        discovered_cyclic_core.insert(universe_nodes[k]);
    const std::set<Node> nodes = synthesize_predicted_core(n);
    const bool direct_core_matches = nodes == discovered_cyclic_core;
    if (dump_schema) {
        for (const Node& node : universe) {
            const auto descriptor = describe_displacement(node.x);
            if (!descriptor)
                throw std::runtime_error("schema dump lacks descriptor");
            std::printf("SCHEMA n=%zu core=%d i=%zu j=%zu kind=%c sign=%lld "
                        "a=%zu b=%zu c=%zu\n",
                        n, nodes.count(node) ? 1 : 0, node.pair.i,
                        node.pair.j,
                        descriptor->kind == DisplacementKind::Root ? 'R' : 'T',
                        descriptor->sign, descriptor->a, descriptor->b,
                        descriptor->c);
        }
    }
    std::set<std::pair<Node, Node>> edges;
    for (const auto& edge : universe_edges)
        if (nodes.count(edge.first) && nodes.count(edge.second))
            edges.insert(edge);

    std::map<Node, std::vector<Node>> forward, reverse;
    std::set<LabelPair> actual_pairs;
    std::map<LabelPair, std::size_t> actual_pair_counts;
    std::set<LabelTransition> actual_label_transitions;
    for (const Node& node : nodes) {
        actual_pairs.insert(node.pair);
        ++actual_pair_counts[node.pair];
    }
    for (const auto& [source, destination] : edges) {
        forward[source].push_back(destination);
        reverse[destination].push_back(source);
        actual_label_transitions.insert({
            source.pair, destination.pair,
            (source.pair.j > 0 ? 1LL : 0LL) -
                (source.pair.i > 0 ? 1LL : 0LL)});
    }

    bool endpoints_ok = true;
    for (const LabelPair pair : label_pairs(n)) {
        bool have = false;
        mathlib::QElem minimum = context.ring.zero();
        mathlib::QElem maximum = context.ring.zero();
        for (const Node& node : nodes) {
            if (node.pair != pair) continue;
            const auto t = height(context, node.x);
            if (!have) {
                minimum = maximum = t;
                have = true;
            } else {
                if (mathlib::qbeta_sign(context.ring.sub(t, minimum),
                                        context.ring,
                                        context.beta_interval) < 0)
                    minimum = t;
                if (mathlib::qbeta_sign(context.ring.sub(t, maximum),
                                        context.ring,
                                        context.beta_interval) > 0)
                    maximum = t;
            }
        }
        const auto expected = endpoint_witnesses(n, pair);
        endpoints_ok = endpoints_ok && have &&
            context.ring.sub(minimum, height(context, expected.lower)).is_zero() &&
            context.ring.sub(maximum, height(context, expected.upper)).is_zero();
    }

    // `is_recurrent_scc` means cyclic (size > 1 or a self-loop), not
    // merely a closed/sink component.  Hence uniqueness here says that all
    // other diagonal blocks in Frobenius normal form are nilpotent: there is
    // no hidden competing nonzero spectral block in the synthesized universe.
    const bool unique_cyclic_core = component_diagnostics.size() == 1;
    const bool size_ok = direct_core_matches && unique_cyclic_core &&
                         candidate_components == 1 &&
                         nodes.size() == predicted_core_size(n);
    const bool pairs_ok = actual_pairs == label_pairs(n);
    bool pair_counts_ok = pairs_ok;
    for (const LabelPair pair : label_pairs(n))
        pair_counts_ok = pair_counts_ok &&
            actual_pair_counts[pair] == predicted_pair_node_count(n, pair);
    const bool edge_count_ok = edges.size() == predicted_core_edge_count(n);
    const bool transitions_ok = actual_label_transitions == label_transitions(n);
    const bool endpoints_witnessed = endpoints_ok;
    const bool strongly_connected = !nodes.empty() &&
        reached(*nodes.begin(), forward) == nodes.size() &&
        reached(*nodes.begin(), reverse) == nodes.size();
    const bool ok = displacements.size() == predicted_displacement_count(n) &&
        size_ok && pairs_ok && pair_counts_ok && edge_count_ok &&
        transitions_ok && endpoints_witnessed && strongly_connected;

    std::printf("n=%zu: displacements=%zu universe=%zu core=%zu edges=%zu; "
                "direct-core=%s size=%s pairs=%s strata=%s edge-count=%s transitions=%s "
                "endpoints=%s SCC=%s\n",
                n, displacements.size(), universe.size(), nodes.size(),
                edges.size(),
                direct_core_matches ? "exact" : "FAIL",
                size_ok ? "exact" : "FAIL", pairs_ok ? "exact" : "FAIL",
                pair_counts_ok ? "exact" : "FAIL",
                edge_count_ok ? "exact" : "FAIL",
                transitions_ok ? "exact" : "FAIL",
                endpoints_witnessed ? "exact" : "FAIL",
                strongly_connected ? "one" : "FAIL");
    std::printf("  cyclic SCC ledger (%zu):", component_diagnostics.size());
    for (const auto& diagnostic : component_diagnostics)
        std::printf(" %zu/q%d/rho%.6f", diagnostic.size,
                    diagnostic.quotient_classes,
                    diagnostic.perron_estimate);
    std::printf("\n");
    return ok;
}

}  // namespace

int main(int argc, char** argv) {
    if (argc == 3 && std::strcmp(argv[1], "--dump-schema") == 0) {
        const auto n = static_cast<std::size_t>(std::strtoul(argv[2], nullptr, 10));
        if (n < 3) {
            std::fprintf(stderr, "dimension must be at least 3\n");
            return 2;
        }
        const bool ok = check(n, true);
        std::printf("Refined exact invariant synthesis: %s\n",
                    ok ? "PASS" : "FAIL");
        return ok ? 0 : 1;
    }
    if (argc != 1) {
        std::fprintf(stderr, "usage: %s [--dump-schema N]\n", argv[0]);
        return 2;
    }
    bool ok = true;
    for (std::size_t n = 3; n <= 10; ++n) ok = check(n) && ok;
    std::printf("Refined exact invariant synthesis: %s\n", ok ? "PASS" : "FAIL");
    return ok ? 0 : 1;
}
