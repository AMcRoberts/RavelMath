#pragma once

#include <cstddef>
#include <cstdint>
#include <functional>
#include <limits>
#include <string>
#include <utility>

#include "ravel/proof/defect_corrected_shell_rank.hpp"

namespace ravel::proof {

using WideRank = __int128_t;

struct AffineShellRay {
    ShellState base;
    ShellState direction;
};

inline ShellState eval_affine_shell_ray(const AffineShellRay& ray,
                                        std::int64_t q) {
    if (ray.base.size() != ray.direction.size() || q < 0)
        throw std::invalid_argument("invalid affine shell ray");
    ShellState out = ray.base;
    for (std::size_t i = 0; i < out.size(); ++i)
        out[i] += q * ray.direction[i];
    return out;
}

struct IntegerQuadratic {
    WideRank constant = 0;
    WideRank linear = 0;
    WideRank quadratic = 0;
};

inline WideRank eval_integer_quadratic(const IntegerQuadratic& p,
                                       std::int64_t q) {
    const auto wq = static_cast<WideRank>(q);
    return p.constant + p.linear * wq + p.quadratic * wq * wq;
}

/** Recover the unique degree-at-most-two polynomial from values at 0,1,2.
 * The second finite difference must be even for an integer-coefficient
 * polynomial in the monomial basis. */
inline std::pair<IntegerQuadratic, bool> interpolate_integer_quadratic(
    WideRank y0, WideRank y1, WideRank y2) {
    const auto second = y2 - 2 * y1 + y0;
    if (second % 2 != 0) return {{}, false};
    IntegerQuadratic p;
    p.constant = y0;
    p.quadratic = second / 2;
    p.linear = y1 - y0 - p.quadratic;
    return {p, true};
}

struct QuadraticPositivityCertificate {
    IntegerQuadratic polynomial;
    std::int64_t checked_through = 0;
    bool nonnegative_leading = false;
    bool exact_minimum_checked = false;
    bool strictly_positive_on_nonnegative_integers = false;
    std::string obstruction;
};

/** Decide strict positivity of an integer quadratic on q>=0 exactly.
 * For a>0 the minimum over nonnegative integers occurs at one of the two
 * integers nearest -b/(2a), together with q=0. Linear and constant cases are
 * handled separately. */
inline QuadraticPositivityCertificate certify_positive_on_nonnegative_integers(
    IntegerQuadratic p) {
    QuadraticPositivityCertificate cert;
    cert.polynomial = p;
    cert.nonnegative_leading = p.quadratic >= 0;
    if (p.quadratic < 0) {
        cert.obstruction = "negative quadratic coefficient";
        return cert;
    }
    if (p.quadratic == 0) {
        if (p.linear < 0) {
            cert.obstruction = "negative linear tail";
            return cert;
        }
        cert.exact_minimum_checked = true;
        cert.strictly_positive_on_nonnegative_integers = p.constant > 0;
        if (!cert.strictly_positive_on_nonnegative_integers)
            cert.obstruction = "nonpositive value at q=0";
        return cert;
    }

    // Avoid floating point. Candidate floor of -b/(2a), clamped to q>=0.
    std::int64_t q0 = 0;
    const auto denom = 2 * p.quadratic;
    if (p.linear < 0) {
        const auto numerator = -p.linear;
        const auto floor_wide = numerator / denom;
        q0 = floor_wide > static_cast<WideRank>(std::numeric_limits<std::int64_t>::max())
            ? std::numeric_limits<std::int64_t>::max()
            : static_cast<std::int64_t>(floor_wide);
    }
    WideRank minimum = eval_integer_quadratic(p, 0);
    cert.checked_through = q0;
    for (const auto q : {q0, q0 == std::numeric_limits<std::int64_t>::max() ? q0 : q0 + 1})
        minimum = std::min(minimum, eval_integer_quadratic(p, q));
    cert.exact_minimum_checked = true;
    cert.strictly_positive_on_nonnegative_integers = minimum > 0;
    if (!cert.strictly_positive_on_nonnegative_integers)
        cert.obstruction = "quadratic has a nonpositive nonnegative-integer value";
    return cert;
}

struct ParametricRadialRankCertificate {
    AffineShellRay source;
    AffineShellRay target;
    IntegerQuadratic gain;
    QuadraticPositivityCertificate positivity;
    bool dimensions_match = false;
    bool quadratic_replay = false;
    bool all_radii_closed = false;
    std::string obstruction;
};

/** Certify a strict rank inequality for every q>=0 along a pair of affine
 * shell rays. The caller supplies an executable rank operation; its values at
 * q=0,1,2 derive the quadratic, and q=3,4 replay that the rank expression is
 * genuinely degree at most two on the announced rays. */
template<class RankOperation>
inline ParametricRadialRankCertificate derive_parametric_radial_rank(
    AffineShellRay source, AffineShellRay target, RankOperation rank) {
    ParametricRadialRankCertificate cert;
    cert.source = std::move(source);
    cert.target = std::move(target);
    cert.dimensions_match = cert.source.base.size() == cert.source.direction.size() &&
        cert.target.base.size() == cert.target.direction.size() &&
        cert.source.base.size() == cert.target.base.size();
    if (!cert.dimensions_match) {
        cert.obstruction = "affine shell ray dimension mismatch";
        return cert;
    }
    auto gain_at = [&](std::int64_t q) -> WideRank {
        return rank(eval_affine_shell_ray(cert.target, q)) -
               rank(eval_affine_shell_ray(cert.source, q));
    };
    const auto [p, integral] = interpolate_integer_quadratic(
        gain_at(0), gain_at(1), gain_at(2));
    if (!integral) {
        cert.obstruction = "rank gain is not an integer quadratic";
        return cert;
    }
    cert.gain = p;
    cert.quadratic_replay =
        eval_integer_quadratic(p, 3) == gain_at(3) &&
        eval_integer_quadratic(p, 4) == gain_at(4);
    if (!cert.quadratic_replay) {
        cert.obstruction = "rank gain failed degree-two replay";
        return cert;
    }
    cert.positivity = certify_positive_on_nonnegative_integers(p);
    cert.all_radii_closed = cert.quadratic_replay &&
        cert.positivity.strictly_positive_on_nonnegative_integers;
    if (!cert.all_radii_closed) cert.obstruction = cert.positivity.obstruction;
    return cert;
}

} // namespace ravel::proof
