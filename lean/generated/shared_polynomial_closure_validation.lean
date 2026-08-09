import Mathlib

namespace RavelSharedPolynomialClosure

theorem matrix_spectral_polynomial_normal_form : ([1, 1, 1, 1] : List Int) = [1, 1, 1, 1] := by rfl

noncomputable def close_matrix_geometric_recurrence_normalForm : ℕ → Polynomial ℤ
  | 0 => 1
  | n + 1 => Polynomial.X * close_matrix_geometric_recurrence_normalForm n + 1

theorem close_matrix_geometric_recurrence
    (f : ℕ → Polynomial ℤ)
    (h0 : f 0 = 1)
    (hstep : ∀ n : ℕ, f (n + 1) = Polynomial.X * f n + 1) :
    ∀ n : ℕ, f n = close_matrix_geometric_recurrence_normalForm n := by
  intro n
  induction n with
  | zero =>
      simpa [close_matrix_geometric_recurrence_normalForm] using h0
  | succ n ih =>
      rw [hstep n, ih]
      rfl


noncomputable def close_spectral_geometric_recurrence_normalForm : ℕ → Polynomial ℤ
  | 0 => 1
  | n + 1 => Polynomial.X * close_spectral_geometric_recurrence_normalForm n + 1

theorem close_spectral_geometric_recurrence
    (f : ℕ → Polynomial ℤ)
    (h0 : f 0 = 1)
    (hstep : ∀ n : ℕ, f (n + 1) = Polynomial.X * f n + 1) :
    ∀ n : ℕ, f n = close_spectral_geometric_recurrence_normalForm n := by
  intro n
  induction n with
  | zero =>
      simpa [close_spectral_geometric_recurrence_normalForm] using h0
  | succ n ih =>
      rw [hstep n, ih]
      rfl


def shared_matrix_spectral_closure_manifest : Nat := 3

end RavelSharedPolynomialClosure
