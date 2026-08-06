#pragma once

#include <cstddef>
#include <cstdint>
#include <deque>
#include <map>
#include <set>
#include <string>
#include <unordered_set>
#include <utility>
#include <vector>

#include "ravel/proof/block_height_shell_rank.hpp"
#include "ravel/proof/shell_shadow_rank_transport.hpp"

namespace ravel::proof {

struct ShellFirstReturnGraphCertificate {
    std::size_t dimension = 0;
    std::int64_t bound = 0;
    ShellReturnGraph graph;
    std::size_t shell_states = 0;
    std::size_t return_edges = 0;
    bool vertices_on_shell = false;
    bool edges_replayed = false;
    bool valid = false;
    std::string obstruction;
};

/** Build the exact first-return relation on the coefficient shell.
 *
 * This is the canonical graph constructor used by the parametric reduction;
 * it is intentionally factored out of the rank miner so simulations and rank
 * transport consume exactly the same transition semantics.
 */
inline ShellFirstReturnGraphCertificate derive_shell_first_return_graph(
    std::size_t n, std::int64_t bound) {
    ShellFirstReturnGraphCertificate c;
    c.dimension = n;
    c.bound = bound;
    if (n < 2 || bound < 1) {
        c.obstruction = "invalid shell graph parameters";
        return c;
    }

    std::vector<ShellState> shell;
    std::unordered_set<ShellState, ShellStateHash> shell_set;
    ShellState state(n, 0);
    enumerate_box_states_rec(n, bound, 0, state, [&](const ShellState& x) {
        if (shell_radius(x) == bound) {
            shell.push_back(x);
            shell_set.insert(x);
            c.graph.try_emplace(x);
        }
    });
    c.shell_states = shell.size();
    c.vertices_on_shell = true;
    for (const auto& x : shell)
        c.vertices_on_shell &= shell_radius(x) == bound;

    for (const auto& source : shell) {
        std::deque<ShellState> pending;
        std::unordered_set<ShellState, ShellStateHash> seen;
        pending.push_back(source);
        seen.insert(source);
        while (!pending.empty()) {
            auto current = std::move(pending.front());
            pending.pop_front();
            for (auto target : bounded_carry_successors(current, bound)) {
                if (shell_set.contains(target)) {
                    c.graph[source].insert(target);
                } else if (seen.insert(target).second) {
                    pending.push_back(std::move(target));
                }
            }
        }
    }

    c.return_edges = 0;
    c.edges_replayed = true;
    for (const auto& [source, targets] : c.graph) {
        c.edges_replayed &= shell_set.contains(source);
        c.return_edges += targets.size();
        for (const auto& target : targets)
            c.edges_replayed &= shell_set.contains(target);
    }
    c.valid = c.vertices_on_shell && c.edges_replayed;
    if (!c.valid) c.obstruction = "shell first-return graph replay failed";
    return c;
}

struct FixedBaseShellInstanceCertificate {
    std::int64_t upper_bound = 0;
    ShellFirstReturnGraphCertificate upper_graph;
    ShellShadowSimulationCertificate simulation;
    SimulatedShellRankCertificate transported_rank;
    bool valid = false;
    std::string obstruction;
};

struct FixedBaseShellReductionCertificate {
    std::size_t dimension = 0;
    std::int64_t base_bound = 2;
    ShellFirstReturnGraphCertificate base_graph;
    ConcreteShellRankCertificate base_rank;
    std::vector<FixedBaseShellInstanceCertificate> instances;
    bool base_acyclic = false;
    bool every_simulation_total = false;
    bool every_rank_transport_strict = false;
    bool finite_family_closed = false;
    std::string obstruction;
};

/** Exact finite realization of the universal proof pattern.
 *
 * Every announced shell is related directly to the canonical shell M=2;
 * the base topological rank is transported by the greatest simulation.
 * No chain of M->M-1 simulations is used, so failed intermediate shadows do
 * not accumulate defects.
 */
inline FixedBaseShellReductionCertificate derive_fixed_base_shell_reduction(
    std::size_t n, std::int64_t maximum_bound) {
    FixedBaseShellReductionCertificate c;
    c.dimension = n;
    if (n < 2 || maximum_bound < c.base_bound) {
        c.obstruction = "invalid fixed-base reduction range";
        return c;
    }
    c.base_graph = derive_shell_first_return_graph(n, c.base_bound);
    if (!c.base_graph.valid) {
        c.obstruction = c.base_graph.obstruction;
        return c;
    }
    c.base_rank = derive_concrete_shell_rank(c.base_graph.graph);
    c.base_acyclic = c.base_rank.valid;
    if (!c.base_acyclic) {
        c.obstruction = "canonical shell M=2 is not ranked";
        return c;
    }

    c.every_simulation_total = true;
    c.every_rank_transport_strict = true;
    for (std::int64_t bound = c.base_bound + 1;
         bound <= maximum_bound; ++bound) {
        FixedBaseShellInstanceCertificate instance;
        instance.upper_bound = bound;
        instance.upper_graph = derive_shell_first_return_graph(n, bound);
        if (!instance.upper_graph.valid) {
            instance.obstruction = instance.upper_graph.obstruction;
        } else {
            instance.simulation = derive_shell_shadow_simulation_to_bound(
                bound, c.base_bound,
                instance.upper_graph.graph, c.base_graph.graph);
            if (!instance.simulation.recurrent_descent) {
                instance.obstruction = instance.simulation.obstruction;
            } else {
                instance.transported_rank = transport_shell_rank_through_simulation(
                    instance.simulation, c.base_rank);
                if (!instance.transported_rank.valid)
                    instance.obstruction = instance.transported_rank.obstruction;
            }
        }
        instance.valid = instance.upper_graph.valid &&
                         instance.simulation.recurrent_descent &&
                         instance.transported_rank.valid;
        c.every_simulation_total &= instance.simulation.recurrent_descent;
        c.every_rank_transport_strict &= instance.transported_rank.valid;
        c.instances.push_back(std::move(instance));
    }
    c.finite_family_closed = c.base_acyclic && c.every_simulation_total &&
                             c.every_rank_transport_strict;
    if (!c.finite_family_closed)
        c.obstruction = "fixed-base shell reduction failed in announced range";
    return c;
}

/** Typed premises of the dimension/radius-parametric theorem.
 *
 * These fields must be produced by derivation operations.  This composition
 * deliberately contains no fallback to finite enumeration: once both
 * uniform premises are present, all maximum shells M>=2 are excluded.
 */
struct UniformFixedBasePremises {
    std::size_t minimum_dimension = 2;
    bool shell_two_rank_for_all_dimensions = false;
    bool fixed_base_simulation_for_all_radii = false;
    bool transition_semantics_replayed = false;
    std::string shell_two_operation;
    std::string simulation_operation;
};

struct ParametricMaximumShellCertificate {
    UniformFixedBasePremises premises;
    bool rank_transport_composition = false;
    bool every_maximum_shell_acyclic = false;
    bool maximal_recurrent_shell_contradiction = false;
    bool valid = false;
    std::string obstruction;
};

inline ParametricMaximumShellCertificate certify_parametric_maximum_shell(
    UniformFixedBasePremises premises) {
    ParametricMaximumShellCertificate c;
    c.premises = std::move(premises);
    // The statement once targeted by this composition is false: for every
    // M>=1 and n=q*(4M)+1 there is a closed triangular-wave shell cycle.
    // Retain the operation only as an explicit rejection point so fabricated
    // Boolean premises cannot resurrect the refuted theorem.
    c.obstruction =
        "uniform maximum-shell exclusion is refuted by triangular-wave terminal cycles";
    return c;
}

struct UniformShellPartitionPremises {
    bool triangular_terminal_family_derived = false;
    bool every_other_recurrent_shell_classified = false;
    bool residual_first_return_ranked = false;
    bool transition_semantics_replayed = false;
    std::string terminal_family_operation;
    std::string residual_partition_operation;
};

struct ParametricMaximumShellPartitionCertificate {
    UniformShellPartitionPremises premises;
    bool terminal_components_retained = false;
    bool residual_components_excluded = false;
    bool recurrent_shell_partitioned = false;
    bool valid = false;
    std::string obstruction;
};

inline ParametricMaximumShellPartitionCertificate
 certify_parametric_maximum_shell_partition(UniformShellPartitionPremises premises) {
    ParametricMaximumShellPartitionCertificate c;
    c.premises = std::move(premises);
    if (!c.premises.triangular_terminal_family_derived ||
        c.premises.terminal_family_operation.empty()) {
        c.obstruction = "missing triangular terminal-shell derivation";
        return c;
    }
    if (!c.premises.every_other_recurrent_shell_classified ||
        !c.premises.residual_first_return_ranked ||
        c.premises.residual_partition_operation.empty()) {
        c.obstruction = "missing residual recurrent-shell classification/rank";
        return c;
    }
    if (!c.premises.transition_semantics_replayed) {
        c.obstruction = "shell partition lacks replay provenance";
        return c;
    }
    c.terminal_components_retained = true;
    c.residual_components_excluded = true;
    c.recurrent_shell_partitioned = true;
    c.valid = true;
    return c;
}

} // namespace ravel::proof
