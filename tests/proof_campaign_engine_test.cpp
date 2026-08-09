#include <cassert>
#include <string>

#include "math/poly_matrix.hpp"
#include "math/proof_reflection.hpp"
#include "ravel/proof/proof_campaign_engine.hpp"

int main() {
    mathlib::reflection::Trace trace("test.nbonacci.executable_campaign");
    {
        mathlib::reflection::ScopedTrace active(&trace);
        (void)mathlib::det(mathlib::nbonacci_r_matrix(6));
        (void)mathlib::det(mathlib::nbonacci_q_matrix(6));
    }

    ravel::proof::CampaignGenerator generator;
    auto campaign = generator.generate(trace);
    assert(!campaign.tasks.empty());

    ravel::proof::ProofCampaignExecutor executor;
    const auto result = executor.run(std::move(campaign), trace);

    const auto* triangular = result.find_task("r_matrix.lower_triangular");
    assert(triangular != nullptr);
    assert(triangular->state == ravel::proof::TaskState::Closed);
    assert(triangular->artifact.has_value());
    assert(!triangular->artifact->has_open_goals);

    const auto* determinant = result.find_task("r_matrix.determinant");
    assert(determinant != nullptr);
    assert(determinant->state == ravel::proof::TaskState::Closed);

    const auto* qminor = result.find_task("q_matrix.minor_transport");
    assert(qminor != nullptr);
    assert(qminor->state == ravel::proof::TaskState::Closed);
    assert(qminor->artifact.has_value());
    assert(!qminor->artifact->has_open_goals);
    assert(qminor->artifact->artifact_id == "q_matrix.minor_transport");

    const auto* universal = result.find_task("nbonacci.universal_n");
    assert(universal != nullptr);
    assert(universal->state == ravel::proof::TaskState::Closed);
    assert(universal->artifact.has_value());
    assert(!universal->artifact->has_open_goals);

    const auto lean = ravel::proof::render_closed_campaign_lean(result);
    assert(lean.find("rMatrix_lowerTriangular") != std::string::npos);
    assert(lean.find("Matrix.det_of_lowerTriangular") != std::string::npos);
    assert(lean.find("qMatrix_minor_eq_qMatrix") != std::string::npos);
    assert(lean.find("qResidualCore_eq_rMatrix") != std::string::npos);
    assert(lean.find("qResidualMinor_det") != std::string::npos);
    assert(lean.find("qMatrix_det_recurrence") != std::string::npos);
    assert(lean.find("nbonacciCharacteristic_minor_q") != std::string::npos);
    assert(lean.find("nbonacciCharacteristic_minor_r") != std::string::npos);
    assert(lean.find("nbonacci_characteristic_split") != std::string::npos);
    assert(lean.find("nbonacci_universal_n") != std::string::npos);
    assert(lean.find("Matrix.submatrix") != std::string::npos);
    assert(lean.find("Matrix.reindex") != std::string::npos);
    assert(lean.find("Fin.succ") != std::string::npos);
    assert(lean.find("Fin.castSucc") != std::string::npos);
    assert(lean.find("have hrow :") != std::string::npos);
    assert(lean.find("have hinner_col :") != std::string::npos);
    assert(lean.find("have hcol_skip :") != std::string::npos);
    assert(lean.find("have hcol :") != std::string::npos);
    assert(lean.find("hdet_equiv.symm j.succ") != std::string::npos);
    assert(lean.find("⟨j.val + 2, by omega⟩") != std::string::npos);
    assert(lean.find("have hzero_map") != std::string::npos);
    assert(lean.find("have hlast_map") != std::string::npos);
    assert(lean.find("split_ifs <;> grind") != std::string::npos);
    assert(lean.find("Ravel.Matrix.EraseIndex") == std::string::npos);
    assert(lean.find("sorry") == std::string::npos);
    assert(lean.find("admit") == std::string::npos);

    const auto report = result.report();
    assert(report.find("campaign closed: yes") != std::string::npos);
    return 0;
}

// refined exact-error repair marker
