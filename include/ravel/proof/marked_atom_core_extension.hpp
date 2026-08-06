#pragma once

#include <cstddef>
#include <map>
#include <set>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include "ravel/proof/graded_core_descent.hpp"

namespace ravel::proof {

struct MarkedAtomCoreExtension {
    std::size_t source_vertices = 0;
    std::size_t marked_states = 0;
    std::size_t concrete_edge_occurrences = 0;
    std::size_t marked_edge_occurrences = 0;
    std::size_t maximum_fibre = 0;
    std::size_t uncovered_vertices = 0;
    std::size_t nondeterministic_mark_transports = 0;
    std::size_t noninjective_core_branches = 0;
    bool decomposition_replayed = false;
    bool partial_permutation_extension = false;
    bool path_injection_to_core = false;
    std::string obstruction;
};

namespace marked_atom_detail {

inline std::vector<GradeDescentWitness> complete_marked_fibre(
    const DisplacementSumCatalogue& catalogue,
    LabelPair pair,
    const std::vector<long long>& x) {
    auto result = catalogue.derive_one_stage(pair, x);
    if (!result.empty()) return result;
    const auto descriptor = nbonacci_margin::describe_displacement(x);
    if (!descriptor ||
        !nbonacci_margin::predicted_core_member(
            catalogue.dimension(), pair, *descriptor))
        return {};
    result.push_back(GradeDescentWitness{
        1, 0, *descriptor, x,
        std::vector<long long>(x.size(), 0),
        GradeDescentPhase{}});
    return result;
}

inline bool same_mark_transport(
    std::size_t n,
    LabelPair source_pair,
    const GradeDescentWitness& source,
    LabelPair destination_pair,
    const GradeDescentWitness& destination) {
    const nbonacci_margin::CoreState active_source{
        source_pair, source.active_descriptor};
    const nbonacci_margin::CoreState active_destination{
        destination_pair, destination.active_descriptor};
    if (!nbonacci_margin::predicted_core_successors(n, active_source)
             .count(active_destination))
        return false;
    return destination.remainder ==
        nbonacci_margin::forward_displacement(source.remainder, 0);
}

} // namespace marked_atom_detail

/** Prove that a recurrent component is a finite partial-permutation extension
 *  of the predicted core after one elementary atom is marked.
 *
 *  `successors(u)` must retain edge multiplicity by repeating destinations.
 *  For each marked decomposition and each concrete edge occurrence, the mark
 *  must have exactly one transported continuation.  Distinct concrete edge
 *  occurrences from a marked state must project to distinct core edges.  A
 *  source path plus one initial mark therefore determines a unique core path,
 *  giving an injection of source path languages into the predicted core.
 */
template <class SuccessorFn, class PairFn, class VectorFn>
MarkedAtomCoreExtension derive_marked_atom_core_extension(
    const DisplacementSumCatalogue& catalogue,
    std::size_t vertex_count,
    SuccessorFn successors,
    PairFn pair_of,
    VectorFn vector_of) {
    MarkedAtomCoreExtension out;
    out.source_vertices = vertex_count;
    const auto n = catalogue.dimension();

    std::vector<std::vector<GradeDescentWitness>> fibres(vertex_count);
    std::vector<std::vector<std::size_t>> outgoing(vertex_count);
    for (std::size_t u = 0; u < vertex_count; ++u) {
        fibres[u] = marked_atom_detail::complete_marked_fibre(
            catalogue, pair_of(u), vector_of(u));
        outgoing[u] = successors(u);
        out.marked_states += fibres[u].size();
        out.maximum_fibre = std::max(out.maximum_fibre, fibres[u].size());
        out.concrete_edge_occurrences += outgoing[u].size();
        out.uncovered_vertices += fibres[u].empty();
    }
    if (out.uncovered_vertices) {
        out.obstruction = "marked-atom extension: some states have no core atom decomposition";
        return out;
    }

    out.decomposition_replayed = true;
    for (std::size_t u = 0; u < vertex_count; ++u) {
        const auto pair_u = pair_of(u);
        const auto x_u = vector_of(u);
        const long long delta =
            (pair_u.j > 0 ? 1LL : 0LL) - (pair_u.i > 0 ? 1LL : 0LL);
        for (const auto v : outgoing[u]) {
            if (v >= vertex_count)
                throw std::invalid_argument("marked-atom extension: bad successor");
            if (nbonacci_margin::forward_displacement(x_u, delta) != vector_of(v))
                out.decomposition_replayed = false;
        }
    }
    if (!out.decomposition_replayed) {
        out.obstruction = "marked-atom extension: affine carry replay failed";
        return out;
    }

    for (std::size_t u = 0; u < vertex_count; ++u) {
        for (const auto& mark : fibres[u]) {
            std::set<nbonacci_margin::CoreState> projected_destinations;
            for (const auto v : outgoing[u]) {
                std::vector<std::size_t> compatible;
                for (std::size_t w = 0; w < fibres[v].size(); ++w)
                    if (marked_atom_detail::same_mark_transport(
                            n, pair_of(u), mark, pair_of(v), fibres[v][w]))
                        compatible.push_back(w);
                if (compatible.size() != 1) {
                    ++out.nondeterministic_mark_transports;
                    continue;
                }
                ++out.marked_edge_occurrences;
                const auto& next = fibres[v][compatible.front()];
                const nbonacci_margin::CoreState core_destination{
                    pair_of(v), next.active_descriptor};
                if (!projected_destinations.insert(core_destination).second)
                    ++out.noninjective_core_branches;
            }
        }
    }

    out.partial_permutation_extension =
        out.nondeterministic_mark_transports == 0;
    out.path_injection_to_core = out.partial_permutation_extension &&
        out.noninjective_core_branches == 0;
    if (!out.path_injection_to_core) {
        if (out.nondeterministic_mark_transports)
            out.obstruction = "marked-atom extension: mark transport is not a partial permutation";
        else
            out.obstruction = "marked-atom extension: distinct source branches collapse to one core branch";
    }
    return out;
}

} // namespace ravel::proof
