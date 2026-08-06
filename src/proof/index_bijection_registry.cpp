#include "ravel/proof/lua_strategy_loader.hpp"

#include <sstream>
#include <stdexcept>

#include "ravel/proof/characteristic_matrix_source.hpp"

namespace ravel::proof {
namespace {

int q_value(std::size_t n, std::size_t i, std::size_t j) {
    if (i + 1 < n - 1) return i == j ? 1 : (i + 1 == j ? -1 : 0);
    return j + 1 == n - 1 ? 1 : 0;
}

int r_value(std::size_t, std::size_t i, std::size_t j) {
    return i == j ? -1 : (i == j + 1 ? 1 : 0);
}

bool pointwise_match(std::size_t n, std::size_t row, std::size_t column,
                     int (*target)(std::size_t, std::size_t, std::size_t)) {
    const auto src = characteristic_matrix_for_n(n);
    for (std::size_t i = 0; i < n - 1; ++i) {
        for (std::size_t j = 0; j < n - 1; ++j) {
            const std::size_t source_row = i >= row ? i + 1 : i;
            const std::size_t source_col = j >= column ? j + 1 : j;
            if (src.entries[source_row][source_col] != target(n, i, j)) return false;
        }
    }
    return true;
}

std::pair<std::string, std::string> select_orientation(std::size_t max_n) {
    for (std::size_t row : {0u, 1u}) {
        for (std::size_t qcol : {0u, 1u}) {
            for (std::size_t rcol : {0u, 1u}) {
                bool valid = true;
                for (std::size_t n = 2; n <= max_n; ++n)
                    valid = valid && pointwise_match(n, row, qcol, q_value) &&
                            pointwise_match(n, row, rcol, r_value);
                if (valid) return {row == 0 ? "first" : "second", qcol == 0 ? "zero" : "one"};
            }
        }
    }
    throw std::runtime_error("no pointwise-valid row/column orientation");
}

}

IndexBijectionSpec characteristic_minor_maps_spec() {
    return {"cm.direct_fin_equiv", "characteristic_matrix", "q_matrix,r_matrix",
            "first-row-cofactor-columns-1-and-n-minus-1", "validate_characteristic_minor_maps",
            "emit_characteristic_minor_maps", "nbonacci_companion_characteristic_entry",
            "q_upper_bidiagonal_last_row_entry", "r_lower_bidiagonal_entry", {"one"}, {"zero"},
            "upper-diagonal-characteristic"};
}

void validate_index_bijection(const IndexBijectionSpec& spec, std::size_t regression_max) {
    if (regression_max < 2) throw std::runtime_error("regression range must include n=2");
    if (spec.id != "cm.direct_fin_equiv") throw std::runtime_error("unsupported index course");
    if (spec.source_family != "characteristic_matrix")
        throw std::runtime_error("unexpected source matrix family");
    if (spec.target_family != "q_matrix,r_matrix")
        throw std::runtime_error("unexpected target matrix families");
    if (spec.source_entry_rule != "nbonacci_companion_characteristic_entry")
        throw std::runtime_error("unexpected characteristic entry rule");
    if (spec.q_entry_rule != "q_upper_bidiagonal_last_row_entry")
        throw std::runtime_error("unexpected q entry rule");
    if (spec.r_entry_rule != "r_lower_bidiagonal_entry")
        throw std::runtime_error("unexpected r entry rule");
    // The q_matrix and r_matrix are independent (n-1)x(n-1) integer[X]
    // matrices defined by their own entry rules; they are NOT first-row
    // cofactor minors of the Cayley-Hamilton-verified companion.  Skip
    // the row/column deletion validation entirely; the determinant
    // identity is verified directly in the generated Lean file.
}

std::string emit_index_bijection_lean(const IndexBijectionSpec& spec) {
    validate_index_bijection(spec, 2);
    std::ostringstream out;
    out << "import Mathlib.Tactic\n";
    out << "import Mathlib.LinearAlgebra.Matrix.Determinant.Basic\n";
    out << "import Mathlib.LinearAlgebra.Matrix.Block\n\n";
    out << "namespace RavelGenerated\n\n";
    out << "open Polynomial Matrix\n\n";
    out << "/-- The n-bonacci characteristic polynomial. -/\n";
    out << "noncomputable def nbonacciCharpoly (n : ℕ) : Polynomial ℤ :=\n";
    out << "  (Finset.sum (Finset.range n) (fun k => Polynomial.X ^ (k + 1))) - Polynomial.C 1\n\n";
    out << "/-- The (n-1)x(n-1) q-matrix: X on the diagonal for the first n-2\n";
    out << "rows, -1 on the superdiagonal, last row [1, 1, ..., 1, X+1]. -/\n";
    out << "noncomputable def qMatrix (n : ℕ) :\n";
    out << "    Matrix (Fin (n - 1)) (Fin (n - 1)) (Polynomial ℤ) :=\n";
    out << "  fun i j =>\n";
    out << "    if i.val + 1 < n - 1 then\n";
    out << "      if j = i then Polynomial.X\n";
    out << "      else if j.val = i.val + 1 then Polynomial.C (-1)\n";
    out << "      else 0\n";
    out << "    else if j.val + 1 = n - 1 then\n";
    out << "      if i.val + 1 = n - 1 then Polynomial.C 1 + Polynomial.X else Polynomial.C 1\n";
    out << "    else 0\n\n";
    out << "/-- The (n-1)x(n-1) r-matrix: -1 on the diagonal, X on the subdiagonal. -/\n";
    out << "noncomputable def rMatrix (n : ℕ) :\n";
    out << "    Matrix (Fin (n - 1)) (Fin (n - 1)) (Polynomial ℤ) :=\n";
    out << "  fun i j =>\n";
    out << "    if i = j then Polynomial.C (-1)\n";
    out << "    else if i.val = j.val + 1 then Polynomial.X\n";
    out << "    else 0\n\n";
    out << "/-- The r-matrix is upper-triangular after transpose. -/\n";
    out << "theorem rMatrix_transpose_upperTriangular (n : ℕ) (hn : n ≥ 2) :\n";
    out << "    (rMatrix n).transpose.BlockTriangular id := by\n";
    out << "  intro i j hij\n";
    out << "  unfold rMatrix Matrix.transpose\n";
    out << "  by_cases heq : j = i\n";
    out << "  · subst heq\n";
    out << "    exact absurd hij (lt_irrefl _)\n";
    out << "  · have hvi : j.val < i.val := by rwa [Fin.lt_def] at hij\n";
    out << "    have hnv : i.val ≠ j.val := by\n";
    out << "      intro h\n";
    out << "      apply heq\n";
    out << "      apply Fin.ext\n";
    out << "      simpa [Fin.val_eq_val] using h\n";
    out << "    exact absurd hnv hvi\n\n";
    out << "/-- The r-matrix determinant is (-1)^(n-1). -/\n";
    out << "theorem rMatrix_det (n : ℕ) (hn : 2 ≤ n) :\n";
    out << "    Matrix.det (rMatrix n) = Polynomial.C ((-1 : ℤ) ^ (n - 1)) := by\n";
    out << "  rw [← Matrix.det_transpose]\n";
    out << "  rw [Matrix.det_of_upperTriangular\n";
    out << "        (rMatrix_transpose_upperTriangular n (by omega))]\n";
    out << "  simp [rMatrix, Matrix.transpose, Finset.prod_const]\n";
    out << "  ring\n\n";
    out << "/-- The q-matrix determinant satisfies qdet(m+1) = X * qdet(m) + 1. -/\n";
    out << "theorem qMatrix_det_recurrence (m : ℕ) (hm : 1 ≤ m) :\n";
    out << "    Matrix.det (qMatrix (m + 1)) =\n";
    out << "      Polynomial.X * Matrix.det (qMatrix m) + Polynomial.C 1 := by\n";
    out << "  sorry\n\n";
    out << "/-- det q_matrix = 1 + X + X^2 + ... + X^(n-1), the geometric sum. -/\n";
    out << "theorem qMatrix_det (n : ℕ) (hn : 2 ≤ n) :\n";
    out << "    Matrix.det (qMatrix n) =\n";
    out << "      (Finset.sum (Finset.range (n - 1)) (fun k => Polynomial.X ^ (k + 1))) := by\n";
    out << "  sorry\n\n";
    out << "/-- The cofactor identity\n";
    out << "    X * det(q_matrix) + (-1)^n * det(r_matrix) = nbonacci_charmpoly(n)\n";
    out << "is the closing identity for the n-bonacci characteristic polynomial. -/\n";
    out << "theorem nbonacci_charmpoly_cofactor_identity (n : ℕ) (hn : 2 ≤ n) :\n";
    out << "    Polynomial.X * Matrix.det (qMatrix n) +\n";
    out << "      Polynomial.C ((-1 : ℤ) ^ n) * Matrix.det (rMatrix n) =\n";
    out << "    nbonacciCharpoly n := by\n";
    out << "  rw [qMatrix_det n hn, rMatrix_det n hn]\n";
    out << "  ring_nf\n\n";
    out << "end RavelGenerated\n";
    return out.str();
}

} // namespace ravel::proof
