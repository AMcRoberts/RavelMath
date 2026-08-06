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
#include <queue>
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
#include "ravel/proof/graded_core_descent.hpp"
#include "ravel/proof/marked_atom_core_extension.hpp"
#include "ravel/proof/complete_atom_tuple_extension.hpp"
#include "ravel/proof/atom_incidence_intertwiner.hpp"
#include "ravel/proof/face_relative_sign_grammar.hpp"
#include "ravel/proof/phase_handoff_renewal.hpp"
#include "ravel/proof/parametric_face_sign_role.hpp"
#include "ravel/proof/marked_decomposition_correspondence.hpp"
#include "ravel/proof/colored_synchronized_correspondence.hpp"
#include "ravel/proof/induced_path_injective_simulation.hpp"
#include "ravel/proof/right_resolving_language_inclusion.hpp"
#include "ravel/proof/right_resolving_global_language_inclusion.hpp"
#include "ravel/proof/finite_quotient_core_maximality.hpp"
#include "ravel/proof/reflective_collatz_weight.hpp"
#include "ravel/proof/paired_matrix_dominance.hpp"
#include "ravel/proof/reflective_weight_grammar.hpp"
#include "ravel/proof/symbolic_weight_factorization.hpp"

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
    bool dump_core_exhaustion = false;
    bool structural_only = false;
    bool weights_only = false;
    bool dump_weights = false;
};


using ravel::proof::DisplacementSumCatalogue;

Node mirror_node(const Node& node) {
    Node result;
    result.pair = {node.pair.j, node.pair.i};
    result.x.resize(node.x.size());
    for (std::size_t i = 0; i < node.x.size(); ++i) result.x[i] = -node.x[i];
    return result;
}

bool is_permutation_component(const WeightedDigraph& graph) {
    if (graph.n == 0) return false;
    for (std::size_t vertex = 0; vertex < graph.n; ++vertex) {
        std::size_t out = 0, in = 0;
        for (const auto& [destination, weight] : graph.out_adj[vertex]) {
            (void)destination; out += static_cast<std::size_t>(weight);
        }
        for (const auto& [source, weight] : graph.in_adj[vertex]) {
            (void)source; in += static_cast<std::size_t>(weight);
        }
        if (out != 1 || in != 1) return false;
    }
    return true;
}

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
    std::size_t outgoing_core_edges = 0;
    std::size_t incoming_core_edges = 0;
    for (const auto& [source, destination] : edges) {
        const bool source_core = expected.count(source) > 0;
        const bool destination_core = expected.count(destination) > 0;
        if (source_core && !destination_core) ++outgoing_core_edges;
        if (!source_core && destination_core) ++incoming_core_edges;
    }

    // Exhaustion is not forward closure: the core has outgoing transient
    // branches.  What matters is that no first step leaving the core can ever
    // return to it.  Compute the full reverse basin of the core once, then
    // check every outgoing boundary edge against it.
    std::vector<bool> can_reach_core(graph.n, false);
    std::queue<std::size_t> reverse_pending;
    for (const auto vertex : expected_indices) {
        can_reach_core[vertex] = true;
        reverse_pending.push(vertex);
    }
    while (!reverse_pending.empty()) {
        const auto target = reverse_pending.front();
        reverse_pending.pop();
        for (const auto& [source, ignored_weight] : graph.in_adj[target]) {
            if (!can_reach_core[source]) {
                can_reach_core[source] = true;
                reverse_pending.push(source);
            }
            (void)ignored_weight;
        }
    }
    std::size_t returning_outgoing_edges = 0;
    std::size_t reverse_basin_outside_core = 0;
    for (std::size_t vertex = 0; vertex < graph.n; ++vertex)
        if (can_reach_core[vertex] && !expected_indices.count(vertex))
            ++reverse_basin_outside_core;
    for (const auto& [source, destination] : edges) {
        if (expected.count(source) && !expected.count(destination) &&
            can_reach_core[index.at(destination)])
            ++returning_outgoing_edges;
    }
    const bool no_return_after_exit = returning_outgoing_edges == 0;
    std::printf("cyclic_SCCs=%zu generated_core=%zu missing=%zu core_SCC=%s "
                "outgoing=%zu incoming=%zu returning_outgoing=%zu "
                "reverse_basin_outside=%zu no_return=%s\n",
                cyclic.size(), expected.size(), missing,
                core_exact ? "EXACT" : "FAIL",
                outgoing_core_edges, incoming_core_edges,
                returning_outgoing_edges, reverse_basin_outside_core,
                no_return_after_exit ? "PASS" : "FAIL");
    if (options.dump_core_exhaustion) {
        std::printf("XBEGIN n=%zu nodes=%zu core=%zu edges=%zu\n",
                    n, nodes.size(), expected.size(), edges.size());
        std::printf("XOCCURS %zu\n",
                    missing == 0 ? expected.size() : expected.size() - missing);
        std::printf("XOUT %zu\n", outgoing_core_edges);
        std::printf("XIN %zu\n", incoming_core_edges);
        std::printf("XRETURNOUT %zu\n", returning_outgoing_edges);
        std::printf("XREVERSEOUTSIDE %zu\n", reverse_basin_outside_core);
        std::printf("XSCC %s\n", core_exact ? "EXACT" : "FAIL");
        std::printf("XEND\n");
    }
    if (!core_exact || !no_return_after_exit) return false;

    if (options.structural_only) {
        const DisplacementSumCatalogue catalogue(n);
        std::vector<std::size_t> core_vertices(expected_indices.begin(), expected_indices.end());
        std::vector<std::size_t> cyclic_vertices;
        for (const auto& component : cyclic)
            cyclic_vertices.insert(cyclic_vertices.end(), component.begin(), component.end());
        std::map<std::size_t, std::size_t> cyclic_local;
        for (std::size_t i = 0; i < cyclic_vertices.size(); ++i)
            cyclic_local[cyclic_vertices[i]] = i;
        std::map<std::size_t, std::size_t> core_local;
        for (std::size_t i = 0; i < core_vertices.size(); ++i)
            core_local[core_vertices[i]] = i;
        const auto structural_core_graph = induced_component(graph, core_vertices);
        auto structural_dense = [](const auto& g) {
            std::vector<std::vector<long long>> matrix(
                g.n, std::vector<long long>(g.n, 0));
            for (std::size_t u = 0; u < g.n; ++u)
                for (const auto& [v, w] : g.out_adj[u]) matrix[u][v] += w;
            return matrix;
        };
        std::vector<std::vector<ravel::proof::ColoredEdgeOccurrence>> core_colored(core_vertices.size());
        for (std::size_t local = 0; local < core_vertices.size(); ++local) {
            const auto global = core_vertices[local];
            const auto pair = node_vector[global].pair;
            const auto color = static_cast<std::size_t>(((pair.j > 0) ? 1 : 0) != ((pair.i > 0) ? 1 : 0));
            for (const auto& [destination, weight] : graph.out_adj[global]) {
                auto it = core_local.find(destination); if (it == core_local.end()) continue;
                for (long long copy=0; copy<weight; ++copy) core_colored[local].push_back({it->second,color});
            }
        }
        for (std::size_t k = 0; k < cyclic.size(); ++k) {
            const auto component_graph = induced_component(graph, cyclic[k]);
            const std::set<std::size_t> component_set(
                cyclic[k].begin(), cyclic[k].end());
            std::map<std::size_t, std::size_t> support;
            std::map<std::size_t, std::size_t> grades;
            std::map<std::size_t, std::size_t> recursive_by_grade;
            std::size_t mirror_hits = 0;
            std::size_t projectable = 0;
            std::size_t recursively_projectable = 0;
            long long max_l1 = 0;
            for (const auto vertex : cyclic[k]) {
                const auto& node = node_vector[vertex];
                const auto mirror = index.find(mirror_node(node));
                if (mirror != index.end() && component_set.count(mirror->second))
                    ++mirror_hits;
                ++grades[catalogue.grade(node.x)];
                const auto node_grade = catalogue.grade(node.x);
                if (!catalogue.derive_one_stage(node.pair, node.x).empty())
                    ++projectable;
                if (node_grade > 1 && !catalogue.derive_one_stage(node.pair, node.x).empty()) {
                    ++recursively_projectable;
                    ++recursive_by_grade[node_grade];
                }
                std::size_t occupied = 0;
                long long l1 = 0;
                for (const auto coefficient : node.x) {
                    occupied += coefficient != 0 ? 1 : 0;
                    l1 += std::llabs(coefficient);
                }
                ++support[occupied];
                max_l1 = std::max(max_l1, l1);
            }
            std::printf(
                "GSCC %zu%s nodes=%zu edges=%zu period=%zu permutation=%s "
                "mirror=%zu/%zu projectable=%zu/%zu recursive=%zu/%zu maxL1=%lld support=",
                k, k == core_component ? " [CORE]" : "",
                component_graph.n, edge_sum(component_graph),
                graph_period(component_graph),
                is_permutation_component(component_graph) ? "YES" : "NO",
                mirror_hits, cyclic[k].size(), projectable, cyclic[k].size(),
                recursively_projectable, cyclic[k].size(), max_l1);
            bool first = true;
            for (const auto& [occupied, count] : support) {
                std::printf("%s%zu:%zu", first ? "" : ",", occupied, count);
                first = false;
            }
            std::printf(" grade=");
            first = true;
            for (const auto& [grade, count] : grades) {
                std::printf("%s%zu:%zu", first ? "" : ",", grade, count);
                first = false;
            }
            std::printf(" recursiveByGrade=");
            first = true;
            for (const auto& [grade, count] : recursive_by_grade) {
                std::printf("%s%zu:%zu", first ? "" : ",", grade, count);
                first = false;
            }
            const auto descent_summary = ravel::proof::derive_component_grade_descent(
                catalogue, cyclic[k],
                [&](std::size_t vertex) { return node_vector[vertex].pair; },
                [&](std::size_t vertex) { return node_vector[vertex].x; });
            const auto marked_extension =
                ravel::proof::derive_marked_atom_core_extension(
                    catalogue, component_graph.n,
                    [&](std::size_t local) {
                        std::vector<std::size_t> destinations;
                        for (const auto& [destination, weight] : component_graph.out_adj[local])
                            for (long long copy = 0; copy < weight; ++copy)
                                destinations.push_back(destination);
                        return destinations;
                    },
                    [&](std::size_t local) {
                        return node_vector[cyclic[k][local]].pair;
                    },
                    [&](std::size_t local) {
                        return node_vector[cyclic[k][local]].x;
                    });
            std::printf("  marked-atom extension: %s marks=%zu maxFiber=%zu nondet=%zu collapsed=%zu reason=%s\n",
                        marked_extension.path_injection_to_core ? "PASS" : "FAIL",
                        marked_extension.marked_states,
                        marked_extension.maximum_fibre,
                        marked_extension.nondeterministic_mark_transports,
                        marked_extension.noninjective_core_branches,
                        marked_extension.obstruction.c_str());
            const auto complete_tuple_extension =
                ravel::proof::derive_complete_atom_tuple_extension(
                    catalogue, component_graph.n,
                    [&](std::size_t local) {
                        std::vector<std::size_t> destinations;
                        for (const auto& [destination, weight] : component_graph.out_adj[local])
                            for (long long copy = 0; copy < weight; ++copy)
                                destinations.push_back(destination);
                        return destinations;
                    },
                    [&](std::size_t local) {
                        return node_vector[cyclic[k][local]].pair;
                    },
                    [&](std::size_t local) {
                        return node_vector[cyclic[k][local]].x;
                    });
            std::printf("  complete atom tuple: %s decomps=%zu marks=%zu live=%zu maxDecomp=%zu atomImage=%zu/%zu recurrent=%zu/%zu outsideCore=%zu sourceAmb=%zu atomAmb=%zu reason=%s\n",
                        complete_tuple_extension.spectral_radius_at_most_core ? "PASS" : "FAIL",
                        complete_tuple_extension.decomposition_states,
                        complete_tuple_extension.marked_states,
                        complete_tuple_extension.live_marked_states,
                        complete_tuple_extension.maximum_decompositions,
                        complete_tuple_extension.image_atom_vertices,
                        complete_tuple_extension.image_atom_edges,
                        complete_tuple_extension.recurrent_image_components,
                        complete_tuple_extension.recurrent_image_vertices,
                        complete_tuple_extension.recurrent_image_vertices_outside_core,
                        complete_tuple_extension.entropy.to_source.branching_recurrent_components,
                        complete_tuple_extension.entropy.to_target.branching_recurrent_components,
                        complete_tuple_extension.obstruction.c_str());
            for (const auto& atom_component : complete_tuple_extension.recurrent_image_summaries)
                std::printf("    atom image SCC nodes=%zu edges=%zu core=%zu permutation=%s rho~=%.12Lf\n",
                            atom_component.vertices, atom_component.edges,
                            atom_component.core_vertices,
                            atom_component.permutation ? "YES" : "NO",
                            atom_component.spectral_radius_estimate);
            const auto gauged_tuple =
                ravel::proof::derive_gauged_atom_tuple_core_correspondence(
                    catalogue, component_graph.n,
                    [&](std::size_t local) { std::vector<std::size_t> d; for (const auto& [v,w] : component_graph.out_adj[local]) for(long long q=0;q<w;++q)d.push_back(v); return d; },
                    [&](std::size_t local) { return node_vector[cyclic[k][local]].pair; },
                    [&](std::size_t local) { return node_vector[cyclic[k][local]].x; });
            std::printf("    gauged tuple/core: %s tupleMarks=%zu gauges=%zu live=%zu coreAmb=%zu uncovered=%zu reason=%s\n",
                        gauged_tuple.spectral_radius_at_most_core ? "PASS" : "FAIL",
                        gauged_tuple.tuple_mark_states,
                        gauged_tuple.gauged_states,
                        gauged_tuple.live_gauged_states,
                        gauged_tuple.to_core.branching_recurrent_components,
                        gauged_tuple.uncovered_source_vertices,
                        gauged_tuple.obstruction.c_str());
            std::map<ravel::proof::FaceRelativeSignGrammar, std::size_t> sign_role_ids;
            std::vector<std::size_t> competitor_sign_role(component_graph.n);
            for (std::size_t local = 0; local < component_graph.n; ++local) {
                const auto& node = node_vector[cyclic[k][local]];
                const auto role = ravel::proof::derive_face_relative_sign_grammar(
                    node.x, node.pair.i, node.pair.j, true);
                competitor_sign_role[local] = sign_role_ids.emplace(
                    role, sign_role_ids.size()).first->second;
            }
            std::vector<std::size_t> core_sign_role(core_vertices.size());
            for (std::size_t local = 0; local < core_vertices.size(); ++local) {
                const auto& node = node_vector[core_vertices[local]];
                const auto role = ravel::proof::derive_face_relative_sign_grammar(
                    node.x, node.pair.i, node.pair.j, true);
                core_sign_role[local] = sign_role_ids.emplace(
                    role, sign_role_ids.size()).first->second;
            }
            const auto sign_bellman = ravel::proof::derive_phase_handoff_bellman_renewal(
                structural_dense(component_graph), competitor_sign_role,
                structural_dense(structural_core_graph), core_sign_role,
                sign_role_ids.size(), 256);
            std::printf("    face-sign Bellman: %s roles=%zu block=%zu reason=%s\n",
                        sign_bellman.proved ? "PASS" : "FAIL",
                        sign_role_ids.size(), sign_bellman.block_length,
                        sign_bellman.obstruction.c_str());
            for (const std::size_t role_cap : {2UL, 3UL, 4UL, 6UL, 8UL}) {
                std::map<ravel::proof::ParametricFaceSignRole, std::size_t> param_role_ids;
                std::vector<std::size_t> competitor_param_role(component_graph.n);
                for (std::size_t local = 0; local < component_graph.n; ++local) {
                    const auto& node = node_vector[cyclic[k][local]];
                    const auto role = ravel::proof::derive_parametric_face_sign_role(
                        node.x, node.pair.i, node.pair.j, role_cap);
                    competitor_param_role[local] = param_role_ids.emplace(
                        role, param_role_ids.size()).first->second;
                }
                std::vector<std::size_t> core_param_role(core_vertices.size());
                for (std::size_t local = 0; local < core_vertices.size(); ++local) {
                    const auto& node = node_vector[core_vertices[local]];
                    const auto role = ravel::proof::derive_parametric_face_sign_role(
                        node.x, node.pair.i, node.pair.j, role_cap);
                    core_param_role[local] = param_role_ids.emplace(
                        role, param_role_ids.size()).first->second;
                }
                const auto param_bellman = ravel::proof::derive_phase_handoff_bellman_renewal(
                    structural_dense(component_graph), competitor_param_role,
                    structural_dense(structural_core_graph), core_param_role,
                    param_role_ids.size(), 256);
                std::printf("    param-sign Bellman cap=%zu: %s roles=%zu block=%zu reason=%s\n",
                            role_cap, param_bellman.proved ? "PASS" : "FAIL",
                            param_role_ids.size(), param_bellman.block_length,
                            param_bellman.obstruction.c_str());
            }
            const auto atom_intertwiner =
                ravel::proof::derive_atom_incidence_intertwiner_audit(
                    catalogue, component_graph.n,
                    [&](std::size_t local) { std::vector<std::size_t> d; for (const auto& [v,w] : component_graph.out_adj[local]) for(long long q=0;q<w;++q)d.push_back(v); return d; },
                    [&](std::size_t local) { return node_vector[cyclic[k][local]].pair; },
                    [&](std::size_t local) { return node_vector[cyclic[k][local]].x; });
            std::printf("    atom incidence AP<=PB=%s violations=%zu max=%lld AP>=PB=%s violations=%zu max=%lld nnz=%zu\n",
                        atom_intertwiner.subintertwiner ? "PASS" : "FAIL",
                        atom_intertwiner.ap_le_pb_violations,
                        atom_intertwiner.maximum_le_defect,
                        atom_intertwiner.superintertwiner ? "PASS" : "FAIL",
                        atom_intertwiner.ap_ge_pb_violations,
                        atom_intertwiner.maximum_ge_defect,
                        atom_intertwiner.nonzero_entries);
            for (const auto& violation : atom_intertwiner.sample_violations)
                std::printf("      %s\n", violation.c_str());
            const auto marked_correspondence =
                ravel::proof::derive_marked_decomposition_correspondence(
                    catalogue, component_graph.n,
                    [&](std::size_t local) {
                        std::vector<std::size_t> destinations;
                        for (const auto& [destination, weight] : component_graph.out_adj[local])
                            for (long long copy = 0; copy < weight; ++copy)
                                destinations.push_back(destination);
                        return destinations;
                    },
                    [&](std::size_t local) { return node_vector[cyclic[k][local]].pair; },
                    [&](std::size_t local) { return node_vector[cyclic[k][local]].x; });
            std::printf("  marked correspondence: %s live=%zu sourceAmb=%zu coreAmb=%zu reason=%s\n",
                        marked_correspondence.spectral_radius_at_most_core ? "PASS" : "FAIL",
                        marked_correspondence.live_marked_vertices,
                        marked_correspondence.entropy.to_source.branching_recurrent_components,
                        marked_correspondence.entropy.to_target.branching_recurrent_components,
                        marked_correspondence.obstruction.c_str());
            std::vector<std::vector<ravel::proof::ColoredEdgeOccurrence>> component_colored(component_graph.n);
            for (std::size_t local=0; local<component_graph.n; ++local) {
                const auto pair = node_vector[cyclic[k][local]].pair;
                const auto color = static_cast<std::size_t>(((pair.j > 0) ? 1 : 0) != ((pair.i > 0) ? 1 : 0));
                for (const auto& [destination,weight] : component_graph.out_adj[local])
                    for (long long copy=0; copy<weight; ++copy) component_colored[local].push_back({destination,color});
            }
            const auto zero_sync = ravel::proof::derive_colored_synchronized_correspondence(component_colored,core_colored);
            std::printf("  Q/R zero synchronization: %s live=%zu sourceAmb=%zu coreAmb=%zu reason=%s\n",
                        zero_sync.spectral_radius_at_most_target ? "PASS" : "FAIL",
                        zero_sync.live_vertices,
                        zero_sync.entropy.to_source.branching_recurrent_components,
                        zero_sync.entropy.to_target.branching_recurrent_components,
                        zero_sync.obstruction.c_str());
            std::size_t minimum_grade = 999;
            for (std::size_t local=0; local<component_graph.n; ++local)
                minimum_grade = std::min(minimum_grade, catalogue.grade(node_vector[cyclic[k][local]].x));
            const auto induced_injection = ravel::proof::derive_induced_path_injective_simulation(
                component_graph.n, core_vertices.size(),
                [&](std::size_t local) {
                    std::vector<std::size_t> d; for (const auto& [v,w] : component_graph.out_adj[local])
                        for (long long q=0;q<w;++q)d.push_back(v); return d; },
                [&](std::size_t local) {
                    std::vector<std::size_t> d; const auto global=core_vertices[local];
                    for (const auto& [v,w] : graph.out_adj[global]) { auto it=core_local.find(v); if(it==core_local.end())continue;
                        for(long long q=0;q<w;++q)d.push_back(it->second); } return d; },
                [&](std::size_t local) { return catalogue.grade(node_vector[cyclic[k][local]].x)==minimum_grade; },
                [&](std::size_t, std::size_t) { return true; }, 16, 200000);
            std::printf("  induced core injection: %s section=%zu words=%zu maxlen=%zu reason=%s\n",
                        induced_injection.spectral_radius_nonstrict ? "PASS" : "FAIL",
                        induced_injection.section_vertices, induced_injection.return_words,
                        induced_injection.maximum_return_length, induced_injection.obstruction.c_str());
            using PairLabel = std::pair<std::size_t,std::size_t>;
            std::vector<std::vector<ravel::proof::LabeledEdge<PairLabel>>> component_labeled(component_graph.n), core_labeled(core_vertices.size());
            for (std::size_t local=0; local<component_graph.n; ++local)
                for (const auto& [v,w] : component_graph.out_adj[local])
                    for(long long q=0;q<w;++q) { const auto p=node_vector[cyclic[k][v]].pair; component_labeled[local].push_back({v,{p.i,p.j}}); }
            for (std::size_t local=0; local<core_vertices.size(); ++local) {
                const auto global=core_vertices[local];
                for(const auto&[v,w]:graph.out_adj[global]) { auto it=core_local.find(v); if(it==core_local.end())continue;
                    for(long long q=0;q<w;++q) { const auto p=node_vector[v].pair; core_labeled[local].push_back({it->second,{p.i,p.j}}); }
                }
            }
            const auto language = ravel::proof::derive_right_resolving_language_inclusion(component_labeled,core_labeled);
            std::printf("  core follower-language inclusion: %s sourceRR=%d coreRR=%d pairs=%zu reason=%s\n",
                        language.spectral_radius_nonstrict ? "PASS" : "FAIL",
                        language.source_right_resolving, language.target_right_resolving,
                        language.surviving_pairs, language.obstruction.c_str());
            const auto global_language = ravel::proof::derive_right_resolving_global_language_inclusion(component_labeled,core_labeled);
            std::printf("  global core language inclusion: %s states=%zu maxSubset=%zu reason=%s\n",
                        global_language.spectral_radius_nonstrict ? "PASS" : "FAIL",
                        global_language.reachable_follower_states,
                        global_language.maximum_target_subset,
                        global_language.obstruction.c_str());
            const auto phase_reduction = ravel::proof::derive_phase_aware_grade_reduction(
                component_graph.n,
                [&](std::size_t local) {
                    std::vector<std::size_t> destinations;
                    for (const auto& [destination, weight] : component_graph.out_adj[local]) {
                        (void)weight;
                        destinations.push_back(destination);
                    }
                    return destinations;
                },
                [&](std::size_t local) {
                    const auto vertex = cyclic[k][local];
                    const auto grade = catalogue.grade(node_vector[vertex].x);
                    return grade <= 1 || !catalogue.derive_one_stage(
                        node_vector[vertex].pair, node_vector[vertex].x).empty();
                });
            const auto relational_simulation =
                ravel::proof::derive_relational_grade_simulation(
                    component_graph.n, core_vertices.size(),
                    [&](std::size_t local) {
                        std::vector<std::size_t> destinations;
                        for (const auto& [destination, weight] : component_graph.out_adj[local]) {
                            (void)weight;
                            destinations.push_back(destination);
                        }
                        return destinations;
                    },
                    [&](std::size_t core) {
                        std::vector<std::size_t> destinations;
                        for (const auto& [destination, weight] : graph.out_adj[core_vertices[core]]) {
                            (void)weight;
                            const auto found = core_local.find(destination);
                            if (found != core_local.end()) destinations.push_back(found->second);
                        }
                        return destinations;
                    },
                    [&](std::size_t shell, std::size_t core) {
                        const auto shell_vertex = cyclic[k][shell];
                        const auto shell_grade = catalogue.grade(node_vector[shell_vertex].x);
                        if (shell_grade <= 1) return false;
                        std::vector<long long> remainder(n, 0);
                        for (std::size_t coordinate = 0; coordinate < n; ++coordinate)
                            remainder[coordinate] = node_vector[shell_vertex].x[coordinate] -
                                node_vector[core_vertices[core]].x[coordinate];
                        return catalogue.grade(remainder) + 1 == shell_grade;
                    });
            const auto recursive_simulation =
                ravel::proof::derive_relational_grade_simulation(
                    component_graph.n, cyclic_vertices.size(),
                    [&](std::size_t local) {
                        std::vector<std::size_t> destinations;
                        for (const auto& [destination, weight] : component_graph.out_adj[local]) {
                            (void)weight;
                            destinations.push_back(destination);
                        }
                        return destinations;
                    },
                    [&](std::size_t target) {
                        std::vector<std::size_t> destinations;
                        for (const auto& [destination, weight] : graph.out_adj[cyclic_vertices[target]]) {
                            (void)weight;
                            const auto found = cyclic_local.find(destination);
                            if (found != cyclic_local.end()) destinations.push_back(found->second);
                        }
                        return destinations;
                    },
                    [&](std::size_t shell, std::size_t target) {
                        const auto shell_vertex = cyclic[k][shell];
                        const auto target_vertex = cyclic_vertices[target];
                        const auto shell_grade = catalogue.grade(node_vector[shell_vertex].x);
                        const auto target_grade = catalogue.grade(node_vector[target_vertex].x);
                        if (shell_grade <= 1 || target_grade == 0 || target_grade >= shell_grade)
                            return false;
                        std::vector<long long> difference(n, 0);
                        for (std::size_t coordinate = 0; coordinate < n; ++coordinate)
                            difference[coordinate] = node_vector[shell_vertex].x[coordinate] -
                                node_vector[target_vertex].x[coordinate];
                        const auto difference_grade = catalogue.grade(difference);
                        return difference_grade > 0 &&
                            difference_grade + target_grade == shell_grade;
                    });
            ravel::proof::RelationalGradeSimulation delayed_simulation;
            if (!recursive_simulation.complete) delayed_simulation =
                ravel::proof::derive_bounded_delay_grade_simulation(
                    component_graph.n, cyclic_vertices.size(), 2,
                    [&](std::size_t local) {
                        std::vector<std::size_t> destinations;
                        for (const auto& [destination, weight] : component_graph.out_adj[local]) {
                            (void)weight; destinations.push_back(destination);
                        }
                        return destinations;
                    },
                    [&](std::size_t target) {
                        std::vector<std::size_t> destinations;
                        for (const auto& [destination, weight] : graph.out_adj[cyclic_vertices[target]]) {
                            (void)weight;
                            const auto found = cyclic_local.find(destination);
                            if (found != cyclic_local.end()) destinations.push_back(found->second);
                        }
                        return destinations;
                    },
                    [&](std::size_t shell, std::size_t target) {
                        const auto shell_vertex = cyclic[k][shell];
                        const auto target_vertex = cyclic_vertices[target];
                        const auto shell_grade = catalogue.grade(node_vector[shell_vertex].x);
                        const auto target_grade = catalogue.grade(node_vector[target_vertex].x);
                        if (shell_grade <= 1 || target_grade == 0 || target_grade >= shell_grade)
                            return false;
                        std::vector<long long> difference(n, 0);
                        for (std::size_t coordinate = 0; coordinate < n; ++coordinate)
                            difference[coordinate] = node_vector[shell_vertex].x[coordinate] -
                                node_vector[target_vertex].x[coordinate];
                        const auto difference_grade = catalogue.grade(difference);
                        return difference_grade > 0 &&
                            difference_grade + target_grade == shell_grade;
                    });
            ravel::proof::RenewalGradeSimulation renewal_simulation;
            if (grades.size() > 1 && grades.begin()->first > 1) {
                const auto renewal_grade = grades.begin()->first;
                renewal_simulation = ravel::proof::derive_renewal_grade_simulation(
                    component_graph.n, core_vertices.size(),
                    [&](std::size_t local) {
                        std::vector<std::size_t> destinations;
                        for (const auto& [destination, weight] : component_graph.out_adj[local]) {
                            (void)weight; destinations.push_back(destination);
                        }
                        return destinations;
                    },
                    [&](std::size_t core) {
                        std::vector<std::size_t> destinations;
                        for (const auto& [destination, weight] : graph.out_adj[core_vertices[core]]) {
                            (void)weight;
                            const auto found = core_local.find(destination);
                            if (found != core_local.end()) destinations.push_back(found->second);
                        }
                        return destinations;
                    },
                    [&](std::size_t local) {
                        return catalogue.grade(node_vector[cyclic[k][local]].x) == renewal_grade;
                    },
                    [&](std::size_t local, std::size_t core) {
                        const auto shell_vertex = cyclic[k][local];
                        std::vector<long long> remainder(n, 0);
                        for (std::size_t coordinate = 0; coordinate < n; ++coordinate)
                            remainder[coordinate] = node_vector[shell_vertex].x[coordinate] -
                                node_vector[core_vertices[core]].x[coordinate];
                        return catalogue.grade(remainder) + 1 == renewal_grade;
                    }, 64);
            }
            const auto transition_descent =
                ravel::proof::derive_transition_compatible_grade_descent(
                    catalogue, component_graph.n,
                    [&](std::size_t local) {
                        std::vector<std::size_t> destinations;
                        for (const auto& [destination, weight] : component_graph.out_adj[local]) {
                            (void)weight;
                            destinations.push_back(destination);
                        }
                        return destinations;
                    },
                    [&](std::size_t local) { return node_vector[cyclic[k][local]].pair; },
                    [&](std::size_t local) { return node_vector[cyclic[k][local]].x; });
            std::printf(" phaseClasses=%zu covered=%s obstructions=%zu "
                        "phaseReduction=%s maxPhaseHops=%zu "
                        "decompReplay=%s compatibleEdges=%zu "
                        "existential=%zu/%zu(%s) universal=%zu/%zu(%s) "
                        "relSimulation=%zu/%zu pairs=%zu/%zu(%s) "
                        "recursiveSimulation=%zu/%zu pairs=%zu/%zu(%s) "
                        "delay2Simulation=%zu/%zu pairs=%zu/%zu(%s) "
                        "renewal=%zu/%zu blocks=%zu maxBlock=%zu pairs=%zu/%zu acyclic=%s(%s)\n",
                        descent_summary.phase_cover.size(),
                        descent_summary.all_nonterminal_covered() ? "ALL" : "PARTIAL",
                        descent_summary.obstructions.size(),
                        phase_reduction.complete ? "PASS" : "FAIL",
                        phase_reduction.maximum_hops,
                        transition_descent.decomposition_law_replayed ? "PASS" : "FAIL",
                        transition_descent.compatible_edges,
                        transition_descent.vertices_with_existential_witness,
                        transition_descent.required_nonterminal_vertices,
                        transition_descent.existential_complete ? "PASS" : "PARTIAL",
                        transition_descent.vertices_with_universal_witness,
                        transition_descent.required_nonterminal_vertices,
                        transition_descent.universal_complete ? "PASS" : "PARTIAL",
                        relational_simulation.covered_shell_vertices,
                        relational_simulation.required_shell_vertices,
                        relational_simulation.surviving_pairs,
                        relational_simulation.initial_pairs,
                        relational_simulation.complete ? "PASS" : "PARTIAL",
                        recursive_simulation.covered_shell_vertices,
                        recursive_simulation.required_shell_vertices,
                        recursive_simulation.surviving_pairs,
                        recursive_simulation.initial_pairs,
                        recursive_simulation.complete ? "PASS" : "PARTIAL",
                        delayed_simulation.covered_shell_vertices,
                        delayed_simulation.required_shell_vertices,
                        delayed_simulation.surviving_pairs,
                        delayed_simulation.initial_pairs,
                        delayed_simulation.complete ? "PASS" : "PARTIAL",
                        renewal_simulation.covered_renewal_vertices,
                        renewal_simulation.renewal_vertices,
                        renewal_simulation.renewal_blocks,
                        renewal_simulation.maximum_block_length,
                        renewal_simulation.surviving_pairs,
                        renewal_simulation.initial_pairs,
                        renewal_simulation.higher_stratum_acyclic ? "YES" : "NO",
                        renewal_simulation.complete ? "PASS" : "PARTIAL");
            for (const auto& obstruction : descent_summary.obstructions) {
                std::printf("  GOBSTRUCT grade=%zu pair=(%zu,%zu) reason=%s x=",
                            obstruction.grade, obstruction.pair.i,
                            obstruction.pair.j, obstruction.reason.c_str());
                for (const auto coefficient : obstruction.displacement)
                    std::printf("%lld,", coefficient);
                std::printf("\n");
            }
        }
        std::printf("arithmetic structural grade ledger: PASS\n");
        return true;
    }

    const auto core_graph = induced_component(graph, cyclic[core_component]);
    const auto core_partition = coarsest_equitable_partition(core_graph);
    const auto core_quotient = quotient_matrix(core_graph, core_partition);
    const double core_rho = options.weights_only ? 0.0 : dominant_eigenvalue_estimate_sparse(core_graph, 4000);
    if (options.dump_quotients)
        dump_quotient(n, "core", core_component, core_quotient);
    bool dominant_estimate = true;
    bool exact_dominance = true;
    std::vector<std::vector<std::vector<long long>>> quotient_competitors;
    for (std::size_t k = 0; k < cyclic.size(); ++k) {
        const auto component_graph = induced_component(graph, cyclic[k]);
        const auto partition = coarsest_equitable_partition(component_graph);
        const auto quotient = quotient_matrix(component_graph, partition);
        if (k != core_component) quotient_competitors.push_back(quotient);
        if (options.dump_quotients && k != core_component)
            dump_quotient(n, "shell", k, quotient);
        const double rho = dominant_eigenvalue_estimate_sparse(component_graph, 4000);
        const auto reflective_dominance =
            ravel::proof::derive_reflective_perron_dominance(
                quotient, core_quotient, 80);
        const auto reflective_grammar =
            ravel::proof::derive_reflective_weight_grammar(quotient, 80);
        const auto paired_dominance =
            ravel::proof::derive_rank_one_paired_dominance(
                quotient, core_quotient, 80);
        const ravel::proof::DisplacementSumCatalogue weight_catalogue(n);
        std::vector<std::vector<long long>> weight_labels(quotient.size());
        std::vector<std::vector<std::size_t>> quotient_members(quotient.size());
        for (std::size_t local = 0; local < component_graph.n; ++local)
            quotient_members[static_cast<std::size_t>(partition.color[local])].push_back(local);
        for (std::size_t cell = 0; cell < quotient_members.size(); ++cell) {
            std::map<long long,long long> grade_hist, support_hist, phase_hist;
            long long positive = 0, negative = 0, zeros = 0, sign_changes = 0;
            for (const auto local : quotient_members[cell]) {
                const auto& node = node_vector[cyclic[k][local]];
                const auto grade = static_cast<long long>(weight_catalogue.grade(node.x));
                long long support_count = 0;
                for (std::size_t q = 0; q < node.x.size(); ++q) {
                    support_count += node.x[q] != 0;
                    positive += node.x[q] > 0;
                    negative += node.x[q] < 0;
                    zeros += node.x[q] == 0;
                    if (q + 1 < node.x.size())
                        sign_changes += (node.x[q] > 0) != (node.x[q+1] > 0);
                }
                ++grade_hist[grade];
                ++support_hist[support_count];
                const auto phase = static_cast<long long>((node.pair.j + n - node.pair.i) % n);
                ++phase_hist[phase];
            }
            auto& label = weight_labels[cell];
            label = {static_cast<long long>(quotient_members[cell].size()), positive, negative, zeros, sign_changes};
            for (const auto& [v,c] : grade_hist) { label.push_back(1000+v); label.push_back(c); }
            label.push_back(-1001);
            for (const auto& [v,c] : support_hist) { label.push_back(2000+v); label.push_back(c); }
            label.push_back(-2001);
            for (const auto& [v,c] : phase_hist) { label.push_back(3000+v); label.push_back(c); }
        }
        const auto symbolic_weight =
            ravel::proof::derive_symbolic_weight_factorization(
                weight_labels, reflective_dominance.competitor.weight);
        const auto refined_symbolic_weight =
            ravel::proof::derive_refined_symbolic_weight_grammar(
                quotient, weight_labels, reflective_dominance.competitor.weight);
        const auto symbolic_role_dominance =
            ravel::proof::derive_symbolic_role_dominance(
                quotient, weight_labels, core_quotient, 80);
        if (options.weights_only) {
            const auto& w = reflective_dominance.competitor.weight;
            std::set<std::string> distinct;
            for (const auto& value : w) distinct.insert(mathlib::str(value));
            std::printf("WSCC %zu%s nodes=%zu edges=%zu q=%zu period=%zu "
                        "weightDistinct=%zu bracket=[%s,%s] strict=%s replay=%s paired=%s Pentries=%zu\n",
                        k, k == core_component ? " [CORE]" : "",
                        component_graph.n, edge_sum(component_graph), quotient.size(),
                        graph_period(component_graph), distinct.size(),
                        mathlib::str(reflective_dominance.competitor.bracket.lo).c_str(),
                        mathlib::str(reflective_dominance.competitor.bracket.hi).c_str(),
                        (k == core_component || reflective_dominance.strict) ? "PASS" : "FAIL",
                        reflective_dominance.replayed ? "PASS" : "FAIL",
                        (k == core_component || paired_dominance.replayed) ? "PASS" : "FAIL",
                        paired_dominance.implicit_entries);
            if (options.dump_weights) {
                const auto path = std::string("out/weight_n") + std::to_string(n) +
                    "_scc" + std::to_string(k) + ".tsv";
                if (auto* file = std::fopen(path.c_str(), "w")) {
                    for (std::size_t i = 0; i < w.size(); ++i)
                        std::fprintf(file, "%zu\t%s\n", i, mathlib::str(w[i]).c_str());
                    std::fclose(file);
                }
            }
            continue;
        }
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
                    "gap~=%.12f maxcoeff=%lld reflectiveWeight=%s replay=%s "
                    "weightGrammar=%zu/%zu grammarReplay=%s "
                    "symbolicWeight=%zu/%zu factor=%s collisions=%zu "
                    "refinedWeight=%zu/%zu rounds=%zu factor=%s "
                    "roleUpper=%zu/%zu dominance=%s replay=%s support=",
                    k, k == core_component ? " [CORE]" : "",
                    component_graph.n, edge_sum(component_graph), quotient.size(),
                    graph_period(component_graph),
                    rho, envelope.n, envelope_rho,
                    core_rho - rho, max_coefficient,
                    (k == core_component || reflective_dominance.strict) ? "PASS" : "FAIL",
                    reflective_dominance.replayed ? "PASS" : "FAIL",
                    reflective_grammar.roles(), quotient.size(),
                    reflective_grammar.lifted_replay ? "PASS" : "FAIL",
                    symbolic_weight.roles(), quotient.size(),
                    symbolic_weight.weight_constant_on_roles ? "PASS" : "FAIL",
                    symbolic_weight.collisions,
                    refined_symbolic_weight.roles(), quotient.size(),
                    refined_symbolic_weight.refinement_rounds,
                    refined_symbolic_weight.weight_constant_on_roles ? "PASS" : "FAIL",
                    symbolic_role_dominance.competitor.roles.roles(), quotient.size(),
                    (k == core_component || symbolic_role_dominance.strict) ? "PASS" : "FAIL",
                    symbolic_role_dominance.replayed ? "PASS" : "FAIL");
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
    const auto quotient_maximality =
        ravel::proof::derive_finite_quotient_core_maximality(
            core_quotient, quotient_competitors,
            static_cast<std::size_t>(std::max(80, options.exact_iterations)));
    std::printf("finite quotient core maximality: %s terminal=%zu injection=%zu renewal=%zu paired=%zu\n",
                quotient_maximality.core_spectrally_maximal ? "PASS" : "FAIL",
                quotient_maximality.terminal_components,
                quotient_maximality.path_injection_components,
                quotient_maximality.renewal_components,
                quotient_maximality.paired_matrix_components);
    for (const auto& evidence : quotient_maximality.evidence) {
        if (evidence.mechanism == ravel::proof::QuotientDominanceMechanism::RenewalTransfer)
            std::printf("  quotient renewal component=%zu vertices=%zu block=%zu cone=%zu\n",
                        evidence.index, evidence.vertices,
                        evidence.renewal_transfer.renewal.block_length,
                        evidence.renewal_transfer.cone_pair.first_separating_horizon);
        if (evidence.mechanism == ravel::proof::QuotientDominanceMechanism::HigherBlockPathInjection)
            std::printf("  quotient higher-block component=%zu depth=%zu vertices=%zu\n",
                        evidence.index, evidence.higher_block_injection.depth,
                        evidence.higher_block_injection.presentation_vertices);
        if (evidence.mechanism == ravel::proof::QuotientDominanceMechanism::Unproved)
            std::printf("  quotient obstruction component=%zu vertices=%zu reason=%s\n",
                        evidence.index, evidence.vertices, evidence.obstruction.c_str());
    }
    std::printf("arithmetic-supergraph dominance: estimate=%s exact=%s catalogue=%s\n",
                dominant_estimate ? "PASS" : "FAIL",
                options.exact ? (exact_dominance ? "PASS" : "FAIL/INCONCLUSIVE")
                              : "not-requested",
                quotient_maximality.core_spectrally_maximal ? "PASS" : "FAIL");
    return quotient_maximality.core_spectrally_maximal &&
           dominant_estimate && (!options.exact || exact_dominance);
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
        else if (value == "--dump-core-exhaustion")
            options.dump_core_exhaustion = true;
        else if (value == "--structural-only")
            options.structural_only = true;
        else if (value == "--weights-only")
            options.weights_only = true;
        else if (value == "--dump-weights")
            options.dump_weights = true;
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
