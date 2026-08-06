#pragma once

#include <cstddef>
#include <cstdlib>
#include <map>
#include <set>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

#include "ravel/contact_boundary.hpp"
#include "ravel/corona.hpp"
#include "ravel/proof/generalized_multinacci_prefix_phase.hpp"

namespace ravel::proof {

template <std::size_t d>
struct MultinacciBoundaryWordEdge {
    std::size_t source = 0;
    std::size_t target = 0;
    int backward_type = 1;
    std::size_t left_prefix_position = 0;
    std::size_t right_prefix_position = 0;
    long long signed_prefix_defect = 0;
    std::vector<PrefixPrimitiveGenerator> word;
};

template <std::size_t d>
struct MultinacciBoundaryWordLiftProof {
    std::size_t dimension = d;
    std::size_t multiplicity = 0;
    std::vector<MultinacciBoundaryWordEdge<d>> edges;
    std::map<std::string, std::size_t> word_multiplicity;
    std::vector<std::vector<long long>> projected_adjacency;
    bool every_edge_has_prefix_witness = false;
    bool every_word_is_positive_qr = false;
    bool signed_side_twist_retained = false;
    bool projection_equals_existing_boundary_graph = false;
    bool proved = false;
    std::string obstruction;
};

inline std::vector<PrefixPrimitiveGenerator>
prefix_defect_word(long long signed_defect) {
    const auto magnitude = static_cast<std::size_t>(std::llabs(signed_defect));
    if (magnitude == 0)
        return {PrefixPrimitiveGenerator::balanced_q};
    return std::vector<PrefixPrimitiveGenerator>(
        magnitude, PrefixPrimitiveGenerator::residual_r);
}

template <std::size_t d>
MultinacciBoundaryWordLiftProof<d>
derive_generalized_multinacci_boundary_word_lift(
    const Substitution<d>& substitution,
    const ContactBoundaryReport& report,
    std::size_t multiplicity) {
    MultinacciBoundaryWordLiftProof<d> out;
    out.multiplicity = multiplicity;
    if (report.boundary_nodes.size() != report.gb_matrix.size()) {
        out.obstruction = "boundary node/matrix dimension mismatch";
        return out;
    }
    const std::size_t n = report.boundary_nodes.size();
    out.projected_adjacency.assign(n, std::vector<long long>(n, 0));

    std::map<ANode<d>, std::size_t> index;
    for (std::size_t k = 0; k < n; ++k) {
        const auto& raw = report.boundary_nodes[k];
        const auto& xv = std::get<1>(raw);
        if (xv.size() != d) {
            out.obstruction = "boundary node has wrong displacement dimension";
            return out;
        }
        ANode<d> node;
        node.i = std::get<0>(raw);
        node.j = std::get<2>(raw);
        for (std::size_t r = 0; r < d; ++r) node.x[r] = xv[r];
        index[node] = k;
    }

    out.every_edge_has_prefix_witness = true;
    out.every_word_is_positive_qr = true;
    out.signed_side_twist_retained = true;
    for (const auto& [source_node, source] : index) {
        SNode<d> simple_source;
        simple_source.i = source_node.i;
        simple_source.j = source_node.j;
        simple_source.x = source_node.x;
        for (const auto& [destination, prefixes] :
             simple_forward_targets_exact<d>(substitution, simple_source)) {
            ANode<d> destination_key;
            destination_key.i = destination.i;
            destination_key.j = destination.j;
            destination_key.x = destination.x;
            const auto target_it = index.find(destination_key);
            if (target_it == index.end()) continue;
            const auto left_position = prefixes.first.size();
            const auto right_position = prefixes.second.size();
            const long long signed_defect =
                static_cast<long long>(right_position) -
                static_cast<long long>(left_position);
            MultinacciBoundaryWordEdge<d> edge;
            edge.source = source;
            edge.target = target_it->second;
            edge.backward_type = 1;
            edge.left_prefix_position = left_position;
            edge.right_prefix_position = right_position;
            edge.signed_prefix_defect = signed_defect;
            edge.word = prefix_defect_word(signed_defect);
            if (edge.word.empty()) out.every_word_is_positive_qr = false;
            const auto word = prefix_primitive_word_string(edge.word);
            if (word.empty() || (word[0] == 'Q' && word.size() != 1))
                out.every_word_is_positive_qr = false;
            for (char c : word) if (c != 'Q' && c != 'R')
                out.every_word_is_positive_qr = false;
            if ((signed_defect == 0) != (word == "Q"))
                out.signed_side_twist_retained = false;
            ++out.projected_adjacency[edge.source][edge.target];
            ++out.word_multiplicity[word];
            out.edges.push_back(std::move(edge));
        }
    }

    out.projection_equals_existing_boundary_graph =
        out.projected_adjacency == report.gb_matrix;
    out.proved = out.every_edge_has_prefix_witness &&
                 out.every_word_is_positive_qr &&
                 out.signed_side_twist_retained &&
                 out.projection_equals_existing_boundary_graph;
    if (!out.proved && out.obstruction.empty()) {
        if (!out.projection_equals_existing_boundary_graph)
            out.obstruction = "labelled backward-witness projection differs from G_B adjacency";
        else
            out.obstruction = "boundary word-lift invariant failed";
    }
    return out;
}

} // namespace ravel::proof
