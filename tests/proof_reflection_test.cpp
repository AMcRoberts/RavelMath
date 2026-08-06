#include <cassert>
#include <string>

#include "math/poly_matrix.hpp"
#include "math/proof_reflection.hpp"
#include "ravel/proof/proof_campaign_engine.hpp"

int main() {
    mathlib::reflection::Trace trace("test.nbonacci.reflective");
    mathlib::PolyZ qdet;
    {
        mathlib::reflection::ScopedTrace active(&trace);
        qdet = mathlib::det(mathlib::nbonacci_q_matrix(5));
    }
    assert(qdet == mathlib::geometric_sum_z(5));
    assert(!trace.empty());
    assert(!trace.find<mathlib::reflection::MatrixFamily>().empty());
    assert(!trace.find<mathlib::reflection::MatrixInstance>().empty());
    assert(!trace.find<mathlib::reflection::EraseIndexMap>().empty());
    assert(!trace.find<mathlib::reflection::SparseSupportCertificate>().empty());
    assert(!trace.find<mathlib::reflection::ProofObligation>().empty());

    mathlib::reflection::Trace rtrace("test.nbonacci.r.universal");
    {
        mathlib::reflection::ScopedTrace active(&rtrace);
        (void)mathlib::det(mathlib::nbonacci_r_matrix(6));
    }
    assert(!rtrace.find<mathlib::reflection::TriangularityCertificate>().empty());
    assert(rtrace.find<mathlib::reflection::LemmaApplication>().empty());
    ravel::proof::CampaignGenerator generator;
    ravel::proof::ProofCampaignExecutor executor;
    const auto result = executor.run(generator.generate(rtrace), rtrace);
    const auto lean = ravel::proof::render_closed_campaign_lean(result);
    assert(lean.find("lemma rMatrix_det") != std::string::npos);
    assert(lean.find("Matrix.det_of_lowerTriangular") != std::string::npos);

    mathlib::reflection::Trace disabled("disabled");
    (void)mathlib::det(mathlib::nbonacci_r_matrix(4));
    assert(disabled.empty());
    return 0;
}
