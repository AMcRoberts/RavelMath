#pragma once

#include <string>
#include <utility>
#include <vector>

#include "ravel/proof/generalized_campaign.hpp"
#include "ravel/proof/proof_campaign_engine.hpp"

namespace ravel::proof::generalized {

// Adapts a closed artifact from the first-generation matrix campaign engine
// into the compartment-neutral artifact ledger. This makes the kernel-checked
// universal-n campaign a reference backend/instance rather than a parallel
// one-off system. The bridge is intentionally lossless for declarations and
// established facts; later refactors can move individual matrix operations
// into the generalized registry without changing downstream consumers.
inline ProofArtifact import_legacy_artifact(
        const std::string& campaign_id,
        const std::string& task_id,
        const ravel::proof::ClosedProofArtifact& source) {
    if (source.has_open_goals)
        throw std::logic_error("cannot import an open legacy proof artifact");

    LegacyCampaignArtifactEvidence evidence;
    evidence.source_campaign = campaign_id;
    evidence.source_task = task_id;
    evidence.definition_count = source.definitions.size();
    evidence.theorem_count = source.theorems.size();
    evidence.established_facts = source.established_facts;

    std::vector<LeanDeclaration> declarations;
    declarations.reserve(source.definitions.size() + source.theorems.size());
    for (const auto& definition : source.definitions)
        declarations.push_back({definition.name, "legacy typed Lean definition"});
    for (const auto& theorem : source.theorems)
        declarations.push_back({theorem.name, "legacy typed Lean theorem"});

    return ProofArtifact{
        campaign_id + "." + task_id,
        Compartment::Matrix,
        std::move(evidence),
        std::move(declarations),
        source.established_facts,
        true,
    };
}

inline std::vector<ProofArtifact> import_legacy_result(
        const ravel::proof::CampaignResult& result) {
    std::vector<ProofArtifact> imported;
    for (const auto& task : result.tasks) {
        if (task.state != ravel::proof::TaskState::Closed || !task.artifact) continue;
        imported.push_back(import_legacy_artifact(result.theorem_id, task.task_id, *task.artifact));
    }
    return imported;
}

}  // namespace ravel::proof::generalized
