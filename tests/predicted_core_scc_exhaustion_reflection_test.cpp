// Wires lean/predicted_core_scc_exhaustion.lean (previously flat: zero C++
// consumer) to the reflection pipeline. certify_predicted_core_scc
// independently enumerates the concrete predicted-core graph at a given
// dimension, checks exact node/edge/predecessor-table counts against the
// closed-form combinatorial formulas, and runs Tarjan's algorithm to
// verify the graph forms exactly one SCC. On success the renderer
// instantiates exact_scc_of_stronglyConnected_noReturnAfterExit at a
// concrete Fin node_count vertex type with Core := Set.univ.

#include <cassert>
#include <fstream>
#include <iostream>

#include "math/proof_reflection.hpp"
#include "ravel/proof/predicted_core_scc_exhaustion_reflection.hpp"
#include "ravel/proof/reflective_lean_renderer.hpp"

int main() {
    using namespace ravel::proof;

    mathlib::reflection::Trace trace("predicted_core_scc_exhaustion_batch");
    {
        mathlib::reflection::ScopedTrace scope(&trace);
        for (std::size_t n : {3, 4}) {
            const auto cert = certify_predicted_core_scc(n);
            assert(cert.replay_checked);
            assert(cert.direct_tarjan_one_scc);
            stage_predicted_core_scc_exhaustion(
                cert, "dimension " + std::to_string(n) + " predicted-core graph");
        }

        // Negative control: an un-replay-checked certificate must not stage.
        ravel::proof::PredictedCoreSccCertificate bad;
        bad.dimension = 99;
        bad.replay_checked = false;
        stage_predicted_core_scc_exhaustion(bad, "unreplayed control");
    }

    auto nodes = trace.find<mathlib::reflection::PredictedCoreSccExhaustionCertificate>();
    std::cout << "trace recorded " << nodes.size()
              << " PredictedCoreSccExhaustionCertificate nodes\n";
    assert(nodes.size() == 2);

    std::string lean = ravel::proof::render_reflective_lean_module(trace);
    assert(lean.find("theorem exact_scc_of_stronglyConnected_noReturnAfterExit") != std::string::npos);
    assert(lean.find("predicted_core_scc_exhaustion_instance_0") != std::string::npos);
    assert(lean.find("predicted_core_scc_exhaustion_instance_1") != std::string::npos);
    assert(lean.find("predicted_core_scc_exhaustion_instance_2") == std::string::npos);

    std::ofstream out("lean/generated/predicted_core_scc_exhaustion_batch.lean");
    out << lean;
    out.close();

    std::cout << "predicted_core_scc_exhaustion_reflection_test: PASS\n";
    return 0;
}
