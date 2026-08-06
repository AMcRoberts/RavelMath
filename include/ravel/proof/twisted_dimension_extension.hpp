#pragma once

#include <algorithm>
#include <cstddef>
#include <map>
#include <queue>
#include <set>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include "ravel/nbonacci_margin_invariant.hpp"
#include "ravel/proof/face_relative_sign_grammar.hpp"
#include "ravel/proof/predicted_core_scc_identification.hpp"
#include "ravel/proof/twisted_quotient.hpp"

namespace ravel::proof {

struct TwistedDimensionExtensionProof {
    std::size_t lower_dimension = 0;
    std::size_t upper_dimension = 0;
    std::size_t lower_states = 0;
    std::size_t upper_states = 0;
    std::size_t lower_roles = 0;
    std::size_t upper_shadow_roles = 0;
    std::size_t lower_edges = 0;
    std::size_t direct_shadow_edges = 0;
    std::size_t path_substituted_edges = 0;
    std::size_t maximum_substitution_length = 0;
    bool shadow_embedding_total = false;
    bool base_role_embedding_well_defined = false;
    bool sheet_correction_well_defined = false;
    bool lower_transport_well_defined = false;
    bool upper_transport_well_defined = false;
    bool every_edge_has_upper_path = false;
    bool cocycle_compatible = false;
    bool substitution_count_formula_exact = false;
    bool maximum_length_formula_exact = false;
    bool proved = false;
    std::string obstruction;
};

namespace twisted_extension_detail {

struct RoleFiberData {
    std::vector<FaceRelativeSignGrammar> grammar;
    std::vector<std::size_t> role;
    std::vector<int> fiber;
    std::size_t role_count = 0;
};

inline RoleFiberData derive_role_fibers(
    std::size_t dimension,
    const std::vector<PredictedCoreNode>& nodes) {
    RoleFiberData out;
    std::map<FaceRelativeSignGrammar, std::size_t> ids;
    out.grammar.reserve(nodes.size());
    out.role.resize(nodes.size());
    out.fiber.resize(nodes.size());
    for (std::size_t k = 0; k < nodes.size(); ++k) {
        const auto& node = nodes[k];
        const auto displacement = nbonacci_margin::displacement_from_descriptor(
            dimension, node.displacement);
        const auto raw = derive_face_relative_sign_grammar(
            displacement, node.pair.i, node.pair.j, false);
        const auto canonical = derive_face_relative_sign_grammar(
            displacement, node.pair.i, node.pair.j, true);
        if (!raw.replayed || !canonical.replayed)
            throw std::runtime_error("twisted extension: role replay failed");
        const auto negated = negate_runs(raw.runs);
        int fiber = -1;
        if (canonical.runs == raw.runs) fiber = 0;
        else if (canonical.runs == negated) fiber = 1;
        else throw std::runtime_error("twisted extension: orientation mismatch");
        const auto [it, inserted] = ids.emplace(canonical, ids.size());
        (void)inserted;
        out.grammar.push_back(canonical);
        out.role[k] = it->second;
        out.fiber[k] = fiber;
    }
    out.role_count = ids.size();
    return out;
}

inline std::vector<std::size_t> shortest_path(
    const std::vector<std::vector<std::size_t>>& adjacency,
    std::size_t source,
    std::size_t target) {
    std::vector<std::size_t> parent(adjacency.size(), adjacency.size());
    std::queue<std::size_t> pending;
    parent[source] = source;
    pending.push(source);
    while (!pending.empty() && parent[target] == adjacency.size()) {
        const auto u = pending.front(); pending.pop();
        for (const auto v : adjacency[u]) {
            if (parent[v] != adjacency.size()) continue;
            parent[v] = u;
            pending.push(v);
        }
    }
    if (parent[target] == adjacency.size()) return {};
    std::vector<std::size_t> path;
    for (std::size_t v = target;; v = parent[v]) {
        path.push_back(v);
        if (v == source) break;
    }
    std::reverse(path.begin(), path.end());
    return path;
}

} // namespace twisted_extension_detail

// Derive the twisted n -> n+1 transport on the formula-defined predicted
// core.  The old core embeds as the previous-alphabet shadow.  Because that
// shadow is not an induced subgraph, each lower edge is transported to a
// shortest upper path.  The orientation correction must factor through lower
// base roles, and the path cocycle must agree with the corrected lower edge.
inline TwistedDimensionExtensionProof derive_twisted_predicted_core_extension(
    std::size_t lower_dimension) {
    if (lower_dimension < 3)
        throw std::invalid_argument("twisted extension: dimension must be >= 3");

    TwistedDimensionExtensionProof out;
    out.lower_dimension = lower_dimension;
    out.upper_dimension = lower_dimension + 1;
    const auto lower = certify_predicted_core_scc(lower_dimension);
    const auto upper = certify_predicted_core_scc(lower_dimension + 1);
    out.lower_states = lower.nodes.size();
    out.upper_states = upper.nodes.size();
    out.lower_edges = lower.edges.size();

    std::map<PredictedCoreNode, std::size_t> upper_index;
    for (std::size_t k = 0; k < upper.nodes.size(); ++k)
        upper_index.emplace(upper.nodes[k], k);
    std::vector<std::size_t> embedding(lower.nodes.size(), upper.nodes.size());
    out.shadow_embedding_total = true;
    for (std::size_t k = 0; k < lower.nodes.size(); ++k) {
        const auto it = upper_index.find(lower.nodes[k]);
        if (it == upper_index.end() ||
            !nbonacci_margin::lies_in_previous_alphabet_shadow(
                lower_dimension + 1, upper.nodes[it->second])) {
            out.shadow_embedding_total = false;
            break;
        }
        embedding[k] = it->second;
    }
    if (!out.shadow_embedding_total) {
        out.obstruction = "lower core does not embed totally in upper shadow";
        return out;
    }

    const auto lower_rf = twisted_extension_detail::derive_role_fibers(
        lower_dimension, lower.nodes);
    const auto upper_rf = twisted_extension_detail::derive_role_fibers(
        lower_dimension + 1, upper.nodes);
    out.lower_roles = lower_rf.role_count;

    std::map<std::size_t, std::size_t> role_embedding;
    std::map<std::size_t, int> sheet_correction;
    std::set<std::size_t> upper_shadow_roles;
    out.base_role_embedding_well_defined = true;
    out.sheet_correction_well_defined = true;
    for (std::size_t k = 0; k < lower.nodes.size(); ++k) {
        const auto lr = lower_rf.role[k];
        const auto uk = embedding[k];
        const auto ur = upper_rf.role[uk];
        const int eta = lower_rf.fiber[k] ^ upper_rf.fiber[uk];
        upper_shadow_roles.insert(ur);
        const auto [rit, rins] = role_embedding.emplace(lr, ur);
        if (!rins && rit->second != ur)
            out.base_role_embedding_well_defined = false;
        const auto [eit, eins] = sheet_correction.emplace(lr, eta);
        if (!eins && eit->second != eta)
            out.sheet_correction_well_defined = false;
    }
    out.upper_shadow_roles = upper_shadow_roles.size();
    if (!out.base_role_embedding_well_defined ||
        !out.sheet_correction_well_defined) {
        out.obstruction = "base-role embedding or sheet correction is ambiguous";
        return out;
    }

    const auto lower_twist = derive_z2_twisted_quotient(
        lower.edges, lower_rf.role, lower_rf.fiber, lower_rf.role_count);
    const auto upper_twist = derive_z2_twisted_quotient(
        upper.edges, upper_rf.role, upper_rf.fiber, upper_rf.role_count);
    out.lower_transport_well_defined = lower_twist.transport_well_defined;
    out.upper_transport_well_defined = upper_twist.transport_well_defined;
    if (!out.lower_transport_well_defined || !out.upper_transport_well_defined) {
        out.obstruction = "fixed-dimensional quotient transport is ambiguous";
        return out;
    }

    std::vector<std::vector<std::size_t>> upper_adjacency(upper.nodes.size());
    std::set<std::pair<std::size_t,std::size_t>> upper_edges(
        upper.edges.begin(), upper.edges.end());
    for (const auto& [u,v] : upper.edges) upper_adjacency[u].push_back(v);

    out.every_edge_has_upper_path = true;
    out.cocycle_compatible = true;
    for (const auto& [s,t] : lower.edges) {
        const auto us = embedding[s];
        const auto ut = embedding[t];
        const auto path = twisted_extension_detail::shortest_path(
            upper_adjacency, us, ut);
        if (path.empty()) {
            out.every_edge_has_upper_path = false;
            break;
        }
        const auto length = path.size() - 1;
        out.maximum_substitution_length =
            std::max(out.maximum_substitution_length, length);
        if (length == 1 && upper_edges.contains({us,ut}))
            ++out.direct_shadow_edges;
        else
            ++out.path_substituted_edges;

        // Endpoint-fiber transport telescopes along the upper path.  The
        // extension correction is required to depend only on the lower role.
        const int lower_tau = lower_rf.fiber[s] ^ lower_rf.fiber[t];
        const int upper_tau = upper_rf.fiber[us] ^ upper_rf.fiber[ut];
        const int eta_s = sheet_correction.at(lower_rf.role[s]);
        const int eta_t = sheet_correction.at(lower_rf.role[t]);
        if (upper_tau != (eta_s ^ lower_tau ^ eta_t)) {
            out.cocycle_compatible = false;
            break;
        }
    }

    const std::size_t n = lower_dimension;
    const std::size_t expected_substituted = 5 * n * n - 15 * n + 12;
    out.substitution_count_formula_exact =
        out.path_substituted_edges == expected_substituted;
    out.maximum_length_formula_exact =
        out.maximum_substitution_length == 2 * n + 6;

    out.proved = out.shadow_embedding_total &&
        out.base_role_embedding_well_defined &&
        out.sheet_correction_well_defined &&
        out.lower_transport_well_defined &&
        out.upper_transport_well_defined &&
        out.every_edge_has_upper_path && out.cocycle_compatible &&
        out.substitution_count_formula_exact &&
        out.maximum_length_formula_exact;
    if (!out.proved)
        out.obstruction = "twisted path substitution did not close";
    return out;
}

inline std::string render_twisted_dimension_extension_report(
    const TwistedDimensionExtensionProof& p) {
    std::ostringstream o;
    o << "TWISTED_PREDICTED_CORE_DIMENSION_EXTENSION\n";
    o << "dimensions=" << p.lower_dimension << "->" << p.upper_dimension << "\n";
    o << "lower_states=" << p.lower_states << "\n";
    o << "upper_states=" << p.upper_states << "\n";
    o << "lower_roles=" << p.lower_roles << "\n";
    o << "upper_shadow_roles=" << p.upper_shadow_roles << "\n";
    o << "lower_edges=" << p.lower_edges << "\n";
    o << "direct_shadow_edges=" << p.direct_shadow_edges << "\n";
    o << "path_substituted_edges=" << p.path_substituted_edges << "\n";
    o << "maximum_substitution_length=" << p.maximum_substitution_length << "\n";
    o << "base_role_embedding_well_defined=" << (p.base_role_embedding_well_defined ? "true" : "false") << "\n";
    o << "sheet_correction_well_defined=" << (p.sheet_correction_well_defined ? "true" : "false") << "\n";
    o << "cocycle_compatible=" << (p.cocycle_compatible ? "true" : "false") << "\n";
    o << "substitution_count_formula_exact=" << (p.substitution_count_formula_exact ? "true" : "false") << "\n";
    o << "maximum_length_formula_exact=" << (p.maximum_length_formula_exact ? "true" : "false") << "\n";
    o << "proved=" << (p.proved ? "true" : "false") << "\n";
    if (!p.obstruction.empty()) o << "obstruction=" << p.obstruction << "\n";
    return o.str();
}

inline std::string render_twisted_dimension_extension_lean() {
    return R"LEAN(import Mathlib

namespace RavelPlayground

/-- A twisted extension carries a base edge to an upper path.  The sheet
    correction eta is a 0-cochain, so upper path transport is lower edge
    transport corrected at the two endpoints. -/
theorem twisted_extension_transport
    (lowerFiber upperFiber etaS etaT : Bool)
    (hsource : upperFiber = (etaS != lowerFiber))
    (targetLower targetUpper : Bool)
    (htarget : targetUpper = (etaT != targetLower)) :
    (upperFiber != targetUpper) =
      (etaS != (lowerFiber != targetLower) != etaT) := by
  subst upperFiber
  subst targetUpper
  cases etaS <;> cases etaT <;>
    cases lowerFiber <;> cases targetLower <;> decide

/-- Replacing each lower edge by a nonempty upper path preserves cocycle
    composition because intermediate endpoint fibers cancel. -/
theorem xor_path_substitution
    (a b c : Bool) :
    (a != b) != (b != c) = (a != c) := by
  cases a <;> cases b <;> cases c <;> decide

end RavelPlayground
)LEAN";
}

} // namespace ravel::proof
