#pragma once

#include <algorithm>
#include <cstddef>
#include <functional>
#include <map>
#include <numeric>
#include <set>
#include <stdexcept>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

#include "ravel/proof/finite_graph_correspondence.hpp"
#include "ravel/proof/graded_core_descent.hpp"

namespace ravel::proof {

using AtomTuple = std::vector<DisplacementDescriptor>;

struct AtomImageComponentSummary {
    std::size_t vertices = 0;
    std::size_t edges = 0;
    std::size_t core_vertices = 0;
    bool permutation = false;
    long double spectral_radius_estimate = 0.0L;
};

struct CompleteAtomTupleExtension {
    std::size_t source_vertices = 0;
    std::size_t source_edges = 0;
    std::size_t decomposition_states = 0;
    std::size_t marked_states = 0;
    std::size_t marked_edges = 0;
    std::size_t live_marked_states = 0;
    std::size_t maximum_decompositions = 0;
    std::size_t maximum_tuple_size = 0;
    std::size_t uncovered_source_vertices = 0;
    std::size_t uncovered_source_edges = 0;
    std::size_t full_atom_vertices = 0;
    std::size_t full_atom_edges = 0;
    std::size_t image_atom_vertices = 0;
    std::size_t image_atom_edges = 0;
    std::size_t recurrent_image_components = 0;
    std::size_t recurrent_image_vertices = 0;
    std::size_t recurrent_image_vertices_outside_core = 0;
    bool atom_image_recurrent_kernel_in_core = false;
    std::vector<AtomImageComponentSummary> recurrent_image_summaries;
    bool source_path_surjective = false;
    FiniteGraphCorrespondenceEntropyBound entropy;
    bool spectral_radius_at_most_full_atom = false;
    bool spectral_radius_at_most_core = false;
    std::string obstruction;
};

namespace complete_atom_tuple_detail {

inline std::vector<long long> subtract(
    const std::vector<long long>& lhs,
    const std::vector<long long>& rhs) {
    std::vector<long long> out(lhs.size());
    for (std::size_t k = 0; k < lhs.size(); ++k) out[k] = lhs[k] - rhs[k];
    return out;
}

inline bool zero_vector(const std::vector<long long>& x) {
    return std::all_of(x.begin(), x.end(), [](long long v) { return v == 0; });
}

inline std::vector<AtomTuple> exact_atom_decompositions(
    const DisplacementSumCatalogue& catalogue,
    const std::vector<long long>& x) {
    const auto grade = catalogue.grade(x);
    if (grade == 0) return {};
    const auto& described = catalogue.described_atoms();
    std::vector<AtomTuple> result;
    AtomTuple tuple;
    std::function<void(std::size_t, std::size_t, const std::vector<long long>&)> rec;
    rec = [&](std::size_t start, std::size_t remaining,
              const std::vector<long long>& residual) {
        if (remaining == 0) {
            if (zero_vector(residual)) result.push_back(tuple);
            return;
        }
        for (std::size_t a = start; a < described.size(); ++a) {
            tuple.push_back(described[a].first);
            rec(a, remaining - 1, subtract(residual, described[a].second));
            tuple.pop_back();
        }
    };
    rec(0, grade, x);
    std::sort(result.begin(), result.end());
    result.erase(std::unique(result.begin(), result.end()), result.end());
    return result;
}

struct TransportedTuple {
    AtomTuple target;
    std::vector<std::size_t> destination_of_source_atom;
    std::vector<long long> local_deltas;
};

inline std::vector<TransportedTuple> transport_tuple(
    std::size_t n, const AtomTuple& source, long long global_delta) {
    struct Choice {
        DisplacementDescriptor descriptor;
        long long delta = 0;
        std::size_t source_index = 0;
    };
    std::vector<TransportedTuple> result;
    std::vector<Choice> choices;
    std::function<void(std::size_t, long long)> rec;
    rec = [&](std::size_t index, long long sum) {
        if (index == source.size()) {
            if (sum != global_delta) return;
            auto sorted = choices;
            std::sort(sorted.begin(), sorted.end(), [](const Choice& a, const Choice& b) {
                if (a.descriptor != b.descriptor) return a.descriptor < b.descriptor;
                return a.source_index < b.source_index;
            });
            TransportedTuple transported;
            transported.destination_of_source_atom.resize(source.size());
            transported.local_deltas.resize(source.size());
            for (std::size_t k = 0; k < sorted.size(); ++k) {
                transported.target.push_back(sorted[k].descriptor);
                transported.destination_of_source_atom[sorted[k].source_index] = k;
                transported.local_deltas[sorted[k].source_index] = sorted[k].delta;
            }
            result.push_back(std::move(transported));
            return;
        }
        for (long long delta : {-1LL, 0LL, 1LL}) {
            const auto next = nbonacci_margin::forward_descriptor(n, source[index], delta);
            if (!next) continue;
            choices.push_back(Choice{*next, delta, index});
            rec(index + 1, sum + delta);
            choices.pop_back();
        }
    };
    rec(0, 0);
    std::sort(result.begin(), result.end(), [](const auto& a, const auto& b) {
        return std::tie(a.target, a.destination_of_source_atom, a.local_deltas) <
               std::tie(b.target, b.destination_of_source_atom, b.local_deltas);
    });
    result.erase(std::unique(result.begin(), result.end(), [](const auto& a, const auto& b) {
        return a.target == b.target &&
               a.destination_of_source_atom == b.destination_of_source_atom &&
               a.local_deltas == b.local_deltas;
    }), result.end());
    return result;
}

struct AtomState {
    LabelPair pair;
    DisplacementDescriptor descriptor;
    auto operator<=>(const AtomState&) const = default;
};

struct AtomEdgeKey {
    AtomState source;
    AtomState destination;
    long long local_delta = 0;
    auto operator<=>(const AtomEdgeKey&) const = default;
};

struct FullAtomAutomaton {
    std::vector<AtomState> states;
    std::map<AtomState, std::size_t> state_id;
    std::vector<std::pair<std::size_t, std::size_t>> edges;
    std::map<AtomEdgeKey, std::size_t> edge_id;
};

inline FullAtomAutomaton full_atom_automaton(std::size_t n) {
    FullAtomAutomaton out;
    for (const auto pair : nbonacci_margin::label_pairs(n))
        for (const auto descriptor : nbonacci_margin::displacement_descriptors(n)) {
            const AtomState state{pair, descriptor};
            out.state_id.emplace(state, out.states.size());
            out.states.push_back(state);
        }
    for (const auto transition : nbonacci_margin::label_transitions(n)) {
        for (const auto descriptor : nbonacci_margin::displacement_descriptors(n)) {
            for (long long local_delta : {-1LL, 0LL, 1LL}) {
                const auto next = nbonacci_margin::forward_descriptor(
                    n, descriptor, local_delta);
                if (!next) continue;
                const AtomState source{transition.source, descriptor};
                const AtomState destination{transition.destination, *next};
                const auto source_id = out.state_id.at(source);
                const auto destination_id = out.state_id.at(destination);
                const AtomEdgeKey key{source, destination, local_delta};
                if (out.edge_id.count(key)) continue;
                out.edge_id.emplace(key, out.edges.size());
                out.edges.push_back({source_id, destination_id});
            }
        }
    }
    return out;
}

} // namespace complete_atom_tuple_detail

/** Complete ordered-decomposition lift over the full one-particle automaton.
 *
 * Unlike one-stage grade descent, every exact atom decomposition is retained.
 * A concrete carry defect is distributed among all atoms, subject only to
 * exact sparse-atom transport and conservation of the total defect.  Atom
 * identities persist through canonical tuple sorting, so a marked atom may
 * leave the recurrent core, pass through the grade-one halo, and later return.
 */
template <class SuccessorFn, class PairFn, class VectorFn>
CompleteAtomTupleExtension derive_complete_atom_tuple_extension(
    const DisplacementSumCatalogue& catalogue,
    std::size_t vertex_count,
    SuccessorFn successors,
    PairFn pair_of,
    VectorFn vector_of) {
    using namespace complete_atom_tuple_detail;
    CompleteAtomTupleExtension out;
    out.source_vertices = vertex_count;
    const auto n = catalogue.dimension();
    if (vertex_count == 0) {
        out.obstruction = "complete atom tuple extension: empty source graph";
        return out;
    }

    std::vector<std::vector<std::size_t>> source_out(vertex_count);
    std::vector<std::vector<AtomTuple>> decompositions(vertex_count);
    std::vector<std::map<AtomTuple, std::size_t>> decomposition_id(vertex_count);
    for (std::size_t u = 0; u < vertex_count; ++u) {
        source_out[u] = successors(u);
        out.source_edges += source_out[u].size();
        decompositions[u] = exact_atom_decompositions(catalogue, vector_of(u));
        out.maximum_decompositions = std::max(
            out.maximum_decompositions, decompositions[u].size());
        out.uncovered_source_vertices += decompositions[u].empty();
        for (std::size_t d = 0; d < decompositions[u].size(); ++d) {
            decomposition_id[u].emplace(decompositions[u][d], d);
            ++out.decomposition_states;
            out.maximum_tuple_size = std::max(
                out.maximum_tuple_size, decompositions[u][d].size());
        }
    }
    if (out.uncovered_source_vertices) {
        out.obstruction = "complete atom tuple extension: source state has no exact atom decomposition";
        return out;
    }

    struct MarkedState {
        std::size_t source = 0;
        std::size_t decomposition = 0;
        std::size_t mark = 0;
    };
    std::vector<MarkedState> marked;
    std::vector<std::vector<std::vector<std::size_t>>> marked_id(vertex_count);
    for (std::size_t u = 0; u < vertex_count; ++u) {
        marked_id[u].resize(decompositions[u].size());
        for (std::size_t d = 0; d < decompositions[u].size(); ++d) {
            marked_id[u][d].resize(decompositions[u][d].size());
            for (std::size_t m = 0; m < decompositions[u][d].size(); ++m) {
                marked_id[u][d][m] = marked.size();
                marked.push_back({u, d, m});
            }
        }
    }
    out.marked_states = marked.size();

    std::vector<std::pair<std::size_t, std::size_t>> source_edges;
    std::vector<std::vector<std::size_t>> source_edge_ids(vertex_count);
    for (std::size_t u = 0; u < vertex_count; ++u)
        for (const auto v : source_out[u]) {
            if (v >= vertex_count)
                throw std::invalid_argument("complete atom tuple extension: bad successor");
            source_edge_ids[u].push_back(source_edges.size());
            source_edges.push_back({u, v});
        }

    const auto atom_automaton = full_atom_automaton(n);
    out.full_atom_vertices = atom_automaton.states.size();
    out.full_atom_edges = atom_automaton.edges.size();

    std::vector<std::vector<CorrespondenceEdge>> to_source(marked.size());
    std::vector<std::vector<CorrespondenceEdge>> to_atom(marked.size());
    for (std::size_t id = 0; id < marked.size(); ++id) {
        const auto state = marked[id];
        const auto u = state.source;
        const auto& tuple = decompositions[u][state.decomposition];
        const auto pair_u = pair_of(u);
        for (std::size_t epos = 0; epos < source_out[u].size(); ++epos) {
            const auto v = source_out[u][epos];
            const auto pair_v = pair_of(v);
            const long long global_delta =
                (pair_u.j > 0 ? 1LL : 0LL) - (pair_u.i > 0 ? 1LL : 0LL);
            for (const auto& transported : transport_tuple(n, tuple, global_delta)) {
                const auto target_it = decomposition_id[v].find(transported.target);
                if (target_it == decomposition_id[v].end()) continue;
                const auto target_decomposition = target_it->second;
                const auto target_mark = transported.destination_of_source_atom[state.mark];
                const auto destination = marked_id[v][target_decomposition][target_mark];
                const auto source_edge = source_edge_ids[u][epos];
                to_source[id].push_back({destination, source_edge});

                const AtomState atom_source{pair_u, tuple[state.mark]};
                const AtomState atom_destination{
                    pair_v, transported.target[target_mark]};
                const AtomEdgeKey atom_key{
                    atom_source, atom_destination,
                    transported.local_deltas[state.mark]};
                const auto atom_edge = atom_automaton.edge_id.find(atom_key);
                if (atom_edge == atom_automaton.edge_id.end())
                    throw std::logic_error(
                        "complete atom tuple extension: missing full-atom edge");
                to_atom[id].push_back({destination, atom_edge->second});
                ++out.marked_edges;
            }
        }
    }

    // Greatest subgraph locally surjective over every concrete source edge.
    std::vector<bool> live(marked.size(), true);
    bool changed = true;
    while (changed) {
        changed = false;
        for (std::size_t id = 0; id < marked.size(); ++id) {
            if (!live[id]) continue;
            const auto u = marked[id].source;
            bool keep = true;
            for (const auto source_edge : source_edge_ids[u]) {
                bool found = false;
                for (const auto& edge : to_source[id])
                    if (edge.target_edge == source_edge && live[edge.destination]) {
                        found = true;
                        break;
                    }
                if (!found) {
                    keep = false;
                    break;
                }
            }
            if (!keep) {
                live[id] = false;
                changed = true;
            }
        }
    }

    std::vector<std::size_t> remap(marked.size(), static_cast<std::size_t>(-1));
    for (std::size_t id = 0; id < marked.size(); ++id)
        if (live[id]) remap[id] = out.live_marked_states++;
    if (out.live_marked_states == 0) {
        out.obstruction = "complete atom tuple extension: decomposition lift has no recurrently usable states";
        return out;
    }

    std::vector<std::size_t> source_vertex_map(out.live_marked_states);
    std::vector<std::size_t> atom_vertex_map(out.live_marked_states);
    std::vector<std::vector<CorrespondenceEdge>> source_domain(out.live_marked_states);
    std::vector<std::vector<CorrespondenceEdge>> atom_domain(out.live_marked_states);
    for (std::size_t id = 0; id < marked.size(); ++id) {
        if (!live[id]) continue;
        const auto new_id = remap[id];
        const auto state = marked[id];
        const auto& tuple = decompositions[state.source][state.decomposition];
        source_vertex_map[new_id] = state.source;
        atom_vertex_map[new_id] = atom_automaton.state_id.at(
            AtomState{pair_of(state.source), tuple[state.mark]});
        for (const auto& edge : to_source[id])
            if (live[edge.destination])
                source_domain[new_id].push_back(
                    {remap[edge.destination], edge.target_edge});
        for (const auto& edge : to_atom[id])
            if (live[edge.destination])
                atom_domain[new_id].push_back(
                    {remap[edge.destination], edge.target_edge});
    }

    out.source_path_surjective = true;
    for (std::size_t u = 0; u < vertex_count; ++u) {
        bool covered = false;
        for (std::size_t id = 0; id < marked.size(); ++id)
            if (live[id] && marked[id].source == u) {
                covered = true;
                break;
            }
        if (!covered) ++out.uncovered_source_edges;
        out.source_path_surjective = out.source_path_surjective && covered;
    }

    // Restrict the one-particle target to the exact image used by the live
    // tuple correspondence.  The unrestricted halo deliberately permits
    // unrelated local defect choices and is strongly connected; only this
    // image retains the synchronized defect budget of the source component.
    std::set<std::size_t> used_atom_vertices(atom_vertex_map.begin(), atom_vertex_map.end());
    std::set<std::size_t> used_atom_edge_ids;
    for (const auto& edges : atom_domain)
        for (const auto& edge : edges) used_atom_edge_ids.insert(edge.target_edge);
    out.image_atom_vertices = used_atom_vertices.size();
    out.image_atom_edges = used_atom_edge_ids.size();
    std::map<std::size_t, std::size_t> image_vertex_id;
    std::vector<std::size_t> image_vertex_global;
    for (const auto global : used_atom_vertices) {
        image_vertex_id.emplace(global, image_vertex_global.size());
        image_vertex_global.push_back(global);
    }
    std::vector<std::pair<std::size_t, std::size_t>> image_edges;
    std::map<std::size_t, std::size_t> image_edge_id;
    for (const auto global_edge : used_atom_edge_ids) {
        const auto [a, b] = atom_automaton.edges[global_edge];
        if (!image_vertex_id.count(a) || !image_vertex_id.count(b)) continue;
        image_edge_id.emplace(global_edge, image_edges.size());
        image_edges.push_back({image_vertex_id.at(a), image_vertex_id.at(b)});
    }
    std::vector<std::size_t> image_atom_vertex_map(atom_vertex_map.size());
    std::vector<std::vector<CorrespondenceEdge>> image_atom_domain(atom_domain.size());
    for (std::size_t u = 0; u < atom_vertex_map.size(); ++u) {
        image_atom_vertex_map[u] = image_vertex_id.at(atom_vertex_map[u]);
        for (const auto& edge : atom_domain[u]) {
            const auto it = image_edge_id.find(edge.target_edge);
            if (it != image_edge_id.end())
                image_atom_domain[u].push_back({edge.destination, it->second});
        }
    }

    std::vector<std::vector<std::size_t>> image_out(image_vertex_global.size());
    for (const auto [a, b] : image_edges) image_out[a].push_back(b);
    for (const auto& component : correspondence_detail::sccs(image_out)) {
        std::set<std::size_t> inside(component.begin(), component.end());
        bool recurrent = component.size() > 1;
        if (component.size() == 1)
            for (const auto v : image_out[component.front()])
                recurrent = recurrent || v == component.front();
        if (!recurrent) continue;
        ++out.recurrent_image_components;
        out.recurrent_image_vertices += component.size();
        AtomImageComponentSummary summary;
        summary.vertices = component.size();
        std::map<std::size_t, std::size_t> local_id;
        for (std::size_t k = 0; k < component.size(); ++k)
            local_id.emplace(component[k], k);
        std::vector<std::vector<std::size_t>> component_out(component.size());
        std::vector<std::size_t> indegree(component.size(), 0);
        for (std::size_t k = 0; k < component.size(); ++k) {
            const auto local = component[k];
            const auto& state = atom_automaton.states[image_vertex_global[local]];
            if (nbonacci_margin::predicted_core_member(
                    n, state.pair, state.descriptor))
                ++summary.core_vertices;
            else
                ++out.recurrent_image_vertices_outside_core;
            for (const auto destination : image_out[local]) {
                const auto it = local_id.find(destination);
                if (it == local_id.end()) continue;
                component_out[k].push_back(it->second);
                ++indegree[it->second];
                ++summary.edges;
            }
        }
        summary.permutation = true;
        for (std::size_t k = 0; k < component.size(); ++k)
            summary.permutation = summary.permutation &&
                component_out[k].size() == 1 && indegree[k] == 1;
        std::vector<long double> value(component.size(), 1.0L);
        long double rho = 0.0L;
        for (std::size_t iteration = 0; iteration < 256; ++iteration) {
            std::vector<long double> next(component.size(), 0.0L);
            for (std::size_t u = 0; u < component.size(); ++u)
                for (const auto v : component_out[u]) next[u] += value[v];
            long double norm = 0.0L;
            for (const auto x : next) norm = std::max(norm, x);
            if (norm == 0.0L) break;
            for (auto& x : next) x /= norm;
            value.swap(next);
            rho = norm;
        }
        summary.spectral_radius_estimate = rho;
        out.recurrent_image_summaries.push_back(summary);
    }
    out.atom_image_recurrent_kernel_in_core =
        out.recurrent_image_vertices_outside_core == 0;

    auto source_map = derive_finite_to_one_graph_map(
        source_vertex_map, vertex_count, source_domain, source_edges);
    auto atom_map = derive_finite_to_one_graph_map(
        image_atom_vertex_map, image_vertex_global.size(), image_atom_domain,
        image_edges);
    out.entropy.to_source = std::move(source_map);
    out.entropy.to_target = std::move(atom_map);
    out.entropy.source_path_surjective = out.source_path_surjective;
    // For h(source) <= h(target), the correspondence need only be
    // path-surjective over the source and finite-to-one over the target.
    // Branching among multiple decomposition lifts of one source path is
    // harmless: it enlarges the intermediate language, not the source.
    out.entropy.entropy_source_at_most_target =
        out.source_path_surjective && out.entropy.to_target.finite_to_one;
    if (!out.entropy.entropy_source_at_most_target)
        out.entropy.obstruction =
            "complete atom tuple extension: source is not path-covered or atom projection branches recurrently";
    out.spectral_radius_at_most_full_atom =
        out.entropy.entropy_source_at_most_target;
    out.spectral_radius_at_most_core =
        out.spectral_radius_at_most_full_atom &&
        out.atom_image_recurrent_kernel_in_core;
    if (!out.spectral_radius_at_most_core) {
        if (!out.spectral_radius_at_most_full_atom)
            out.obstruction = out.entropy.obstruction;
        else
            out.obstruction =
                "complete atom tuple extension: recurrent atom image leaves the predicted core";
    }
    return out;
}


struct GaugedAtomTupleCoreCorrespondence {
    std::size_t source_vertices = 0;
    std::size_t tuple_mark_states = 0;
    std::size_t gauged_states = 0;
    std::size_t gauged_edges = 0;
    std::size_t live_gauged_states = 0;
    std::size_t uncovered_source_vertices = 0;
    bool source_path_surjective = false;
    FiniteToOneGraphMap to_core;
    bool spectral_radius_at_most_core = false;
    std::string obstruction;
};

/** Gauge each marked atom by an independently chosen core-compatible label
 *  chart.  The global component labels and the atom's core chart are allowed
 *  to differ; core-chart transport is constrained only by the exact
 *  predicted-core successor grammar.  This is the finite groupoid twist
 *  missing from same-pair grade descent.
 */
template <class SuccessorFn, class PairFn, class VectorFn>
GaugedAtomTupleCoreCorrespondence derive_gauged_atom_tuple_core_correspondence(
    const DisplacementSumCatalogue& catalogue,
    std::size_t vertex_count,
    SuccessorFn successors,
    PairFn pair_of,
    VectorFn vector_of) {
    using namespace complete_atom_tuple_detail;
    GaugedAtomTupleCoreCorrespondence out;
    out.source_vertices = vertex_count;
    const auto n = catalogue.dimension();
    if (!vertex_count) {
        out.obstruction = "gauged tuple correspondence: empty source";
        return out;
    }

    std::vector<std::vector<std::size_t>> source_out(vertex_count);
    std::vector<std::vector<AtomTuple>> decompositions(vertex_count);
    std::vector<std::map<AtomTuple, std::size_t>> decomposition_id(vertex_count);
    for (std::size_t u = 0; u < vertex_count; ++u) {
        source_out[u] = successors(u);
        decompositions[u] = exact_atom_decompositions(catalogue, vector_of(u));
        if (decompositions[u].empty()) ++out.uncovered_source_vertices;
        for (std::size_t d = 0; d < decompositions[u].size(); ++d)
            decomposition_id[u].emplace(decompositions[u][d], d);
    }
    if (out.uncovered_source_vertices) {
        out.obstruction = "gauged tuple correspondence: source state has no exact decomposition";
        return out;
    }

    struct TupleMark { std::size_t source, decomposition, mark; };
    std::vector<TupleMark> tuple_marks;
    std::vector<std::vector<std::vector<std::size_t>>> tuple_mark_id(vertex_count);
    for (std::size_t u = 0; u < vertex_count; ++u) {
        tuple_mark_id[u].resize(decompositions[u].size());
        for (std::size_t d = 0; d < decompositions[u].size(); ++d) {
            tuple_mark_id[u][d].resize(decompositions[u][d].size());
            for (std::size_t m = 0; m < decompositions[u][d].size(); ++m) {
                tuple_mark_id[u][d][m] = tuple_marks.size();
                tuple_marks.push_back({u, d, m});
            }
        }
    }
    out.tuple_mark_states = tuple_marks.size();

    std::vector<std::pair<std::size_t, std::size_t>> source_edges;
    std::vector<std::vector<std::size_t>> source_edge_ids(vertex_count);
    for (std::size_t u = 0; u < vertex_count; ++u)
        for (const auto v : source_out[u]) {
            if (v >= vertex_count)
                throw std::invalid_argument("gauged tuple correspondence: bad successor");
            source_edge_ids[u].push_back(source_edges.size());
            source_edges.push_back({u, v});
        }

    std::vector<nbonacci_margin::CoreState> core_states;
    std::map<nbonacci_margin::CoreState, std::size_t> core_id;
    std::map<DisplacementDescriptor, std::vector<std::size_t>> gauges;
    for (const auto pair : nbonacci_margin::label_pairs(n))
        for (const auto descriptor : nbonacci_margin::displacement_descriptors(n))
            if (nbonacci_margin::predicted_core_member(n, pair, descriptor)) {
                const nbonacci_margin::CoreState state{pair, descriptor};
                const auto id = core_states.size();
                core_id.emplace(state, id);
                core_states.push_back(state);
                gauges[descriptor].push_back(id);
            }
    std::vector<std::pair<std::size_t, std::size_t>> core_edges;
    std::map<std::pair<std::size_t, std::size_t>, std::size_t> core_edge_id;
    for (std::size_t c = 0; c < core_states.size(); ++c)
        for (const auto target : nbonacci_margin::predicted_core_successors(
                 n, core_states[c])) {
            const auto t = core_id.at(target);
            core_edge_id.emplace(std::make_pair(c, t), core_edges.size());
            core_edges.push_back({c, t});
        }

    struct GaugedState { std::size_t tuple_mark, core; };
    std::vector<GaugedState> gauged;
    std::vector<std::vector<std::size_t>> gauged_ids(tuple_marks.size());
    for (std::size_t tm = 0; tm < tuple_marks.size(); ++tm) {
        const auto mark = tuple_marks[tm];
        const auto descriptor =
            decompositions[mark.source][mark.decomposition][mark.mark];
        for (const auto c : gauges[descriptor]) {
            gauged_ids[tm].push_back(gauged.size());
            gauged.push_back({tm, c});
        }
    }
    out.gauged_states = gauged.size();

    std::vector<std::vector<CorrespondenceEdge>> to_source(gauged.size());
    std::vector<std::vector<CorrespondenceEdge>> to_core(gauged.size());
    for (std::size_t g = 0; g < gauged.size(); ++g) {
        const auto [tm, core_source] = gauged[g];
        const auto mark = tuple_marks[tm];
        const auto u = mark.source;
        const auto& tuple = decompositions[u][mark.decomposition];
        const auto pair_u = pair_of(u);
        for (std::size_t epos = 0; epos < source_out[u].size(); ++epos) {
            const auto v = source_out[u][epos];
            const long long global_delta =
                (pair_u.j > 0 ? 1LL : 0LL) - (pair_u.i > 0 ? 1LL : 0LL);
            for (const auto& transported : transport_tuple(n, tuple, global_delta)) {
                const auto target_it = decomposition_id[v].find(transported.target);
                if (target_it == decomposition_id[v].end()) continue;
                const auto target_mark = transported.destination_of_source_atom[mark.mark];
                const auto target_tm = tuple_mark_id[v][target_it->second][target_mark];
                for (const auto target_g : gauged_ids[target_tm]) {
                    const auto core_target = gauged[target_g].core;
                    const auto ce = core_edge_id.find({core_source, core_target});
                    if (ce == core_edge_id.end()) continue;
                    to_source[g].push_back(
                        {target_g, source_edge_ids[u][epos]});
                    to_core[g].push_back({target_g, ce->second});
                    ++out.gauged_edges;
                }
            }
        }
    }

    // Retain the greatest subgraph that covers every concrete source edge.
    std::vector<bool> live(gauged.size(), true);
    bool changed = true;
    while (changed) {
        changed = false;
        for (std::size_t g = 0; g < gauged.size(); ++g) {
            if (!live[g]) continue;
            const auto u = tuple_marks[gauged[g].tuple_mark].source;
            bool keep = true;
            for (const auto se : source_edge_ids[u]) {
                bool found = false;
                for (const auto& edge : to_source[g])
                    if (edge.target_edge == se && live[edge.destination]) {
                        found = true;
                        break;
                    }
                if (!found) { keep = false; break; }
            }
            if (!keep) { live[g] = false; changed = true; }
        }
    }

    std::vector<std::size_t> remap(gauged.size(), static_cast<std::size_t>(-1));
    for (std::size_t g = 0; g < gauged.size(); ++g)
        if (live[g]) remap[g] = out.live_gauged_states++;
    if (!out.live_gauged_states) {
        out.obstruction = "gauged tuple correspondence: no live gauged states";
        return out;
    }

    std::vector<std::size_t> core_vertex_map(out.live_gauged_states);
    std::vector<std::vector<CorrespondenceEdge>> core_domain(out.live_gauged_states);
    for (std::size_t g = 0; g < gauged.size(); ++g) {
        if (!live[g]) continue;
        const auto ng = remap[g];
        core_vertex_map[ng] = gauged[g].core;
        for (const auto& edge : to_core[g])
            if (live[edge.destination])
                core_domain[ng].push_back(
                    {remap[edge.destination], edge.target_edge});
    }

    out.source_path_surjective = true;
    for (std::size_t u = 0; u < vertex_count; ++u) {
        bool covered = false;
        for (std::size_t g = 0; g < gauged.size(); ++g)
            if (live[g] && tuple_marks[gauged[g].tuple_mark].source == u) {
                covered = true;
                break;
            }
        if (!covered) ++out.uncovered_source_vertices;
        out.source_path_surjective = out.source_path_surjective && covered;
    }
    out.to_core = derive_finite_to_one_graph_map(
        core_vertex_map, core_states.size(), core_domain, core_edges);
    out.spectral_radius_at_most_core =
        out.source_path_surjective && out.to_core.finite_to_one;
    if (!out.spectral_radius_at_most_core) {
        if (!out.source_path_surjective)
            out.obstruction = "gauged tuple correspondence: source path coverage failed";
        else
            out.obstruction = out.to_core.obstruction;
    }
    return out;
}

} // namespace ravel::proof
