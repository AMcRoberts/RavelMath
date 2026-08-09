// Wires lean/generated/finite_positive_grammar_majorant.lean -- the
// GENERAL theorem behind the Q/R (2-generator), plastic/supergolden
// (3-generator), and the found 4/5-generator witnesses -- to the
// reflection pipeline. finite_positive_grammar_majorant_test.cpp already
// exercises a genuine 3-generator case but never emits any Lean.
// stage_finite_positive_grammar_majorant independently re-sums the
// per-generator count/norm-weighted matrices from raw channel data.

#include <cassert>
#include <fstream>
#include <iostream>

#include "math/proof_reflection.hpp"
#include "ravel/proof/finite_positive_grammar_majorant.hpp"
#include "ravel/proof/reflective_lean_renderer.hpp"

int main() {
    using namespace ravel::proof;

    mathlib::reflection::Trace trace("finite_positive_grammar_majorant_batch");
    {
        mathlib::reflection::ScopedTrace scope(&trace);

        // Reuses the exact three-generator case (balanced, left defect,
        // right defect) from tests/finite_positive_grammar_majorant_test.cpp.
        const std::vector<NormWeightedGrammarChannel> channels{
            {0, 0, 0, 1, {1, 1}},
            {0, 1, 1, 2, {1, 2}},
            {1, 0, 2, 1, {3, 4}},
            {1, 1, 0, 1, {1, 1}},
        };
        const std::vector<PositiveGeneratorWord> boundary{
            {0}, {1, 2}, {0, 1, 2}, {2, 1, 0}
        };
        const auto cert = derive_finite_positive_grammar_majorant(2, 3, channels, boundary);
        assert(cert.proved);
        assert(cert.generator_count == 3);
        stage_finite_positive_grammar_majorant(cert, "3-generator balanced/left-defect/right-defect grammar");

        // Negative control: malformed channel must not stage.
        const auto bad = derive_finite_positive_grammar_majorant(
            1, 2, {{0, 0, 2, 1, {1, 1}}});
        assert(!bad.proved);
        stage_finite_positive_grammar_majorant(bad, "malformed control");
    }

    auto nodes = trace.find<mathlib::reflection::FinitePositiveGrammarMajorantReflectionCertificate>();
    std::cout << "trace recorded " << nodes.size()
              << " FinitePositiveGrammarMajorantReflectionCertificate nodes\n";
    assert(nodes.size() == 1);
    assert(nodes.front().second->generator_count == 3);

    std::string lean = ravel::proof::render_reflective_lean_module(trace);
    assert(lean.find("theorem norm_generator_word_majorant") != std::string::npos);
    assert(lean.find("grammar_majorant_instance_0") != std::string::npos);
    assert(lean.find("grammar_majorant_instance_1_bound") == std::string::npos);

    std::ofstream out("lean/generated/finite_positive_grammar_majorant_batch.lean");
    out << lean;
    out.close();

    std::cout << "finite_positive_grammar_majorant_reflection_test: PASS\n";
    return 0;
}
