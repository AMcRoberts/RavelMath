// Generic finite contact-boundary / parent-role intertwiner.
//
// The boundary and parent-role constructions are the same for every finite
// alphabet and every bounded prefix-defect range.  This operation keeps that
// proof shape in one place so individual fourth-generator examples only supply
// their substitution and exact spectral data.
#pragma once

#include <cstddef>
#include <map>
#include <set>
#include <stdexcept>
#include <string>
#include <tuple>
#include <vector>

#include "ravel/ambient_graph.hpp"
#include "ravel/contact_boundary.hpp"
#include "ravel/corona.hpp"
#include "ravel/proof/condition_f_pair_boundary_substitution.hpp"
#include "ravel/proof/finite_positive_grammar_majorant.hpp"

namespace ravel::proof {

template <std::size_t Alphabet>
struct ContactBoundaryGeneratorIntertwinerCertificate {
    std::size_t generator_count{};
    std::size_t max_defect{};
    std::size_t boundary_states{};
    std::size_t pre_contact_states{};
    std::size_t contact_states{};
    std::size_t universal_roles{};
    long long boundary_edges{};
    long long universal_edges{};
    NonnegativeMatrix intertwiner;
    std::vector<NonnegativeMatrix> boundary_generators;
    std::vector<NonnegativeMatrix> universal_generators;
    bool parent_catalogue_complete{};
    bool boundary_complete{};
    bool closure_stopped_early{};
    bool corona_capped{};
    bool every_boundary_edge_has_universal_witness{};
    bool base_role_projection_exact{};
    std::vector<bool> generator_intertwines;
    bool simultaneous_intertwiner{};
    std::vector<bool> boundary_realizes_defect;
    bool finite_positive_grammar_ready{};
    bool proved{};
    std::string obstruction;
};

namespace contact_boundary_generator_intertwiner_detail {

inline NonnegativeMatrix zero_matrix(std::size_t rows, std::size_t columns) {
    return NonnegativeMatrix(rows, std::vector<long long>(columns, 0));
}

inline int defect_index(long long defect, std::size_t max_defect) {
    if (defect < -static_cast<long long>(max_defect) ||
        defect > static_cast<long long>(max_defect))
        throw std::runtime_error("contact-boundary generator defect outside configured range");
    return static_cast<int>(defect + static_cast<long long>(max_defect));
}

template <std::size_t Alphabet>
inline std::size_t role(long long left, long long right) {
    if (left < 0 || right < 0 || static_cast<std::size_t>(left) >= Alphabet ||
        static_cast<std::size_t>(right) >= Alphabet)
        throw std::out_of_range("contact-boundary generator role");
    return static_cast<std::size_t>(left) * Alphabet + static_cast<std::size_t>(right);
}

}  // namespace contact_boundary_generator_intertwiner_detail

template <std::size_t Alphabet>
ContactBoundaryGeneratorIntertwinerCertificate<Alphabet>
derive_contact_boundary_generator_intertwiner(
    const SubstitutionRule& rule, double beta, double beta_conjugate,
    std::size_t max_defect, const ContactBoundaryLimits& limits,
    long long search_bound = 2) {
    using namespace contact_boundary_generator_intertwiner_detail;
    using Certificate = ContactBoundaryGeneratorIntertwinerCertificate<Alphabet>;
    Certificate out;
    out.max_defect = max_defect;
    out.generator_count = 2 * max_defect + 1;
    out.universal_roles = Alphabet * Alphabet;
    out.generator_intertwines.assign(out.generator_count, false);
    out.boundary_realizes_defect.assign(out.generator_count, false);
    if (max_defect == 0) {
        out.obstruction = "generator range must contain a nonzero defect";
        return out;
    }

    const auto report = compute_contact_boundary_from_subst<Alphabet>(
        rule, beta, beta_conjugate, search_bound, limits);
    out.boundary_complete = report.converged && !report.closure_stopped_early &&
        !report.corona_capped;
    out.closure_stopped_early = report.closure_stopped_early;
    out.corona_capped = report.corona_capped;
    out.pre_contact_states = report.pre_contact_size;
    out.contact_states = report.contact_size;
    out.boundary_states = report.boundary_size;
    if (!out.boundary_complete) {
        out.obstruction = "contact boundary computation hit an explicit cap";
        return out;
    }
    const auto subst = make_substitution<Alphabet>(rule, beta);
    std::vector<SNode<Alphabet>> boundary;
    boundary.reserve(report.boundary_nodes.size());
    std::map<SNode<Alphabet>, std::size_t> boundary_index;
    for (const auto& tuple : report.boundary_nodes) {
        SNode<Alphabet> node;
        node.i = std::get<0>(tuple);
        node.j = std::get<2>(tuple);
        const auto& coordinates = std::get<1>(tuple);
        for (std::size_t q = 0; q < Alphabet; ++q) node.x[q] = coordinates[q];
        boundary_index.emplace(node, boundary.size());
        boundary.push_back(node);
    }
    out.boundary_states = boundary.size();
    if (boundary.empty()) {
        out.obstruction = "contact boundary unexpectedly empty";
        return out;
    }

    out.boundary_generators.resize(out.generator_count,
        zero_matrix(boundary.size(), boundary.size()));
    out.universal_generators.resize(out.generator_count,
        zero_matrix(out.universal_roles, out.universal_roles));
    using WitnessKey = std::tuple<std::size_t, std::size_t, int>;
    std::set<WitnessKey> universal_witnesses;
    for (long long left = 0; left < static_cast<long long>(Alphabet); ++left)
        for (long long right = 0; right < static_cast<long long>(Alphabet); ++right) {
            const auto left_parents = parent_decompositions<Alphabet>(subst.images, left);
            const auto right_parents = parent_decompositions<Alphabet>(subst.images, right);
            for (const auto& lp : left_parents) for (const auto& rp : right_parents) {
                const int generator = defect_index(
                    static_cast<long long>(rp.p.size()) - static_cast<long long>(lp.p.size()),
                    max_defect);
                const auto source = role<Alphabet>(left, right);
                const auto target = role<Alphabet>(lp.parent_letter, rp.parent_letter);
                ++out.universal_generators[static_cast<std::size_t>(generator)][source][target];
                universal_witnesses.insert({source, target, generator});
                ++out.universal_edges;
            }
        }
    out.parent_catalogue_complete = !universal_witnesses.empty();

    out.every_boundary_edge_has_universal_witness = true;
    for (std::size_t source = 0; source < boundary.size(); ++source) {
        for (const auto& [target, prefixes] : simple_forward_targets_exact<Alphabet>(
                 subst, boundary[source])) {
            const auto target_it = boundary_index.find(target);
            if (target_it == boundary_index.end()) continue;
            const int generator = defect_index(
                static_cast<long long>(prefixes.second.size()) -
                static_cast<long long>(prefixes.first.size()), max_defect);
            ++out.boundary_generators[static_cast<std::size_t>(generator)]
                [source][target_it->second];
            ++out.boundary_edges;
            out.boundary_realizes_defect[static_cast<std::size_t>(generator)] = true;
            const auto source_role = role<Alphabet>(boundary[source].i, boundary[source].j);
            const auto target_role = role<Alphabet>(target.i, target.j);
            if (!universal_witnesses.count({source_role, target_role, generator}))
                out.every_boundary_edge_has_universal_witness = false;
        }
    }

    out.intertwiner = zero_matrix(boundary.size(), out.universal_roles);
    out.base_role_projection_exact = true;
    for (std::size_t source = 0; source < boundary.size(); ++source) {
        const auto& node = boundary[source];
        if (node.i < 0 || node.j < 0 || static_cast<std::size_t>(node.i) >= Alphabet ||
            static_cast<std::size_t>(node.j) >= Alphabet) {
            out.base_role_projection_exact = false;
            out.obstruction = "boundary role outside alphabet";
            return out;
        }
        out.intertwiner[source][role<Alphabet>(node.i, node.j)] = 1;
    }

    out.simultaneous_intertwiner = true;
    for (std::size_t generator = 0; generator < out.generator_count; ++generator) {
        const auto lhs = condition_f_pair_boundary_detail::rectangular_product(
            out.boundary_generators[generator], out.intertwiner);
        const auto rhs = condition_f_pair_boundary_detail::rectangular_product(
            out.intertwiner, out.universal_generators[generator]);
        out.generator_intertwines[generator] =
            condition_f_pair_boundary_detail::leq(lhs, rhs);
        out.simultaneous_intertwiner &= out.generator_intertwines[generator];
    }

    std::vector<NormWeightedGrammarChannel> channels;
    for (std::size_t generator = 0; generator < out.generator_count; ++generator)
        for (std::size_t source = 0; source < out.universal_roles; ++source)
            for (std::size_t target = 0; target < out.universal_roles; ++target)
                if (const auto multiplicity = out.universal_generators[generator][source][target];
                    multiplicity > 0)
                    channels.push_back({source, target, generator,
                                        static_cast<std::size_t>(multiplicity), {1, 1}});
    const auto grammar = derive_finite_positive_grammar_majorant(
        out.universal_roles, out.generator_count, channels);
    out.finite_positive_grammar_ready = grammar.proved;
    out.proved = out.parent_catalogue_complete &&
        out.every_boundary_edge_has_universal_witness &&
        out.base_role_projection_exact && out.simultaneous_intertwiner &&
        out.finite_positive_grammar_ready;
    if (!out.proved && out.obstruction.empty())
        out.obstruction = "contact-boundary generator intertwiner failed";
    return out;
}

}  // namespace ravel::proof
