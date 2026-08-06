#pragma once

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <limits>
#include <optional>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include "ravel/proof/covering_translation_tube.hpp"
#include "ravel/proof/cyclic_continuation_controller.hpp"
#include "ravel/proof/symbolic_radius_one_controller.hpp"

namespace ravel::proof {

/** A replayable closed n-bonacci carry cycle. */
struct CarryCycleWitness {
    std::string witness_id;
    std::vector<IntegerState> states;
    std::vector<std::int64_t> digits;
    bool dimensions_match = false;
    bool lengths_match = false;
    bool transition_replay = false;
    bool closes = false;
    bool valid = false;
};

inline CarryCycleWitness replay_carry_cycle(
    std::string witness_id,
    std::vector<IntegerState> states,
    std::vector<std::int64_t> digits,
    std::int64_t digit_min = -1,
    std::int64_t digit_max = 1) {
    CarryCycleWitness witness;
    witness.witness_id = std::move(witness_id);
    witness.states = std::move(states);
    witness.digits = std::move(digits);
    witness.lengths_match =
        !witness.states.empty() &&
        witness.states.size() == witness.digits.size() + 1;
    if (!witness.lengths_match) return witness;

    const auto dimension = witness.states.front().size();
    witness.dimensions_match = dimension > 0;
    for (const auto& state : witness.states)
        witness.dimensions_match &= state.size() == dimension;
    if (!witness.dimensions_match) return witness;

    witness.transition_replay = true;
    for (std::size_t k = 0; k < witness.digits.size(); ++k) {
        witness.transition_replay &=
            witness.digits[k] >= digit_min && witness.digits[k] <= digit_max;
        witness.transition_replay &=
            nbonacci_step(witness.states[k], witness.digits[k]) ==
            witness.states[k + 1];
    }
    witness.closes = witness.states.front() == witness.states.back();
    witness.valid = witness.transition_replay && witness.closes;
    return witness;
}

struct StrictShellPumpCertificate {
    std::string certificate_id;
    CarryCycleWitness source_cycle;
    CyclicRunCertificate controller_run;
    std::vector<IntegerState> translation_cycle;
    std::vector<std::int64_t> translation_defects;
    CarryCycleWitness lifted_cycle;
    std::uint64_t source_radius = 0;
    std::uint64_t lifted_radius = 0;
    bool controller_state_dimensions_match = false;
    bool controller_cycle_replay = false;
    bool adjusted_digits_admissible = false;
    bool affine_transport_replay = false;
    bool face_aligned = false;
    bool strict_outward = false;
    bool valid = false;
    std::string unsupported_reason;
};

inline std::uint64_t unsigned_sup_norm(const IntegerState& state) {
    return static_cast<std::uint64_t>(sup_norm(state));
}

/**
 * Turn a concrete closed carry cycle and a cyclic continuation-controller run
 * into the exact local StrictShellPump witness.
 *
 * The operation does not trust pre-computed defects or transported states.  It
 * reconstructs each translation defect from the controller transition,
 * derives the adjusted digit, replays the lifted n-bonacci cycle, checks
 * closure, and finally checks strict shell growth at the aligned basepoint.
 */
inline StrictShellPumpCertificate certify_strict_shell_pump(
    std::string certificate_id,
    CarryCycleWitness source_cycle,
    const CyclicControllerPlant& plant,
    CyclicRunCertificate controller_run,
    std::int64_t adjusted_digit_min = -1,
    std::int64_t adjusted_digit_max = 1) {
    StrictShellPumpCertificate cert;
    cert.certificate_id = std::move(certificate_id);
    cert.source_cycle = std::move(source_cycle);
    cert.controller_run = std::move(controller_run);

    if (!cert.source_cycle.valid) {
        cert.unsupported_reason = "source cycle did not replay and close";
        return cert;
    }
    if (!cert.controller_run.valid) {
        cert.unsupported_reason = "controller run did not replay and close";
        return cert;
    }
    if (cert.controller_run.base_word != cert.source_cycle.digits) {
        cert.unsupported_reason = "controller word differs from source digits";
        return cert;
    }
    if (cert.controller_run.controller_states.size() !=
        cert.source_cycle.states.size()) {
        cert.unsupported_reason = "controller and source path lengths differ";
        return cert;
    }

    const auto dimension = cert.source_cycle.states.front().size();
    cert.controller_state_dimensions_match = true;
    for (const auto state_id : cert.controller_run.controller_states) {
        if (state_id >= plant.states.size()) {
            cert.controller_state_dimensions_match = false;
            break;
        }
        cert.controller_state_dimensions_match &=
            plant.states[state_id].size() == dimension;
        if (!cert.controller_state_dimensions_match) break;
        cert.translation_cycle.push_back(plant.states[state_id]);
    }
    if (!cert.controller_state_dimensions_match) {
        cert.unsupported_reason = "controller state dimension mismatch";
        return cert;
    }

    cert.controller_cycle_replay =
        cert.translation_cycle.front() == cert.translation_cycle.back();
    cert.adjusted_digits_admissible = true;
    cert.affine_transport_replay = true;

    std::vector<IntegerState> lifted_states;
    std::vector<std::int64_t> lifted_digits;
    lifted_states.reserve(cert.source_cycle.states.size());
    lifted_digits.reserve(cert.source_cycle.digits.size());

    for (std::size_t k = 0; k < cert.source_cycle.states.size(); ++k)
        lifted_states.push_back(add_states(
            cert.source_cycle.states[k], cert.translation_cycle[k]));

    for (std::size_t k = 0; k < cert.source_cycle.digits.size(); ++k) {
        const auto& translation = cert.translation_cycle[k];
        const auto& translation_next = cert.translation_cycle[k + 1];
        const auto defect =
            translation_next.back() - controller_center(translation);
        cert.translation_defects.push_back(defect);
        cert.controller_cycle_replay &=
            nbonacci_step(translation, defect) == translation_next;

        const auto adjusted = cert.source_cycle.digits[k] + defect;
        lifted_digits.push_back(adjusted);
        cert.adjusted_digits_admissible &=
            adjusted >= adjusted_digit_min && adjusted <= adjusted_digit_max;
        cert.affine_transport_replay &=
            nbonacci_step(lifted_states[k], adjusted) ==
            lifted_states[k + 1];
    }

    cert.lifted_cycle = replay_carry_cycle(
        cert.certificate_id + ".lifted",
        std::move(lifted_states),
        std::move(lifted_digits),
        adjusted_digit_min,
        adjusted_digit_max);

    cert.source_radius = unsigned_sup_norm(cert.source_cycle.states.front());
    cert.lifted_radius = unsigned_sup_norm(cert.lifted_cycle.states.front());

    // Face alignment means an extremal source coordinate is translated in its
    // own sign direction.  This is stronger than merely observing a larger norm.
    cert.face_aligned = false;
    const auto& source = cert.source_cycle.states.front();
    const auto& translation = cert.translation_cycle.front();
    const auto signed_source_radius = static_cast<std::int64_t>(cert.source_radius);
    for (std::size_t i = 0; i < dimension; ++i) {
        if (source[i] == signed_source_radius && translation[i] > 0)
            cert.face_aligned = true;
        if (source[i] == -signed_source_radius && translation[i] < 0)
            cert.face_aligned = true;
    }
    cert.strict_outward =
        cert.face_aligned && cert.source_radius < cert.lifted_radius;

    cert.valid = cert.controller_cycle_replay &&
                 cert.adjusted_digits_admissible &&
                 cert.affine_transport_replay &&
                 cert.lifted_cycle.valid &&
                 cert.strict_outward;
    if (!cert.valid && cert.unsupported_reason.empty()) {
        if (!cert.controller_cycle_replay)
            cert.unsupported_reason = "translation cycle failed recurrence replay";
        else if (!cert.adjusted_digits_admissible)
            cert.unsupported_reason = "transported digits are not admissible";
        else if (!cert.affine_transport_replay || !cert.lifted_cycle.valid)
            cert.unsupported_reason = "lifted carry cycle failed replay or closure";
        else if (!cert.face_aligned)
            cert.unsupported_reason = "closing controller is not outward face-aligned";
        else
            cert.unsupported_reason = "transport does not strictly increase shell radius";
    }
    return cert;
}

/** Search all cyclic fixed points, retaining the first one that produces a
 * fully replayed strict shell lift. */
inline StrictShellPumpCertificate synthesize_strict_shell_pump(
    std::string certificate_id,
    CarryCycleWitness source_cycle,
    const CyclicControllerPlant& plant) {
    StrictShellPumpCertificate last;
    last.certificate_id = certificate_id;
    last.source_cycle = source_cycle;
    if (!source_cycle.valid) {
        last.unsupported_reason = "source cycle did not replay and close";
        return last;
    }

    const auto fixed = fixed_points_under_word(plant, source_cycle.digits);
    if (fixed.empty()) {
        last.unsupported_reason = "no cyclic controller fixed point for source word";
        return last;
    }

    for (const auto initial : fixed) {
        auto restricted = plant;
        // Force the generic synthesizer to choose this fixed point first.
        if (initial != 0) {
            std::swap(restricted.states[0], restricted.states[initial]);
            std::map<std::pair<std::size_t, std::int64_t>,
                     std::vector<std::size_t>> remapped;
            const auto remap = [initial](std::size_t id) {
                if (id == 0) return initial;
                if (id == initial) return std::size_t{0};
                return id;
            };
            for (const auto& [key, values] : restricted.successors) {
                auto mapped_values = values;
                for (auto& value : mapped_values) value = remap(value);
                remapped[{remap(key.first), key.second}] =
                    std::move(mapped_values);
            }
            restricted.successors = std::move(remapped);
        }

        auto run = synthesize_cyclic_run(
            restricted, source_cycle.digits,
            certificate_id + ".controller");
        auto candidate = certify_strict_shell_pump(
            certificate_id, source_cycle, restricted, std::move(run));
        if (candidate.valid) return candidate;
        last = std::move(candidate);
    }
    return last;
}

}  // namespace ravel::proof
