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
#include "ravel/substitution.hpp"

using namespace ravel;

namespace {

struct Options {
    bool exact = false;
    bool dump_states = false;
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
    const auto candidate_partition =
        coarsest_equitable_partition(candidate_graph);
    const auto candidate_quotient =
        quotient_matrix(candidate_graph, candidate_partition);
    const double candidate_rho =
        dominant_eigenvalue_estimate_sparse(candidate_graph, 4000);
    std::printf("candidate spectrum: nodes=%zu quotient=%zu rho~=%.12f\n",
                candidate_graph.n, candidate_quotient.size(), candidate_rho);

    bool exact_dominance = true;
    for (std::size_t k = 0; k < cyclic.size(); ++k) {
        const auto component_graph = induced_component(full.graph, cyclic[k]);
        const auto partition = coarsest_equitable_partition(component_graph);
        const auto quotient = quotient_matrix(component_graph, partition);
        const double rho =
            dominant_eigenvalue_estimate_sparse(component_graph, 4000);
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

        if (k != candidate_component && options.exact) {
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
    if (options.exact) {
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
