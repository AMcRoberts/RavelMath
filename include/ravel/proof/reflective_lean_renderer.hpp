#pragma once

#include <sstream>
#include <stdexcept>
#include <string>

#include "math/proof_reflection.hpp"

namespace ravel::proof {

inline bool has_r_matrix_proof(const mathlib::reflection::Trace& trace) {
    for (const auto& [id, lemma] : trace.find<mathlib::reflection::LemmaApplication>()) {
        (void)id;
        if (lemma->theorem_name == "Matrix.det_of_lowerTriangular" &&
            lemma->conclusion.find("rMatrix") != std::string::npos) return true;
    }
    return false;
}

// Renders an n x n integer matrix in Lean's `!![a, b; c, d]` notation.
inline std::string render_lean_int_matrix(const std::vector<long long>& flat, long long n) {
    std::ostringstream out;
    out << "!![";
    for (long long i = 0; i < n; ++i) {
        if (i > 0) out << "; ";
        for (long long j = 0; j < n; ++j) {
            if (j > 0) out << ", ";
            out << flat[static_cast<std::size_t>(i * n + j)];
        }
    }
    out << "]";
    return out.str();
}

// The general lemma this renderer's per-instance corollaries cite --
// embedded here (not imported cross-file) so each emitted module is
// self-contained, exactly matching how this same file already embeds
// `rMatrix_det`'s statement below. The lemma itself was independently
// hand-derived and kernel-checked once
// (lean/barge_diamond_lattice_line.lean) -- this string reproduces
// that already-verified text; it is not re-derived by the renderer.
inline const char* barge_diamond_general_lemma_lean() {
    return
        "/-- THE CORE ALGEBRAIC FACT closing Barge & Diamond's contradiction\n"
        "    (Bull. Soc. Math. France 130, 2002, proof of Theorem 1): if an integer\n"
        "    matrix `M` has a nonzero-at-`i` integer-valued eigenvector `w` for a real\n"
        "    eigenvalue `β`, then `β` is rational. Reproduced from the independently\n"
        "    kernel-checked `lean/barge_diamond_lattice_line.lean` (not re-derived\n"
        "    here). -/\n"
        "theorem irrational_eigenvalue_has_no_integer_eigenvector\n"
        "    {n : ℕ} (M : Matrix (Fin n) (Fin n) ℤ) (w : Fin n → ℤ) (β : ℝ)\n"
        "    (hβ : Irrational β) (i : Fin n) (hwi : w i ≠ 0)\n"
        "    (heig : β * (w i : ℝ) = ((M.mulVec w) i : ℝ)) :\n"
        "    False := by\n"
        "  have hwi' : (w i : ℝ) ≠ 0 := Int.cast_ne_zero.mpr hwi\n"
        "  have hrat : β = ((M.mulVec w) i : ℝ) / (w i : ℝ) := by\n"
        "    field_simp\n"
        "    linarith [heig]\n"
        "  exact hβ.ne_rational ((M.mulVec w) i) (w i) hrat\n\n";
}

// Mechanically emits one Lean corollary PER `IntegerEigenvectorNoWitness`
// node found in the trace -- not a single hardcoded pattern, a genuine
// walk over however many such nodes the certificate recorded, each
// instantiated from that node's own concrete matrix data.
inline std::string render_barge_diamond_instances(const mathlib::reflection::Trace& trace) {
    std::ostringstream out;
    long long counter = 0;
    auto nodes = trace.find<mathlib::reflection::IntegerEigenvectorNoWitness>();
    if (nodes.empty()) return {};
    out << barge_diamond_general_lemma_lean();
    for (const auto& [id, node] : nodes) {
        (void)id;
        std::string name = "barge_diamond_instance_" + std::to_string(counter++);
        std::string matrix_lean = render_lean_int_matrix(node->matrix_flat, node->n);
        out << "/-- Mechanically emitted: instantiates the general lemma above for\n";
        out << "    this substitution's own incidence matrix (" << node->charpoly_description << "). -/\n";
        out << "theorem " << name << " (β : ℝ) (hβ : Irrational β)\n";
        out << "    (w : Fin " << node->n << " → ℤ) (i : Fin " << node->n << ") (hwi : w i ≠ 0)\n";
        out << "    (heig : β * (w i : ℝ) = (((" << matrix_lean << ").mulVec w) i : ℝ)) : False :=\n";
        out << "  irrational_eigenvalue_has_no_integer_eigenvector " << matrix_lean << " w β hβ i hwi heig\n\n";
    }
    return out.str();
}

inline std::string render_reflective_lean_module(const mathlib::reflection::Trace& trace) {
    if (trace.empty()) throw std::runtime_error("cannot render proof module without provenance");
    std::ostringstream out;
    out << "import Mathlib\n\n";
    out << "/-! Generated from typed semantic nodes produced inside the exact math library.\n";
    out << "    Concrete observations are comments; only registered, structurally justified\n";
    out << "    lemma applications become theorem declarations. -/\n\n";
    out << "namespace RavelGenerated\n\n";
    out << "open Matrix\n\n";

    if (has_r_matrix_proof(trace)) {
        out << "/-- Symbolic family reflected by `mathlib::nbonacci_r_matrix`. -/\n";
        out << "noncomputable def rMatrix (n : ℕ) :\n";
        out << "    Matrix (Fin (n - 1)) (Fin (n - 1)) (Polynomial ℤ) :=\n";
        out << "  fun i j =>\n";
        out << "    if j.val + 1 = i.val then Polynomial.X\n";
        out << "    else if i.val = j.val then Polynomial.C (-1)\n";
        out << "    else 0\n\n";
        out << "lemma rMatrix_lowerTriangular (n : ℕ) :\n";
        out << "    (rMatrix n).BlockTriangular toDual := by\n";
        out << "  intro i j hij\n";
        out << "  simp only [rMatrix]\n";
        out << "  have hlt : i.val < j.val := by\n";
        out << "    simpa [Fin.lt_iff_val_lt_val] using hij\n";
        out << "  have hsub : j.val + 1 ≠ i.val := by omega\n";
        out << "  have hdiag : i.val ≠ j.val := Nat.ne_of_lt hlt\n";
        out << "  simp [hsub, hdiag]\n\n";
        out << "/-- The library recognized lower triangularity and selected Mathlib's\n";
        out << "    determinant theorem; Lean checks the universal consequence. -/\n";
        out << "theorem rMatrix_det (n : ℕ) :\n";
        out << "    (rMatrix n).det = (Polynomial.C (-1) : Polynomial ℤ) ^ (n - 1) := by\n";
        out << "  rw [Matrix.det_of_lowerTriangular (rMatrix_lowerTriangular n)]\n";
        out << "  simp [rMatrix, Finset.prod_const, Finset.card_univ, Fintype.card_fin]\n\n";
    }

    out << render_barge_diamond_instances(trace);

    out << "/- Semantic proof graph for: " << trace.theorem_id() << "\n";
    for (std::size_t i = 0; i < trace.nodes().size(); ++i) {
        const auto& node = trace.nodes()[i];
        out << "  [" << i << "] " << mathlib::reflection::payload_name(node.payload)
            << " :: " << mathlib::reflection::payload_detail(node.payload) << "\n";
    }
    out << "-/\n\n";
    out << "def reflectedNodeCount : Nat := " << trace.nodes().size() << "\n\n";
    out << "end RavelGenerated\n";
    return out.str();
}

inline std::string render_reflective_lean_skeleton(const mathlib::reflection::Trace& trace) {
    return render_reflective_lean_module(trace);
}

} // namespace ravel::proof
