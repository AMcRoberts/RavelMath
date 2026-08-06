#pragma once

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

#include "ravel/proof/condition_f_prefix_two_matrix.hpp"

namespace ravel::proof {

struct ConditionFTwoSheetFourierProof {
    std::size_t base_vertices = 0;
    std::vector<std::vector<std::int64_t>> q_balanced;
    std::vector<std::vector<std::int64_t>> r_residual;
    std::vector<std::vector<std::int64_t>> even_sector;
    std::vector<std::vector<std::int64_t>> odd_sector;
    bool balanced_preserves_orientation = false;
    bool residual_flips_orientation = false;
    bool lift_decomposes_as_even_odd = false;
    bool perron_sector_is_untwisted = false;
    bool proved = false;
    std::string obstruction;
};

inline std::vector<std::vector<std::int64_t>> matrix_add_signed(
    const std::vector<std::vector<std::int64_t>>& a,
    const std::vector<std::vector<std::int64_t>>& b,
    std::int64_t sign) {
    auto out = a;
    for (std::size_t i = 0; i < out.size(); ++i)
        for (std::size_t j = 0; j < out.size(); ++j)
            out[i][j] += sign * b[i][j];
    return out;
}

/** Fourier decomposition of the orientation two-lift.
 *
 * Balanced prefix channels preserve the two orientation sheets and residual
 * channels swap them.  Therefore the lifted adjacency is
 *
 *      [ Q  R ]
 *      [ R  Q ]
 *
 * and the even/odd sheet transform gives Q+R and Q-R.  Since |Q-R| <= Q+R
 * entrywise, every eigenvalue in the twisted sector has modulus at most
 * rho(Q+R); the Perron sector is the untwisted channel-count matrix.
 */
inline ConditionFTwoSheetFourierProof derive_condition_f_two_sheet_fourier(
    const ConditionFPrefixTwoMatrixProof& prefix_pair) {
    ConditionFTwoSheetFourierProof out;
    out.base_vertices = prefix_pair.base_vertices;
    if (!prefix_pair.proved || prefix_pair.fibre_size != 2) {
        out.obstruction = "two-sheet Fourier theorem requires a proved two-sheet prefix pair";
        return out;
    }
    const auto n = prefix_pair.base_vertices;
    out.q_balanced.assign(n, std::vector<std::int64_t>(n,0));
    out.r_residual.assign(n, std::vector<std::int64_t>(n,0));
    out.balanced_preserves_orientation = true;
    out.residual_flips_orientation = true;
    for (std::size_t k = 0; k < prefix_pair.cocycle.edges.size(); ++k) {
        const auto& e = prefix_pair.cocycle.edges[k];
        if (e.generator == 0) {
            ++out.q_balanced[e.source][e.target];
            out.balanced_preserves_orientation &=
                e.permutation == std::vector<std::size_t>({0,1});
        } else {
            ++out.r_residual[e.source][e.target];
            out.residual_flips_orientation &=
                e.permutation == std::vector<std::size_t>({1,0});
        }
    }
    if (!out.balanced_preserves_orientation || !out.residual_flips_orientation) {
        out.obstruction = "prefix pair is not the canonical orientation two-lift";
        return out;
    }
    out.even_sector = matrix_add_signed(out.q_balanced,out.r_residual,+1);
    out.odd_sector = matrix_add_signed(out.q_balanced,out.r_residual,-1);
    out.lift_decomposes_as_even_odd = true;
    // Standard comparison: |Q-R| <= Q+R entrywise, hence rho(Q-R)<=rho(Q+R).
    out.perron_sector_is_untwisted = true;
    out.proved = true;
    return out;
}

} // namespace ravel::proof
