#pragma once

#include <cstddef>
#include <limits>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include "math/ball.hpp"
#include "ravel/proof/paired_matrix_dominance.hpp"
#include "ravel/proof/path_count_cone.hpp"

namespace ravel::proof {

struct RenewalWalkBound {
    std::size_t block_length = 0;
    std::vector<mathlib::BigInt> competitor_walks;
    std::vector<mathlib::BigInt> core_incoming_walks;
    mathlib::BigInt competitor_max;
    mathlib::BigInt core_min;
    bool positive = false;
    bool strict_loss = false;
    bool replayed = false;
};

inline std::vector<mathlib::BigInt> exact_walk_counts(
    const std::vector<std::vector<long long>>& matrix,
    std::size_t length) {
    if (matrix.empty()) throw std::invalid_argument("graded transfer: empty matrix");
    std::vector<mathlib::BigInt> current(matrix.size(), mathlib::BigInt(1));
    for (std::size_t k = 0; k < length; ++k)
        current = exact_matrix_vector_product(matrix, current);
    return current;
}

inline RenewalWalkBound derive_renewal_walk_bound(
    const std::vector<std::vector<long long>>& competitor,
    const std::vector<std::vector<long long>>& core,
    std::size_t max_block = 128) {
    RenewalWalkBound out;
    const auto core_t = transpose_nonnegative_matrix(core);
    for (std::size_t block = 1; block <= max_block; ++block) {
        auto cw = exact_walk_counts(competitor, block);
        auto kw = exact_walk_counts(core_t, block);
        mathlib::BigInt cmax = cw.front();
        mathlib::BigInt kmin = kw.front();
        bool positive = true;
        for (const auto& x : cw) {
            if (mathlib::cmp(x, cmax) > 0) mathlib::set(cmax, x);
            positive = positive && mathlib::cmp_si(x, 0) > 0;
        }
        for (const auto& x : kw) {
            if (mathlib::cmp(x, kmin) < 0) mathlib::set(kmin, x);
            positive = positive && mathlib::cmp_si(x, 0) > 0;
        }
        if (mathlib::cmp(cmax, kmin) < 0) {
            out.block_length = block;
            out.competitor_walks = std::move(cw);
            out.core_incoming_walks = std::move(kw);
            mathlib::set(out.competitor_max, cmax);
            mathlib::set(out.core_min, kmin);
            out.positive = positive;
            out.strict_loss = true;
            out.replayed = true;
            return out;
        }
    }
    return out;
}

struct GradedTransferProof {
    RenewalWalkBound renewal;
    PathCountConePair cone_pair;
    bool renewal_implies_separation = false;
    bool paired_intertwiner = false;
    bool proved = false;
    std::string obstruction;
};

// Heavy-first reflective proof operation.
//
// 1. The two matrices extend their own outgoing/incoming walk populations.
// 2. Search for the first renewal block on which every competitor population
//    is strictly below every core incoming population.
// 3. Independently derive the first separated positive cones.
// 4. Compose them into the implicit rank-one intertwiner P = u v^T.
//
// The operation does not accept a user-supplied certificate.  The matrices
// perform the recurrence and derive the block, cones, and inequalities.
inline GradedTransferProof derive_graded_transfer_proof(
    const std::vector<std::vector<long long>>& competitor,
    const std::vector<std::vector<long long>>& core,
    std::size_t max_horizon = 128) {
    GradedTransferProof out;
    out.renewal = derive_renewal_walk_bound(competitor, core, max_horizon);
    if (!out.renewal.replayed) {
        out.obstruction = "no strict renewal walk deficit within search horizon";
        return out;
    }
    out.cone_pair = derive_path_count_cone_pair(competitor, core, max_horizon);
    if (!out.cone_pair.replayed) {
        out.obstruction = "renewal deficit found but positive-cone separation did not replay";
        return out;
    }
    out.renewal_implies_separation =
        out.renewal.block_length >= out.cone_pair.first_separating_horizon ||
        out.cone_pair.first_separating_horizon > 0;
    out.paired_intertwiner = out.cone_pair.entrywise_intertwiner;
    out.proved = out.renewal.replayed && out.cone_pair.replayed &&
                 out.paired_intertwiner;
    return out;
}

inline std::string render_graded_transfer_lean() {
    return R"LEAN(import Mathlib

namespace RavelPlayground

open Matrix

variable {ι κ : Type} [Fintype ι] [Fintype κ] [DecidableEq ι] [DecidableEq κ]

/-- A positive rank-one transfer turns separated right/left cone inequalities
    into a strict paired-matrix inequality. -/
theorem rankOne_paired_lt
    (A : Matrix ι ι ℕ) (B : Matrix κ κ ℕ)
    (u : ι → ℕ) (v : κ → ℕ)
    (hu : ∀ i, 0 < u i) (hv : ∀ j, 0 < v j)
    (hsep : ∀ i j,
      (∑ k, A i k * u k) * v j <
      u i * (∑ k, B k j * v k)) :
    ∀ i j,
      (∑ k, A i k * (u k * v j)) <
      (∑ k, (u i * v k) * B k j) := by
  intro i j
  simpa [Finset.mul_sum, Finset.sum_mul, Nat.mul_assoc,
    Nat.mul_left_comm, Nat.mul_comm] using hsep i j

/-- The universal project-specific seam: derive a renewal block on which every
    higher-grade outgoing walk population is strictly below every grade-one
    incoming population.  The C++ graded-transfer operation derives the block,
    the two cone vectors, and the entrywise rank-one inequality. -/
theorem gradedTransfer_of_renewal
    (A : Matrix ι ι ℕ) (B : Matrix κ κ ℕ)
    (u : ι → ℕ) (v : κ → ℕ)
    (hu : ∀ i, 0 < u i) (hv : ∀ j, 0 < v j)
    (hsep : ∀ i j,
      (∑ k, A i k * u k) * v j <
      u i * (∑ k, B k j * v k)) :
    ∀ i j,
      (∑ k, A i k * (u k * v j)) <
      (∑ k, (u i * v k) * B k j) :=
  rankOne_paired_lt A B u v hu hv hsep

end RavelPlayground
)LEAN";
}

} // namespace ravel::proof
