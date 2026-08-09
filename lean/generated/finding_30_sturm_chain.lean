import Mathlib
import Mathlib.Analysis.Polynomial.SturmCertificate

/-! Generated from typed semantic nodes produced inside the exact math library.
    Concrete observations are comments; only registered, structurally justified
    lemma applications become theorem declarations. -/

namespace RavelGenerated

open Matrix

/-- Mechanically emitted from the exact classifier: plastic polynomial x^3 - x - 1.
    This certifies its isolated real-root count; the classifier's separate
    complex-modulus checks remain computational certificate data. -/
noncomputable def sturm_chain_instance_0_p : Polynomial ℝ := (Polynomial.C ((-1 : ℝ) / (1 : ℝ)) + Polynomial.C ((-1 : ℝ) / (1 : ℝ)) * Polynomial.X ^ 1 + Polynomial.C ((0 : ℝ) / (1 : ℝ)) * Polynomial.X ^ 2 + Polynomial.C ((1 : ℝ) / (1 : ℝ)) * Polynomial.X ^ 3)

noncomputable def sturm_chain_instance_0_s0 : Polynomial ℝ := (Polynomial.C ((-1 : ℝ) / (1 : ℝ)) + Polynomial.C ((-1 : ℝ) / (1 : ℝ)) * Polynomial.X ^ 1 + Polynomial.C ((0 : ℝ) / (1 : ℝ)) * Polynomial.X ^ 2 + Polynomial.C ((1 : ℝ) / (1 : ℝ)) * Polynomial.X ^ 3)

noncomputable def sturm_chain_instance_0_s1 : Polynomial ℝ := (Polynomial.C ((-1 : ℝ) / (1 : ℝ)) + Polynomial.C ((0 : ℝ) / (1 : ℝ)) * Polynomial.X ^ 1 + Polynomial.C ((3 : ℝ) / (1 : ℝ)) * Polynomial.X ^ 2)

noncomputable def sturm_chain_instance_0_s2 : Polynomial ℝ := (Polynomial.C ((1 : ℝ) / (1 : ℝ)) + Polynomial.C ((2 : ℝ) / (3 : ℝ)) * Polynomial.X ^ 1)

noncomputable def sturm_chain_instance_0_s3 : Polynomial ℝ := (Polynomial.C ((-23 : ℝ) / (4 : ℝ)))

noncomputable def sturm_chain_instance_0_q0 : Polynomial ℝ := (Polynomial.C ((0 : ℝ) / (1 : ℝ)) + Polynomial.C ((1 : ℝ) / (3 : ℝ)) * Polynomial.X ^ 1)

noncomputable def sturm_chain_instance_0_q1 : Polynomial ℝ := (Polynomial.C ((-27 : ℝ) / (4 : ℝ)) + Polynomial.C ((9 : ℝ) / (2 : ℝ)) * Polynomial.X ^ 1)

noncomputable def sturm_chain_instance_0_u : Polynomial ℝ := (Polynomial.C ((27 : ℝ) / (4 : ℝ)) + Polynomial.C ((-9 : ℝ) / (2 : ℝ)) * Polynomial.X ^ 1)

noncomputable def sturm_chain_instance_0_v : Polynomial ℝ := (Polynomial.C ((-1 : ℝ) / (1 : ℝ)) + Polynomial.C ((-9 : ℝ) / (4 : ℝ)) * Polynomial.X ^ 1 + Polynomial.C ((3 : ℝ) / (2 : ℝ)) * Polynomial.X ^ 2)

noncomputable def sturm_chain_instance_0_chain : List (Polynomial ℝ) := [sturm_chain_instance_0_s0, sturm_chain_instance_0_s1, sturm_chain_instance_0_s2, sturm_chain_instance_0_s3]

theorem sturm_chain_instance_0_certified :
    Polynomial.CertifiedSturmChain sturm_chain_instance_0_p sturm_chain_instance_0_chain := by
  refine {
    ne_nil := by simp [sturm_chain_instance_0_chain]
    length_ge_two := by simp [sturm_chain_instance_0_chain]
    second_mem := by simp [sturm_chain_instance_0_chain]
    head_eq_p := by simp [sturm_chain_instance_0_chain, sturm_chain_instance_0_p, sturm_chain_instance_0_s0]
    second_eq_derivative := by norm_num [sturm_chain_instance_0_chain, sturm_chain_instance_0_p, sturm_chain_instance_0_s1, map_natCast, map_intCast, Polynomial.C_eq_natCast, Polynomial.C_mul, Polynomial.C_add, Polynomial.C_mul_X_pow_eq_monomial] <;> ring_nf
    recurrence := ?_
    terminal_constant := ?_
    bezout := ?_ }
  · intro i hi
    simp [sturm_chain_instance_0_chain] at hi
    have hi' : i ≤ 1 := by omega
    interval_cases i
    · refine ⟨((1 : ℝ) / (1 : ℝ)), sturm_chain_instance_0_q0, by norm_num, ?_⟩
      norm_num [sturm_chain_instance_0_chain, sturm_chain_instance_0_s0, sturm_chain_instance_0_s1, sturm_chain_instance_0_s2, sturm_chain_instance_0_q0, map_natCast, map_intCast, Polynomial.C_eq_natCast, Polynomial.C_mul, Polynomial.C_add, Polynomial.C_mul_X_pow_eq_monomial] <;> ring_nf
      ring_nf
    · refine ⟨((1 : ℝ) / (1 : ℝ)), sturm_chain_instance_0_q1, by norm_num, ?_⟩
      norm_num [sturm_chain_instance_0_chain, sturm_chain_instance_0_s1, sturm_chain_instance_0_s2, sturm_chain_instance_0_s3, sturm_chain_instance_0_q1, map_natCast, map_intCast, Polynomial.C_eq_natCast, Polynomial.C_mul, Polynomial.C_add, Polynomial.C_mul_X_pow_eq_monomial] <;> ring_nf
      ring_nf
  · refine ⟨((-23 : ℝ) / (4 : ℝ)), by norm_num, ?_⟩
    norm_num [sturm_chain_instance_0_chain, sturm_chain_instance_0_s3, map_natCast, map_intCast]
  · refine ⟨sturm_chain_instance_0_u, sturm_chain_instance_0_v, ((-23 : ℝ) / (4 : ℝ)), by norm_num, ?_⟩
    norm_num [sturm_chain_instance_0_chain, sturm_chain_instance_0_u, sturm_chain_instance_0_v, sturm_chain_instance_0_p, sturm_chain_instance_0_s1, map_natCast, map_intCast, Polynomial.C_eq_natCast, Polynomial.C_mul, Polynomial.C_add, Polynomial.C_mul_X_pow_eq_monomial] <;> ring_nf
    ring_nf

theorem sturm_chain_instance_0_root_count :
    ({x : ℝ | ((728271398757 : ℝ) / (549755813888 : ℝ)) < x ∧ x ≤ ((2913085595029 : ℝ) / (2199023255552 : ℝ)) ∧ sturm_chain_instance_0_p.eval x = 0}.ncard : ℤ) = 1 := by
  have hp : sturm_chain_instance_0_p ≠ 0 := by
    intro h
    have he := congrArg (fun q : Polynomial ℝ => q.eval ((728271398757 : ℝ) / (549755813888 : ℝ))) h
    norm_num [sturm_chain_instance_0_p] at he
  have hcount := sturm_chain_instance_0_certified.count_roots_between hp ((728271398757 : ℝ) / (549755813888 : ℝ)) ((2913085595029 : ℝ) / (2199023255552 : ℝ)) (by norm_num)
  have hsign_lo_0 : SignType.sign (sturm_chain_instance_0_s0.eval ((728271398757 : ℝ) / (549755813888 : ℝ))) = -1 := by norm_num [sturm_chain_instance_0_s0]
  have hsign_lo_1 : SignType.sign (sturm_chain_instance_0_s1.eval ((728271398757 : ℝ) / (549755813888 : ℝ))) = 1 := by norm_num [sturm_chain_instance_0_s1]
  have hsign_lo_2 : SignType.sign (sturm_chain_instance_0_s2.eval ((728271398757 : ℝ) / (549755813888 : ℝ))) = 1 := by norm_num [sturm_chain_instance_0_s2]
  have hsign_lo_3 : SignType.sign (sturm_chain_instance_0_s3.eval ((728271398757 : ℝ) / (549755813888 : ℝ))) = -1 := by norm_num [sturm_chain_instance_0_s3]
  have hsign_hi_0 : SignType.sign (sturm_chain_instance_0_s0.eval ((2913085595029 : ℝ) / (2199023255552 : ℝ))) = 1 := by norm_num [sturm_chain_instance_0_s0]
  have hsign_hi_1 : SignType.sign (sturm_chain_instance_0_s1.eval ((2913085595029 : ℝ) / (2199023255552 : ℝ))) = 1 := by norm_num [sturm_chain_instance_0_s1]
  have hsign_hi_2 : SignType.sign (sturm_chain_instance_0_s2.eval ((2913085595029 : ℝ) / (2199023255552 : ℝ))) = 1 := by norm_num [sturm_chain_instance_0_s2]
  have hsign_hi_3 : SignType.sign (sturm_chain_instance_0_s3.eval ((2913085595029 : ℝ) / (2199023255552 : ℝ))) = -1 := by norm_num [sturm_chain_instance_0_s3]
  have hlo : Polynomial.sturmVariations sturm_chain_instance_0_chain ((728271398757 : ℝ) / (549755813888 : ℝ)) = 2 := by
    simp only [Polynomial.sturmVariations, sturm_chain_instance_0_chain, List.map]
    rw [hsign_lo_0]
    rw [hsign_lo_1]
    rw [hsign_lo_2]
    rw [hsign_lo_3]
    decide
  have hhi : Polynomial.sturmVariations sturm_chain_instance_0_chain ((2913085595029 : ℝ) / (2199023255552 : ℝ)) = 1 := by
    simp only [Polynomial.sturmVariations, sturm_chain_instance_0_chain, List.map]
    rw [hsign_hi_0]
    rw [hsign_hi_1]
    rw [hsign_hi_2]
    rw [hsign_hi_3]
    decide
  rw [hlo, hhi] at hcount
  norm_num at hcount
  exact hcount.symm

/- Semantic proof graph for: finding_30_sturm_chain
  [0] lean.sturm_chain_certificate :: plastic polynomial x^3 - x - 1 -- chain=4 V(lo)=2 V(hi)=1 roots=1 classifier_pisot=true
-/

def reflectedNodeCount : Nat := 1

end RavelGenerated
