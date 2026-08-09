// ravel/proof/recurrent_family_exhaustion_reflection.hpp
//
// Wires lean/playground_recurrent_family_exhaustion.lean (previously flat:
// zero C++ consumer) to the reflection pipeline. ravel::proof::
// derive_recurrent_family_exhaustion aggregates a per-component witness
// list; the real caller (tests/recurrent_family_exhaustion_real_test.cpp)
// independently extracts every recurrent SCC of a concrete corona-truth
// graph via Tarjan and classifies each one from its own replayed
// grade/permutation/core-predicate evidence -- nothing here is asserted,
// every witness field is re-derived from the raw graph. This file stages
// a `.proved` exhaustion proof's per-component classification into
// mathlib::reflection so the renderer instantiates the general
// `recurrent_family_exhaustion` theorem's `classified` premise at this
// CONCRETE dimension's component list.

#pragma once

#include <string>

#include "math/proof_reflection.hpp"
#include "ravel/proof/recurrent_family_exhaustion.hpp"

namespace ravel::proof {

inline void stage_recurrent_family_exhaustion(
    const RecurrentFamilyExhaustionProof& proof, const std::string& description) {
    if (!proof.proved) return;
    if (!mathlib::reflection::enabled()) return;
    mathlib::reflection::RecurrentFamilyExhaustionCertificate node;
    node.dimension = static_cast<long long>(proof.dimension);
    node.description = description;
    for (const auto& witness : proof.witnesses)
        node.family_kinds.push_back(recurrent_family_kind_name(witness.kind));
    mathlib::reflection::record(mathlib::reflection::NodeKind::LemmaApplication, node);
}

}  // namespace ravel::proof
