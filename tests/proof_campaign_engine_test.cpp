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
    assert(lean.find("rMatrix_det") != std::string::npos);
    assert(lean.find("qMatrix_minor_eq_qMatrix") != std::string::npos);
    assert(lean.find("qMatrix_det_recurrence") != std::string::npos);
    assert(lean.find("have hprincipal") != std::string::npos);
    assert(lean.find("have hresidual") != std::string::npos);
    assert(lean.find("have hdim : (n - 1).succ = n := by omega") != std::string::npos);
    assert(lean.find("have hcol : Fin.cast hdim (oneSucc.succAbove j)") != std::string::npos);
    assert(lean.find("by_cases hj : j.val = 0") != std::string::npos);
    assert(lean.find("have hlt : j.castSucc <") != std::string::npos);
    assert(lean.find("have hnotlt : ¬ j.castSucc <") != std::string::npos);
    assert(lean.find("split_ifs <;> grind") != std::string::npos);
    assert(lean.find("have hzero_summand") != std::string::npos);
    assert(lean.find("have hone_summand") != std::string::npos);
    assert(lean.find("have htail_zero") != std::string::npos);
    assert(lean.find("have htail_sum") != std::string::npos);
    assert(lean.find("rw [Finset.sum_eq_single zeroTail]") != std::string::npos);
    assert(lean.find("have hq_cofactor") != std::string::npos);
    assert(lean.find("have htop_summand") != std::string::npos);
    assert(lean.find("let characteristicTailSummand : Fin n → Polynomial ℤ") != std::string::npos);
    assert(lean.find("let lastTail : Fin n := ⟨n - 1, by omega⟩") != std::string::npos);
    assert(lean.find("apply Finset.sum_eq_single lastTail") != std::string::npos);
    assert(lean.find("have hlastTailVal : lastTail.val = n - 1 := by") != std::string::npos);
    assert(lean.find("have hlastEntry : nbonacciCharacteristicMatrix (n + 1) lastTail.succ 0 = Polynomial.C (-1) := by") != std::string::npos);
    assert(lean.find("norm_num <;> rfl") != std::string::npos);
    assert(lean.find("((Fin.last n).succAbove i)") != std::string::npos);
    assert(lean.find("have hzero_ne_n : (0 : ℕ) ≠ n := by omega") != std::string::npos);
    assert(lean.find("simp [pow_succ]\n    ring") == std::string::npos);
    assert(lean.find("have hr_cofactor :") != std::string::npos);
    assert(lean.find("rw [hr_cofactor]\n    rfl") == std::string::npos);
    assert(lean.find("simp [pow_succ]\n    rfl") != std::string::npos);
    assert(lean.find("all_goals (set_option maxRecDepth 16384 in simp_all)") == std::string::npos);
    assert(lean.find("have hdet_dim : n - 1 = (n - 2).succ := by omega") != std::string::npos);
    assert(lean.find("let hdet_equiv := (Fin.castOrderIso hdet_dim).toEquiv") != std::string::npos);
    assert(lean.find("let qResidualMinorSucc : Matrix (Fin (n - 2).succ) (Fin (n - 2).succ) (Polynomial ℤ) := fun i j => (qResidualMinor n hn) ⟨i.val, by omega⟩ ⟨j.val, by omega⟩") != std::string::npos);
    assert(lean.find("have hdet_reindex : qResidualMinorSucc = (Matrix.reindex hdet_equiv hdet_equiv) (qResidualMinor n hn) := by") != std::string::npos);
    assert(lean.find("change (qResidualMinor n hn) ⟨i.val, by omega⟩ ⟨j.val, by omega⟩ = (qResidualMinor n hn) (hdet_equiv.symm i) (hdet_equiv.symm j)") != std::string::npos);
    assert(lean.find("exact Matrix.det_reindex_self hdet_equiv (qResidualMinor n hn)") != std::string::npos);
    assert(lean.find("rw [← hdet_transport, Matrix.det_succ_column_zero qResidualMinorSucc]") != std::string::npos);
    assert(lean.find("have hfirst_column (x : Fin (n - 2).succ)") != std::string::npos);
    assert(lean.find("qResidualMinorSucc x 0 = if 1 + x.val = n - 1 then 1 else 0") != std::string::npos);
    assert(lean.find("qResidualMinor, qMatrix, Ravel.Matrix.EraseIndex.minor") != std::string::npos);
    assert(lean.find("by_cases hx : 1 + x.val = n - 1") != std::string::npos);
    assert(lean.find("have hx_bound : x.val < n - 1 := by") != std::string::npos);
    assert(lean.find("have hx_comm : x.val + 1 = n - 1 := by omega") != std::string::npos);
    assert(lean.find("have hn_one : 1 < n := by omega") != std::string::npos);
    assert(lean.find("have hzero_ne : (0 : ℕ) ≠ n - 1 := by omega") != std::string::npos);
    assert(lean.find("have hx_comm : x.val + 1 ≠ n - 1 := by omega") != std::string::npos);
    assert(lean.find("have hx_row_lt : x.val + 1 < n - 1 := by omega") != std::string::npos);
    assert(lean.find("have hx_row_ne_zero : x.val + 1 ≠ 0 := by omega") != std::string::npos);
    assert(lean.find("Fin.last_val") == std::string::npos);
    assert(lean.find("have hlast_val : (Fin.last (n - 2)).val = n - 2 := rfl") != std::string::npos);
    assert(lean.find("rw [hlast_val]") != std::string::npos);
    assert(lean.find("have hpow : (n - 2) + (n - 2) = 2 * (n - 2) := by omega") != std::string::npos);
    assert(lean.find("rw [hpow, pow_mul]") != std::string::npos);
    assert(lean.find("rw [Finset.sum_eq_single (Fin.last (n - 2))]") != std::string::npos);
    assert(lean.find("hterminal_cofactor_det (Fin.last (n - 2)) hx") != std::string::npos);
    assert(lean.find("simp [hterminal_cofactor_det, qResidualMinor,") == std::string::npos);
    assert(lean.find("have hterminal_cofactor (x : Fin (n - 2).succ) (hx : 1 + x.val = n - 1)") != std::string::npos);
    assert(lean.find("(qResidualMinorSucc.submatrix x.succAbove Fin.succ) = qResidualCore n hn") != std::string::npos);
    assert(lean.find("simp [qResidualMinorSucc, qResidualCore n hn,") == std::string::npos);
    assert(lean.find("hterminal_cofactor") != std::string::npos);
    assert(lean.find("have hterminal_cofactor_det (x : Fin (n - 2).succ) (hx : 1 + x.val = n - 1)") != std::string::npos);
    assert(lean.find("(qResidualMinorSucc.submatrix x.succAbove Fin.succ).det = (Polynomial.C (-1) : Polynomial ℤ) ^ x.val") != std::string::npos);
    assert(lean.find("rw [hterminal_cofactor x hx, qResidualCore_eq_rMatrix n hn, rMatrix_det]") != std::string::npos);
    assert(lean.find("have hfirst_column (x : Fin (n - 2).succ)") != std::string::npos);
    assert(lean.find("simp [qResidualMinorSucc, hterminal_cofactor") == std::string::npos);
    assert(lean.find("have hdet_dim : n = (n - 1).succ := by omega") != std::string::npos);
    assert(lean.find("let qMatrixSucc : Matrix (Fin (n - 1).succ) (Fin (n - 1).succ) (Polynomial ℤ) := fun i j => (qMatrix (n + 1)) ⟨i.val, by omega⟩ ⟨j.val, by omega⟩") != std::string::npos);
    assert(lean.find("have hdet_reindex : qMatrixSucc = (Matrix.reindex hdet_equiv hdet_equiv) (qMatrix (n + 1)) := by") != std::string::npos);
    assert(lean.find("change (qMatrix (n + 1)) ⟨i.val, by omega⟩ ⟨j.val, by omega⟩ = (qMatrix (n + 1)) (hdet_equiv.symm i) (hdet_equiv.symm j)") != std::string::npos);
    assert(lean.find("exact Matrix.det_reindex_self hdet_equiv (qMatrix (n + 1))") != std::string::npos);
    assert(lean.find("rw [← hdet_transport, Matrix.det_succ_row_zero qMatrixSucc]") != std::string::npos);
    assert(lean.find("let qResidualMinorSucc := fun") == std::string::npos);
    assert(lean.find("let qMatrixSucc := fun") == std::string::npos);
    assert(lean.find("let zeroTail : Fin (n - 1) := ⟨0, by omega⟩") != std::string::npos);
    assert(lean.find("let oneSucc : Fin (n - 1).succ := zeroTail.succ") != std::string::npos);
    assert(lean.find("rw [Matrix.det_succ_column_zero (qResidualMinor n hn)]") == std::string::npos);
    assert(lean.find("rw [Matrix.det_succ_row_zero (qMatrix (n + 1))]") == std::string::npos);
    assert(lean.find("hdet_dim ▸ (qResidualMinor n hn)") == std::string::npos);
    assert(lean.find("hdet_dim ▸ (qMatrix (n + 1))") == std::string::npos);
    assert(lean.find("cases hdet_dim") == std::string::npos);
    assert(lean.find("Equiv.cast (congrArg Fin hdet_dim)") == std::string::npos);
    assert(lean.find("simp [qResidualMinorSucc, hdet_equiv, Matrix.reindex_apply") == std::string::npos);
    assert(lean.find("simp [qMatrixSucc, hdet_equiv, Matrix.reindex_apply") == std::string::npos);
    assert(lean.find("congr 1 <;> apply Fin.ext <;> rfl") != std::string::npos);
    assert(lean.find("simp [qResidualMinorSucc, hdet_equiv, Matrix.reindex_apply, Fin.castOrderIso_apply]") == std::string::npos);
    assert(lean.find("qMatrix_det_closed_form") != std::string::npos);
    assert(lean.find("nbonacci_characteristic_split") != std::string::npos);
    assert(lean.find("nbonacci_universal_n") != std::string::npos);
    assert(lean.find("sorry") == std::string::npos);
    assert(lean.find("by_cases hx : 1 + x.val = n - 1") != std::string::npos);
    assert(lean.find("h_i_terminal") == std::string::npos);
    assert(lean.find("  funext i j") != std::string::npos);
    assert(lean.find("  all_goals (set_option maxRecDepth 4096 in simp_all) <;> omega") != std::string::npos);
    assert(lean.find("simp only [nbonacciCharacteristicMatrix, qMatrix, Ravel.Matrix.EraseIndex.minor, Ravel.Matrix.EraseIndex.skip_zero_val]") != std::string::npos);
    assert(lean.find("simp only [nbonacciCharacteristicMatrix, rMatrix, Ravel.Matrix.EraseIndex.minor, Ravel.Matrix.EraseIndex.skip_finLast_val, Ravel.Matrix.EraseIndex.skip_zero_val]") != std::string::npos);
    assert(lean.find("  all_goals simp_all only <;> omega <;> ring") == std::string::npos);
    assert(lean.find("  all_goals omega") == std::string::npos);
    assert(lean.find("hi_characteristic_branch_bound") == std::string::npos);
    assert(lean.find("hj_characteristic_branch_bound") == std::string::npos);
    assert(lean.find("have hn_sub_two : n - 2 + 2 = n := by omega") != std::string::npos);
    assert(lean.find("have hi_bound : i.val < n - 1 := i.isLt") != std::string::npos);
    assert(lean.find("have hj_bound : j.val < n - 1 := j.isLt") != std::string::npos);
    assert(lean.find("have hj_bound : j.val < n - 1 := i.isLt") == std::string::npos);
    assert(lean.find("have hi_terminal_bound : i.val ≤ n - 2 := by omega") != std::string::npos);
    assert(lean.find("have hj_shift_bound : j.val + 1 < n := by omega") != std::string::npos);
    assert(lean.find("have hi_double_shift_bound : i.val + 2 < n := by omega") != std::string::npos);
    assert(lean.find("have hn_branch_positive : 0 < n + 1 - 2 := by omega") != std::string::npos);
    assert(lean.find("have hj_branch_bound : j.val < n + 1 - 2 := by omega") != std::string::npos);
    assert(lean.find("have hj_residual_double_shift_bound : j.val + 2 < n + 1 - 1 := by omega") != std::string::npos);
    assert(lean.find("change i.val < j.val at hij") != std::string::npos);
    assert(lean.find("simpa [Fin.lt_def] using hij") == std::string::npos);
    // The q-minor transport is now Closed, so the renderer omits the
    // blocked diagnostic from the emitted module.
    assert(lean.find("generic erase-minor") == std::string::npos);

    const auto report = result.report();
    assert(report.find("campaign closed: yes") != std::string::npos);
    return 0;
}

// refined exact-error repair marker
