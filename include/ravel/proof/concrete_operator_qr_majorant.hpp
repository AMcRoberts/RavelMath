#pragma once

#include <cstddef>
#include <cstdint>
#include <limits>
#include <numeric>
#include <string>
#include <utility>
#include <vector>

#include "ravel/proof/norm_weighted_qr_majorant.hpp"

namespace ravel::proof {

struct ExactSignedRational {
    std::int64_t numerator = 0;
    std::uint64_t denominator = 1;
};

inline bool signed_rational_valid(const ExactSignedRational& x) {
    return x.denominator != 0 && x.numerator != std::numeric_limits<std::int64_t>::min();
}

inline ExactSignedRational reduce_signed_rational(ExactSignedRational x) {
    if (!signed_rational_valid(x)) return x;
    const std::uint64_t a = x.numerator < 0
        ? static_cast<std::uint64_t>(-x.numerator)
        : static_cast<std::uint64_t>(x.numerator);
    const auto g = std::gcd(a, x.denominator);
    x.numerator /= static_cast<std::int64_t>(g);
    x.denominator /= g;
    return x;
}

inline bool signed_rational_add(const ExactSignedRational& a,
                                const ExactSignedRational& b,
                                ExactSignedRational& out) {
    if (!signed_rational_valid(a) || !signed_rational_valid(b)) return false;
    const __int128 n = static_cast<__int128>(a.numerator) * b.denominator +
                       static_cast<__int128>(b.numerator) * a.denominator;
    const unsigned __int128 d =
        static_cast<unsigned __int128>(a.denominator) * b.denominator;
    if (n < std::numeric_limits<std::int64_t>::min() ||
        n > std::numeric_limits<std::int64_t>::max() ||
        d > std::numeric_limits<std::uint64_t>::max()) return false;
    out = reduce_signed_rational({static_cast<std::int64_t>(n),
                                  static_cast<std::uint64_t>(d)});
    return true;
}

inline bool signed_rational_mul(const ExactSignedRational& a,
                                const ExactSignedRational& b,
                                ExactSignedRational& out) {
    if (!signed_rational_valid(a) || !signed_rational_valid(b)) return false;
    const __int128 n = static_cast<__int128>(a.numerator) * b.numerator;
    const unsigned __int128 d =
        static_cast<unsigned __int128>(a.denominator) * b.denominator;
    if (n < std::numeric_limits<std::int64_t>::min() ||
        n > std::numeric_limits<std::int64_t>::max() ||
        d > std::numeric_limits<std::uint64_t>::max()) return false;
    out = reduce_signed_rational({static_cast<std::int64_t>(n),
                                  static_cast<std::uint64_t>(d)});
    return true;
}

inline ExactNonnegativeRational signed_rational_abs(const ExactSignedRational& x) {
    return reduce_exact_rational({x.numerator < 0
        ? static_cast<std::uint64_t>(-x.numerator)
        : static_cast<std::uint64_t>(x.numerator), x.denominator});
}

using ExactSignedMatrix = std::vector<std::vector<ExactSignedRational>>;

inline ExactSignedMatrix signed_zero_matrix(std::size_t rows, std::size_t cols) {
    return ExactSignedMatrix(rows, std::vector<ExactSignedRational>(cols, {0, 1}));
}

inline ExactSignedMatrix signed_identity_matrix(std::size_t n) {
    auto out = signed_zero_matrix(n, n);
    for (std::size_t i = 0; i < n; ++i) out[i][i] = {1, 1};
    return out;
}

inline bool signed_matrix_square_well_formed(const ExactSignedMatrix& a,
                                             std::size_t n) {
    if (a.size() != n) return false;
    for (const auto& row : a) {
        if (row.size() != n) return false;
        for (const auto& x : row) if (!signed_rational_valid(x)) return false;
    }
    return true;
}

inline bool signed_matrix_add_in_place(ExactSignedMatrix& a,
                                       const ExactSignedMatrix& b) {
    if (a.size() != b.size()) return false;
    for (std::size_t i = 0; i < a.size(); ++i) {
        if (a[i].size() != b[i].size()) return false;
        for (std::size_t j = 0; j < a[i].size(); ++j) {
            ExactSignedRational sum;
            if (!signed_rational_add(a[i][j], b[i][j], sum)) return false;
            a[i][j] = sum;
        }
    }
    return true;
}

inline bool signed_matrix_multiply(const ExactSignedMatrix& a,
                                   const ExactSignedMatrix& b,
                                   ExactSignedMatrix& out) {
    if (a.empty() || b.empty() || a[0].size() != b.size()) return false;
    const std::size_t rows = a.size(), inner = b.size(), cols = b[0].size();
    for (const auto& row : a) if (row.size() != inner) return false;
    for (const auto& row : b) if (row.size() != cols) return false;
    out = signed_zero_matrix(rows, cols);
    for (std::size_t i = 0; i < rows; ++i)
        for (std::size_t k = 0; k < inner; ++k)
            for (std::size_t j = 0; j < cols; ++j) {
                ExactSignedRational p, s;
                if (!signed_rational_mul(a[i][k], b[k][j], p) ||
                    !signed_rational_add(out[i][j], p, s)) return false;
                out[i][j] = s;
            }
    return true;
}

/** Exact induced infinity norm: max_i sum_j |a_ij|. */
inline bool exact_induced_infinity_norm(const ExactSignedMatrix& a,
                                        ExactNonnegativeRational& out) {
    if (a.empty() || a[0].empty()) return false;
    const std::size_t cols = a[0].size();
    out = {0, 1};
    for (const auto& row : a) {
        if (row.size() != cols) return false;
        ExactNonnegativeRational sum{0, 1};
        for (const auto& x : row) {
            if (!signed_rational_valid(x)) return false;
            ExactNonnegativeRational next;
            if (!exact_rational_add(sum, signed_rational_abs(x), next)) return false;
            sum = next;
        }
        if (exact_rational_lt(out, sum)) out = sum;
    }
    return true;
}

struct ConcreteOperatorQRChannel {
    std::size_t source = 0;
    std::size_t target = 0;
    std::size_t generator = 0;
    std::size_t multiplicity = 1;
    ExactSignedMatrix fibre_operator;
};

struct ConcretePowerMajorantWitness {
    std::size_t exponent = 0;
    ExactSignedMatrix twisted_power;
    ExactRationalMatrix scalar_majorant_power;
    bool every_block_infinity_norm_bounded = false;
};

struct ConcreteOperatorQRMajorantCertificate {
    std::size_t base_vertices = 0;
    std::size_t fibre_dimension = 0;
    std::vector<ExactNonnegativeRational> derived_edge_norm_bounds;
    NormWeightedQRMajorantCertificate qr_majorant;
    ExactSignedMatrix twisted_transfer;
    ExactRationalMatrix scalar_transfer_majorant;
    std::vector<ConcretePowerMajorantWitness> power_witnesses;

    bool concrete_matrices_well_formed = false;
    bool edge_norm_bounds_computed_exactly = false;
    bool induced_infinity_norm_is_submultiplicative = false;
    bool twisted_block_transfer_assembled_exactly = false;
    bool scalar_majorant_assembled_exactly = false;
    bool universal_block_power_majorization_derived = false;
    bool finite_power_replay_verified = false;
    bool gelfand_spectral_radius_closure_derived = false;
    bool spectral_radius_twist_at_most_majorant = false;
    bool proved = false;
    std::string obstruction;
};

inline bool assemble_concrete_transfer(
    std::size_t base_vertices,
    std::size_t fibre_dimension,
    const std::vector<ConcreteOperatorQRChannel>& channels,
    ExactSignedMatrix& out) {
    const std::size_t n = base_vertices * fibre_dimension;
    out = signed_zero_matrix(n, n);
    for (const auto& e : channels) {
        for (std::size_t copy = 0; copy < e.multiplicity; ++copy)
            for (std::size_t i = 0; i < fibre_dimension; ++i)
                for (std::size_t j = 0; j < fibre_dimension; ++j) {
                    // target block rows, source block columns
                    auto& dst = out[e.target * fibre_dimension + i]
                                   [e.source * fibre_dimension + j];
                    ExactSignedRational sum;
                    if (!signed_rational_add(dst, e.fibre_operator[i][j], sum))
                        return false;
                    dst = sum;
                }
    }
    return true;
}

inline bool block_infinity_norms_bounded(
    const ExactSignedMatrix& twisted,
    const ExactRationalMatrix& scalar,
    std::size_t base_vertices,
    std::size_t fibre_dimension) {
    if (twisted.size() != base_vertices * fibre_dimension ||
        scalar.size() != base_vertices) return false;
    for (std::size_t target = 0; target < base_vertices; ++target)
        for (std::size_t source = 0; source < base_vertices; ++source) {
            ExactSignedMatrix block = signed_zero_matrix(fibre_dimension, fibre_dimension);
            for (std::size_t i = 0; i < fibre_dimension; ++i)
                for (std::size_t j = 0; j < fibre_dimension; ++j)
                    block[i][j] = twisted[target * fibre_dimension + i]
                                          [source * fibre_dimension + j];
            ExactNonnegativeRational norm;
            if (!exact_induced_infinity_norm(block, norm) ||
                !exact_rational_le(norm, scalar[source][target])) return false;
        }
    return true;
}

/**
 * Derive concrete edge norms, the exact norm-weighted Q/R pair, and the
 * operator-valued transfer majorant.  The universal closure is justified by
 * block multiplication, triangle inequality, and submultiplicativity of the
 * induced infinity norm.  Exact replay powers are independent executable
 * witnesses; they are not used as a substitute for the universal induction.
 */
inline ConcreteOperatorQRMajorantCertificate derive_concrete_operator_qr_majorant(
    std::size_t base_vertices,
    std::size_t fibre_dimension,
    const std::vector<ConcreteOperatorQRChannel>& channels,
    const std::vector<QRGeneratorWord>& boundary_words = {},
    std::size_t replay_power_through = 4) {
    ConcreteOperatorQRMajorantCertificate out;
    out.base_vertices = base_vertices;
    out.fibre_dimension = fibre_dimension;
    if (base_vertices == 0 || fibre_dimension == 0 || channels.empty()) {
        out.obstruction = "concrete operator Q/R majorant requires nonempty base, fibre, and channels";
        return out;
    }

    std::vector<NormWeightedQRChannel> weighted;
    out.concrete_matrices_well_formed = true;
    for (const auto& e : channels) {
        if (e.source >= base_vertices || e.target >= base_vertices ||
            e.generator > 1 || e.multiplicity == 0 ||
            !signed_matrix_square_well_formed(e.fibre_operator, fibre_dimension)) {
            out.concrete_matrices_well_formed = false;
            out.obstruction = "malformed concrete operator Q/R channel";
            return out;
        }
        ExactNonnegativeRational norm;
        if (!exact_induced_infinity_norm(e.fibre_operator, norm)) {
            out.obstruction = "failed exact induced-infinity norm derivation";
            return out;
        }
        out.derived_edge_norm_bounds.push_back(norm);
        weighted.push_back({e.source, e.target, e.generator, e.multiplicity, norm});
    }
    out.edge_norm_bounds_computed_exactly = true;
    out.induced_infinity_norm_is_submultiplicative = true;
    out.qr_majorant = derive_norm_weighted_qr_majorant(
        base_vertices, weighted, boundary_words);
    if (!out.qr_majorant.proved) {
        out.obstruction = out.qr_majorant.obstruction;
        return out;
    }
    if (!assemble_concrete_transfer(base_vertices, fibre_dimension, channels,
                                    out.twisted_transfer)) {
        out.obstruction = "exact overflow while assembling twisted block transfer";
        return out;
    }
    out.twisted_block_transfer_assembled_exactly = true;
    out.scalar_transfer_majorant = out.qr_majorant.q_norm;
    if (!exact_matrix_add_in_place(out.scalar_transfer_majorant,
                                   out.qr_majorant.r_norm)) {
        out.obstruction = "exact overflow while assembling scalar transfer majorant";
        return out;
    }
    out.scalar_majorant_assembled_exactly = true;

    // Universal theorem: block norm matrix of T is <= A; block multiplication
    // plus triangle/submultiplicativity inductively gives blockNorm(T^k)<=A^k.
    out.universal_block_power_majorization_derived =
        block_infinity_norms_bounded(out.twisted_transfer,
                                     out.scalar_transfer_majorant,
                                     base_vertices, fibre_dimension);
    if (!out.universal_block_power_majorization_derived) {
        out.obstruction = "first-power block majorant failed";
        return out;
    }

    ExactSignedMatrix tp = signed_identity_matrix(base_vertices * fibre_dimension);
    ExactRationalMatrix ap = exact_identity_matrix(base_vertices);
    out.finite_power_replay_verified = true;
    for (std::size_t k = 0; k <= replay_power_through; ++k) {
        ConcretePowerMajorantWitness witness;
        witness.exponent = k;
        witness.twisted_power = tp;
        witness.scalar_majorant_power = ap;
        witness.every_block_infinity_norm_bounded =
            block_infinity_norms_bounded(tp, ap, base_vertices, fibre_dimension);
        out.finite_power_replay_verified &= witness.every_block_infinity_norm_bounded;
        out.power_witnesses.push_back(std::move(witness));
        if (k != replay_power_through) {
            ExactSignedMatrix tnext;
            ExactRationalMatrix anext;
            if (!signed_matrix_multiply(tp, out.twisted_transfer, tnext) ||
                !exact_matrix_multiply(ap, out.scalar_transfer_majorant, anext)) {
                out.obstruction = "exact overflow during power replay";
                return out;
            }
            tp = std::move(tnext);
            ap = std::move(anext);
        }
    }

    // Gelfand closure: for finite-dimensional operators and any
    // submultiplicative matrix norm, rho(T)=lim ||T^k||^(1/k). The universal
    // block-power inequality therefore implies rho(T)<=rho(A).
    out.gelfand_spectral_radius_closure_derived =
        out.universal_block_power_majorization_derived &&
        out.induced_infinity_norm_is_submultiplicative;
    out.spectral_radius_twist_at_most_majorant =
        out.gelfand_spectral_radius_closure_derived;
    out.proved = out.concrete_matrices_well_formed &&
        out.edge_norm_bounds_computed_exactly &&
        out.twisted_block_transfer_assembled_exactly &&
        out.scalar_majorant_assembled_exactly &&
        out.universal_block_power_majorization_derived &&
        out.finite_power_replay_verified &&
        out.spectral_radius_twist_at_most_majorant;
    return out;
}

} // namespace ravel::proof
