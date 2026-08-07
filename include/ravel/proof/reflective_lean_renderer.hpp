#pragma once

#include <algorithm>
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

// The second general lemma this renderer's per-instance corollaries
// cite -- hand-derived and kernel-checked once
// (lean/period_rotation_forces_equal_modulus.lean), reproduced here
// verbatim, not re-derived. `M` is kept as an integer matrix (only
// cast to ℂ inside the eigenvector sum) precisely so that per
// instance, the coloring hypothesis reduces to decidable integer
// arithmetic.
inline const char* period_rotation_general_lemma_lean() {
    return
        "/-- THE CORE ALGEBRAIC FACT behind Finding 35 (`g>1` forces the incidence\n"
        "    matrix's Perron-Frobenius period to be `>=2`, contradicting Pisot's\n"
        "    unique-dominant-eigenvalue requirement): a period-`p` integer coloring of\n"
        "    `M`'s support graph rotates any eigenvalue `lam` to `lam * zeta⁻¹` for\n"
        "    every `p`-th root of unity `zeta`. Reproduced from the independently\n"
        "    kernel-checked `lean/period_rotation_forces_equal_modulus.lean` (not\n"
        "    re-derived here). -/\n"
        "theorem period_coloring_rotates_eigenvalue\n"
        "    {n : ℕ} (M : Matrix (Fin n) (Fin n) ℤ) (c : Fin n → ℤ) (p : ℕ) (k : Fin n → Fin n → ℤ)\n"
        "    (v : Fin n → ℂ) (lam : ℂ) (hEig : (fun i => ∑ j, (M i j : ℂ) * v j) = fun i => lam * v i)\n"
        "    (hColor : ∀ i j, M i j ≠ 0 → c i = c j + 1 + k i j * (p : ℤ))\n"
        "    (zeta : ℂ) (hzne : zeta ≠ 0) (hzeta : zeta ^ p = 1) :\n"
        "    (fun i => ∑ j, (M i j : ℂ) * (zeta ^ (c j) * v j))\n"
        "      = fun i => (lam * zeta⁻¹) * (zeta ^ (c i) * v i) := by\n"
        "  funext i\n"
        "  have step : ∀ j : Fin n, (M i j : ℂ) * zeta ^ (c j) = (M i j : ℂ) * (zeta ^ (c i) * zeta⁻¹) := by\n"
        "    intro j\n"
        "    by_cases h : M i j = 0\n"
        "    · simp [h]\n"
        "    · have hk := hColor i j h\n"
        "      have hzp : zeta ^ (p : ℤ) = 1 := by\n"
        "        rw [zpow_natCast]; exact hzeta\n"
        "      have hkp : zeta ^ (k i j * (p : ℤ)) = 1 := by\n"
        "        rw [mul_comm, _root_.zpow_mul, hzp, _root_.one_zpow]\n"
        "      have : zeta ^ (c i) = zeta ^ (c j + 1) := by\n"
        "        rw [hk, zpow_add₀ hzne, hkp, mul_one]\n"
        "      rw [this, zpow_add₀ hzne, zpow_one]\n"
        "      field_simp\n"
        "  have hEigi : (∑ j, (M i j : ℂ) * v j) = lam * v i := by\n"
        "    have h := congrFun hEig i\n"
        "    simpa using h\n"
        "  calc\n"
        "    (∑ j, (M i j : ℂ) * (zeta ^ (c j) * v j))\n"
        "        = ∑ j, ((M i j : ℂ) * zeta ^ (c j)) * v j := by\n"
        "          apply Finset.sum_congr rfl; intro j _; ring\n"
        "    _ = ∑ j, ((M i j : ℂ) * (zeta ^ (c i) * zeta⁻¹)) * v j := by\n"
        "          apply Finset.sum_congr rfl; intro j _; rw [step j]\n"
        "    _ = zeta ^ (c i) * zeta⁻¹ * ∑ j, (M i j : ℂ) * v j := by\n"
        "          rw [Finset.mul_sum]; apply Finset.sum_congr rfl; intro j _; ring\n"
        "    _ = zeta ^ (c i) * zeta⁻¹ * (lam * v i) := by rw [hEigi]\n"
        "    _ = (lam * zeta⁻¹) * (zeta ^ (c i) * v i) := by ring\n\n"
        "/-- A period-`p` rotated eigenvalue (`p >= 2`, `zeta ≠ 1`) is DISTINCT from\n"
        "    the original but has the SAME modulus -- rules out a unique dominant\n"
        "    (Pisot) eigenvalue whenever such a rotation exists. Reproduced from\n"
        "    `lean/period_rotation_forces_equal_modulus.lean`. -/\n"
        "theorem rotated_eigenvalue_has_same_modulus\n"
        "    (lam zeta : ℂ) (p : ℕ) (hp : p ≠ 0) (hzeta : zeta ^ p = 1) (hzne1 : zeta ≠ 1) (hlam : lam ≠ 0) :\n"
        "    lam * zeta⁻¹ ≠ lam ∧ ‖lam * zeta⁻¹‖ = ‖lam‖ := by\n"
        "  have hzne : zeta ≠ 0 := by\n"
        "    rintro rfl\n"
        "    exact absurd hzeta (by simp [zero_pow hp])\n"
        "  have habs1 : ‖zeta‖ = 1 := by\n"
        "    have h1 : ‖zeta‖ ^ p = (1:ℝ) ^ p := by\n"
        "      rw [← norm_pow, hzeta, one_pow]; simp\n"
        "    exact (pow_left_inj₀ (norm_nonneg _) zero_le_one hp).1 h1\n"
        "  refine ⟨?_, ?_⟩\n"
        "  · intro heq\n"
        "    apply hzne1\n"
        "    have hlz : lam * zeta⁻¹ = lam * 1 := by rw [heq, mul_one]\n"
        "    have hz1 : zeta⁻¹ = 1 := mul_left_cancel₀ hlam hlz\n"
        "    exact inv_eq_one.mp hz1\n"
        "  · rw [norm_mul, norm_inv, habs1, inv_one, mul_one]\n\n";
}

// Renders `c : Fin n → ℤ` as an explicit Lean vector literal.
inline std::string render_lean_int_vector(const std::vector<long long>& v) {
    std::ostringstream out;
    out << "(![";
    for (std::size_t i = 0; i < v.size(); ++i) {
        if (i > 0) out << ", ";
        out << v[i];
    }
    out << "] : Fin " << v.size() << " → ℤ)";
    return out.str();
}

// Mechanically emits one Lean corollary PER `PeriodRotationCertificate`
// node found in the trace. The coloring hypothesis is discharged
// entirely by the renderer's own emitted tactic (finite case split +
// `omega`, both decidable over the concrete integer data) -- no
// hand-authored per-instance proof term.
inline std::string render_period_rotation_instances(const mathlib::reflection::Trace& trace) {
    std::ostringstream out;
    long long counter = 0;
    auto nodes = trace.find<mathlib::reflection::PeriodRotationCertificate>();
    if (nodes.empty()) return {};
    out << period_rotation_general_lemma_lean();
    for (const auto& [id, node] : nodes) {
        (void)id;
        std::string name = "period_rotation_instance_" + std::to_string(counter++);
        std::string matrix_lean = render_lean_int_matrix(node->matrix_flat, node->n);
        std::string color_lean = render_lean_int_vector(node->coloring);
        std::string k_lean = render_lean_int_matrix(node->k_flat, node->n);
        std::string M_typed = "(" + matrix_lean + " : Matrix (Fin " + std::to_string(node->n) +
                               ") (Fin " + std::to_string(node->n) + ") ℤ)";
        std::string K_typed = "(" + k_lean + " : Matrix (Fin " + std::to_string(node->n) +
                               ") (Fin " + std::to_string(node->n) + ") ℤ)";
        out << "/-- Mechanically emitted: instantiates the general lemma above for\n";
        out << "    this substitution's own incidence matrix and coloring (" << node->description << "). -/\n";
        out << "theorem " << name << " (v : Fin " << node->n << " → ℂ) (lam : ℂ)\n";
        out << "    (hEig : (fun i => ∑ j, (" << M_typed << " i j : ℂ) * v j) = fun i => lam * v i)\n";
        out << "    (zeta : ℂ) (hzne : zeta ≠ 0) (hzeta : zeta ^ " << node->p << " = 1) :\n";
        out << "    (fun i => ∑ j, (" << M_typed << " i j : ℂ) * (zeta ^ (" << color_lean << " j) * v j))\n";
        out << "      = fun i => (lam * zeta⁻¹) * (zeta ^ (" << color_lean << " i) * v i) :=\n";
        out << "  period_coloring_rotates_eigenvalue " << M_typed << " " << color_lean << " "
            << node->p << " " << K_typed << " v lam hEig\n";
        out << "    (by decide) zeta hzne hzeta\n\n";
    }
    return out.str();
}

// A THIRD general lemma, extracted from the SAME family as the
// previous two -- hand-derived and kernel-checked once, in the same
// file (`lean/period_rotation_forces_equal_modulus.lean`). This is
// the shared mechanism underlying both `period_coloring_rotates_
// eigenvalue` above and Finding 26's own `d + dist(terminal) = 0
// (mod g)` invariant (`coincidence_gcd_obstruction_theorem.hpp`):
// given the SAME kind of coloring, any two walks between the same
// pair of vertices have lengths differing by a multiple of `p`. Its
// per-instance corollaries reuse the EXACT SAME `PeriodRotationCertificate`
// trace nodes `render_period_rotation_instances` already renders --
// no new C++ certificate, no new trace payload, purely a second
// mechanical reading of data already computed once. This is the
// "convert a shared piece of code into a more general lemma" pattern
// applied literally.
inline const char* colored_walk_congruence_general_lemma_lean() {
    return
        "/-- THE SHARED GRAPH FACT underlying both Finding 35 and Finding 26: if a\n"
        "    directed graph `E` on `Fin n` carries an integer coloring `c` that steps by\n"
        "    exactly `1` modulo `p` along every edge, then walking `L` edges from any\n"
        "    start vertex changes the coloring by exactly `L` modulo `p`. Reproduced from\n"
        "    the independently kernel-checked\n"
        "    `lean/period_rotation_forces_equal_modulus.lean` (not re-derived here). -/\n"
        "theorem colored_walk_congruence\n"
        "    {n : ℕ} (E : Fin n → Fin n → Prop) (c : Fin n → ℤ) (p : ℕ) (k : Fin n → Fin n → ℤ)\n"
        "    (hColor : ∀ i j, E i j → c j = c i + 1 + k i j * (p : ℤ))\n"
        "    (w : ℕ → Fin n) (L : ℕ) (hwalk : ∀ i, i < L → E (w i) (w (i + 1))) :\n"
        "    ∃ m : ℤ, c (w L) = c (w 0) + (L : ℤ) + m * (p : ℤ) := by\n"
        "  induction L with\n"
        "  | zero => exact ⟨0, by simp⟩\n"
        "  | succ L ih =>\n"
        "    obtain ⟨m, hm⟩ := ih (fun i hi => hwalk i (Nat.lt_succ_of_lt hi))\n"
        "    have hstep := hColor (w L) (w (L + 1)) (hwalk L (Nat.lt_succ_self L))\n"
        "    refine ⟨m + k (w L) (w (L + 1)), ?_⟩\n"
        "    rw [hstep, hm]\n"
        "    simp only [Nat.cast_add, Nat.cast_one]\n"
        "    ring\n\n"
        "/-- Finding 26's own conclusion, in general graph form: TWO walks between the\n"
        "    SAME pair of vertices must have lengths differing by a multiple of `p`.\n"
        "    Reproduced from `lean/period_rotation_forces_equal_modulus.lean`. -/\n"
        "theorem colored_walk_lengths_agree_mod\n"
        "    {n : ℕ} (E : Fin n → Fin n → Prop) (c : Fin n → ℤ) (p : ℕ) (k : Fin n → Fin n → ℤ)\n"
        "    (hColor : ∀ i j, E i j → c j = c i + 1 + k i j * (p : ℤ))\n"
        "    (w1 w2 : ℕ → Fin n) (L1 L2 : ℕ)\n"
        "    (hwalk1 : ∀ i, i < L1 → E (w1 i) (w1 (i + 1)))\n"
        "    (hwalk2 : ∀ i, i < L2 → E (w2 i) (w2 (i + 1)))\n"
        "    (hstart : w1 0 = w2 0) (hend : w1 L1 = w2 L2) :\n"
        "    ∃ m : ℤ, (L1 : ℤ) - (L2 : ℤ) = m * (p : ℤ) := by\n"
        "  obtain ⟨m1, hm1⟩ := colored_walk_congruence E c p k hColor w1 L1 hwalk1\n"
        "  obtain ⟨m2, hm2⟩ := colored_walk_congruence E c p k hColor w2 L2 hwalk2\n"
        "  rw [hstart, hend] at hm1\n"
        "  refine ⟨m2 - m1, ?_⟩\n"
        "  linear_combination hm2 - hm1\n\n";
}

// Mechanically emits one Lean corollary of `colored_walk_lengths_
// agree_mod` PER `PeriodRotationCertificate` node -- the SAME nodes
// `render_period_rotation_instances` reads, read a second time for a
// different consequence of the identical concrete data.
inline std::string render_colored_walk_congruence_instances(const mathlib::reflection::Trace& trace) {
    std::ostringstream out;
    long long counter = 0;
    auto nodes = trace.find<mathlib::reflection::PeriodRotationCertificate>();
    if (nodes.empty()) return {};
    out << colored_walk_congruence_general_lemma_lean();
    for (const auto& [id, node] : nodes) {
        (void)id;
        std::string name = "walk_congruence_instance_" + std::to_string(counter++);
        std::string matrix_lean = render_lean_int_matrix(node->matrix_flat, node->n);
        std::string color_lean = render_lean_int_vector(node->coloring);
        std::string k_lean = render_lean_int_matrix(node->k_flat, node->n);
        std::string M_typed = "(" + matrix_lean + " : Matrix (Fin " + std::to_string(node->n) +
                               ") (Fin " + std::to_string(node->n) + ") ℤ)";
        std::string K_typed = "(" + k_lean + " : Matrix (Fin " + std::to_string(node->n) +
                               ") (Fin " + std::to_string(node->n) + ") ℤ)";
        // E i j means "walk edge FROM i TO j" (parent -> child): since
        // matrix_flat[i][j] != 0 means i is the CHILD of j (i appears
        // in letter j's image -- the opposite direction), E and the k
        // witness both need their indices swapped relative to the raw
        // matrix/coloring data.
        std::string E_expr = "(fun i j => " + M_typed + " j i ≠ 0)";
        std::string K_expr = "(fun i j => " + K_typed + " j i)";
        out << "/-- Mechanically emitted: Finding 26's own conclusion for this\n";
        out << "    substitution's full-alphabet graph and coloring (" << node->description << "). -/\n";
        out << "theorem " << name << " (w1 w2 : ℕ → Fin " << node->n << ") (L1 L2 : ℕ)\n";
        out << "    (hwalk1 : ∀ i, i < L1 → " << E_expr << " (w1 i) (w1 (i + 1)))\n";
        out << "    (hwalk2 : ∀ i, i < L2 → " << E_expr << " (w2 i) (w2 (i + 1)))\n";
        out << "    (hstart : w1 0 = w2 0) (hend : w1 L1 = w2 L2) :\n";
        out << "    ∃ m : ℤ, (L1 : ℤ) - (L2 : ℤ) = m * " << node->p << " :=\n";
        out << "  colored_walk_lengths_agree_mod " << E_expr << " " << color_lean << " " << node->p
            << " " << K_expr << "\n";
        out << "    (by decide) w1 w2 L1 L2 hwalk1 hwalk2 hstart hend\n\n";
    }
    return out.str();
}

// A FOURTH general lemma, a structurally different shape again
// (combinatorics on lists, not linear algebra) -- Finding 17,
// hand-derived and kernel-checked once
// (lean/constant_first_letter_forces_prefix_coincidence.lean), models
// the PREFIX half of `adelic::pair_has_coincidence`'s own loop
// directly (the `running` accumulator, the recorded (letter,vector)
// pairs) rather than re-deriving an abstract restatement.
inline const char* constant_first_letter_general_lemma_lean() {
    return
        "def runningSeq {d : ℕ} : List (Fin d) → List (Fin d → ℤ)\n"
        "  | [] => []\n"
        "  | (a :: rest) =>\n"
        "      (fun _ => (0 : ℤ)) :: (runningSeq rest).map (fun v => fun j => v j + if j = a then 1 else 0)\n\n"
        "def prefixPairs {d : ℕ} (w : List (Fin d)) : List (Fin d × (Fin d → ℤ)) :=\n"
        "  w.zip (runningSeq w)\n\n"
        "/-- The PREFIX half of `pair_has_coincidence` (see\n"
        "    `lean/constant_first_letter_forces_prefix_coincidence.lean` for the full\n"
        "    correspondence to the C++ loop). -/\n"
        "def hasCoincidencePrefix {d : ℕ} (w1 w2 : List (Fin d)) : Prop :=\n"
        "  ∃ p, p ∈ prefixPairs w1 ∧ p ∈ prefixPairs w2\n\n"
        "/-- Finding 17: if `w1` and `w2` both start with the same letter `c`, the pair\n"
        "    `(c, zeroVector)` is recorded in BOTH lists' `prefixPairs`, so the prefix\n"
        "    coincidence fires immediately, unconditionally. Reproduced from the\n"
        "    independently kernel-checked\n"
        "    `lean/constant_first_letter_forces_prefix_coincidence.lean` (not re-derived\n"
        "    here). -/\n"
        "theorem constant_first_letter_forces_prefix_coincidence\n"
        "    {d : ℕ} (c : Fin d) (w1' w2' : List (Fin d)) :\n"
        "    hasCoincidencePrefix (c :: w1') (c :: w2') := by\n"
        "  refine ⟨(c, fun _ => (0 : ℤ)), ?_, ?_⟩ <;>\n"
        "    simp [prefixPairs, runningSeq]\n\n";
}

// Renders a substitution image (a list of letters, as `long long`
// alphabet indices) as a Lean `List (Fin d)` literal.
inline std::string render_lean_fin_list(const std::vector<long long>& letters) {
    std::ostringstream out;
    out << "[";
    for (std::size_t i = 0; i < letters.size(); ++i) {
        if (i > 0) out << ", ";
        out << letters[i];
    }
    out << "]";
    return out.str();
}

// Mechanically emits one Lean corollary of `constant_first_letter_
// forces_prefix_coincidence` PER PAIR of images in each
// `ConstantFirstLetterCertificate` node -- every image starts with
// the same recorded `constant_letter` by construction, so every pair
// is a genuine instance; the renderer walks all C(d,2) pairs per
// node, using each image's OWN concrete data (not a generic pattern).
inline std::string render_constant_first_letter_instances(const mathlib::reflection::Trace& trace) {
    std::ostringstream out;
    long long counter = 0;
    auto nodes = trace.find<mathlib::reflection::ConstantFirstLetterCertificate>();
    if (nodes.empty()) return {};
    out << constant_first_letter_general_lemma_lean();
    for (const auto& [id, node] : nodes) {
        (void)id;
        for (std::size_t i = 0; i < node->images.size(); ++i) {
            for (std::size_t j = i + 1; j < node->images.size(); ++j) {
                std::string name = "constant_first_letter_instance_" + std::to_string(counter++);
                std::string w1 = render_lean_fin_list(node->images[i]);
                std::string w2 = render_lean_fin_list(node->images[j]);
                std::vector<long long> tail1(node->images[i].begin() + 1, node->images[i].end());
                std::vector<long long> tail2(node->images[j].begin() + 1, node->images[j].end());
                out << "/-- Mechanically emitted: instantiates the general lemma above for\n";
                out << "    this substitution's own images " << i << " and " << j
                    << " (" << node->description << "). -/\n";
                out << "theorem " << name << " :\n";
                out << "    @hasCoincidencePrefix " << node->d << " " << w1 << " " << w2 << " :=\n";
                out << "  constant_first_letter_forces_prefix_coincidence "
                    << "(" << node->constant_letter << " : Fin " << node->d << ") "
                    << render_lean_fin_list(tail1) << " " << render_lean_fin_list(tail2) << "\n\n";
            }
        }
    }
    return out.str();
}

// The exact dual of `constant_first_letter_general_lemma_lean` --
// Finding 38, hand-derived and kernel-checked once
// (lean/constant_last_letter_forces_suffix_coincidence.lean), models
// the SUFFIX half of `pair_has_coincidence`'s own loop directly.
inline const char* constant_last_letter_general_lemma_lean() {
    return
        "def abelianize {d : ℕ} : List (Fin d) → (Fin d → ℤ)\n"
        "  | [] => fun _ => 0\n"
        "  | (a :: rest) => fun j => abelianize rest j + if j = a then 1 else 0\n\n"
        "def sufSeq {d : ℕ} : List (Fin d) → List (Fin d → ℤ)\n"
        "  | [] => []\n"
        "  | (_ :: rest) => abelianize rest :: sufSeq rest\n\n"
        "def suffixPairs {d : ℕ} (w : List (Fin d)) : List (Fin d × (Fin d → ℤ)) :=\n"
        "  w.zip (sufSeq w)\n\n"
        "def hasCoincidenceSuffix {d : ℕ} (w1 w2 : List (Fin d)) : Prop :=\n"
        "  ∃ p, p ∈ suffixPairs w1 ∧ p ∈ suffixPairs w2\n\n"
        "theorem mem_suffixPairs_append_singleton {d : ℕ} (w : List (Fin d)) (c : Fin d) :\n"
        "    (c, fun _ => (0 : ℤ)) ∈ suffixPairs (w ++ [c]) := by\n"
        "  induction w with\n"
        "  | nil => simp [suffixPairs, sufSeq, abelianize]\n"
        "  | cons a w' ih =>\n"
        "      simp only [List.cons_append, suffixPairs, sufSeq, List.zip_cons_cons, List.mem_cons]\n"
        "      exact Or.inr ih\n\n"
        "/-- Finding 38: if `w1` and `w2` both END with the same letter `c`, the pair\n"
        "    `(c, zeroVector)` is recorded in BOTH lists' `suffixPairs`. Reproduced from\n"
        "    the independently kernel-checked\n"
        "    `lean/constant_last_letter_forces_suffix_coincidence.lean` (not re-derived\n"
        "    here). -/\n"
        "theorem constant_last_letter_forces_suffix_coincidence\n"
        "    {d : ℕ} (c : Fin d) (w1' w2' : List (Fin d)) :\n"
        "    hasCoincidenceSuffix (w1' ++ [c]) (w2' ++ [c]) :=\n"
        "  ⟨(c, fun _ => 0), mem_suffixPairs_append_singleton w1' c, mem_suffixPairs_append_singleton w2' c⟩\n\n";
}

// Mechanically emits one Lean corollary of `constant_last_letter_
// forces_suffix_coincidence` PER PAIR of images in each
// `ConstantLastLetterCertificate` node.
inline std::string render_constant_last_letter_instances(const mathlib::reflection::Trace& trace) {
    std::ostringstream out;
    long long counter = 0;
    auto nodes = trace.find<mathlib::reflection::ConstantLastLetterCertificate>();
    if (nodes.empty()) return {};
    out << constant_last_letter_general_lemma_lean();
    for (const auto& [id, node] : nodes) {
        (void)id;
        for (std::size_t i = 0; i < node->images.size(); ++i) {
            for (std::size_t j = i + 1; j < node->images.size(); ++j) {
                std::string name = "constant_last_letter_instance_" + std::to_string(counter++);
                std::string w1 = render_lean_fin_list(node->images[i]);
                std::string w2 = render_lean_fin_list(node->images[j]);
                std::vector<long long> head1(node->images[i].begin(), node->images[i].end() - 1);
                std::vector<long long> head2(node->images[j].begin(), node->images[j].end() - 1);
                out << "/-- Mechanically emitted: instantiates the general lemma above for\n";
                out << "    this substitution's own images " << i << " and " << j
                    << " (" << node->description << "). -/\n";
                out << "theorem " << name << " :\n";
                out << "    @hasCoincidenceSuffix " << node->d << " " << w1 << " " << w2 << " :=\n";
                out << "  constant_last_letter_forces_suffix_coincidence "
                    << "(" << node->constant_letter << " : Fin " << node->d << ") "
                    << render_lean_fin_list(head1) << " " << render_lean_fin_list(head2) << "\n\n";
            }
        }
    }
    return out.str();
}

// A FIFTH general lemma, foundation infrastructure plus the actual
// coincidence consequence for Finding 39/41's "same-chain" special
// case -- hand-derived and kernel-checked once
// (lean/substitution_iteration_infrastructure.lean,
// lean/zero_run_same_chain_coincidence.lean), reproduced here.
inline const char* zero_run_same_chain_general_lemma_lean() {
    return
        "def applyOnce {d : ℕ} (sigma : Fin d → List (Fin d)) (w : List (Fin d)) : List (Fin d) :=\n"
        "  w.flatMap sigma\n\n"
        "def applyN {d : ℕ} (sigma : Fin d → List (Fin d)) (k : ℕ) (w : List (Fin d)) : List (Fin d) :=\n"
        "  (applyOnce sigma)^[k] w\n\n"
        "theorem applyN_succ {d : ℕ} (sigma : Fin d → List (Fin d)) (k : ℕ) (w : List (Fin d)) :\n"
        "    applyN sigma (k + 1) w = applyOnce sigma (applyN sigma k w) := by\n"
        "  simp [applyN, Function.iterate_succ_apply']\n\n"
        "theorem applyOnce_singleton_of_passthrough {d : ℕ} (sigma : Fin d → List (Fin d))\n"
        "    (s s' : Fin d) (h : sigma s = [s']) :\n"
        "    applyOnce sigma [s] = [s'] := by\n"
        "  simp [applyOnce, h]\n\n"
        "theorem applyN_singleton_chain {d : ℕ} (sigma : Fin d → List (Fin d)) (next : Fin d → Fin d)\n"
        "    (hchain : ∀ s, sigma s = [next s]) (s0 : Fin d) (k : ℕ) :\n"
        "    applyN sigma k [s0] = [next^[k] s0] := by\n"
        "  induction k with\n"
        "  | zero => rfl\n"
        "  | succ k ih =>\n"
        "      have hs : sigma (next^[k] s0) = [next^[k + 1] s0] := by\n"
        "        rw [hchain (next^[k] s0)]\n"
        "        congr 1\n"
        "        exact (Function.iterate_succ_apply' next k s0).symm\n"
        "      rw [applyN_succ, ih, applyOnce_singleton_of_passthrough sigma (next^[k] s0) (next^[k + 1] s0) hs]\n\n"
        "def runningSeq {d : ℕ} : List (Fin d) → List (Fin d → ℤ)\n"
        "  | [] => []\n"
        "  | (a :: rest) =>\n"
        "      (fun _ => (0 : ℤ)) :: (runningSeq rest).map (fun v => fun j => v j + if j = a then 1 else 0)\n\n"
        "def prefixPairs {d : ℕ} (w : List (Fin d)) : List (Fin d × (Fin d → ℤ)) :=\n"
        "  w.zip (runningSeq w)\n\n"
        "def hasCoincidencePrefix {d : ℕ} (w1 w2 : List (Fin d)) : Prop :=\n"
        "  ∃ p, p ∈ prefixPairs w1 ∧ p ∈ prefixPairs w2\n\n"
        "theorem constant_first_letter_forces_prefix_coincidence\n"
        "    {d : ℕ} (c : Fin d) (w1' w2' : List (Fin d)) :\n"
        "    hasCoincidencePrefix (c :: w1') (c :: w2') := by\n"
        "  refine ⟨(c, fun _ => (0 : ℤ)), ?_, ?_⟩ <;>\n"
        "    simp [prefixPairs, runningSeq]\n\n"
        "theorem pass_through_chain_synchronizes {d : ℕ} (sigma : Fin d → List (Fin d)) (next : Fin d → Fin d)\n"
        "    (hchain : ∀ s, sigma s = [next s]) (s1 s2 : Fin d) (k1 k2 : ℕ)\n"
        "    (hmeet : next^[k1] s1 = next^[k2] s2) :\n"
        "    applyN sigma k1 [s1] = applyN sigma k2 [s2] := by\n"
        "  rw [applyN_singleton_chain sigma next hchain s1 k1,\n"
        "      applyN_singleton_chain sigma next hchain s2 k2, hmeet]\n\n"
        "theorem identical_words_have_prefix_coincidence {d : ℕ} (c : Fin d) (w' : List (Fin d)) :\n"
        "    hasCoincidencePrefix (c :: w') (c :: w') :=\n"
        "  constant_first_letter_forces_prefix_coincidence c w' w'\n\n"
        "/-- Finding 39/41's \"same-chain\" special case: two letters inside the same\n"
        "    pass-through zero-run, converging onto the same terminal letter at possibly\n"
        "    different depths, exhibit prefix coincidence. Reproduced from the\n"
        "    independently kernel-checked `lean/zero_run_same_chain_coincidence.lean`\n"
        "    (not re-derived here). -/\n"
        "theorem same_chain_forces_coincidence {d : ℕ} (sigma : Fin d → List (Fin d)) (next : Fin d → Fin d)\n"
        "    (hchain : ∀ s, sigma s = [next s]) (s1 s2 : Fin d) (k1 k2 : ℕ)\n"
        "    (hmeet : next^[k1] s1 = next^[k2] s2) :\n"
        "    hasCoincidencePrefix (applyN sigma k1 [s1]) (applyN sigma k2 [s2]) := by\n"
        "  rw [pass_through_chain_synchronizes sigma next hchain s1 s2 k1 k2 hmeet,\n"
        "      applyN_singleton_chain sigma next hchain s2 k2]\n"
        "  exact identical_words_have_prefix_coincidence (next^[k2] s2) []\n\n";
}

// Mechanically emits one Lean corollary of `same_chain_forces_
// coincidence` PER `ZeroRunSameChainCertificate` node -- the local
// chain model (`Fin (R+1)`, `next := offset -> offset+1` truncated
// with a self-loop at R) is built entirely from the node's own `R`;
// `hchain` is trivial (`fun _ => rfl`, since `sigma` is defined AS
// `fun s => [next s]`); `hmeet` is decidable (finite, concrete).
inline std::string render_zero_run_same_chain_instances(const mathlib::reflection::Trace& trace) {
    std::ostringstream out;
    long long counter = 0;
    auto nodes = trace.find<mathlib::reflection::ZeroRunSameChainCertificate>();
    if (nodes.empty()) return {};
    out << zero_run_same_chain_general_lemma_lean();
    for (const auto& [id, node] : nodes) {
        (void)id;
        std::string name = "zero_run_same_chain_instance_" + std::to_string(counter++);
        long long R = node->run_length;
        std::ostringstream nextvec;
        nextvec << "(![";
        for (long long i = 0; i <= R; ++i) {
            if (i > 0) nextvec << ", ";
            nextvec << std::min(i + 1, R);
        }
        nextvec << "] : Fin " << (R + 1) << " → Fin " << (R + 1) << ")";
        std::string next_typed = nextvec.str();
        long long k1 = R - node->s1_offset;
        long long k2 = R - node->s2_offset;
        out << "/-- Mechanically emitted: instantiates the general lemma above for\n";
        out << "    " << node->description << ". -/\n";
        out << "theorem " << name << " :\n";
        out << "    hasCoincidencePrefix\n";
        out << "      (applyN (fun s => [" << next_typed << " s]) " << k1 << " [(" << node->s1_offset
            << " : Fin " << (R + 1) << ")])\n";
        out << "      (applyN (fun s => [" << next_typed << " s]) " << k2 << " [(" << node->s2_offset
            << " : Fin " << (R + 1) << ")]) :=\n";
        out << "  same_chain_forces_coincidence (fun s => [" << next_typed << " s]) " << next_typed
            << " (fun _ => rfl)\n";
        out << "    (" << node->s1_offset << " : Fin " << (R + 1) << ") (" << node->s2_offset
            << " : Fin " << (R + 1) << ") " << k1 << " " << k2 << " (by decide)\n\n";
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
    out << render_period_rotation_instances(trace);
    out << render_colored_walk_congruence_instances(trace);
    out << render_constant_first_letter_instances(trace);
    out << render_constant_last_letter_instances(trace);
    out << render_zero_run_same_chain_instances(trace);

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
