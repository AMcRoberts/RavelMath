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

inline std::string render_reflective_lean_module(const mathlib::reflection::Trace& trace) {
    if (trace.empty()) throw std::runtime_error("cannot render proof module without provenance");
    std::ostringstream out;
    out << "import Mathlib.LinearAlgebra.Matrix.Determinant.Basic\n";
    out << "import Mathlib.Tactic\n\n";
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
