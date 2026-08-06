#pragma once

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <functional>
#include <numeric>
#include <string>
#include <utility>
#include <vector>

#include "ravel/proof/block_height_shell_rank.hpp"
#include "ravel/proof/parametric_radial_rank.hpp"

namespace ravel::proof {

struct AffineInteger {
    std::int64_t intercept = 0;
    std::int64_t slope = 0;
};

inline std::int64_t stabilization_threshold(AffineInteger x) {
    if (x.slope == 0) return 0;
    return std::llabs(x.intercept) / std::llabs(x.slope) + 1;
}

inline std::vector<AffineInteger> affine_forcing_features(const AffineShellRay& ray) {
    const auto b = block_forcing_dual_coefficients(ray.base);
    const auto a = block_forcing_dual_coefficients(ray.direction);
    std::vector<AffineInteger> out(b.size());
    for (std::size_t i = 0; i < b.size(); ++i) out[i] = {b[i], a[i]};
    return out;
}

inline std::vector<AffineInteger> affine_moment_features(const AffineShellRay& ray) {
    const auto b = carry_krylov_moments(ray.base);
    const auto a = carry_krylov_moments(ray.direction);
    std::vector<AffineInteger> out(b.size());
    for (std::size_t i = 0; i < b.size(); ++i) out[i] = {b[i], a[i]};
    return out;
}

inline std::int64_t affine_gcd_period(const std::vector<AffineInteger>& xs) {
    std::int64_t determinant_gcd = 0;
    for (std::size_t i = 0; i < xs.size(); ++i)
        for (std::size_t j = i + 1; j < xs.size(); ++j) {
            const __int128 d = static_cast<__int128>(xs[i].slope) * xs[j].intercept -
                               static_cast<__int128>(xs[j].slope) * xs[i].intercept;
            if (d > std::numeric_limits<std::int64_t>::max() ||
                d < std::numeric_limits<std::int64_t>::min())
                throw std::overflow_error("affine gcd determinant overflow");
            determinant_gcd = std::gcd(determinant_gcd,
                static_cast<std::int64_t>(d < 0 ? -d : d));
        }
    return determinant_gcd;
}

struct QuasiQuadraticResidueCertificate {
    std::int64_t residue = 0;
    IntegerQuadratic polynomial;
    QuadraticPositivityCertificate positivity;
    bool replay = false;
    bool valid = false;
};

struct ParametricBlockHeightRankCertificate {
    AffineShellRay source;
    AffineShellRay target;
    std::int64_t threshold = 0;
    std::int64_t period = 0;
    std::vector<WideRank> finite_prefix_gains;
    std::vector<QuasiQuadraticResidueCertificate> residues;
    bool chamber_stable = false;
    bool eventual_feature_grammar = false;
    bool finite_prefix_positive = false;
    bool tail_positive = false;
    bool all_radii_closed = false;
    std::string obstruction;
};

inline void include_stabilization_threshold(std::int64_t& threshold,
                                             const std::vector<AffineInteger>& xs) {
    for (const auto& x : xs)
        threshold = std::max(threshold, stabilization_threshold(x));
}

/** Exact all-q rank transport for the block/height feature grammar.
 *
 * Along affine rays, coordinates, forcing coefficients, moments, and moment
 * differences are affine. After a computable threshold their signs and zero
 * sets are fixed, hence all absolute-value summaries are affine and all energy
 * summaries are quadratic. The forcing gcd is periodic modulo the gcd of the
 * affine 2x2 determinants, provided that determinant gcd is nonzero. Thus the
 * rank gain is one quadratic on each residue class of a finite period.
 */
template<class RankOperation>
inline ParametricBlockHeightRankCertificate derive_parametric_block_height_rank(
    AffineShellRay source, AffineShellRay target, RankOperation rank) {
    ParametricBlockHeightRankCertificate cert;
    cert.source = std::move(source);
    cert.target = std::move(target);
    if (cert.source.base.size() != cert.source.direction.size() ||
        cert.target.base.size() != cert.target.direction.size() ||
        cert.source.base.size() != cert.target.base.size()) {
        cert.obstruction = "affine shell ray dimension mismatch";
        return cert;
    }

    const auto source_force = affine_forcing_features(cert.source);
    const auto target_force = affine_forcing_features(cert.target);
    const auto source_moments = affine_moment_features(cert.source);
    const auto target_moments = affine_moment_features(cert.target);
    include_stabilization_threshold(cert.threshold, source_force);
    include_stabilization_threshold(cert.threshold, target_force);
    include_stabilization_threshold(cert.threshold, source_moments);
    include_stabilization_threshold(cert.threshold, target_moments);
    std::vector<AffineInteger> source_diffs, target_diffs;
    for (std::size_t i = 1; i < source_moments.size(); ++i)
        source_diffs.push_back({source_moments[i].intercept-source_moments[i-1].intercept,
                                source_moments[i].slope-source_moments[i-1].slope});
    for (std::size_t i = 1; i < target_moments.size(); ++i)
        target_diffs.push_back({target_moments[i].intercept-target_moments[i-1].intercept,
                                target_moments[i].slope-target_moments[i-1].slope});
    include_stabilization_threshold(cert.threshold, source_diffs);
    include_stabilization_threshold(cert.threshold, target_diffs);
    // Coordinate signs select the chamber coefficients.
    std::vector<AffineInteger> source_coordinates, target_coordinates;
    for (std::size_t i = 0; i < cert.source.base.size(); ++i) {
        source_coordinates.push_back({cert.source.base[i], cert.source.direction[i]});
        target_coordinates.push_back({cert.target.base[i], cert.target.direction[i]});
    }
    include_stabilization_threshold(cert.threshold, source_coordinates);
    include_stabilization_threshold(cert.threshold, target_coordinates);
    cert.chamber_stable = true;

    const auto source_period = affine_gcd_period(source_force);
    const auto target_period = affine_gcd_period(target_force);
    if (source_period == 0 || target_period == 0) {
        cert.obstruction = "forcing gcd has no bounded affine determinant period";
        return cert;
    }
    cert.period = std::lcm(source_period, target_period);
    if (cert.period <= 0 || cert.period > 100000) {
        cert.obstruction = "forcing gcd period is invalid or impractically large";
        return cert;
    }
    cert.eventual_feature_grammar = true;

    auto gain_at = [&](std::int64_t q) -> WideRank {
        return rank(eval_affine_shell_ray(cert.target, q)) -
               rank(eval_affine_shell_ray(cert.source, q));
    };
    cert.finite_prefix_positive = true;
    cert.finite_prefix_gains.reserve(cert.threshold);
    for (std::int64_t q = 0; q < cert.threshold; ++q) {
        const auto gain = gain_at(q);
        cert.finite_prefix_gains.push_back(gain);
        cert.finite_prefix_positive &= gain > 0;
    }
    if (!cert.finite_prefix_positive) {
        cert.obstruction = "finite preperiod contains a nonpositive gain";
        return cert;
    }

    cert.tail_positive = true;
    cert.residues.reserve(cert.period);
    for (std::int64_t r = 0; r < cert.period; ++r) {
        const auto q0 = cert.threshold + r;
        const auto y0 = gain_at(q0);
        const auto y1 = gain_at(q0 + cert.period);
        const auto y2 = gain_at(q0 + 2 * cert.period);
        const auto [poly, integral] = interpolate_integer_quadratic(y0,y1,y2);
        QuasiQuadraticResidueCertificate rc;
        rc.residue = r;
        rc.polynomial = poly;
        if (integral) {
            rc.replay = eval_integer_quadratic(poly,3) == gain_at(q0+3*cert.period) &&
                        eval_integer_quadratic(poly,4) == gain_at(q0+4*cert.period);
            rc.positivity = certify_positive_on_nonnegative_integers(poly);
            rc.valid = rc.replay && rc.positivity.strictly_positive_on_nonnegative_integers;
        }
        cert.tail_positive &= rc.valid;
        cert.residues.push_back(std::move(rc));
    }
    cert.all_radii_closed = cert.chamber_stable && cert.eventual_feature_grammar &&
        cert.finite_prefix_positive && cert.tail_positive;
    if (!cert.all_radii_closed && cert.obstruction.empty())
        cert.obstruction = "one or more quasi-quadratic residue classes failed";
    return cert;
}

} // namespace ravel::proof
