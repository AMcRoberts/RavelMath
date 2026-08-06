#pragma once

#include <string>

#include "ravel/proof/multinacci_branching_reduction.hpp"
#include "ravel/proof/finite_quotient_core_maximality.hpp"
#include "ravel/proof/condition_f_component_maximality.hpp"
#include "ravel/proof/condition_f_channel_count_collapse.hpp"
#include "ravel/proof/condition_f_joint_pair_comparison.hpp"

namespace ravel::proof {

struct UniversalDominanceReductionCertificate {
    std::size_t dimension = 0;
    BranchingTerminalReductionTheorem shell_reduction;
    bool core_occurs = false;
    bool terminal_components_below_core = false;
    bool finite_quotient_core_maximal = false;
    bool upper_bound_derived = false;
    bool lower_bound_derived = false;
    bool equality_derived = false;
    std::string obstruction;
};

/** Final sandwich after the unbounded terminal direction is quotiented out.
 *
 * Terminal permutation SCCs have rho=1.  Every branching SCC has the same
 * rho as its finite Condition-F quotient.  Therefore core occurrence plus
 * spectral maximality of the core among finite quotient SCCs is sufficient;
 * literal uniqueness of the core SCC and a finite absolute coefficient hull
 * are not required.
 */
inline UniversalDominanceReductionCertificate
 compose_universal_dominance_from_finite_quotient(
    std::size_t n,
    bool core_occurs,
    bool core_rho_strictly_above_one,
    bool finite_quotient_core_maximal) {
    UniversalDominanceReductionCertificate c;
    c.dimension = n;
    c.shell_reduction = derive_multinacci_branching_terminal_reduction(n);
    c.core_occurs = core_occurs;
    c.terminal_components_below_core = core_rho_strictly_above_one;
    c.finite_quotient_core_maximal = finite_quotient_core_maximal;
    c.lower_bound_derived = c.core_occurs;
    c.upper_bound_derived = c.shell_reduction.proved &&
        c.terminal_components_below_core && c.finite_quotient_core_maximal;
    c.equality_derived = c.lower_bound_derived && c.upper_bound_derived;
    if (!c.equality_derived)
        c.obstruction = "finite branching quotient core maximality is not yet certified";
    return c;
}

inline UniversalDominanceReductionCertificate
 compose_universal_dominance_from_finite_quotient(
    std::size_t n,
    bool core_occurs,
    bool core_rho_strictly_above_one,
    const FiniteQuotientCoreMaximality& quotient_maximality) {
    return compose_universal_dominance_from_finite_quotient(
        n, core_occurs, core_rho_strictly_above_one,
        quotient_maximality.core_spectrally_maximal);
}


inline UniversalDominanceReductionCertificate
 compose_universal_dominance_from_finite_quotient(
    std::size_t n,
    bool core_occurs,
    bool core_rho_strictly_above_one,
    const ConditionFComponentMaximalityProof& quotient_maximality) {
    return compose_universal_dominance_from_finite_quotient(
        n, core_occurs, core_rho_strictly_above_one,
        quotient_maximality.core_spectrally_maximal);
}

inline UniversalDominanceReductionCertificate
 compose_universal_dominance_from_finite_quotient(
    std::size_t n,
    bool core_occurs,
    bool core_rho_strictly_above_one,
    const ConditionFChannelCountCollapse& quotient_collapse) {
    return compose_universal_dominance_from_finite_quotient(
        n, core_occurs, core_rho_strictly_above_one,
        quotient_collapse.core_spectrally_maximal_in_lift);
}

inline UniversalDominanceReductionCertificate
 compose_universal_dominance_from_finite_quotient(
    std::size_t n,
    bool core_occurs,
    bool core_rho_strictly_above_one,
    const ConditionFJointPairComparison& pair_comparison) {
    return compose_universal_dominance_from_finite_quotient(
        n, core_occurs, core_rho_strictly_above_one,
        pair_comparison.quotient_core_pair_maximal);
}

} // namespace ravel::proof
