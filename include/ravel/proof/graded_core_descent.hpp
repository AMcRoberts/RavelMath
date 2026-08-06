#pragma once

#include <algorithm>
#include <cstddef>
#include <map>
#include <queue>
#include <set>
#include <stdexcept>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

#include "ravel/nbonacci_margin_invariant.hpp"

namespace ravel::proof {

using nbonacci_margin::DisplacementDescriptor;
using nbonacci_margin::DisplacementKind;
using nbonacci_margin::LabelPair;

struct GradeDescentPhase {
    bool diagonal = false;
    bool pair_ascending = false;
    DisplacementKind kind = DisplacementKind::Root;
    long long sign = 1;
    bool active_starts_on_left_face = false;
    bool active_starts_on_right_face = false;
    bool active_touches_last = false;

    auto operator<=>(const GradeDescentPhase&) const = default;
};

struct GradeDescentWitness {
    std::size_t source_grade = 0;
    std::size_t remainder_grade = 0;
    DisplacementDescriptor active_descriptor;
    std::vector<long long> active;
    std::vector<long long> remainder;
    GradeDescentPhase phase;
};

struct TwoStageGradeDescentWitness {
    GradeDescentWitness first;
    std::vector<long long> second_atom;
    std::vector<long long> final_atom;
};

struct GradeDescentObstruction {
    std::size_t grade = 0;
    LabelPair pair;
    std::vector<long long> displacement;
    std::string reason;
};

class DisplacementSumCatalogue {
public:
    explicit DisplacementSumCatalogue(std::size_t n) : n_(n) {
        if (n < 2) throw std::invalid_argument("grade catalogue: n < 2");
        for (const auto descriptor : nbonacci_margin::displacement_descriptors(n)) {
            const auto atom = nbonacci_margin::displacement_from_descriptor(n, descriptor);
            described_atoms_.push_back({descriptor, atom});
            atoms_.push_back(atom);
            atom_set_.insert(atom);
        }
        for (std::size_t lhs = 0; lhs < atoms_.size(); ++lhs)
            for (std::size_t rhs = lhs; rhs < atoms_.size(); ++rhs)
                two_sums_.insert(add(atoms_[lhs], atoms_[rhs]));
    }

    std::size_t dimension() const { return n_; }
    const std::vector<std::vector<long long>>& atoms() const { return atoms_; }
    const std::vector<std::pair<DisplacementDescriptor, std::vector<long long>>>&
    described_atoms() const { return described_atoms_; }

    std::size_t grade(const std::vector<long long>& x) const {
        require_dimension(x);
        if (atom_set_.count(x)) return 1;
        if (two_sums_.count(x)) return 2;
        for (const auto& atom : atoms_)
            if (two_sums_.count(subtract(x, atom))) return 3;
        return 0;
    }

    std::vector<GradeDescentWitness> derive_one_stage(
        LabelPair pair, const std::vector<long long>& x) const {
        const auto source_grade = grade(x);
        std::vector<GradeDescentWitness> result;
        if (source_grade <= 1) return result;
        for (const auto& [descriptor, active] : described_atoms_) {
            if (!nbonacci_margin::predicted_core_member(n_, pair, descriptor))
                continue;
            auto remainder = subtract(x, active);
            const auto remainder_grade = grade(remainder);
            if (remainder_grade + 1 != source_grade) continue;
            result.push_back(GradeDescentWitness{
                source_grade, remainder_grade, descriptor, active,
                std::move(remainder), phase_of(pair, descriptor)});
        }
        return result;
    }

    std::vector<TwoStageGradeDescentWitness> derive_two_stage(
        LabelPair pair, const std::vector<long long>& x) const {
        std::vector<TwoStageGradeDescentWitness> result;
        for (const auto& first : derive_one_stage(pair, x)) {
            if (first.source_grade != 3 || first.remainder_grade != 2) continue;
            for (std::size_t lhs = 0; lhs < atoms_.size(); ++lhs) {
                const auto rhs = subtract(first.remainder, atoms_[lhs]);
                if (!atom_set_.count(rhs)) continue;
                result.push_back({first, atoms_[lhs], rhs});
            }
        }
        return result;
    }

    GradeDescentObstruction explain_failure(
        LabelPair pair, const std::vector<long long>& x) const {
        const auto g = grade(x);
        if (g <= 1)
            return {g, pair, x, "state is already grade one or ungraded"};
        bool any_core_active = false;
        std::set<std::size_t> remainder_grades;
        for (const auto& [descriptor, active] : described_atoms_) {
            if (!nbonacci_margin::predicted_core_member(n_, pair, descriptor))
                continue;
            any_core_active = true;
            remainder_grades.insert(grade(subtract(x, active)));
        }
        if (!any_core_active)
            return {g, pair, x, "label phase admits no predicted-core active summand"};
        std::string reason = "admissible active summands have remainder grades";
        for (const auto rg : remainder_grades) reason += " " + std::to_string(rg);
        return {g, pair, x, std::move(reason)};
    }

private:
    std::size_t n_;
    std::vector<std::vector<long long>> atoms_;
    std::vector<std::pair<DisplacementDescriptor, std::vector<long long>>>
        described_atoms_;
    std::set<std::vector<long long>> atom_set_;
    std::set<std::vector<long long>> two_sums_;

    void require_dimension(const std::vector<long long>& x) const {
        if (x.size() != n_) throw std::invalid_argument("grade catalogue: wrong dimension");
    }

    static std::vector<long long> add(
        const std::vector<long long>& lhs,
        const std::vector<long long>& rhs) {
        std::vector<long long> out(lhs.size());
        for (std::size_t i = 0; i < lhs.size(); ++i) out[i] = lhs[i] + rhs[i];
        return out;
    }

    static std::vector<long long> subtract(
        const std::vector<long long>& lhs,
        const std::vector<long long>& rhs) {
        std::vector<long long> out(lhs.size());
        for (std::size_t i = 0; i < lhs.size(); ++i) out[i] = lhs[i] - rhs[i];
        return out;
    }

    GradeDescentPhase phase_of(
        LabelPair pair, DisplacementDescriptor descriptor) const {
        return GradeDescentPhase{
            pair.i == pair.j,
            pair.i < pair.j,
            descriptor.kind,
            descriptor.sign,
            descriptor.a == pair.i,
            descriptor.a == pair.j,
            descriptor.b + 1 == n_ ||
                (descriptor.kind == DisplacementKind::AlternatingTriple &&
                 descriptor.c + 1 == n_)};
    }
};

struct ComponentGradeDescentSummary {
    std::size_t vertices = 0;
    std::map<std::size_t, std::size_t> grade_histogram;
    std::map<std::size_t, std::size_t> covered_by_grade;
    std::map<GradeDescentPhase, std::size_t> phase_cover;
    std::vector<GradeDescentObstruction> obstructions;

    bool all_nonterminal_covered() const {
        std::size_t required = 0, covered = 0;
        for (const auto& [g, count] : grade_histogram)
            if (g > 1) required += count;
        for (const auto& [g, count] : covered_by_grade)
            if (g > 1) covered += count;
        return required == covered;
    }
};

template <class Range, class PairFn, class VectorFn>
ComponentGradeDescentSummary derive_component_grade_descent(
    const DisplacementSumCatalogue& catalogue,
    const Range& vertices, PairFn pair_of, VectorFn vector_of,
    std::size_t obstruction_limit = 16) {
    ComponentGradeDescentSummary out;
    for (const auto& vertex : vertices) {
        ++out.vertices;
        const auto pair = pair_of(vertex);
        const auto x = vector_of(vertex);
        const auto g = catalogue.grade(x);
        ++out.grade_histogram[g];
        if (g <= 1) continue;
        const auto witnesses = catalogue.derive_one_stage(pair, x);
        if (!witnesses.empty()) {
            ++out.covered_by_grade[g];
            std::set<GradeDescentPhase> phases;
            for (const auto& witness : witnesses) phases.insert(witness.phase);
            for (const auto& phase : phases) ++out.phase_cover[phase];
        } else if (out.obstructions.size() < obstruction_limit) {
            out.obstructions.push_back(catalogue.explain_failure(pair, x));
        }
    }
    return out;
}


struct RenewalGradeSimulation {
    std::size_t source_vertices = 0;
    std::size_t renewal_vertices = 0;
    std::size_t target_vertices = 0;
    std::size_t renewal_blocks = 0;
    std::size_t maximum_block_length = 0;
    std::size_t initial_pairs = 0;
    std::size_t surviving_pairs = 0;
    std::size_t covered_renewal_vertices = 0;
    bool higher_stratum_acyclic = true;
    bool complete = false;
};

// Block/renewal simulation for mixed-grade recurrent components.  The source
// is observed only on a renewal section (normally the minimum positive grade).
// A macro edge is every path from one renewal vertex to the next whose strict
// interior avoids the renewal section.  Such a block may be matched by a
// target path of the same length.  The greatest relation retained below obeys
//
//   R(s,t) and s ==block(k)==> s'
//     ==> exists t', t ==path(k)==> t' and R(s',t').
//
// Thus grade-changing source edges are consumed as a whole excursion rather
// than being forced into a false one-edge synchronization.
template <class SourceSuccessorFn, class TargetSuccessorFn,
          class RenewalFn, class AllowedFn>
RenewalGradeSimulation derive_renewal_grade_simulation(
    std::size_t source_count,
    std::size_t target_count,
    SourceSuccessorFn source_successors,
    TargetSuccessorFn target_successors,
    RenewalFn is_renewal,
    AllowedFn allowed,
    std::size_t maximum_length = 64) {
    RenewalGradeSimulation out;
    out.source_vertices = source_count;
    out.target_vertices = target_count;
    std::vector<std::vector<std::size_t>> source_out(source_count);
    std::vector<std::vector<std::size_t>> target_out(target_count);
    for (std::size_t s = 0; s < source_count; ++s)
        source_out[s] = source_successors(s);
    for (std::size_t t = 0; t < target_count; ++t)
        target_out[t] = target_successors(t);

    std::vector<std::size_t> renewal;
    std::vector<std::size_t> renewal_local(source_count, source_count);
    for (std::size_t s = 0; s < source_count; ++s)
        if (is_renewal(s)) {
            renewal_local[s] = renewal.size();
            renewal.push_back(s);
        }
    out.renewal_vertices = renewal.size();

    struct Block { std::size_t destination; std::size_t length; };
    std::vector<std::vector<Block>> blocks(renewal.size());
    for (std::size_t r = 0; r < renewal.size(); ++r) {
        const auto start = renewal[r];
        std::queue<std::pair<std::size_t, std::size_t>> queue;
        std::set<std::pair<std::size_t, std::size_t>> seen;
        for (const auto next : source_out[start]) {
            queue.push({next, 1});
            seen.insert({next, 1});
        }
        while (!queue.empty()) {
            const auto [v, length] = queue.front();
            queue.pop();
            if (is_renewal(v)) {
                blocks[r].push_back({renewal_local[v], length});
                ++out.renewal_blocks;
                out.maximum_block_length = std::max(out.maximum_block_length, length);
                continue;
            }
            if (length >= maximum_length) {
                out.higher_stratum_acyclic = false;
                continue;
            }
            for (const auto next : source_out[v])
                if (seen.insert({next, length + 1}).second)
                    queue.push({next, length + 1});
        }
        std::sort(blocks[r].begin(), blocks[r].end(),
                  [](const Block& a, const Block& b) {
                      return std::tie(a.destination, a.length) <
                             std::tie(b.destination, b.length);
                  });
        blocks[r].erase(std::unique(blocks[r].begin(), blocks[r].end(),
            [](const Block& a, const Block& b) {
                return a.destination == b.destination && a.length == b.length;
            }), blocks[r].end());
    }

    const auto max_length = out.maximum_block_length;
    std::vector<std::vector<std::vector<std::size_t>>> reach(
        max_length + 1,
        std::vector<std::vector<std::size_t>>(target_count));
    for (std::size_t t = 0; t < target_count; ++t) reach[0][t] = {t};
    for (std::size_t length = 1; length <= max_length; ++length)
        for (std::size_t t = 0; t < target_count; ++t) {
            std::set<std::size_t> destinations;
            for (const auto next : target_out[t])
                for (const auto destination : reach[length - 1][next])
                    destinations.insert(destination);
            reach[length][t].assign(destinations.begin(), destinations.end());
        }

    std::vector<std::vector<bool>> live(
        renewal.size(), std::vector<bool>(target_count, false));
    for (std::size_t r = 0; r < renewal.size(); ++r)
        for (std::size_t t = 0; t < target_count; ++t)
            if (allowed(renewal[r], t)) {
                live[r][t] = true;
                ++out.initial_pairs;
            }

    bool changed = true;
    while (changed) {
        changed = false;
        for (std::size_t r = 0; r < renewal.size(); ++r)
            for (std::size_t t = 0; t < target_count; ++t) {
                if (!live[r][t]) continue;
                bool keep = !blocks[r].empty();
                for (const auto& block : blocks[r]) {
                    bool matched = false;
                    for (const auto tp : reach[block.length][t])
                        if (live[block.destination][tp]) {
                            matched = true;
                            break;
                        }
                    if (!matched) {
                        keep = false;
                        break;
                    }
                }
                if (!keep) {
                    live[r][t] = false;
                    changed = true;
                }
            }
    }

    for (std::size_t r = 0; r < renewal.size(); ++r) {
        bool covered = false;
        for (std::size_t t = 0; t < target_count; ++t)
            if (live[r][t]) {
                ++out.surviving_pairs;
                covered = true;
            }
        out.covered_renewal_vertices += covered;
    }
    out.complete = out.higher_stratum_acyclic &&
        out.covered_renewal_vertices == out.renewal_vertices;
    return out;
}

}  // namespace ravel::proof

namespace ravel::proof {

struct PhaseAwareDescentPath {
    std::size_t source = 0;
    std::size_t destination = 0;
    std::vector<std::size_t> vertices;
};

struct PhaseAwareGradeReduction {
    std::vector<PhaseAwareDescentPath> paths;
    std::size_t maximum_hops = 0;
    bool complete = false;
};

// Move each non-directly-covered vertex through the recurrent component to a
// phase where a grade-lowering core summand is admissible.  The operation is
// graph-generic: the caller supplies the component-local successor relation
// and the directly-covered predicate.  Every returned path is replayable.
template <class SuccessorFn, class CoveredFn>
PhaseAwareGradeReduction derive_phase_aware_grade_reduction(
    std::size_t vertex_count, SuccessorFn successors, CoveredFn covered) {
    PhaseAwareGradeReduction out;
    std::vector<std::vector<std::size_t>> reverse(vertex_count);
    for (std::size_t source = 0; source < vertex_count; ++source)
        for (const auto destination : successors(source)) {
            if (destination >= vertex_count)
                throw std::invalid_argument("phase reduction: bad successor");
            reverse[destination].push_back(source);
        }

    const auto none = vertex_count;
    std::vector<std::size_t> next(vertex_count, none);
    std::vector<std::size_t> root(vertex_count, none);
    std::vector<std::size_t> distance(vertex_count, none);
    std::vector<std::size_t> queue;
    queue.reserve(vertex_count);
    for (std::size_t v = 0; v < vertex_count; ++v) {
        if (!covered(v)) continue;
        root[v] = v;
        distance[v] = 0;
        queue.push_back(v);
    }
    for (std::size_t head = 0; head < queue.size(); ++head) {
        const auto target = queue[head];
        for (const auto source : reverse[target]) {
            if (distance[source] != none) continue;
            distance[source] = distance[target] + 1;
            next[source] = target;
            root[source] = root[target];
            queue.push_back(source);
        }
    }

    out.complete = true;
    for (std::size_t source = 0; source < vertex_count; ++source) {
        if (covered(source)) continue;
        if (distance[source] == none) {
            out.complete = false;
            continue;
        }
        PhaseAwareDescentPath path;
        path.source = source;
        path.destination = root[source];
        path.vertices.push_back(source);
        auto cursor = source;
        while (!covered(cursor)) {
            cursor = next[cursor];
            if (cursor == none || path.vertices.size() > vertex_count + 1)
                throw std::logic_error("phase reduction: broken parent chain");
            path.vertices.push_back(cursor);
        }
        out.maximum_hops = std::max(out.maximum_hops, path.vertices.size() - 1);
        out.paths.push_back(std::move(path));
    }
    return out;
}


struct CompatibleGradeDescentEdge {
    std::size_t source_witness = 0;
    std::size_t destination_witness = 0;
    std::size_t destination_vertex = 0;
};

struct TransitionCompatibleGradeDescent {
    std::size_t vertices = 0;
    std::size_t witness_nodes = 0;
    std::size_t compatible_edges = 0;
    std::size_t existentially_serial_witnesses = 0;
    std::size_t universally_serial_witnesses = 0;
    std::size_t vertices_with_existential_witness = 0;
    std::size_t vertices_with_universal_witness = 0;
    std::size_t required_nonterminal_vertices = 0;
    bool decomposition_law_replayed = false;
    bool existential_complete = false;
    bool universal_complete = false;
};

inline bool compatible_grade_descent_edge(
    std::size_t n,
    LabelPair source_pair,
    const GradeDescentWitness& source,
    LabelPair destination_pair,
    const GradeDescentWitness& destination) {
    const nbonacci_margin::CoreState active_source{
        source_pair, source.active_descriptor};
    const nbonacci_margin::CoreState active_destination{
        destination_pair, destination.active_descriptor};
    const auto successors =
        nbonacci_margin::predicted_core_successors(n, active_source);
    if (!successors.count(active_destination)) return false;
    const auto expected_remainder =
        nbonacci_margin::forward_displacement(source.remainder, 0);
    return destination.remainder == expected_remainder;
}

// Derive the transition-compatible grade-lowering relation directly from the
// executable carry and predicted-core successor operations.  A decomposition
// x = active + remainder is transported across a labelled edge by sending the
// active summand through the predicted core with the source label defect and
// the remainder through the homogeneous (delta = 0) carry map.
//
// Two greatest fixed points are reported:
//   * existential: a compatible continuation exists along some source edge;
//   * universal: for every source edge, a compatible continuation exists.
// The latter is the simulation property needed to project arbitrary source
// paths without choosing their edges in advance.
template <class SuccessorFn, class PairFn, class VectorFn>
TransitionCompatibleGradeDescent derive_transition_compatible_grade_descent(
    const DisplacementSumCatalogue& catalogue,
    std::size_t vertex_count,
    SuccessorFn successors,
    PairFn pair_of,
    VectorFn vector_of) {
    TransitionCompatibleGradeDescent out;
    out.vertices = vertex_count;
    const auto n = catalogue.dimension();

    std::vector<std::vector<GradeDescentWitness>> witnesses(vertex_count);
    std::vector<std::vector<std::size_t>> outgoing(vertex_count);
    for (std::size_t v = 0; v < vertex_count; ++v) {
        witnesses[v] = catalogue.derive_one_stage(pair_of(v), vector_of(v));
        outgoing[v] = successors(v);
        out.witness_nodes += witnesses[v].size();
    }

    // Replay the affine decomposition law on every concrete graph edge.
    out.decomposition_law_replayed = true;
    for (std::size_t u = 0; u < vertex_count; ++u) {
        const auto source_x = vector_of(u);
        const auto source_pair = pair_of(u);
        const long long delta =
            (source_pair.j > 0 ? 1LL : 0LL) -
            (source_pair.i > 0 ? 1LL : 0LL);
        for (const auto v : outgoing[u]) {
            if (v >= vertex_count)
                throw std::invalid_argument("grade transition: bad successor");
            const auto expected = nbonacci_margin::forward_displacement(
                source_x, delta);
            if (expected != vector_of(v)) out.decomposition_law_replayed = false;
            for (std::size_t wi = 0; wi < witnesses[u].size(); ++wi) {
                const auto& sw = witnesses[u][wi];
                const auto active_next = nbonacci_margin::forward_displacement(
                    sw.active, delta);
                const auto remainder_next = nbonacci_margin::forward_displacement(
                    sw.remainder, 0);
                std::vector<long long> recomposed(n, 0);
                for (std::size_t k = 0; k < n; ++k)
                    recomposed[k] = active_next[k] + remainder_next[k];
                if (recomposed != expected) out.decomposition_law_replayed = false;
            }
        }
    }

    struct WitnessNode { std::size_t vertex; std::size_t local; };
    std::vector<WitnessNode> nodes;
    std::vector<std::vector<std::size_t>> node_id(vertex_count);
    for (std::size_t v = 0; v < vertex_count; ++v) {
        node_id[v].resize(witnesses[v].size());
        for (std::size_t w = 0; w < witnesses[v].size(); ++w) {
            node_id[v][w] = nodes.size();
            nodes.push_back({v, w});
        }
    }
    std::vector<std::map<std::size_t, std::vector<std::size_t>>> by_destination(
        nodes.size());
    std::vector<std::vector<std::size_t>> compatible(nodes.size());
    for (std::size_t id = 0; id < nodes.size(); ++id) {
        const auto [u, wi] = nodes[id];
        for (const auto v : outgoing[u]) {
            for (std::size_t wj = 0; wj < witnesses[v].size(); ++wj) {
                if (!compatible_grade_descent_edge(
                        n, pair_of(u), witnesses[u][wi],
                        pair_of(v), witnesses[v][wj])) continue;
                const auto target = node_id[v][wj];
                compatible[id].push_back(target);
                by_destination[id][v].push_back(target);
                ++out.compatible_edges;
            }
        }
    }

    auto greatest_fixed_point = [&](bool universal) {
        std::vector<bool> live(nodes.size(), true);
        bool changed = true;
        while (changed) {
            changed = false;
            for (std::size_t id = 0; id < nodes.size(); ++id) {
                if (!live[id]) continue;
                const auto u = nodes[id].vertex;
                bool keep = false;
                if (universal) {
                    keep = true;
                    for (const auto v : outgoing[u]) {
                        bool found = false;
                        const auto found_targets = by_destination[id].find(v);
                        if (found_targets != by_destination[id].end())
                            for (const auto target : found_targets->second)
                                if (live[target]) { found = true; break; }
                        if (!found) { keep = false; break; }
                    }
                    // A recurrent component should have outgoing edges.  Do
                    // not treat a dead end as a vacuous universal simulation.
                    if (outgoing[u].empty()) keep = false;
                } else {
                    for (const auto target : compatible[id])
                        if (live[target]) { keep = true; break; }
                }
                if (!keep) { live[id] = false; changed = true; }
            }
        }
        return live;
    };

    const auto existential = greatest_fixed_point(false);
    const auto universal = greatest_fixed_point(true);
    std::vector<bool> vertex_existential(vertex_count, false);
    std::vector<bool> vertex_universal(vertex_count, false);
    for (std::size_t id = 0; id < nodes.size(); ++id) {
        if (existential[id]) {
            ++out.existentially_serial_witnesses;
            vertex_existential[nodes[id].vertex] = true;
        }
        if (universal[id]) {
            ++out.universally_serial_witnesses;
            vertex_universal[nodes[id].vertex] = true;
        }
    }
    for (std::size_t v = 0; v < vertex_count; ++v) {
        const auto grade = catalogue.grade(vector_of(v));
        if (grade <= 1) continue;
        out.vertices_with_existential_witness += vertex_existential[v];
        out.vertices_with_universal_witness += vertex_universal[v];
    }
    for (std::size_t v = 0; v < vertex_count; ++v)
        out.required_nonterminal_vertices += catalogue.grade(vector_of(v)) > 1;
    out.existential_complete =
        out.vertices_with_existential_witness == out.required_nonterminal_vertices;
    out.universal_complete =
        out.vertices_with_universal_witness == out.required_nonterminal_vertices;
    return out;
}


struct RelationalGradeSimulation {
    std::size_t shell_vertices = 0;
    std::size_t core_vertices = 0;
    std::size_t initial_pairs = 0;
    std::size_t surviving_pairs = 0;
    std::size_t covered_shell_vertices = 0;
    std::size_t required_shell_vertices = 0;
    bool complete = false;
};

// Greatest forward simulation supported by grade-lowering decompositions.
// Unlike a persistent active atom, the core representative may hand off at
// every step.  The retained relation R satisfies:
//   R(s,c) and s -> s'  ==>  exists c', c -> c' and R(s',c').
// This is the natural phase-aware path projection and is derived by pruning
// unsupported shell/core pairs to a greatest fixed point.
template <class ShellSuccessorFn, class CoreSuccessorFn, class AllowedFn>
RelationalGradeSimulation derive_relational_grade_simulation(
    std::size_t shell_count,
    std::size_t core_count,
    ShellSuccessorFn shell_successors,
    CoreSuccessorFn core_successors,
    AllowedFn allowed) {
    RelationalGradeSimulation out;
    out.shell_vertices = shell_count;
    out.core_vertices = core_count;
    std::vector<std::vector<std::size_t>> shell_out(shell_count);
    std::vector<std::vector<std::size_t>> core_out(core_count);
    for (std::size_t s = 0; s < shell_count; ++s) shell_out[s] = shell_successors(s);
    for (std::size_t c = 0; c < core_count; ++c) core_out[c] = core_successors(c);
    std::vector<std::vector<bool>> live(shell_count,
                                        std::vector<bool>(core_count, false));
    for (std::size_t s = 0; s < shell_count; ++s)
        for (std::size_t c = 0; c < core_count; ++c)
            if (allowed(s, c)) {
                live[s][c] = true;
                ++out.initial_pairs;
            }

    bool changed = true;
    while (changed) {
        changed = false;
        for (std::size_t s = 0; s < shell_count; ++s) {
            for (std::size_t c = 0; c < core_count; ++c) {
                if (!live[s][c]) continue;
                bool keep = !shell_out[s].empty() && !core_out[c].empty();
                for (const auto st : shell_out[s]) {
                    if (st >= shell_count)
                        throw std::invalid_argument("grade simulation: bad shell successor");
                    bool matched = false;
                    for (const auto ct : core_out[c]) {
                        if (ct >= core_count)
                            throw std::invalid_argument("grade simulation: bad core successor");
                        if (live[st][ct]) { matched = true; break; }
                    }
                    if (!matched) { keep = false; break; }
                }
                if (!keep) { live[s][c] = false; changed = true; }
            }
        }
    }
    for (std::size_t s = 0; s < shell_count; ++s) {
        bool covered = false;
        for (std::size_t c = 0; c < core_count; ++c)
            if (live[s][c]) { ++out.surviving_pairs; covered = true; }
        if (covered) ++out.covered_shell_vertices;
    }
    out.required_shell_vertices = shell_count;
    out.complete = out.covered_shell_vertices == shell_count;
    return out;
}


// Bounded-delay variant of the phase-aware simulation.  One source edge may
// be matched by between zero and `max_target_hops` target edges.  This is the
// correct reusable abstraction when source grade/face phase changes faster
// than the selected lower-grade representative.
template <class ShellSuccessorFn, class TargetSuccessorFn, class AllowedFn>
RelationalGradeSimulation derive_bounded_delay_grade_simulation(
    std::size_t shell_count,
    std::size_t target_count,
    std::size_t max_target_hops,
    ShellSuccessorFn shell_successors,
    TargetSuccessorFn target_successors,
    AllowedFn allowed) {
    std::vector<std::vector<std::size_t>> shell_out(shell_count);
    std::vector<std::vector<std::size_t>> target_out(target_count);
    for (std::size_t s = 0; s < shell_count; ++s) shell_out[s] = shell_successors(s);
    for (std::size_t t = 0; t < target_count; ++t) target_out[t] = target_successors(t);

    std::vector<std::vector<std::size_t>> bounded_reach(target_count);
    for (std::size_t source = 0; source < target_count; ++source) {
        std::vector<bool> seen(target_count, false);
        std::vector<std::pair<std::size_t, std::size_t>> queue{{source, 0}};
        seen[source] = true;
        for (std::size_t head = 0; head < queue.size(); ++head) {
            const auto [u, depth] = queue[head];
            bounded_reach[source].push_back(u);
            if (depth == max_target_hops) continue;
            for (const auto v : target_out[u]) {
                if (v >= target_count)
                    throw std::invalid_argument("bounded grade simulation: bad target successor");
                if (!seen[v]) { seen[v] = true; queue.push_back({v, depth + 1}); }
            }
        }
    }

    RelationalGradeSimulation out;
    out.shell_vertices = shell_count;
    out.core_vertices = target_count;
    std::vector<std::vector<bool>> live(shell_count,
                                        std::vector<bool>(target_count, false));
    for (std::size_t s = 0; s < shell_count; ++s)
        for (std::size_t t = 0; t < target_count; ++t)
            if (allowed(s, t)) { live[s][t] = true; ++out.initial_pairs; }
    bool changed = true;
    while (changed) {
        changed = false;
        for (std::size_t s = 0; s < shell_count; ++s)
            for (std::size_t t = 0; t < target_count; ++t) {
                if (!live[s][t]) continue;
                bool keep = !shell_out[s].empty();
                for (const auto sp : shell_out[s]) {
                    bool matched = false;
                    for (const auto tp : bounded_reach[t])
                        if (live[sp][tp]) { matched = true; break; }
                    if (!matched) { keep = false; break; }
                }
                if (!keep) { live[s][t] = false; changed = true; }
            }
    }
    for (std::size_t s = 0; s < shell_count; ++s) {
        bool covered = false;
        for (std::size_t t = 0; t < target_count; ++t)
            if (live[s][t]) { ++out.surviving_pairs; covered = true; }
        out.covered_shell_vertices += covered;
    }
    out.required_shell_vertices = shell_count;
    out.complete = out.covered_shell_vertices == shell_count;
    return out;
}



}  // namespace ravel::proof
