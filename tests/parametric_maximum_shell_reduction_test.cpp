#include <cassert>
#include <iostream>

#include "ravel/proof/parametric_maximum_shell_reduction.hpp"

using namespace ravel::proof;

int main() {
    // These are deliberately exact graph/relation tests, not sampled paths.
    const auto n2 = derive_fixed_base_shell_reduction(2, 8);
    assert(n2.finite_family_closed);
    const auto n3 = derive_fixed_base_shell_reduction(3, 8);
    assert(n3.finite_family_closed);
    const auto n4 = derive_fixed_base_shell_reduction(4, 4);
    assert(n4.finite_family_closed);

    auto missing = certify_parametric_maximum_shell({});
    assert(!missing.valid);

    UniformFixedBasePremises premises;
    premises.shell_two_rank_for_all_dimensions = true;
    premises.fixed_base_simulation_for_all_radii = true;
    premises.transition_semantics_replayed = true;
    premises.shell_two_operation = "derive_shell_two_dimension_shadow_rank";
    premises.simulation_operation = "derive_symbolic_fixed_base_shell_simulation";
    const auto composed = certify_parametric_maximum_shell(premises);
    assert(!composed.valid);
    assert(composed.obstruction.find("refuted") != std::string::npos);

    UniformShellPartitionPremises partition;
    partition.triangular_terminal_family_derived = true;
    partition.every_other_recurrent_shell_classified = true;
    partition.residual_first_return_ranked = true;
    partition.transition_semantics_replayed = true;
    partition.terminal_family_operation = "derive_triangular_wave_terminal_shell";
    partition.residual_partition_operation = "derive_residual_shell_partition";
    const auto corrected = certify_parametric_maximum_shell_partition(partition);
    assert(corrected.valid);
    assert(corrected.recurrent_shell_partitioned);

    std::cout << "parametric maximum shell reduction PASS"
              << " n2_instances=" << n2.instances.size()
              << " n3_instances=" << n3.instances.size()
              << " n4_instances=" << n4.instances.size() << "\n";
}
