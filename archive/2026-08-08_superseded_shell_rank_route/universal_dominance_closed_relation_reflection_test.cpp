// Wires lean/universal_dominance_shell_return_validation.lean and
// lean/universal_dominance_phase_rank_transport.lean (previously flat:
// their only prior C++ reference, universal_dominance_campaign.hpp, is a
// documentation catalogue that never emitted any Lean) to the reflection
// pipeline. validate_shell_return_certificate independently verifies from
// raw edge data that every state in a concrete relation has an outgoing
// edge (the exact `hout` hypothesis these theorems need); the theorems
// themselves are then applied at that concrete relation with rank/level
// kept universally quantified, since the content is that no integer rank
// could make the relation strict -- reuses the exact 3-cycle "closed"
// case from tests/universal_dominance_campaign_test.cpp.

#include <cassert>
#include <fstream>
#include <iostream>

#include "math/proof_reflection.hpp"
#include "ravel/proof/reflective_lean_renderer.hpp"
#include "ravel/proof/universal_dominance_campaign.hpp"

int main() {
    using namespace ravel::proof::universal_dominance;

    mathlib::reflection::Trace trace("universal_dominance_closed_relation_batch");
    {
        mathlib::reflection::ScopedTrace scope(&trace);

        ShellReturnCertificate cert;
        cert.dimension = 3;
        cert.shell_radius = 2;
        cert.shell_states = {{2, 0, 0}, {0, 2, 0}, {0, 0, 2}};
        cert.edges = {{0, 1}, {1, 2}, {2, 0}};
        cert.rank = {0, 1, 2};
        auto validation = validate_shell_return_certificate(cert);
        assert(validation.certificate_closed);
        assert(!validation.rank_strict);  // a closed finite strict rank is impossible
        ravel::proof::universal_dominance::stage_universal_dominance_closed_relation(
            cert, "3-cycle closed relation");

        // Negative control: not closed (state 2 has no outgoing edge).
        ShellReturnCertificate open_cert = cert;
        open_cert.edges = {{0, 1}, {1, 2}};
        ravel::proof::universal_dominance::stage_universal_dominance_closed_relation(
            open_cert, "not closed control");
    }

    auto nodes = trace.find<mathlib::reflection::UniversalDominanceClosedRelationCertificate>();
    std::cout << "trace recorded " << nodes.size()
              << " UniversalDominanceClosedRelationCertificate nodes\n";
    assert(nodes.size() == 1);

    std::string lean = ravel::proof::render_reflective_lean_module(trace);
    assert(lean.find("theorem no_strict_rank_relation_closed") != std::string::npos);
    assert(lean.find("theorem shell_empty_of_phase_rank_transport") != std::string::npos);
    assert(lean.find("universal_dominance_no_strict_rank_instance_0") != std::string::npos);
    assert(lean.find("universal_dominance_shell_empty_instance_0") != std::string::npos);
    assert(lean.find("universal_dominance_no_nonempty_instance_0") != std::string::npos);
    assert(lean.find("universal_dominance_phase_rank_instance_0") != std::string::npos);
    assert(lean.find("universal_dominance_no_strict_rank_instance_1") == std::string::npos);

    std::ofstream out("lean/generated/universal_dominance_closed_relation_batch.lean");
    out << lean;
    out.close();

    std::cout << "universal_dominance_closed_relation_reflection_test: PASS\n";
    return 0;
}
