#pragma once

#include <cstddef>
#include <string>
#include <vector>

#include "ravel/proof/condition_f_local_channel_recurrence.hpp"

namespace ravel::proof {

enum class ConditionFBoundaryInterfaceKind {
    BalancedCornerFan,
    PositiveResidualStrip,
    NegativeResidualStrip
};

struct ConditionFBoundaryInterfaceChannel {
    ConditionFBoundaryInterfaceKind kind =
        ConditionFBoundaryInterfaceKind::BalancedCornerFan;
    ConditionFLocalChannelCase channel;
};

struct ConditionFBoundaryInterfaceRecurrence {
    std::size_t dimension = 0;
    std::vector<ConditionFBoundaryInterfaceChannel> channels;
    std::size_t balanced_corner_fan = 0;
    std::size_t positive_residual_strip = 0;
    std::size_t negative_residual_strip = 0;
    bool exactly_three_interface_families = false;
    bool no_positive_positive_boundary = false;
    bool interface_size_linear_in_dimension = false;
    bool finite_interface_reduction = false;
    bool proved = false;
    std::string obstruction;
};

/** Factor the n->n+1 terminal-parent correction into three interfaces.
 *
 * New parent D can occur only above inner 0.  Consequently every new local
 * channel belongs to exactly one family:
 *
 *  C: source inner pair (0,0), delta=0, 2D+1 balanced corner-fan channels;
 *  P: source (0,j), j>0, delta=+1, D-1 positive residual channels;
 *  N: source (i,0), i>0, delta=-1, D-1 negative residual channels.
 *
 * There are no new channels over positive-positive inner pairs.  Thus the
 * growing Condition-F pair is a transported principal block plus a fixed
 * three-type boundary interface.  This is the transfer-operator analogue of
 * the q/r sparse-cofactor split in the characteristic-polynomial proof.
 */
inline ConditionFBoundaryInterfaceRecurrence
 derive_condition_f_boundary_interface(std::size_t D) {
    ConditionFBoundaryInterfaceRecurrence out;
    out.dimension = D;
    const auto local = derive_condition_f_local_channel_recurrence(D);
    if (!local.proved) {
        out.obstruction = local.obstruction;
        return out;
    }
    out.no_positive_positive_boundary = true;
    for (const auto& e : local.new_boundary_channels) {
        ConditionFBoundaryInterfaceKind kind;
        if (e.left_inner == 0 && e.right_inner == 0 &&
            e.signed_defect == 0) {
            kind = ConditionFBoundaryInterfaceKind::BalancedCornerFan;
            ++out.balanced_corner_fan;
        } else if (e.left_inner == 0 && e.right_inner > 0 &&
                   e.signed_defect == 1) {
            kind = ConditionFBoundaryInterfaceKind::PositiveResidualStrip;
            ++out.positive_residual_strip;
        } else if (e.left_inner > 0 && e.right_inner == 0 &&
                   e.signed_defect == -1) {
            kind = ConditionFBoundaryInterfaceKind::NegativeResidualStrip;
            ++out.negative_residual_strip;
        } else {
            out.no_positive_positive_boundary = false;
            out.obstruction = "terminal-parent boundary channel lies outside the three interface families";
            return out;
        }
        out.channels.push_back({kind,e});
    }
    out.exactly_three_interface_families =
        out.balanced_corner_fan == 2*D+1 &&
        out.positive_residual_strip == D-1 &&
        out.negative_residual_strip == D-1;
    out.interface_size_linear_in_dimension = out.channels.size() == 4*D-1;
    out.finite_interface_reduction = out.exactly_three_interface_families &&
        out.no_positive_positive_boundary &&
        out.interface_size_linear_in_dimension;
    out.proved = out.finite_interface_reduction;
    return out;
}

} // namespace ravel::proof
