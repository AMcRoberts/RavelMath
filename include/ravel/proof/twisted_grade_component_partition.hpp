#pragma once

#include <algorithm>
#include <cstddef>
#include <map>
#include <set>
#include <string>
#include <utility>
#include <vector>

#include "ravel/proof/condition_f_twisted_component_quotient.hpp"

namespace ravel::proof {

struct TwistedGradeSccSummary {
    std::size_t component = 0;
    std::size_t vertices = 0;
    std::set<std::size_t> grades;
    bool terminal_permutation = false;
    bool pure_grade = false;
    bool mixed_grade = false;
};

struct TwistedGradeComponentPartitionProof {
    std::size_t lifted_components = 0;
    std::size_t terminal_components = 0;
    std::size_t pure_grade_components = 0;
    std::size_t mixed_grade_components = 0;
    std::size_t holonomy_mixed_components = 0;
    bool quotient_replayed = false;
    bool every_component_classified = false;
    bool proved = false;
    std::vector<TwistedGradeSccSummary> components;
    std::string obstruction;
};

namespace twisted_grade_detail {

inline std::vector<std::size_t> scc_labels(
    const std::vector<std::vector<std::size_t>>& adjacency) {
    const std::size_t n = adjacency.size();
    std::vector<std::vector<std::size_t>> reverse(n);
    for (std::size_t u = 0; u < n; ++u)
        for (const auto v : adjacency[u]) reverse[v].push_back(u);
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
        seen[u] = true; label[u] = component;
        for (const auto v : reverse[u]) if (!seen[v]) self(self, v);
    };
    for (auto it = order.rbegin(); it != order.rend(); ++it) {
        if (seen[*it]) continue;
        dfs2(dfs2, *it);
        ++component;
    }
    return label;
}

} // namespace twisted_grade_detail

/** Classify the actual lifted SCCs after twisted quotient assembly.
 *
 * Grades are concrete-state evidence.  The quotient channel proof determines
 * which sheets are glued by holonomy.  Only after that gluing is resolved do
 * we classify an SCC as terminal, pure-grade, or mixed-grade.  In particular,
 * a twist may combine separately pure untwisted sheets into a mixed-grade SCC;
 * such a component must be routed to renewal/reflective-weight machinery,
 * never to one-stage pure-grade descent.
 */
inline TwistedGradeComponentPartitionProof
 derive_twisted_grade_component_partition(
    const ConditionFTwistedComponentQuotientProof& quotient,
    const std::vector<std::size_t>& concrete_grade) {
    TwistedGradeComponentPartitionProof out;
    if (!quotient.proved || !quotient.skew_product.valid) {
        out.obstruction = "twisted grade partition: quotient proof not closed";
        return out;
    }
    const auto& adjacency = quotient.skew_product.lifted_adjacency;
    if (concrete_grade.size() != adjacency.size()) {
        out.obstruction = "twisted grade partition: grade vector has wrong size";
        return out;
    }
    out.quotient_replayed = true;
    const auto label = twisted_grade_detail::scc_labels(adjacency);
    out.lifted_components = label.empty() ? 0 :
        (*std::max_element(label.begin(), label.end()) + 1);
    out.components.resize(out.lifted_components);
    for (std::size_t c = 0; c < out.components.size(); ++c)
        out.components[c].component = c;
    for (std::size_t v = 0; v < adjacency.size(); ++v) {
        auto& s = out.components[label[v]];
        ++s.vertices;
        s.grades.insert(concrete_grade[v]);
    }

    for (auto& s : out.components) {
        std::size_t internal_edges = 0;
        bool permutation = true;
        for (std::size_t v = 0; v < adjacency.size(); ++v) {
            if (label[v] != s.component) continue;
            std::size_t out_degree = 0, in_degree = 0;
            for (const auto w : adjacency[v])
                if (label[w] == s.component) { ++out_degree; ++internal_edges; }
            for (std::size_t u = 0; u < adjacency.size(); ++u)
                if (label[u] == s.component)
                    for (const auto w : adjacency[u])
                        if (w == v) ++in_degree;
            permutation = permutation && out_degree == 1 && in_degree == 1;
        }
        (void)internal_edges;
        s.terminal_permutation = permutation;
        s.pure_grade = !permutation && s.grades.size() == 1;
        s.mixed_grade = !permutation && s.grades.size() > 1;
        if (s.terminal_permutation) ++out.terminal_components;
        else if (s.pure_grade) ++out.pure_grade_components;
        else if (s.mixed_grade) {
            ++out.mixed_grade_components;
            if (quotient.holonomy_glues_sheets) ++out.holonomy_mixed_components;
        }
    }
    out.every_component_classified = std::all_of(
        out.components.begin(), out.components.end(), [](const auto& s) {
            return static_cast<int>(s.terminal_permutation) +
                   static_cast<int>(s.pure_grade) +
                   static_cast<int>(s.mixed_grade) == 1;
        });
    out.proved = out.quotient_replayed && out.every_component_classified;
    if (!out.proved) out.obstruction = "twisted grade partition incomplete";
    return out;
}

} // namespace ravel::proof
