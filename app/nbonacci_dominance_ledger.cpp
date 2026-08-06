// nbonacci_dominance_ledger.cpp
//
// Non-circular finite audit for the open arbitrary-n dominance seam.
// Unlike extract_dominant_recurrent_core, this driver never uses a Perron
// estimate to identify the candidate component.  It constructs the full
// corona graph G_B, selects nodes solely with the integral predicate in
// nbonacci_margin_invariant.hpp, and requires that those nodes form exactly
// one cyclic SCC with exactly the generated internal edge relation.  Only
// after that structural identification does it compare the candidate's
// Perron root with every competing cyclic SCC.
//
// This is theorem-discovery machinery, not an all-n theorem: corona
// occurrence and recurrent-SCC exhaustion are still checked at each supplied
// dimension.  The stable, normalized component ledger is intended to expose
// the shell families or an intertwining simulation that can replace that
// finite enumeration.

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <map>
#include <numeric>
#include <set>
#include <string>
#include <tuple>
#include <unordered_map>
#include <utility>
#include <vector>

#include "math/ball.hpp"
#include "ravel/ambient_graph.hpp"
#include "ravel/contact_boundary.hpp"
#include "ravel/core.hpp"
#include "ravel/corona.hpp"
#include "ravel/d_cont_check.hpp"
#include "ravel/faces.hpp"
#include "ravel/graph_divisor.hpp"
#include "ravel/involution_helpers.hpp"
#include "ravel/nbonacci_margin_invariant.hpp"
#include "ravel/proof/paired_matrix_dominance.hpp"
#include "ravel/proof/path_count_cone.hpp"
#include "ravel/proof/graded_transfer.hpp"
#include "ravel/proof/phase_handoff_renewal.hpp"
#include "ravel/proof/face_relative_sign_grammar.hpp"
#include "ravel/proof/signed_interval_bellman.hpp"
#include "ravel/proof/twisted_quotient.hpp"
#include "ravel/substitution.hpp"

using namespace ravel;

namespace {

struct Options {
    bool exact = false;
    bool dump_states = false;
    bool structural_only = false;
    bool paired_only = false;
    bool diagnose_run_merges = false;
    bool run_truth_only = false;
    bool twist_test = false;
    int exact_iterations = 160;
};

struct DisplacementSumCatalogue {
    std::vector<std::vector<long long>> atoms;
    std::vector<std::pair<nbonacci_margin::DisplacementDescriptor,
                          std::vector<long long>>> described_atoms;
    std::set<std::vector<long long>> atom_set;
    std::set<std::vector<long long>> two_sums;

    explicit DisplacementSumCatalogue(std::size_t n) {
        for (const auto descriptor :
             nbonacci_margin::displacement_descriptors(n))
            described_atoms.push_back({descriptor,
                nbonacci_margin::displacement_from_descriptor(n, descriptor)});
        const auto catalogue = nbonacci_margin::displacement_catalogue(n);
        atoms.assign(catalogue.begin(), catalogue.end());
        atom_set = catalogue;
        for (std::size_t lhs = 0; lhs < atoms.size(); ++lhs)
            for (std::size_t rhs = lhs; rhs < atoms.size(); ++rhs) {
                std::vector<long long> sum(n, 0);
                for (std::size_t coordinate = 0; coordinate < n; ++coordinate)
                    sum[coordinate] =
                        atoms[lhs][coordinate] + atoms[rhs][coordinate];
                two_sums.insert(std::move(sum));
            }
    }

    // The first three layers suffice through the current compiled range and
    // are enough to detect the predicted new layer at n=7.  Zero means that
    // the vector is not a sum of at most three grade-1 catalogue vectors.
    std::size_t grade(const std::vector<long long>& x) const {
        if (atom_set.count(x)) return 1;
        if (two_sums.count(x)) return 2;
        for (const auto& atom : atoms) {
            std::vector<long long> remainder(x.size(), 0);
            for (std::size_t coordinate = 0; coordinate < x.size(); ++coordinate)
                remainder[coordinate] = x[coordinate] - atom[coordinate];
            if (two_sums.count(remainder)) return 3;
        }
        return 0;
    }

    std::size_t grade_two_core_projections(
        std::size_t n, nbonacci_margin::LabelPair pair,
        const std::vector<long long>& x) const {
        std::size_t result = 0;
        for (const auto& [descriptor, active] : described_atoms) {
            if (!nbonacci_margin::predicted_core_member(n, pair, descriptor))
                continue;
            std::vector<long long> passive(n, 0);
            for (std::size_t coordinate = 0; coordinate < n; ++coordinate)
                passive[coordinate] = x[coordinate] - active[coordinate];
            if (atom_set.count(passive)) ++result;
        }
        return result;
    }
};

template <std::size_t D>
std::vector<long long> as_vector(const IVec<D>& x) {
    return std::vector<long long>(x.begin(), x.end());
}

template <std::size_t D>
SNode<D> make_node(std::size_t i, const std::vector<long long>& x,
                   std::size_t j) {
    if (x.size() != D) throw std::invalid_argument("make_node: wrong dimension");
    SNode<D> node;
    node.i = static_cast<long long>(i);
    node.j = static_cast<long long>(j);
    std::copy(x.begin(), x.end(), node.x.begin());
    return node;
}

WeightedDigraph induced_component(
    const WeightedDigraph& graph, const std::vector<std::size_t>& vertices) {
    std::unordered_map<std::size_t, std::size_t> local;
    for (std::size_t k = 0; k < vertices.size(); ++k) local[vertices[k]] = k;
    WeightedDigraph result(vertices.size());
    for (std::size_t k = 0; k < vertices.size(); ++k) {
        for (const auto& [destination, weight] : graph.out_adj[vertices[k]]) {
            const auto found = local.find(destination);
            if (found != local.end()) result.add_edge(k, found->second, weight);
        }
    }
    return result;
}



std::vector<std::vector<long long>> dense_matrix(const WeightedDigraph& graph) {
    std::vector<std::vector<long long>> result(
        graph.n, std::vector<long long>(graph.n, 0));
    for (std::size_t source = 0; source < graph.n; ++source)
        for (const auto& [target, weight] : graph.out_adj[source])
            result[source][target] += weight;
    return result;
}

template <std::size_t D> struct FullGraph;

template <std::size_t D>
std::size_t node_support(const SNode<D>& node) {
    std::size_t result = 0;
    for (const auto x : node.x) result += x != 0 ? 1 : 0;
    return result;
}

template <std::size_t D>
std::uint64_t node_sign_code(const SNode<D>& node) {
    std::uint64_t result = 0;
    for (const auto x : node.x) {
        result *= 3;
        result += x < 0 ? 0 : (x == 0 ? 1 : 2);
    }
    return result;
}

template <std::size_t D>
std::pair<std::vector<std::size_t>, std::vector<std::size_t>>
role_labels(const FullGraph<D>& full,
            const std::vector<std::size_t>& competitor_vertices,
            const std::vector<std::size_t>& core_vertices,
            bool with_support, bool with_sign,
            std::size_t& role_count) {
    using Key = std::tuple<long long,long long,std::size_t,std::uint64_t>;
    std::map<Key,std::size_t> ids;
    auto label = [&](std::size_t vertex) {
        const auto& node = full.nodes[vertex];
        const Key key{node.i, node.j,
            with_support ? node_support(node) : 0,
            with_sign ? node_sign_code(node) : 0};
        const auto [it, inserted] = ids.emplace(key, ids.size());
        (void)inserted;
        return it->second;
    };
    std::vector<std::size_t> competitor, core;
    competitor.reserve(competitor_vertices.size());
    core.reserve(core_vertices.size());
    for (const auto v : competitor_vertices) competitor.push_back(label(v));
    for (const auto v : core_vertices) core.push_back(label(v));
    role_count = ids.size();
    return {std::move(competitor), std::move(core)};
}

template <std::size_t D>
std::pair<std::vector<std::size_t>, std::vector<std::size_t>>
face_relative_run_role_labels(
            const FullGraph<D>& full,
            const std::vector<std::size_t>& competitor_vertices,
            const std::vector<std::size_t>& core_vertices,
            bool canonicalize_sign,
            std::size_t& role_count) {
    using Key = ravel::proof::FaceRelativeSignGrammar;
    std::map<Key,std::size_t> ids;
    auto label = [&](std::size_t vertex) {
        const auto& node = full.nodes[vertex];
        std::vector<long long> coordinates(node.x.begin(), node.x.end());
        auto key = ravel::proof::derive_face_relative_sign_grammar(
            coordinates, static_cast<std::size_t>(node.i),
            static_cast<std::size_t>(node.j), canonicalize_sign);
        if (!key.replayed)
            throw std::runtime_error("face-relative run grammar replay failed");
        const auto [it, inserted] = ids.emplace(std::move(key), ids.size());
        (void)inserted;
        return it->second;
    };
    std::vector<std::size_t> competitor, core;
    competitor.reserve(competitor_vertices.size());
    core.reserve(core_vertices.size());
    for (const auto v : competitor_vertices) competitor.push_back(label(v));
    for (const auto v : core_vertices) core.push_back(label(v));
    role_count = ids.size();
    return {std::move(competitor), std::move(core)};
}

template <std::size_t D>
void diagnose_first_run_merge(
            const FullGraph<D>& full,
            const std::vector<std::size_t>& competitor_vertices,
            const std::vector<std::size_t>& core_vertices) {
    using FullKey = std::tuple<long long,long long,std::size_t,std::uint64_t>;
    using RunKey = ravel::proof::FaceRelativeSignGrammar;
    struct Witness {
        FullKey full_key;
        std::size_t vertex = 0;
        bool competitor = false;
    };
    std::map<RunKey, std::vector<Witness>> buckets;
    auto add = [&](std::size_t vertex, bool competitor) {
        const auto& node = full.nodes[vertex];
        std::vector<long long> coordinates(node.x.begin(), node.x.end());
        auto run = ravel::proof::derive_face_relative_sign_grammar(
            coordinates, static_cast<std::size_t>(node.i),
            static_cast<std::size_t>(node.j), false);
        const FullKey full_key{node.i, node.j, node_support(node), node_sign_code(node)};
        buckets[std::move(run)].push_back({full_key, vertex, competitor});
    };
    for (const auto v : competitor_vertices) add(v, true);
    for (const auto v : core_vertices) add(v, false);
    for (const auto& [run, witnesses] : buckets) {
        std::map<FullKey, Witness> distinct;
        for (const auto& witness : witnesses)
            distinct.emplace(witness.full_key, witness);
        if (distinct.size() <= 1) continue;
        std::printf("  RUN_FALSE_MERGE dimension=%zu active=%zu target_offset=%zu support=%zu runs=",
                    run.dimension, run.active_face, run.target_face_offset, run.support);
        for (const auto& r : run.runs)
            std::printf("%s%d:%zu", r == run.runs.front() ? "" : ",", r.sign, r.length);
        std::printf(" distinct_full_roles=%zu\n", distinct.size());
        std::size_t shown = 0;
        for (const auto& [key, witness] : distinct) {
            const auto& node = full.nodes[witness.vertex];
            const auto& [i,j,support,sign_code] = key;
            std::printf("    witness side=%s i=%lld j=%lld support=%zu sign_code=%llu x=",
                        witness.competitor ? "competitor" : "core",
                        i, j, support,
                        static_cast<unsigned long long>(sign_code));
            for (std::size_t c = 0; c < D; ++c)
                std::printf("%s%lld", c == 0 ? "" : ",", node.x[c]);
            std::printf("\n");
            if (++shown == 4) break;
        }
        return;
    }
    std::printf("  RUN_FALSE_MERGE none\n");
}


template <std::size_t D> struct FullGraph;

template <std::size_t D>
ravel::proof::Z2TwistedQuotientProof derive_component_sign_twist(
    const FullGraph<D>& full,
    const std::vector<std::size_t>& vertices,
    const WeightedDigraph& component_graph) {
    using Role = ravel::proof::FaceRelativeSignGrammar;
    std::map<Role, std::size_t> ids;
    std::vector<std::size_t> roles(vertices.size());
    std::vector<int> fibers(vertices.size(), 0);
    for (std::size_t local = 0; local < vertices.size(); ++local) {
        const auto& node = full.nodes[vertices[local]];
        std::vector<long long> coordinates(node.x.begin(), node.x.end());
        const auto raw = ravel::proof::derive_face_relative_sign_grammar(
            coordinates, static_cast<std::size_t>(node.i),
            static_cast<std::size_t>(node.j), false);
        const auto canonical = ravel::proof::derive_face_relative_sign_grammar(
            coordinates, static_cast<std::size_t>(node.i),
            static_cast<std::size_t>(node.j), true);
        if (!raw.replayed || !canonical.replayed)
            throw std::runtime_error("twist diagnostic: grammar replay failed");
        const auto neg = ravel::proof::negate_runs(raw.runs);
        if (canonical.runs == raw.runs) fibers[local] = 0;
        else if (canonical.runs == neg) fibers[local] = 1;
        else throw std::runtime_error("twist diagnostic: canonical orientation mismatch");
        const auto [it, inserted] = ids.emplace(canonical, ids.size());
        (void)inserted;
        roles[local] = it->second;
    }
    std::vector<std::pair<std::size_t,std::size_t>> edges;
    for (std::size_t source = 0; source < component_graph.n; ++source)
        for (const auto& [target, weight] : component_graph.out_adj[source])
            for (long long copy = 0; copy < weight; ++copy)
                edges.push_back({source, target});
    return ravel::proof::derive_z2_twisted_quotient(
        edges, roles, fibers, ids.size());
}

std::size_t graph_period(const WeightedDigraph& graph) {
    if (graph.n == 0) return 0;
    std::vector<long long> depth(graph.n, -1);
    std::vector<std::size_t> stack{0};
    depth[0] = 0;
    long long g = 0;
    while (!stack.empty()) {
        const auto source = stack.back();
        stack.pop_back();
        for (const auto& [destination, weight] : graph.out_adj[source]) {
            (void)weight;
            if (depth[destination] < 0) {
                depth[destination] = depth[source] + 1;
                stack.push_back(destination);
            } else {
                g = std::gcd(g, std::llabs(
                    depth[source] + 1 - depth[destination]));
            }
        }
    }
    return static_cast<std::size_t>(g);
}

bool is_permutation_component(const WeightedDigraph& graph) {
    if (graph.n == 0) return false;
    for (std::size_t vertex = 0; vertex < graph.n; ++vertex) {
        std::size_t out = 0;
        std::size_t in = 0;
        for (const auto& [destination, weight] : graph.out_adj[vertex]) {
            (void)destination;
            out += static_cast<std::size_t>(weight);
        }
        for (const auto& [source, weight] : graph.in_adj[vertex]) {
            (void)source;
            in += static_cast<std::size_t>(weight);
        }
        if (out != 1 || in != 1) return false;
    }
    return true;
}

std::size_t edge_weight_sum(const WeightedDigraph& graph) {
    std::size_t result = 0;
    for (const auto& row : graph.out_adj)
        for (const auto& [destination, weight] : row) {
            (void)destination;
            result += static_cast<std::size_t>(weight);
        }
    return result;
}

const char* perron_order_name(mathlib::PerronOrder order) {
    switch (order) {
        case mathlib::PerronOrder::less: return "candidate<shell";
        case mathlib::PerronOrder::equal: return "equal";
        case mathlib::PerronOrder::greater: return "candidate>shell";
        case mathlib::PerronOrder::inconclusive: return "inconclusive";
    }
    return "invalid";
}

std::string short_rat(const mathlib::Rat& value) {
    const char* raw = mpq_get_str(nullptr, 10, value.get());
    std::string result(raw == nullptr ? "?" : raw);
    if (raw != nullptr) std::free(const_cast<char*>(raw));
    constexpr std::size_t limit = 34;
    if (result.size() > limit)
        result = result.substr(0, 15) + "..." +
                 result.substr(result.size() - 15);
    return result;
}

template <std::size_t D>
struct FullGraph {
    Substitution<D> substitution;
    std::set<SNode<D>> node_set;
    std::vector<SNode<D>> nodes;
    std::map<SNode<D>, std::size_t> index;
    WeightedDigraph graph;
    int corona_rounds = 0;
    bool converged = false;
};

template <std::size_t D>
FullGraph<D> build_full_graph() {
    const auto sigma = n_bonacci_rule(D);
    const double beta = n_bonacci_beta(D);
    SubstitutionRule rule(sigma);
    auto substitution = make_substitution<D>(rule, beta);

    const auto candidates = search_D_cont<D>(substitution, 2);
    std::vector<ANode<D>> contact_seed;
    for (const auto& candidate : candidates) {
        ANode<D> node;
        node.i = candidate.i;
        node.j = candidate.j;
        node.x = candidate.x;
        contact_seed.push_back(node);
    }
    const auto closure = backward_closure<D>(
        substitution, contact_seed, kContactBoundaryMaxNodesDefault);
    const auto closure_edges = induced_restricted_edges<D>(substitution, closure);
    const std::set<ANode<D>> closure_set(closure.begin(), closure.end());
    const auto reduced_contact = red_anode<D>(closure_set, closure_edges);
    std::set<SNode<D>> contact;
    for (const auto& source : reduced_contact.first) {
        SNode<D> node;
        node.i = source.i;
        node.j = source.j;
        node.x = source.x;
        contact.insert(node);
    }

    auto previous = build_signed_contact_set<D>(contact);
    std::set<SNode<D>> gb = previous;
    bool converged = false;
    int rounds = 1;
    constexpr int max_rounds = 10;
    constexpr std::size_t max_nodes = 100000;
    for (int round = 2; round <= max_rounds; ++round) {
        auto corona = c_corona<D>(substitution, previous, previous);
        if (corona.size() > max_nodes) {
            // Retain the previous reduced layer for discovery, exactly as the
            // older n=7 probes did, but do not claim fixed-point convergence.
            // The structural verdict will remain FAIL until a streamed corona
            // closure eliminates this materialization cap.
            std::printf(
                "corona materialization cap: round=%d input=%zu candidates=%zu "
                "cap=%zu\n",
                round, previous.size(), corona.size(), max_nodes);
            break;
        }
        std::vector<std::tuple<SNode<D>, SNode<D>, std::vector<long long>,
                               std::vector<long long>>> edges;
        for (const auto& source : corona) {
            for (const auto& [destination, prefixes] :
                 simple_forward_targets<D>(substitution, source)) {
                (void)prefixes;
                if (corona.count(destination))
                    edges.push_back({source, destination, {}, {}});
            }
        }
        auto reduced = red<D>(corona, edges);
        gb = reduced.first;
        rounds = round;
        if (gb == previous) {
            converged = true;
            break;
        }
        previous = gb;
    }

    std::vector<SNode<D>> nodes(gb.begin(), gb.end());
    std::map<SNode<D>, std::size_t> index;
    for (std::size_t k = 0; k < nodes.size(); ++k) index[nodes[k]] = k;
    WeightedDigraph graph(nodes.size());
    for (std::size_t source = 0; source < nodes.size(); ++source) {
        for (const auto& [destination, prefixes] :
             simple_forward_targets<D>(substitution, nodes[source])) {
            (void)prefixes;
            const auto found = index.find(destination);
            if (found != index.end()) graph.add_edge(source, found->second, 1);
        }
    }
    return {std::move(substitution), std::move(gb), std::move(nodes),
            std::move(index), std::move(graph), rounds, converged};
}

template <std::size_t D>
std::set<SNode<D>> generated_candidate_nodes() {
    std::set<SNode<D>> result;
    for (const auto pair : nbonacci_margin::label_pairs(D)) {
        for (const auto descriptor :
             nbonacci_margin::displacement_descriptors(D)) {
            if (!nbonacci_margin::predicted_core_member(D, pair, descriptor))
                continue;
            result.insert(make_node<D>(
                pair.i,
                nbonacci_margin::displacement_from_descriptor(D, descriptor),
                pair.j));
        }
    }
    return result;
}

template <std::size_t D>
std::set<std::tuple<SNode<D>, SNode<D>>> generated_candidate_edges() {
    std::set<std::tuple<SNode<D>, SNode<D>>> result;
    for (const auto pair : nbonacci_margin::label_pairs(D)) {
        for (const auto descriptor :
             nbonacci_margin::displacement_descriptors(D)) {
            const nbonacci_margin::CoreState source{pair, descriptor};
            if (!nbonacci_margin::predicted_core_member(D, source)) continue;
            const auto source_node = make_node<D>(
                pair.i,
                nbonacci_margin::displacement_from_descriptor(D, descriptor),
                pair.j);
            for (const auto& destination :
                 nbonacci_margin::predicted_core_successors(D, source)) {
                result.insert({source_node,
                    make_node<D>(destination.pair.i,
                        nbonacci_margin::displacement_from_descriptor(
                            D, destination.displacement),
                        destination.pair.j)});
            }
        }
    }
    return result;
}

template <std::size_t D>
std::set<std::tuple<SNode<D>, SNode<D>>> actual_internal_edges(
    const FullGraph<D>& full, const std::set<SNode<D>>& candidate) {
    std::set<std::tuple<SNode<D>, SNode<D>>> result;
    for (const auto& source : candidate) {
        for (const auto& [destination, prefixes] :
             simple_forward_targets<D>(full.substitution, source)) {
            (void)prefixes;
            if (candidate.count(destination)) result.insert({source, destination});
        }
    }
    return result;
}

template <std::size_t D>
std::uint64_t component_signature(
    const std::vector<SNode<D>>& nodes,
    const std::vector<std::size_t>& component) {
    // This deliberately hashes a sorted multiset of translation/label
    // statistics rather than Tarjan's incidental vertex ordering.  It is an
    // exact within-dimension identifier; the --dump-states output supplies
    // the richer data used by the later cross-n schema aligner.
    std::vector<std::vector<long long>> records;
    for (const auto vertex : component) {
        const auto& node = nodes[vertex];
        std::vector<long long> record{node.i, node.j};
        record.insert(record.end(), node.x.begin(), node.x.end());
        records.push_back(std::move(record));
    }
    std::sort(records.begin(), records.end());
    std::uint64_t hash = 1469598103934665603ULL;
    for (const auto& record : records)
        for (const auto value : record) {
            const std::uint64_t bits = static_cast<std::uint64_t>(value);
            for (unsigned shift = 0; shift < 64; shift += 8) {
                hash ^= (bits >> shift) & 0xffULL;
                hash *= 1099511628211ULL;
            }
        }
    return hash;
}

template <std::size_t D>
bool run_dimension(const Options& options) {
    std::printf("\n=== n=%zu full-G_B dominance ledger ===\n", D);
    auto full = build_full_graph<D>();
    const DisplacementSumCatalogue sum_catalogue(D);
    std::printf("corona: |G_B|=%zu edges=%zu fixed_round=%d converged=%s\n",
                full.nodes.size(), edge_weight_sum(full.graph),
                full.corona_rounds, full.converged ? "YES" : "NO");
    bool ok = full.converged;

    const auto expected = generated_candidate_nodes<D>();
    std::set<SNode<D>> present;
    for (const auto& node : full.node_set)
        if (nbonacci_margin::predicted_core_member(
                D, {static_cast<std::size_t>(node.i),
                    static_cast<std::size_t>(node.j)}, as_vector(node.x)))
            present.insert(node);
    std::size_t missing = 0;
    for (const auto& node : expected)
        if (!full.node_set.count(node)) ++missing;
    const bool occurrence_exact = present == expected && missing == 0;
    std::printf("candidate occurrence: generated=%zu present=%zu missing=%zu %s\n",
                expected.size(), present.size(), missing,
                occurrence_exact ? "EXACT" : "FAIL");
    ok = occurrence_exact && ok;

    const auto symbolic_edges = generated_candidate_edges<D>();
    const auto concrete_edges = actual_internal_edges(full, expected);
    const bool edge_exact = symbolic_edges == concrete_edges;
    std::printf("candidate edges: generated=%zu concrete=%zu formula=%zu %s\n",
                symbolic_edges.size(), concrete_edges.size(),
                nbonacci_margin::predicted_core_edge_count(D),
                edge_exact ? "EXACT" : "FAIL");
    ok = edge_exact &&
         symbolic_edges.size() ==
             nbonacci_margin::predicted_core_edge_count(D) && ok;

    auto components = tarjan_scc(full.graph);
    std::vector<std::vector<std::size_t>> cyclic;
    for (auto& component : components)
        if (is_recurrent_scc(full.graph, component))
            cyclic.push_back(std::move(component));
    std::sort(cyclic.begin(), cyclic.end(), [](const auto& lhs, const auto& rhs) {
        if (lhs.size() != rhs.size()) return lhs.size() > rhs.size();
        return *std::min_element(lhs.begin(), lhs.end()) <
               *std::min_element(rhs.begin(), rhs.end());
    });

    std::set<std::size_t> candidate_indices;
    for (const auto& node : expected) {
        const auto found = full.index.find(node);
        if (found != full.index.end()) candidate_indices.insert(found->second);
    }
    std::size_t candidate_component = cyclic.size();
    for (std::size_t k = 0; k < cyclic.size(); ++k) {
        const std::set<std::size_t> component_set(
            cyclic[k].begin(), cyclic[k].end());
        if (component_set == candidate_indices) candidate_component = k;
        else {
            std::vector<std::size_t> overlap;
            std::set_intersection(component_set.begin(), component_set.end(),
                                  candidate_indices.begin(), candidate_indices.end(),
                                  std::back_inserter(overlap));
            if (!overlap.empty()) {
                std::printf("candidate SCC: PARTIAL overlap with component %zu\n", k);
                ok = false;
            }
        }
    }
    const bool candidate_is_scc = candidate_component < cyclic.size();
    std::printf("recurrent decomposition: cyclic_SCCs=%zu candidate_component=%s\n",
                cyclic.size(), candidate_is_scc ? "EXACT" : "MISSING");
    ok = candidate_is_scc && ok;
    if (!candidate_is_scc) return false;

    const auto candidate_graph =
        induced_component(full.graph, cyclic[candidate_component]);
    std::vector<std::vector<long long>> candidate_quotient;
    std::vector<std::vector<long long>> candidate_full_matrix;
    std::vector<std::size_t> candidate_phases;
    double candidate_rho = 0.0;
    if (!options.structural_only) {
        const auto candidate_partition =
            coarsest_equitable_partition(candidate_graph);
        candidate_quotient = quotient_matrix(candidate_graph, candidate_partition);
        candidate_full_matrix = dense_matrix(candidate_graph);
        candidate_phases.reserve(cyclic[candidate_component].size());
        for (const auto vertex : cyclic[candidate_component]) {
            const auto& node = full.nodes[vertex];
            candidate_phases.push_back(static_cast<std::size_t>(node.i) * D +
                                       static_cast<std::size_t>(node.j));
        }
        if (!options.paired_only)
            candidate_rho = dominant_eigenvalue_estimate_sparse(candidate_graph, 4000);
        if (options.paired_only)
            std::printf("candidate paired matrix: nodes=%zu quotient=%zu\n",
                        candidate_graph.n, candidate_quotient.size());
        else
            std::printf("candidate spectrum: nodes=%zu quotient=%zu rho~=%.12f\n",
                        candidate_graph.n, candidate_quotient.size(), candidate_rho);
    } else {
        std::printf("candidate structural: nodes=%zu period=%zu permutation=%s\n",
                    candidate_graph.n, graph_period(candidate_graph),
                    is_permutation_component(candidate_graph) ? "YES" : "NO");
    }

    bool exact_dominance = true;
    for (std::size_t k = 0; k < cyclic.size(); ++k) {
        const auto component_graph = induced_component(full.graph, cyclic[k]);
        if (options.twist_test) {
            const auto twist = derive_component_sign_twist(full, cyclic[k], component_graph);
            std::printf(
                "  TWIST base_roles=%zu concrete=%zu base_edges=%zu ambiguous=%zu "
                "holonomy=%zu well_defined=%s coboundary=%s genuine=%s\n",
                twist.base_roles, twist.concrete_states, twist.base_edges,
                twist.ambiguous_base_edges, twist.nontrivial_holonomy_witnesses,
                twist.transport_well_defined ? "YES" : "NO",
                twist.coboundary ? "YES" : "NO",
                twist.genuinely_twisted ? "YES" : "NO");
        }
        std::vector<std::vector<long long>> quotient;
        double rho = 0.0;
        if (!options.structural_only) {
            const auto partition = coarsest_equitable_partition(component_graph);
            quotient = quotient_matrix(component_graph, partition);
            if (!options.paired_only)
                rho = dominant_eigenvalue_estimate_sparse(component_graph, 4000);
        }
        std::size_t mirror_hits = 0;
        std::size_t catalogue_hits = 0;
        std::map<std::size_t, std::size_t> support_histogram;
        std::map<std::size_t, std::size_t> grade_histogram;
        long long max_l1 = 0;
        std::size_t grade_two_projectable = 0;
        const std::set<std::size_t> component_set(
            cyclic[k].begin(), cyclic[k].end());
        for (const auto vertex : cyclic[k]) {
            const auto& node = full.nodes[vertex];
            const auto mirror = node.mirror();
            const auto found = full.index.find(mirror);
            if (found != full.index.end() && component_set.count(found->second))
                ++mirror_hits;
            const auto x = as_vector(node.x);
            if (nbonacci_margin::describe_displacement(x)) ++catalogue_hits;
            ++grade_histogram[sum_catalogue.grade(x)];
            if (sum_catalogue.grade_two_core_projections(
                    D, {static_cast<std::size_t>(node.i),
                        static_cast<std::size_t>(node.j)}, x) > 0)
                ++grade_two_projectable;
            std::size_t support = 0;
            long long l1 = 0;
            for (const auto value : x) {
                support += value != 0 ? 1 : 0;
                l1 += std::llabs(value);
            }
            ++support_histogram[support];
            max_l1 = std::max(max_l1, l1);
        }
        std::size_t outgoing = 0, incoming = 0;
        for (const auto vertex : cyclic[k]) {
            for (const auto& [destination, weight] : full.graph.out_adj[vertex])
                if (!component_set.count(destination))
                    outgoing += static_cast<std::size_t>(weight);
            for (const auto& [source, weight] : full.graph.in_adj[vertex])
                if (!component_set.count(source))
                    incoming += static_cast<std::size_t>(weight);
        }
        if (options.structural_only) {
            std::printf(
                "SCC %zu%s: nodes=%zu edges=%zu period=%zu permutation=%s "
                "mirror=%zu/%zu catalogue=%zu/%zu maxL1=%lld cross(in=%zu,out=%zu) "
                "sig=%016llx projectable=%zu/%zu support=",
                k, k == candidate_component ? " [CANDIDATE]" : "",
                component_graph.n, edge_weight_sum(component_graph),
                graph_period(component_graph),
                is_permutation_component(component_graph) ? "YES" : "NO",
                mirror_hits, cyclic[k].size(), catalogue_hits, cyclic[k].size(),
                max_l1, incoming, outgoing,
                static_cast<unsigned long long>(
                    component_signature(full.nodes, cyclic[k])),
                grade_two_projectable, cyclic[k].size());
        } else {
            std::printf(
                "SCC %zu%s: nodes=%zu edges=%zu q=%zu rho~=%.12f gap~=%.12f "
                "mirror=%zu/%zu catalogue=%zu/%zu maxL1=%lld cross(in=%zu,out=%zu) "
                "sig=%016llx projectable=%zu/%zu support=",
                k, k == candidate_component ? " [CANDIDATE]" : "",
                component_graph.n, edge_weight_sum(component_graph), quotient.size(),
                rho, candidate_rho - rho, mirror_hits, cyclic[k].size(),
                catalogue_hits, cyclic[k].size(), max_l1, incoming, outgoing,
                static_cast<unsigned long long>(
                    component_signature(full.nodes, cyclic[k])),
                grade_two_projectable, cyclic[k].size());
        }
        for (const auto& [support, count] : support_histogram)
            std::printf("%s%zu:%zu", support == support_histogram.begin()->first ? "" : ",",
                        support, count);
        std::printf(" grade=");
        bool first_grade = true;
        for (const auto& [grade, count] : grade_histogram) {
            std::printf("%s%zu:%zu", first_grade ? "" : ",", grade, count);
            first_grade = false;
        }
        std::printf("\n");

        if (k != candidate_component && options.run_truth_only && !options.structural_only) {
            std::size_t sign_roles = 0, run_roles = 0;
            const auto sign_labels = role_labels(
                full, cyclic[k], cyclic[candidate_component], true, true,
                sign_roles);
            const auto run_labels = face_relative_run_role_labels(
                full, cyclic[k], cyclic[candidate_component], false,
                run_roles);
            if (options.diagnose_run_merges)
                diagnose_first_run_merge(
                    full, cyclic[k], cyclic[candidate_component]);
            const auto component_matrix = dense_matrix(component_graph);
            const auto sign_bellman = ravel::proof::derive_phase_handoff_bellman_renewal(
                component_matrix, sign_labels.first,
                candidate_full_matrix, sign_labels.second, sign_roles,
                options.exact_iterations);
            const auto run_bellman = ravel::proof::derive_phase_handoff_bellman_renewal(
                component_matrix, run_labels.first,
                candidate_full_matrix, run_labels.second, run_roles,
                options.exact_iterations);
            const bool same = sign_roles == run_roles &&
                sign_bellman.proved == run_bellman.proved &&
                sign_bellman.block_length == run_bellman.block_length;
            std::printf(
                "  RUN_TRUTH sign_roles=%zu sign_block=%zu sign=%s "
                "run_roles=%zu run_block=%zu run=%s same=%s\n",
                sign_roles, sign_bellman.block_length,
                sign_bellman.proved ? "PASS" : "FAIL",
                run_roles, run_bellman.block_length,
                run_bellman.proved ? "PASS" : "FAIL",
                same ? "YES" : "NO");
            exact_dominance = same && exact_dominance;
        }

        if (k != candidate_component && options.paired_only && !options.run_truth_only && !options.structural_only) {
            const auto paired = ravel::proof::derive_rank_one_paired_dominance(
                quotient, candidate_quotient, options.exact_iterations);
            const auto cone = ravel::proof::derive_path_count_cone_pair(
                quotient, candidate_quotient, options.exact_iterations);
            const auto transfer = ravel::proof::derive_graded_transfer_proof(
                quotient, candidate_quotient, options.exact_iterations);
            std::vector<std::size_t> component_phases;
            component_phases.reserve(cyclic[k].size());
            for (const auto vertex : cyclic[k]) {
                const auto& node = full.nodes[vertex];
                component_phases.push_back(static_cast<std::size_t>(node.i) * D +
                                           static_cast<std::size_t>(node.j));
            }
            const auto phase_transfer = ravel::proof::derive_phase_handoff_renewal(
                dense_matrix(component_graph), component_phases,
                candidate_full_matrix, candidate_phases, D * D,
                options.exact_iterations);
            const auto phase_bellman = ravel::proof::derive_phase_handoff_bellman_renewal(
                dense_matrix(component_graph), component_phases,
                candidate_full_matrix, candidate_phases, D * D,
                options.exact_iterations);
            std::size_t support_roles = 0, sign_roles = 0;
            const auto support_labels = role_labels(
                full, cyclic[k], cyclic[candidate_component], true, false,
                support_roles);
            const auto sign_labels = role_labels(
                full, cyclic[k], cyclic[candidate_component], true, true,
                sign_roles);
            std::size_t run_roles = 0, symmetric_run_roles = 0;
            const auto run_labels = face_relative_run_role_labels(
                full, cyclic[k], cyclic[candidate_component], false,
                run_roles);
            const auto symmetric_run_labels = face_relative_run_role_labels(
                full, cyclic[k], cyclic[candidate_component], true,
                symmetric_run_roles);
            if (options.diagnose_run_merges)
                diagnose_first_run_merge(
                    full, cyclic[k], cyclic[candidate_component]);
            const auto support_bellman = ravel::proof::derive_phase_handoff_bellman_renewal(
                dense_matrix(component_graph), support_labels.first,
                candidate_full_matrix, support_labels.second, support_roles,
                options.exact_iterations);
            const auto sign_bellman = ravel::proof::derive_phase_handoff_bellman_renewal(
                dense_matrix(component_graph), sign_labels.first,
                candidate_full_matrix, sign_labels.second, sign_roles,
                options.exact_iterations);
            const auto run_bellman = ravel::proof::derive_phase_handoff_bellman_renewal(
                dense_matrix(component_graph), run_labels.first,
                candidate_full_matrix, run_labels.second, run_roles,
                options.exact_iterations);
            const auto symmetric_run_bellman = ravel::proof::derive_phase_handoff_bellman_renewal(
                dense_matrix(component_graph), symmetric_run_labels.first,
                candidate_full_matrix, symmetric_run_labels.second,
                symmetric_run_roles, options.exact_iterations);
            std::printf("  paired: %s entries=%zu strict=%zu competitor_hi=%s core_left_lo=%s cone_horizon=%zu cone=%s renewal_block=%zu graded_transfer=%s phase_block=%zu phase_transfer=%s bellman_block=%zu bellman=%s support_roles=%zu support_block=%zu support=%s sign_roles=%zu sign_block=%zu sign=%s run_roles=%zu run_block=%zu run=%s symmetric_run_roles=%zu symmetric_run_block=%zu symmetric_run=%s\n",
                paired.replayed ? "PASS" : "FAIL",
                paired.implicit_entries, paired.strict_entries,
                short_rat(paired.competitor_right.bracket.hi).c_str(),
                short_rat(paired.core_left.bracket.lo).c_str(),
                cone.first_separating_horizon, cone.replayed ? "PASS" : "FAIL",
                transfer.renewal.block_length, transfer.proved ? "PASS" : "FAIL",
                phase_transfer.block_length, phase_transfer.proved ? "PASS" : "FAIL",
                phase_bellman.block_length, phase_bellman.proved ? "PASS" : "FAIL",
                support_roles, support_bellman.block_length,
                support_bellman.proved ? "PASS" : "FAIL",
                sign_roles, sign_bellman.block_length,
                sign_bellman.proved ? "PASS" : "FAIL",
                run_roles, run_bellman.block_length,
                run_bellman.proved ? "PASS" : "FAIL",
                symmetric_run_roles, symmetric_run_bellman.block_length,
                symmetric_run_bellman.proved ? "PASS" : "FAIL");
            if (!paired.replayed || !cone.replayed || !transfer.proved) exact_dominance = false;
        }

        if (k != candidate_component && options.exact && !options.structural_only) {
            try {
                const auto comparison = mathlib::compare_perron_roots_exact(
                    candidate_quotient, quotient,
                    options.exact_iterations, 1e-10);
                std::printf("  exact: %s  candidate=[%s,%s] shell=[%s,%s]\n",
                    perron_order_name(comparison.order),
                    short_rat(comparison.lhs.bracket.lo).c_str(),
                    short_rat(comparison.lhs.bracket.hi).c_str(),
                    short_rat(comparison.rhs.bracket.lo).c_str(),
                    short_rat(comparison.rhs.bracket.hi).c_str());
                if (comparison.order != mathlib::PerronOrder::greater)
                    exact_dominance = false;
            } catch (const std::exception& error) {
                std::printf("  exact: ERROR: %s\n", error.what());
                exact_dominance = false;
            }
        }

        if (options.dump_states) {
            std::vector<SNode<D>> component_nodes;
            for (const auto vertex : cyclic[k])
                component_nodes.push_back(full.nodes[vertex]);
            std::sort(component_nodes.begin(), component_nodes.end());
            for (const auto& node : component_nodes) {
                std::printf("  state scc=%zu i=%lld j=%lld x=", k, node.i, node.j);
                for (std::size_t coordinate = 0; coordinate < D; ++coordinate)
                    std::printf("%s%lld", coordinate == 0 ? "" : ",",
                                node.x[coordinate]);
                std::printf("\n");
            }
        }
    }
    if (options.structural_only) {
        std::printf("structural-only mode: spectral and quotient work skipped\n");
    } else if (options.exact) {
        std::printf("finite exact dominance certificate: %s\n",
                    exact_dominance ? "PASS" : "FAIL/INCONCLUSIVE");
        ok = exact_dominance && ok;
    } else {
        std::printf("finite spectral ordering: estimates only (rerun with --exact)\n");
    }
    std::printf("n=%zu structural ledger: %s\n", D, ok ? "PASS" : "FAIL");
    return ok;
}

bool dispatch(std::size_t n, const Options& options) {
    switch (n) {
        case 3: return run_dimension<3>(options);
        case 4: return run_dimension<4>(options);
        case 5: return run_dimension<5>(options);
        case 6: return run_dimension<6>(options);
        case 7: return run_dimension<7>(options);
        case 8: return run_dimension<8>(options);
        default:
            std::fprintf(stderr, "unsupported n=%zu (compiled range is 3..8)\n", n);
            return false;
    }
}

}  // namespace

int main(int argc, char** argv) {
    Options options;
    std::vector<std::size_t> dimensions;
    for (int arg = 1; arg < argc; ++arg) {
        const std::string value(argv[arg]);
        if (value == "--exact") options.exact = true;
        else if (value == "--structural-only") options.structural_only = true;
        else if (value == "--paired-only") options.paired_only = true;
        else if (value == "--diagnose-run-merges") options.diagnose_run_merges = true;
        else if (value == "--run-truth-only") { options.run_truth_only = true; options.paired_only = true; }
        else if (value == "--twist-test") options.twist_test = true;
        else if (value == "--dump-states") options.dump_states = true;
        else if (value.rfind("--exact-iters=", 0) == 0)
            options.exact_iterations = std::stoi(value.substr(14));
        else dimensions.push_back(static_cast<std::size_t>(std::stoul(value)));
    }
    if (dimensions.empty()) dimensions = {3, 4, 5, 6};
    bool ok = true;
    for (const auto dimension : dimensions)
        ok = dispatch(dimension, options) && ok;
    return ok ? 0 : 1;
}
