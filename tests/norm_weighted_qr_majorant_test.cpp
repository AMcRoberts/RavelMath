#include <cassert>
#include <fstream>
#include <iostream>
#include <vector>

#include "math/proof_reflection.hpp"
#include "ravel/proof/norm_weighted_qr_majorant.hpp"
#include "ravel/proof/reflective_lean_renderer.hpp"

using namespace ravel::proof;

int main() {
    mathlib::reflection::Trace trace("norm_weighted_qr_majorant_batch");
    mathlib::reflection::ScopedTrace scope(&trace);

    std::vector<NormWeightedQRChannel> contractive{
        {0, 0, 0, 2, {1, 1}},
        {0, 1, 1, 1, {1, 2}},
        {1, 0, 0, 1, {3, 4}},
        {1, 1, 1, 1, {1, 1}}
    };
    const std::vector<QRGeneratorWord> boundary{{0}, {1,0}, {0,1,1}};
    const auto c = derive_norm_weighted_qr_majorant(2, contractive, boundary);
    assert(c.proved);
    assert(c.exact_qr_augmentation_derived);
    assert(c.universal_qr_word_majorant_derived);
    assert(c.nonnegative_boundary_polynomial_majorant_derived);
    assert(c.all_channels_contractive);
    assert(c.ordinary_qr_count_majorizes_norm_qr);
    assert(c.ordinary_word_and_boundary_projection_derived);
    assert(!c.expansive_channel_detected);
    assert(c.replayed_words.size() == boundary.size());
    stage_norm_weighted_qr_majorant(c, "4-channel contractive Q/R majorant sweep");

    std::vector<NormWeightedQRChannel> expansive{{0, 0, 0, 1, {2, 1}}};
    const auto e = derive_norm_weighted_qr_majorant(1, expansive, {{0}, {0,0}});
    assert(e.proved);
    assert(e.expansive_channel_detected);
    assert(!e.ordinary_qr_count_majorizes_norm_qr);
    assert(!e.ordinary_word_and_boundary_projection_derived);
    assert(e.unweighted_dominance_counterexample_derived);
    assert(e.norm_weighted_theorem_covers_expansive_case);
    assert(e.q_count[0][0].numerator == 1);
    assert(e.q_norm[0][0].numerator == 2);
    assert(e.replayed_words[1].norm_word[0][0].numerator == 4);
    stage_norm_weighted_qr_majorant(e, "1-channel expansive Q/R majorant sweep");

    auto malformed = expansive;
    malformed[0].certified_operator_norm_bound.denominator = 0;
    assert(!derive_norm_weighted_qr_majorant(1, malformed).proved);
    stage_norm_weighted_qr_majorant(
        derive_norm_weighted_qr_majorant(1, malformed), "malformed, must not record");

    auto nodes = trace.find<mathlib::reflection::NormWeightedQRMajorantReflectionCertificate>();
    std::cout << "trace recorded " << nodes.size()
              << " NormWeightedQRMajorantReflectionCertificate nodes\n";
    assert(nodes.size() == 2);

    std::string lean = render_reflective_lean_module(trace);
    assert(lean.find("norm_qr_word_majorant") != std::string::npos);
    assert(lean.find("scalar_two_loop_counterexample") != std::string::npos);

    std::ofstream out("lean/generated/norm_weighted_qr_majorant.lean");
    out << lean;
    out.close();

    std::cout << "norm-weighted Q/R majorant PASS\n";
}
