#pragma once

#include <cstddef>
#include <string>
#include <utility>
#include <vector>

#include "ravel/proof/role_constrained_controller_lift.hpp"
#include "ravel/proof/strict_shell_pump.hpp"

namespace ravel::proof {

struct RoleConstrainedShellPumpCertificate {
    RoleConstrainedControllerLiftCertificate lift;
    CarryCycleWitness repeated_source_cycle;
    StrictShellPumpCertificate strict_pump;
    bool source_role_is_active_face = false;
    bool repetition_count_integral = false;
    bool source_cycle_replay = false;
    bool valid = false;
    std::string obstruction;
};

inline CarryCycleWitness repeat_carry_cycle(
    const CarryCycleWitness& source,
    std::size_t repetitions,
    std::string witness_id) {
    if (!source.valid || repetitions == 0) {
        CarryCycleWitness empty;
        empty.witness_id = std::move(witness_id);
        return empty;
    }
    std::vector<IntegerState> states;
    std::vector<std::int64_t> digits;
    states.push_back(source.states.front());
    for (std::size_t r = 0; r < repetitions; ++r) {
        digits.insert(digits.end(), source.digits.begin(), source.digits.end());
        states.insert(states.end(), source.states.begin() + 1, source.states.end());
    }
    return replay_carry_cycle(
        std::move(witness_id), std::move(states), std::move(digits));
}

/** Compose the exact role-constrained controller lift with StrictShellPump.
 * The selected initial stepped-face role must be an active face of the source
 * shell state.  The source carry cycle is repeated exactly as many times as
 * the reconstructed controller word requires. */
inline RoleConstrainedShellPumpCertificate
 certify_role_constrained_shell_pump(
    std::string certificate_id,
    CarryCycleWitness source_cycle,
    const FirstReturnJointProduct& product,
    RoleConstrainedControllerLiftCertificate lift) {
    RoleConstrainedShellPumpCertificate cert;
    cert.lift = std::move(lift);
    if (!source_cycle.valid || !cert.lift.valid) {
        cert.obstruction = "source cycle or controller lift is invalid";
        return cert;
    }
    if (cert.lift.lap_roles.empty() || source_cycle.states.empty()) {
        cert.obstruction = "missing source face role";
        return cert;
    }
    const auto role = cert.lift.lap_roles.front();
    const auto radius = static_cast<std::int64_t>(
        unsigned_sup_norm(source_cycle.states.front()));
    cert.source_role_is_active_face =
        role.first < source_cycle.states.front().size() &&
        source_cycle.states.front()[role.first] == role.second * radius;
    if (!cert.source_role_is_active_face) {
        cert.obstruction = "selected role is not an active source-shell face";
        return cert;
    }
    cert.repetition_count_integral =
        !source_cycle.digits.empty() &&
        cert.lift.controller_run.base_word.size() % source_cycle.digits.size() == 0;
    if (!cert.repetition_count_integral) {
        cert.obstruction = "controller word is not an integral source-cycle repetition";
        return cert;
    }
    const auto repetitions =
        cert.lift.controller_run.base_word.size() / source_cycle.digits.size();
    cert.repeated_source_cycle = repeat_carry_cycle(
        source_cycle, repetitions, certificate_id + ".source-repeat");
    cert.source_cycle_replay = cert.repeated_source_cycle.valid &&
        cert.repeated_source_cycle.digits == cert.lift.controller_run.base_word;
    if (!cert.source_cycle_replay) {
        cert.obstruction = "repeated source cycle failed replay";
        return cert;
    }

    CyclicControllerPlant plant;
    plant.states = product.controller_states;
    plant.successors = product.controller_plant.successors;
    cert.strict_pump = certify_strict_shell_pump(
        std::move(certificate_id),
        cert.repeated_source_cycle,
        plant,
        cert.lift.controller_run);
    cert.valid = cert.source_role_is_active_face &&
                 cert.repetition_count_integral &&
                 cert.source_cycle_replay && cert.strict_pump.valid;
    if (!cert.valid) cert.obstruction = cert.strict_pump.unsupported_reason;
    return cert;
}

} // namespace ravel::proof
