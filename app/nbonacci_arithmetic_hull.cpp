// nbonacci_arithmetic_hull.cpp
//
// Corona-free arithmetic over-approximation for the n-bonacci dominance
// theorem.  Instead of constructing C-coronas and reducing them, enumerate a
// bounded coefficient cube in Z^n, retain exactly those labelled translations
// in the Q(beta) face window, and install the exact n-bonacci inverse-incidence
// edges.  Any proved coefficient/carry bound that contains G_B turns this
// graph into a certified supergraph of G_B.  If the formula-defined core is
// already Perron-dominant here, corona exhaustion is unnecessary.
//
// The coefficient bound is currently an explicit command-line hypothesis.
// The program is therefore a discovery/certificate generator; the universal
// theorem still needs the carry-bound lemma connecting the arithmetic hull to
// every corona state.

#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <map>
#include <numeric>
#include <set>
#include <stdexcept>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

#include "math/ball.hpp"
#include "math/bezout.hpp"
#include "math/charpoly.hpp"
#include "math/linalg_qbeta.hpp"
#include "math/sturm.hpp"
#include "math/in_h_sigma.hpp"
#include "ravel/graph_divisor.hpp"
#include "ravel/nbonacci_margin_invariant.hpp"

namespace {

using namespace ravel;
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

struct Options {
    long long coefficient_bound = 1;
    bool exact = false;
    int exact_iterations = 160;
    bool dump_quotients = false;
    bool dump_relations = false;
    bool relaxed_relations = false;
    bool passive_carries = false;
    bool passive_only = false;
    bool dump_passive_relations = false;
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
        throw std::runtime_error("exact left Perron covector failed");
    const auto inverse = mathlib::invert_in_qbeta(eigen.v[0], ring);
    if (!inverse.invertible)
        throw std::runtime_error("exact Perron normalization failed");
    for (auto& coordinate : eigen.v)
        coordinate = ring.mul(coordinate, inverse.inverse);
    return {std::move(ring), interval, std::move(eigen.v)};
}

mathlib::QElem height(const ExactContext& context,
                      const std::vector<long long>& x) {
    return mathlib::dot_qbeta(x, context.v, context.ring);
}

bool exact_valid(const ExactContext& context, const Node& node) {
    bool zero = true;
    for (const auto coordinate : node.x) zero = zero && coordinate == 0;
    // Signed simple-node convention: only the trivial [i,0,i] is excluded;
    // both orientations with i!=j occur in +/- (A x H_sigma).
    if (zero && node.pair.i == node.pair.j) return false;
    const auto t = height(context, node.x);
    return mathlib::qbeta_sign(
               context.ring.add(t, context.v[node.pair.i]), context.ring,
               context.beta_interval) > 0 &&
           mathlib::qbeta_sign(
               context.ring.sub(context.v[node.pair.j], t), context.ring,
               context.beta_interval) > 0;
}

void enumerate_cube_recursive(
    std::vector<long long>& x, std::size_t coordinate, long long bound,
    const auto& callback) {
    if (coordinate == x.size()) {
        callback(x);
        return;
    }
    for (long long value = -bound; value <= bound; ++value) {
        x[coordinate] = value;
        enumerate_cube_recursive(x, coordinate + 1, bound, callback);
    }
}

std::set<Node> arithmetic_nodes(
    std::size_t n, long long bound, const ExactContext& context,
    std::size_t& translations_tested) {
    std::set<Node> result;
    std::vector<long long> x(n, 0);
    translations_tested = 0;
    enumerate_cube_recursive(x, 0, bound, [&](const auto& translation) {
        ++translations_tested;
        for (std::size_t i = 0; i < n; ++i)
            for (std::size_t j = 0; j < n; ++j) {
                Node node{{i, j}, translation};
                if (exact_valid(context, node)) result.insert(std::move(node));
            }
    });
    return result;
}

std::set<std::pair<Node, Node>> arithmetic_edges(
    std::size_t n, const std::set<Node>& nodes) {
    std::set<std::pair<Node, Node>> result;
    for (const auto& source : nodes) {
        const long long delta =
            (source.pair.j > 0 ? 1LL : 0LL) -
            (source.pair.i > 0 ? 1LL : 0LL);
        const auto next_x = forward_displacement(source.x, delta);
        for (const auto i : parents(n, source.pair.i))
            for (const auto j : parents(n, source.pair.j)) {
                const Node destination{{i, j}, next_x};
                if (nodes.count(destination)) result.insert({source, destination});
            }
    }
    return result;
}

WeightedDigraph induced_component(
    const WeightedDigraph& graph, const std::vector<std::size_t>& vertices) {
    std::map<std::size_t, std::size_t> local;
    for (std::size_t k = 0; k < vertices.size(); ++k) local[vertices[k]] = k;
    WeightedDigraph result(vertices.size());
    for (std::size_t k = 0; k < vertices.size(); ++k)
        for (const auto& [destination, weight] : graph.out_adj[vertices[k]]) {
            const auto found = local.find(destination);
            if (found != local.end())
                result.add_edge(k, found->second, weight);
        }
    return result;
}

WeightedDigraph translation_envelope(
    const std::vector<Node>& nodes, const WeightedDigraph& graph,
    const std::vector<std::size_t>& component) {
    std::set<std::vector<long long>> translations;
    for (const auto vertex : component) translations.insert(nodes[vertex].x);
    std::vector<std::vector<long long>> translation_vector(
        translations.begin(), translations.end());
    std::map<std::vector<long long>, std::size_t> translation_index;
    for (std::size_t k = 0; k < translation_vector.size(); ++k)
        translation_index[translation_vector[k]] = k;
    const std::set<std::size_t> component_set(
        component.begin(), component.end());
    std::map<std::pair<std::size_t, std::size_t>, long long> maximum_weight;
    for (const auto source : component) {
        std::map<std::size_t, long long> row_counts;
        for (const auto& [destination, weight] : graph.out_adj[source])
            if (component_set.count(destination))
                row_counts[translation_index.at(nodes[destination].x)] += weight;
        const auto source_translation = translation_index.at(nodes[source].x);
        for (const auto& [destination_translation, weight] : row_counts) {
            auto& maximum = maximum_weight[
                {source_translation, destination_translation}];
            maximum = std::max(maximum, weight);
        }
    }
    WeightedDigraph result(translation_vector.size());
    for (const auto& [edge, weight] : maximum_weight)
        result.add_edge(edge.first, edge.second, weight);
    return result;
}

struct ProjectionCertificate {
    bool found = false;
    bool strict = false;
    std::size_t image_size = 0;
    std::size_t missing_core_edges = 0;
};

// Search for the especially strong 0-1 intertwiner suggested by the graded
// displacement data.  A shell vertex is sent to a core vertex with the same
// face labels.  Once the image of one vertex is chosen, the deterministic
// displacement update forces the image of every successor; strong
// connectivity therefore turns each seed choice into a linear-time exact
// consistency check.
//
// On success, f sends every shell edge to a distinct core edge and hence its
// incidence matrix P obeys A_shell P <= P A_core.  A missing core branch makes
// the inequality strict.  This is the desired symbolic Perron certificate,
// not a numerical comparison.
ProjectionCertificate find_core_projection(
    std::size_t n, const std::vector<Node>& nodes,
    const WeightedDigraph& graph, const std::vector<std::size_t>& component,
    const std::set<Node>& core_nodes) {
    const std::set<std::size_t> component_set(
        component.begin(), component.end());
    const std::size_t seed = component.front();
    std::vector<Node> seed_images;
    for (const auto& core : core_nodes)
        if (core.pair == nodes[seed].pair) seed_images.push_back(core);

    for (const auto& seed_image : seed_images) {
        std::map<std::size_t, Node> image;
        std::vector<std::size_t> pending{seed};
        image.emplace(seed, seed_image);
        bool consistent = true;
        for (std::size_t head = 0; head < pending.size() && consistent; ++head) {
            const std::size_t source = pending[head];
            const auto& projected_source = image.at(source);
            const long long delta =
                (nodes[source].pair.j > 0 ? 1LL : 0LL) -
                (nodes[source].pair.i > 0 ? 1LL : 0LL);
            const auto projected_x =
                forward_displacement(projected_source.x, delta);
            for (const auto& [destination, weight] : graph.out_adj[source]) {
                if (!component_set.count(destination)) continue;
                if (weight != 1) {
                    consistent = false;
                    break;
                }
                const Node projected_destination{
                    nodes[destination].pair, projected_x};
                if (!core_nodes.count(projected_destination)) {
                    consistent = false;
                    break;
                }
                const auto inserted = image.emplace(
                    destination, projected_destination);
                if (inserted.second) pending.push_back(destination);
                else if (inserted.first->second != projected_destination) {
                    consistent = false;
                    break;
                }
            }
        }
        if (!consistent || image.size() != component.size()) continue;

        bool dominated = true;
        bool strict = false;
        std::size_t missing = 0;
        std::set<Node> image_nodes;
        for (const auto& [vertex, projected] : image) {
            image_nodes.insert(projected);
            std::multiset<Node> shell_images;
            for (const auto& [destination, weight] : graph.out_adj[vertex]) {
                if (!component_set.count(destination)) continue;
                for (long long copy = 0; copy < weight; ++copy)
                    shell_images.insert(image.at(destination));
            }
            std::multiset<Node> core_successors;
            const long long delta =
                (projected.pair.j > 0 ? 1LL : 0LL) -
                (projected.pair.i > 0 ? 1LL : 0LL);
            const auto next_x = forward_displacement(projected.x, delta);
            for (const auto i : parents(n, projected.pair.i))
                for (const auto j : parents(n, projected.pair.j)) {
                    const Node successor{{i, j}, next_x};
                    if (core_nodes.count(successor))
                        core_successors.insert(successor);
                }
            for (const auto& successor : shell_images) {
                const auto found = core_successors.find(successor);
                if (found == core_successors.end()) {
                    dominated = false;
                    break;
                }
                core_successors.erase(found);
            }
            if (!dominated) break;
            missing += core_successors.size();
            strict = strict || !core_successors.empty();
        }
        if (dominated)
            return {true, strict, image_nodes.size(), missing};
    }
    return {};
}

std::size_t edge_sum(const WeightedDigraph& graph) {
    std::size_t result = 0;
    for (const auto& row : graph.out_adj)
        for (const auto& [destination, weight] : row) {
            (void)destination;
            result += static_cast<std::size_t>(weight);
        }
    return result;
}

std::size_t graph_period(const WeightedDigraph& graph) {
    if (graph.n == 0) return 0;
    std::vector<long long> distance(graph.n, -1);
    std::vector<std::size_t> pending{0};
    distance[0] = 0;
    for (std::size_t head = 0; head < pending.size(); ++head) {
        const auto source = pending[head];
        for (const auto& [destination, weight] : graph.out_adj[source]) {
            (void)weight;
            if (distance[destination] == -1) {
                distance[destination] = distance[source] + 1;
                pending.push_back(destination);
            }
        }
    }
    std::size_t period = 0;
    for (std::size_t source = 0; source < graph.n; ++source)
        for (const auto& [destination, weight] : graph.out_adj[source]) {
            (void)weight;
            const long long defect =
                distance[source] + 1 - distance[destination];
            period = std::gcd(
                period, static_cast<std::size_t>(std::llabs(defect)));
        }
    return period;
}

std::set<Node> generated_core(std::size_t n) {
    std::set<Node> result;
    for (const auto pair : label_pairs(n))
        for (const auto descriptor : displacement_descriptors(n))
            if (predicted_core_member(n, pair, descriptor))
                result.insert({pair,
                    displacement_from_descriptor(n, descriptor)});
    return result;
}

void report_passive_carry_graph(std::size_t n) {
    const auto atom_set = displacement_catalogue(n);
    const std::vector<std::vector<long long>> atoms(
        atom_set.begin(), atom_set.end());
    std::map<std::vector<long long>, std::size_t> index;
    for (std::size_t k = 0; k < atoms.size(); ++k) index[atoms[k]] = k;
    WeightedDigraph graph(atoms.size());
    std::size_t labelled_edges = 0;
    for (std::size_t source = 0; source < atoms.size(); ++source)
        for (long long delta = -2; delta <= 2; ++delta) {
            const auto target = forward_displacement(atoms[source], delta);
            const auto found = index.find(target);
            if (found != index.end()) {
                graph.add_edge(source, found->second, 1);
                ++labelled_edges;
            }
        }
    auto components = tarjan_scc(graph);
    std::vector<std::vector<std::size_t>> cyclic;
    for (auto& component : components)
        if (is_recurrent_scc(graph, component)) cyclic.push_back(std::move(component));
    std::sort(cyclic.begin(), cyclic.end(), [](const auto& lhs, const auto& rhs) {
        return lhs.size() > rhs.size();
    });
    std::printf("passive carry graph: atoms=%zu labelled_edges=%zu cyclic_SCCs=%zu\n",
                atoms.size(), labelled_edges, cyclic.size());
    for (std::size_t k = 0; k < cyclic.size(); ++k) {
        const auto component = induced_component(graph, cyclic[k]);
        const auto partition = coarsest_equitable_partition(component);
        std::printf("  passive SCC %zu nodes=%zu edges=%zu q=%zu period=%zu rho~=%.9f\n",
                    k, component.n, edge_sum(component),
                    static_cast<std::size_t>(partition.num_classes),
                    graph_period(component),
                    dominant_eigenvalue_estimate_sparse(component, 4000));
    }
}

const char* order_name(mathlib::PerronOrder order) {
    switch (order) {
        case mathlib::PerronOrder::less: return "core<competitor";
        case mathlib::PerronOrder::equal: return "equal";
        case mathlib::PerronOrder::greater: return "core>competitor";
        case mathlib::PerronOrder::inconclusive: return "inconclusive";
    }
    return "invalid";
}

void dump_quotient(std::size_t n, const char* kind, std::size_t component,
                   const std::vector<std::vector<long long>>& matrix) {
    std::printf("QBEGIN n=%zu kind=%s component=%zu rows=%zu\n",
                n, kind, component, matrix.size());
    for (std::size_t row = 0; row < matrix.size(); ++row)
        for (std::size_t column = 0; column < matrix[row].size(); ++column)
            if (matrix[row][column] != 0)
                std::printf("QEDGE %zu %zu %lld\n",
                            row, column, matrix[row][column]);
    std::printf("QEND\n");
}

void dump_label_relation(
    std::size_t n, std::size_t component_number,
    const std::vector<Node>& all_nodes, const WeightedDigraph& graph,
    const std::vector<std::size_t>& shell_vertices,
    const std::set<Node>& core_node_set, bool relaxed) {
    std::vector<Node> core_nodes(core_node_set.begin(), core_node_set.end());
    std::map<Node, std::size_t> core_index;
    for (std::size_t k = 0; k < core_nodes.size(); ++k)
        core_index[core_nodes[k]] = k;
    std::map<std::size_t, std::size_t> shell_index;
    for (std::size_t k = 0; k < shell_vertices.size(); ++k)
        shell_index[shell_vertices[k]] = k;

    const auto atom_set = displacement_catalogue(n);
    std::set<std::vector<long long>> two_sums;
    const std::vector<std::vector<long long>> atoms(
        atom_set.begin(), atom_set.end());
    for (std::size_t lhs = 0; lhs < atoms.size(); ++lhs)
        for (std::size_t rhs = lhs; rhs < atoms.size(); ++rhs) {
            std::vector<long long> sum(n, 0);
            for (std::size_t coordinate = 0; coordinate < n; ++coordinate)
                sum[coordinate] =
                    atoms[lhs][coordinate] + atoms[rhs][coordinate];
            two_sums.insert(std::move(sum));
        }

    std::vector<std::pair<std::size_t, std::size_t>> allowed;
    std::vector<std::size_t> allowed_per_shell(shell_vertices.size(), 0);
    for (std::size_t shell = 0; shell < shell_vertices.size(); ++shell) {
        const auto& node = all_nodes[shell_vertices[shell]];
        const bool shell_is_grade_two = two_sums.count(node.x) > 0;
        for (std::size_t core = 0; core < core_nodes.size(); ++core) {
            std::vector<long long> remainder(n, 0);
            for (std::size_t coordinate = 0; coordinate < n; ++coordinate)
                remainder[coordinate] =
                    node.x[coordinate] - core_nodes[core].x[coordinate];
            const bool exact_lower_grade = relaxed
                ? (atom_set.count(remainder) > 0 ||
                   two_sums.count(remainder) > 0)
                : (shell_is_grade_two
                    ? atom_set.count(remainder) > 0
                    : two_sums.count(remainder) > 0);
            if (exact_lower_grade) {
                allowed.push_back({shell, core});
                ++allowed_per_shell[shell];
            }
        }
    }
    // A component with no grade-one summand is outside this two-stage
    // relation. Terminal permutation components already have an explicit
    // rho=1 certificate, so omit rather than admit arbitrary supports.
    if (std::find(allowed_per_shell.begin(), allowed_per_shell.end(), 0) !=
        allowed_per_shell.end())
        return;

    std::printf("RBEGIN n=%zu component=%zu shell=%zu core=%zu allowed=%zu\n",
                n, component_number, shell_vertices.size(), core_nodes.size(),
                allowed.size());
    for (std::size_t source = 0; source < core_nodes.size(); ++source) {
        const auto& node = core_nodes[source];
        const long long delta =
            (node.pair.j > 0 ? 1LL : 0LL) -
            (node.pair.i > 0 ? 1LL : 0LL);
        const auto next_x = forward_displacement(node.x, delta);
        for (const auto i : parents(n, node.pair.i))
            for (const auto j : parents(n, node.pair.j)) {
                const auto found = core_index.find(Node{{i, j}, next_x});
                if (found != core_index.end())
                    std::printf("RCE %zu %zu 1\n", source, found->second);
            }
    }
    for (std::size_t source = 0; source < shell_vertices.size(); ++source)
        for (const auto& [destination, weight] :
             graph.out_adj[shell_vertices[source]]) {
            const auto found = shell_index.find(destination);
            if (found != shell_index.end())
                std::printf("RSE %zu %zu %lld\n",
                            source, found->second, weight);
        }
    for (const auto& [shell, core] : allowed)
        std::printf("RALLOW %zu %zu\n", shell, core);
    std::printf("REND\n");
}

void dump_passive_relation(
    std::size_t n, std::size_t component_number,
    const std::vector<Node>& all_nodes, const WeightedDigraph& graph,
    const std::vector<std::size_t>& shell_vertices) {
    const auto atom_set = displacement_catalogue(n);
    const std::vector<std::vector<long long>> atoms(
        atom_set.begin(), atom_set.end());
    std::map<std::vector<long long>, std::size_t> atom_index;
    for (std::size_t k = 0; k < atoms.size(); ++k) atom_index[atoms[k]] = k;
    std::set<std::vector<long long>> two_sums;
    for (std::size_t lhs = 0; lhs < atoms.size(); ++lhs)
        for (std::size_t rhs = lhs; rhs < atoms.size(); ++rhs) {
            std::vector<long long> sum(n, 0);
            for (std::size_t coordinate = 0; coordinate < n; ++coordinate)
                sum[coordinate] =
                    atoms[lhs][coordinate] + atoms[rhs][coordinate];
            two_sums.insert(std::move(sum));
        }
    std::map<std::size_t, std::size_t> shell_index;
    for (std::size_t k = 0; k < shell_vertices.size(); ++k)
        shell_index[shell_vertices[k]] = k;

    std::vector<std::pair<std::size_t, std::size_t>> allowed;
    std::vector<std::size_t> allowed_per_shell(shell_vertices.size(), 0);
    for (std::size_t shell = 0; shell < shell_vertices.size(); ++shell) {
        const auto& x = all_nodes[shell_vertices[shell]].x;
        const bool grade_two = two_sums.count(x) > 0;
        for (std::size_t atom = 0; atom < atoms.size(); ++atom) {
            std::vector<long long> remainder(n, 0);
            for (std::size_t coordinate = 0; coordinate < n; ++coordinate)
                remainder[coordinate] = x[coordinate] - atoms[atom][coordinate];
            const bool exact_lower_grade = grade_two
                ? atom_set.count(remainder) > 0
                : two_sums.count(remainder) > 0;
            if (exact_lower_grade) {
                allowed.push_back({shell, atom});
                ++allowed_per_shell[shell];
            }
        }
    }
    if (std::find(allowed_per_shell.begin(), allowed_per_shell.end(), 0) !=
        allowed_per_shell.end())
        return;

    std::printf("RBEGIN n=%zu component=%zu shell=%zu core=%zu allowed=%zu\n",
                n, component_number, shell_vertices.size(), atoms.size(),
                allowed.size());
    for (std::size_t source = 0; source < atoms.size(); ++source)
        for (long long delta = -2; delta <= 2; ++delta) {
            const auto target = forward_displacement(atoms[source], delta);
            const auto found = atom_index.find(target);
            if (found != atom_index.end())
                std::printf("RCE %zu %zu 1\n", source, found->second);
        }
    for (std::size_t source = 0; source < shell_vertices.size(); ++source)
        for (const auto& [destination, weight] :
             graph.out_adj[shell_vertices[source]]) {
            const auto found = shell_index.find(destination);
            if (found != shell_index.end())
                std::printf("RSE %zu %zu %lld\n",
                            source, found->second, weight);
        }
    for (const auto& [shell, atom] : allowed)
        std::printf("RALLOW %zu %zu\n", shell, atom);
    std::printf("REND\n");
}

bool check(std::size_t n, const Options& options) {
    std::printf("\n=== arithmetic hull n=%zu coefficient_bound=%lld ===\n",
                n, options.coefficient_bound);
    const auto context = exact_context(n);
    if (options.passive_carries) report_passive_carry_graph(n);
    if (options.passive_only) return true;
    std::size_t translations_tested = 0;
    const auto nodes = arithmetic_nodes(
        n, options.coefficient_bound, context, translations_tested);
    const auto edges = arithmetic_edges(n, nodes);
    std::vector<Node> node_vector(nodes.begin(), nodes.end());
    std::map<Node, std::size_t> index;
    for (std::size_t k = 0; k < node_vector.size(); ++k)
        index[node_vector[k]] = k;
    WeightedDigraph graph(node_vector.size());
    for (const auto& [source, destination] : edges)
        graph.add_edge(index.at(source), index.at(destination));
    std::printf("exact enumeration: translations=%zu labelled_nodes=%zu edges=%zu\n",
                translations_tested, nodes.size(), edges.size());

    auto all_components = tarjan_scc(graph);
    std::vector<std::vector<std::size_t>> cyclic;
    for (auto& component : all_components)
        if (is_recurrent_scc(graph, component)) cyclic.push_back(std::move(component));
    std::sort(cyclic.begin(), cyclic.end(), [](const auto& lhs, const auto& rhs) {
        if (lhs.size() != rhs.size()) return lhs.size() > rhs.size();
        return *std::min_element(lhs.begin(), lhs.end()) <
               *std::min_element(rhs.begin(), rhs.end());
    });

    const auto expected = generated_core(n);
    std::set<std::size_t> expected_indices;
    std::size_t missing = 0;
    for (const auto& node : expected) {
        const auto found = index.find(node);
        if (found == index.end()) ++missing;
        else expected_indices.insert(found->second);
    }
    std::size_t core_component = cyclic.size();
    bool partial_overlap = false;
    for (std::size_t k = 0; k < cyclic.size(); ++k) {
        const std::set<std::size_t> component(
            cyclic[k].begin(), cyclic[k].end());
        if (component == expected_indices) core_component = k;
        else {
            std::vector<std::size_t> overlap;
            std::set_intersection(component.begin(), component.end(),
                                  expected_indices.begin(), expected_indices.end(),
                                  std::back_inserter(overlap));
            partial_overlap = partial_overlap || !overlap.empty();
        }
    }
    const bool core_exact = missing == 0 && !partial_overlap &&
                            core_component < cyclic.size();
    std::printf("cyclic_SCCs=%zu generated_core=%zu missing=%zu core_SCC=%s\n",
                cyclic.size(), expected.size(), missing,
                core_exact ? "EXACT" : "FAIL");
    if (!core_exact) return false;

    const auto core_graph = induced_component(graph, cyclic[core_component]);
    const auto core_partition = coarsest_equitable_partition(core_graph);
    const auto core_quotient = quotient_matrix(core_graph, core_partition);
    const double core_rho = dominant_eigenvalue_estimate_sparse(core_graph, 4000);
    if (options.dump_quotients)
        dump_quotient(n, "core", core_component, core_quotient);
    bool dominant_estimate = true;
    bool exact_dominance = true;
    for (std::size_t k = 0; k < cyclic.size(); ++k) {
        const auto component_graph = induced_component(graph, cyclic[k]);
        const auto partition = coarsest_equitable_partition(component_graph);
        const auto quotient = quotient_matrix(component_graph, partition);
        if (options.dump_quotients && k != core_component)
            dump_quotient(n, "shell", k, quotient);
        const double rho = dominant_eigenvalue_estimate_sparse(component_graph, 4000);
        const auto envelope =
            translation_envelope(node_vector, graph, cyclic[k]);
        const double envelope_rho =
            dominant_eigenvalue_estimate_sparse(envelope, 4000);
        std::map<std::size_t, std::size_t> support;
        long long max_coefficient = 0;
        for (const auto vertex : cyclic[k]) {
            std::size_t occupied = 0;
            for (const auto coefficient : node_vector[vertex].x) {
                occupied += coefficient != 0 ? 1 : 0;
                max_coefficient = std::max(max_coefficient,
                                            std::llabs(coefficient));
            }
            ++support[occupied];
        }
        std::printf("SCC %zu%s nodes=%zu edges=%zu q=%zu period=%zu rho~=%.12f "
                    "translation_envelope=(%zu,%.12f) "
                    "gap~=%.12f maxcoeff=%lld support=",
                    k, k == core_component ? " [CORE]" : "",
                    component_graph.n, edge_sum(component_graph), quotient.size(),
                    graph_period(component_graph),
                    rho, envelope.n, envelope_rho,
                    core_rho - rho, max_coefficient);
        bool first = true;
        for (const auto& [occupied, count] : support) {
            std::printf("%s%zu:%zu", first ? "" : ",", occupied, count);
            first = false;
        }
        std::printf("\n");
        if (k == core_component) continue;
        if (options.dump_relations)
            dump_label_relation(
                n, k, node_vector, graph, cyclic[k], expected,
                options.relaxed_relations);
        if (options.dump_passive_relations)
            dump_passive_relation(
                n, k, node_vector, graph, cyclic[k]);
        const auto projection = find_core_projection(
            n, node_vector, graph, cyclic[k], expected);
        std::printf("  core projection: %s%s image=%zu missing_core_edges=%zu\n",
                    projection.found ? "FOUND" : "none",
                    projection.found ? (projection.strict ? " (STRICT)" : " (equality)") : "",
                    projection.image_size, projection.missing_core_edges);
        dominant_estimate = dominant_estimate && core_rho > rho + 1e-9;
        if (options.exact) {
            const auto comparison = mathlib::compare_perron_roots_exact(
                core_quotient, quotient, options.exact_iterations, 1e-10);
            std::printf("  exact: %s\n", order_name(comparison.order));
            exact_dominance = exact_dominance &&
                comparison.order == mathlib::PerronOrder::greater;
        }
    }
    std::printf("arithmetic-supergraph dominance: estimate=%s exact=%s\n",
                dominant_estimate ? "PASS" : "FAIL",
                options.exact ? (exact_dominance ? "PASS" : "FAIL/INCONCLUSIVE")
                              : "not-requested");
    return dominant_estimate && (!options.exact || exact_dominance);
}

}  // namespace

int main(int argc, char** argv) {
    Options options;
    std::vector<std::size_t> dimensions;
    for (int arg = 1; arg < argc; ++arg) {
        const std::string value(argv[arg]);
        if (value == "--exact") options.exact = true;
        else if (value == "--dump-quotients") options.dump_quotients = true;
        else if (value == "--dump-relations") options.dump_relations = true;
        else if (value == "--relaxed-relations") options.relaxed_relations = true;
        else if (value == "--passive-carries") options.passive_carries = true;
        else if (value == "--passive-only") {
            options.passive_carries = true;
            options.passive_only = true;
        }
        else if (value == "--dump-passive-relations")
            options.dump_passive_relations = true;
        else if (value.rfind("--bound=", 0) == 0)
            options.coefficient_bound = std::stoll(value.substr(8));
        else if (value.rfind("--exact-iters=", 0) == 0)
            options.exact_iterations = std::stoi(value.substr(14));
        else dimensions.push_back(static_cast<std::size_t>(std::stoul(value)));
    }
    if (options.coefficient_bound < 1 || options.coefficient_bound > 3) {
        std::fprintf(stderr, "--bound must be in 1..3\n");
        return 2;
    }
    if (dimensions.empty()) dimensions = {3, 4, 5, 6};
    bool ok = true;
    for (const auto n : dimensions) ok = check(n, options) && ok;
    return ok ? 0 : 1;
}
