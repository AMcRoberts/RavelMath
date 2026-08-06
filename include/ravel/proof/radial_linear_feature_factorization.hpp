#pragma once

#include <cstdint>
#include <numeric>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include "ravel/proof/block_height_shell_rank.hpp"

namespace ravel::proof {

struct RadialLinearFeatureFactorization {
    ShellState source;
    ShellState sign;
    ShellState gap_base;
    std::int64_t minimum_magnitude = 0;
    std::int64_t modulus = 0;
    std::int64_t radial_quotient = 0;
    std::int64_t radial_residue = 0;

    std::vector<std::int64_t> forcing_source;
    std::vector<std::int64_t> forcing_base_residue;
    std::vector<std::int64_t> forcing_sign;
    std::vector<std::int64_t> moments_source;
    std::vector<std::int64_t> moments_base_residue;
    std::vector<std::int64_t> moments_sign;

    bool state_decomposition_replay = false;
    bool forcing_affine_replay = false;
    bool moments_affine_replay = false;
    bool normalized_cell_determined = false;
    std::string obstruction;
};

inline ShellState shell_sign_state(const ShellState& x) {
    ShellState s;
    s.reserve(x.size());
    for (const auto v : x) s.push_back(v < 0 ? -1 : v > 0 ? 1 : 0);
    return s;
}

inline ShellState shell_gap_base(const ShellState& x,
                                 const ShellState& sign,
                                 std::int64_t minimum) {
    if (x.size() != sign.size()) throw std::invalid_argument("dimension mismatch");
    ShellState base(x.size(), 0);
    for (std::size_t i = 0; i < x.size(); ++i)
        base[i] = x[i] - minimum * sign[i];
    return base;
}

inline ShellState add_scaled(const ShellState& a, const ShellState& b,
                             std::int64_t scale) {
    if (a.size() != b.size()) throw std::invalid_argument("dimension mismatch");
    ShellState out = a;
    for (std::size_t i = 0; i < out.size(); ++i) out[i] += scale * b[i];
    return out;
}

inline std::vector<std::int64_t> add_scaled_features(
    const std::vector<std::int64_t>& a,
    const std::vector<std::int64_t>& b,
    std::int64_t scale) {
    if (a.size() != b.size()) throw std::invalid_argument("dimension mismatch");
    auto out = a;
    for (std::size_t i = 0; i < out.size(); ++i) out[i] += scale * b[i];
    return out;
}

/**
 * Prove that every linear feature built from the block-forcing dual map or the
 * carry Krylov map factors through the existing radial phase plus one affine
 * radial quotient.  In particular, after quotienting radial translation by
 * n+1, the exact joint block/height cell contains no separator beyond
 * signs|gaps|min-residue.
 */
inline RadialLinearFeatureFactorization
    derive_radial_linear_feature_factorization(const ShellState& x) {
    RadialLinearFeatureFactorization cert;
    cert.source = x;
    if (x.size() < 2) {
        cert.obstruction = "dimension must be at least two";
        return cert;
    }

    cert.sign = shell_sign_state(x);
    cert.minimum_magnitude = shell_radius(x);
    for (const auto v : x)
        cert.minimum_magnitude = std::min(cert.minimum_magnitude,
                                         static_cast<std::int64_t>(std::llabs(v)));
    cert.modulus = static_cast<std::int64_t>(x.size() + 1);
    cert.radial_quotient = cert.minimum_magnitude / cert.modulus;
    cert.radial_residue = cert.minimum_magnitude % cert.modulus;
    cert.gap_base = shell_gap_base(x, cert.sign, cert.minimum_magnitude);

    const auto residue_base = add_scaled(
        cert.gap_base, cert.sign, cert.radial_residue);
    cert.state_decomposition_replay =
        add_scaled(residue_base, cert.sign,
                   cert.radial_quotient * cert.modulus) == x;

    cert.forcing_source = block_forcing_dual_coefficients(x);
    cert.forcing_base_residue = block_forcing_dual_coefficients(residue_base);
    cert.forcing_sign = block_forcing_dual_coefficients(cert.sign);
    cert.forcing_affine_replay =
        add_scaled_features(cert.forcing_base_residue, cert.forcing_sign,
                   cert.radial_quotient * cert.modulus) == cert.forcing_source;

    cert.moments_source = carry_krylov_moments(x);
    cert.moments_base_residue = carry_krylov_moments(residue_base);
    cert.moments_sign = carry_krylov_moments(cert.sign);
    cert.moments_affine_replay =
        add_scaled_features(cert.moments_base_residue, cert.moments_sign,
                   cert.radial_quotient * cert.modulus) == cert.moments_source;

    cert.normalized_cell_determined = cert.state_decomposition_replay &&
        cert.forcing_affine_replay && cert.moments_affine_replay;
    if (!cert.normalized_cell_determined)
        cert.obstruction = "linear radial factorization failed replay";
    return cert;
}

} // namespace ravel::proof
