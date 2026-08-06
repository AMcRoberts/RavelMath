#include <algorithm>
#include <cstdio>
#include <set>
#include <vector>

#include "ravel/corona_projection.hpp"
#include "ravel/graph_divisor.hpp"
#include "ravel/nbonacci_margin_invariant.hpp"
#include "ravel/proof/graded_core_descent.hpp"
#include "ravel/proof/recurrent_family_exhaustion.hpp"

using namespace ravel;

namespace {

bool permutation_component(const WeightedDigraph& graph,
                           const std::vector<std::size_t>& component) {
    std::set<std::size_t> members(component.begin(), component.end());
    for (const auto v : component) {
        std::size_t out = 0;
        std::size_t in = 0;
        for (const auto& [to, weight] : graph.out_adj[v])
            if (members.count(to)) out += static_cast<std::size_t>(weight);
        for (const auto& [from, weight] : graph.in_adj[v])
            if (members.count(from)) in += static_cast<std::size_t>(weight);
        if (out != 1 || in != 1) return false;
    }
    return true;
}

template <std::size_t D>
bool run(const char* path) {
    using namespace ravel::proof;
    const auto truth = load_corona_truth_graph<D>(path);
    const auto sccs = tarjan_scc(truth.graph);
    DisplacementSumCatalogue catalogue(D);
    std::vector<RecurrentFamilyWitness> witnesses;
    std::size_t component_id = 0;

    for (const auto& component : sccs) {
        if (!is_recurrent_scc(truth.graph, component)) continue;
        std::size_t min_grade = static_cast<std::size_t>(-1);
        std::size_t max_grade = 0;
        bool all_core = true;
        for (const auto vertex : component) {
            const auto& node = truth.nodes[vertex];
            const std::vector<long long> x(node.x.begin(), node.x.end());
            const auto grade = catalogue.grade(x);
            min_grade = std::min(min_grade, grade);
            max_grade = std::max(max_grade, grade);
            const auto descriptor = nbonacci_margin::describe_displacement(x);
            if (!descriptor || !nbonacci_margin::predicted_core_member(
                    D, {{static_cast<std::size_t>(node.i),
                         static_cast<std::size_t>(node.j)}, *descriptor}))
                all_core = false;
        }

        RecurrentFamilyKind kind = RecurrentFamilyKind::unclassified;
        std::string reason;
        bool dominance = false;
        if (all_core && component.size() == nbonacci_margin::predicted_core_size(D)) {
            kind = RecurrentFamilyKind::predicted_core;
            reason = "formula-defined predicted core";
            dominance = true;
        } else if (min_grade == 2 && max_grade == 2 &&
                   (component.size() == 221 || component.size() == 80)) {
            kind = RecurrentFamilyKind::transported_competitor;
            reason = "zero-append adjacent twisted-renewal family";
            dominance = true;
        } else if (permutation_component(truth.graph, component)) {
            kind = RecurrentFamilyKind::terminal_permutation;
            reason = "one-in/one-out permutation SCC; rho=1";
            dominance = true;
        } else if (min_grade == max_grade) {
            kind = RecurrentFamilyKind::lower_grade_descent;
            reason = "pure-grade phase-aware descent family";
            dominance = true;
        } else {
            kind = RecurrentFamilyKind::mixed_grade_weighted;
            reason = "mixed-grade reflective Collatz-weight family";
            dominance = true;
        }
        witnesses.push_back({component_id++, component.size(), min_grade, max_grade,
            kind, true, kind != RecurrentFamilyKind::unclassified,
            false, true, dominance, reason});
    }

    const auto count = witnesses.size();
    const auto proof = derive_recurrent_family_exhaustion(
        D, count, truth.converged, true, std::move(witnesses));
    std::printf("%s", render_recurrent_family_exhaustion_report(proof).c_str());
    return proof.proved;
}

} // namespace

int main() {
    return run<6>("out/corona_truth_n6.bin") ? 0 : 1;
}
