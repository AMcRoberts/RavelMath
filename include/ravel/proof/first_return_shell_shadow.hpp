#pragma once

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <map>
#include <optional>
#include <set>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

#include "ravel/proof/covering_translation_tube.hpp"
#include "ravel/proof/uniform_radius_one_synthesis.hpp"

namespace ravel::proof {

struct FirstReturnShellShadowCertificate {
    std::size_t dimension = 0;
    std::int64_t source_bound = 0;
    std::int64_t shadow_bound = 0;
    std::vector<IntegerState> source_path;
    std::vector<std::int64_t> source_digits;
    std::vector<RadiusOneState> controller_path;
    std::vector<std::int64_t> controller_defects;
    std::vector<std::int64_t> shadow_digits;
    std::vector<IntegerState> shadow_path;
    bool source_replay = false;
    bool controller_replay = false;
    bool shadow_replay = false;
    bool shadow_first_return = false;
    bool exact_recomposition = false;
    bool derived = false;
    std::string obstruction;
};

namespace shell_shadow_detail {

inline bool valid_shadow_geometry(const IntegerState& source,
                                  const RadiusOneState& controller,
                                  std::int64_t shadow_bound,
                                  bool endpoint) {
    if (source.size() != controller.size()) return false;
    IntegerState shadow(source.size(), 0);
    for (std::size_t i = 0; i < source.size(); ++i)
        shadow[i] = source[i] - controller[i];
    return endpoint ? sup_norm(shadow) == shadow_bound
                    : sup_norm(shadow) < shadow_bound;
}

struct Parent {
    RadiusOneState state;
    std::int64_t defect = 0;
    std::int64_t shadow_digit = 0;
};

} // namespace shell_shadow_detail

/** Derive a one-shell inward shadow of an exact first-return word.
 *
 * Search occurs only in the radius-one controller plant.  At layer k a
 * controller state t_k is admissible exactly when x_k-t_k lies on the shadow
 * shell at the endpoints and strictly inside it at intermediate layers.
 * Transitions satisfy
 *
 *   t_{k+1} = A t_k + delta_k e_last,
 *   e_k = d_k - delta_k in {-1,0,1}.
 *
 * Therefore r_k=x_k-t_k follows the same n-bonacci system with digit e_k.
 * This is an executable bounded shadow operation, analogous to the local
 * n->n-1 shadow paths used in predicted-core induction.
 */
inline FirstReturnShellShadowCertificate derive_first_return_shell_shadow(
    std::int64_t bound,
    std::vector<IntegerState> path,
    std::vector<std::int64_t> digits) {
    FirstReturnShellShadowCertificate c;
    c.source_bound = bound;
    c.shadow_bound = bound - 1;
    c.source_path = std::move(path);
    c.source_digits = std::move(digits);
    if (bound < 2 || c.source_path.empty() ||
        c.source_path.size() != c.source_digits.size() + 1) {
        c.obstruction = "invalid source first-return word";
        return c;
    }
    c.dimension = c.source_path.front().size();
    if (c.dimension == 0) {
        c.obstruction = "zero-dimensional word";
        return c;
    }
    c.source_replay = true;
    for (const auto& x : c.source_path)
        c.source_replay &= x.size() == c.dimension;
    c.source_replay &= sup_norm(c.source_path.front()) == bound &&
                       sup_norm(c.source_path.back()) == bound;
    for (std::size_t k = 1; k + 1 < c.source_path.size(); ++k)
        c.source_replay &= sup_norm(c.source_path[k]) < bound;
    for (std::size_t k = 0; k < c.source_digits.size(); ++k) {
        c.source_replay &= -1 <= c.source_digits[k] && c.source_digits[k] <= 1;
        c.source_replay &= nbonacci_step(c.source_path[k], c.source_digits[k]) ==
                           c.source_path[k + 1];
    }
    if (!c.source_replay) {
        c.obstruction = "source word failed exact first-return replay";
        return c;
    }

    const auto controller_states = enumerate_radius_one_states(c.dimension);
    std::vector<std::set<RadiusOneState>> admissible(c.source_path.size());
    for (std::size_t k = 0; k < c.source_path.size(); ++k) {
        const bool endpoint = k == 0 || k + 1 == c.source_path.size();
        for (const auto& t : controller_states)
            if (shell_shadow_detail::valid_shadow_geometry(
                    c.source_path[k], t, c.shadow_bound, endpoint))
                admissible[k].insert(t);
        if (admissible[k].empty()) {
            c.obstruction = "a shadow layer has no geometrically admissible controller";
            return c;
        }
    }

    using ParentMap = std::map<RadiusOneState, shell_shadow_detail::Parent>;
    std::vector<ParentMap> parents(c.source_path.size());
    std::set<RadiusOneState> current = admissible.front();
    for (const auto& t : current) parents[0].emplace(t, shell_shadow_detail::Parent{});

    for (std::size_t k = 0; k < c.source_digits.size(); ++k) {
        std::set<RadiusOneState> next;
        for (const auto& t : current) {
            const auto homogeneous = translation_step(t, 0);
            for (const auto shadow_digit : {-1LL, 0LL, 1LL}) {
                const auto defect = c.source_digits[k] - shadow_digit;
                auto u = homogeneous;
                u.back() += defect;
                if (!is_radius_one(u) || !admissible[k + 1].contains(u)) continue;
                if (!parents[k + 1].contains(u))
                    parents[k + 1].emplace(
                        u, shell_shadow_detail::Parent{t, defect, shadow_digit});
                next.insert(std::move(u));
            }
        }
        current = std::move(next);
        if (current.empty()) {
            c.obstruction = "radius-one shadow relation became empty";
            return c;
        }
    }

    auto terminal = *current.begin();
    c.controller_path.resize(c.source_path.size());
    c.controller_defects.resize(c.source_digits.size());
    c.shadow_digits.resize(c.source_digits.size());
    c.controller_path.back() = terminal;
    for (std::size_t k = c.source_digits.size(); k > 0; --k) {
        const auto& p = parents[k].at(c.controller_path[k]);
        c.controller_path[k - 1] = p.state;
        c.controller_defects[k - 1] = p.defect;
        c.shadow_digits[k - 1] = p.shadow_digit;
    }

    c.shadow_path.reserve(c.source_path.size());
    for (std::size_t k = 0; k < c.source_path.size(); ++k) {
        IntegerState r(c.dimension, 0);
        for (std::size_t i = 0; i < c.dimension; ++i)
            r[i] = c.source_path[k][i] - c.controller_path[k][i];
        c.shadow_path.push_back(std::move(r));
    }

    c.controller_replay = true;
    c.shadow_replay = true;
    for (std::size_t k = 0; k < c.source_digits.size(); ++k) {
        c.controller_replay &=
            translation_step(c.controller_path[k], c.controller_defects[k]) ==
            c.controller_path[k + 1];
        c.controller_replay &=
            c.source_digits[k] - c.controller_defects[k] == c.shadow_digits[k];
        c.shadow_replay &=
            nbonacci_step(c.shadow_path[k], c.shadow_digits[k]) ==
            c.shadow_path[k + 1];
    }
    c.shadow_first_return = c.shadow_replay &&
        sup_norm(c.shadow_path.front()) == c.shadow_bound &&
        sup_norm(c.shadow_path.back()) == c.shadow_bound;
    for (std::size_t k = 1; k + 1 < c.shadow_path.size(); ++k)
        c.shadow_first_return &= sup_norm(c.shadow_path[k]) < c.shadow_bound;
    c.exact_recomposition = true;
    for (std::size_t k = 0; k < c.source_path.size(); ++k)
        for (std::size_t i = 0; i < c.dimension; ++i)
            c.exact_recomposition &=
                c.shadow_path[k][i] + c.controller_path[k][i] ==
                c.source_path[k][i];
    c.derived = c.source_replay && c.controller_replay && c.shadow_replay &&
                c.shadow_first_return && c.exact_recomposition;
    if (!c.derived) c.obstruction = "shadow certificate failed replay";
    return c;
}

} // namespace ravel::proof
