// Bounded long-run probe for the sigma_{0,2} contact/return lift.
// The caller supplies the wall-clock and memory limits; this program reports
// explicit boundary and lift caps rather than treating exhaustion as closure.
#include <cstdio>
#include <cstdint>
#include <tuple>
#include <vector>

#include "ravel/contact_boundary.hpp"
#include "ravel/d_cont_check.hpp"
#include "ravel/marker_power_return_core.hpp"
#include "ravel/return_contact_lift.hpp"
#include "ravel/return_history_factor.hpp"
#include "ravel/spectral.hpp"
#include "ravel/substitution.hpp"
#include "ravel/survey.hpp"

int main() {
    setvbuf(stdout, nullptr, _IONBF, 0);
    const ravel::SubstitutionRule rule({{1, 1, 2}, {2}, {0}});
    const auto powered = ravel::marker_power_rule(rule, 3);
    const auto phases = ravel::build_return_phase_system(powered, 0);
    std::printf("sigma02: return_words=%zu phases=%zu\n",
                phases.induced.words.size(), phases.states.size());

    std::vector<std::vector<long long>> incidence(3,
        std::vector<long long>(3, 0));
    for (std::size_t source = 0; source < 3; ++source)
        for (auto letter : powered.image(source))
            ++incidence[static_cast<std::size_t>(letter)][source];
    const auto spectral = ravel::classify_matrix_spectral(incidence);
    std::printf("sigma02: beta=%.17g b2=%.17g\n",
                spectral.beta, spectral.b2);
    const auto substitution = ravel::make_substitution<3>(powered, spectral.beta);
    std::vector<std::tuple<long long, std::vector<long long>, long long>> d_cont;
    for (const auto& candidate : ravel::search_D_cont<3>(substitution, 2))
        d_cont.emplace_back(candidate.i,
            std::vector<long long>(candidate.x.begin(), candidate.x.end()),
            candidate.j);
    std::printf("sigma02: d_cont=%zu; starting boundary\n", d_cont.size());

    ravel::ContactBoundaryLimits limits;
    limits.closure_cap = 1'000'000;
    limits.corona_cap = 10'000'000;
    limits.max_corona_rounds = 100;
    limits.retain_boundary_matrix = false;
    std::printf("sigma02: requested closure_cap=%zu corona_cap=%zu rounds=%d\n",
                limits.closure_cap, limits.corona_cap,
                limits.max_corona_rounds);
    const auto boundary = ravel::compute_contact_boundary_dispatch(
        powered, spectral.beta, spectral.b2, d_cont);
    std::printf("sigma02: boundary converged=%d early_stop=%d corona_capped=%d "
                "nodes=%zu pre_contact=%zu max_a=%zu cap=%zu rounds=%d\n",
                boundary.converged,
                boundary.closure_stopped_early, boundary.corona_capped,
                boundary.boundary_nodes.size(), boundary.pre_contact_size,
                boundary.max_a_size_reached, boundary.limits_used.corona_cap,
                boundary.limits_used.max_corona_rounds);
    if (!boundary.converged || boundary.closure_stopped_early
        || boundary.corona_capped) {
        std::printf("TERMINATION=boundary_cap_or_nonconvergence\n");
        return 2;
    }

    std::vector<ravel::SNode<3>> bare;
    for (const auto& [i, x, j] : boundary.boundary_nodes) {
        ravel::SNode<3> node{};
        node.i = i; node.j = j;
        for (std::size_t k = 0; k < 3; ++k) node.x[k] = x[k];
        bare.push_back(node);
    }
    std::vector<ravel::ReturnContactState<3>> seeds;
    for (const auto& node : bare) {
        const auto compatible = ravel::compatible_return_contact_seeds(node, phases);
        seeds.insert(seeds.end(), compatible.begin(), compatible.end());
    }
    ravel::ReturnContactLimits lift_limits;
    lift_limits.max_states = 1 << 20;
    lift_limits.max_edges = 1 << 22;
    const auto lift = ravel::build_reachable_return_contact_lift(
        substitution, bare, phases, seeds, lift_limits);
    const auto history = ravel::probe_return_history_factor(lift, phases);
    std::printf("sigma02: lift_complete=%d states=%zu edges=%zu history=%zu "
                "conflicts=%zu offset_sensitive=%zu\n", lift.complete(),
                lift.states.size(), lift.edges.size(), history.history_keys,
                history.conflicting_keys, history.offset_sensitive_keys);
    if (!lift.complete()) {
        std::printf("TERMINATION=lift_cap\n");
        return 3;
    }
    std::printf("TERMINATION=finite_contact_lift_complete\n");
    return 0;
}
