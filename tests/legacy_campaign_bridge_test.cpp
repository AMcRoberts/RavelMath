#include <cassert>
#include <iostream>

#include "ravel/proof/legacy_campaign_bridge.hpp"

int main() {
    ravel::proof::ClosedProofArtifact legacy;
    legacy.artifact_id = "closed.matrix.step";
    legacy.definitions.push_back({"matrix", "M", {}, "Type", "value"});
    legacy.theorems.push_back({"theorem", "det_M", {}, "M.det = p", {}});
    legacy.established_facts = {"determinant formula"};
    legacy.has_open_goals = false;

    const auto imported = ravel::proof::generalized::import_legacy_artifact(
        "universal-n", "determinant", legacy);
    assert(imported.closed);
    assert(imported.compartment == ravel::proof::generalized::Compartment::Matrix);
    const auto& evidence = std::get<ravel::proof::generalized::LegacyCampaignArtifactEvidence>(
        imported.evidence);
    assert(evidence.definition_count == 1);
    assert(evidence.theorem_count == 1);
    assert(imported.lean_declarations.size() == 2);
    std::cout << "legacy universal-n artifact bridge: closed\n";
}
