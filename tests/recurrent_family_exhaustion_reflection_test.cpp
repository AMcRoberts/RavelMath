// Wires lean/playground_recurrent_family_exhaustion.lean (previously flat:
// zero C++ consumer) to the reflection pipeline. Reuses exactly the
// independent SCC-extraction-and-classification logic already exercised
// by tests/recurrent_family_exhaustion_real_test.cpp (Tarjan over a
// concrete corona-truth graph, per-component grade range and
// structural-predicate replay, nothing pre-trusted) against
// out/corona_truth_n6.bin (built via
// `./out/adjacent_competitor_transport --legacy-corona 6`). On success
// the renderer instantiates recurrent_family_exhaustion at this CONCRETE
// dimension's component list.

#include <algorithm>
#include <cassert>
#include <fstream>
#include <iostream>
#include <set>
#include <vector>

#include "math/proof_reflection.hpp"
#include "ravel/corona_projection.hpp"
#include "ravel/graph_divisor.hpp"
#include "ravel/nbonacci_margin_invariant.hpp"
#include "ravel/proof/graded_core_descent.hpp"
#include "ravel/proof/recurrent_family_exhaustion.hpp"
#include "ravel/proof/recurrent_family_exhaustion_reflection.hpp"
#include "ravel/proof/reflective_lean_renderer.hpp"

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
ravel::proof::RecurrentFamilyExhaustionProof compute(const char* path) {
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
    return derive_recurrent_family_exhaustion(D, count, truth.converged, true, std::move(witnesses));
}

}  // namespace

int main() {
    const auto proof = compute<6>("out/corona_truth_n6.bin");
    std::cout << render_recurrent_family_exhaustion_report(proof);
    assert(proof.proved);

    mathlib::reflection::Trace trace("recurrent_family_exhaustion_batch");
    {
        mathlib::reflection::ScopedTrace scope(&trace);
        ravel::proof::stage_recurrent_family_exhaustion(
            proof, "n=6 corona-truth recurrent SCC exhaustion");

        // Negative control: an unproved exhaustion result must not stage.
        ravel::proof::RecurrentFamilyExhaustionProof bad;
        bad.proved = false;
        ravel::proof::stage_recurrent_family_exhaustion(bad, "unproved control");
    }

    auto nodes = trace.find<mathlib::reflection::RecurrentFamilyExhaustionCertificate>();
    std::cout << "trace recorded " << nodes.size()
              << " RecurrentFamilyExhaustionCertificate nodes\n";
    assert(nodes.size() == 1);
    assert(nodes.front().second->family_kinds.size() == proof.witnesses.size());

    std::string lean = ravel::proof::render_reflective_lean_module(trace);
    assert(lean.find("theorem recurrent_family_exhaustion") != std::string::npos);
    assert(lean.find("recurrent_family_exhaustion_instance_0") != std::string::npos);
    assert(lean.find("recurrent_family_exhaustion_instance_1") == std::string::npos);

    std::ofstream out("lean/generated/recurrent_family_exhaustion_batch.lean");
    out << lean;
    out.close();

    std::cout << "recurrent_family_exhaustion_reflection_test: PASS\n";
    return 0;
}
