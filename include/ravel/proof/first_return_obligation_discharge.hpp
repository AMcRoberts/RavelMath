#pragma once

#include <filesystem>
#include <map>
#include <set>
#include <string>
#include <utility>
#include <vector>

#include "ravel/proof/first_return_joint_product.hpp"
#include "ravel/proof/covering_translation_tube.hpp"

namespace ravel::proof {

struct ExtendedFirstReturnWitness {
    FirstReturnWitness basic;
    std::vector<IntegerState> translation_windows;
    std::vector<std::int64_t> defects;
    std::vector<std::int64_t> adjusted_digits;
};

inline std::vector<ExtendedFirstReturnWitness>
load_extended_first_return_witnesses(const std::filesystem::path& path) {
    const auto text = detail::read_text(path);
    auto p = detail::find_value(text, "bound");
    const auto bound = detail::parse_int(text, p);
    const auto objects = detail::split_object_array(text, "certificates");
    std::vector<ExtendedFirstReturnWitness> result;
    result.reserve(objects.size());
    for (const auto& object : objects) {
        ExtendedFirstReturnWitness e;
        e.basic.bound = bound;
        auto q = detail::find_value(object, "digits");
        e.basic.digits = detail::parse_int_array_at(object, q);
        q = detail::find_value(object, "base_path");
        e.basic.base_path = detail::parse_matrix_at(object, q);
        q = detail::find_value(object, "target");
        e.basic.target = detail::parse_int_array_at(object, q);
        q = detail::find_value(object, "translation_windows");
        e.translation_windows = detail::parse_matrix_at(object, q);
        q = detail::find_value(object, "defects");
        e.defects = detail::parse_int_array_at(object, q);
        q = detail::find_value(object, "adjusted_digits");
        e.adjusted_digits = detail::parse_int_array_at(object, q);
        e.basic.dimension = e.basic.target.size();
        result.push_back(std::move(e));
    }
    return result;
}

struct FirstReturnObligationDischarge {
    std::size_t dimension = 0;
    std::size_t witness_count = 0;
    std::size_t controller_path_replays = 0;
    std::size_t endpoint_monitor_accepts = 0;
    std::size_t affine_transport_replays = 0;
    std::size_t strict_shell_segments = 0;
    bool realized_segment_language_complete = false;
    bool accepted_path_affine_sound = false;
    bool recurrent_splice_complete = false;
    std::string remaining_obstruction;
};

inline FirstReturnObligationDischarge discharge_first_return_obligations(
    std::size_t dimension,
    const std::vector<std::filesystem::path>& files) {
    FirstReturnObligationDischarge r;
    r.dimension = dimension;
    for (const auto& file : files) {
        const auto witnesses = load_extended_first_return_witnesses(file);
        for (const auto& w : witnesses) {
            if (w.basic.dimension != dimension) continue;
            ++r.witness_count;

            bool controller_replay =
                w.translation_windows.size() == w.basic.digits.size() + 1;
            if (controller_replay) {
                for (std::size_t k = 0; k < w.basic.digits.size(); ++k) {
                    const auto successors = symbolic_successors(
                        w.translation_windows[k], w.basic.digits[k]);
                    controller_replay &=
                        successors.contains(w.translation_windows[k + 1]);
                }
            }
            if (controller_replay) ++r.controller_path_replays;

            const auto faces = detail::target_faces(
                w.basic.target, w.basic.bound);
            bool terminal_accepts = false;
            if (!w.translation_windows.empty()) {
                const auto& terminal = w.translation_windows.back();
                for (const auto& [coord, sign] : faces) {
                    terminal_accepts |= coord < terminal.size() &&
                        terminal[coord] == sign;
                }
            }
            if (terminal_accepts) ++r.endpoint_monitor_accepts;

            const auto cert = certify_defect_spliced_tube(
                "first-return-obligation",
                w.basic.bound,
                1,
                w.basic.base_path,
                w.translation_windows,
                w.basic.digits,
                w.defects);
            const bool affine = cert.lengths_match &&
                cert.dimensions_match &&
                cert.translation_cover_checked &&
                cert.splice_recurrence_checked &&
                cert.adjusted_digits_admissible &&
                cert.transported_replay_checked;
            if (affine) ++r.affine_transport_replays;
            if (cert.first_return_transport) ++r.strict_shell_segments;
        }
    }
    r.realized_segment_language_complete =
        r.witness_count > 0 &&
        r.controller_path_replays == r.witness_count &&
        r.endpoint_monitor_accepts == r.witness_count;
    r.accepted_path_affine_sound =
        r.witness_count > 0 &&
        r.affine_transport_replays == r.witness_count &&
        r.strict_shell_segments == r.witness_count;
    r.recurrent_splice_complete = false;
    r.remaining_obstruction =
        "prove compatible controller/translation endpoint splicing around every "
        "realized recurrent first-return cycle; individual accepted segments "
        "already replay and transport strictly outward";
    return r;
}

inline std::string render_first_return_obligation_report(
    const FirstReturnObligationDischarge& r) {
    return "FIRST_RETURN_OBLIGATION_DISCHARGE\n"
        "dimension=" + std::to_string(r.dimension) + "\n"
        "witnesses=" + std::to_string(r.witness_count) + "\n"
        "controller_path_replays=" + std::to_string(r.controller_path_replays) + "\n"
        "endpoint_monitor_accepts=" + std::to_string(r.endpoint_monitor_accepts) + "\n"
        "affine_transport_replays=" + std::to_string(r.affine_transport_replays) + "\n"
        "strict_shell_segments=" + std::to_string(r.strict_shell_segments) + "\n"
        "realized_segment_language_complete=" +
            std::string(r.realized_segment_language_complete ? "true" : "false") + "\n"
        "accepted_path_affine_sound=" +
            std::string(r.accepted_path_affine_sound ? "true" : "false") + "\n"
        "recurrent_splice_complete=false\n"
        "remaining_obstruction=" + r.remaining_obstruction + "\n";
}

} // namespace ravel::proof
