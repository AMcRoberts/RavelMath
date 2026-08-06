#pragma once

#include <filesystem>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <string>

#include "ravel/proof/first_return_dimension_extension.hpp"

namespace ravel::proof {

struct FirstReturnUniversalComposition {
    bool joint_quotient_ready = false;
    bool dimension_extension_ready = false;
    bool realized_language_complete = false;
    bool outward_acceptance_ready = false;
    bool universal_theorem_emitted = false;
    std::string obstruction;
};

inline FirstReturnUniversalComposition compose_first_return_universal_proof(
    const JointRoleQuotientProof& base,
    const DimensionExtensionProof& extension) {
    FirstReturnUniversalComposition p;
    p.joint_quotient_ready = base.exact_on_reachable_product &&
                             base.predecessor_congruent;
    p.dimension_extension_ready = extension.proved;
    if (!p.joint_quotient_ready) {
        p.obstruction = "reachable joint quotient is not exact";
        return p;
    }
    if (!p.dimension_extension_ready) {
        p.obstruction = extension.obstruction;
        return p;
    }
    p.obstruction =
        "dimension extension proved, but completeness of the realized first-return "
        "plant and outward terminal acceptance still require proof objects";
    return p;
}

inline std::string render_universal_composition_report(
    const FirstReturnUniversalComposition& p) {
    std::ostringstream o;
    o << "FIRST_RETURN_UNIVERSAL_COMPOSITION\n";
    o << "joint_quotient_ready=" << (p.joint_quotient_ready ? "true" : "false") << "\n";
    o << "dimension_extension_ready=" << (p.dimension_extension_ready ? "true" : "false") << "\n";
    o << "realized_language_complete=" << (p.realized_language_complete ? "true" : "false") << "\n";
    o << "outward_acceptance_ready=" << (p.outward_acceptance_ready ? "true" : "false") << "\n";
    o << "universal_theorem_emitted=" << (p.universal_theorem_emitted ? "true" : "false") << "\n";
    if (!p.obstruction.empty()) o << "obstruction=" << p.obstruction << "\n";
    return o.str();
}

} // namespace ravel::proof
