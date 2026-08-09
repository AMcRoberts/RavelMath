#include <cassert>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include "math/proof_reflection.hpp"
#include "ravel/proof/reflective_lean_renderer.hpp"
#include "ravel/proof/sturm_chain_certificate.hpp"

int main() {
    using ravel::proof::SturmChainStageResult;

    const std::vector<long long> plastic_polynomial{-1, -1, 0};
    assert(ravel::proof::stage_sturm_chain_certificate(plastic_polynomial)
           == SturmChainStageResult::reflection_disabled);
    assert(ravel::proof::stage_sturm_chain_certificate({})
           == SturmChainStageResult::invalid_degree);

    mathlib::reflection::Trace trace("finding_30_sturm_chain");
    {
        mathlib::reflection::ScopedTrace scope(&trace);
        assert(ravel::proof::stage_sturm_chain_certificate(
                   plastic_polynomial, "plastic polynomial x^3 - x - 1")
               == SturmChainStageResult::staged);
    }

    const auto nodes = trace.find<mathlib::reflection::SturmChainCertificate>();
    assert(nodes.size() == 1);
    const auto& certificate = *nodes.front().second;
    assert(certificate.classifier_is_pisot);
    assert(certificate.chain.size() == 4);
    assert(certificate.quotients.size() == 2);
    assert(certificate.positive_scales.size() == 2);
    assert(certificate.variations_lo - certificate.variations_hi == 1);
    assert(certificate.root_count == 1);
    assert(certificate.polynomial.front().numerator == "-1");
    assert(certificate.polynomial.back().numerator == "1");

    const std::string lean = ravel::proof::render_reflective_lean_module(trace);
    assert(lean.find("import Mathlib.Analysis.Polynomial.SturmCertificate") != std::string::npos);
    assert(lean.find("sturm_chain_instance_0_certified") != std::string::npos);
    assert(lean.find("sturm_chain_instance_0_root_count") != std::string::npos);

    std::ofstream out("lean/generated/finding_30_sturm_chain.lean");
    out << lean;
    out.close();

    std::cout << "sturm_chain_reflection_test: PASS\n";
    return 0;
}
