#pragma once

#include <algorithm>
#include <cstdint>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include "ravel/proof/covering_translation_tube.hpp"

namespace ravel::proof {

struct RadialFirstReturnSchemaCertificate {
    std::string certificate_id;
    std::int64_t base_bound = 0;
    std::int64_t radial_increment = 0;
    std::vector<IntegerState> base_path;
    std::vector<std::int64_t> digits;
    IntegerState source_direction;
    std::vector<IntegerState> direction_windows;
    IntegerState target_direction;
    bool base_replay = false;
    bool base_first_return = false;
    bool homogeneous_transport = false;
    bool direction_box = false;
    bool source_face_aligned = false;
    bool target_face_aligned = false;
    bool all_radius_geometry = false;
    bool all_radius_recurrence = false;
    bool proved = false;
    std::string obstruction;
};

inline bool same_signed_extreme(std::int64_t b, std::int64_t d,
                                std::int64_t bound, std::int64_t increment) {
    return std::llabs(b) == bound && std::llabs(d) == increment &&
           ((b > 0 && d > 0) || (b < 0 && d < 0));
}

/** Uniform radial recurrence for a first-return word.
 *
 * The base path is a first-return path at radius M.  A single homogeneous
 * controller direction t_0 is propagated through the same digit word by the
 * linear n-bonacci step, producing t_k.  If every t_k lies in the radius-R
 * controller box, the endpoint directions meet active source/target faces,
 * and the base intermediates are strictly inside radius M, then
 *
 *   x_k(q) = x_k + q t_k
 *
 * is a first-return path at radius M+qR for every q>=0.  This is the shell
 * analogue of the bounded shadow/minor recurrence used by the completed
 * universal-n proofs: one local schema generates the whole parameter ray.
 */
inline RadialFirstReturnSchemaCertificate derive_radial_first_return_schema(
    std::string certificate_id,
    std::int64_t base_bound,
    std::vector<IntegerState> base_path,
    std::vector<std::int64_t> digits,
    IntegerState source_direction) {
    RadialFirstReturnSchemaCertificate c;
    c.certificate_id = std::move(certificate_id);
    c.base_bound = base_bound;
    c.base_path = std::move(base_path);
    c.digits = std::move(digits);
    c.source_direction = std::move(source_direction);
    if (base_bound <= 0 || c.base_path.empty() ||
        c.base_path.size() != c.digits.size() + 1) {
        c.obstruction = "invalid base first-return word";
        return c;
    }
    const auto n = c.base_path.front().size();
    if (n == 0 || c.source_direction.size() != n) {
        c.obstruction = "dimension mismatch";
        return c;
    }
    for (const auto& x : c.base_path)
        if (x.size() != n) {
            c.obstruction = "ragged base path";
            return c;
        }

    c.radial_increment = sup_norm(c.source_direction);
    if (c.radial_increment <= 0) {
        c.obstruction = "zero radial direction";
        return c;
    }

    c.base_replay = true;
    for (std::size_t k = 0; k < c.digits.size(); ++k) {
        c.base_replay &= -1 <= c.digits[k] && c.digits[k] <= 1;
        c.base_replay &= nbonacci_step(c.base_path[k], c.digits[k]) ==
                         c.base_path[k + 1];
    }
    c.base_first_return = c.base_replay &&
        sup_norm(c.base_path.front()) == base_bound &&
        sup_norm(c.base_path.back()) == base_bound;
    for (std::size_t k = 1; k + 1 < c.base_path.size(); ++k)
        c.base_first_return &= sup_norm(c.base_path[k]) < base_bound;
    if (!c.base_first_return) {
        c.obstruction = "base word is not a first return";
        return c;
    }

    c.direction_windows.push_back(c.source_direction);
    for (std::size_t k = 0; k < c.digits.size(); ++k)
        c.direction_windows.push_back(
            nbonacci_step(c.direction_windows.back(), 0));
    c.target_direction = c.direction_windows.back();
    c.homogeneous_transport = true;
    for (std::size_t k = 0; k < c.digits.size(); ++k)
        c.homogeneous_transport &=
            nbonacci_step(c.direction_windows[k], 0) ==
            c.direction_windows[k + 1];

    c.direction_box = true;
    for (const auto& t : c.direction_windows)
        c.direction_box &= sup_norm(t) <= c.radial_increment;

    c.source_face_aligned = false;
    c.target_face_aligned = false;
    for (std::size_t i = 0; i < n; ++i) {
        c.source_face_aligned |= same_signed_extreme(
            c.base_path.front()[i], c.source_direction[i],
            base_bound, c.radial_increment);
        c.target_face_aligned |= same_signed_extreme(
            c.base_path.back()[i], c.target_direction[i],
            base_bound, c.radial_increment);
    }

    // Triangle inequality proves every intermediate remains strictly inside:
    // |x_i + q t_i| <= (M-1)+qR < M+qR.
    c.all_radius_geometry = c.direction_box && c.source_face_aligned &&
                            c.target_face_aligned;
    c.all_radius_recurrence = c.base_replay && c.homogeneous_transport;
    c.proved = c.all_radius_geometry && c.all_radius_recurrence;
    if (!c.proved) c.obstruction = "radial schema conditions are incomplete";
    return c;
}

struct RadialFirstReturnReductionCertificate {
    std::int64_t quotient = 0;
    std::int64_t outer_bound = 0;
    std::vector<IntegerState> lifted_path;
    std::vector<IntegerState> reduced_path;
    bool lifted_replay = false;
    bool lifted_first_return = false;
    bool reduction_exact = false;
    bool proved = false;
};

inline RadialFirstReturnReductionCertificate replay_radial_first_return_schema(
    const RadialFirstReturnSchemaCertificate& schema, std::int64_t q) {
    RadialFirstReturnReductionCertificate c;
    c.quotient = q;
    if (!schema.proved || q < 0) return c;
    c.outer_bound = schema.base_bound + q * schema.radial_increment;
    c.lifted_path.reserve(schema.base_path.size());
    c.reduced_path.reserve(schema.base_path.size());
    for (std::size_t k = 0; k < schema.base_path.size(); ++k) {
        IntegerState y(schema.base_path[k].size(), 0);
        IntegerState x(schema.base_path[k].size(), 0);
        for (std::size_t i = 0; i < y.size(); ++i) {
            y[i] = schema.base_path[k][i] + q * schema.direction_windows[k][i];
            x[i] = y[i] - q * schema.direction_windows[k][i];
        }
        c.lifted_path.push_back(std::move(y));
        c.reduced_path.push_back(std::move(x));
    }
    c.lifted_replay = true;
    for (std::size_t k = 0; k < schema.digits.size(); ++k)
        c.lifted_replay &= nbonacci_step(c.lifted_path[k], schema.digits[k]) ==
                           c.lifted_path[k + 1];
    c.lifted_first_return = c.lifted_replay &&
        sup_norm(c.lifted_path.front()) == c.outer_bound &&
        sup_norm(c.lifted_path.back()) == c.outer_bound;
    for (std::size_t k = 1; k + 1 < c.lifted_path.size(); ++k)
        c.lifted_first_return &= sup_norm(c.lifted_path[k]) < c.outer_bound;
    c.reduction_exact = c.reduced_path == schema.base_path;
    c.proved = c.lifted_first_return && c.reduction_exact;
    return c;
}

} // namespace ravel::proof
