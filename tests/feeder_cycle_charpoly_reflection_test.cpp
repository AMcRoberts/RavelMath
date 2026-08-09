// Wires the "installed graph attractor model" (docs/GRAPH_CYCLE_
// CANONICALIZATION_AND_FACTOR_SEAM.md, lean/cycle_charpoly_campaign_
// validation.lean, lean/graph_cycle_charpoly_factor_validation.lean) to the
// reflection pipeline: certify_feeder_cycle_structure independently
// reconstructs the (n+1)-cycle-plus-feeder edge set and checks every degree
// condition feeder_cycle_charpoly_closed's hypotheses require; on success
// the renderer instantiates the closed form X*(X^(n+1)-1) at that CONCRETE
// n via concreteCycleMatrix_charpoly (freshly completed this session --
// the dimension-transport step docs/CYCLE_CHARPOLY_PROOF_CAMPAIGN.md's own
// plan left as future work) combined with feeder_cycle_charpoly_closed,
// not merely restated as a comment.

#include <cassert>
#include <fstream>
#include <iostream>

#include "math/proof_reflection.hpp"
#include "ravel/proof/feeder_cycle_charpoly_certificate.hpp"
#include "ravel/proof/reflective_lean_renderer.hpp"

int main() {
    mathlib::reflection::Trace trace("feeder_cycle_charpoly_batch");
    {
        mathlib::reflection::ScopedTrace scope(&trace);
        ravel::proof::stage_feeder_cycle_charpoly(
            6, 0, "7-cycle with feeder edge into vertex 0");
        ravel::proof::stage_feeder_cycle_charpoly(
            3, 2, "4-cycle with feeder edge into vertex 2");

        // Negative control: feeder_target out of range must record nothing.
        ravel::proof::stage_feeder_cycle_charpoly(
            4, 99, "out-of-range feeder target (negative control)");
    }

    auto nodes = trace.find<mathlib::reflection::FeederCycleCharpolyCertificate>();
    std::cout << "trace recorded " << nodes.size() << " FeederCycleCharpolyCertificate nodes\n";
    assert(nodes.size() == 2);

    std::string lean = ravel::proof::render_reflective_lean_module(trace);
    assert(lean.find("concreteCycleMatrix_charpoly") != std::string::npos);
    assert(lean.find("feeder_cycle_charpoly_closed") != std::string::npos);
    assert(lean.find("feeder_cycle_charpoly_instance_0") != std::string::npos);
    assert(lean.find("feeder_cycle_charpoly_instance_1") != std::string::npos);
    assert(lean.find("feeder_cycle_charpoly_instance_2") == std::string::npos);

    std::ofstream out("lean/generated/feeder_cycle_charpoly_batch.lean");
    out << lean;
    out.close();

    std::cout << "feeder_cycle_charpoly_reflection_test: PASS\n";
    return 0;
}
