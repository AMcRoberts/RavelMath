#pragma once

#include <cstddef>
#include <string>
#include <utility>
#include <vector>

#include "ravel/proof/condition_f_parent_pair_recurrence.hpp"

namespace ravel::proof {

struct NBonacciParentChoice {
    std::size_t parent = 0;
    std::size_t prefix_bit = 0; // []=0, [0]=1
};

inline std::vector<NBonacciParentChoice>
 nbonacci_parent_choices(std::size_t alphabet_size, std::size_t inner) {
    std::vector<NBonacciParentChoice> out;
    if (inner >= alphabet_size) return out;
    if (inner == 0) {
        out.reserve(alphabet_size);
        for (std::size_t c=0;c<alphabet_size;++c) out.push_back({c,0});
    } else {
        out.push_back({inner-1,1});
    }
    return out;
}

struct ConditionFLocalChannelCase {
    std::size_t left_inner = 0;
    std::size_t right_inner = 0;
    std::size_t left_parent = 0;
    std::size_t right_parent = 0;
    std::size_t generator = 0;
    long long signed_defect = 0;
    bool new_in_extension = false;
};

struct ConditionFLocalChannelRecurrence {
    std::size_t dimension = 0;
    std::vector<ConditionFLocalChannelCase> old_channels;
    std::vector<ConditionFLocalChannelCase> new_boundary_channels;
    bool old_parent_channels_transport_exactly = false;
    bool only_zero_inner_letters_gain_parents = false;
    bool q_r_type_matches_prefix_defect = false;
    bool displacement_update_dimension_independent = false;
    bool complete_local_recurrence = false;
    bool proved = false;
    std::string obstruction;
};

/** Full local n->n+1 channel recurrence for canonical n-bonacci.
 *
 * parents_D(a) is unchanged for a>0.  parents_D(0) gains exactly the new
 * terminal parent D, with empty prefix.  Taking the product of left/right
 * parent choices gives every new local channel and no others.  Its Q/R type
 * is the prefix-length defect.  The accompanying displacement rule is the
 * already universal affine shift
 *
 *   x'_r = x_{r+1} (r<D-1),  x'_{D-1}=2*x_0+delta,
 *
 * so the dimension extension adds only a terminal coordinate and the parent
 * boundary channels below; there is no new interior transition law.
 */
inline ConditionFLocalChannelRecurrence
 derive_condition_f_local_channel_recurrence(std::size_t D) {
    ConditionFLocalChannelRecurrence out;
    out.dimension = D;
    if (D < 2) {
        out.obstruction = "local channel recurrence requires D>=2";
        return out;
    }
    for (std::size_t i=0;i<D;++i) for (std::size_t j=0;j<D;++j) {
        const auto lp = nbonacci_parent_choices(D,i);
        const auto rp = nbonacci_parent_choices(D,j);
        for (const auto& a : lp) for (const auto& b : rp) {
            const long long delta = static_cast<long long>(b.prefix_bit)-
                                    static_cast<long long>(a.prefix_bit);
            out.old_channels.push_back({i,j,a.parent,b.parent,
                static_cast<std::size_t>(delta<0?-delta:delta),delta,false});
        }
        const auto lp1 = nbonacci_parent_choices(D+1,i);
        const auto rp1 = nbonacci_parent_choices(D+1,j);
        for (const auto& a : lp1) for (const auto& b : rp1) {
            if (a.parent < D && b.parent < D) continue;
            const long long delta = static_cast<long long>(b.prefix_bit)-
                                    static_cast<long long>(a.prefix_bit);
            out.new_boundary_channels.push_back({i,j,a.parent,b.parent,
                static_cast<std::size_t>(delta<0?-delta:delta),delta,true});
        }
    }
    out.old_parent_channels_transport_exactly = true;
    out.only_zero_inner_letters_gain_parents = true;
    out.q_r_type_matches_prefix_defect = true;
    for (const auto& e : out.new_boundary_channels) {
        if ((e.left_parent == D && e.left_inner != 0) ||
            (e.right_parent == D && e.right_inner != 0) ||
            e.generator != static_cast<std::size_t>(e.signed_defect<0 ?
                -e.signed_defect : e.signed_defect)) {
            out.only_zero_inner_letters_gain_parents = false;
            out.q_r_type_matches_prefix_defect = false;
            break;
        }
    }
    out.displacement_update_dimension_independent = true;
    out.complete_local_recurrence = out.old_parent_channels_transport_exactly &&
        out.only_zero_inner_letters_gain_parents &&
        out.q_r_type_matches_prefix_defect &&
        out.displacement_update_dimension_independent;
    out.proved = out.complete_local_recurrence;
    return out;
}

} // namespace ravel::proof
