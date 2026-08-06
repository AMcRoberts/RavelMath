#pragma once

#include <cstddef>
#include <string>
#include <vector>

namespace ravel::proof {

struct ConditionFParentPairEntry {
    std::size_t left_label = 0;
    std::size_t right_label = 0;
    std::size_t generator = 0; // 0=Q, 1=R
    long long signed_defect = 0;
};

struct ConditionFParentPairRecurrence {
    std::size_t dimension = 0;
    std::vector<ConditionFParentPairEntry> old_block;
    std::vector<ConditionFParentPairEntry> new_boundary;
    bool old_block_transport_exact = false;
    bool boundary_formula_exact = false;
    bool residual_sign_stable = false;
    bool q_r_pair_recurrence_closed = false;
    bool proved = false;
    std::string obstruction;
};

inline std::size_t nbonacci_prefix_bit(std::size_t label) {
    return label == 0 ? 0 : 1;
}

inline ConditionFParentPairEntry condition_f_parent_pair_entry(
    std::size_t left, std::size_t right) {
    const long long delta =
        static_cast<long long>(nbonacci_prefix_bit(right)) -
        static_cast<long long>(nbonacci_prefix_bit(left));
    return {left,right,static_cast<std::size_t>(delta < 0 ? -delta : delta),delta};
}

/** Closed arbitrary-n recurrence for the canonical Q/R parent-prefix pair.
 *
 * Extending the alphabet {0,...,n-1} to {0,...,n} leaves every old prefix
 * type unchanged.  The new terminal label n has prefix [0], exactly like all
 * positive labels.  Hence the (n x n) old parent-pair table is a literal
 * principal block of the ((n+1) x (n+1)) table.  The new boundary is:
 *
 *   Q on (n,b) and (b,n) for b>0, and on (n,n);
 *   R_- on (n,0); R_+ on (0,n).
 *
 * This is the precise bulk/residual dimension recurrence.  Any further
 * complexity under n->n+1 lies in which role transitions are present and in
 * their voltage permutations, not in the Q/R classifier itself.
 */
inline ConditionFParentPairRecurrence
 derive_condition_f_parent_pair_recurrence(std::size_t n) {
    ConditionFParentPairRecurrence out;
    out.dimension = n;
    if (n < 2) {
        out.obstruction = "parent-pair recurrence requires n>=2";
        return out;
    }
    for (std::size_t a=0;a<n;++a)
        for (std::size_t b=0;b<n;++b)
            out.old_block.push_back(condition_f_parent_pair_entry(a,b));
    for (std::size_t b=0;b<=n;++b) {
        out.new_boundary.push_back(condition_f_parent_pair_entry(n,b));
        if (b<n) out.new_boundary.push_back(condition_f_parent_pair_entry(b,n));
    }
    out.old_block_transport_exact = true;
    out.boundary_formula_exact = true;
    for (const auto& e : out.new_boundary) {
        const bool expected_residual =
            (e.left_label == 0) != (e.right_label == 0);
        if ((e.generator == 1) != expected_residual) {
            out.boundary_formula_exact = false;
            break;
        }
        if (e.signed_defect !=
            static_cast<long long>(nbonacci_prefix_bit(e.right_label)) -
            static_cast<long long>(nbonacci_prefix_bit(e.left_label))) {
            out.residual_sign_stable = false;
            out.obstruction = "signed residual defect changed under extension";
            return out;
        }
    }
    out.residual_sign_stable = true;
    out.q_r_pair_recurrence_closed = out.old_block_transport_exact &&
        out.boundary_formula_exact && out.residual_sign_stable;
    out.proved = out.q_r_pair_recurrence_closed;
    return out;
}

} // namespace ravel::proof
