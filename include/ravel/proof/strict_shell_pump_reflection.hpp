// ravel/proof/strict_shell_pump_reflection.hpp
//
// Wires lean/universal_shell_pumping_proof.lean (previously flat: zero C++
// consumer) to the reflection pipeline. ravel::proof::certify_strict_shell_pump
// (strict_shell_pump.hpp) already independently replays a concrete n-bonacci
// carry cycle, a cyclic continuation-controller run, and the resulting
// affine-transported lifted cycle -- checking translation-cycle recurrence,
// admissible adjusted digits, affine-transport replay, closure, and strict
// outward face-aligned radius growth. This file stages that certificate's
// success into mathlib::reflection so the renderer instantiates a concrete
// (source_radius, lifted_radius) instance of the abstract StrictShellPump
// predicate -- not merely restate the general theorem.

#pragma once

#include <string>

#include "math/proof_reflection.hpp"
#include "ravel/proof/strict_shell_pump.hpp"

namespace ravel::proof {

// Self-validating check: certify_strict_shell_pump has already independently
// re-derived every field from raw states/digits (see strict_shell_pump.hpp);
// this only refuses to stage an invalid or non-strictly-outward certificate.
inline bool certify_strict_shell_pump_reflectable(const StrictShellPumpCertificate& cert) {
    return cert.valid && cert.strict_outward && cert.source_radius < cert.lifted_radius;
}

inline void stage_strict_shell_pump_instance(const StrictShellPumpCertificate& cert) {
    if (!certify_strict_shell_pump_reflectable(cert)) return;
    if (!mathlib::reflection::enabled()) return;
    mathlib::reflection::StrictShellPumpInstanceCertificate node;
    node.certificate_id = cert.certificate_id;
    node.source_radius = cert.source_radius;
    node.lifted_radius = cert.lifted_radius;
    node.face_aligned = cert.face_aligned;
    mathlib::reflection::record(mathlib::reflection::NodeKind::LemmaApplication, node);
}

}  // namespace ravel::proof
