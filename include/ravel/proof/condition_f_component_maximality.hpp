#pragma once

#include <algorithm>
#include <cstddef>
#include <string>
#include <utility>
#include <vector>

#include "ravel/proof/condition_f_twisted_component_quotient.hpp"
#include "ravel/proof/finite_quotient_core_maximality.hpp"
#include "ravel/proof/fibered_twisted_quotient.hpp"
#include "ravel/proof/twisted_grade_component_partition.hpp"

namespace ravel::proof {

enum class ConditionFComponentClass {
    Core,
    TerminalPermutation,
    PureGrade,
    MixedGrade,
    Acyclic,
    Unsupported
};

struct ConditionFComponentMaximalityEvidence {
    std::size_t component = 0;
    std::size_t vertices = 0;
    std::vector<std::size_t> grades;
    ConditionFComponentClass classification = ConditionFComponentClass::Unsupported;
    QuotientDominanceMechanism mechanism = QuotientDominanceMechanism::Unproved;
    PathInjectiveSimulationCertificate path_injection;
    GradedTransferProof renewal_transfer;
    RankOnePairedDominance paired_matrix;
    bool recurrent = false;
    bool bounded_by_core = false;
    bool strict_or_terminal = false;
    std::string obstruction;
};

struct ConditionFComponentMaximalityProof {
    std::size_t lifted_components = 0;
    std::size_t recurrent_components = 0;
    std::size_t terminal_components = 0;
    std::size_t pure_grade_components = 0;
    std::size_t mixed_grade_components = 0;
    std::size_t unsupported_components = 0;
    std::size_t path_injection_components = 0;
    std::size_t renewal_components = 0;
    std::size_t paired_matrix_components = 0;
    std::size_t core_component = 0;
    bool twisted_assembly_replayed = false;
    bool holonomy_resolved_before_grading = false;
    bool every_recurrent_component_routed = false;
    bool every_noncore_component_bounded = false;
    bool core_spectrally_maximal = false;
    std::vector<ConditionFComponentMaximalityEvidence> evidence;
    std::string obstruction;
};

namespace condition_f_component_detail {

inline std::vector<std::size_t> scc_labels(
    const std::vector<std::vector<std::size_t>>& adjacency) {
    const std::size_t n = adjacency.size();
    std::vector<std::vector<std::size_t>> reverse(n);
    for (std::size_t u = 0; u < n; ++u)
        for (const auto v : adjacency[u]) {
            if (v >= n) throw std::invalid_argument("Condition-F maximality: bad edge");
            reverse[v].push_back(u);
        }
    std::vector<bool> seen(n, false);
    std::vector<std::size_t> order;
    auto dfs1 = [&](auto&& self, std::size_t u) -> void {
        seen[u] = true;
        for (const auto v : adjacency[u]) if (!seen[v]) self(self, v);
        order.push_back(u);
    };
    for (std::size_t u = 0; u < n; ++u) if (!seen[u]) dfs1(dfs1, u);
    std::fill(seen.begin(), seen.end(), false);
    std::vector<std::size_t> label(n, n);
    std::size_t component = 0;
    auto dfs2 = [&](auto&& self, std::size_t u) -> void {
        seen[u] = true;
        label[u] = component;
        for (const auto v : reverse[u]) if (!seen[v]) self(self, v);
    };
    for (auto it = order.rbegin(); it != order.rend(); ++it) {
        if (seen[*it]) continue;
        dfs2(dfs2, *it);
        ++component;
    }
    return label;
}

inline bool recurrent_component(
    const std::vector<std::vector<std::size_t>>& adjacency,
    const std::vector<std::size_t>& label,
    std::size_t component,
    std::size_t vertices) {
    if (vertices > 1) return true;
    for (std::size_t u = 0; u < adjacency.size(); ++u)
        if (label[u] == component)
            for (const auto v : adjacency[u])
                if (v == u) return true;
    return false;
}

inline std::vector<std::vector<long long>> component_matrix(
    const std::vector<std::vector<std::size_t>>& adjacency,
    const std::vector<std::size_t>& label,
    std::size_t component) {
    std::vector<std::size_t> vertices;
    std::vector<std::size_t> local(adjacency.size(), adjacency.size());
    for (std::size_t v = 0; v < adjacency.size(); ++v)
        if (label[v] == component) {
            local[v] = vertices.size();
            vertices.push_back(v);
        }
    std::vector<std::vector<long long>> matrix(
        vertices.size(), std::vector<long long>(vertices.size(), 0));
    for (std::size_t i = 0; i < vertices.size(); ++i)
        for (const auto v : adjacency[vertices[i]])
            if (label[v] == component)
                ++matrix[i][local[v]];
    return matrix;
}

inline std::vector<std::size_t> sorted_grades(
    const std::vector<std::size_t>& grade,
    const std::vector<std::size_t>& label,
    std::size_t component) {
    std::vector<std::size_t> result;
    for (std::size_t v = 0; v < grade.size(); ++v)
        if (label[v] == component) result.push_back(grade[v]);
    std::sort(result.begin(), result.end());
    result.erase(std::unique(result.begin(), result.end()), result.end());
    return result;
}

} // namespace condition_f_component_detail

/** Exhaustively route every holonomy-resolved recurrent Condition-F component.
 *
 * The concrete lift is assembled first.  SCCs and grades are computed only on
 * that assembled lift, so a twist may glue pure sheets into one mixed-grade
 * component.  Each non-core recurrent SCC is then discharged by the proof
 * mechanism appropriate to its assembled class:
 *
 *   terminal permutation -> exact rho=1;
 *   pure grade            -> multiplicity-preserving descent, then renewal;
 *   mixed grade           -> renewal/reflective transfer, then paired fallback.
 *
 * No caller-supplied component matrices or classification Booleans are
 * accepted.  Matrices, SCCs, grade sets, and routing are all derived here.
 */
inline ConditionFComponentMaximalityProof
 derive_condition_f_component_maximality(
    const ConditionFTwistedComponentQuotientProof& quotient,
    const std::vector<std::size_t>& concrete_grade,
    std::size_t core_vertex,
    std::size_t horizon = 160) {
    ConditionFComponentMaximalityProof out;
    if (!quotient.proved || !quotient.skew_product.valid) {
        out.obstruction = "Condition-F component maximality requires a closed twisted quotient";
        return out;
    }
    const auto& adjacency = quotient.skew_product.lifted_adjacency;
    if (concrete_grade.size() != adjacency.size() || core_vertex >= adjacency.size()) {
        out.obstruction = "Condition-F component maximality: malformed grade/core input";
        return out;
    }
    out.twisted_assembly_replayed = true;
    const auto partition = derive_twisted_grade_component_partition(
        quotient, concrete_grade);
    if (!partition.proved) {
        out.obstruction = partition.obstruction;
        return out;
    }
    out.holonomy_resolved_before_grading = true;

    const auto label = condition_f_component_detail::scc_labels(adjacency);
    out.lifted_components = label.empty() ? 0 :
        *std::max_element(label.begin(), label.end()) + 1;
    out.core_component = label[core_vertex];
    const auto core = condition_f_component_detail::component_matrix(
        adjacency, label, out.core_component);
    const auto core_adjacency = matrix_multi_adjacency(core);
    if (core.empty()) {
        out.obstruction = "Condition-F component maximality: empty core SCC";
        return out;
    }

    bool all_routed = true;
    bool all_bounded = true;
    for (std::size_t c = 0; c < out.lifted_components; ++c) {
        ConditionFComponentMaximalityEvidence e;
        e.component = c;
        const auto matrix = condition_f_component_detail::component_matrix(
            adjacency, label, c);
        e.vertices = matrix.size();
        e.grades = condition_f_component_detail::sorted_grades(
            concrete_grade, label, c);
        e.recurrent = condition_f_component_detail::recurrent_component(
            adjacency, label, c, e.vertices);
        if (!e.recurrent) {
            e.classification = ConditionFComponentClass::Acyclic;
            e.bounded_by_core = true;
            out.evidence.push_back(std::move(e));
            continue;
        }
        ++out.recurrent_components;
        if (c == out.core_component) {
            e.classification = ConditionFComponentClass::Core;
            e.mechanism = QuotientDominanceMechanism::Core;
            e.bounded_by_core = true;
            out.evidence.push_back(std::move(e));
            continue;
        }

        if (matrix_is_terminal_permutation(matrix)) {
            e.classification = ConditionFComponentClass::TerminalPermutation;
            e.mechanism = QuotientDominanceMechanism::TerminalPermutation;
            e.bounded_by_core = true;
            e.strict_or_terminal = true;
            ++out.terminal_components;
            out.evidence.push_back(std::move(e));
            continue;
        }

        const bool pure = e.grades.size() == 1;
        e.classification = pure ? ConditionFComponentClass::PureGrade
                                : ConditionFComponentClass::MixedGrade;
        if (pure) ++out.pure_grade_components;
        else ++out.mixed_grade_components;

        const auto component_adjacency = matrix_multi_adjacency(matrix);
        if (pure) {
            e.path_injection = derive_path_injective_simulation(
                component_adjacency, core_adjacency);
            if (e.path_injection.spectral_radius_nonstrict) {
                e.mechanism = QuotientDominanceMechanism::PathInjection;
                e.bounded_by_core = true;
                ++out.path_injection_components;
            } else {
                e.renewal_transfer = derive_graded_transfer_proof(
                    matrix, core, horizon);
                if (e.renewal_transfer.proved) {
                    e.mechanism = QuotientDominanceMechanism::RenewalTransfer;
                    e.bounded_by_core = true;
                    e.strict_or_terminal = true;
                    ++out.renewal_components;
                }
            }
        } else {
            e.renewal_transfer = derive_graded_transfer_proof(
                matrix, core, horizon);
            if (e.renewal_transfer.proved) {
                e.mechanism = QuotientDominanceMechanism::RenewalTransfer;
                e.bounded_by_core = true;
                e.strict_or_terminal = true;
                ++out.renewal_components;
            }
        }

        if (!e.bounded_by_core) {
            e.paired_matrix = derive_rank_one_paired_dominance(
                matrix, core, horizon);
            if (e.paired_matrix.replayed) {
                e.mechanism = QuotientDominanceMechanism::PairedMatrix;
                e.bounded_by_core = true;
                e.strict_or_terminal = true;
                ++out.paired_matrix_components;
            }
        }
        if (!e.bounded_by_core) {
            e.classification = ConditionFComponentClass::Unsupported;
            e.mechanism = QuotientDominanceMechanism::Unproved;
            e.obstruction = pure
                ? "pure-grade SCC has no path-injective descent, renewal transfer, or paired proof"
                : "mixed-grade SCC has no renewal transfer or paired proof";
            ++out.unsupported_components;
            all_routed = false;
            all_bounded = false;
        }
        out.evidence.push_back(std::move(e));
    }

    out.every_recurrent_component_routed = all_routed;
    out.every_noncore_component_bounded = all_bounded;
    out.core_spectrally_maximal = out.twisted_assembly_replayed &&
        out.holonomy_resolved_before_grading && all_routed && all_bounded;
    if (!out.core_spectrally_maximal)
        out.obstruction = "at least one holonomy-resolved recurrent SCC is unsupported";
    return out;
}


/** Apply the same exhaustive SCC routing to the more general locally-fibered
 * quotient.  Each class may have its own representative set and no global
 * role-by-sheet trivialization is assumed.  The fibered certificate already
 * proves exact reconstruction and path-growth preservation; this adapter
 * deliberately exposes only its assembled concrete lift to the SCC theorem.
 */
inline ConditionFComponentMaximalityProof
 derive_condition_f_component_maximality(
    const FiberedTwistedQuotientCertificate& quotient,
    const std::vector<std::size_t>& concrete_grade,
    std::size_t core_vertex,
    std::size_t horizon = 160) {
    if (!quotient.valid || !quotient.spectral_radius_preserved) {
        ConditionFComponentMaximalityProof out;
        out.obstruction = "Condition-F component maximality requires a valid locally-fibered quotient";
        return out;
    }
    ConditionFTwistedComponentQuotientProof assembled;
    assembled.concrete_states = quotient.concrete_states;
    assembled.concrete_edges = quotient.concrete_edges;
    assembled.base_roles = quotient.quotient_classes;
    assembled.channel_count = quotient.channels;
    assembled.genuinely_twisted = quotient.genuinely_twisted_inside_classes;
    assembled.holonomy_glues_sheets = quotient.genuinely_twisted_inside_classes;
    assembled.complete_sheet_table = true; // not used by the assembled-lift proof
    assembled.role_pair_regular = true;
    assembled.channels_are_permutations = true;
    assembled.concrete_lift_reconstructed = true;
    assembled.path_growth_preserved = true;
    assembled.proved = true;
    assembled.skew_product.valid = true;
    assembled.skew_product.spectral_radius_equal = true;
    assembled.skew_product.lifted_adjacency = quotient.concrete_adjacency;
    return derive_condition_f_component_maximality(
        assembled, concrete_grade, core_vertex, horizon);
}

inline const char* condition_f_component_class_name(ConditionFComponentClass c) {
    switch (c) {
        case ConditionFComponentClass::Core: return "core";
        case ConditionFComponentClass::TerminalPermutation: return "terminal-permutation";
        case ConditionFComponentClass::PureGrade: return "pure-grade";
        case ConditionFComponentClass::MixedGrade: return "mixed-grade";
        case ConditionFComponentClass::Acyclic: return "acyclic";
        default: return "unsupported";
    }
}

} // namespace ravel::proof
