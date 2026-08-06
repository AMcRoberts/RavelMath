#pragma once

#include <cstdint>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace ravel::proof {

struct ShellPumpStep {
    std::string source_id;
    std::string target_id;
    std::uint64_t source_radius = 0;
    std::uint64_t target_radius = 0;
    bool source_return_capable = false;
    bool target_return_capable = false;
    bool strict_outward = false;
};

struct UniversalShellPumpingCertificate {
    std::uint64_t admissible_radius_bound = 0;
    std::vector<ShellPumpStep> pump_steps;
    bool every_step_recurrent = false;
    bool every_step_strict = false;
    bool replay_checked = false;
};

inline UniversalShellPumpingCertificate replay_shell_pumping_certificate(
    std::uint64_t admissible_radius_bound,
    std::vector<ShellPumpStep> steps) {
    UniversalShellPumpingCertificate cert;
    cert.admissible_radius_bound = admissible_radius_bound;
    cert.pump_steps = std::move(steps);
    cert.every_step_recurrent = true;
    cert.every_step_strict = true;

    for (auto& step : cert.pump_steps) {
        step.strict_outward = step.source_radius < step.target_radius;
        cert.every_step_recurrent &=
            step.source_return_capable && step.target_return_capable;
        cert.every_step_strict &= step.strict_outward;
    }

    cert.replay_checked =
        cert.every_step_recurrent && cert.every_step_strict;
    return cert;
}

inline bool outer_recurrence_contradicts_bound(
    std::uint64_t radius,
    std::uint64_t admissible_radius_bound,
    std::uint64_t strict_lifts) {
    if (radius < 2) return false;
    if (strict_lifts <= admissible_radius_bound) return false;
    return radius + strict_lifts > admissible_radius_bound;
}

}  // namespace ravel::proof
