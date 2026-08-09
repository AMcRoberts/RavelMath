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
#include <chrono>
#include <csignal>
#include <exception>
#include <sys/resource.h>
#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <fstream>
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
#include "ravel/corona_projection.hpp"
#include "ravel/d_cont_check.hpp"
#include "ravel/faces.hpp"
#include "ravel/graph_divisor.hpp"
#include "ravel/involution_helpers.hpp"
#include "ravel/nbonacci_margin_invariant.hpp"
#include "ravel/proof/paired_matrix_dominance.hpp"
#include "ravel/proof/path_count_cone.hpp"
#include "ravel/proof/graded_transfer.hpp"
#include "ravel/proof/graded_core_descent.hpp"
#include "ravel/proof/phase_handoff_renewal.hpp"
#include "ravel/proof/face_relative_sign_grammar.hpp"
#include "ravel/proof/signed_interval_bellman.hpp"
#include "ravel/proof/twisted_quotient.hpp"
#include "ravel/proof/twisted_bellman_transport.hpp"
#include "ravel/proof/twisted_dimension_extension.hpp"
#include "ravel/proof/adjacent_twisted_renewal.hpp"
#include "ravel/proof/rejected_boundary_exhaustion.hpp"
#include "ravel/substitution.hpp"

using namespace ravel;

namespace {

using SteadyClock = std::chrono::steady_clock;
static SteadyClock::time_point g_process_start = SteadyClock::now();
static const char* g_stage = "startup";

long long elapsed_ms() {
    return std::chrono::duration_cast<std::chrono::milliseconds>(
        SteadyClock::now() - g_process_start).count();
}

long long peak_rss_kb() {
    struct rusage usage {};
    return getrusage(RUSAGE_SELF, &usage) == 0 ? usage.ru_maxrss : -1;
}

void stage(const char* name) {
    g_stage = name;
    std::fprintf(stderr,
        "STAGE_BEGIN name=%s elapsed_ms=%lld peak_rss_kb=%lld\n",
        name, elapsed_ms(), peak_rss_kb());
    std::fflush(stderr);
    std::fflush(stdout);
}

void stage_done(const char* name, std::size_t items = 0) {
    std::fprintf(stderr,
        "STAGE_DONE name=%s elapsed_ms=%lld peak_rss_kb=%lld items=%zu\n",
        name, elapsed_ms(), peak_rss_kb(), items);
    std::fflush(stderr);
    std::fflush(stdout);
}

void fatal_signal(int signal_number) {
    const char* label = signal_number == SIGSEGV ? "SIGSEGV" :
        signal_number == SIGABRT ? "SIGABRT" :
        signal_number == SIGTERM ? "SIGTERM" :
        signal_number == SIGINT ? "SIGINT" : "SIGNAL";
    std::fprintf(stderr,
        "FATAL_SIGNAL signal=%s(%d) stage=%s elapsed_ms=%lld peak_rss_kb=%lld\n",
        label, signal_number, g_stage, elapsed_ms(), peak_rss_kb());
    std::fflush(stderr);
    std::_Exit(128 + signal_number);
}

void install_failure_reporting() {
    std::signal(SIGSEGV, fatal_signal);
    std::signal(SIGABRT, fatal_signal);
    std::signal(SIGTERM, fatal_signal);
    std::signal(SIGINT, fatal_signal);
    std::set_terminate([] {
        std::fprintf(stderr,
            "TERMINATE stage=%s elapsed_ms=%lld peak_rss_kb=%lld\n",
            g_stage, elapsed_ms(), peak_rss_kb());
        if (const auto error = std::current_exception()) {
            try { std::rethrow_exception(error); }
            catch (const std::exception& ex) {
                std::fprintf(stderr, "TERMINATE_EXCEPTION what=%s\n", ex.what());
            } catch (...) {
                std::fprintf(stderr, "TERMINATE_EXCEPTION what=non_std_exception\n");
            }
        }
        std::fflush(stderr);
        std::_Exit(134);
    });
}

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
struct CoronaInput {
    Substitution<D> substitution;
    std::set<SNode<D>> contact;
};

template <std::size_t D>
CoronaInput<D> build_corona_input() {
    const auto sigma = n_bonacci_rule(D);
    const double beta = n_bonacci_beta(D);
    SubstitutionRule rule(sigma);
    auto substitution = make_substitution<D>(rule, beta);
    const auto candidates = search_D_cont<D>(substitution, 2);
    std::vector<ANode<D>> contact_seed;
    for (const auto& candidate : candidates)
        contact_seed.push_back({candidate.i, candidate.x, candidate.j});
    const auto closure = backward_closure<D>(
        substitution, contact_seed, kContactBoundaryMaxNodesDefault);
    const auto closure_edges = induced_restricted_edges<D>(substitution, closure);
    const std::set<ANode<D>> closure_set(closure.begin(), closure.end());
    const auto reduced_contact = red_anode<D>(closure_set, closure_edges);
    std::set<SNode<D>> contact;
    for (const auto& source : reduced_contact.first)
        contact.insert({source.i, source.x, source.j});
    return {std::move(substitution), std::move(contact)};
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
    auto input = build_corona_input<D>();
    auto substitution = std::move(input.substitution);
    auto contact = std::move(input.contact);
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
FullGraph<D> full_graph_from_nodes(Substitution<D> substitution,
                                   std::set<SNode<D>> nodeset,
                                   int rounds,
                                   bool converged) {
    std::vector<SNode<D>> nodes(nodeset.begin(), nodeset.end());
    std::map<SNode<D>, std::size_t> index;
    for (std::size_t k = 0; k < nodes.size(); ++k) index[nodes[k]] = k;
    WeightedDigraph graph(nodes.size());
    for (std::size_t source = 0; source < nodes.size(); ++source)
        for (const auto& [target, witness] :
             simple_forward_targets<D>(substitution, nodes[source])) {
            (void)witness;
            const auto found = index.find(target);
            if (found != index.end()) graph.add_edge(source, found->second, 1);
        }
    return {std::move(substitution), std::move(nodeset), std::move(nodes),
            std::move(index), std::move(graph), rounds, converged};
}

template <std::size_t D>
FullGraph<D> build_projected_recurrent_graph(std::size_t maximum_grade = 2) {
    auto input = build_corona_input<D>();
    ravel::proof::DisplacementSumCatalogue catalogue(D);
    const auto trace = algorithm2_projected_trace<D>(
        input.substitution, input.contact,
        [&](const SNode<D>& node) {
            const std::vector<long long> x(node.x.begin(), node.x.end());
            const auto grade = catalogue.grade(x);
            return grade != 0 && grade <= maximum_grade;
        },
        CoronaConnectorPolicy::evolving_layer, 10, 250000,
        CoronaEdgeArithmetic::fast_rounded);
    std::printf(
        "CORONA_PROJECT n=%zu scope=grades<=%zu nodes=%zu rounds=%zu "
        "converged=%s rejected=%zu\n",
        D, maximum_grade, trace.final_nodes.size(), trace.layers.size(),
        trace.converged ? "YES" : "NO", trace.rejected_boundary.size());
    return full_graph_from_nodes<D>(
        std::move(input.substitution), trace.final_nodes,
        static_cast<int>(trace.layers.size()), trace.converged);
}

template <std::size_t D>
CoronaTruthGraph<D> truth_projection_owner(const FullGraph<D>& full) {
    CoronaTruthGraph<D> result;
    result.nodes = full.nodes;
    result.index = full.index;
    result.graph = full.graph;
    result.corona_rounds = full.corona_rounds;
    result.converged = full.converged;
    return result;
}

template <std::size_t D>
FullGraph<D> load_or_build_full_graph() {
    const char* cache_root = std::getenv("RAVEL_CORONA_CACHE_DIR");
    const std::string root = cache_root == nullptr ? "out" : cache_root;
    const std::string path = root + "/corona_truth_n" + std::to_string(D) + ".bin";
    {
        std::ifstream probe(path, std::ios::binary);
        if (probe.good()) {
            probe.close();
            auto cached = load_corona_truth_graph<D>(path);
            const auto sigma = n_bonacci_rule(D);
            const double beta = n_bonacci_beta(D);
            SubstitutionRule rule(sigma);
            auto substitution = make_substitution<D>(rule, beta);
            std::set<SNode<D>> node_set(cached.nodes.begin(), cached.nodes.end());
            std::printf("CORONA_CACHE n=%zu source=cache nodes=%zu path=%s\n",
                        D, cached.nodes.size(), path.c_str());
            return {std::move(substitution), std::move(node_set),
                    std::move(cached.nodes), std::move(cached.index),
                    std::move(cached.graph), cached.corona_rounds,
                    cached.converged};
        }
    }
    auto full = build_full_graph<D>();
    std::filesystem::create_directories(root);
    save_corona_truth_graph(truth_projection_owner(full), path);
    std::printf("CORONA_CACHE n=%zu source=built nodes=%zu path=%s\n",
                D, full.nodes.size(), path.c_str());
    return full;
}

template <std::size_t D>
SNode<D + 1> append_zero(const SNode<D>& x);

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

// Request-driven recurrent-family discovery.  For D <= 5 the compact
// grade-bounded projected graph is already cheap.  For higher dimensions,
// derive each competitor family only from the zero-appended shadow of its
// predecessor family, and combine those complete family images with the
// formula-generated predicted core.  No unrelated grade-two region is built.
template <std::size_t D>
FullGraph<D> build_request_driven_recurrent_graph() {
    if constexpr (D <= 5) {
        return build_projected_recurrent_graph<D>(2);
    } else {
        auto previous = build_request_driven_recurrent_graph<D - 1>();
        const auto previous_components = recurrent_components(previous);
        const auto previous_core = predicted_component_index(
            previous, previous_components);
        if (previous_core == previous_components.size())
            throw std::runtime_error(
                "request-driven family discovery: missing previous core");

        auto input = build_corona_input<D>();
        ravel::proof::DisplacementSumCatalogue previous_catalogue(D - 1);
        ravel::proof::DisplacementSumCatalogue catalogue(D);
        CoronaSurface<D> surface(input.substitution, input.contact);
        std::set<SNode<D>> discovered = generated_candidate_nodes<D>();
        std::set<SNode<D>> rejected_noncore_boundary;
        std::size_t discovered_families = 0;

        for (std::size_t component = 0;
             component < previous_components.size(); ++component) {
            if (component == previous_core) continue;
            const auto& vertices = previous_components[component];
            if (vertices.empty()) continue;
            std::set<std::size_t> family_grades;
            for (const auto vertex : vertices) {
                const auto& source_node = previous.nodes[vertex];
                const std::vector<long long> source_x(
                    source_node.x.begin(), source_node.x.end());
                const auto source_grade = previous_catalogue.grade(source_x);
                if (source_grade != 0) family_grades.insert(source_grade);
            }
            if (family_grades.empty()) {
                std::printf(
                    "LOWER_FAMILY_DISCOVERY n=%zu source_component=%zu "
                    "unsupported_grades=empty\n", D, component);
                continue;
            }

            const auto previous_component_graph =
                induced_component(previous.graph, vertices);
            const bool terminal_permutation =
                is_permutation_component(previous_component_graph);
            CoronaProjectionRequest<D> request;
            request.node_cap = terminal_permutation ? 1000000 : 500000;
            request.operations = terminal_permutation
                ? static_cast<unsigned>(CoronaSurfaceOperation::simple_forward)
                : static_cast<unsigned>(CoronaSurfaceOperation::simple_forward) |
                  static_cast<unsigned>(CoronaSurfaceOperation::simple_backward);
            for (const auto vertex : vertices)
                request.seeds.push_back(append_zero(previous.nodes[vertex]));
            request.accept = [&](const SNode<D>& node) {
                const std::vector<long long> x(node.x.begin(), node.x.end());
                return family_grades.count(catalogue.grade(x)) != 0;
            };
            const auto image = surface.project(request);
            for (const auto& node : image.rejected_boundary) {
                const std::vector<long long> x(node.x.begin(), node.x.end());
                const auto g = catalogue.grade(x);
                if (g >= 2 && g <= 3) rejected_noncore_boundary.insert(node);
            }
            if (!image.complete())
                throw std::runtime_error(
                    "request-driven family discovery: incomplete surface image");

            const auto requested_nodes = image.nodes;
            const auto trace = algorithm2_projected_trace<D>(
                input.substitution, input.contact,
                [&](const SNode<D>& node) {
                    const std::vector<long long> x(node.x.begin(), node.x.end());
                    return catalogue.grade(x) == 1 ||
                           requested_nodes.count(node) != 0;
                },
                CoronaConnectorPolicy::evolving_layer, 10, 500000,
                CoronaEdgeArithmetic::fast_rounded);
            for (const auto& node : trace.rejected_boundary) {
                const std::vector<long long> x(node.x.begin(), node.x.end());
                const auto g = catalogue.grade(x);
                if (g >= 2 && g <= 3) rejected_noncore_boundary.insert(node);
            }
            if (!trace.converged || trace.node_cap_hit)
                throw std::runtime_error(
                    "request-driven family discovery: projected Algorithm 2 incomplete");

            auto scope = full_graph_from_nodes<D>(
                input.substitution, trace.final_nodes,
                static_cast<int>(trace.layers.size()), true);
            const auto components = recurrent_components(scope);
            std::size_t matching = components.size();
            for (std::size_t candidate = 0;
                 candidate < components.size(); ++candidate) {
                const std::set<std::size_t> candidate_set(
                    components[candidate].begin(), components[candidate].end());
                bool contains_shadow = true;
                for (const auto vertex : vertices) {
                    const auto found = scope.index.find(
                        append_zero(previous.nodes[vertex]));
                    if (found == scope.index.end() ||
                        candidate_set.count(found->second) == 0) {
                        contains_shadow = false;
                        break;
                    }
                }
                if (contains_shadow) {
                    matching = candidate;
                    break;
                }
            }
            if (matching == components.size())
                throw std::runtime_error(
                    "request-driven family discovery: no recurrent family contains shadow");

            for (const auto vertex : components[matching])
                discovered.insert(scope.nodes[vertex]);
            ++discovered_families;
            std::printf(
                "LOWER_FAMILY_DISCOVERY n=%zu source_dimension=%zu "
                "source_states=%zu request_nodes=%zu scope_nodes=%zu "
                "family_states=%zu complete=YES\n",
                D, D - 1, vertices.size(), image.nodes.size(),
                trace.final_nodes.size(), components[matching].size());
        }

        // Close the rejected non-core boundary.  Any recurrent component
        // born here is part of the inductive family state and must be carried
        // forward on the next step; terminal at one dimension does not imply
        // terminal after zero-extension.
        if (!rejected_noncore_boundary.empty()) {
            CoronaProjectionRequest<D> boundary_request;
            boundary_request.seeds.assign(
                rejected_noncore_boundary.begin(), rejected_noncore_boundary.end());
            boundary_request.node_cap = 1000000;
            boundary_request.operations =
                static_cast<unsigned>(CoronaSurfaceOperation::simple_forward) |
                static_cast<unsigned>(CoronaSurfaceOperation::simple_backward);
            boundary_request.accept = [&](const SNode<D>& node) {
                const std::vector<long long> x(node.x.begin(), node.x.end());
                const auto g = catalogue.grade(x);
                return g >= 2 && g <= 3;
            };
            const auto boundary_image = surface.project(boundary_request);
            if (!boundary_image.complete())
                throw std::runtime_error(
                    "request-driven family discovery: incomplete rejected-boundary image");
            auto boundary_nodes = boundary_image.nodes;
            boundary_nodes.insert(discovered.begin(), discovered.end());
            const auto boundary_trace = algorithm2_projected_trace<D>(
                input.substitution, input.contact,
                [&](const SNode<D>& node) {
                    const std::vector<long long> x(node.x.begin(), node.x.end());
                    return catalogue.grade(x) == 1 || boundary_nodes.count(node) != 0;
                },
                CoronaConnectorPolicy::evolving_layer, 10, 1000000,
                CoronaEdgeArithmetic::fast_rounded);
            if (!boundary_trace.converged || boundary_trace.node_cap_hit)
                throw std::runtime_error(
                    "request-driven family discovery: rejected-boundary Algorithm 2 incomplete");
            auto boundary_scope = full_graph_from_nodes<D>(
                input.substitution, boundary_trace.final_nodes,
                static_cast<int>(boundary_trace.layers.size()), true);
            const auto boundary_components = recurrent_components(boundary_scope);
            std::size_t births = 0;
            for (const auto& family : boundary_components) {
                bool already_known = true;
                for (const auto vertex : family)
                    already_known = already_known &&
                        discovered.count(boundary_scope.nodes[vertex]) != 0;
                if (already_known) continue;
                for (const auto vertex : family)
                    discovered.insert(boundary_scope.nodes[vertex]);
                ++births;
            }
            std::printf(
                "LOWER_FAMILY_BOUNDARY n=%zu seeds=%zu image_nodes=%zu "
                "scope_nodes=%zu births=%zu complete=YES\n",
                D, rejected_noncore_boundary.size(), boundary_image.nodes.size(),
                boundary_trace.final_nodes.size(), births);
        }

        auto result = full_graph_from_nodes<D>(
            std::move(input.substitution), std::move(discovered), 0, true);
        std::printf(
            "LOWER_FAMILY_DISCOVERY_SUMMARY n=%zu families=%zu nodes=%zu "
            "strategy=recursive_requested_images\n",
            D, discovered_families, result.nodes.size());
        return result;
    }
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



template <std::size_t D>
std::vector<std::vector<std::size_t>> recurrent_components(const FullGraph<D>& full) {
    auto components = tarjan_scc(full.graph);
    std::vector<std::vector<std::size_t>> cyclic;
    for (auto& c : components) if (is_recurrent_scc(full.graph, c)) cyclic.push_back(std::move(c));
    std::sort(cyclic.begin(), cyclic.end(), [](const auto& a, const auto& b) {
        if (a.size() != b.size()) return a.size() > b.size();
        return *std::min_element(a.begin(), a.end()) < *std::min_element(b.begin(), b.end());
    });
    return cyclic;
}

template <std::size_t D>
std::size_t predicted_component_index(const FullGraph<D>& full,
                                      const std::vector<std::vector<std::size_t>>& cyclic) {
    const auto expected = generated_candidate_nodes<D>();
    std::set<std::size_t> indices;
    for (const auto& node : expected) {
        auto it = full.index.find(node);
        if (it != full.index.end()) indices.insert(it->second);
    }
    for (std::size_t k=0;k<cyclic.size();++k) {
        if (std::set<std::size_t>(cyclic[k].begin(),cyclic[k].end()) == indices) return k;
    }
    return cyclic.size();
}

template <std::size_t D>
SNode<D+1> append_zero(const SNode<D>& x) {
    SNode<D+1> y;
    y.i=x.i; y.j=x.j;
    for (std::size_t k=0;k<D;++k) y.x[k]=x.x[k];
    y.x[D]=0;
    return y;
}

inline std::vector<std::size_t> shortest_path_local(
    const WeightedDigraph& g, std::size_t s, std::size_t t) {
    std::vector<std::size_t> parent(g.n,g.n), q;
    q.push_back(s); parent[s]=s;
    for (std::size_t h=0;h<q.size() && parent[t]==g.n;++h) {
        auto u=q[h];
        for (auto [v,w]:g.out_adj[u]) { (void)w; if(parent[v]==g.n){parent[v]=u;q.push_back(v);} }
    }
    if(parent[t]==g.n) return {};
    std::vector<std::size_t> p;
    for(auto v=t;;v=parent[v]) {p.push_back(v); if(v==s)break;}
    std::reverse(p.begin(),p.end()); return p;
}

template <std::size_t D>
bool run_adjacent_competitor_transport_legacy() {
    auto lower=load_or_build_full_graph<D>();
    auto upper=load_or_build_full_graph<D+1>();
    auto lc=recurrent_components(lower), uc=recurrent_components(upper);
    auto lcore=predicted_component_index(lower,lc), ucore=predicted_component_index(upper,uc);
    if(lcore==lc.size()||ucore==uc.size()) return false;
    bool all=true; std::size_t families=0;
    for(std::size_t li=0;li<lc.size();++li) {
        if(li==lcore) continue;
        std::map<std::size_t,std::size_t> upper_global_to_local;
        std::size_t ui=uc.size();
        std::vector<std::size_t> embedded;
        for(std::size_t cand=0;cand<uc.size();++cand) {
            if(cand==ucore) continue;
            std::set<std::size_t> cset(uc[cand].begin(),uc[cand].end());
            std::vector<std::size_t> trial; bool ok=true;
            for(auto lv:lc[li]) {
                auto it=upper.index.find(append_zero(lower.nodes[lv]));
                if(it==upper.index.end()||!cset.count(it->second)){ok=false;break;}
                trial.push_back(it->second);
            }
            if(ok){ui=cand;embedded=std::move(trial);break;}
        }
        if(ui==uc.size()) {
            std::printf("ADJ_COMP n=%zu lower_scc=%zu NO_UPPER_FAMILY\n",D,li); all=false; continue;
        }
        const auto upper_owner = truth_projection_owner(upper);
        const auto lower_owner = truth_projection_owner(lower);
        const auto upper_view = project_corona_vertices(upper_owner, uc[ui]);
        const auto lower_view = project_corona_vertices(lower_owner, lc[li]);
        auto ug = upper_view.materialize();
        auto lg = lower_view.materialize();
        for(std::size_t k=0;k<uc[ui].size();++k)
            upper_global_to_local[uc[ui][k]]=upper_view.local(uc[ui][k]);
        std::map<std::size_t,std::size_t> lower_global_to_local;
        for(std::size_t k=0;k<lc[li].size();++k)
            lower_global_to_local[lc[li][k]]=lower_view.local(lc[li][k]);
        bool paths=true; std::size_t direct=0,sub=0,maxlen=0,edges=0;
        for(std::size_t ls=0;ls<lg.n;++ls) for(auto [lt,w]:lg.out_adj[ls]) {
            for(long long copy=0;copy<w;++copy) {
                ++edges;
                auto us=upper_global_to_local.at(embedded[ls]);
                auto ut=upper_global_to_local.at(embedded[lt]);
                auto p=shortest_path_local(ug,us,ut);
                if(p.empty()){paths=false;continue;}
                auto len=p.size()-1; maxlen=std::max(maxlen,len); if(len==1)++direct;else ++sub;
            }
        }
        // Derive raw and canonical role transport from embedded representatives.
        using Role=ravel::proof::FaceRelativeSignGrammar;
        std::map<Role,std::size_t> lrids, urids;
        std::vector<std::size_t> lr(lg.n), ur(ug.n);
        auto role_of=[](const auto& node,bool canon){
            std::vector<long long> x(node.x.begin(),node.x.end());
            return ravel::proof::derive_face_relative_sign_grammar(x,(std::size_t)node.i,(std::size_t)node.j,canon);
        };
        for(std::size_t k=0;k<lg.n;++k){auto r=role_of(lower.nodes[lc[li][k]],false);lr[k]=lrids.emplace(r,lrids.size()).first->second;}
        for(std::size_t k=0;k<ug.n;++k){auto r=role_of(upper.nodes[uc[ui][k]],false);ur[k]=urids.emplace(r,urids.size()).first->second;}
        std::map<std::size_t,std::size_t> rembed; bool role_wd=true,injective=true;
        for(std::size_t k=0;k<lg.n;++k){auto a=lr[k],b=ur[upper_global_to_local.at(embedded[k])];auto[it,ins]=rembed.emplace(a,b);if(!ins&&it->second!=b)role_wd=false;}
        std::set<std::size_t> image; for(auto [a,b]:rembed){(void)a;if(!image.insert(b).second)injective=false;}
        // Compare complete outgoing branch profiles on lower states against macro-edge destinations.
        bool profiles=true; std::size_t checked=0;
        std::map<std::size_t,std::set<std::vector<long long>>> lower_profiles, macro_profiles;
        for(std::size_t s=0;s<lg.n;++s){
            std::vector<long long> lp(lrids.size(),0), mp(urids.size(),0);
            for(auto[t,w]:lg.out_adj[s]){lp[lr[t]]+=w; mp[ur[upper_global_to_local.at(embedded[t])]]+=w;}
            lower_profiles[lr[s]].insert(lp); macro_profiles[ur[upper_global_to_local.at(embedded[s])]].insert(mp); ++checked;
        }
        for(auto const& [r,fam]:lower_profiles){
            std::set<std::vector<long long>> expected;
            for(auto const& p:fam){std::vector<long long> q(urids.size(),0);for(std::size_t a=0;a<p.size();++a)q[rembed.at(a)]+=p[a];expected.insert(q);}
            if(expected!=macro_profiles[rembed.at(r)]) profiles=false;
        }
        bool pass=paths&&role_wd&&injective&&profiles;
        ++families; all=all&&pass;
        std::printf("ADJ_COMP n=%zu->%zu lower_scc=%zu upper_scc=%zu lower_states=%zu upper_states=%zu lower_roles=%zu upper_roles=%zu edges=%zu direct=%zu substituted=%zu max_path=%zu role_wd=%s injective=%s profiles=%s PASS=%s\n",
            D,D+1,li,ui,lg.n,ug.n,lrids.size(),urids.size(),edges,direct,sub,maxlen,role_wd?"YES":"NO",injective?"YES":"NO",profiles?"YES":"NO",pass?"YES":"NO");
    }
    std::printf("ADJ_COMP_SUMMARY n=%zu->%zu families=%zu result=%s\n",D,D+1,families,all?"PASS":"FAIL");
    return all;
}


template <std::size_t D>
bool run_adjacent_competitor_transport_projected() {
    stage("lower_family_discovery");
    auto lower = build_request_driven_recurrent_graph<D>();
    stage_done("lower_family_discovery", lower.nodes.size());
    stage("lower_scc_extraction");
    const auto lower_components = recurrent_components(lower);
    stage_done("lower_scc_extraction", lower_components.size());
    const auto lower_core = predicted_component_index(lower, lower_components);
    if (lower_core == lower_components.size()) {
        std::printf("ADJ_COMP_PROJECT n=%zu missing lower predicted core\n", D);
        return false;
    }

    stage("upper_corona_input");
    auto upper_input = build_corona_input<D + 1>();
    stage_done("upper_corona_input", upper_input.contact.size());
    ravel::proof::DisplacementSumCatalogue lower_catalogue(D);
    ravel::proof::DisplacementSumCatalogue upper_catalogue(D + 1);
    CoronaSurface<D + 1> upper_surface(
        upper_input.substitution, upper_input.contact);

    bool all = true;
    std::size_t families = 0;
    std::set<SNode<D + 1>> rejected_grade_two;
    std::set<SNode<D + 1>> known_upper_family_nodes;
    for (std::size_t lower_component = 0;
         lower_component < lower_components.size(); ++lower_component) {
        if (lower_component == lower_core) continue;
        const auto& lower_vertices = lower_components[lower_component];
        const auto lower_component_graph = induced_component(lower.graph, lower_vertices);
        const bool lower_terminal_permutation =
            is_permutation_component(lower_component_graph);
        const auto& sample = lower.nodes[lower_vertices.front()];
        const std::vector<long long> sample_x(sample.x.begin(), sample.x.end());
        const auto grade = lower_catalogue.grade(sample_x);
        if (grade == 0) {
            std::printf(
                "ADJ_COMP_PROJECT n=%zu lower_scc=%zu unsupported_grade=0\n",
                D, lower_component);
            all = false;
            continue;
        }

        std::vector<SNode<D + 1>> seeds;
        for (const auto vertex : lower_vertices)
            seeds.push_back(append_zero(lower.nodes[vertex]));

        CoronaProjectionRequest<D + 1> request;
        request.seeds = seeds;
        request.node_cap = lower_terminal_permutation ? 1000000 : 250000;
        request.operations = lower_terminal_permutation
            ? static_cast<unsigned>(CoronaSurfaceOperation::simple_forward)
            : static_cast<unsigned>(CoronaSurfaceOperation::simple_forward) |
              static_cast<unsigned>(CoronaSurfaceOperation::simple_backward);
        request.accept = [&](const SNode<D + 1>& node) {
            const std::vector<long long> x(node.x.begin(), node.x.end());
            return upper_catalogue.grade(x) == grade;
        };
        std::fprintf(stderr, "FAMILY_BEGIN lower_scc=%zu lower_states=%zu grade=%zu seeds=%zu elapsed_ms=%lld peak_rss_kb=%lld\n", lower_component, lower_vertices.size(), grade, seeds.size(), elapsed_ms(), peak_rss_kb());
        std::fflush(stderr);
        stage("upper_surface_projection");
        const auto requested_image = upper_surface.project(request);
        stage_done("upper_surface_projection", requested_image.nodes.size());
        if (!requested_image.complete()) {
            std::printf(
                "ADJ_COMP_PROJECT n=%zu lower_scc=%zu image_incomplete "
                "nodes=%zu node_cap=%s expansion_cap=%s\n",
                D, lower_component, requested_image.nodes.size(),
                requested_image.certificate.node_cap_hit ? "YES" : "NO",
                requested_image.certificate.expansion_cap_hit ? "YES" : "NO");
            all = false;
            continue;
        }

        for (const auto& node : requested_image.rejected_boundary) {
            const auto atom_witness = derive_corona_atom_witness(node);
            if (!atom_witness) continue;
            const std::vector<long long> x(node.x.begin(), node.x.end());
            // A two-atom witness is not by itself a grade-two witness:
            // overlapping atoms can cancel and leave a grade-one vector.
            // Only retain candidates whose canonical catalogue grade is
            // exactly two; lower-grade boundary points are already covered
            // by the accepted predicate and must not poison the exhaustion
            // seed set.
            if (upper_catalogue.grade(x) != 2) continue;
            rejected_grade_two.insert(node);
        }
        const auto requested_nodes = requested_image.nodes;
        stage("upper_projected_algorithm2");
        const auto projected_trace = algorithm2_projected_trace<D + 1>(
            upper_input.substitution, upper_input.contact,
            [&](const SNode<D + 1>& node) {
                const std::vector<long long> x(node.x.begin(), node.x.end());
                return upper_catalogue.grade(x) == 1 ||
                       requested_nodes.count(node) != 0;
            },
            CoronaConnectorPolicy::evolving_layer, 10, 250000,
            CoronaEdgeArithmetic::fast_rounded);
        stage_done("upper_projected_algorithm2", projected_trace.final_nodes.size());
        for (const auto& node : projected_trace.rejected_boundary) {
            const auto atom_witness = derive_corona_atom_witness(node);
            if (!atom_witness) continue;
            const std::vector<long long> x(node.x.begin(), node.x.end());
            if (upper_catalogue.grade(x) != 2) continue;
            rejected_grade_two.insert(node);
        }
        if (!projected_trace.converged || projected_trace.node_cap_hit) {
            std::printf(
                "ADJ_COMP_PROJECT n=%zu lower_scc=%zu projected_algorithm2 "
                "incomplete nodes=%zu rounds=%zu\n",
                D, lower_component, projected_trace.final_nodes.size(),
                projected_trace.layers.size());
            all = false;
            continue;
        }

        stage("upper_graph_materialization");
        auto upper = full_graph_from_nodes<D + 1>(
            upper_input.substitution, projected_trace.final_nodes,
            static_cast<int>(projected_trace.layers.size()), true);
        stage_done("upper_graph_materialization", upper.nodes.size());
        stage("upper_scc_extraction");
        const auto upper_components = recurrent_components(upper);
        stage_done("upper_scc_extraction", upper_components.size());
        const auto upper_core = predicted_component_index(upper, upper_components);
        if (upper_core == upper_components.size()) {
            std::printf(
                "ADJ_COMP_PROJECT n=%zu lower_scc=%zu missing upper predicted core\n",
                D, lower_component);
            all = false;
            continue;
        }
        std::size_t matching_component = upper_components.size();
        std::vector<std::size_t> embedded;
        for (std::size_t candidate = 0;
             candidate < upper_components.size(); ++candidate) {
            const std::set<std::size_t> candidate_set(
                upper_components[candidate].begin(),
                upper_components[candidate].end());
            std::vector<std::size_t> trial;
            bool matches = true;
            for (const auto lower_vertex : lower_vertices) {
                const auto found = upper.index.find(
                    append_zero(lower.nodes[lower_vertex]));
                if (found == upper.index.end() ||
                    candidate_set.count(found->second) == 0) {
                    matches = false;
                    break;
                }
                trial.push_back(found->second);
            }
            if (matches) {
                matching_component = candidate;
                embedded = std::move(trial);
                break;
            }
        }
        // A lower family may enter its upper recurrent successor through a
        // transient shadow path rather than landing inside it.  This occurs
        // for terminal permutation families: several lower cycles can merge
        // into one nonterminal upper family.  Derive the common recurrent
        // destination and retain the chosen entrance endpoints as the role
        // representatives used by the macro-profile proof.
        if (matching_component == upper_components.size()) {
            for (std::size_t candidate = 0;
                 candidate < upper_components.size(); ++candidate) {
                if (candidate == upper_core) continue;
                std::vector<std::size_t> trial;
                bool all_enter = true;
                // Build the pre-Red request graph lazily.  Transient shadows
                // can disappear under Red even though they carry certified
                // paths into the surviving recurrent family.  One reverse
                // multi-source BFS from the candidate SCC computes the nearest
                // recurrent endpoint for every request node at once.
                std::vector<SNode<D + 1>> request_vector(
                    requested_image.nodes.begin(), requested_image.nodes.end());
                std::map<SNode<D + 1>, std::size_t> request_index;
                for (std::size_t q = 0; q < request_vector.size(); ++q)
                    request_index[request_vector[q]] = q;
                WeightedDigraph request_graph(request_vector.size());
                for (const auto& [source_node, target_node, a, b] :
                     requested_image.edges) {
                    (void)a; (void)b;
                    request_graph.add_edge(
                        request_index.at(source_node), request_index.at(target_node), 1);
                }
                std::vector<std::size_t> endpoint_for(
                    request_graph.n, upper.nodes.size());
                std::queue<std::size_t> reverse_frontier;
                for (const auto target : upper_components[candidate]) {
                    const auto target_found = request_index.find(upper.nodes[target]);
                    if (target_found == request_index.end()) continue;
                    if (endpoint_for[target_found->second] == upper.nodes.size()) {
                        endpoint_for[target_found->second] = target;
                        reverse_frontier.push(target_found->second);
                    }
                }
                while (!reverse_frontier.empty()) {
                    const auto current = reverse_frontier.front();
                    reverse_frontier.pop();
                    for (const auto& [source, weight] : request_graph.in_adj[current]) {
                        (void)weight;
                        if (endpoint_for[source] != upper.nodes.size()) continue;
                        endpoint_for[source] = endpoint_for[current];
                        reverse_frontier.push(source);
                    }
                }
                for (const auto lower_vertex : lower_vertices) {
                    const auto shadow = append_zero(lower.nodes[lower_vertex]);
                    const auto found = request_index.find(shadow);
                    if (found == request_index.end() ||
                        endpoint_for[found->second] == upper.nodes.size()) {
                        all_enter = false;
                        break;
                    }
                    trial.push_back(endpoint_for[found->second]);
                }
                if (all_enter) {
                    matching_component = candidate;
                    embedded = std::move(trial);
                    std::printf(
                        "ADJ_FAMILY_ENTRANCE n=%zu lower_scc=%zu upper_scc=%zu "
                        "mode=transient_to_recurrent\n",
                        D, lower_component, candidate);
                    break;
                }
            }
        }
        if (matching_component == upper_components.size()) {
            std::printf(
                "ADJ_COMP_PROJECT n=%zu lower_scc=%zu NO_UPPER_FAMILY "
                "deferred_to_boundary_exhaustion=YES\n",
                D, lower_component);
            // A failed individual family request is not itself a proof failure.
            // Its rejected grade-two shadows are seeds of the replayed boundary
            // theorem below, which must classify or dominate every recurrent
            // successor before the whole adjacent-dimension run can pass.
            continue;
        }

        // Reuse the same proof checks as the legacy path, specialized to the
        // request-derived upper family.
        const auto& upper_vertices = upper_components[matching_component];
        for (const auto vertex : upper_vertices)
            known_upper_family_nodes.insert(upper.nodes[vertex]);
        std::map<std::size_t, std::size_t> upper_global_to_local;
        const auto upper_owner = truth_projection_owner(upper);
        const auto lower_owner = truth_projection_owner(lower);
        const auto upper_view = project_corona_vertices(
            upper_owner, upper_vertices);
        const auto lower_view = project_corona_vertices(
            lower_owner, lower_vertices);
        const auto upper_graph = upper_view.materialize();
        const auto lower_graph = lower_view.materialize();
        for (const auto vertex : upper_vertices)
            upper_global_to_local[vertex] = upper_view.local(vertex);

        bool paths = true;
        std::size_t direct = 0, substituted = 0, maximum_path = 0, edges = 0;
        for (std::size_t source = 0; source < lower_graph.n; ++source)
            for (const auto& [target, weight] : lower_graph.out_adj[source])
                for (long long copy = 0; copy < weight; ++copy) {
                    (void)copy;
                    ++edges;
                    const auto upper_source =
                        upper_global_to_local.at(embedded[source]);
                    const auto upper_target =
                        upper_global_to_local.at(embedded[target]);
                    const auto path = shortest_path_local(
                        upper_graph, upper_source, upper_target);
                    if (path.empty()) { paths = false; continue; }
                    const auto length = path.size() - 1;
                    maximum_path = std::max(maximum_path, length);
                    if (length == 1) ++direct; else ++substituted;
                }

        using Role = ravel::proof::FaceRelativeSignGrammar;
        std::map<Role, std::size_t> lower_role_ids, upper_role_ids;
        std::vector<std::size_t> lower_roles(lower_graph.n),
                                 upper_roles(upper_graph.n);
        const auto role_of = [](const auto& node) {
            const std::vector<long long> x(node.x.begin(), node.x.end());
            return ravel::proof::derive_face_relative_sign_grammar(
                x, static_cast<std::size_t>(node.i),
                static_cast<std::size_t>(node.j), false);
        };
        for (std::size_t k = 0; k < lower_graph.n; ++k) {
            const auto role = role_of(lower.nodes[lower_vertices[k]]);
            lower_roles[k] = lower_role_ids.emplace(
                role, lower_role_ids.size()).first->second;
        }
        for (std::size_t k = 0; k < upper_graph.n; ++k) {
            const auto role = role_of(upper.nodes[upper_vertices[k]]);
            upper_roles[k] = upper_role_ids.emplace(
                role, upper_role_ids.size()).first->second;
        }
        std::map<std::size_t, std::size_t> role_embedding;
        bool role_well_defined = true;
        for (std::size_t k = 0; k < lower_graph.n; ++k) {
            const auto lower_role = lower_roles[k];
            const auto upper_role = upper_roles[
                upper_global_to_local.at(embedded[k])];
            const auto [found, inserted] = role_embedding.emplace(
                lower_role, upper_role);
            if (!inserted && found->second != upper_role)
                role_well_defined = false;
        }
        std::set<std::size_t> role_image;
        bool injective = true;
        for (const auto& [lower_role, upper_role] : role_embedding) {
            (void)lower_role;
            if (!role_image.insert(upper_role).second) injective = false;
        }
        bool profiles = true;
        std::map<std::size_t, std::set<std::vector<long long>>>
            lower_profiles, macro_profiles;
        for (std::size_t source = 0; source < lower_graph.n; ++source) {
            std::vector<long long> lower_profile(lower_role_ids.size(), 0);
            std::vector<long long> macro_profile(upper_role_ids.size(), 0);
            for (const auto& [target, weight] : lower_graph.out_adj[source]) {
                lower_profile[lower_roles[target]] += weight;
                macro_profile[upper_roles[
                    upper_global_to_local.at(embedded[target])]] += weight;
            }
            lower_profiles[lower_roles[source]].insert(lower_profile);
            macro_profiles[upper_roles[
                upper_global_to_local.at(embedded[source])]].insert(
                    macro_profile);
        }
        for (const auto& [role, family] : lower_profiles) {
            std::set<std::vector<long long>> expected;
            for (const auto& profile : family) {
                std::vector<long long> transported(upper_role_ids.size(), 0);
                for (std::size_t target_role = 0;
                     target_role < profile.size(); ++target_role)
                    transported[role_embedding.at(target_role)] +=
                        profile[target_role];
                expected.insert(std::move(transported));
            }
            if (expected != macro_profiles[role_embedding.at(role)])
                profiles = false;
        }

        // Compose the concrete competitor path simulation with the predicted-core
        // path simulation and the generic twisted Bellman theorem.  This is the
        // self-explanatory proof boundary: each premise is reconstructed from
        // literal states and edges, and the final certificate names the first
        // premise that fails instead of collapsing to a bare boolean.
        const auto& lower_core_vertices = lower_components[lower_core];
        const auto& upper_core_vertices = upper_components[upper_core];
        const auto lower_core_view = project_corona_vertices(
            lower_owner, lower_core_vertices);
        const auto upper_core_view = project_corona_vertices(
            upper_owner, upper_core_vertices);
        const auto lower_core_graph = lower_core_view.materialize();
        const auto upper_core_graph = upper_core_view.materialize();

        std::map<std::size_t, std::size_t> upper_core_global_to_local;
        for (const auto vertex : upper_core_vertices)
            upper_core_global_to_local[vertex] = upper_core_view.local(vertex);

        std::map<Role, std::size_t> lower_shared_role_ids, upper_shared_role_ids;
        std::vector<std::size_t> lower_comp_shared(lower_graph.n),
                                 lower_core_shared(lower_core_graph.n),
                                 upper_comp_embedded(lower_graph.n),
                                 upper_core_embedded(lower_core_graph.n);
        auto shared_role = [](const auto& node) {
            const std::vector<long long> x(node.x.begin(), node.x.end());
            return ravel::proof::derive_face_relative_sign_grammar(
                x, static_cast<std::size_t>(node.i),
                static_cast<std::size_t>(node.j), false);
        };
        for (std::size_t k = 0; k < lower_graph.n; ++k)
            lower_comp_shared[k] = lower_shared_role_ids.emplace(
                shared_role(lower.nodes[lower_vertices[k]]),
                lower_shared_role_ids.size()).first->second;
        for (std::size_t k = 0; k < lower_core_graph.n; ++k)
            lower_core_shared[k] = lower_shared_role_ids.emplace(
                shared_role(lower.nodes[lower_core_vertices[k]]),
                lower_shared_role_ids.size()).first->second;
        for (std::size_t k = 0; k < upper_graph.n; ++k)
            (void)upper_shared_role_ids.emplace(
                shared_role(upper.nodes[upper_vertices[k]]),
                upper_shared_role_ids.size());
        for (std::size_t k = 0; k < upper_core_graph.n; ++k)
            (void)upper_shared_role_ids.emplace(
                shared_role(upper.nodes[upper_core_vertices[k]]),
                upper_shared_role_ids.size());

        bool core_shadow_total = true;
        bool core_paths = true;
        std::size_t core_maximum_path = 0;
        for (std::size_t k = 0; k < lower_graph.n; ++k) {
            const auto& upper_node = upper.nodes[embedded[k]];
            upper_comp_embedded[k] = upper_shared_role_ids.at(
                shared_role(upper_node));
        }
        std::vector<std::size_t> embedded_core_global(lower_core_graph.n,
                                                       upper.nodes.size());
        const std::set<std::size_t> upper_core_set(
            upper_core_vertices.begin(), upper_core_vertices.end());
        for (std::size_t k = 0; k < lower_core_graph.n; ++k) {
            const auto transported = append_zero(
                lower.nodes[lower_core_vertices[k]]);
            const auto found = upper.index.find(transported);
            if (found == upper.index.end() ||
                upper_core_set.count(found->second) == 0) {
                core_shadow_total = false;
                continue;
            }
            embedded_core_global[k] = found->second;
            upper_core_embedded[k] = upper_shared_role_ids.at(
                shared_role(upper.nodes[found->second]));
        }
        if (core_shadow_total) {
            for (std::size_t source = 0; source < lower_core_graph.n; ++source)
                for (const auto& [target, weight] :
                     lower_core_graph.out_adj[source])
                    for (long long copy = 0; copy < weight; ++copy) {
                        (void)copy;
                        const auto us = upper_core_global_to_local.at(
                            embedded_core_global[source]);
                        const auto ut = upper_core_global_to_local.at(
                            embedded_core_global[target]);
                        const auto path = shortest_path_local(
                            upper_core_graph, us, ut);
                        if (path.empty()) { core_paths = false; continue; }
                        core_maximum_path = std::max(
                            core_maximum_path, path.size() - 1);
                    }
        }

        std::vector<std::size_t> shared_embedding(
            lower_shared_role_ids.size(), upper_shared_role_ids.size());
        bool shared_embedding_total = true;
        auto assign_shared_embedding = [&](std::size_t lower_role,
                                           std::size_t upper_role) {
            if (shared_embedding[lower_role] == upper_shared_role_ids.size())
                shared_embedding[lower_role] = upper_role;
            else if (shared_embedding[lower_role] != upper_role)
                shared_embedding_total = false;
        };
        for (std::size_t k = 0; k < lower_graph.n; ++k)
            assign_shared_embedding(lower_comp_shared[k],
                                    upper_comp_embedded[k]);
        if (core_shadow_total)
            for (std::size_t k = 0; k < lower_core_graph.n; ++k)
                assign_shared_embedding(lower_core_shared[k],
                                        upper_core_embedded[k]);
        for (const auto q : shared_embedding)
            if (q == upper_shared_role_ids.size())
                shared_embedding_total = false;

        const auto lower_grammar = ravel::proof::derive_phase_bellman_grammar(
            dense_matrix(lower_graph), lower_comp_shared,
            dense_matrix(lower_core_graph), lower_core_shared,
            lower_shared_role_ids.size());
        const auto upper_macro_grammar =
            ravel::proof::derive_phase_bellman_grammar(
                dense_matrix(lower_graph), upper_comp_embedded,
                dense_matrix(lower_core_graph), upper_core_embedded,
                upper_shared_role_ids.size());
        const std::vector<int> zero_sheet(
            lower_shared_role_ids.size(), 0);
        const auto adjacent_renewal =
            ravel::proof::derive_adjacent_twisted_renewal(
                D, edges, maximum_path, core_maximum_path,
                paths, profiles, core_shadow_total, core_paths,
                shared_embedding_total, lower_grammar, upper_macro_grammar,
                shared_embedding, zero_sheet, 256);

        const bool pass = role_well_defined && injective &&
            adjacent_renewal.proved;
        ++families;
        all = all && pass;
        std::printf(
            "ADJ_COMP_PROJECT n=%zu->%zu lower_scc=%zu "
            "lower_states=%zu request_nodes=%zu projected_nodes=%zu "
            "upper_states=%zu edges=%zu direct=%zu substituted=%zu "
            "max_path=%zu role_wd=%s injective=%s profiles=%s PASS=%s\n",
            D, D + 1, lower_component, lower_graph.n,
            requested_image.nodes.size(), projected_trace.final_nodes.size(),
            upper_graph.n, edges, direct, substituted, maximum_path,
            role_well_defined ? "YES" : "NO",
            injective ? "YES" : "NO", profiles ? "YES" : "NO",
            pass ? "YES" : "NO");
        std::printf(
            "  ADJ_RENEWAL_EXPLAIN core_shadow=%s core_paths=%s "
            "core_max_path=%zu shared_role_map=%s twisted_core=%s "
            "bellman_profiles=(competitor:%zu,core:%zu) "
            "renewal_block=%zu inherited=%s obstruction=%s\n",
            core_shadow_total ? "TOTAL" : "FAIL",
            core_paths ? "PASS" : "FAIL", core_maximum_path,
            shared_embedding_total ? "TOTAL" : "AMBIGUOUS",
            adjacent_renewal.predicted_core_twist_proved ? "PASS" : "FAIL",
            adjacent_renewal.bellman.competitor_profiles_checked,
            adjacent_renewal.bellman.core_profiles_checked,
            adjacent_renewal.bellman.renewal_block,
            adjacent_renewal.proved ? "PASS" : "FAIL",
            adjacent_renewal.obstruction.empty() ? "none" :
                adjacent_renewal.obstruction.c_str());
    }
    stage("grade_two_boundary_closure");
    ravel::proof::RejectedBoundaryExhaustionProof boundary_proof;
    std::vector<SNode<D + 1>> boundary_replay_nodes;
    if (rejected_grade_two.empty()) {
        WeightedDigraph empty(0);
        boundary_proof = ravel::proof::derive_rejected_boundary_exhaustion(
            D + 1, 0, empty, true, true, 0, 0,
            [](std::size_t) { return false; });
    } else {
        CoronaProjectionRequest<D + 1> boundary_request;
        boundary_request.seeds.assign(rejected_grade_two.begin(), rejected_grade_two.end());
        boundary_request.node_cap = 1000000;
        boundary_request.operations =
            static_cast<unsigned>(CoronaSurfaceOperation::simple_forward) |
            static_cast<unsigned>(CoronaSurfaceOperation::simple_backward);
        boundary_request.accept = [&](const SNode<D + 1>& node) {
            const auto atom_witness = derive_corona_atom_witness(node);
            if (!atom_witness) return false;
            const std::vector<long long> x(node.x.begin(), node.x.end());
            // The witness extractor recognizes decompositions that may
            // collapse to a grade-one atom through cancellation.  The
            // boundary theorem consumes only genuine grade-two seeds.
            return upper_catalogue.grade(x) == 2;
        };
        const auto boundary_image = upper_surface.project(boundary_request);
        auto boundary_nodes = boundary_image.nodes;
        boundary_nodes.insert(known_upper_family_nodes.begin(), known_upper_family_nodes.end());
        const auto boundary_trace = algorithm2_projected_trace<D + 1>(
            upper_input.substitution, upper_input.contact,
            [&](const SNode<D + 1>& node) {
                const std::vector<long long> x(node.x.begin(), node.x.end());
                return upper_catalogue.grade(x) == 1 || boundary_nodes.count(node) != 0;
            },
            CoronaConnectorPolicy::evolving_layer, 10, 1000000,
            CoronaEdgeArithmetic::fast_rounded);
        const auto boundary_replay =
            ravel::proof::derive_replayed_boundary_graph(
                upper_input.substitution, boundary_trace);
        boundary_replay_nodes = boundary_replay.nodes;
        boundary_proof = ravel::proof::derive_rejected_boundary_exhaustion(
            D + 1, rejected_grade_two.size(), boundary_replay.graph,
            boundary_replay.closure_complete,
            boundary_replay.replay_evidence_valid,
            boundary_replay.replay_edges,
            boundary_replay.replay_witnesses,
            [&](std::size_t vertex) {
                const auto& node = boundary_replay.nodes.at(vertex);
                const auto pair = nbonacci_margin::LabelPair{
                    static_cast<std::size_t>(node.i),
                    static_cast<std::size_t>(node.j)};
                const std::vector<long long> x(node.x.begin(), node.x.end());
                return known_upper_family_nodes.count(node) != 0 ||
                       nbonacci_margin::predicted_core_member(D + 1, pair, x);
            });
        boundary_proof = ravel::proof::derive_replayed_boundary_symbolic_partition(
            std::move(boundary_proof), upper_input.substitution, boundary_replay,
            [&](const SNode<D + 1>& node) {
                const auto pair = nbonacci_margin::LabelPair{
                    static_cast<std::size_t>(node.i),
                    static_cast<std::size_t>(node.j)};
                const std::vector<long long> x(node.x.begin(), node.x.end());
                return known_upper_family_nodes.count(node) != 0 ||
                       nbonacci_margin::predicted_core_member(D + 1, pair, x);
            });
        boundary_proof = ravel::proof::derive_replayed_boundary_dominance(
            std::move(boundary_proof), boundary_replay,
            [&](const SNode<D + 1>& node) {
                const auto pair = nbonacci_margin::LabelPair{
                    static_cast<std::size_t>(node.i),
                    static_cast<std::size_t>(node.j)};
                const std::vector<long long> x(node.x.begin(), node.x.end());
                return nbonacci_margin::predicted_core_member(
                    D + 1, pair, x);
            }, 160);
        boundary_proof = ravel::proof::derive_replayed_boundary_phase_twist(
            std::move(boundary_proof), upper_input.substitution, boundary_replay);
    }
    stage_done("grade_two_boundary_closure", boundary_proof.closure_vertices);
    std::printf("%s", ravel::proof::render_rejected_boundary_exhaustion_report(boundary_proof).c_str());
    if (!boundary_proof.proved && !rejected_grade_two.empty()) {
        for (const auto& witness : boundary_proof.witnesses) {
            if (witness.known_transported_family || witness.terminal_permutation ||
                witness.strictly_dominated) continue;
            std::printf("UNKNOWN_BOUNDARY_COMPONENT component=%zu states=%zu\n",
                        witness.component, witness.vertex_indices.size());
            for (const auto vertex : witness.vertex_indices) {
                const auto& node = boundary_replay_nodes.at(vertex);
                const std::vector<long long> x(node.x.begin(), node.x.end());
                std::printf("  STATE i=%lld j=%lld grade=%zu x=",
                            node.i, node.j, upper_catalogue.grade(x));
                for (std::size_t k = 0; k < x.size(); ++k)
                    std::printf("%s%lld", k == 0 ? "[" : ",", x[k]);
                std::printf("] predicted_core=%s known_family=%s\n",
                    nbonacci_margin::predicted_core_member(
                        D + 1,
                        {static_cast<std::size_t>(node.i),
                         static_cast<std::size_t>(node.j)}, x) ? "YES" : "NO",
                    known_upper_family_nodes.count(node) ? "YES" : "NO");
            }
        }
    }
    all = all && boundary_proof.proved;
    std::printf(
        "ADJ_COMP_PROJECT_SUMMARY n=%zu->%zu families=%zu boundary=%s result=%s\n",
        D, D + 1, families, boundary_proof.proved ? "PASS" : "FAIL",
        all ? "PASS" : "FAIL");
    return all;
}

template <std::size_t D>
bool run_adjacent_competitor_transport(CoronaExecutionMode mode) {
    return mode == CoronaExecutionMode::legacy_materialized
        ? run_adjacent_competitor_transport_legacy<D>()
        : run_adjacent_competitor_transport_projected<D>();
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

bool build_cache_only(std::size_t n) {
    switch (n) {
        case 6:
            (void)load_or_build_full_graph<6>();
            (void)load_or_build_full_graph<7>();
            return true;
        default:
            std::fprintf(stderr,
                         "cache-only mode currently supports n=6 (and its n=7 upper cache)\n");
            return false;
    }
}

}  // namespace

int main(int argc, char** argv) {
    install_failure_reporting();
    std::setvbuf(stdout, nullptr, _IOLBF, 0);
    std::setvbuf(stderr, nullptr, _IOLBF, 0);
    std::size_t n = 4;
    CoronaExecutionMode mode = default_corona_execution_mode();
    bool cache_only = false;
    for (int arg = 1; arg < argc; ++arg) {
        const std::string value = argv[arg];
        if (value == "--legacy-corona")
            mode = CoronaExecutionMode::legacy_materialized;
        else if (value == "--projected-corona")
            mode = CoronaExecutionMode::projected_surface;
        else if (value == "--cache-only")
            cache_only = true;
        else
            n = static_cast<std::size_t>(std::stoul(value));
    }
    std::printf("CORONA_MODE %s\n",
        mode == CoronaExecutionMode::projected_surface
            ? "projected" : "legacy");
    try {
        if (cache_only) return build_cache_only(n) ? 0 : 2;
        switch (n) {
            case 3: return run_adjacent_competitor_transport<3>(mode) ? 0 : 1;
            case 4: return run_adjacent_competitor_transport<4>(mode) ? 0 : 1;
            case 5: return run_adjacent_competitor_transport<5>(mode) ? 0 : 1;
            case 6: return run_adjacent_competitor_transport<6>(mode) ? 0 : 1;
            default: return 2;
        }
    } catch (const std::exception& ex) {
        std::fprintf(stderr,
            "UNCAUGHT_EXCEPTION stage=%s elapsed_ms=%lld peak_rss_kb=%lld what=%s\n",
            g_stage, elapsed_ms(), peak_rss_kb(), ex.what());
        return 70;
    } catch (...) {
        std::fprintf(stderr,
            "UNCAUGHT_EXCEPTION stage=%s elapsed_ms=%lld peak_rss_kb=%lld what=non_std_exception\n",
            g_stage, elapsed_ms(), peak_rss_kb());
        return 71;
    }
}
