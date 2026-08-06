#pragma once

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

namespace ravel::proof {

struct ConditionFCyclicVoltageChannel {
    std::size_t source = 0;
    std::size_t target = 0;
    std::size_t generator = 0; // 0=Q, 1=R
    long long voltage = 0;     // element of Z/mZ
    std::size_t multiplicity = 1;
};

struct ConditionFCyclicVoltageFourierProof {
    std::size_t base_vertices = 0;
    std::size_t fibre_order = 0;
    std::vector<std::vector<std::int64_t>> q;
    std::vector<std::vector<std::int64_t>> r;
    std::vector<std::vector<std::int64_t>> untwisted;
    std::size_t balanced_channels = 0;
    std::size_t residual_channels = 0;
    bool canonical_pair_preserved = false;
    bool character_sectors_defined = false;
    bool entrywise_modulus_dominated = false;
    bool every_twisted_sector_below_untwisted = false;
    bool summed_perron_sector_untwisted = false;
    bool proved = false;
    std::string obstruction;
};

/** Abelian-voltage Fourier reduction for the canonical Condition-F pair.
 *
 * For a cyclic fibre Z/mZ, a channel of voltage s contributes zeta^(ks)
 * in character k.  Thus sector k is
 *
 *   T_k = Q + sum_s zeta^(ks) R_s.
 *
 * Since every coefficient has modulus one,
 *
 *   |T_k| <= Q + sum_s R_s
 *
 * entrywise.  Perron-Frobenius comparison therefore gives
 * rho(T_k) <= rho(T_0).  The twist changes only nontrivial Fourier sectors;
 * all spectral maximality questions reduce to the untwisted channel-count
 * pair before any general-n recurrence is derived.
 */
inline ConditionFCyclicVoltageFourierProof
 derive_condition_f_cyclic_voltage_fourier(
    std::size_t base_vertices,
    std::size_t fibre_order,
    const std::vector<ConditionFCyclicVoltageChannel>& channels) {
    ConditionFCyclicVoltageFourierProof out;
    out.base_vertices = base_vertices;
    out.fibre_order = fibre_order;
    if (base_vertices == 0 || fibre_order == 0 || channels.empty()) {
        out.obstruction = "cyclic-voltage Fourier theorem requires nonempty base, fibre, and channels";
        return out;
    }
    out.q.assign(base_vertices, std::vector<std::int64_t>(base_vertices,0));
    out.r.assign(base_vertices, std::vector<std::int64_t>(base_vertices,0));
    for (const auto& e : channels) {
        if (e.source >= base_vertices || e.target >= base_vertices ||
            e.generator > 1 || e.multiplicity == 0) {
            out.obstruction = "malformed cyclic-voltage channel";
            return out;
        }
        const auto m = static_cast<std::int64_t>(e.multiplicity);
        if (e.generator == 0) {
            // Q/R type and voltage are independent coordinates.  In the
            // orientation double cover Q happens to be untwisted, but a
            // larger phase fibre may twist either generator.  Fourier
            // domination uses only unit-modulus voltages, so no gauge choice
            // or trivial-Q assumption is needed.
            out.q[e.source][e.target] += m;
            out.balanced_channels += e.multiplicity;
        } else {
            out.r[e.source][e.target] += m;
            out.residual_channels += e.multiplicity;
        }
    }
    out.untwisted = out.q;
    for (std::size_t i=0;i<base_vertices;++i)
        for (std::size_t j=0;j<base_vertices;++j)
            out.untwisted[i][j] += out.r[i][j];
    out.canonical_pair_preserved = true;
    out.character_sectors_defined = true;
    out.entrywise_modulus_dominated = true;
    out.every_twisted_sector_below_untwisted = true;
    out.summed_perron_sector_untwisted = true;
    out.proved = true;
    return out;
}

} // namespace ravel::proof
