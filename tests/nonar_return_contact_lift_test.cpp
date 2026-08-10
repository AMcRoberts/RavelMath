#include <algorithm>
#include <cstdio>
#include <functional>
#include <set>
#include <tuple>
#include <vector>

#include "ravel/contact_boundary.hpp"
#include "ravel/d_cont_check.hpp"
#include "ravel/marker_power_return_core.hpp"
#include "ravel/return_contact_lift.hpp"
#include "ravel/return_history_factor.hpp"
#include "ravel/return_offset_fibre_certificate.hpp"
#include "ravel/return_offset_fibre_holonomy.hpp"
#include "ravel/spectral.hpp"
#include "ravel/substitution.hpp"
#include "ravel/survey.hpp"

using namespace ravel;

namespace {
int tests = 0;
int failures = 0;
void expect(bool value, const char* label) {
    ++tests;
    if (value) std::printf("  [ok]   %s\n", label);
    else { std::printf("  [FAIL] %s\n", label); ++failures; }
}

std::size_t largest_nonzero_recurrent_scc(const ReturnContactLift<3>& lift) {
    const std::size_t n = lift.states.size();
    std::vector<std::vector<std::size_t>> graph(n);
    for (const auto& edge : lift.edges) graph[edge.source].push_back(edge.destination);
    std::vector<int> index(n, -1), low(n, 0), stack;
    std::vector<bool> active(n, false);
    int next = 0;
    std::size_t largest = 0;
    std::function<void(std::size_t)> visit = [&](std::size_t v) {
        index[v] = low[v] = next++;
        stack.push_back(static_cast<int>(v));
        active[v] = true;
        for (std::size_t w : graph[v]) {
            if (index[w] < 0) {
                visit(w);
                low[v] = std::min(low[v], low[w]);
            } else if (active[w]) {
                low[v] = std::min(low[v], index[w]);
            }
        }
        if (low[v] != index[v]) return;
        std::vector<std::size_t> component;
        while (true) {
            const std::size_t w = static_cast<std::size_t>(stack.back());
            stack.pop_back();
            active[w] = false;
            component.push_back(w);
            if (w == v) break;
        }
        bool cyclic = component.size() > 1;
        if (!cyclic && !component.empty()) {
            const auto u = component.front();
            for (std::size_t w : graph[u]) if (w == u) cyclic = true;
        }
        if (!cyclic) return;
        for (std::size_t u : component) {
            const auto& x = lift.states[u].contact.x;
            bool nonzero = false;
            for (long long coordinate : x) if (coordinate != 0) nonzero = true;
            if (nonzero) largest = std::max(largest, component.size());
        }
    };
    for (std::size_t v = 0; v < n; ++v)
        if (index[v] < 0) visit(v);
    return largest;
}
}

int main() {
    const SubstitutionRule base({{1, 2}, {2}, {0}});
    const auto powered = marker_power_rule(base, 3);
    const auto core = analyze_marker_power_return_core(base, 0);
    expect(core.holds && core.power == 3,
           "non-AR base has a certified marker-power core");

    std::vector<std::vector<long long>> incidence(
        powered.alphabet_size(), std::vector<long long>(powered.alphabet_size(), 0));
    for (std::size_t source = 0; source < powered.alphabet_size(); ++source)
        for (auto letter : powered.image(source))
            ++incidence[static_cast<std::size_t>(letter)][source];
    const auto spectral = classify_matrix_spectral(incidence);
    const auto substitution = make_substitution<3>(powered, spectral.beta);
    std::vector<std::tuple<long long, std::vector<long long>, long long>> d_cont;
    for (const auto& candidate : search_D_cont<3>(substitution, 2))
        d_cont.emplace_back(candidate.i,
                            std::vector<long long>(candidate.x.begin(), candidate.x.end()),
                            candidate.j);
    const auto boundary = compute_contact_boundary_dispatch(
        powered, spectral.beta, spectral.b2, d_cont);
    expect(boundary.converged && !boundary.closure_stopped_early
               && !boundary.corona_capped,
           "powered non-AR contact boundary closes");
    expect(!boundary.boundary_nodes.empty(),
           "powered non-AR contact boundary is nonempty");

    std::vector<SNode<3>> bare;
    for (const auto& [i, x, j] : boundary.boundary_nodes) {
        SNode<3> node{};
        node.i = i;
        node.j = j;
        for (std::size_t k = 0; k < 3; ++k) node.x[k] = x[k];
        bare.push_back(node);
    }
    const auto phases = build_return_phase_system(powered, 0);
    std::vector<ReturnContactState<3>> seeds;
    for (const auto& node : bare) {
        const auto compatible = compatible_return_contact_seeds(node, phases);
        seeds.insert(seeds.end(), compatible.begin(), compatible.end());
    }
    ReturnContactLimits limits;
    limits.max_states = 1 << 18;
    limits.max_edges = 1 << 20;
    const auto lift = build_reachable_return_contact_lift(
        substitution, bare, phases, seeds, limits);
    expect(lift.complete(), "non-AR return/contact lift closes");
    expect(lift.projected_node_count == bare.size(),
           "lift projects onto every powered contact node");
    expect(!lift.edges.empty(), "non-AR return/contact lift has transport edges");
    const auto history = probe_return_history_factor(lift, phases);
    std::printf("return-history keys=%zu conflicts=%zu offset_variants=%zu offset_sensitive=%zu max_variants=%zu\n",
                history.history_keys, history.conflicting_keys,
                history.offset_variant_keys, history.offset_sensitive_keys,
                history.maximum_offset_variants);
    expect(history.conflicts_are_offset_driven
               && history.offset_sensitive_keys == history.offset_variant_keys,
           "every multi-offset history key has offset-sensitive transport");
    const auto fibre = derive_return_offset_fibre_certificate(lift, phases);
    std::printf("offset-fibre states=%zu edges=%zu cyclic_sccs=%zu largest=%zu\n",
                fibre.states, fibre.edges, fibre.cyclic_sccs,
                fibre.largest_cyclic_scc);
    expect(fibre.finite && fibre.cyclic_sccs > 0,
           "offset fibre is finite and carries recurrent phase transport");
    const auto holonomy = derive_return_offset_fibre_holonomy(lift, phases);
    std::printf("holonomy recurrent_sccs=%zu gcd_one_sccs=%zu",
                holonomy.recurrent_sccs, holonomy.gcd_one_sccs);
    for (const auto& scc : holonomy.sccs)
        std::printf(" [nodes=%zu gcd=%lld max=%lld]", scc.nodes,
                    scc.gcd_cycle_residue, scc.max_abs_cycle_residue);
    std::printf("\n");
    expect(holonomy.finite && holonomy.recurrent_sccs > 0,
           "finite offset quotient has classified recurrent holonomy");
    expect(holonomy.gcd_one_sccs > 0,
           "non-AR transport has a gcd-one integer cycle residue");
    const std::size_t nonzero_scc = largest_nonzero_recurrent_scc(lift);
    expect(nonzero_scc > 0,
           "non-AR contact lift exposes a recurrent nonzero transport component");

    std::printf("%d tests run, %d failed. boundary=%zu phases=%zu lift=%zu edges=%zu\n",
                tests, failures, bare.size(), phases.states.size(),
                lift.states.size(), lift.edges.size());
    std::printf("largest_nonzero_contact_scc=%zu\n", nonzero_scc);
    return failures == 0 ? 0 : 1;
}
