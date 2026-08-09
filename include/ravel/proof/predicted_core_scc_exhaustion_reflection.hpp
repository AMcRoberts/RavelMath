// ravel/proof/predicted_core_scc_exhaustion_reflection.hpp
//
// Wires lean/predicted_core_scc_exhaustion.lean (previously flat: zero C++
// consumer) to the reflection pipeline. ravel::proof::certify_predicted_core_scc
// independently runs Tarjan's algorithm over the concrete predicted-core
// graph at a given dimension, verifying it forms exactly one SCC together
// with exact node/edge/predecessor-table counts against the closed-form
// combinatorial formulas -- nothing here is asserted, every field is
// recomputed from the raw node/edge enumeration.

#pragma once

#include <string>

#include "math/proof_reflection.hpp"
#include "ravel/proof/predicted_core_scc_identification.hpp"

namespace ravel::proof {

inline void stage_predicted_core_scc_exhaustion(
    const PredictedCoreSccCertificate& cert, const std::string& description) {
    if (!cert.replay_checked || !cert.direct_tarjan_one_scc) return;
    if (!mathlib::reflection::enabled()) return;
    mathlib::reflection::PredictedCoreSccExhaustionCertificate node;
    node.dimension = static_cast<long long>(cert.dimension);
    node.node_count = static_cast<long long>(cert.nodes.size());
    node.edge_count = static_cast<long long>(cert.edges.size());
    node.description = description;
    mathlib::reflection::record(mathlib::reflection::NodeKind::LemmaApplication, node);
}

}  // namespace ravel::proof
