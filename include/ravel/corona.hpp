// corona.hpp
//
// C-corona iteration and Algorithm 2 of arXiv:2511.16442, the final
// piece of the contact-boundary logic port (W10).  Together with
// ambient_graph.hpp this completes the Def 3.1 → 3.5 → 3.9 pipeline.
//
// Algorithm 2:
//   A[1] = Ghat_C = ±C (the signed contact graph, C ∪ ±C, including
//                       the identity elements [k, 0, k] for k in A).
//   For p >= 2:
//     corona_nodes = C-Corona(A[p-1])  (Def 3.9)
//     edges = simple-forward-edges on corona_nodes
//     A[p] = Red(corona_nodes, edges)    (remove out-degree-0 nodes)
//   Until A[p] == A[p-1].
//
// IMPORTANT bug fix (per the reference's "Tale of Pair- vs Triple-level
// Negation"): the ⊟-type test (pair-level negation, SAME letter, negate
// only the vector) is a DIFFERENT operation from the ⊖-type test
// (triple-level negation, swap-and-negate, used for the unrestricted
// ±-node space). Conflating them makes Algorithm 2 diverge to 139
// nodes for σ_1 instead of converging at 26.  The same_letter_H
// helper below implements the correct ⊟-type test.

#pragma once

#include <array>
#include <deque>
#include <functional>
#include <optional>
#include <string>
#include <string_view>
#include <cstddef>
#include <map>
#include <cstdint>
#include <cstdlib>
#include <set>
#include <stdexcept>
#include <unordered_set>
#include <vector>
#include <utility>
#include <algorithm>

#include "ravel/core.hpp"
#include "ravel/ambient_graph.hpp"
#include "ravel/faces.hpp"
#include "ravel/nbonacci_margin_invariant.hpp"

namespace ravel {

// A simple-graph node is a triple (i, x, j) in the unrestricted
// ±-node space (i.e. the unsigned or signed D; both are valid
// per-face nodes here, not requiring the (i < j) anti-symmetrization).
template <std::size_t d>
struct SNode {
    long long i;
    std::array<long long, d> x;
    long long j;
    // The signed-mirror: negate x, swap i and j.
    SNode<d> mirror() const {
        SNode<d> r;
        r.i = j;
        r.j = i;
        for (std::size_t k = 0; k < d; ++k) r.x[k] = -x[k];
        return r;
    }
    // Equality (used for unordered_set and "is the same node" tests).
    bool operator==(const SNode& o) const { return i == o.i && x == o.x && j == o.j; }
};

// Hash for unordered_set<SNode>.
template <std::size_t d>
struct SNodeHash {
    std::size_t operator()(const SNode<d>& s) const noexcept {
        std::size_t h = static_cast<std::size_t>(s.i) * 131071
                          + static_cast<std::size_t>(s.j) * 17;
        for (std::size_t k = 0; k < d; ++k) h = h * 31 + static_cast<std::size_t>(s.x[k]);
        return h;
    }
};

// is_valid_simple_node per Def 3.5: [i, x, j] is in ±(A x H_sigma) iff
//   x != 0  OR  i != j        (i.e. not the trivial [i, 0, i] self-element)
// AND ( [x, j] in H_sigma  OR  [-x, i] in H_sigma )
// (the OR is the SIGNED-face validity: either face [x, j] OR its
// triple-negation mirror [-x, i] is in H_sigma).
template <std::size_t d>
bool is_valid_simple_node(const Substitution<d>& subst,
                          const SNode<d>& node) {
    bool trivial = true;
    for (std::size_t k = 0; k < d; ++k) if (node.x[k] != 0) trivial = false;
    if (trivial && node.i == node.j) return false;
    SNode<d> mirror = node.mirror();
    return subst.in_H_sigma(node.x, static_cast<std::size_t>(node.j))
        || subst.in_H_sigma(mirror.x, static_cast<std::size_t>(mirror.j));
}

// same_letter_H per the reference's "Tale of Pair- vs Triple-level
// Negation": [x, j] in H_sigma UNION (-H_sigma), where -H_sigma here
// is the PAIR-level negation (SAME letter, negate only the vector).
// -H_sigma = {[x', j] : [-x', j] in H_sigma}.
template <std::size_t d>
bool same_letter_H(const Substitution<d>& subst,
                    const std::array<long long, d>& x,
                    std::size_t j) {
    if (subst.in_H_sigma(x, j)) return true;
    std::array<long long, d> negx{};
    for (std::size_t k = 0; k < d; ++k) negx[k] = -x[k];
    return subst.in_H_sigma(negx, j);
}

// Simple forward targets (Def 3.5, TYPE 1 ONLY).
// [i, x, j] -l(p1)|l(q1)-> [i', x', j'] if exist (p1,i,s1), (q1,j,t1) in P
// with sigma(i') = p1 i s1, sigma(j') = q1 j t1, M x' = x + l(q1) - l(p1).
// Destination filtered by is_valid_simple_node (NOT by the D anti-sym
// rule: we're in unrestricted +- space now).
//
// **"fast and messy"** version: uses `solve_Mx_eq_rhs` (double
// precision, 1e-12/1e-9 tolerance + llround snap).  Cheap, no
// heap allocation, matches the σ_1 / σ_2 / Tribonacci / Tetrabonacci
// surveys bit-exactly.  For near-Salem / "EXPLODED" candidates
// where the tolerance window is the actual bug, use
// `simple_forward_targets_exact` (below) instead.
template <std::size_t d>
std::vector<std::pair<SNode<d>, std::pair<std::vector<long long>,
                                          std::vector<long long>>>>
simple_forward_targets(const Substitution<d>& subst, const SNode<d>& node) {
    std::vector<std::pair<SNode<d>, std::pair<std::vector<long long>,
                                          std::vector<long long>>>> out;
    auto parents_i = parent_decompositions<d>(subst.images, node.i);
    auto parents_j = parent_decompositions<d>(subst.images, node.j);
    for (const auto& pd : parents_i) {
        auto lp1 = abelianization<d>(pd.p);
        for (const auto& qd : parents_j) {
            auto lq1 = abelianization<d>(qd.p);
            std::array<double, d> rhs;
            for (std::size_t k = 0; k < d; ++k)
                rhs[k] = static_cast<double>(node.x[k])
                       + lq1[k] - lp1[k];
            auto xprime_opt = solve_Mx_eq_rhs<d>(subst.M, rhs);
            if (!xprime_opt.has_value()) continue;
            auto xprime = *xprime_opt;
            SNode<d> cand;
            cand.i = pd.parent_letter;
            cand.x = xprime;
            cand.j = qd.parent_letter;
            if (!is_valid_simple_node<d>(subst, cand)) continue;
            out.push_back({cand, {pd.p, qd.p}});
        }
    }
    return out;
}

// **"checking"** version: same Def 3.5 type-1 construction, but
// uses `solve_Mx_eq_rhs_exact` (mpq_t Gaussian elimination, no
// tolerance) and feeds integer rhs throughout.  Bit-exact; the
// diff between this and the messy version, on any given input,
// is exactly the tolerance window where the messy version is
// silently lying.  Use as a slow reference for verifying
// suspect contact-boundary candidates, or as the default for
// near-Salem Pisot substitutions where the messy version is
// known to misclassify.
template <std::size_t d>
std::vector<std::pair<SNode<d>, std::pair<std::vector<long long>,
                                          std::vector<long long>>>>
simple_forward_targets_exact(const Substitution<d>& subst, const SNode<d>& node) {
    std::vector<std::pair<SNode<d>, std::pair<std::vector<long long>,
                                          std::vector<long long>>>> out;
    auto parents_i = parent_decompositions<d>(subst.images, node.i);
    auto parents_j = parent_decompositions<d>(subst.images, node.j);
    for (const auto& pd : parents_i) {
        auto lp1 = abelianization<d>(pd.p);
        for (const auto& qd : parents_j) {
            auto lq1 = abelianization<d>(qd.p);
            std::array<long long, d> rhs;
            for (std::size_t k = 0; k < d; ++k)
                rhs[k] = node.x[k] + lq1[k] - lp1[k];
            auto xprime_opt = solve_Mx_eq_rhs_exact<d>(subst.M, rhs);
            if (!xprime_opt.has_value()) continue;
            auto xprime = *xprime_opt;
            SNode<d> cand;
            cand.i = pd.parent_letter;
            cand.x = xprime;
            cand.j = qd.parent_letter;
            if (!is_valid_simple_node<d>(subst, cand)) continue;
            out.push_back({cand, {pd.p, qd.p}});
        }
    }
    return out;
}

// Simple backward targets (Def 3.5, INDEPENDENTLY re-derived).
// Given (i', x', j'), find all (i, x, j) with a Def-3.5 type-1 edge
// into it.  x = M x' + l(p1) - l(q1) (no division, exact integer).
template <std::size_t d>
std::vector<std::pair<SNode<d>, std::pair<std::vector<long long>,
                                          std::vector<long long>>>>
simple_backward_targets(const Substitution<d>& subst, const SNode<d>& node) {
    std::vector<std::pair<SNode<d>, std::pair<std::vector<long long>,
                                          std::vector<long long>>>> out;
    auto Mxp = std::array<long long, d>{};
    for (std::size_t i = 0; i < d; ++i) {
        long long s = 0;
        for (std::size_t j = 0; j < d; ++j) s += subst.M[i][j] * node.x[j];
        Mxp[i] = s;
    }
    for (std::size_t pi = 0; pi < subst.images[node.i].size(); ++pi) {
        std::vector<long long> p1(subst.images[node.i].begin(),
                                  subst.images[node.i].begin() + pi);
        auto lp1 = abelianization<d>(p1);
        for (std::size_t pj = 0; pj < subst.images[node.j].size(); ++pj) {
            std::vector<long long> q1(subst.images[node.j].begin(),
                                      subst.images[node.j].begin() + pj);
            auto lq1 = abelianization<d>(q1);
            SNode<d> cand;
            cand.i = subst.images[node.i][pi];
            cand.j = subst.images[node.j][pj];
            for (std::size_t k = 0; k < d; ++k)
                cand.x[k] = Mxp[k] + lp1[k] - lq1[k];
            if (!is_valid_simple_node<d>(subst, cand)) continue;
            out.push_back({cand, {p1, q1}});
        }
    }
    return out;
}

// Build the signed contact set: ±C = C ∪ ±C where each node's mirror
// is its triple-negation.  Returns a std::set<SNode> with a per-instance
// comparator (lexicographic by (i, x, j)).  Note: using std::set here
// to be more portable than unordered_set<SNode> (which requires
// custom hash + equality definition).
template <std::size_t d>
std::set<SNode<d>> build_signed_contact_set(const std::set<SNode<d>>& C) {
    std::set<SNode<d>> out;
    for (const auto& c : C) {
        out.insert(c);
        out.insert(c.mirror());
    }
    return out;
}

// C-Corona (Def 3.9): {[i1, y+delta, i2] : exists [i1, y, j] in A_prev
// with same_letter_H(y, j), exists [j, delta, i2] in ±C U {identity},
// not (i1 == i2 AND y+delta == 0), same_letter_H(y+delta, i2)}.
// The same_letter_H fix (both endpoints) is essential.
template <std::size_t d>
std::set<SNode<d>> c_corona(const Substitution<d>& subst,
                          const std::set<SNode<d>>& A_prev,
                          const std::set<SNode<d>>& pmC) {
    std::set<SNode<d>> out;
    // Build hop set: ±C union identity elements [k, 0, k].
    std::set<std::tuple<long long, std::array<long long, d>, long long>> hops;
    for (const auto& c : pmC) {
        hops.emplace(c.i, c.x, c.j);
    }
    for (std::size_t k = 0; k < d; ++k) {
        std::array<long long, d> zero{};
        hops.emplace(static_cast<long long>(k), zero, static_cast<long long>(k));
    }
    // Index hops by starting color j for fast iteration.
    std::map<long long, std::vector<std::pair<std::array<long long, d>,
                                              long long>>> by_j;
    for (const auto& [jcol, x, i2] : hops) {
        by_j[jcol].push_back({x, i2});
    }
    // For each [i1, y, j] in A_prev, hop over each [j, delta, i2] in hops.
    for (const auto& [i1, y, j] : A_prev) {
        if (!same_letter_H<d>(subst, y, static_cast<std::size_t>(j))) continue;
        auto it = by_j.find(j);
        if (it == by_j.end()) continue;
        for (const auto& [delta, i2] : it->second) {
            SNode<d> cand;
            cand.i = i1;
            for (std::size_t k = 0; k < d; ++k) cand.x[k] = y[k] + delta[k];
            cand.j = i2;
            // Exclude trivial [i, 0, i].
            bool trivial = true;
            for (std::size_t k = 0; k < d; ++k) if (cand.x[k] != 0) trivial = false;
            if (trivial && cand.i == cand.j) continue;
            // same_letter_H fix on the destination.
            if (!same_letter_H<d>(subst, cand.x,
                                 static_cast<std::size_t>(cand.j))) continue;
            out.insert(cand);
        }
    }
    return out;
}


enum class CoronaConnectorPolicy {
    fixed_signed_contact,
    evolving_layer,
};

enum class CoronaEdgeArithmetic {
    fast_rounded,
    exact_rational,
};


// Streaming C-corona projection.  This is the same Def. 3.9 operation as
// c_corona, but the request predicate is applied before insertion.  It avoids
// materializing irrelevant candidates and records the rejected boundary for
// completeness audits.
template <std::size_t d, class Predicate>
std::set<SNode<d>> c_corona_projected(
        const Substitution<d>& subst,
        const std::set<SNode<d>>& A_prev,
        const std::set<SNode<d>>& connectors,
        Predicate accept,
        std::set<SNode<d>>* rejected_boundary = nullptr) {
    std::set<SNode<d>> out;
    std::map<long long, std::vector<std::pair<std::array<long long, d>,
                                              long long>>> by_start;
    for (const auto& connector : connectors)
        by_start[connector.i].push_back({connector.x, connector.j});
    for (std::size_t k = 0; k < d; ++k) {
        std::array<long long, d> zero{};
        by_start[static_cast<long long>(k)].push_back(
            {zero, static_cast<long long>(k)});
    }
    for (const auto& source : A_prev) {
        if (!same_letter_H<d>(subst, source.x,
                              static_cast<std::size_t>(source.j)))
            continue;
        const auto found = by_start.find(source.j);
        if (found == by_start.end()) continue;
        for (const auto& [delta, target_letter] : found->second) {
            SNode<d> candidate;
            candidate.i = source.i;
            candidate.j = target_letter;
            for (std::size_t k = 0; k < d; ++k)
                candidate.x[k] = source.x[k] + delta[k];
            bool trivial = candidate.i == candidate.j;
            for (const auto x : candidate.x) trivial = trivial && x == 0;
            if (trivial || !same_letter_H<d>(
                    subst, candidate.x,
                    static_cast<std::size_t>(candidate.j)))
                continue;
            if (accept(candidate)) out.insert(candidate);
            else if (rejected_boundary != nullptr)
                rejected_boundary->insert(candidate);
        }
    }
    return out;
}

template <std::size_t d>
struct ProjectedCoronaLayer {
    int round = 1;
    std::set<SNode<d>> input_nodes;
    std::set<SNode<d>> pre_red_nodes;
    std::set<SNode<d>> nodes;
    std::set<SNode<d>> rejected_boundary;
    // Canonical two-atom evidence is attached when candidates are generated
    // and retained for pre-Red nodes, survivors, rejected nodes, and pruned
    // ranks.  Absence is meaningful and must be handled explicitly by proof
    // consumers rather than reconstructed silently after SCC discovery.
    std::map<SNode<d>, nbonacci_margin::GradeTwoAtomWitness> atom_witnesses;
    std::vector<std::set<SNode<d>>> red_pruning_ranks;
    std::vector<std::tuple<SNode<d>, SNode<d>,
                           std::vector<long long>,
                           std::vector<long long>>> edges;
};

template <std::size_t d>
struct ProjectedCoronaTrace {
    CoronaConnectorPolicy connector_policy =
        CoronaConnectorPolicy::fixed_signed_contact;
    CoronaEdgeArithmetic edge_arithmetic =
        CoronaEdgeArithmetic::fast_rounded;
    std::set<SNode<d>> signed_contact;
    std::vector<ProjectedCoronaLayer<d>> layers;
    std::set<SNode<d>> final_nodes;
    std::set<SNode<d>> rejected_boundary;
    std::map<SNode<d>, nbonacci_margin::GradeTwoAtomWitness> atom_witnesses;
    bool converged = false;
    bool node_cap_hit = false;
    bool predicate_closed = false;
};

template <std::size_t d>
std::optional<nbonacci_margin::GradeTwoAtomWitness>
derive_corona_atom_witness(const SNode<d>& node) {
    return nbonacci_margin::derive_grade_two_atom_witness(
        std::vector<long long>(node.x.begin(), node.x.end()));
}

template <std::size_t d, class Predicate>
ProjectedCoronaTrace<d> algorithm2_projected_trace(
        const Substitution<d>& subst,
        const std::set<SNode<d>>& C,
        Predicate accept,
        CoronaConnectorPolicy policy =
            CoronaConnectorPolicy::fixed_signed_contact,
        int max_rounds = 50,
        std::size_t node_cap = 0,
        CoronaEdgeArithmetic edge_arithmetic =
            CoronaEdgeArithmetic::fast_rounded) {
    ProjectedCoronaTrace<d> trace;
    trace.connector_policy = policy;
    trace.edge_arithmetic = edge_arithmetic;
    trace.signed_contact = build_signed_contact_set<d>(C);
    std::set<SNode<d>> previous;
    for (const auto& node : trace.signed_contact) {
        if (const auto witness = derive_corona_atom_witness(node))
            trace.atom_witnesses.emplace(node, *witness);
        if (accept(node)) previous.insert(node);
        else trace.rejected_boundary.insert(node);
    }
    ProjectedCoronaLayer<d> initial;
    initial.round = 1;
    initial.nodes = previous;
    trace.layers.push_back(std::move(initial));

    for (int round = 2; round <= max_rounds; ++round) {
        const auto& connectors =
            policy == CoronaConnectorPolicy::fixed_signed_contact
                ? trace.signed_contact : previous;
        ProjectedCoronaLayer<d> layer;
        layer.round = round;
        layer.input_nodes = previous;
        layer.pre_red_nodes = c_corona_projected<d>(
            subst, previous, connectors, accept,
            &layer.rejected_boundary);
        trace.rejected_boundary.insert(layer.rejected_boundary.begin(),
                                       layer.rejected_boundary.end());
        for (const auto& node : layer.pre_red_nodes)
            if (const auto witness = derive_corona_atom_witness(node)) {
                layer.atom_witnesses.emplace(node, *witness);
                trace.atom_witnesses.emplace(node, *witness);
            }
        for (const auto& node : layer.rejected_boundary)
            if (const auto witness = derive_corona_atom_witness(node)) {
                layer.atom_witnesses.emplace(node, *witness);
                trace.atom_witnesses.emplace(node, *witness);
            }
        if (node_cap != 0 && layer.pre_red_nodes.size() > node_cap) {
            trace.node_cap_hit = true;
            trace.final_nodes = previous;
            trace.layers.push_back(std::move(layer));
            return trace;
        }
        for (const auto& source : layer.pre_red_nodes) {
            const auto targets =
                edge_arithmetic == CoronaEdgeArithmetic::exact_rational
                    ? simple_forward_targets_exact<d>(subst, source)
                    : simple_forward_targets<d>(subst, source);
            for (const auto& [target, witness] : targets)
                if (layer.pre_red_nodes.count(target) != 0)
                    layer.edges.push_back(
                        {source, target, witness.first, witness.second});
        }
        auto reduced = red_trace<d>(layer.pre_red_nodes, layer.edges);
        layer.nodes = std::move(reduced.survivors);
        layer.edges = std::move(reduced.survivor_edges);
        layer.red_pruning_ranks = std::move(reduced.pruning_ranks);
        const bool fixed = layer.nodes == previous;
        previous = layer.nodes;
        trace.layers.push_back(std::move(layer));
        if (fixed) {
            trace.converged = true;
            trace.final_nodes = previous;
            // The projection is globally predicate-closed exactly when no
            // generated corona candidate crossed the predicate boundary.
            trace.predicate_closed = trace.rejected_boundary.empty();
            return trace;
        }
    }
    trace.final_nodes = previous;
    trace.predicate_closed = trace.rejected_boundary.empty();
    return trace;
}

// Request-driven corona projection.
//
// The legacy Algorithm-2 implementation materializes every intermediate
// corona layer before a consumer can inspect a small recurrent family.  The
// surface API below instead treats the corona rules as an exact transition
// oracle.  A consumer supplies seeds, an admissibility predicate, and the
// closure operations relevant to its theorem.  The engine explores only that
// requested image and returns a certificate that the accepted region is
// closed under every requested operation.  Rejected boundary candidates are
// retained as evidence; hitting a cap makes the result explicitly incomplete.
//
// This is a general corona-layer capability, not a cache or proof-specific
// adapter.  Legacy materialization remains available for differential tests.
enum class CoronaExecutionMode {
    projected_surface,
    legacy_materialized,
};

inline CoronaExecutionMode corona_execution_mode_from_string(
        std::string_view value) {
    if (value == "legacy" || value == "materialized" || value == "full")
        return CoronaExecutionMode::legacy_materialized;
    if (value == "projected" || value == "surface" || value == "lazy" ||
        value.empty())
        return CoronaExecutionMode::projected_surface;
    throw std::invalid_argument("unknown corona execution mode: " +
                                std::string(value));
}

inline CoronaExecutionMode default_corona_execution_mode() {
    const char* value = std::getenv("RAVEL_CORONA_MODE");
    return corona_execution_mode_from_string(value == nullptr ? "projected" : value);
}

template <std::size_t d>
std::vector<SNode<d>> c_corona_targets_from_node(
        const Substitution<d>& subst,
        const SNode<d>& source,
        const std::set<SNode<d>>& connectors) {
    std::vector<SNode<d>> out;
    if (!same_letter_H<d>(subst, source.x,
                          static_cast<std::size_t>(source.j)))
        return out;
    for (const auto& connector : connectors) {
        if (connector.i != source.j) continue;
        SNode<d> candidate;
        candidate.i = source.i;
        candidate.j = connector.j;
        for (std::size_t k = 0; k < d; ++k)
            candidate.x[k] = source.x[k] + connector.x[k];
        bool trivial = candidate.i == candidate.j;
        for (const auto x : candidate.x) trivial = trivial && x == 0;
        if (trivial) continue;
        if (!same_letter_H<d>(subst, candidate.x,
                              static_cast<std::size_t>(candidate.j)))
            continue;
        out.push_back(candidate);
    }
    // Identity connectors are part of Def. 3.9 even when absent from ±C.
    SNode<d> identity = source;
    bool trivial = identity.i == identity.j;
    for (const auto x : identity.x) trivial = trivial && x == 0;
    if (!trivial) out.push_back(identity);
    std::sort(out.begin(), out.end());
    out.erase(std::unique(out.begin(), out.end()), out.end());
    return out;
}

template <std::size_t d>
std::vector<SNode<d>> c_corona_sources_to_node(
        const Substitution<d>& subst,
        const SNode<d>& target,
        const std::set<SNode<d>>& connectors) {
    std::vector<SNode<d>> out;
    if (!same_letter_H<d>(subst, target.x,
                          static_cast<std::size_t>(target.j)))
        return out;
    for (const auto& connector : connectors) {
        if (connector.j != target.j) continue;
        SNode<d> source;
        source.i = target.i;
        source.j = connector.i;
        for (std::size_t k = 0; k < d; ++k)
            source.x[k] = target.x[k] - connector.x[k];
        bool trivial = source.i == source.j;
        for (const auto x : source.x) trivial = trivial && x == 0;
        if (trivial) continue;
        if (!same_letter_H<d>(subst, source.x,
                              static_cast<std::size_t>(source.j)))
            continue;
        out.push_back(source);
    }
    // Reverse of the identity connector.
    bool trivial = target.i == target.j;
    for (const auto x : target.x) trivial = trivial && x == 0;
    if (!trivial) out.push_back(target);
    std::sort(out.begin(), out.end());
    out.erase(std::unique(out.begin(), out.end()), out.end());
    return out;
}

enum class CoronaSurfaceOperation : unsigned {
    simple_forward = 1u << 0,
    simple_backward = 1u << 1,
    corona_forward = 1u << 2,
    corona_backward = 1u << 3,
};

inline constexpr unsigned operator|(CoronaSurfaceOperation lhs,
                                    CoronaSurfaceOperation rhs) {
    return static_cast<unsigned>(lhs) | static_cast<unsigned>(rhs);
}

inline constexpr bool corona_surface_has(unsigned operations,
                                         CoronaSurfaceOperation operation) {
    return (operations & static_cast<unsigned>(operation)) != 0;
}

template <std::size_t d>
struct CoronaProjectionRequest {
    std::vector<SNode<d>> seeds;
    std::function<bool(const SNode<d>&)> accept =
        [](const SNode<d>&) { return true; };
    unsigned operations =
        static_cast<unsigned>(CoronaSurfaceOperation::simple_forward) |
        static_cast<unsigned>(CoronaSurfaceOperation::simple_backward);
    CoronaEdgeArithmetic edge_arithmetic = CoronaEdgeArithmetic::fast_rounded;
    std::size_t node_cap = 0;       // zero means no artificial cap
    std::size_t expansion_cap = 0;  // zero means no artificial cap
};

template <std::size_t d>
struct CoronaProjectionCertificate {
    std::size_t expanded_nodes = 0;
    std::size_t generated_candidates = 0;
    std::size_t rejected_boundary_candidates = 0;
    bool closed_under_requested_operations = false;
    bool node_cap_hit = false;
    bool expansion_cap_hit = false;
};

template <std::size_t d>
struct DerivedCoronaImage {
    std::set<SNode<d>> nodes;
    std::vector<std::tuple<SNode<d>, SNode<d>,
                           std::vector<long long>,
                           std::vector<long long>>> edges;
    std::set<SNode<d>> rejected_boundary;
    std::map<SNode<d>, nbonacci_margin::GradeTwoAtomWitness> atom_witnesses;
    CoronaProjectionCertificate<d> certificate;

    bool complete() const {
        return certificate.closed_under_requested_operations &&
               !certificate.node_cap_hit &&
               !certificate.expansion_cap_hit;
    }
};

template <std::size_t d>
class CoronaSurface {
public:
    CoronaSurface(const Substitution<d>& subst,
                  const std::set<SNode<d>>& contact)
        : subst_(&subst), signed_contact_(build_signed_contact_set<d>(contact)) {}

    CoronaSurface(const Substitution<d>& subst,
                  std::set<SNode<d>> signed_contact,
                  bool already_signed)
        : subst_(&subst), signed_contact_(already_signed
              ? std::move(signed_contact)
              : build_signed_contact_set<d>(signed_contact)) {}

    const Substitution<d>& substitution() const { return *subst_; }
    const std::set<SNode<d>>& signed_contact() const { return signed_contact_; }

    DerivedCoronaImage<d> project(const CoronaProjectionRequest<d>& request) const {
        if (!request.accept)
            throw std::invalid_argument("CoronaSurface::project: empty predicate");
        DerivedCoronaImage<d> result;
        std::deque<SNode<d>> frontier;
        auto admit = [&](const SNode<d>& node) {
            if (const auto witness = derive_corona_atom_witness(node))
                result.atom_witnesses.emplace(node, *witness);
            if (!request.accept(node)) {
                result.rejected_boundary.insert(node);
                return false;
            }
            if (result.nodes.insert(node).second) {
                frontier.push_back(node);
                if (request.node_cap != 0 &&
                    result.nodes.size() > request.node_cap) {
                    result.certificate.node_cap_hit = true;
                    return false;
                }
            }
            return true;
        };
        for (const auto& seed : request.seeds) admit(seed);

        while (!frontier.empty()) {
            if (result.certificate.node_cap_hit) break;
            if (request.expansion_cap != 0 &&
                result.certificate.expanded_nodes >= request.expansion_cap) {
                result.certificate.expansion_cap_hit = true;
                break;
            }
            const auto source = frontier.front();
            frontier.pop_front();
            ++result.certificate.expanded_nodes;
            const auto consume = [&](const auto& candidates) {
                result.certificate.generated_candidates += candidates.size();
                for (const auto& candidate : candidates) admit(candidate);
            };
            if (corona_surface_has(request.operations,
                                   CoronaSurfaceOperation::simple_forward)) {
                if (request.edge_arithmetic == CoronaEdgeArithmetic::exact_rational) {
                    std::vector<SNode<d>> candidates;
                    for (const auto& [node, witness] :
                         simple_forward_targets_exact<d>(*subst_, source)) {
                        (void)witness; candidates.push_back(node);
                    }
                    consume(candidates);
                } else {
                    std::vector<SNode<d>> candidates;
                    for (const auto& [node, witness] :
                         simple_forward_targets<d>(*subst_, source)) {
                        (void)witness; candidates.push_back(node);
                    }
                    consume(candidates);
                }
            }
            if (corona_surface_has(request.operations,
                                   CoronaSurfaceOperation::simple_backward)) {
                std::vector<SNode<d>> candidates;
                for (const auto& [node, witness] :
                     simple_backward_targets<d>(*subst_, source)) {
                    (void)witness; candidates.push_back(node);
                }
                consume(candidates);
            }
            if (corona_surface_has(request.operations,
                                   CoronaSurfaceOperation::corona_forward))
                consume(c_corona_targets_from_node<d>(
                    *subst_, source, signed_contact_));
            if (corona_surface_has(request.operations,
                                   CoronaSurfaceOperation::corona_backward))
                consume(c_corona_sources_to_node<d>(
                    *subst_, source, signed_contact_));
        }

        result.certificate.rejected_boundary_candidates =
            result.rejected_boundary.size();
        result.certificate.closed_under_requested_operations = frontier.empty();

        // Materialize only the exact induced simple-edge relation requested by
        // the consumer.  Prefix witnesses are preserved for proof replay.
        for (const auto& source : result.nodes) {
            const auto targets =
                request.edge_arithmetic == CoronaEdgeArithmetic::exact_rational
                    ? simple_forward_targets_exact<d>(*subst_, source)
                    : simple_forward_targets<d>(*subst_, source);
            for (const auto& [target, witness] : targets)
                if (result.nodes.count(target) != 0)
                    result.edges.push_back(
                        {source, target, witness.first, witness.second});
        }
        return result;
    }

private:
    const Substitution<d>* subst_;
    std::set<SNode<d>> signed_contact_;
};

// Restrict an ambient-graph node to the simple-graph node space
// (drop the i < j anti-symmetrization).  Just relabel to SNode.
template <std::size_t d>
SNode<d> to_simple(const ANode<d>& n) {
    SNode<d> r;
    r.i = n.i;
    r.x = n.x;
    r.j = n.j;
    return r;
}

// Restricted forward edges for the contact graph closure
// (graph_closure.py::induced_edges equivalent).  Uses BOTH type-1
// and type-2 ambient-graph edges (matching forward_edges()); the
// earlier draft only used type-1 and produced an empty Red result
// for sigma_1 because too many nodes lost their last out-edge.
template <std::size_t d>
std::vector<std::tuple<ANode<d>, ANode<d>, std::vector<long long>,
                        std::vector<long long>>>
induced_restricted_edges(const Substitution<d>& subst,
                         const std::vector<ANode<d>>& nodes) {
    std::set<ANode<d>> nodeset(nodes.begin(), nodes.end());
    std::vector<std::tuple<ANode<d>, ANode<d>,
                           std::vector<long long>, std::vector<long long>>> out;
    for (const auto& n : nodes) {
        auto edges = forward_edges<d>(subst, n);
        for (const auto& e : edges) {
            if (nodeset.count(e) > 0) out.push_back({n, e, {}, {}});
        }
    }
    return out;
}

// is_valid_anode per Def 3.1 restricted-graph validity:
//   [i, x, j] in D iff in_H_sigma([x, j]) AND (x != 0 OR i < j).
// Used by backward_edges to filter candidates independently of
// forward_edges' code path (per the differential-testing discipline:
// forward and backward must each be re-derived from scratch).
template <std::size_t d>
bool is_valid_anode(const Substitution<d>& subst, const ANode<d>& n) {
    bool all_zero = true;
    for (std::size_t k = 0; k < d; ++k) if (n.x[k] != 0) { all_zero = false; break; }
    if (all_zero && !(n.i < n.j)) return false;
    return subst.in_H_sigma(n.x, static_cast<std::size_t>(n.j));
}

// Backward edges for the restricted ambient graph (Def 3.1):
// given (i', x', j'), find all (i, x, j) with a type-1 or type-2
// edge INTO it.  Independently re-derived from the equations:
//   type 1: M x' = x + l(q1) - l(p1)  ==>  x = M x' + l(p1) - l(q1)
//           where sigma(i') = p1 i s1, sigma(j') = q1 j t1.
//   type 2: -M x' = x + l(q1) - l(p1) ==>  x = -M x' + l(p1) - l(q1)
//           where sigma(j') = p1 i s1, sigma(i') = q1 j t1.
// Both branches enumerate images[i'] / images[j'] directly (not
// via parent_decompositions) so this is genuinely independent of
// forward_edges_type1/type2's code path.
template <std::size_t d>
std::vector<ANode<d>> backward_edges(const Substitution<d>& subst,
                                     const ANode<d>& node) {
    std::vector<ANode<d>> out;
    const auto iprime = node.i;
    const auto jprime = node.j;
    auto Mxp = std::array<long long, d>{};
    for (std::size_t r = 0; r < d; ++r) {
        long long s = 0;
        for (std::size_t c = 0; c < d; ++c) s += subst.M[r][c] * node.x[c];
        Mxp[r] = s;
    }
    // Prefix abelianizations depend only on the image and cut position.
    // Cache them once per call: the old implementation rebuilt a temporary
    // vector and rescanned it for every (pi,pj) pair, which dominates the
    // five-letter backward-closure probe.
    std::vector<std::vector<std::array<long long, d>>> prefix_counts(
        subst.images.size());
    for (std::size_t image = 0; image < subst.images.size(); ++image) {
        prefix_counts[image].resize(subst.images[image].size() + 1);
        prefix_counts[image][0].fill(0);
        for (std::size_t cut = 0; cut < subst.images[image].size(); ++cut) {
            prefix_counts[image][cut + 1] = prefix_counts[image][cut];
            ++prefix_counts[image][cut + 1][
                static_cast<std::size_t>(subst.images[image][cut])];
        }
    }
    // Type 1: x = M x' + l(p1) - l(q1)
    for (std::size_t pi = 0; pi < subst.images[iprime].size(); ++pi) {
        const auto& lp1 = prefix_counts[iprime][pi];
        long long letter_i = subst.images[iprime][pi];
        for (std::size_t pj = 0; pj < subst.images[jprime].size(); ++pj) {
            const auto& lq1 = prefix_counts[jprime][pj];
            long long letter_j = subst.images[jprime][pj];
            ANode<d> cand;
            cand.i = letter_i;
            cand.j = letter_j;
            for (std::size_t k = 0; k < d; ++k)
                cand.x[k] = Mxp[k] + lp1[k] - lq1[k];
            if (is_valid_anode<d>(subst, cand)) out.push_back(cand);
        }
    }
    // Type 2: x = -M x' + l(p1) - l(q1), with the (i',j') roles swapped
    for (std::size_t pi = 0; pi < subst.images[jprime].size(); ++pi) {
        const auto& lp1 = prefix_counts[jprime][pi];
        long long letter_i = subst.images[jprime][pi];
        for (std::size_t pj = 0; pj < subst.images[iprime].size(); ++pj) {
            const auto& lq1 = prefix_counts[iprime][pj];
            long long letter_j = subst.images[iprime][pj];
            ANode<d> cand;
            cand.i = letter_i;
            cand.j = letter_j;
            for (std::size_t k = 0; k < d; ++k)
                cand.x[k] = -Mxp[k] + lp1[k] - lq1[k];
            if (is_valid_anode<d>(subst, cand)) out.push_back(cand);
        }
    }
    return out;
}

template <std::size_t d>
struct BackwardEdgeWitness {
    ANode<d> predecessor;
    int type = 1;
    std::size_t first_prefix_position = 0;
    std::size_t second_prefix_position = 0;
};

// Backward closure is often the first large allocation in a contact-boundary
// run.  Its membership operation is hash-based in spirit; keeping it in a
// tree makes the theta5 frontier pay a logarithmic lookup plus a node-heavy
// red-black allocation for every predecessor.  Use a compact structural hash
// while retaining a sorted vector at the API boundary below.
template <std::size_t d>
struct ANodeHash {
    std::size_t operator()(const ANode<d>& node) const noexcept {
        std::size_t h = static_cast<std::size_t>(node.i) * 131071u +
                        static_cast<std::size_t>(node.j) * 17u;
        for (const auto value : node.x)
            h = h * 31u + static_cast<std::size_t>(value);
        return h;
    }
};

// Labelled form of backward_edges used by affine-family certificates.
// Positions are the two prefix cut locations in the corresponding
// substitution images; projection to `predecessor` agrees with
// backward_edges up to duplicate witnesses.
template <std::size_t d>
std::vector<BackwardEdgeWitness<d>> backward_edge_witnesses(
        const Substitution<d>& subst, const ANode<d>& node,
        bool require_restricted_validity = true) {
    std::vector<BackwardEdgeWitness<d>> out;
    auto Mxp = std::array<long long, d>{};
    for (std::size_t r = 0; r < d; ++r)
        for (std::size_t c = 0; c < d; ++c)
            Mxp[r] += subst.M[r][c] * node.x[c];
    const auto add = [&](int type, std::size_t pi, std::size_t pj,
                         std::size_t first_image,
                         std::size_t second_image) {
        const auto lp = abelianization<d>(std::vector<long long>(
            subst.images[first_image].begin(),
            subst.images[first_image].begin() + pi));
        const auto lq = abelianization<d>(std::vector<long long>(
            subst.images[second_image].begin(),
            subst.images[second_image].begin() + pj));
        ANode<d> candidate;
        candidate.i = subst.images[first_image][pi];
        candidate.j = subst.images[second_image][pj];
        for (std::size_t k = 0; k < d; ++k)
            candidate.x[k] =
                (type == 1 ? Mxp[k] : -Mxp[k]) + lp[k] - lq[k];
        if (!require_restricted_validity
                || is_valid_anode<d>(subst, candidate))
            out.push_back({candidate, type, pi, pj});
    };
    for (std::size_t pi = 0; pi < subst.images[node.i].size(); ++pi)
        for (std::size_t pj = 0; pj < subst.images[node.j].size(); ++pj)
            add(1, pi, pj, node.i, node.j);
    for (std::size_t pi = 0; pi < subst.images[node.j].size(); ++pi)
        for (std::size_t pj = 0; pj < subst.images[node.i].size(); ++pj)
            add(2, pi, pj, node.j, node.i);
    return out;
}

// Backward closure: starting from seed_nodes, repeatedly add any node
// that has a type-1 OR type-2 edge INTO the current set, until closure.
// Frontier-based BFS (round-by-round, NOT push-back-during-iteration,
// which would invalidate iterators and trigger use-after-free in std::vector).
template <std::size_t d>
std::vector<ANode<d>> backward_closure(const Substitution<d>& subst,
                                     const std::vector<ANode<d>>& seed_nodes,
                                     std::size_t max_nodes = 500) {
    std::unordered_set<ANode<d>, ANodeHash<d>> visited;
    for (const auto& n : seed_nodes) visited.insert(n);
    std::vector<ANode<d>> frontier(seed_nodes.begin(), seed_nodes.end());
    while (!frontier.empty()) {
        std::vector<ANode<d>> next_frontier;
        for (const auto& v : frontier) {
            for (const auto& pred : backward_edges<d>(subst, v)) {
                if (visited.insert(pred).second) {
                    if (visited.size() > max_nodes) {
                        // Mirror graph_closure.py's loud-stop policy:
                        // paper claims 14 for sigma1; if we blow past 500
                        // the closure is wrong, not just "needs more".
                        std::vector<ANode<d>> result(visited.begin(), visited.end());
                        std::sort(result.begin(), result.end(), [](const auto& lhs,
                                                                   const auto& rhs) {
                            if (lhs.i != rhs.i) return lhs.i < rhs.i;
                            if (lhs.x != rhs.x) return lhs.x < rhs.x;
                            return lhs.j < rhs.j;
                        });
                        return result;
                    }
                    next_frontier.push_back(pred);
                }
            }
        }
        frontier = std::move(next_frontier);
    }
    std::vector<ANode<d>> result(visited.begin(), visited.end());
    std::sort(result.begin(), result.end(), [](const auto& lhs, const auto& rhs) {
        if (lhs.i != rhs.i) return lhs.i < rhs.i;
        if (lhs.x != rhs.x) return lhs.x < rhs.x;
        return lhs.j < rhs.j;
    });
    return result;
}

// Red together with its ranked exclusion witness.  pruning_ranks[0]
// contains the sinks of the original restricted graph;
// pruning_ranks[k] contains the nodes that become sinks only after all
// earlier ranks have been removed.  Thus the ranks partition exactly
// nodes \ survivors and expose the universal-exclusion half of Red,
// while the surviving edge set exposes its positive recurrent half.
template <std::size_t d>
struct RedTrace {
    std::set<SNode<d>> survivors;
    std::vector<std::tuple<SNode<d>, SNode<d>,
                           std::vector<long long>,
                           std::vector<long long>>> survivor_edges;
    std::vector<std::set<SNode<d>>> pruning_ranks;
};

template <std::size_t d>
RedTrace<d> red_trace(
        const std::set<SNode<d>>& nodes,
        const std::vector<std::tuple<SNode<d>, SNode<d>,
                                    std::vector<long long>,
                                    std::vector<long long>>>& edges) {
    RedTrace<d> result;
    std::set<SNode<d>> cur(nodes);
    while (true) {
        // Build out-edge set.
        std::set<SNode<d>> has_out;
        for (const auto& [src, dest, p1, q1] : edges) {
            // Only edges within cur.
            if (cur.count(src) > 0 && cur.count(dest) > 0) has_out.insert(src);
        }
        std::set<SNode<d>> rank;
        for (const auto& node : cur)
            if (has_out.count(node) == 0) rank.insert(node);
        if (rank.empty()) break;
        for (const auto& node : rank) cur.erase(node);
        result.pruning_ranks.push_back(std::move(rank));
    }
    // Filter edges to surviving nodes.
    for (const auto& e : edges) {
        if (cur.count(std::get<0>(e)) > 0 && cur.count(std::get<1>(e)) > 0)
            result.survivor_edges.push_back(e);
    }
    result.survivors = std::move(cur);
    return result;
}

// Compatibility projection used by existing callers.
template <std::size_t d>
std::pair<std::set<SNode<d>>, std::vector<std::tuple<SNode<d>, SNode<d>,
                                               std::vector<long long>,
                                               std::vector<long long>>>>
red(const std::set<SNode<d>>& nodes,
    const std::vector<std::tuple<SNode<d>, SNode<d>,
                               std::vector<long long>,
                               std::vector<long long>>>& edges) {
    auto trace = red_trace<d>(nodes, edges);
    return {std::move(trace.survivors), std::move(trace.survivor_edges)};
}

// red for ANode (used during the restricted-graph closure phase,
// where the i < j anti-symmetrization of D still applies).  Red
// removes nodes with no out-edge, repeatedly.
template <std::size_t d>
std::pair<std::set<ANode<d>>,
          std::vector<std::tuple<ANode<d>, ANode<d>,
                                 std::vector<long long>,
                                 std::vector<long long>>>>
red_anode(const std::set<ANode<d>>& nodes,
          const std::vector<std::tuple<ANode<d>, ANode<d>,
                                 std::vector<long long>,
                                 std::vector<long long>>>& edges) {
    std::set<ANode<d>> cur(nodes);
    bool changed = true;
    while (changed) {
        changed = false;
        std::set<ANode<d>> has_out;
        for (const auto& [src, dest, _p1, _q1] : edges) {
            if (cur.count(src) > 0 && cur.count(dest) > 0) has_out.insert(src);
        }
        for (auto it = cur.begin(); it != cur.end(); ) {
            if (has_out.count(*it) == 0) {
                it = cur.erase(it);
                changed = true;
            } else {
                ++it;
            }
        }
    }
    std::vector<std::tuple<ANode<d>, ANode<d>, std::vector<long long>,
                           std::vector<long long>>> filtered;
    for (const auto& e : edges) {
        if (cur.count(std::get<0>(e)) > 0 && cur.count(std::get<1>(e)) > 0)
            filtered.push_back(e);
    }
    return {cur, filtered};
}

// A fully inspectable Algorithm-2 trace.  The distinction between a
// fixed ±C connector set and an evolving A[p-1] connector set is
// deliberately explicit: Definition 3.9 uses the former, while the
// historical project implementation used the latter.  Keeping both
// policies available supports differential verification before old
// computational claims are migrated.
template <std::size_t d>
struct CoronaLayer {
    int round = 1;
    std::set<SNode<d>> input_nodes;
    std::set<SNode<d>> pre_red_nodes;
    std::set<SNode<d>> nodes;
    std::vector<std::set<SNode<d>>> red_pruning_ranks;
    std::vector<std::tuple<SNode<d>, SNode<d>,
                           std::vector<long long>,
                           std::vector<long long>>> edges;
};

template <std::size_t d>
struct CoronaTrace {
    CoronaConnectorPolicy connector_policy =
        CoronaConnectorPolicy::fixed_signed_contact;
    CoronaEdgeArithmetic edge_arithmetic =
        CoronaEdgeArithmetic::fast_rounded;
    std::set<SNode<d>> signed_contact;
    std::vector<CoronaLayer<d>> layers;
    std::set<SNode<d>> final_nodes;
    bool converged = false;
    bool node_cap_hit = false;
};

template <std::size_t d>
CoronaTrace<d> algorithm2_trace(
        const Substitution<d>& subst,
        const std::set<SNode<d>>& C,
        CoronaConnectorPolicy policy =
            CoronaConnectorPolicy::fixed_signed_contact,
        int max_rounds = 50,
        std::size_t node_cap = 0,
        CoronaEdgeArithmetic edge_arithmetic =
            CoronaEdgeArithmetic::fast_rounded) {
    CoronaTrace<d> trace;
    trace.connector_policy = policy;
    trace.edge_arithmetic = edge_arithmetic;
    trace.signed_contact = build_signed_contact_set<d>(C);
    auto A_prev = trace.signed_contact;

    CoronaLayer<d> initial;
    initial.round = 1;
    initial.nodes = A_prev;
    trace.layers.push_back(std::move(initial));

    for (int p = 2; p <= max_rounds; ++p) {
        const auto& connectors =
            policy == CoronaConnectorPolicy::fixed_signed_contact
                ? trace.signed_contact
                : A_prev;
        auto corona_nodes = c_corona<d>(subst, A_prev, connectors);

        CoronaLayer<d> layer;
        layer.round = p;
        layer.input_nodes = A_prev;
        layer.pre_red_nodes = corona_nodes;
        for (const auto& n : corona_nodes) {
            const auto targets =
                edge_arithmetic == CoronaEdgeArithmetic::exact_rational
                    ? simple_forward_targets_exact<d>(subst, n)
                    : simple_forward_targets<d>(subst, n);
            for (const auto& [dest, pq] : targets) {
                if (corona_nodes.count(dest) > 0) {
                    layer.edges.push_back(
                        {n, dest, pq.first, pq.second});
                }
            }
        }
        auto reduced = red_trace<d>(corona_nodes, layer.edges);
        layer.nodes = std::move(reduced.survivors);
        layer.edges = std::move(reduced.survivor_edges);
        layer.red_pruning_ranks = std::move(reduced.pruning_ranks);

        if (node_cap != 0 && layer.nodes.size() > node_cap) {
            trace.node_cap_hit = true;
            trace.final_nodes = A_prev;
            trace.layers.push_back(std::move(layer));
            return trace;
        }
        const bool fixed = layer.nodes == A_prev;
        A_prev = layer.nodes;
        trace.layers.push_back(std::move(layer));
        if (fixed) {
            trace.converged = true;
            trace.final_nodes = std::move(A_prev);
            return trace;
        }
    }
    trace.final_nodes = std::move(A_prev);
    return trace;
}

// Algorithm 2: the fixed-point iteration.
//   A[1] = ±C (signed contact graph, including identity elements).
//   A[p] = Red(C-Corona(A[p-1])) for p >= 2, where every corona
//   composition uses the fixed signed contact set ±C.
// Until A[p] == A[p-1].
//
// IMPORTANT: edges are restricted to within corona_nodes, NOT all
// forward edges from corona_nodes -- this matches the Python's
// induced_simple_edges() which gates with `if dest in nodeset`.
// Including edges to nodes outside corona_nodes causes Red to
// prune spuriously (those outside-destinations can still satisfy
// `cur.count(dest) > 0` only if they happen to also be in cur, but
// in general the constraint is on edges with both endpoints in cur).
template <std::size_t d>
std::set<SNode<d>> algorithm2(const Substitution<d>& subst,
                            const std::set<SNode<d>>& C) {
    return algorithm2_trace<d>(
        subst, C, CoronaConnectorPolicy::fixed_signed_contact)
        .final_nodes;
}

// Lexicographic ordering for SNode, used by std::set<SNode>.
template <std::size_t d>
bool operator<(const SNode<d>& a, const SNode<d>& b) {
    if (a.i != b.i) return a.i < b.i;
    if (a.x != b.x) return a.x < b.x;
    return a.j < b.j;
}

}  // namespace ravel
