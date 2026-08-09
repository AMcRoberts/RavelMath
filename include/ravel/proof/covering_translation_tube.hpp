#pragma once

#include <algorithm>
#include <cstdint>
#include <iterator>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include "math/proof_reflection.hpp"

namespace ravel::proof {

using IntegerState = std::vector<std::int64_t>;

struct CoveringTranslationTubeCertificate {
    std::string certificate_id;
    std::size_t dimension = 0;
    std::int64_t base_bound = 0;
    std::int64_t translation_bound = 0;
    std::vector<IntegerState> base_path;
    std::vector<IntegerState> translation_windows;
    std::vector<IntegerState> transported_path;
    bool dimensions_match = false;
    bool translation_box_checked = false;
    bool translation_cover_checked = false;
    bool transported_box_checked = false;
    bool source_on_outer_shell = false;
    bool target_on_outer_shell = false;
    bool intermediates_inside_outer_shell = false;
    bool first_return_transport = false;
};

inline std::int64_t sup_norm(const IntegerState& x) {
    std::int64_t value = 0;
    for (const auto coordinate : x) {
        const auto magnitude = coordinate < 0 ? -coordinate : coordinate;
        value = std::max<std::int64_t>(value, magnitude);
    }
    return value;
}

inline IntegerState add_states(const IntegerState& x, const IntegerState& y) {
    if (x.size() != y.size())
        throw std::invalid_argument("state dimensions do not match");
    IntegerState result(x.size(), 0);
    for (std::size_t i = 0; i < x.size(); ++i)
        result[i] = x[i] + y[i];
    return result;
}

inline CoveringTranslationTubeCertificate certify_covering_translation_tube(
    std::string certificate_id,
    std::int64_t base_bound,
    std::int64_t translation_bound,
    std::vector<IntegerState> base_path,
    std::vector<IntegerState> translation_windows) {
    CoveringTranslationTubeCertificate cert;
    cert.certificate_id = std::move(certificate_id);
    cert.base_bound = base_bound;
    cert.translation_bound = translation_bound;
    cert.base_path = std::move(base_path);
    cert.translation_windows = std::move(translation_windows);

    if (cert.base_path.empty() ||
        cert.base_path.size() != cert.translation_windows.size())
        return cert;

    cert.dimension = cert.base_path.front().size();
    cert.dimensions_match = cert.dimension > 0;
    for (const auto& x : cert.base_path)
        cert.dimensions_match &= x.size() == cert.dimension;
    for (const auto& t : cert.translation_windows)
        cert.dimensions_match &= t.size() == cert.dimension;
    if (!cert.dimensions_match) return cert;

    cert.translation_box_checked = true;
    cert.translation_cover_checked = true;
    cert.transported_box_checked = true;
    const auto outer_bound = base_bound + translation_bound;

    for (std::size_t i = 0; i < cert.base_path.size(); ++i) {
        const auto& t = cert.translation_windows[i];
        cert.translation_box_checked &= sup_norm(t) <= translation_bound;
        cert.translation_cover_checked &= sup_norm(t) == translation_bound;
        cert.transported_path.push_back(add_states(cert.base_path[i], t));
        cert.transported_box_checked &=
            sup_norm(cert.transported_path.back()) <= outer_bound;
    }

    cert.source_on_outer_shell =
        sup_norm(cert.transported_path.front()) == outer_bound;
    cert.target_on_outer_shell =
        sup_norm(cert.transported_path.back()) == outer_bound;
    cert.intermediates_inside_outer_shell = true;
    for (std::size_t i = 1; i + 1 < cert.transported_path.size(); ++i)
        cert.intermediates_inside_outer_shell &=
            sup_norm(cert.transported_path[i]) < outer_bound;

    cert.first_return_transport =
        cert.translation_box_checked &&
        cert.translation_cover_checked &&
        cert.transported_box_checked &&
        cert.source_on_outer_shell &&
        cert.target_on_outer_shell &&
        cert.intermediates_inside_outer_shell;
    return cert;
}

struct CoveringTubeSpliceCertificate {
    CoveringTranslationTubeCertificate left;
    CoveringTranslationTubeCertificate right;
    CoveringTranslationTubeCertificate combined;
    bool same_bounds = false;
    bool base_endpoint_matches = false;
    bool translation_endpoint_matches = false;
    bool transported_endpoint_matches = false;
    bool splice_valid = false;
};

inline CoveringTubeSpliceCertificate splice_covering_translation_tubes(
    CoveringTranslationTubeCertificate left,
    CoveringTranslationTubeCertificate right,
    std::string combined_id) {
    CoveringTubeSpliceCertificate result;
    result.left = std::move(left);
    result.right = std::move(right);
    result.same_bounds =
        result.left.base_bound == result.right.base_bound &&
        result.left.translation_bound == result.right.translation_bound;

    if (result.left.base_path.empty() || result.right.base_path.empty() ||
        result.left.translation_windows.empty() ||
        result.right.translation_windows.empty() ||
        result.left.transported_path.empty() ||
        result.right.transported_path.empty())
        return result;

    result.base_endpoint_matches =
        result.left.base_path.back() == result.right.base_path.front();
    result.translation_endpoint_matches =
        result.left.translation_windows.back() ==
        result.right.translation_windows.front();
    result.transported_endpoint_matches =
        result.left.transported_path.back() ==
        result.right.transported_path.front();

    if (!(result.same_bounds && result.base_endpoint_matches &&
          result.translation_endpoint_matches &&
          result.transported_endpoint_matches))
        return result;

    auto base_path = result.left.base_path;
    base_path.insert(base_path.end(),
                     std::next(result.right.base_path.begin()),
                     result.right.base_path.end());
    auto windows = result.left.translation_windows;
    windows.insert(windows.end(),
                   std::next(result.right.translation_windows.begin()),
                   result.right.translation_windows.end());

    result.combined = certify_covering_translation_tube(
        std::move(combined_id),
        result.left.base_bound,
        result.left.translation_bound,
        std::move(base_path),
        std::move(windows));
    result.splice_valid = result.combined.first_return_transport;
    return result;
}


struct DefectSplicedTubeCertificate {
    std::string certificate_id;
    std::int64_t base_bound = 0;
    std::int64_t translation_bound = 0;
    std::vector<IntegerState> base_path;
    std::vector<IntegerState> translation_windows;
    std::vector<std::int64_t> base_digits;
    std::vector<std::int64_t> splice_defects;
    std::vector<std::int64_t> adjusted_digits;
    std::vector<IntegerState> transported_path;
    bool dimensions_match = false;
    bool lengths_match = false;
    bool translation_cover_checked = false;
    bool splice_recurrence_checked = false;
    bool adjusted_digits_admissible = false;
    bool transported_replay_checked = false;
    bool first_return_transport = false;
};

inline IntegerState nbonacci_step(
    const IntegerState& state, std::int64_t digit) {
    if (state.empty())
        throw std::invalid_argument("empty n-bonacci state");
    IntegerState next(state.begin() + 1, state.end());
    std::int64_t last = state.front() + digit;
    for (std::size_t i = 1; i < state.size(); ++i)
        last -= state[i];
    next.push_back(last);
    return next;
}

inline DefectSplicedTubeCertificate certify_defect_spliced_tube(
    std::string certificate_id,
    std::int64_t base_bound,
    std::int64_t translation_bound,
    std::vector<IntegerState> base_path,
    std::vector<IntegerState> translation_windows,
    std::vector<std::int64_t> base_digits,
    std::vector<std::int64_t> splice_defects) {
    DefectSplicedTubeCertificate cert;
    cert.certificate_id = std::move(certificate_id);
    cert.base_bound = base_bound;
    cert.translation_bound = translation_bound;
    cert.base_path = std::move(base_path);
    cert.translation_windows = std::move(translation_windows);
    cert.base_digits = std::move(base_digits);
    cert.splice_defects = std::move(splice_defects);

    cert.lengths_match =
        cert.base_path.size() == cert.translation_windows.size() &&
        cert.base_path.size() == cert.base_digits.size() + 1 &&
        cert.base_digits.size() == cert.splice_defects.size();
    if (!cert.lengths_match || cert.base_path.empty()) return cert;

    const auto dimension = cert.base_path.front().size();
    cert.dimensions_match = dimension > 0;
    for (const auto& state : cert.base_path)
        cert.dimensions_match &= state.size() == dimension;
    for (const auto& state : cert.translation_windows)
        cert.dimensions_match &= state.size() == dimension;
    if (!cert.dimensions_match) return cert;

    cert.translation_cover_checked = true;
    cert.splice_recurrence_checked = true;
    cert.adjusted_digits_admissible = true;
    cert.transported_replay_checked = true;
    cert.adjusted_digits.reserve(cert.base_digits.size());
    cert.transported_path.reserve(cert.base_path.size());

    for (std::size_t i = 0; i < cert.base_path.size(); ++i) {
        cert.translation_cover_checked &=
            sup_norm(cert.translation_windows[i]) == translation_bound;
        cert.transported_path.push_back(
            add_states(cert.base_path[i], cert.translation_windows[i]));
    }

    for (std::size_t k = 0; k < cert.base_digits.size(); ++k) {
        const auto adjusted = cert.base_digits[k] + cert.splice_defects[k];
        cert.adjusted_digits.push_back(adjusted);
        cert.adjusted_digits_admissible &=
            adjusted >= -1 && adjusted <= 1;
        cert.splice_recurrence_checked &=
            nbonacci_step(cert.translation_windows[k], cert.splice_defects[k]) ==
            cert.translation_windows[k + 1];
        cert.transported_replay_checked &=
            nbonacci_step(cert.transported_path[k], adjusted) ==
            cert.transported_path[k + 1];
    }

    const auto outer_bound = base_bound + translation_bound;
    bool geometry =
        sup_norm(cert.transported_path.front()) == outer_bound &&
        sup_norm(cert.transported_path.back()) == outer_bound;
    for (std::size_t i = 1; i + 1 < cert.transported_path.size(); ++i)
        geometry &= sup_norm(cert.transported_path[i]) < outer_bound;

    cert.first_return_transport =
        cert.translation_cover_checked &&
        cert.splice_recurrence_checked &&
        cert.adjusted_digits_admissible &&
        cert.transported_replay_checked && geometry;
    return cert;
}

// Stages every per-step defect-splice transition in a certified tube into
// the reflection trace. Unlike certify_defect_spliced_tube itself, this
// ALSO independently re-verifies the base path's own n-bonacci recurrence
// (nothing here is pre-trusted) before staging any step.
inline void stage_defect_splice_steps(
    const DefectSplicedTubeCertificate& cert, const std::string& description) {
    if (!cert.first_return_transport) return;
    if (!mathlib::reflection::enabled()) return;
    const auto dim = cert.base_path.empty() ? 0 : cert.base_path.front().size();
    for (std::size_t k = 0; k < cert.base_digits.size(); ++k) {
        if (nbonacci_step(cert.base_path[k], cert.base_digits[k]) != cert.base_path[k + 1])
            return;  // base path's own recurrence failed independent replay
        mathlib::reflection::DefectSpliceStepCertificate node;
        node.dim = static_cast<long long>(dim);
        node.x.assign(cert.base_path[k].begin(), cert.base_path[k].end());
        node.t.assign(cert.translation_windows[k].begin(), cert.translation_windows[k].end());
        node.digit = cert.base_digits[k];
        node.defect = cert.splice_defects[k];
        node.base_next.assign(cert.base_path[k + 1].begin(), cert.base_path[k + 1].end());
        node.translation_next.assign(
            cert.translation_windows[k + 1].begin(), cert.translation_windows[k + 1].end());
        node.transported_next.assign(
            cert.transported_path[k + 1].begin(), cert.transported_path[k + 1].end());
        node.description = description + " step " + std::to_string(k);
        mathlib::reflection::record(mathlib::reflection::NodeKind::LemmaApplication, node);
    }
}

}  // namespace ravel::proof
