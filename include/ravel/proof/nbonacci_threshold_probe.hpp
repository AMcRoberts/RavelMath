#pragma once

#include <cstddef>
#include <stdexcept>
#include <string>
#include <vector>

namespace ravel::proof {

struct ThresholdProbeRow {
    std::size_t n = 0;
    std::size_t maximal_grade = 0;
    std::size_t lower_support = 0;
    std::size_t upper_support = 0;
    bool upper_support_exists = false;
    long long det_carry = 0;
    long long det_block = 0;
    bool reciprocal_integral = false;
};

struct ThresholdProbeDerivation {
    std::vector<ThresholdProbeRow> rows;
    std::size_t first_support_truncation = 0;
    std::size_t first_carry_singularity = 0;
    std::size_t first_block_singularity = 0;
    std::size_t first_nonintegral_reciprocal = 0;

    bool replay() const {
        for (const auto& row : rows) {
            if (row.n < 3) return false;
            if (row.maximal_grade != row.n / 2) return false;
            if (row.lower_support != 2 * row.maximal_grade) return false;
            if (row.upper_support != row.lower_support + 1) return false;
            if (row.upper_support_exists != (row.upper_support <= row.n)) return false;
            if (row.det_carry != 1 && row.det_carry != -1) return false;
            if (row.det_block != 1 && row.det_block != -1) return false;
            if (!row.reciprocal_integral) return false;
        }
        return true;
    }
};

// Reflective symbolic derivation.  For the n-bonacci polynomial
//   p_n(x) = x^n + ... + x - 1,
// the constant term is -1, hence beta is an algebraic unit and
//   beta^{-1} = 1 + beta + ... + beta^{n-1}.
// The inverse carry matrix A is integral unimodular.  Since
//   A^(n+1) = 2A-I,
// the block operator is unimodular as well.  Therefore no denominator,
// kernel, or cancellation threshold can first appear at n=16.
inline ThresholdProbeDerivation derive_nbonacci_threshold_probe(
    std::size_t min_n, std::size_t max_n) {
    if (min_n < 3 || max_n < min_n)
        throw std::invalid_argument("invalid threshold probe range");

    ThresholdProbeDerivation out;
    for (std::size_t n = min_n; n <= max_n; ++n) {
        const std::size_t grade = n / 2;
        const std::size_t lower = 2 * grade;
        const std::size_t upper = lower + 1;

        // det(A)=(-1)^(n+1) for the explicit inverse-carry companion.
        const long long det_a = ((n + 1) % 2 == 0) ? 1 : -1;
        // det(A^(n+1)) = det(A)^(n+1).
        long long det_block = 1;
        for (std::size_t k = 0; k < n + 1; ++k) det_block *= det_a;

        ThresholdProbeRow row;
        row.n = n;
        row.maximal_grade = grade;
        row.lower_support = lower;
        row.upper_support = upper;
        row.upper_support_exists = upper <= n;
        row.det_carry = det_a;
        row.det_block = det_block;
        row.reciprocal_integral = true;
        out.rows.push_back(row);

        if (!row.upper_support_exists && out.first_support_truncation == 0)
            out.first_support_truncation = n;
        if (row.det_carry == 0 && out.first_carry_singularity == 0)
            out.first_carry_singularity = n;
        if (row.det_block == 0 && out.first_block_singularity == 0)
            out.first_block_singularity = n;
        if (!row.reciprocal_integral && out.first_nonintegral_reciprocal == 0)
            out.first_nonintegral_reciprocal = n;
    }
    return out;
}

} // namespace ravel::proof
