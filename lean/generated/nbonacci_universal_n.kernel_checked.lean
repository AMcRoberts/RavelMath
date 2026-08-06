import Mathlib.Algebra.BigOperators.Group.Finset.Defs
import Mathlib.Algebra.Polynomial.Basic
import Mathlib.LinearAlgebra.Matrix.Determinant.Basic
import Mathlib.Tactic
import Ravel.Matrix.EraseIndex
import Ravel.Polynomial.Normalization

namespace RavelGenerated

open Matrix BigOperators

/-- Symbolic family generated from the reflected lower-bidiagonal entry rule. -/
noncomputable def rMatrix (n : ℕ) :
    Matrix (Fin (n - 1)) (Fin (n - 1)) (Polynomial ℤ) :=
  fun i j => if j.val + 1 = i.val then Polynomial.X else if i.val = j.val then Polynomial.C (-1) else 0

/-- Symbolic family generated from the reflected piecewise entry rule. -/
noncomputable def qMatrix (n : ℕ) :
    Matrix (Fin (n - 1)) (Fin (n - 1)) (Polynomial ℤ) :=
  fun i j => if ((i.val = (n - 2)) ∧ (j.val = (n - 2))) then (Polynomial.C 1 + Polynomial.X : Polynomial ℤ) else if ((i.val = (n - 2)) ∧ (j.val < (n - 2))) then (Polynomial.C 1 : Polynomial ℤ) else if ((i.val < (n - 2)) ∧ (j.val = (i.val + 1))) then (Polynomial.C (-1) : Polynomial ℤ) else if ((i.val < (n - 2)) ∧ (j.val = i.val)) then (Polynomial.X : Polynomial ℤ) else (0 : Polynomial ℤ)

/-- The non-principal row-zero cofactor selected by the sparse-cofactor operator. -/
noncomputable def qResidualMinor (n : ℕ) (hn : 2 ≤ n) :
    Matrix (Fin (n - 1)) (Fin (n - 1)) (Polynomial ℤ) :=
  Ravel.Matrix.EraseIndex.minor (qMatrix (n + 1)) ⟨0, by omega⟩ ⟨1, by omega⟩

/-- The core left after the residual minor's singleton first-column expansion. -/
noncomputable def qResidualCore (n : ℕ) (hn : 2 ≤ n) :
    Matrix (Fin (n - 2)) (Fin (n - 2)) (Polynomial ℤ) :=
  Ravel.Matrix.EraseIndex.minor (qResidualMinor n hn) (Ravel.Matrix.EraseIndex.lastPred hn) (Ravel.Matrix.EraseIndex.firstPred hn)

/-- Characteristic-matrix family supplied by the theorem campaign. -/
noncomputable def nbonacciCharacteristicMatrix (n : ℕ) :
    Matrix (Fin n) (Fin n) (Polynomial ℤ) :=
  fun i j => if i.val = n - 1 ∧ j.val = n - 1 then Polynomial.X + 1 else if i.val = n - 1 ∧ j.val = 0 then Polynomial.C (-1) else if i.val = n - 1 then Polynomial.C 1 else if i.val + 1 = j.val then Polynomial.C (-1) else if i.val = j.val then Polynomial.X else 0

/-- The universal target polynomial. -/
noncomputable def nbonacciPolynomial (n : ℕ) :
    Polynomial ℤ :=
  (∑ k ∈ Finset.range n, (Polynomial.X : Polynomial ℤ) ^ (k + 1)) - 1

/-- Automatically discharged support certificate for the reflected family. -/
lemma rMatrix_lowerTriangular (n : ℕ) :
    (rMatrix n).BlockTriangular (OrderDual.toDual : Fin (n - 1) → OrderDual (Fin (n - 1))) := by
  intro i j hij
  simp only [rMatrix]
  have hlt : i.val < j.val := by
    change i.val < j.val at hij
    exact hij
  have hsub : j.val + 1 ≠ i.val := by omega
  have hdiag : i.val ≠ j.val := Nat.ne_of_lt hlt
  simp [hsub, hdiag]

/-- Automatically composed from the closed triangularity certificate. -/
lemma rMatrix_det (n : ℕ) :
    (rMatrix n).det = (Polynomial.C (-1) : Polynomial ℤ) ^ (n - 1) := by
  rw [Matrix.det_of_lowerTriangular (rMatrix n) (rMatrix_lowerTriangular n)]
  simp [rMatrix]

/-- Erased-index transport generated from the typed family and transform. -/
lemma qMatrix_minor_eq_qMatrix (n : ℕ) (hn : 2 ≤ n) :
    Ravel.Matrix.EraseIndex.minor (qMatrix (n + 1)) ⟨0, by omega⟩ ⟨0, by omega⟩ = qMatrix n := by
  funext i j
  have hi := i.isLt
  have hj := j.isLt
  have hn_sub_two : n - 2 + 2 = n := by omega
  have hn_succ_sub_two : n + 1 - 2 = n - 1 := by omega
  have hn_sub_one : n - 1 + 1 = n := by omega
  have hn_branch_positive : 0 < n + 1 - 2 := by omega
  have hi_bound : i.val < n - 1 := i.isLt
  have hj_bound : j.val < n - 1 := j.isLt
  have hi_branch_bound : i.val < n + 1 - 2 := by omega
  have hj_branch_bound : j.val < n + 1 - 2 := by omega
  have hi_terminal_bound : i.val ≤ n - 2 := by omega
  have hj_terminal_bound : j.val ≤ n - 2 := by omega
  have hi_shift_bound : i.val + 1 < n := by omega
  have hj_shift_bound : j.val + 1 < n := by omega
  simp only [qMatrix, Ravel.Matrix.EraseIndex.minor, Ravel.Matrix.EraseIndex.skip_zero_val]
  split_ifs
  all_goals (set_option maxRecDepth 4096 in simp_all) <;> omega

/-- Entrywise transport from the residual cofactor core to the reflected residual family. -/
lemma qResidualCore_eq_rMatrix (n : ℕ) (hn : 2 ≤ n) :
    qResidualCore n hn = rMatrix (n - 1) := by
  funext i j
  have hi := i.isLt
  have hj := j.isLt
  have hn_sub_two : n - 2 + 2 = n := by omega
  have hn_succ_sub_two : n + 1 - 2 = n - 1 := by omega
  have hn_succ_sub_one : n + 1 - 1 = n := by omega
  have hn_residual_branch_positive : 0 < n + 1 - 1 := by omega
  have hi_bound : i.val < n - 2 := i.isLt
  have hj_bound : j.val < n - 2 := j.isLt
  have hi_residual_branch_bound : i.val < n + 1 - 1 := by omega
  have hj_residual_branch_bound : j.val < n + 1 - 1 := by omega
  have hi_residual_shift_bound : i.val + 1 < n + 1 - 1 := by omega
  have hj_residual_shift_bound : j.val + 1 < n + 1 - 1 := by omega
  have hi_residual_double_shift_bound : i.val + 2 < n + 1 - 1 := by omega
  have hj_residual_double_shift_bound : j.val + 2 < n + 1 - 1 := by omega
  have hi_shift_bound : i.val + 1 < n - 1 := by omega
  have hj_shift_bound : j.val + 1 < n - 1 := by omega
  have hi_double_shift_bound : i.val + 2 < n := by omega
  have hj_double_shift_bound : j.val + 2 < n := by omega
  simp only [qResidualCore, qResidualMinor, qMatrix, rMatrix, Ravel.Matrix.EraseIndex.minor, Ravel.Matrix.EraseIndex.skip_zero_skip_lastPred_val, Ravel.Matrix.EraseIndex.skip_one_skip_firstPred_val]
  split_ifs
  all_goals (set_option maxRecDepth 4096 in simp_all) <;> omega

/-- The residual cofactor determinant computed through its transported structural core. -/
lemma qResidualMinor_det (n : ℕ) (hn : 2 ≤ n) :
    (qResidualMinor n hn).det = 1 := by
  have hdet_dim : n - 1 = (n - 2).succ := by omega
  let hdet_equiv := (Fin.castOrderIso hdet_dim).toEquiv
  let qResidualMinorSucc : Matrix (Fin (n - 2).succ) (Fin (n - 2).succ) (Polynomial ℤ) := fun i j => (qResidualMinor n hn) ⟨i.val, by omega⟩ ⟨j.val, by omega⟩
  have hdet_reindex : qResidualMinorSucc = (Matrix.reindex hdet_equiv hdet_equiv) (qResidualMinor n hn) := by
    funext i j
    change (qResidualMinor n hn) ⟨i.val, by omega⟩ ⟨j.val, by omega⟩ = (qResidualMinor n hn) (hdet_equiv.symm i) (hdet_equiv.symm j)
    congr 1 <;> apply Fin.ext <;> rfl
  have hdet_transport : qResidualMinorSucc.det = (qResidualMinor n hn).det := by
    rw [hdet_reindex]
    exact Matrix.det_reindex_self hdet_equiv (qResidualMinor n hn)
  rw [← hdet_transport, Matrix.det_succ_column_zero qResidualMinorSucc]
  have hterminal_cofactor (x : Fin (n - 2).succ) (hx : 1 + x.val = n - 1) :
      (qResidualMinorSucc.submatrix x.succAbove Fin.succ) = qResidualCore n hn := by
    have hx_last : x = Fin.last (n - 2) := by
      apply Fin.ext
      simp
      omega
    subst x
    funext i j
    simp [qResidualMinorSucc, qResidualCore, Ravel.Matrix.EraseIndex.minor, Ravel.Matrix.EraseIndex.skip_lastPred_val, Ravel.Matrix.EraseIndex.skip_zero_val]
    congr 1 <;> apply Fin.ext <;> simp
  have hterminal_cofactor_det (x : Fin (n - 2).succ) (hx : 1 + x.val = n - 1) :
      (qResidualMinorSucc.submatrix x.succAbove Fin.succ).det = (Polynomial.C (-1) : Polynomial ℤ) ^ x.val := by
    rw [hterminal_cofactor x hx, qResidualCore_eq_rMatrix n hn, rMatrix_det]
    congr 1
    omega
  have hfirst_column (x : Fin (n - 2).succ) :
      qResidualMinorSucc x 0 = if 1 + x.val = n - 1 then 1 else 0 := by
    have hx_bound : x.val < n - 1 := by
      have h := x.isLt
      omega
    by_cases hx : 1 + x.val = n - 1
    · have hx_comm : x.val + 1 = n - 1 := by omega
      have hn_one : 1 < n := by omega
      have hzero_ne : (0 : ℕ) ≠ n - 1 := by omega
      simp [qResidualMinorSucc, qResidualMinor, qMatrix, Ravel.Matrix.EraseIndex.minor, Ravel.Matrix.EraseIndex.skip_zero_val, Ravel.Matrix.EraseIndex.skip_val_of_lt, hx, hx_comm, hn_one, hzero_ne]
    · have hx_comm : x.val + 1 ≠ n - 1 := by omega
      have hx_row_lt : x.val + 1 < n - 1 := by omega
      have hx_row_ne_zero : x.val + 1 ≠ 0 := by omega
      simp [qResidualMinorSucc, qResidualMinor, qMatrix, Ravel.Matrix.EraseIndex.minor, Ravel.Matrix.EraseIndex.skip_zero_val, Ravel.Matrix.EraseIndex.skip_val_of_lt, hx, hx_comm, hx_row_lt, hx_row_ne_zero]
  rw [Finset.sum_eq_single (Fin.last (n - 2))]
  · have hx : 1 + (Fin.last (n - 2)).val = n - 1 := by
      simp
      omega
    rw [hfirst_column (Fin.last (n - 2)), if_pos hx, hterminal_cofactor_det (Fin.last (n - 2)) hx]
    have hlast_val : (Fin.last (n - 2)).val = n - 2 := rfl
    rw [hlast_val]
    simp only [mul_one]
    have hneg : Polynomial.C (-1 : ℤ) = (-1 : Polynomial ℤ) := by
      norm_num
    rw [hneg]
    rw [← pow_add]
    have hpow : (n - 2) + (n - 2) = 2 * (n - 2) := by omega
    rw [hpow, pow_mul]
    simp
  · intro b hb hne
    have hx : 1 + b.val ≠ n - 1 := by
      intro h
      apply hne
      apply Fin.ext
      simp
      omega
    rw [hfirst_column b, if_neg hx]
    simp
  · simp

/-- Sparse row expansion composed from support, minor transports, and cofactor signs. -/
lemma qMatrix_det_recurrence (n : ℕ) (hn : 2 ≤ n) :
    (qMatrix (n + 1)).det = Polynomial.X * (qMatrix n).det + 1 := by
  have hdet_dim : n = (n - 1).succ := by omega
  let hdet_equiv := (Fin.castOrderIso hdet_dim).toEquiv
  let qMatrixSucc : Matrix (Fin (n - 1).succ) (Fin (n - 1).succ) (Polynomial ℤ) := fun i j => (qMatrix (n + 1)) ⟨i.val, by omega⟩ ⟨j.val, by omega⟩
  have hdet_reindex : qMatrixSucc = (Matrix.reindex hdet_equiv hdet_equiv) (qMatrix (n + 1)) := by
    funext i j
    change (qMatrix (n + 1)) ⟨i.val, by omega⟩ ⟨j.val, by omega⟩ = (qMatrix (n + 1)) (hdet_equiv.symm i) (hdet_equiv.symm j)
    congr 1 <;> apply Fin.ext <;> rfl
  have hdet_transport : qMatrixSucc.det = (qMatrix (n + 1)).det := by
    rw [hdet_reindex]
    exact Matrix.det_reindex_self hdet_equiv (qMatrix (n + 1))
  rw [← hdet_transport, Matrix.det_succ_row_zero qMatrixSucc]
  have hprincipal :
      qMatrixSucc.submatrix Fin.succ Fin.succ = qMatrix n := by
    funext i j
    have hentry := congrArg (fun M => M i j) (qMatrix_minor_eq_qMatrix n hn)
    change qMatrix (n + 1) ⟨i.val + 1, by omega⟩ ⟨j.val + 1, by omega⟩ = qMatrix n i j
    have hrow : (⟨i.val + 1, by omega⟩ : Fin n) = Ravel.Matrix.EraseIndex.skip (⟨0, by omega⟩ : Fin n) i := by
      apply Fin.ext
      simp
    have hcol : (⟨j.val + 1, by omega⟩ : Fin n) = Ravel.Matrix.EraseIndex.skip (⟨0, by omega⟩ : Fin n) j := by
      apply Fin.ext
      simp
    exact (congrArg₂ (qMatrix (n + 1)) hrow hcol).trans hentry
  let zeroTail : Fin (n - 1) := ⟨0, by omega⟩
  let oneSucc : Fin (n - 1).succ := zeroTail.succ
  have hresidual :
      qMatrixSucc.submatrix Fin.succ oneSucc.succAbove = qResidualMinor n hn := by
    funext i j
    change qMatrix (n + 1) ⟨i.val + 1, by omega⟩ ⟨(oneSucc.succAbove j).val, by omega⟩ = qMatrix (n + 1) (Ravel.Matrix.EraseIndex.skip (⟨0, by omega⟩ : Fin n) i) (Ravel.Matrix.EraseIndex.skip (⟨1, by omega⟩ : Fin n) j)
    have hrow : (⟨i.val + 1, by omega⟩ : Fin n) = Ravel.Matrix.EraseIndex.skip (⟨0, by omega⟩ : Fin n) i := by
      apply Fin.ext
      simp
    have hdim : (n - 1).succ = n := by omega
    have hcol : Fin.cast hdim (oneSucc.succAbove j) = Ravel.Matrix.EraseIndex.skip (⟨1, by omega⟩ : Fin n) j := by
      apply Fin.ext
      by_cases hj : j.val = 0
      · have hlt : j.castSucc < (⟨1, by omega⟩ : Fin (n - 1).succ) := by
          change j.val < 1
          omega
        simp [oneSucc, zeroTail, Fin.succAbove, Ravel.Matrix.EraseIndex.skip, hlt, hj]
      · have hnotlt : ¬ j.castSucc < (⟨1, by omega⟩ : Fin (n - 1).succ) := by
          change ¬ j.val < 1
          omega
        simp [oneSucc, zeroTail, Fin.succAbove, Ravel.Matrix.EraseIndex.skip, hnotlt, hj]
    have hcol' : (⟨(oneSucc.succAbove j).val, by omega⟩ : Fin n) = Ravel.Matrix.EraseIndex.skip (⟨1, by omega⟩ : Fin n) j := by
      apply Fin.ext
      simpa [hdim] using congrArg Fin.val hcol
    exact congrArg₂ (qMatrix (n + 1)) hrow hcol'
  have hzero_map : (0 : Fin (n - 1).succ).succAbove = Fin.succ := by
    funext j
    apply Fin.ext
    rfl
  have hzero_summand :
      (-1 : Polynomial ℤ) ^ ((0 : Fin (n - 1).succ) : ℕ) * qMatrixSucc 0 0 * (qMatrixSucc.submatrix Fin.succ (0 : Fin (n - 1).succ).succAbove).det = Polynomial.X * (qMatrix n).det := by
    rw [hzero_map, hprincipal]
    have hn_one : 1 < n := by omega
    have hzero_ne : (0 : ℕ) ≠ n - 1 := by omega
    simp [qMatrixSucc, qMatrix, hn_one, hzero_ne]
  have hone_summand :
      (-1 : Polynomial ℤ) ^ (oneSucc : ℕ) * qMatrixSucc 0 oneSucc * (qMatrixSucc.submatrix Fin.succ oneSucc.succAbove).det = 1 := by
    rw [hresidual, qResidualMinor_det n hn]
    have hn_one : 1 < n := by omega
    have hzero_ne : (0 : ℕ) ≠ n - 1 := by omega
    simp [oneSucc, zeroTail, qMatrixSucc, qMatrix, hn_one, hzero_ne]
  have htail_zero (x : Fin (n - 1)) (hx : x ≠ zeroTail) :
      (-1 : Polynomial ℤ) ^ (x.succ : ℕ) * qMatrixSucc 0 x.succ * (qMatrixSucc.submatrix Fin.succ x.succ.succAbove).det = 0 := by
    have hxv : x.val ≠ 0 := by
      intro h
      apply hx
      apply Fin.ext
      simpa [zeroTail] using h
    have hn_one : 1 < n := by omega
    have hrow_nonterminal : (0 : ℕ) ≠ n - 1 := by omega
    have hcol_ne_one : x.val + 1 ≠ 1 := by omega
    have hentry : qMatrixSucc 0 x.succ = 0 := by
      simp [qMatrixSucc, qMatrix, hn_one, hrow_nonterminal, hcol_ne_one, hxv]
    rw [hentry]
    simp
  rw [Fin.sum_univ_succ]
  rw [hzero_summand]
  have htail_sum :
      (∑ i : Fin (n - 1), (-1 : Polynomial ℤ) ^ (i.succ : ℕ) * qMatrixSucc 0 i.succ * (qMatrixSucc.submatrix Fin.succ i.succ.succAbove).det) = 1 := by
    rw [Finset.sum_eq_single zeroTail]
    · change (-1 : Polynomial ℤ) ^ (oneSucc : ℕ) * qMatrixSucc 0 oneSucc * (qMatrixSucc.submatrix Fin.succ oneSucc.succAbove).det = 1
      exact hone_summand
    · intro b hb hne
      exact htail_zero b hne
    · simp
  rw [htail_sum]

/-- Base case consumed by the recurrence-closure operator. -/
lemma qMatrix_det_closed_form_base :
    (qMatrix 2).det = ∑ k ∈ Finset.range 2, (Polynomial.X : Polynomial ℤ) ^ k := by
  simp [qMatrix, Finset.sum_range_succ]

/-- Successor case consumed by the recurrence-closure operator. -/
lemma qMatrix_det_closed_form_succ (n : ℕ) (hn : 2 ≤ n) (ih : (qMatrix n).det = ∑ k ∈ Finset.range n, (Polynomial.X : Polynomial ℤ) ^ k) :
    (qMatrix (n + 1)).det = ∑ k ∈ Finset.range (n + 1), (Polynomial.X : Polynomial ℤ) ^ k := by
  rw [qMatrix_det_recurrence n hn, ih]
  simpa [add_comm] using Ravel.Polynomial.one_add_X_mul_sum_range_pow n

/-- First-order recurrence closure generated from a base and successor theorem. -/
lemma qMatrix_det_closed_form (n : ℕ) (hn : 2 ≤ n) :
    (qMatrix n).det = ∑ k ∈ Finset.range n, (Polynomial.X : Polynomial ℤ) ^ k := by
  induction n, hn using Nat.le_induction with
  | base => exact qMatrix_det_closed_form_base
  | succ n hn ih => exact qMatrix_det_closed_form_succ n hn ih

/-- Entrywise transport of the selected characteristic minor to the principal family. -/
lemma nbonacciCharacteristic_minor_q (n : ℕ) (hn : 2 ≤ n) :
    Ravel.Matrix.EraseIndex.minor (nbonacciCharacteristicMatrix (n + 1)) ⟨0, by omega⟩ ⟨0, by omega⟩ = qMatrix (n + 1) := by
  funext i j
  have hi := i.isLt
  have hj := j.isLt
  have hn_terminal : n + 1 - 2 = n - 1 := by omega
  have hn_last : n + 1 - 1 = n := by omega
  have hi_shift_bound : i.val + 1 < n + 1 := by omega
  have hj_shift_bound : j.val + 1 < n + 1 := by omega
  have hi_terminal_bound : i.val ≤ n - 1 := by omega
  have hj_terminal_bound : j.val ≤ n - 1 := by omega
  simp only [nbonacciCharacteristicMatrix, qMatrix, Ravel.Matrix.EraseIndex.minor, Ravel.Matrix.EraseIndex.skip_zero_val]
  split_ifs <;> grind

/-- Entrywise transport of the selected characteristic minor to the residual family. -/
lemma nbonacciCharacteristic_minor_r (n : ℕ) (hn : 2 ≤ n) :
    Ravel.Matrix.EraseIndex.minor (nbonacciCharacteristicMatrix (n + 1)) (Fin.last (n)) ⟨0, by omega⟩ = rMatrix (n + 1) := by
  funext i j
  have hi := i.isLt
  have hj := j.isLt
  have hn_last : n + 1 - 1 = n := by omega
  have hj_shift_bound : j.val + 1 < n + 1 := by omega
  simp only [nbonacciCharacteristicMatrix, rMatrix, Ravel.Matrix.EraseIndex.minor, Ravel.Matrix.EraseIndex.skip_finLast_val, Ravel.Matrix.EraseIndex.skip_zero_val]
  split_ifs <;> grind

/-- Sparse cofactor decomposition assembled from transported minors. -/
lemma nbonacci_characteristic_split (n : ℕ) (hn : 2 ≤ n) :
    (nbonacciCharacteristicMatrix (n + 1)).det = Polynomial.X * (qMatrix (n + 1)).det + (-1 : Polynomial ℤ) ^ (n + 1) * (rMatrix (n + 1)).det := by
  rw [Matrix.det_succ_column_zero (nbonacciCharacteristicMatrix (n + 1))]
  rw [Fin.sum_univ_succ]
  have hq_cofactor :
      (nbonacciCharacteristicMatrix (n + 1)).submatrix (0 : Fin (n + 1)).succAbove Fin.succ = qMatrix (n + 1) := by
    funext i j
    have hentry := congrArg (fun M => M i j) (nbonacciCharacteristic_minor_q n hn)
    change nbonacciCharacteristicMatrix (n + 1) ⟨i.val + 1, by omega⟩ ⟨j.val + 1, by omega⟩ = qMatrix (n + 1) i j
    have hrow : (⟨i.val + 1, by omega⟩ : Fin (n + 1)) = Ravel.Matrix.EraseIndex.skip (⟨0, by omega⟩ : Fin (n + 1)) i := by
      apply Fin.ext
      simp
    have hcol : (⟨j.val + 1, by omega⟩ : Fin (n + 1)) = Ravel.Matrix.EraseIndex.skip (⟨0, by omega⟩ : Fin (n + 1)) j := by
      apply Fin.ext
      simp
    exact (congrArg₂ (nbonacciCharacteristicMatrix (n + 1)) hrow hcol).trans hentry
  have htop_summand :
      (-1 : Polynomial ℤ) ^ ((0 : Fin (n + 1)) : ℕ) * nbonacciCharacteristicMatrix (n + 1) 0 0 * ((nbonacciCharacteristicMatrix (n + 1)).submatrix (0 : Fin (n + 1)).succAbove Fin.succ).det = Polynomial.X * (qMatrix (n + 1)).det := by
    rw [hq_cofactor]
    have hentry : nbonacciCharacteristicMatrix (n + 1) 0 0 = Polynomial.X := by
      have hzero_ne_last : (0 : ℕ) ≠ n := by omega
      simp [nbonacciCharacteristicMatrix, hzero_ne_last]
    rw [hentry]
    norm_num <;> rfl
  rw [htop_summand]
  let characteristicTailSummand : Fin n → Polynomial ℤ := fun i =>
    (-1 : Polynomial ℤ) ^ (i.succ : ℕ) * nbonacciCharacteristicMatrix (n + 1) i.succ 0 * ((nbonacciCharacteristicMatrix (n + 1)).submatrix i.succ.succAbove Fin.succ).det
  let lastTail : Fin n := ⟨n - 1, by omega⟩
  have htail_single :
      (∑ i : Fin n, characteristicTailSummand i) = characteristicTailSummand lastTail := by
    apply Finset.sum_eq_single lastTail
    · intro b hb hne
      dsimp [characteristicTailSummand]
      have hb_value_ne_last : b.val + 1 ≠ n := by
        intro h
        apply hne
        apply Fin.ext
        simp [lastTail]
        omega
      have hentry : nbonacciCharacteristicMatrix (n + 1) b.succ 0 = 0 := by
        have hb_lt := b.isLt
        simp [nbonacciCharacteristicMatrix, hb_value_ne_last]
      rw [hentry]
      simp
    · simp
  have htail_sum :
      (∑ i : Fin n, (-1 : Polynomial ℤ) ^ (i.succ : ℕ) * nbonacciCharacteristicMatrix (n + 1) i.succ 0 * ((nbonacciCharacteristicMatrix (n + 1)).submatrix i.succ.succAbove Fin.succ).det) = (-1 : Polynomial ℤ) ^ (n + 1) * (rMatrix (n + 1)).det := by
    change (∑ i : Fin n, characteristicTailSummand i) = _
    rw [htail_single]
    dsimp [characteristicTailSummand]
    have hlast_succ : lastTail.succ = Fin.last n := by
      apply Fin.ext
      simp [lastTail]
      omega
    have hr_cofactor :
        (nbonacciCharacteristicMatrix (n + 1)).submatrix lastTail.succ.succAbove Fin.succ = rMatrix (n + 1) := by
      rw [hlast_succ]
      funext i j
      have hentry := congrArg (fun M => M i j) (nbonacciCharacteristic_minor_r n hn)
      have hrow : (Fin.last n).succAbove i = Ravel.Matrix.EraseIndex.skip (Fin.last n) i := by
        apply Fin.ext
        simp
      have hcol : (⟨j.val + 1, by omega⟩ : Fin (n + 1)) = Ravel.Matrix.EraseIndex.skip (⟨0, by omega⟩ : Fin (n + 1)) j := by
        apply Fin.ext
        simp
      change nbonacciCharacteristicMatrix (n + 1) ((Fin.last n).succAbove i) ⟨j.val + 1, by omega⟩ = rMatrix (n + 1) i j
      exact (congrArg₂ (nbonacciCharacteristicMatrix (n + 1)) hrow hcol).trans hentry
    rw [hr_cofactor]
    have hlastTailVal : lastTail.val = n - 1 := by
      simp [lastTail]
    have hlastEntry : nbonacciCharacteristicMatrix (n + 1) lastTail.succ 0 = Polynomial.C (-1) := by
      rw [hlast_succ]
      have hzero_ne_n : (0 : ℕ) ≠ n := by omega
      simp [nbonacciCharacteristicMatrix, hzero_ne_n]
    rw [hlastEntry]
    rw [hlastTailVal]
    have hexponent : (n - 1) + 1 = n := by omega
    rw [hexponent]
    simp [pow_succ]
    rfl
  rw [htail_sum]

/-- Polynomial closure composed from determinant decomposition and closed component formulas. -/
lemma nbonacci_characteristic_polynomial (n : ℕ) (hn : 2 ≤ n) :
    (nbonacciCharacteristicMatrix (n + 1)).det = nbonacciPolynomial (n + 1) := by
  rw [nbonacci_characteristic_split n hn, qMatrix_det_closed_form (n + 1) (by omega), rMatrix_det (n + 1)]
  simp [nbonacciPolynomial, Finset.mul_sum, Ravel.Polynomial.neg_one_pow_succ_mul_self_int]
  ring_nf

/-- Public universal theorem assembled solely from closed campaign artifacts. -/
lemma nbonacci_universal_n (n : ℕ) (hn : 2 ≤ n) :
    (nbonacciCharacteristicMatrix (n + 1)).det = nbonacciPolynomial (n + 1) := by
  exact nbonacci_characteristic_polynomial n hn

end RavelGenerated
