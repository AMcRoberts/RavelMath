#pragma once

#include <cstddef>
#include <string>
#include <utility>
#include <vector>

#include "ravel/proof/strict_shell_pump.hpp"

namespace ravel::proof {

/** A replayable certificate that a supplied, complete family of realized
 * recurrent cycles all admits strict shell pumps.  Completeness is deliberately
 * an explicit input fact: this operation never mistakes bounded word testing
 * for a universal first-return-language theorem. */
struct StrictShellPumpExistenceCertificate {
    std::string certificate_id;
    std::vector<StrictShellPumpCertificate> witnesses;
    std::size_t realized_cycle_count = 0;
    std::size_t pumped_cycle_count = 0;
    bool source_family_complete = false;
    bool every_outer_cycle_pumped = false;
    bool valid = false;
    std::string unsupported_reason;
};

inline StrictShellPumpExistenceCertificate
certify_strict_shell_pump_existence(
    std::string certificate_id,
    const std::vector<CarryCycleWitness>& realized_cycles,
    const CyclicControllerPlant& plant,
    bool source_family_complete) {
    StrictShellPumpExistenceCertificate result;
    result.certificate_id = std::move(certificate_id);
    result.realized_cycle_count = realized_cycles.size();
    result.source_family_complete = source_family_complete;

    if (!source_family_complete) {
        result.unsupported_reason =
            "realized recurrent-cycle family is not certified complete";
        return result;
    }

    result.every_outer_cycle_pumped = true;
    for (std::size_t i = 0; i < realized_cycles.size(); ++i) {
        const auto& cycle = realized_cycles[i];
        if (!cycle.valid) {
            result.every_outer_cycle_pumped = false;
            result.unsupported_reason =
                "realized recurrent-cycle witness failed replay";
            break;
        }
        if (unsigned_sup_norm(cycle.states.front()) < 2) continue;

        auto witness = synthesize_strict_shell_pump(
            result.certificate_id + ".cycle." + std::to_string(i),
            cycle,
            plant);
        if (!witness.valid) {
            result.every_outer_cycle_pumped = false;
            result.unsupported_reason = witness.unsupported_reason.empty()
                ? "outer recurrent cycle has no certified strict shell pump"
                : witness.unsupported_reason;
            result.witnesses.push_back(std::move(witness));
            break;
        }
        ++result.pumped_cycle_count;
        result.witnesses.push_back(std::move(witness));
    }

    result.valid = result.source_family_complete &&
                   result.every_outer_cycle_pumped;
    return result;
}

}  // namespace ravel::proof
