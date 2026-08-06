#pragma once

#include <cstdint>
#include <string>

#include "ravel/proof/role_constrained_shell_pump.hpp"

namespace ravel::proof {

struct MaximumShellMaximalityContradictionCertificate {
    std::uint64_t claimed_maximum_radius = 0;
    std::uint64_t source_radius = 0;
    std::uint64_t lifted_radius = 0;
    bool source_is_maximal = false;
    bool lift_is_recurrent = false;
    bool strict_outward = false;
    bool contradicts_maximality = false;
    bool valid = false;
    std::string obstruction;
};

/** Close the final logical step after a replayed strict shell pump.
 *
 * `claimed_maximum_radius` is the radius of a recurrent cycle selected maximal
 * in a finite recurrent region.  A valid role-constrained shell pump produces
 * another closed recurrent carry cycle with larger radius, contradicting that
 * maximal choice.
 */
inline MaximumShellMaximalityContradictionCertificate
 certify_maximum_shell_maximality_contradiction(
    std::uint64_t claimed_maximum_radius,
    const RoleConstrainedShellPumpCertificate& pump) {
    MaximumShellMaximalityContradictionCertificate cert;
    cert.claimed_maximum_radius = claimed_maximum_radius;
    cert.source_radius = pump.strict_pump.source_radius;
    cert.lifted_radius = pump.strict_pump.lifted_radius;
    if (!pump.valid) {
        cert.obstruction = "role-constrained shell pump is invalid";
        return cert;
    }
    cert.source_is_maximal = cert.source_radius == claimed_maximum_radius;
    cert.lift_is_recurrent = pump.strict_pump.lifted_cycle.valid;
    cert.strict_outward = cert.source_radius < cert.lifted_radius;
    cert.contradicts_maximality = cert.source_is_maximal &&
        cert.lift_is_recurrent && cert.strict_outward &&
        cert.claimed_maximum_radius < cert.lifted_radius;
    cert.valid = cert.contradicts_maximality;
    if (!cert.valid) cert.obstruction = "strict lift does not contradict the claimed maximum";
    return cert;
}

} // namespace ravel::proof
