#pragma once

#include <algorithm>
#include <cstdint>
#include <map>
#include <optional>
#include <set>
#include <stdexcept>
#include <tuple>
#include <utility>
#include <vector>

namespace ravel::proof {

using RadiusOneState = std::vector<std::int64_t>;

struct BoundaryFace {
    std::size_t coordinate = 0;
    std::int64_t sign = 0;
};

struct RadiusOneSynthesisCertificate {
    std::size_t dimension = 0;
    std::vector<std::int64_t> base_digits;
    std::vector<BoundaryFace> source_faces;
    std::vector<BoundaryFace> target_faces;
    std::vector<RadiusOneState> translation_windows;
    std::vector<std::int64_t> splice_defects;
    std::vector<std::int64_t> adjusted_digits;
    bool source_aligned = false;
    bool target_aligned = false;
    bool radius_one = false;
    bool recurrence_checked = false;
    bool adjusted_digits_admissible = false;
    bool closed = false;
};

inline std::vector<RadiusOneState> enumerate_radius_one_states(
    std::size_t dimension) {
    if (dimension == 0) return {};
    std::vector<RadiusOneState> states;
    RadiusOneState current(dimension, -1);
    while (true) {
        bool boundary = false;
        for (const auto value : current) boundary |= std::llabs(value) == 1;
        if (boundary) states.push_back(current);

        std::size_t i = 0;
        while (i < dimension && current[i] == 1) {
            current[i] = -1;
            ++i;
        }
        if (i == dimension) break;
        ++current[i];
    }
    return states;
}

inline RadiusOneState translation_step(
    const RadiusOneState& state,
    std::int64_t defect) {
    if (state.empty()) throw std::invalid_argument("empty translation state");
    RadiusOneState next(state.size(), 0);
    for (std::size_t i = 0; i + 1 < state.size(); ++i)
        next[i] = state[i + 1];
    std::int64_t tail = state.front() + defect;
    for (std::size_t i = 1; i < state.size(); ++i) tail -= state[i];
    next.back() = tail;
    return next;
}

inline bool is_radius_one(const RadiusOneState& state) {
    bool boundary = false;
    for (const auto value : state) {
        if (value < -1 || value > 1) return false;
        boundary |= std::llabs(value) == 1;
    }
    return boundary;
}

inline bool aligned_with_any_face(
    const RadiusOneState& state,
    const std::vector<BoundaryFace>& faces) {
    for (const auto& face : faces)
        if (face.coordinate < state.size() &&
            state[face.coordinate] == face.sign)
            return true;
    return false;
}

inline RadiusOneSynthesisCertificate synthesize_radius_one_controller(
    std::size_t dimension,
    std::vector<std::int64_t> base_digits,
    std::vector<BoundaryFace> source_faces,
    std::vector<BoundaryFace> target_faces) {
    RadiusOneSynthesisCertificate cert;
    cert.dimension = dimension;
    cert.base_digits = std::move(base_digits);
    cert.source_faces = std::move(source_faces);
    cert.target_faces = std::move(target_faces);

    const auto states = enumerate_radius_one_states(dimension);
    using Parent = std::tuple<RadiusOneState, std::int64_t, std::int64_t>;
    std::vector<std::map<RadiusOneState, Parent>> parents(cert.base_digits.size() + 1);
    std::set<RadiusOneState> current;
    for (const auto& state : states) {
        if (aligned_with_any_face(state, cert.source_faces)) {
            current.insert(state);
            parents[0].emplace(state, Parent{});
        }
    }

    for (std::size_t k = 0; k < cert.base_digits.size(); ++k) {
        std::set<RadiusOneState> next_layer;
        for (const auto& state : current) {
            for (const auto adjusted_digit : {-1LL, 0LL, 1LL}) {
                const auto defect = adjusted_digit - cert.base_digits[k];
                const auto next = translation_step(state, defect);
                if (!is_radius_one(next)) continue;
                if (!parents[k + 1].contains(next)) {
                    parents[k + 1].emplace(
                        next, Parent{state, defect, adjusted_digit});
                    next_layer.insert(next);
                }
            }
        }
        current = std::move(next_layer);
        if (current.empty()) return cert;
    }

    std::optional<RadiusOneState> terminal;
    for (const auto& state : current) {
        if (aligned_with_any_face(state, cert.target_faces)) {
            terminal = state;
            break;
        }
    }
    if (!terminal) return cert;

    cert.translation_windows.resize(cert.base_digits.size() + 1);
    cert.splice_defects.resize(cert.base_digits.size());
    cert.adjusted_digits.resize(cert.base_digits.size());
    cert.translation_windows.back() = *terminal;
    for (std::size_t k = cert.base_digits.size(); k > 0; --k) {
        const auto& [previous, defect, adjusted] =
            parents[k].at(cert.translation_windows[k]);
        cert.translation_windows[k - 1] = previous;
        cert.splice_defects[k - 1] = defect;
        cert.adjusted_digits[k - 1] = adjusted;
    }

    cert.source_aligned = aligned_with_any_face(
        cert.translation_windows.front(), cert.source_faces);
    cert.target_aligned = aligned_with_any_face(
        cert.translation_windows.back(), cert.target_faces);
    cert.radius_one = std::all_of(
        cert.translation_windows.begin(), cert.translation_windows.end(),
        is_radius_one);
    cert.adjusted_digits_admissible = std::all_of(
        cert.adjusted_digits.begin(), cert.adjusted_digits.end(),
        [](std::int64_t digit) { return -1 <= digit && digit <= 1; });
    cert.recurrence_checked = true;
    for (std::size_t k = 0; k < cert.base_digits.size(); ++k) {
        cert.recurrence_checked &=
            cert.adjusted_digits[k] ==
                cert.base_digits[k] + cert.splice_defects[k];
        cert.recurrence_checked &=
            translation_step(cert.translation_windows[k],
                             cert.splice_defects[k]) ==
                cert.translation_windows[k + 1];
    }
    cert.closed = cert.source_aligned && cert.target_aligned &&
                  cert.radius_one && cert.recurrence_checked &&
                  cert.adjusted_digits_admissible;
    return cert;
}

}  // namespace ravel::proof
