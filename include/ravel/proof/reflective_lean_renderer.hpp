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

// A SIXTH general lemma, closing Finding 39/41's general case --
// hand-derived and kernel-checked once
// (lean/first_letter_orbit_coincidence.lean), reproduced here.
inline const char* first_letter_orbit_general_lemma_lean() {
    return
        "def applyOnce {d : ℕ} (sigma : Fin d → List (Fin d)) (w : List (Fin d)) : List (Fin d) :=\n"
        "  w.flatMap sigma\n\n"
        "def applyN {d : ℕ} (sigma : Fin d → List (Fin d)) (k : ℕ) (w : List (Fin d)) : List (Fin d) :=\n"
        "  (applyOnce sigma)^[k] w\n\n"
        "theorem applyN_succ {d : ℕ} (sigma : Fin d → List (Fin d)) (k : ℕ) (w : List (Fin d)) :\n"
        "    applyN sigma (k + 1) w = applyOnce sigma (applyN sigma k w) := by\n"
        "  simp [applyN, Function.iterate_succ_apply']\n\n"
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
        "variable {d : ℕ} [Inhabited (Fin d)]\n\n"
        "def firstLetterMap (sigma : Fin d → List (Fin d)) : Fin d → Fin d :=\n"
        "  fun a => (sigma a).headI\n\n"
        "theorem applyOnce_ne_nil (sigma : Fin d → List (Fin d)) (hne : ∀ a, sigma a ≠ [])\n"
        "    (w : List (Fin d)) (hw : w ≠ []) : applyOnce sigma w ≠ [] := by\n"
        "  cases w with\n"
        "  | nil => exact absurd rfl hw\n"
        "  | cons a rest => simp [applyOnce, hne a]\n\n"
        "theorem applyN_ne_nil (sigma : Fin d → List (Fin d)) (hne : ∀ a, sigma a ≠ [])\n"
        "    (k : ℕ) (w : List (Fin d)) (hw : w ≠ []) : applyN sigma k w ≠ [] := by\n"
        "  induction k with\n"
        "  | zero => exact hw\n"
        "  | succ k ih => rw [applyN_succ]; exact applyOnce_ne_nil sigma hne _ ih\n\n"
        "theorem applyOnce_headI (sigma : Fin d → List (Fin d)) (hne : ∀ a, sigma a ≠ [])\n"
        "    (w : List (Fin d)) (hw : w ≠ []) :\n"
        "    (applyOnce sigma w).headI = firstLetterMap sigma w.headI := by\n"
        "  cases w with\n"
        "  | nil => exact absurd rfl hw\n"
        "  | cons a rest =>\n"
        "      obtain ⟨x, xs, hxs⟩ := List.exists_cons_of_ne_nil (hne a)\n"
        "      simp [applyOnce, hxs, firstLetterMap]\n\n"
        "theorem applyN_headI (sigma : Fin d → List (Fin d)) (hne : ∀ a, sigma a ≠ [])\n"
        "    (a : Fin d) (k : ℕ) :\n"
        "    (applyN sigma k [a]).headI = (firstLetterMap sigma)^[k] a := by\n"
        "  induction k with\n"
        "  | zero => rfl\n"
        "  | succ k ih =>\n"
        "      rw [applyN_succ, applyOnce_headI sigma hne _ (applyN_ne_nil sigma hne k [a] (by simp)), ih,\n"
        "        Function.iterate_succ_apply']\n\n"
        "/-- Finding 39/41's general case: if two letters' `firstLetterMap` orbits\n"
        "    collide at depth `K`, their depth-`K` substitution images share a first\n"
        "    letter, hence exhibit prefix coincidence. Reproduced from the\n"
        "    independently kernel-checked `lean/first_letter_orbit_coincidence.lean`\n"
        "    (not re-derived here). -/\n"
        "theorem first_letter_orbit_collision_forces_coincidence\n"
        "    (sigma : Fin d → List (Fin d)) (hne : ∀ a, sigma a ≠ [])\n"
        "    (i j : Fin d) (k : ℕ) (hcollide : (firstLetterMap sigma)^[k] i = (firstLetterMap sigma)^[k] j) :\n"
        "    hasCoincidencePrefix (applyN sigma k [i]) (applyN sigma k [j]) := by\n"
        "  have hi : (applyN sigma k [i]).headI = (firstLetterMap sigma)^[k] i := applyN_headI sigma hne i k\n"
        "  have hj : (applyN sigma k [j]).headI = (firstLetterMap sigma)^[k] j := applyN_headI sigma hne j k\n"
        "  have hine : applyN sigma k [i] ≠ [] := applyN_ne_nil sigma hne k [i] (by simp)\n"
        "  have hjne : applyN sigma k [j] ≠ [] := applyN_ne_nil sigma hne k [j] (by simp)\n"
        "  obtain ⟨a, w1', hw1⟩ := List.exists_cons_of_ne_nil hine\n"
        "  obtain ⟨b, w2', hw2⟩ := List.exists_cons_of_ne_nil hjne\n"
        "  have ha : a = (firstLetterMap sigma)^[k] i := by rw [← hi, hw1]; simp\n"
        "  have hb : b = (firstLetterMap sigma)^[k] j := by rw [← hj, hw2]; simp\n"
        "  have hab : a = b := by rw [ha, hb, hcollide]\n"
        "  rw [hw1, hw2, hab]\n"
        "  exact constant_first_letter_forces_prefix_coincidence b w1' w2'\n\n";
}

// Mechanically emits one Lean corollary of `first_letter_orbit_
// collision_forces_coincidence` PER `FirstLetterOrbitCertificate`
// node. `sigma` is rendered directly from the node's own image data
// (a `Fin d → List (Fin d)` piecewise function via `![...]`); `hne`
// and `hcollide` both discharge via `decide` (finite, concrete).
inline std::string render_first_letter_orbit_instances(const mathlib::reflection::Trace& trace) {
    std::ostringstream out;
    long long counter = 0;
    auto nodes = trace.find<mathlib::reflection::FirstLetterOrbitCertificate>();
    if (nodes.empty()) return {};
    out << first_letter_orbit_general_lemma_lean();
    for (const auto& [id, node] : nodes) {
        (void)id;
        std::string name = "first_letter_orbit_instance_" + std::to_string(counter++);
        std::ostringstream sigmavec;
        sigmavec << "(![";
        for (long long a = 0; a < node->d; ++a) {
            if (a > 0) sigmavec << ", ";
            sigmavec << render_lean_fin_list(node->images[static_cast<std::size_t>(a)]);
        }
        sigmavec << "] : Fin " << node->d << " → List (Fin " << node->d << "))";
        std::string sigma_typed = sigmavec.str();
        out << "/-- Mechanically emitted: instantiates the general lemma above for\n";
        out << "    " << node->description << " (" << node->k << " collision steps). -/\n";
        out << "theorem " << name << " :\n";
        out << "    hasCoincidencePrefix\n";
        out << "      (applyN " << sigma_typed << " " << node->k << " [(" << node->i << " : Fin "
            << node->d << ")])\n";
        out << "      (applyN " << sigma_typed << " " << node->k << " [(" << node->j << " : Fin "
            << node->d << ")]) :=\n";
        out << "  first_letter_orbit_collision_forces_coincidence " << sigma_typed << " (by decide)\n";
        out << "    (" << node->i << " : Fin " << node->d << ") (" << node->j << " : Fin " << node->d
            << ") " << node->k << " (by decide)\n\n";
    }
    return out.str();
}

// The exact dual of `first_letter_orbit_general_lemma_lean` --
// hand-derived and kernel-checked once
// (lean/last_letter_orbit_coincidence.lean), reproduced here.
inline const char* last_letter_orbit_general_lemma_lean() {
    return
        "def applyOnce {d : ℕ} (sigma : Fin d → List (Fin d)) (w : List (Fin d)) : List (Fin d) :=\n"
        "  w.flatMap sigma\n\n"
        "def applyN {d : ℕ} (sigma : Fin d → List (Fin d)) (k : ℕ) (w : List (Fin d)) : List (Fin d) :=\n"
        "  (applyOnce sigma)^[k] w\n\n"
        "theorem applyN_succ {d : ℕ} (sigma : Fin d → List (Fin d)) (k : ℕ) (w : List (Fin d)) :\n"
        "    applyN sigma (k + 1) w = applyOnce sigma (applyN sigma k w) := by\n"
        "  simp [applyN, Function.iterate_succ_apply']\n\n"
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
        "theorem constant_last_letter_forces_suffix_coincidence\n"
        "    {d : ℕ} (c : Fin d) (w1' w2' : List (Fin d)) :\n"
        "    hasCoincidenceSuffix (w1' ++ [c]) (w2' ++ [c]) :=\n"
        "  ⟨(c, fun _ => 0), mem_suffixPairs_append_singleton w1' c, mem_suffixPairs_append_singleton w2' c⟩\n\n"
        "variable {d : ℕ} [Inhabited (Fin d)]\n\n"
        "def lastLetterMap (sigma : Fin d → List (Fin d)) : Fin d → Fin d :=\n"
        "  fun a => (sigma a).getLastI\n\n"
        "theorem applyOnce_ne_nil (sigma : Fin d → List (Fin d)) (hne : ∀ a, sigma a ≠ [])\n"
        "    (w : List (Fin d)) (hw : w ≠ []) : applyOnce sigma w ≠ [] := by\n"
        "  cases w with\n"
        "  | nil => exact absurd rfl hw\n"
        "  | cons a rest => simp [applyOnce, hne a]\n\n"
        "theorem applyN_ne_nil (sigma : Fin d → List (Fin d)) (hne : ∀ a, sigma a ≠ [])\n"
        "    (k : ℕ) (w : List (Fin d)) (hw : w ≠ []) : applyN sigma k w ≠ [] := by\n"
        "  induction k with\n"
        "  | zero => exact hw\n"
        "  | succ k ih => rw [applyN_succ]; exact applyOnce_ne_nil sigma hne _ ih\n\n"
        "theorem applyOnce_getLastI (sigma : Fin d → List (Fin d)) (hne : ∀ a, sigma a ≠ [])\n"
        "    (w : List (Fin d)) (hw : w ≠ []) :\n"
        "    (applyOnce sigma w).getLastI = lastLetterMap sigma w.getLastI := by\n"
        "  induction w using List.reverseRecOn with\n"
        "  | nil => exact absurd rfl hw\n"
        "  | append_singleton l a _ =>\n"
        "      have hlast : (l ++ [a]).getLastI = a := by\n"
        "        rw [List.getLastI_eq_getLast?_getD, List.getLast?_append_of_ne_nil _ (by simp)]\n"
        "        simp\n"
        "      have hstep : (applyOnce sigma l ++ sigma a).getLastI = (sigma a).getLastI := by\n"
        "        rw [List.getLastI_eq_getLast?_getD, List.getLast?_append_of_ne_nil _ (hne a),\n"
        "            ← List.getLastI_eq_getLast?_getD]\n"
        "      simp only [applyOnce, List.flatMap_append, List.flatMap_cons, List.flatMap_nil,\n"
        "        List.append_nil] at hstep ⊢\n"
        "      rw [hstep, hlast]\n"
        "      rfl\n\n"
        "theorem applyN_getLastI (sigma : Fin d → List (Fin d)) (hne : ∀ a, sigma a ≠ [])\n"
        "    (a : Fin d) (k : ℕ) :\n"
        "    (applyN sigma k [a]).getLastI = (lastLetterMap sigma)^[k] a := by\n"
        "  induction k with\n"
        "  | zero => rfl\n"
        "  | succ k ih =>\n"
        "      rw [applyN_succ, applyOnce_getLastI sigma hne _ (applyN_ne_nil sigma hne k [a] (by simp)), ih,\n"
        "        Function.iterate_succ_apply']\n\n"
        "/-- Finding 39/41's suffix-side general case, dual to\n"
        "    `first_letter_orbit_collision_forces_coincidence`. Reproduced from the\n"
        "    independently kernel-checked `lean/last_letter_orbit_coincidence.lean`\n"
        "    (not re-derived here). -/\n"
        "theorem last_letter_orbit_collision_forces_coincidence\n"
        "    (sigma : Fin d → List (Fin d)) (hne : ∀ a, sigma a ≠ [])\n"
        "    (i j : Fin d) (k : ℕ) (hcollide : (lastLetterMap sigma)^[k] i = (lastLetterMap sigma)^[k] j) :\n"
        "    hasCoincidenceSuffix (applyN sigma k [i]) (applyN sigma k [j]) := by\n"
        "  have hi : (applyN sigma k [i]).getLastI = (lastLetterMap sigma)^[k] i := applyN_getLastI sigma hne i k\n"
        "  have hj : (applyN sigma k [j]).getLastI = (lastLetterMap sigma)^[k] j := applyN_getLastI sigma hne j k\n"
        "  have hine : applyN sigma k [i] ≠ [] := applyN_ne_nil sigma hne k [i] (by simp)\n"
        "  have hjne : applyN sigma k [j] ≠ [] := applyN_ne_nil sigma hne k [j] (by simp)\n"
        "  induction hw1 : applyN sigma k [i] using List.reverseRecOn with\n"
        "  | nil => exact absurd hw1 hine\n"
        "  | append_singleton w1' a _ =>\n"
        "      induction hw2 : applyN sigma k [j] using List.reverseRecOn with\n"
        "      | nil => exact absurd hw2 hjne\n"
        "      | append_singleton w2' b _ =>\n"
        "          have ha : a = (lastLetterMap sigma)^[k] i := by\n"
        "            rw [← hi, hw1]; simp [List.getLastI_eq_getLast?_getD]\n"
        "          have hb : b = (lastLetterMap sigma)^[k] j := by\n"
        "            rw [← hj, hw2]; simp [List.getLastI_eq_getLast?_getD]\n"
        "          have hab : a = b := by rw [ha, hb, hcollide]\n"
        "          rw [hab]\n"
        "          exact constant_last_letter_forces_suffix_coincidence b w1' w2'\n\n";
}

// Mechanically emits one Lean corollary of `last_letter_orbit_
// collision_forces_coincidence` PER `LastLetterOrbitCertificate` node.
inline std::string render_last_letter_orbit_instances(const mathlib::reflection::Trace& trace) {
    std::ostringstream out;
    long long counter = 0;
    auto nodes = trace.find<mathlib::reflection::LastLetterOrbitCertificate>();
    if (nodes.empty()) return {};
    out << last_letter_orbit_general_lemma_lean();
    for (const auto& [id, node] : nodes) {
        (void)id;
        std::string name = "last_letter_orbit_instance_" + std::to_string(counter++);
        std::ostringstream sigmavec;
        sigmavec << "(![";
        for (long long a = 0; a < node->d; ++a) {
            if (a > 0) sigmavec << ", ";
            sigmavec << render_lean_fin_list(node->images[static_cast<std::size_t>(a)]);
        }
        sigmavec << "] : Fin " << node->d << " → List (Fin " << node->d << "))";
        std::string sigma_typed = sigmavec.str();
        out << "/-- Mechanically emitted: instantiates the general lemma above for\n";
        out << "    " << node->description << " (" << node->k << " collision steps). -/\n";
        out << "theorem " << name << " :\n";
        out << "    hasCoincidenceSuffix\n";
        out << "      (applyN " << sigma_typed << " " << node->k << " [(" << node->i << " : Fin "
            << node->d << ")])\n";
        out << "      (applyN " << sigma_typed << " " << node->k << " [(" << node->j << " : Fin "
            << node->d << ")]) :=\n";
        out << "  last_letter_orbit_collision_forces_coincidence " << sigma_typed << " (by decide)\n";
        out << "    (" << node->i << " : Fin " << node->d << ") (" << node->j << " : Fin " << node->d
            << ") " << node->k << " (by decide)\n\n";
    }
    return out.str();
}

// Finding 27's general lemma -- hand-derived and kernel-checked once
// (lean/periodic_point_repetition.lean), reproduced here.
inline const char* periodic_point_repetition_lemma_lean() {
    return
        "/-- If `f^[L] x = x`, then `f^[m*L] x = x` for every `m`. Reproduced from the\n"
        "    independently kernel-checked `lean/periodic_point_repetition.lean` (not\n"
        "    re-derived here). -/\n"
        "theorem periodic_point_iterate_mul {α : Type*} (f : α → α) (L : ℕ) (x : α)\n"
        "    (h : f^[L] x = x) : ∀ m : ℕ, f^[L * m] x = x := by\n"
        "  intro m\n"
        "  induction m with\n"
        "  | zero => simp\n"
        "  | succ m ih =>\n"
        "      rw [Nat.mul_succ, Function.iterate_add_apply, h, ih]\n\n";
}

// Mechanically emits one Lean corollary PER `LeftmostLoopCertificate`
// node -- the loop is modeled as a pure rotation on `Fin L` (`f := fun
// i => i + 1`), so `f^[L] 0 = 0` discharges via `decide` (Fin's own
// wraparound addition makes this true by construction, not something
// that needs the original junction-graph data at all).
inline std::string render_leftmost_loop_instances(const mathlib::reflection::Trace& trace) {
    std::ostringstream out;
    long long counter = 0;
    auto nodes = trace.find<mathlib::reflection::LeftmostLoopCertificate>();
    if (nodes.empty()) return {};
    out << periodic_point_repetition_lemma_lean();
    for (const auto& [id, node] : nodes) {
        (void)id;
        long long L = node->loop_length;
        std::string f_typed = "(fun (i : Fin " + std::to_string(L) + ") => i + 1)";
        for (long long m = 0; m <= node->max_m; ++m) {
            std::string name = "leftmost_loop_instance_" + std::to_string(counter++);
            out << "/-- Mechanically emitted: instantiates the general lemma above for\n";
            out << "    " << node->description << ", m=" << m << ". -/\n";
            out << "theorem " << name << " :\n";
            out << "    " << f_typed << "^[" << L << " * " << m << "] (0 : Fin " << L << ") = 0 :=\n";
            out << "  periodic_point_iterate_mul " << f_typed << " " << L << " (0 : Fin " << L
                << ") (by decide) " << m << "\n\n";
        }
    }
    return out.str();
}

// A self-contained excerpt of `lean/class_ii_affine_shells.lean`
// (ClassIINode, ShellKind, shellNode, shellSourceKind, shellHop,
// composeHop, and the three theorems `class_ii_interior_shell` relies
// on) -- reproduced, not re-derived; the full file already kernel-checks
// independently and covers substantially more (window-validity bounds,
// the pre-contact/contact catalogue, etc.) than this excerpt states.
inline const char* shell_propagation_lemma_lean() {
    return
        "structure ClassIINode where\n"
        "  left : Int\n"
        "  x0 : Int\n"
        "  x1 : Int\n"
        "  x2 : Int\n"
        "  right : Int\n"
        "  deriving DecidableEq\n\n"
        "inductive ShellKind\n"
        "  | n00 | n01 | n02 | n03 | n04 | n05 | n06 | n07 | n08 | n09\n"
        "  | n10 | n11 | n12 | n13 | n14 | n15 | n16 | n17 | n18 | n19\n"
        "  deriving DecidableEq, Fintype\n\n"
        "/-- The same twenty affine formulas as\n"
        "    `ravel::class_ii_interior_shell`, with an integer round parameter. -/\n"
        "def shellNode : ShellKind → Int → ClassIINode\n"
        "  | .n00, q => ⟨0, -q,       q,       0, 0⟩\n"
        "  | .n01, q => ⟨0, -q,       q,       0, 1⟩\n"
        "  | .n02, q => ⟨0, -(q - 1), q - 1,  -1, 0⟩\n"
        "  | .n03, q => ⟨0, -(q - 1), q,       0, 0⟩\n"
        "  | .n04, q => ⟨0, -(q - 1), q,       0, 1⟩\n"
        "  | .n05, q => ⟨0, -(q - 2), q - 1,  -1, 0⟩\n"
        "  | .n06, q => ⟨0, q - 2,   -(q - 1), 1, 0⟩\n"
        "  | .n07, q => ⟨0, q - 2,   -(q - 1), 1, 1⟩\n"
        "  | .n08, q => ⟨0, q - 1,   -q,        0, 0⟩\n"
        "  | .n09, q => ⟨0, q - 1,   -(q - 1),  1, 0⟩\n"
        "  | .n10, q => ⟨0, q - 1,   -(q - 1),  1, 1⟩\n"
        "  | .n11, q => ⟨0, q,       -q,        0, 0⟩\n"
        "  | .n12, q => ⟨1, -(q - 1), q - 1,   -1, 0⟩\n"
        "  | .n13, q => ⟨1, -(q - 2), q - 1,   -1, 0⟩\n"
        "  | .n14, q => ⟨1, q - 1,   -q,         0, 0⟩\n"
        "  | .n15, q => ⟨1, q,       -q,         0, 0⟩\n"
        "  | .n16, q => ⟨2, -(q - 1), q - 1,   -1, 0⟩\n"
        "  | .n17, q => ⟨2, -(q - 1), q,       -1, 0⟩\n"
        "  | .n18, q => ⟨2, q - 2,   -(q - 1),  0, 0⟩\n"
        "  | .n19, q => ⟨2, q - 1,   -(q - 1),  0, 0⟩\n\n"
        "/-- A predecessor choice for each interior-shell state. -/\n"
        "def shellSourceKind : ShellKind → ShellKind\n"
        "  | .n00 => .n00 | .n01 => .n00 | .n02 => .n00 | .n03 => .n00\n"
        "  | .n04 => .n03 | .n05 => .n01 | .n06 => .n06 | .n07 => .n08\n"
        "  | .n08 => .n08 | .n09 => .n09 | .n10 => .n11 | .n11 => .n11\n"
        "  | .n12 => .n12 | .n13 => .n12 | .n14 => .n14 | .n15 => .n15\n"
        "  | .n16 => .n16 | .n17 => .n17 | .n18 => .n18 | .n19 => .n19\n\n"
        "/-- Seven distinct signed-hop values occur in this twenty-entry table. -/\n"
        "def shellHop : ShellKind → ClassIINode\n"
        "  | .n00 => ⟨0, -1,  1,  0, 0⟩\n"
        "  | .n01 => ⟨0, -1,  1,  0, 1⟩\n"
        "  | .n02 => ⟨0,  0,  0, -1, 0⟩\n"
        "  | .n03 => ⟨0,  0,  1,  0, 0⟩\n"
        "  | .n04 => ⟨0, -1,  1,  0, 1⟩\n"
        "  | .n05 => ⟨1,  1,  0, -1, 0⟩\n"
        "  | .n06 => ⟨0,  1, -1,  0, 0⟩\n"
        "  | .n07 => ⟨0,  0,  0,  1, 1⟩\n"
        "  | .n08 => ⟨0,  1, -1,  0, 0⟩\n"
        "  | .n09 => ⟨0,  1, -1,  0, 0⟩\n"
        "  | .n10 => ⟨0,  0,  0,  1, 1⟩\n"
        "  | .n11 => ⟨0,  1, -1,  0, 0⟩\n"
        "  | .n12 => ⟨0, -1,  1,  0, 0⟩\n"
        "  | .n13 => ⟨0,  0,  1,  0, 0⟩\n"
        "  | .n14 => ⟨0,  1, -1,  0, 0⟩\n"
        "  | .n15 => ⟨0,  1, -1,  0, 0⟩\n"
        "  | .n16 => ⟨0, -1,  1,  0, 0⟩\n"
        "  | .n17 => ⟨0, -1,  1,  0, 0⟩\n"
        "  | .n18 => ⟨0,  1, -1,  0, 0⟩\n"
        "  | .n19 => ⟨0,  1, -1,  0, 0⟩\n\n"
        "def composeHop (source hop : ClassIINode) : ClassIINode :=\n"
        "  ⟨source.left, source.x0 + hop.x0, source.x1 + hop.x1,\n"
        "    source.x2 + hop.x2, hop.right⟩\n\n"
        "/-- Universal affine corona-candidate propagation: the displayed state in\n"
        "    round `q` is exactly its predecessor in round `q-1` plus its fixed contact\n"
        "    hop -- no finite parameter sweep. Reproduced from the independently\n"
        "    kernel-checked `lean/class_ii_affine_shells.lean` (not re-derived here). -/\n"
        "theorem shellNode_propagates (kind : ShellKind) (q : Int) :\n"
        "    composeHop (shellNode (shellSourceKind kind) (q - 1))\n"
        "      (shellHop kind) = shellNode kind q := by\n"
        "  cases kind <;>\n"
        "    simp [composeHop, shellNode, shellSourceKind, shellHop,\n"
        "      ClassIINode.mk.injEq] <;> omega\n\n"
        "/-- No two entries in one interior shell coincide once `q >= 4`. -/\n"
        "theorem shellNode_injective_at_round {q : Int} (hq : 4 ≤ q) :\n"
        "    Function.Injective (fun kind => shellNode kind q) := by\n"
        "  intro u v h\n"
        "  cases u <;> cases v <;>\n"
        "    simp [shellNode, ClassIINode.mk.injEq] at h ⊢ <;> omega\n\n";
}

// Mechanically emits, PER `ClassIIShellRoundCertificate` node, two
// corollaries at that node's CONCRETE round `q`: `shellNode_propagates`
// instantiated for ALL twenty kinds at once (`∀ kind, ...`, decidable
// since `ShellKind` is a `Fintype` and `q` is concrete -- discharged
// by `decide`, not cited abstractly) and, when `q >= 4`, `shellNode_
// injective_at_round` at that same `q`.
inline std::string render_class_ii_shell_round_instances(const mathlib::reflection::Trace& trace) {
    std::ostringstream out;
    long long counter = 0;
    auto nodes = trace.find<mathlib::reflection::ClassIIShellRoundCertificate>();
    if (nodes.empty()) return {};
    out << shell_propagation_lemma_lean();
    for (const auto& [id, node] : nodes) {
        (void)id;
        long long q = node->q;
        std::string name = "class_ii_shell_round_instance_" + std::to_string(counter++);
        out << "/-- Mechanically emitted: instantiates the general lemma above at the\n";
        out << "    concrete round " << node->description << ". -/\n";
        out << "theorem " << name << " :\n";
        out << "    ∀ kind : ShellKind, composeHop (shellNode (shellSourceKind kind) ("
            << (q - 1) << " : Int)) (shellHop kind) = shellNode kind (" << q << " : Int) := by\n";
        out << "  decide\n\n";
        if (q >= 4) {
            std::string iname = "class_ii_shell_round_injective_instance_" + std::to_string(counter++);
            out << "/-- Mechanically emitted: injectivity at the same concrete round. -/\n";
            out << "theorem " << iname << " :\n";
            out << "    Function.Injective (fun kind : ShellKind => shellNode kind (" << q << " : Int)) := by\n";
            out << "  decide\n\n";
        }
    }
    return out.str();
}

// A self-contained excerpt of `lean/class_ii_affine_shells.lean`
// covering EVERY finite Kind/Node pair `render_class_ii_fixed_table_
// instances` below can check membership against: ContactKind/
// contactNode (14), PreContactKind/preContactNode (16),
// ContactRedExcludedKind/contactRedExcludedNode (2), DContKind/
// dContNode (9), DContFaceCandidateKind/dContFaceCandidateNode (33).
// Reproduced, not re-derived; the full file additionally proves
// window-validity bounds this excerpt doesn't need.
inline const char* class_ii_fixed_tables_lemma_lean() {
    return
        "structure ClassIINodeG where\n"
        "  left : Int\n  x0 : Int\n  x1 : Int\n  x2 : Int\n  right : Int\n"
        "  deriving DecidableEq\n\n"
        "inductive ContactKindG\n"
        "  | c00 | c01 | c02 | c03 | c04 | c05 | c06\n"
        "  | c07 | c08 | c09 | c10 | c11 | c12 | c13\n"
        "  deriving DecidableEq, Fintype\n\n"
        "def contactNodeG : ContactKindG → ClassIINodeG\n"
        "  | .c00 => ⟨0, -1,  1,  1, 1⟩ | .c01 => ⟨0,  0,  0,  0, 1⟩\n"
        "  | .c02 => ⟨0,  0,  0,  0, 2⟩ | .c03 => ⟨0,  0,  0,  1, 0⟩\n"
        "  | .c04 => ⟨0,  0,  0,  1, 1⟩ | .c05 => ⟨0,  0,  1,  0, 0⟩\n"
        "  | .c06 => ⟨0,  1, -1,  0, 0⟩ | .c07 => ⟨1,  0,  0,  0, 2⟩\n"
        "  | .c08 => ⟨1,  1, -1,  0, 0⟩ | .c09 => ⟨1,  1, -1,  0, 2⟩\n"
        "  | .c10 => ⟨1,  1,  0, -1, 0⟩ | .c11 => ⟨2,  0,  1, -1, 0⟩\n"
        "  | .c12 => ⟨2,  0,  1,  0, 0⟩ | .c13 => ⟨2,  1,  0, -1, 0⟩\n\n"
        "inductive PreContactKindG\n"
        "  | p00 | p01 | p02 | p03 | p04 | p05 | p06 | p07\n"
        "  | p08 | p09 | p10 | p11 | p12 | p13 | p14 | p15\n"
        "  deriving DecidableEq, Fintype\n\n"
        "def preContactNodeG : PreContactKindG → ClassIINodeG\n"
        "  | .p00 => ⟨0, -1,  1,  1, 1⟩ | .p01 => ⟨0,  0,  0,  0, 1⟩\n"
        "  | .p02 => ⟨0,  0,  0,  0, 2⟩ | .p03 => ⟨0,  0,  0,  1, 0⟩\n"
        "  | .p04 => ⟨0,  0,  0,  1, 1⟩ | .p05 => ⟨0,  0,  1,  0, 0⟩\n"
        "  | .p06 => ⟨0,  1, -1,  0, 0⟩ | .p07 => ⟨1,  0,  0,  0, 2⟩\n"
        "  | .p08 => ⟨1,  0,  0,  1, 1⟩ | .p09 => ⟨1,  1, -1,  0, 0⟩\n"
        "  | .p10 => ⟨1,  1, -1,  0, 2⟩ | .p11 => ⟨1,  1,  0, -1, 0⟩\n"
        "  | .p12 => ⟨2,  0,  1, -1, 0⟩ | .p13 => ⟨2,  0,  1, -1, 1⟩\n"
        "  | .p14 => ⟨2,  0,  1,  0, 0⟩ | .p15 => ⟨2,  1,  0, -1, 0⟩\n\n"
        "inductive DContKindG\n"
        "  | d00 | d01 | d02 | d03 | d04 | d05 | d06 | d07 | d08\n"
        "  deriving DecidableEq, Fintype\n\n"
        "def dContNodeG : DContKindG → ClassIINodeG\n"
        "  | .d00 => ⟨0, 0,  0,  1, 0⟩ | .d01 => ⟨0, 0,  1,  0, 0⟩\n"
        "  | .d02 => ⟨0, 0,  0,  0, 1⟩ | .d03 => ⟨0, 0,  0,  0, 2⟩\n"
        "  | .d04 => ⟨1, 1, -1,  0, 0⟩ | .d05 => ⟨1, 0,  0,  1, 1⟩\n"
        "  | .d06 => ⟨1, 0,  0,  0, 2⟩ | .d07 => ⟨2, 1,  0, -1, 0⟩\n"
        "  | .d08 => ⟨2, 0,  1, -1, 1⟩\n\n"
        "inductive DContFaceCandidateKindG\n"
        "  | f00 | f01 | f02 | f03 | f04 | f05 | f06 | f07 | f08 | f09\n"
        "  | f10 | f11 | f12 | f13 | f14 | f15 | f16 | f17 | f18 | f19\n"
        "  | f20 | f21 | f22 | f23 | f24 | f25 | f26 | f27 | f28 | f29\n"
        "  | f30 | f31 | f32\n"
        "  deriving DecidableEq, Fintype\n\n"
        "def dContFaceCandidateNodeG : DContFaceCandidateKindG → ClassIINodeG\n"
        "  | .f00 => ⟨0,  0, -1,  0, 0⟩ | .f01 => ⟨0,  0,  0, -1, 0⟩\n"
        "  | .f02 => ⟨0,  0,  0,  1, 0⟩ | .f03 => ⟨0,  0,  1,  0, 0⟩\n"
        "  | .f04 => ⟨0, -1,  0,  0, 1⟩ | .f05 => ⟨0, -1,  1,  0, 1⟩\n"
        "  | .f06 => ⟨0,  0,  0,  0, 1⟩ | .f07 => ⟨0,  0,  1,  0, 1⟩\n"
        "  | .f08 => ⟨0, -1,  0,  0, 2⟩ | .f09 => ⟨0, -1,  0,  1, 2⟩\n"
        "  | .f10 => ⟨0,  0,  0,  0, 2⟩ | .f11 => ⟨0,  0,  0,  1, 2⟩\n"
        "  | .f12 => ⟨1,  0, -1,  0, 0⟩ | .f13 => ⟨1,  1, -1,  0, 0⟩\n"
        "  | .f14 => ⟨1,  1,  0,  0, 0⟩ | .f15 => ⟨1, -1,  0,  0, 1⟩\n"
        "  | .f16 => ⟨1,  0,  0, -1, 1⟩ | .f17 => ⟨1,  0,  0,  1, 1⟩\n"
        "  | .f18 => ⟨1,  1,  0,  0, 1⟩ | .f19 => ⟨1,  0, -1,  0, 2⟩\n"
        "  | .f20 => ⟨1,  0, -1,  1, 2⟩ | .f21 => ⟨1,  0,  0,  0, 2⟩\n"
        "  | .f22 => ⟨1,  0,  0,  1, 2⟩ | .f23 => ⟨2,  0,  0, -1, 0⟩\n"
        "  | .f24 => ⟨2,  1,  0, -1, 0⟩ | .f25 => ⟨2,  1,  0,  0, 0⟩\n"
        "  | .f26 => ⟨2,  0,  0, -1, 1⟩ | .f27 => ⟨2,  0,  1, -1, 1⟩\n"
        "  | .f28 => ⟨2,  0,  1,  0, 1⟩ | .f29 => ⟨2, -1,  0,  0, 2⟩\n"
        "  | .f30 => ⟨2,  0, -1,  0, 2⟩ | .f31 => ⟨2,  0,  1,  0, 2⟩\n"
        "  | .f32 => ⟨2,  1,  0,  0, 2⟩\n\n"
        "inductive Neighbor2FixedKindG\n"
        "  | n00 | n01 | n02 | n03 | n04 | n05 | n06 | n07\n"
        "  | n08 | n09 | n10 | n11 | n12 | n13 | n14 | n15\n"
        "  | n16 | n17 | n18 | n19 | n20 | n21 | n22 | n23\n"
        "  deriving DecidableEq, Fintype\n\n"
        "def neighbor2FixedNodeG : Neighbor2FixedKindG → ClassIINodeG\n"
        "  | .n00 => ⟨0, -2,  2,  1, 1⟩ | .n01 => ⟨0, -1,  0,  1, 0⟩\n"
        "  | .n02 => ⟨0, -1,  1,  1, 0⟩ | .n03 => ⟨0, -1,  1,  1, 2⟩\n"
        "  | .n04 => ⟨0, -1,  2,  1, 0⟩ | .n05 => ⟨0,  1, -2, -1, 0⟩\n"
        "  | .n06 => ⟨0,  1, -1, -1, 0⟩ | .n07 => ⟨0,  1, -1,  0, 2⟩\n"
        "  | .n08 => ⟨0,  1,  0, -1, 0⟩ | .n09 => ⟨1, -1,  1,  0, 1⟩\n"
        "  | .n10 => ⟨1,  0,  1,  0, 0⟩ | .n11 => ⟨1,  1, -2, -1, 0⟩\n"
        "  | .n12 => ⟨1,  1, -1,  0, 1⟩ | .n13 => ⟨1,  2, -2, -1, 0⟩\n"
        "  | .n14 => ⟨1,  2, -1, -1, 0⟩ | .n15 => ⟨2, -2,  2,  0, 1⟩\n"
        "  | .n16 => ⟨2, -1,  1,  0, 0⟩ | .n17 => ⟨2, -1,  1,  0, 2⟩\n"
        "  | .n18 => ⟨2, -1,  2,  0, 0⟩ | .n19 => ⟨2, -1,  2,  0, 1⟩\n"
        "  | .n20 => ⟨2,  1, -2, -1, 0⟩ | .n21 => ⟨2,  1, -1, -1, 0⟩\n"
        "  | .n22 => ⟨2,  1, -1,  0, 2⟩ | .n23 => ⟨2,  2, -1, -1, 0⟩\n\n"
        "inductive FirstBackwardKindG\n"
        "  | b00 | b01 | b02 | b03 | b04 | b05\n"
        "  deriving DecidableEq, Fintype\n\n"
        "def firstBackwardNodeG : FirstBackwardKindG → ClassIINodeG\n"
        "  | .b00 => ⟨1,  1, -1,  0, 2⟩ | .b01 => ⟨2,  0,  1,  0, 0⟩\n"
        "  | .b02 => ⟨0,  0,  0,  1, 1⟩ | .b03 => ⟨2,  0,  1, -1, 0⟩\n"
        "  | .b04 => ⟨0,  1, -1,  0, 0⟩ | .b05 => ⟨0, -1,  1,  1, 1⟩\n\n"
        "def secondBackwardNodeG : ClassIINodeG := ⟨1, 1, 0, -1, 0⟩\n\n";
}

// Renders `[(left,x0,x1,x2,right), ...]` for a concrete node list.
inline std::string render_class_ii_node_list(
    const std::vector<mathlib::reflection::ClassIINodeData>& nodes) {
    std::ostringstream out;
    out << "[";
    for (std::size_t i = 0; i < nodes.size(); ++i) {
        if (i > 0) out << ", ";
        const auto& n = nodes[i];
        out << "(⟨" << n.left << "," << n.x0 << "," << n.x1 << "," << n.x2 << "," << n.right << "⟩ : ClassIINodeG)";
    }
    out << "]";
    return out.str();
}

// Mechanically emits, PER `ClassIIFixedTableCertificate` node, a
// `decide`-checked membership proof: EVERY one of the CONCRETE nodes
// C++ actually constructed is in the corresponding Lean table's
// range. If the C++ table and the Lean table ever diverged, this
// kernel check would legitimately fail -- it is not a citation keyed
// by a name that could silently drift out of sync.
inline std::string render_class_ii_fixed_table_instances(const mathlib::reflection::Trace& trace) {
    std::ostringstream out;
    long long counter = 0;
    auto nodes = trace.find<mathlib::reflection::ClassIIFixedTableCertificate>();
    if (nodes.empty()) return {};
    out << class_ii_fixed_tables_lemma_lean();
    for (const auto& [id, node] : nodes) {
        (void)id;
        std::string kindType, nodeFn;
        if (node->table == "contact") { kindType = "ContactKindG"; nodeFn = "contactNodeG"; }
        else if (node->table == "pre_contact") { kindType = "PreContactKindG"; nodeFn = "preContactNodeG"; }
        else if (node->table == "d_cont") { kindType = "DContKindG"; nodeFn = "dContNodeG"; }
        else if (node->table == "d_cont_face_candidates") { kindType = "DContFaceCandidateKindG"; nodeFn = "dContFaceCandidateNodeG"; }
        else if (node->table == "neighbor2_fixed") { kindType = "Neighbor2FixedKindG"; nodeFn = "neighbor2FixedNodeG"; }
        else if (node->table == "first_backward") { kindType = "FirstBackwardKindG"; nodeFn = "firstBackwardNodeG"; }
        else if (node->table == "second_backward") {
            // A single fixed node, not Kind-indexed: check direct
            // equality against secondBackwardNodeG instead of an
            // existential membership.
            std::string name = "class_ii_fixed_table_instance_" + std::to_string(counter++);
            out << "/-- Mechanically emitted: every node C++ actually built for the \""
                << node->table << "\" table equals secondBackwardNodeG. -/\n";
            out << "theorem " << name << " :\n";
            out << "    ∀ node ∈ " << render_class_ii_node_list(node->nodes) << ", "
                << "node = secondBackwardNodeG := by\n";
            out << "  decide\n\n";
            continue;
        }
        else continue;  // unknown table name (e.g. "round1_raw27") -- handled elsewhere
        std::string name = "class_ii_fixed_table_instance_" + std::to_string(counter++);
        out << "/-- Mechanically emitted: every node C++ actually built for the \""
            << node->table << "\" table is in " << nodeFn << "'s range. -/\n";
        out << "theorem " << name << " :\n";
        out << "    ∀ node ∈ " << render_class_ii_node_list(node->nodes) << ", "
            << "∃ k : " << kindType << ", " << nodeFn << " k = node := by\n";
        out << "  decide\n\n";
    }
    return out.str();
}

// Reuses the ALREADY-RECORDED `ClassIIFixedTableCertificate{table=
// "d_cont"}` data (no new C++ certificate -- same reuse pattern
// Finding 26's `render_colored_walk_congruence_instances` already
// established) for a SECOND consequence: `lean/class_ii_affine_
// shells.lean` also proves `dContNode_in_preContact` (every D_cont
// seed is itself a pre-contact node, for ALL nine DContKind cases).
// This emits, PER "d_cont" node, a `decide`-checked instance of that
// same containment fact against the CONCRETE nodes C++ actually built
// -- not a re-citation of the general theorem's name, a fresh
// decidable check over the same concrete data the fixed-table
// instance above already threaded.
inline std::string render_class_ii_d_cont_in_pre_contact_instances(const mathlib::reflection::Trace& trace) {
    std::ostringstream out;
    long long counter = 0;
    auto nodes = trace.find<mathlib::reflection::ClassIIFixedTableCertificate>();
    bool any = false;
    for (const auto& [id, node] : nodes) { (void)id; if (node->table == "d_cont") { any = true; break; } }
    if (!any) return {};
    for (const auto& [id, node] : nodes) {
        (void)id;
        if (node->table != "d_cont") continue;
        std::string name = "class_ii_d_cont_in_pre_contact_instance_" + std::to_string(counter++);
        out << "/-- Mechanically emitted (reusing the \"d_cont\" fixed-table data\n";
        out << "    above, no new C++ certificate): every one of THESE CONCRETE\n";
        out << "    d_cont nodes is also a pre-contact node -- the same containment\n";
        out << "    `dContNode_in_preContact` proves for ALL DContKind, checked here\n";
        out << "    directly against the concrete data. -/\n";
        out << "theorem " << name << " :\n";
        out << "    ∀ node ∈ " << render_class_ii_node_list(node->nodes) << ", "
            << "∃ pre : PreContactKindG, node = preContactNodeG pre := by\n";
        out << "  decide\n\n";
    }
    return out.str();
}

// A self-contained excerpt of `lean/class_ii_round1_red_pruning.lean`
// covering the 27-state raw pre-Red target list and its length/x2-bound
// facts -- reproduced, not re-derived; the full file additionally
// proves the six-raw-candidate closure this excerpt doesn't need.
inline const char* round1_raw27_lemma_lean() {
    return
        "abbrev NeighborNodeG := ℤ × ℤ × ℤ × ℤ × ℤ\n\n"
        "def round1Raw27G : List NeighborNodeG :=\n"
        "  [ (0,-1, 1, 1,0), (0,-1, 1, 1,2), (0, 1,-1, 0,2), (0, 1,-1, 1,0),\n"
        "    (0, 1, 0,-1,0), (1, 0, 1, 0,0), (1, 1,-1, 0,1), (1, 2,-1,-1,0),\n"
        "    (2,-1, 2, 0,1), (2, 1,-1, 0,0), (2, 1,-1, 0,2),\n"
        "    (0,-1, 1, 1,1), (0, 0, 0, 0,1), (0, 0, 0, 0,2), (0, 0, 0, 1,0),\n"
        "    (0, 0, 0, 1,1), (0, 0, 1, 0,0), (0, 1,-1, 0,0), (1, 0, 0, 0,2),\n"
        "    (1, 1,-1, 0,0), (1, 1,-1, 0,2), (1, 1, 0,-1,0), (2, 0, 1,-1,0),\n"
        "    (2, 0, 1, 0,0), (2, 1, 0,-1,0),\n"
        "    (1, 0, 0, 1,1), (2, 0, 1,-1,1) ]\n\n"
        "/-- Reproduced from the independently kernel-checked\n"
        "    `lean/class_ii_round1_red_pruning.lean` (not re-derived here). -/\n"
        "theorem round1Raw27G_length : round1Raw27G.length = 27 := by decide\n\n"
        "theorem round1Raw27G_x2_bound :\n"
        "    ∀ n ∈ round1Raw27G, n.2.2.2.1 = -1 ∨ n.2.2.2.1 = 0 ∨ n.2.2.2.1 = 1 := by\n"
        "  decide\n\n";
}

// Renders `[(a,b,c,d,e), ...]` (plain tuple syntax, no wrapping
// structure) for a concrete node list, matching `NeighborNodeG`.
inline std::string render_neighbor_node_list(
    const std::vector<mathlib::reflection::ClassIINodeData>& nodes) {
    std::ostringstream out;
    out << "[";
    for (std::size_t i = 0; i < nodes.size(); ++i) {
        if (i > 0) out << ", ";
        const auto& n = nodes[i];
        out << "(" << n.left << "," << n.x0 << "," << n.x1 << "," << n.x2 << "," << n.right << ")";
    }
    out << "]";
    return out.str();
}

// Mechanically emits, PER `ClassIIFixedTableCertificate` node tagged
// "round1_raw27", a `decide`-checked EQUALITY between the CONCRETE
// list C++ actually threaded (via `class_ii_round1_raw27_targets()`,
// shared with `app/class_ii_neighbor2_round1_red_forward_check.cpp`)
// and the Lean file's own `round1Raw27G` list -- the strongest form of
// this check (not just membership): if the two lists ever diverged in
// content OR order, this kernel check would legitimately fail.
inline std::string render_round1_raw27_instances(const mathlib::reflection::Trace& trace) {
    std::ostringstream out;
    long long counter = 0;
    auto nodes = trace.find<mathlib::reflection::ClassIIFixedTableCertificate>();
    bool any = false;
    for (const auto& [id, node] : nodes) {
        (void)id;
        if (node->table == "round1_raw27") { any = true; break; }
    }
    if (!any) return {};
    out << round1_raw27_lemma_lean();
    for (const auto& [id, node] : nodes) {
        (void)id;
        if (node->table != "round1_raw27") continue;
        std::string name = "round1_raw27_instance_" + std::to_string(counter++);
        out << "/-- Mechanically emitted: the concrete 27-node list C++ actually\n";
        out << "    built equals `round1Raw27G` exactly (content and order). -/\n";
        out << "theorem " << name << " :\n";
        out << "    (" << render_neighbor_node_list(node->nodes) << " : List NeighborNodeG) = round1Raw27G := by\n";
        out << "  decide\n\n";
    }
    return out.str();
}

// A self-contained excerpt of `lean/class_ii_terminal_shells.lean`
// covering the terminal cross-colour edit's distinctness facts --
// reproduced, not re-derived; the full file additionally proves
// interior-shell avoidance this excerpt doesn't need.
inline const char* class_ii_terminal_shell_lemma_lean() {
    return
        "structure TermNodeG where\n"
        "  left : Int\n  x0 : Int\n  x1 : Int\n  x2 : Int\n  right : Int\n"
        "  deriving DecidableEq\n\n"
        "def terminalCrossColour1G (a : Int) : TermNodeG := ⟨0, a - 1, -a, 1, 2⟩\n"
        "def terminalCrossColour2G (a : Int) : TermNodeG := ⟨2, -(a - 1), a - 1, -1, 1⟩\n"
        "def interiorExtreme00G (a : Int) : TermNodeG := ⟨0, -a, a, 0, 0⟩\n"
        "def interiorExtreme11G (a : Int) : TermNodeG := ⟨0, a, -a, 0, 0⟩\n\n"
        "/-- For every integer a, none of the four (cross-colour, interior-extreme)\n"
        "    coincidences hold -- the terminal edit never double-counts. Reproduced\n"
        "    from the independently kernel-checked\n"
        "    `lean/class_ii_terminal_shells.lean` (not re-derived here). -/\n"
        "theorem terminalCrossColours_not_eq_interior_extremesG (a : Int) :\n"
        "    terminalCrossColour1G a ≠ interiorExtreme00G a ∧\n"
        "      terminalCrossColour1G a ≠ interiorExtreme11G a ∧\n"
        "      terminalCrossColour2G a ≠ interiorExtreme00G a ∧\n"
        "      terminalCrossColour2G a ≠ interiorExtreme11G a := by\n"
        "  refine ⟨?_, ?_, ?_, ?_⟩ <;> intro h <;>\n"
        "    simp [terminalCrossColour1G, terminalCrossColour2G, interiorExtreme00G,\n"
        "      interiorExtreme11G, TermNodeG.mk.injEq] at h\n\n";
}

// Mechanically emits, PER `ClassIITerminalShellCertificate` node, a
// `decide`-checked instantiation of the distinctness fact at that
// CONCRETE `a`.
inline std::string render_class_ii_terminal_shell_instances(const mathlib::reflection::Trace& trace) {
    std::ostringstream out;
    long long counter = 0;
    auto nodes = trace.find<mathlib::reflection::ClassIITerminalShellCertificate>();
    if (nodes.empty()) return {};
    out << class_ii_terminal_shell_lemma_lean();
    for (const auto& [id, node] : nodes) {
        (void)id;
        std::string name = "class_ii_terminal_shell_instance_" + std::to_string(counter++);
        out << "/-- Mechanically emitted: instantiates the general lemma above at a="
            << node->a << ". -/\n";
        out << "theorem " << name << " :\n";
        out << "    terminalCrossColour1G (" << node->a << " : Int) ≠ interiorExtreme00G (" << node->a << " : Int) ∧\n";
        out << "      terminalCrossColour1G (" << node->a << " : Int) ≠ interiorExtreme11G (" << node->a << " : Int) ∧\n";
        out << "      terminalCrossColour2G (" << node->a << " : Int) ≠ interiorExtreme00G (" << node->a << " : Int) ∧\n";
        out << "      terminalCrossColour2G (" << node->a << " : Int) ≠ interiorExtreme11G (" << node->a << " : Int) :=\n";
        out << "  terminalCrossColours_not_eq_interior_extremesG (" << node->a << " : Int)\n\n";
    }
    return out.str();
}

// Finding 32's general lemma, hand-derived and kernel-checked once
// (lean/depressed_cubic_complex_pair_modulus.lean) -- generalized (NOT
// specific to sigma_{0,2}): for a positive real root of x^3+c*x+d,
// its complex pair's modulus^2 exceeds 1 IFF beta < -d.
inline const char* depressed_cubic_general_lemma_lean() {
    return
        "theorem depressed_cubic_factors {c d beta : ℝ} (hroot : beta^3 + c * beta + d = 0) :\n"
        "    ∀ x : ℝ, x^3 + c * x + d = (x - beta) * (x^2 + beta * x + (beta^2 + c)) := by\n"
        "  intro x\n"
        "  have hd : d = -beta^3 - c * beta := by linarith\n"
        "  rw [hd]\n"
        "  ring\n\n"
        "theorem quadratic_complex_pair_modulus_sq {p q : ℝ} (hdisc : p^2 < 4 * q) :\n"
        "    ∃ z : ℂ, z^2 + (p:ℂ) * z + (q:ℂ) = 0 ∧ Complex.normSq z = q := by\n"
        "  have hs : (Real.sqrt (4 * q - p^2) : ℝ) ^ 2 = 4 * q - p^2 :=\n"
        "    Real.sq_sqrt (by linarith)\n"
        "  set s : ℝ := Real.sqrt (4 * q - p^2) with hsdef\n"
        "  refine ⟨Complex.mk (-p / 2) (s / 2), ?_, ?_⟩\n"
        "  · apply Complex.ext\n"
        "    · simp [Complex.mul_re, Complex.add_re, pow_two]\n"
        "      nlinarith [hs]\n"
        "    · simp [Complex.mul_im, Complex.add_im, pow_two]\n"
        "      ring\n"
        "  · rw [Complex.normSq_mk]\n"
        "    nlinarith [hs]\n\n"
        "/-- THE GENERAL FACT: for a depressed cubic `x^3+c*x+d` with positive real\n"
        "    root `beta`, the complex pair's modulus^2 `beta^2+c` exceeds `1` IFF\n"
        "    `beta < -d` -- independent of `c`. Reproduced from the independently\n"
        "    kernel-checked `lean/depressed_cubic_complex_pair_modulus.lean` (not\n"
        "    re-derived here). -/\n"
        "theorem depressed_cubic_q_gt_one_iff_beta_lt_neg_d {c d beta : ℝ}\n"
        "    (hpos : 0 < beta) (hroot : beta^3 + c * beta + d = 0) :\n"
        "    beta^2 + c > 1 ↔ beta < -d := by\n"
        "  constructor\n"
        "  · intro hq; nlinarith [hroot, hpos, hq]\n"
        "  · intro hb; nlinarith [hroot, hpos, hb]\n\n";
}

// True iff the trace has any `DepressedCubicNotPisotCertificate` node.
inline bool has_depressed_cubic_nodes(const mathlib::reflection::Trace& trace) {
    return !trace.find<mathlib::reflection::DepressedCubicNotPisotCertificate>().empty();
}

// Mechanically emits one Lean corollary PER `DepressedCubicNotPisotCertificate`
// node -- concrete `c`, `d`, and bracket `(lo,hi)`; `hi <= -d` is
// decidable (concrete integers); the conclusion is CONDITIONAL on an
// arbitrary `beta` satisfying the bracket and root equation (this
// file never constructs beta, matching the general lemma's own
// scope -- existence of such a beta is a separate, un-formalized-here
// fact, same honesty boundary as Finding 42's Barge-Diamond).
inline std::string render_depressed_cubic_instances(const mathlib::reflection::Trace& trace) {
    std::ostringstream out;
    long long counter = 0;
    auto nodes = trace.find<mathlib::reflection::DepressedCubicNotPisotCertificate>();
    if (nodes.empty()) return {};
    out << depressed_cubic_general_lemma_lean();
    for (const auto& [id, node] : nodes) {
        (void)id;
        std::string name = "depressed_cubic_instance_" + std::to_string(counter++);
        out << "/-- Mechanically emitted: instantiates the general lemma above for\n";
        out << "    " << node->description << ". -/\n";
        out << "theorem " << name << " {beta : ℝ} (hpos : 0 < beta)\n";
        out << "    (hlo : (" << node->lo << " : ℝ) < beta) (hhi : beta < (" << node->hi << " : ℝ))\n";
        out << "    (hroot : beta^3 + (" << node->c << " : ℝ) * beta + (" << node->d << " : ℝ) = 0) :\n";
        out << "    beta^2 + (" << node->c << " : ℝ) > 1 := by\n";
        out << "  have hbound : (" << node->hi << " : ℝ) ≤ -(" << node->d << " : ℝ) := by norm_num\n";
        out << "  have hb : beta < -(" << node->d << " : ℝ) := by linarith\n";
        out << "  exact (depressed_cubic_q_gt_one_iff_beta_lt_neg_d hpos hroot).mpr hb\n\n";
    }
    return out.str();
}

// A self-contained excerpt of `lean/class_ii_neighbor_d_support.lean`
// covering the three neighbors' boundary-layer source/target index
// sets -- reproduced, not re-derived; the full file additionally
// proves the affine closed-form identity this excerpt doesn't need.
inline const char* class_ii_neighbor_d_support_lemma_lean() {
    return
        "structure AffineEdgeG where\n"
        "  source : Nat\n  target : Nat\n  intercept : Int\n  slope : Int\n"
        "  deriving DecidableEq\n\n"
        "def neighbor0EdgeListG : List AffineEdgeG :=\n"
        "  [⟨0, 4, 1, 0⟩, ⟨0, 5, 1, 0⟩,\n"
        "   ⟨1, 9, -2, 1⟩, ⟨1, 10, -3, 1⟩, ⟨1, 12, -2, 1⟩, ⟨1, 14, 1, 0⟩,\n"
        "   ⟨2, 9, 1, 0⟩, ⟨2, 10, 1, 0⟩, ⟨2, 12, 1, 0⟩,\n"
        "   ⟨3, 6, 1, 0⟩, ⟨3, 9, 1, 0⟩,\n"
        "   ⟨4, 0, -2, 1⟩, ⟨4, 1, 0, 1⟩, ⟨4, 2, -1, 1⟩, ⟨4, 3, 1, 0⟩,\n"
        "   ⟨4, 11, 1, 0⟩, ⟨5, 7, 1, 0⟩, ⟨5, 13, 1, 0⟩,\n"
        "   ⟨6, 1, 1, 0⟩, ⟨6, 11, 1, 0⟩, ⟨7, 5, 1, 0⟩,\n"
        "   ⟨8, 6, 1, 0⟩, ⟨8, 9, 0, 1⟩, ⟨8, 10, -2, 1⟩, ⟨8, 12, -1, 1⟩,\n"
        "   ⟨8, 14, 1, 0⟩,\n"
        "   ⟨9, 0, -3, 1⟩, ⟨9, 1, -2, 1⟩, ⟨9, 2, -2, 1⟩, ⟨9, 3, 1, 0⟩,\n"
        "   ⟨10, 7, 1, 0⟩, ⟨10, 8, 1, 0⟩, ⟨11, 6, 1, 0⟩, ⟨11, 9, 1, 0⟩,\n"
        "   ⟨12, 0, 1, 0⟩, ⟨12, 1, 1, 0⟩, ⟨12, 2, 1, 0⟩,\n"
        "   ⟨13, 4, 1, 0⟩, ⟨13, 5, 1, 0⟩, ⟨14, 1, 1, 0⟩, ⟨14, 11, 1, 0⟩]\n\n"
        "def neighbor1EdgeListG : List AffineEdgeG :=\n"
        "  [⟨0, 9, 1, 0⟩, ⟨0, 12, 1, 0⟩,\n"
        "   ⟨1, 9, -1, 1⟩, ⟨1, 12, -2, 1⟩, ⟨1, 14, 1, 0⟩,\n"
        "   ⟨2, 15, 1, 0⟩, ⟨3, 9, 1, 0⟩, ⟨3, 12, 1, 0⟩,\n"
        "   ⟨4, 15, 1, 0⟩, ⟨4, 16, 1, 0⟩, ⟨5, 10, 1, 0⟩,\n"
        "   ⟨6, 7, 1, 0⟩, ⟨6, 11, 1, 0⟩, ⟨7, 8, 1, 0⟩, ⟨7, 13, 1, 0⟩,\n"
        "   ⟨8, 12, 1, 0⟩,\n"
        "   ⟨9, 0, -2, 1⟩, ⟨9, 1, -1, 1⟩, ⟨9, 2, -1, 1⟩, ⟨9, 3, 1, 0⟩,\n"
        "   ⟨9, 4, 1, 0⟩, ⟨10, 0, 1, 0⟩, ⟨11, 5, 1, 0⟩, ⟨11, 6, 1, 0⟩,\n"
        "   ⟨12, 0, 1, 0⟩, ⟨12, 1, 1, 0⟩, ⟨12, 2, 1, 0⟩, ⟨13, 9, 1, 0⟩,\n"
        "   ⟨14, 0, 1, 0⟩, ⟨14, 1, 1, 0⟩, ⟨14, 2, 1, 0⟩, ⟨15, 10, 1, 0⟩,\n"
        "   ⟨16, 7, 1, 0⟩, ⟨16, 11, 1, 0⟩]\n\n"
        "def neighbor2EdgeListG : List AffineEdgeG :=\n"
        "  [⟨0, 21, 1, 0⟩, ⟨1, 22, 1, 0⟩, ⟨1, 34, 1, 0⟩, ⟨2, 26, 1, 0⟩,\n"
        "   ⟨3, 15, 0, 1⟩, ⟨3, 16, -1, 1⟩, ⟨3, 18, -2, 1⟩, ⟨3, 24, -1, 1⟩,\n"
        "   ⟨3, 25, -1, 1⟩, ⟨3, 26, -2, 1⟩, ⟨3, 36, 1, 0⟩,\n"
        "   ⟨4, 16, 1, 0⟩, ⟨4, 18, 1, 0⟩, ⟨5, 26, 1, 0⟩, ⟨6, 24, 1, 0⟩,\n"
        "   ⟨7, 19, 1, 0⟩, ⟨7, 25, 1, 0⟩, ⟨7, 28, 1, 0⟩, ⟨7, 38, 1, 0⟩,\n"
        "   ⟨8, 29, 1, 0⟩,\n"
        "   ⟨9, 2, -1, 1⟩, ⟨9, 3, 0, 1⟩, ⟨9, 4, -1, 1⟩, ⟨9, 5, 1, 0⟩,\n"
        "   ⟨9, 6, -1, 1⟩, ⟨9, 7, -2, 1⟩, ⟨9, 20, 0, 1⟩, ⟨9, 30, 1, 0⟩,\n"
        "   ⟨9, 31, 1, 0⟩, ⟨9, 32, 1, 0⟩,\n"
        "   ⟨10, 11, 1, 0⟩, ⟨10, 13, 1, 0⟩, ⟨11, 12, 1, 0⟩,\n"
        "   ⟨12, 21, 1, 0⟩, ⟨12, 22, 1, 0⟩, ⟨12, 34, 1, 0⟩,\n"
        "   ⟨13, 14, 1, 0⟩, ⟨13, 23, 1, 0⟩, ⟨13, 35, 1, 0⟩,\n"
        "   ⟨14, 15, -1, 1⟩, ⟨14, 16, 0, 1⟩, ⟨14, 17, 1, 0⟩,\n"
        "   ⟨14, 18, -1, 1⟩, ⟨14, 24, -2, 1⟩, ⟨14, 25, -1, 1⟩,\n"
        "   ⟨14, 26, 0, 1⟩, ⟨14, 27, 1, 0⟩, ⟨14, 36, 1, 0⟩, ⟨14, 37, 1, 0⟩,\n"
        "   ⟨15, 7, 1, 0⟩,\n"
        "   ⟨16, 2, -2, 1⟩, ⟨16, 3, -1, 1⟩, ⟨16, 4, -1, 1⟩, ⟨16, 5, 1, 0⟩,\n"
        "   ⟨16, 6, 0, 1⟩, ⟨16, 7, -1, 1⟩, ⟨16, 8, 1, 0⟩,\n"
        "   ⟨16, 20, -2, 1⟩, ⟨16, 33, 1, 0⟩,\n"
        "   ⟨17, 7, 1, 0⟩, ⟨18, 20, 1, 0⟩, ⟨19, 13, 1, 0⟩,\n"
        "   ⟨20, 15, 1, 0⟩, ⟨21, 9, 1, 0⟩, ⟨22, 10, 1, 0⟩, ⟨23, 15, 1, 0⟩,\n"
        "   ⟨24, 0, 1, 0⟩, ⟨24, 1, 1, 0⟩, ⟨24, 4, 1, 0⟩,\n"
        "   ⟨25, 2, 1, 0⟩, ⟨25, 3, 1, 0⟩, ⟨26, 6, 1, 0⟩,\n"
        "   ⟨27, 4, 1, 0⟩, ⟨27, 12, 1, 0⟩, ⟨28, 11, 1, 0⟩, ⟨29, 7, 1, 0⟩,\n"
        "   ⟨30, 24, 1, 0⟩, ⟨31, 10, 1, 0⟩, ⟨31, 25, 1, 0⟩,\n"
        "   ⟨32, 15, 1, 0⟩, ⟨33, 15, 1, 0⟩, ⟨34, 9, 1, 0⟩, ⟨34, 26, 1, 0⟩,\n"
        "   ⟨35, 10, 1, 0⟩, ⟨35, 24, 1, 0⟩, ⟨35, 25, 1, 0⟩,\n"
        "   ⟨36, 20, 1, 0⟩, ⟨37, 6, 1, 0⟩,\n"
        "   ⟨38, 0, 1, 0⟩, ⟨38, 1, 1, 0⟩, ⟨38, 4, 1, 0⟩]\n\n"
        "def neighbor0EdgesG : Finset AffineEdgeG := neighbor0EdgeListG.toFinset\n"
        "def neighbor1EdgesG : Finset AffineEdgeG := neighbor1EdgeListG.toFinset\n"
        "def neighbor2EdgesG : Finset AffineEdgeG := neighbor2EdgeListG.toFinset\n\n"
        "def neighbor0BoundarySourceG : Finset Nat :=\n"
        "  ({e ∈ neighbor0EdgesG | e.slope ≠ 0} : Finset AffineEdgeG).image AffineEdgeG.source\n"
        "def neighbor0BoundaryTargetG : Finset Nat :=\n"
        "  ({e ∈ neighbor0EdgesG | e.slope ≠ 0} : Finset AffineEdgeG).image AffineEdgeG.target\n"
        "def neighbor1BoundarySourceG : Finset Nat :=\n"
        "  ({e ∈ neighbor1EdgesG | e.slope ≠ 0} : Finset AffineEdgeG).image AffineEdgeG.source\n"
        "def neighbor1BoundaryTargetG : Finset Nat :=\n"
        "  ({e ∈ neighbor1EdgesG | e.slope ≠ 0} : Finset AffineEdgeG).image AffineEdgeG.target\n"
        "def neighbor2BoundarySourceG : Finset Nat :=\n"
        "  ({e ∈ neighbor2EdgesG | e.slope ≠ 0} : Finset AffineEdgeG).image AffineEdgeG.source\n"
        "def neighbor2BoundaryTargetG : Finset Nat :=\n"
        "  ({e ∈ neighbor2EdgesG | e.slope ≠ 0} : Finset AffineEdgeG).image AffineEdgeG.target\n\n"
        "/-- Reproduced from the independently kernel-checked\n"
        "    `lean/class_ii_neighbor_d_support.lean` (not re-derived here). -/\n"
        "theorem neighbor0BoundarySourceG_eq : neighbor0BoundarySourceG = {1, 4, 8, 9} := by\n"
        "  unfold neighbor0BoundarySourceG neighbor0EdgesG neighbor0EdgeListG; native_decide\n"
        "theorem neighbor0BoundaryTargetG_eq : neighbor0BoundaryTargetG = {0, 1, 2, 9, 10, 12} := by\n"
        "  unfold neighbor0BoundaryTargetG neighbor0EdgesG neighbor0EdgeListG; native_decide\n"
        "theorem neighbor1BoundarySourceG_eq : neighbor1BoundarySourceG = {1, 9} := by\n"
        "  unfold neighbor1BoundarySourceG neighbor1EdgesG neighbor1EdgeListG; native_decide\n"
        "theorem neighbor1BoundaryTargetG_eq : neighbor1BoundaryTargetG = {0, 1, 2, 9, 12} := by\n"
        "  unfold neighbor1BoundaryTargetG neighbor1EdgesG neighbor1EdgeListG; native_decide\n"
        "theorem neighbor2BoundarySourceG_eq : neighbor2BoundarySourceG = {3, 9, 14, 16} := by\n"
        "  unfold neighbor2BoundarySourceG neighbor2EdgesG neighbor2EdgeListG; native_decide\n"
        "theorem neighbor2BoundaryTargetG_eq :\n"
        "    neighbor2BoundaryTargetG = {2, 3, 4, 6, 7, 15, 16, 18, 20, 24, 25, 26} := by\n"
        "  unfold neighbor2BoundaryTargetG neighbor2EdgesG neighbor2EdgeListG; native_decide\n\n";
}

// Renders `{a, b, c, ...}` for a concrete Nat list (Finset literal syntax).
inline std::string render_nat_finset(const std::vector<long long>& xs) {
    std::ostringstream out;
    out << "{";
    for (std::size_t i = 0; i < xs.size(); ++i) {
        if (i > 0) out << ", ";
        out << xs[i];
    }
    out << "}";
    return out.str();
}

// Mechanically emits, PER `ClassIINeighborDSupportCertificate` node, a
// `decide`-checked EQUALITY between the CONCRETE source/target index
// sets C++ actually computed (from its own affine-edge catalog) and
// the corresponding `neighborXBoundarySourceG`/`TargetG` -- which are
// themselves proven, in the SAME embedded excerpt above, to equal the
// literal sets `lean/class_ii_neighbor_d_support.lean` derives from
// its independently maintained edge catalog. A divergence anywhere in
// that chain (C++'s edges, this file's embedded copy, or the two
// catalogs disagreeing on the boundary layer) would make the kernel
// check legitimately fail.
inline std::string render_class_ii_neighbor_d_support_instances(const mathlib::reflection::Trace& trace) {
    std::ostringstream out;
    long long counter = 0;
    auto nodes = trace.find<mathlib::reflection::ClassIINeighborDSupportCertificate>();
    if (nodes.empty()) return {};
    out << class_ii_neighbor_d_support_lemma_lean();
    for (const auto& [id, node] : nodes) {
        (void)id;
        std::string srcDef, tgtDef, srcEq, tgtEq;
        if (node->neighbor == 0) {
            srcDef = "neighbor0BoundarySourceG"; tgtDef = "neighbor0BoundaryTargetG";
            srcEq = "neighbor0BoundarySourceG_eq"; tgtEq = "neighbor0BoundaryTargetG_eq";
        } else if (node->neighbor == 1) {
            srcDef = "neighbor1BoundarySourceG"; tgtDef = "neighbor1BoundaryTargetG";
            srcEq = "neighbor1BoundarySourceG_eq"; tgtEq = "neighbor1BoundaryTargetG_eq";
        } else if (node->neighbor == 2) {
            srcDef = "neighbor2BoundarySourceG"; tgtDef = "neighbor2BoundaryTargetG";
            srcEq = "neighbor2BoundarySourceG_eq"; tgtEq = "neighbor2BoundaryTargetG_eq";
        } else {
            continue;  // unknown neighbor index -- render nothing rather than guess
        }
        std::string name = "class_ii_neighbor_d_support_instance_" + std::to_string(counter++);
        out << "/-- Mechanically emitted: the concrete source/target index sets C++\n";
        out << "    actually computed for neighbor " << node->neighbor
            << " equal " << srcDef << "/" << tgtDef << " exactly, which " << srcEq
            << "/" << tgtEq << " (above) prove equal the literal sets. -/\n";
        out << "theorem " << name << " :\n";
        out << "    (" << render_nat_finset(node->sources) << " : Finset Nat) = " << srcDef
            << " ∧ (" << render_nat_finset(node->targets) << " : Finset Nat) = " << tgtDef << " := by\n";
        out << "  constructor <;> native_decide\n\n";
    }
    return out.str();
}

// Mechanically emits, PER `CayleyHamiltonCubicCertificate` node, a
// `decide`-checked identity M^3 = M + I for the CONCRETE 3x3 integer
// matrix C++ actually verified this for -- Lean re-derives M^3 itself
// via Mathlib's own `Matrix` power (from these nine entries), it does
// not just restate the C++'s own arithmetic.
inline std::string render_cayley_hamilton_cubic_instances(const mathlib::reflection::Trace& trace) {
    std::ostringstream out;
    long long counter = 0;
    auto nodes = trace.find<mathlib::reflection::CayleyHamiltonCubicCertificate>();
    if (nodes.empty()) return {};
    for (const auto& [id, node] : nodes) {
        (void)id;
        const auto& m = node->matrix;
        std::string name = "cayley_hamilton_cubic_instance_" + std::to_string(counter++);
        out << "/-- Mechanically emitted: " << node->description << ". -/\n";
        out << "theorem " << name << " :\n";
        out << "    (!![" << m[0] << "," << m[1] << "," << m[2] << ";"
            << m[3] << "," << m[4] << "," << m[5] << ";"
            << m[6] << "," << m[7] << "," << m[8]
            << "] : Matrix (Fin 3) (Fin 3) Int) ^ 3 =\n";
        out << "    (!![" << m[0] << "," << m[1] << "," << m[2] << ";"
            << m[3] << "," << m[4] << "," << m[5] << ";"
            << m[6] << "," << m[7] << "," << m[8]
            << "] : Matrix (Fin 3) (Fin 3) Int) + 1 := by\n";
        out << "  decide\n\n";
    }
    return out.str();
}

// Finding 29's general ordering lemma, hand-proven once: given a real
// root bracketed above by hi3, another real root bracketed below by
// lo4 and above by hi4, and a gap hi3 < lo4 plus a bound hi4 < target,
// the first root is strictly less than the second, which is strictly
// less than the target. Trivial from the bracket inequalities alone
// (no polynomial-specific reasoning needed) -- the substance is in
// the CONCRETE exact rational brackets each instance below supplies.
inline const char* pisot_root_ordering_general_lemma_lean() {
    return
        "theorem pisot_root_strictly_between {hi3 lo4 hi4 beta3 beta4 target : ℝ}\n"
        "    (h3hi : beta3 ≤ hi3) (h4lo : lo4 ≤ beta4) (h4hi : beta4 ≤ hi4)\n"
        "    (hgap : hi3 < lo4) (hbound : hi4 < target) :\n"
        "    beta3 < beta4 ∧ beta4 < target := by\n"
        "  constructor <;> linarith\n\n";
}

// Mechanically emits one Lean corollary PER `PisotRootOrderingCertificate`
// node -- concrete exact rational brackets from `pisot_classify_3x3`/
// `_4x4` (Sturm-chain isolation, not floating point); the gap/bound
// inequalities are concrete rational facts discharged by `norm_num`.
inline std::string render_pisot_root_ordering_instances(const mathlib::reflection::Trace& trace) {
    std::ostringstream out;
    long long counter = 0;
    auto nodes = trace.find<mathlib::reflection::PisotRootOrderingCertificate>();
    if (nodes.empty()) return {};
    out << pisot_root_ordering_general_lemma_lean();
    for (const auto& [id, node] : nodes) {
        (void)id;
        std::string name = "pisot_root_ordering_instance_" + std::to_string(counter++);
        out << "/-- Mechanically emitted: instantiates the general lemma above for the\n";
        out << "    a-bonacci family at a=" << node->a << " (Finding 29). -/\n";
        out << "theorem " << name << " {beta3 beta4 : ℝ}\n";
        out << "    (h3hi : beta3 ≤ (" << node->hi3_num << " : ℝ) / (" << node->hi3_den << " : ℝ))\n";
        out << "    (h4lo : (" << node->lo4_num << " : ℝ) / (" << node->lo4_den << " : ℝ) ≤ beta4)\n";
        out << "    (h4hi : beta4 ≤ (" << node->hi4_num << " : ℝ) / (" << node->hi4_den << " : ℝ)) :\n";
        out << "    beta3 < beta4 ∧ beta4 < (" << (node->a + 1) << " : ℝ) := by\n";
        out << "  have hgap : (" << node->hi3_num << " : ℝ) / (" << node->hi3_den << " : ℝ) < ("
            << node->lo4_num << " : ℝ) / (" << node->lo4_den << " : ℝ) := by norm_num\n";
        out << "  have hbound : (" << node->hi4_num << " : ℝ) / (" << node->hi4_den << " : ℝ) < ("
            << (node->a + 1) << " : ℝ) := by norm_num\n";
        out << "  exact pisot_root_strictly_between h3hi h4lo h4hi hgap hbound\n\n";
    }
    return out.str();
}

// A self-contained excerpt of `lean/class_ii_six_vertex_graduation.
// lean` -- reproduced, not re-derived. `promotedNodes`/`transferredNode`
// are GENERAL functions of q (not a fixed table), and `promotedNodes_
// nodup`/`promoted_disjoint_transferred` are already proven for EVERY
// q >= 4 -- the general lemmas this excerpt instantiates.
inline const char* class_ii_six_vertex_graduation_lemma_lean() {
    return
        "structure Node5G where\n"
        "  i : Int\n  x0 : Int\n  x1 : Int\n  x2 : Int\n  j : Int\n"
        "  deriving DecidableEq\n\n"
        "def promotedNodesG (q : Int) : List Node5G :=\n"
        "  [⟨0, q - 2, -(q - 2), 2, 1⟩,\n"
        "   ⟨2, -(q - 2), q - 2, -2, 0⟩,\n"
        "   ⟨2, -(q - 1), q - 1, -1, 1⟩,\n"
        "   ⟨2, -q, q, -1, 1⟩,\n"
        "   ⟨2, -q, q, -2, 0⟩,\n"
        "   ⟨2, q - 2, -(q - 2), 1, 1⟩]\n\n"
        "def transferredNodeG (q : Int) : Node5G :=\n"
        "  ⟨2, -(q - 1), q - 1, -2, 0⟩\n\n"
        "/-- Reproduced from the independently kernel-checked\n"
        "    `lean/class_ii_six_vertex_graduation.lean` (not re-derived here). -/\n"
        "theorem promotedNodesG_nodup (q : Int) (_hq : 4 ≤ q) :\n"
        "    (promotedNodesG q).Nodup := by\n"
        "  unfold promotedNodesG; simp [Node5G.mk.injEq]\n\n"
        "theorem promoted_disjoint_transferredG (q : Int) (_hq : 4 ≤ q) :\n"
        "    (promotedNodesG q).all (· ≠ transferredNodeG q) := by\n"
        "  unfold promotedNodesG transferredNodeG; simp [Node5G.mk.injEq] <;> omega\n\n";
}

// Renders `[⟨i,x0,x1,x2,j⟩, ...]` for a concrete Node5G list.
inline std::string render_node5g_list(const std::array<mathlib::reflection::ClassIINodeData, 6>& nodes) {
    std::ostringstream out;
    out << "[";
    for (std::size_t i = 0; i < nodes.size(); ++i) {
        if (i > 0) out << ", ";
        const auto& n = nodes[i];
        out << "(⟨" << n.left << "," << n.x0 << "," << n.x1 << "," << n.x2 << "," << n.right << "⟩ : Node5G)";
    }
    out << "]";
    return out.str();
}

// Mechanically emits, PER `ClassIISixVertexGraduationCertificate`
// node, `decide`-checked equalities between the CONCRETE promoted/
// transferred nodes C++ actually built at that `a` and `promotedNodesG`/
// `transferredNodeG` evaluated at q=a-1 (order-independent for the
// promoted six, via `.toFinset`, since the C++ side is an
// order-agnostic `std::set`), then instantiates the ALREADY-PROVEN
// general `Nodup`/disjoint lemmas at that q and rewrites through the
// equalities to conclude they hold for the concrete data too.
inline std::string render_class_ii_six_vertex_graduation_instances(const mathlib::reflection::Trace& trace) {
    std::ostringstream out;
    long long counter = 0;
    auto nodes = trace.find<mathlib::reflection::ClassIISixVertexGraduationCertificate>();
    if (nodes.empty()) return {};
    out << class_ii_six_vertex_graduation_lemma_lean();
    for (const auto& [id, node] : nodes) {
        (void)id;
        const long long q = node->a - 1;
        const auto& t = node->transferred;
        std::string name = "class_ii_six_vertex_graduation_instance_" + std::to_string(counter++);
        out << "/-- Mechanically emitted: the concrete promoted/transferred nodes C++\n";
        out << "    actually built at a=" << node->a << " equal promotedNodesG/transferredNodeG\n";
        out << "    at q=" << q << " exactly, so the already-proven general Nodup/disjoint\n";
        out << "    facts (above) transfer to this concrete data. -/\n";
        out << "theorem " << name << " :\n";
        out << "    (" << render_node5g_list(node->promoted) << " : List Node5G).toFinset = (promotedNodesG (" << q << " : Int)).toFinset ∧\n";
        out << "    (⟨" << t.left << "," << t.x0 << "," << t.x1 << "," << t.x2 << "," << t.right
            << "⟩ : Node5G) = transferredNodeG (" << q << " : Int) ∧\n";
        out << "    (promotedNodesG (" << q << " : Int)).Nodup ∧\n";
        out << "    (promotedNodesG (" << q << " : Int)).all (· ≠ transferredNodeG (" << q << " : Int)) := by\n";
        out << "  refine ⟨by decide, by decide, ?_, ?_⟩\n";
        out << "  · exact promotedNodesG_nodup (" << q << " : Int) (by norm_num)\n";
        out << "  · exact promoted_disjoint_transferredG (" << q << " : Int) (by norm_num)\n\n";
    }
    return out.str();
}

// A self-contained excerpt of `lean/class_ii_neighbor2_extensions.
// lean`'s `neighbor2TerminalSextet` -- reproduced, not re-derived
// (re-expressed with a locally-defined `ClassIINodeTSG` struct rather
// than the source file's tuple type or the `ClassIINodeG` struct used
// by other excerpts, to keep this excerpt self-contained regardless
// of which other excerpts co-occur in the same generated module).
inline const char* class_ii_terminal_sextet_lemma_lean() {
    return
        "structure ClassIINodeTSG where\n"
        "  left : Int\n  x0 : Int\n  x1 : Int\n  x2 : Int\n  right : Int\n"
        "  deriving DecidableEq\n\n"
        "def neighbor2TerminalSextetG (a : Int) : List ClassIINodeTSG :=\n"
        "  let q := a - 2\n"
        "  [⟨0, q, -(q+1), 2, 2⟩, ⟨0, q, -q, 2, 1⟩, ⟨1, -q, q, -2, 0⟩,\n"
        "   ⟨2, -q, q, -2, 0⟩, ⟨2, -q, q+1, -2, 0⟩, ⟨2, q, -q, 1, 1⟩]\n\n"
        "/-- Reproduced from the independently kernel-checked\n"
        "    `lean/class_ii_neighbor2_extensions.lean` (not re-derived here). -/\n"
        "theorem neighbor2TerminalSextetG_length (a : Int) :\n"
        "    (neighbor2TerminalSextetG a).length = 6 := by\n"
        "  unfold neighbor2TerminalSextetG; rfl\n\n";
}

// Renders `[⟨i,x0,x1,x2,j⟩, ...]` for a concrete ClassIINodeTSG list.
inline std::string render_class_ii_node_tsg_list(
    const std::vector<mathlib::reflection::ClassIINodeData>& nodes) {
    std::ostringstream out;
    out << "[";
    for (std::size_t i = 0; i < nodes.size(); ++i) {
        if (i > 0) out << ", ";
        const auto& n = nodes[i];
        out << "(⟨" << n.left << "," << n.x0 << "," << n.x1 << "," << n.x2 << "," << n.right << "⟩ : ClassIINodeTSG)";
    }
    out << "]";
    return out.str();
}

// Mechanically emits, PER `ClassIITerminalSextetCertificate` node, a
// `decide`-checked EQUALITY between the CONCRETE 6-node list C++
// actually built at that `a` and `neighbor2TerminalSextetG` evaluated
// at that exact `a` (same field order both sides -- a genuine list
// equality, not just membership).
inline std::string render_class_ii_terminal_sextet_instances(const mathlib::reflection::Trace& trace) {
    std::ostringstream out;
    long long counter = 0;
    auto nodes = trace.find<mathlib::reflection::ClassIITerminalSextetCertificate>();
    if (nodes.empty()) return {};
    out << class_ii_terminal_sextet_lemma_lean();
    for (const auto& [id, node] : nodes) {
        (void)id;
        std::array<mathlib::reflection::ClassIINodeData, 6> arr = node->nodes;
        std::vector<mathlib::reflection::ClassIINodeData> vec(arr.begin(), arr.end());
        std::string name = "class_ii_terminal_sextet_instance_" + std::to_string(counter++);
        out << "/-- Mechanically emitted: the concrete terminal sextet C++ actually\n";
        out << "    built at a=" << node->a << " equals neighbor2TerminalSextetG at that a. -/\n";
        out << "theorem " << name << " :\n";
        out << "    " << render_class_ii_node_tsg_list(vec) << " = neighbor2TerminalSextetG (" << node->a << " : Int) := by\n";
        out << "  decide\n\n";
    }
    return out.str();
}

// A self-contained excerpt of `lean/class_ii_neighbor2_extensions.
// lean`'s `neighbor2PenultimatePair` -- reproduced, not re-derived
// (own locally-scoped struct, same reasoning as the terminal-sextet
// excerpt above: stays correct whether rendered alone or alongside
// other excerpts).
inline const char* class_ii_penultimate_pair_lemma_lean() {
    return
        "structure ClassIINodePPG where\n"
        "  left : Int\n  x0 : Int\n  x1 : Int\n  x2 : Int\n  right : Int\n"
        "  deriving DecidableEq\n\n"
        "def neighbor2PenultimatePairG (a : Int) : List ClassIINodePPG :=\n"
        "  [⟨2, -(a-1), a-1, -1, 0⟩, ⟨2, -(a-2), a-2, -2, 0⟩]\n\n"
        "/-- Reproduced from the independently kernel-checked\n"
        "    `lean/class_ii_neighbor2_extensions.lean` (not re-derived here). -/\n"
        "theorem neighbor2PenultimatePairG_length (a : Int) :\n"
        "    (neighbor2PenultimatePairG a).length = 2 := by\n"
        "  unfold neighbor2PenultimatePairG; rfl\n\n";
}

// Mechanically emits, PER `ClassIIPenultimatePairCertificate` node, a
// `decide`-checked EQUALITY (order-independent, via `.toFinset`,
// since the C++ side is an order-agnostic `std::set`) between the
// CONCRETE pair C++ actually built at that `a` and
// `neighbor2PenultimatePairG` evaluated at that exact `a`.
inline std::string render_class_ii_penultimate_pair_instances(const mathlib::reflection::Trace& trace) {
    std::ostringstream out;
    long long counter = 0;
    auto nodes = trace.find<mathlib::reflection::ClassIIPenultimatePairCertificate>();
    if (nodes.empty()) return {};
    out << class_ii_penultimate_pair_lemma_lean();
    for (const auto& [id, node] : nodes) {
        (void)id;
        std::string name = "class_ii_penultimate_pair_instance_" + std::to_string(counter++);
        out << "/-- Mechanically emitted: the concrete penultimate pair C++ actually\n";
        out << "    built at a=" << node->a << " equals neighbor2PenultimatePairG at that a. -/\n";
        out << "theorem " << name << " :\n";
        out << "    ([";
        for (std::size_t i = 0; i < node->nodes.size(); ++i) {
            if (i > 0) out << ", ";
            const auto& n = node->nodes[i];
            out << "(⟨" << n.left << "," << n.x0 << "," << n.x1 << "," << n.x2 << "," << n.right << "⟩ : ClassIINodePPG)";
        }
        out << "] : List ClassIINodePPG).toFinset = (neighbor2PenultimatePairG (" << node->a << " : Int)).toFinset := by\n";
        out << "  decide\n\n";
    }
    return out.str();
}

// A self-contained excerpt of `lean/class_ii_neighbor2_extensions.
// lean`'s `neighbor2InteriorTip` -- reproduced, not re-derived (own
// locally-scoped struct, same reasoning as the other class_ii_
// neighbor2_extensions excerpts above).
inline const char* class_ii_interior_tip_lemma_lean() {
    return
        "structure ClassIINodeITG where\n"
        "  left : Int\n  x0 : Int\n  x1 : Int\n  x2 : Int\n  right : Int\n"
        "  deriving DecidableEq\n\n"
        "def neighbor2InteriorTipG (r : Int) : ClassIINodeITG :=\n"
        "  ⟨2, -r, r, -1, 0⟩\n\n"
        "/-- Reproduced from the independently kernel-checked\n"
        "    `lean/class_ii_neighbor2_extensions.lean` (not re-derived here). -/\n"
        "theorem neighbor2InteriorTipG_injective :\n"
        "    Function.Injective neighbor2InteriorTipG := by\n"
        "  intro r s h\n"
        "  simp [neighbor2InteriorTipG, ClassIINodeITG.mk.injEq] at h\n"
        "  exact h\n\n";
}

// Mechanically emits, PER `ClassIIInteriorTipCertificate` node, a
// `decide`-checked EQUALITY between the CONCRETE node C++ actually
// built at that `r` and `neighbor2InteriorTipG` evaluated at that
// exact `r`.
inline std::string render_class_ii_interior_tip_instances(const mathlib::reflection::Trace& trace) {
    std::ostringstream out;
    long long counter = 0;
    auto nodes = trace.find<mathlib::reflection::ClassIIInteriorTipCertificate>();
    if (nodes.empty()) return {};
    out << class_ii_interior_tip_lemma_lean();
    for (const auto& [id, node] : nodes) {
        (void)id;
        const auto& n = node->node;
        std::string name = "class_ii_interior_tip_instance_" + std::to_string(counter++);
        out << "/-- Mechanically emitted: the concrete node C++ actually built at r="
            << node->r << " equals neighbor2InteriorTipG at that r. -/\n";
        out << "theorem " << name << " :\n";
        out << "    (⟨" << n.left << "," << n.x0 << "," << n.x1 << "," << n.x2 << "," << n.right
            << "⟩ : ClassIINodeITG) = neighbor2InteriorTipG (" << node->r << " : Int) := by\n";
        out << "  decide\n\n";
    }
    return out.str();
}

// A self-contained excerpt of `lean/class_ii_global_round_partition.
// lean` -- reproduced, not re-derived. `classIIGlobalRoundPhase` is a
// GENERAL, total decision procedure over (a, r), already proven
// exhaustive/unique for every legal round (a>=7, 1<=r<=a+1).
inline const char* class_ii_global_round_phase_lemma_lean() {
    return
        "inductive ClassIIGlobalRoundPhaseG\n"
        "  | base | stable | penultimate | terminal | repeated\n"
        "  deriving DecidableEq\n\n"
        "def classIIGlobalRoundPhaseG (a r : Int) : ClassIIGlobalRoundPhaseG :=\n"
        "  if r ≤ 4 then .base\n"
        "  else if r ≤ a - 2 then .stable\n"
        "  else if r = a - 1 then .penultimate\n"
        "  else if r = a then .terminal\n"
        "  else .repeated\n\n"
        "/-- Reproduced from the independently kernel-checked\n"
        "    `lean/class_ii_global_round_partition.lean` (not re-derived here). -/\n"
        "theorem classIIGlobalRoundPhaseG_spec (a r : Int) (ha : 7 ≤ a)\n"
        "    (hr0 : 1 ≤ r) (hr1 : r ≤ a + 1) :\n"
        "    (classIIGlobalRoundPhaseG a r = .base ↔ r ≤ 4) ∧\n"
        "    (classIIGlobalRoundPhaseG a r = .stable ↔ 5 ≤ r ∧ r ≤ a - 2) ∧\n"
        "    (classIIGlobalRoundPhaseG a r = .penultimate ↔ r = a - 1) ∧\n"
        "    (classIIGlobalRoundPhaseG a r = .terminal ↔ r = a) ∧\n"
        "    (classIIGlobalRoundPhaseG a r = .repeated ↔ r = a + 1) := by\n"
        "  unfold classIIGlobalRoundPhaseG\n"
        "  split_ifs <;> simp_all <;> omega\n\n";
}

inline const char* class_ii_global_round_phase_name(int phase) {
    switch (phase) {
        case 0: return "ClassIIGlobalRoundPhaseG.base";
        case 1: return "ClassIIGlobalRoundPhaseG.stable";
        case 2: return "ClassIIGlobalRoundPhaseG.penultimate";
        case 3: return "ClassIIGlobalRoundPhaseG.terminal";
        default: return "ClassIIGlobalRoundPhaseG.repeated";
    }
}

// Mechanically emits, PER `ClassIIGlobalRoundPhaseCertificate` node, a
// `decide`-checked EQUALITY between the CONCRETE phase C++ actually
// computed at that (a, round) and `classIIGlobalRoundPhaseG` evaluated
// at that exact (a, round). The already-proven general `_spec`
// iff-characterization (embedded above, reproduced from the source
// file) additionally guarantees -- for any legal (a, round) -- that
// this phase satisfies its displayed domain condition, without this
// excerpt needing to instantiate it separately per instance.
inline std::string render_class_ii_global_round_phase_instances(const mathlib::reflection::Trace& trace) {
    std::ostringstream out;
    long long counter = 0;
    auto nodes = trace.find<mathlib::reflection::ClassIIGlobalRoundPhaseCertificate>();
    if (nodes.empty()) return {};
    out << class_ii_global_round_phase_lemma_lean();
    for (const auto& [id, node] : nodes) {
        (void)id;
        std::string name = "class_ii_global_round_phase_instance_" + std::to_string(counter++);
        out << "/-- Mechanically emitted: the concrete phase C++ actually computed at\n";
        out << "    a=" << node->a << ", round=" << node->round
            << " equals classIIGlobalRoundPhaseG at that (a,round). -/\n";
        out << "theorem " << name << " :\n";
        out << "    classIIGlobalRoundPhaseG (" << node->a << " : Int) (" << node->round << " : Int) = "
            << class_ii_global_round_phase_name(node->phase) << " := by\n";
        out << "  decide\n\n";
    }
    return out.str();
}

// Finding 12's general lemma, already kernel-checked once
// (lean/class_ii_round234_shape_closure.lean): an affine integer
// function with nonzero slope can equal a fixed target for at most
// one input; if that unique solution is below a threshold, it never
// re-hits the target at or above it.
inline const char* both_fixed_affine_general_lemma_lean() {
    return
        "theorem affine_no_solution_at_or_above_threshold\n"
        "    (const_ slope target a0 threshold : ℤ)\n"
        "    (hslope : slope ≠ 0)\n"
        "    (ha0 : const_ + a0 * slope = target)\n"
        "    (hbelow : a0 < threshold) :\n"
        "    ∀ a : ℤ, threshold ≤ a → const_ + a * slope ≠ target := by\n"
        "  intro a ha hcontra\n"
        "  have heq : a0 * slope = a * slope := by linarith [ha0, hcontra]\n"
        "  have : a0 = a := by\n"
        "    have := mul_right_cancel₀ hslope heq\n"
        "    linarith [this]\n"
        "  omega\n\n";
}

// Mechanically emits one Lean corollary PER `BothFixedAffineCertificate`
// node -- concrete CONST/slope/target/a_required from Finding 12's
// closed-form both-fixed proof; `ha0`/`hbelow` are decided by
// `norm_num` on concrete integers.
inline std::string render_both_fixed_affine_instances(const mathlib::reflection::Trace& trace) {
    std::ostringstream out;
    long long counter = 0;
    auto nodes = trace.find<mathlib::reflection::BothFixedAffineCertificate>();
    if (nodes.empty()) return {};
    out << both_fixed_affine_general_lemma_lean();
    for (const auto& [id, node] : nodes) {
        (void)id;
        std::string name = "both_fixed_affine_instance_" + std::to_string(counter++);
        out << "/-- Mechanically emitted: instantiates the general lemma above for\n";
        out << "    CONST=" << node->const_ << ", slope=" << node->slope
            << ", target=" << node->target << ", a_required=" << node->a_required << ". -/\n";
        out << "theorem " << name << " :\n";
        out << "    ∀ a : ℤ, (7 : ℤ) ≤ a → (" << node->const_ << " : ℤ) + a * (" << node->slope
            << " : ℤ) ≠ (" << node->target << " : ℤ) := by\n";
        out << "  apply affine_no_solution_at_or_above_threshold (" << node->const_ << " : ℤ) ("
            << node->slope << " : ℤ) (" << node->target << " : ℤ) (" << node->a_required << " : ℤ) (7 : ℤ)\n";
        out << "  · norm_num\n";
        out << "  · norm_num\n";
        out << "  · norm_num\n\n";
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

    out << render_class_ii_shell_round_instances(trace);
    out << render_class_ii_fixed_table_instances(trace);
    out << render_class_ii_d_cont_in_pre_contact_instances(trace);
    out << render_round1_raw27_instances(trace);
    out << render_class_ii_neighbor_d_support_instances(trace);
    out << render_class_ii_terminal_shell_instances(trace);
    out << render_cayley_hamilton_cubic_instances(trace);
    out << render_pisot_root_ordering_instances(trace);
    out << render_class_ii_six_vertex_graduation_instances(trace);
    out << render_class_ii_terminal_sextet_instances(trace);
    out << render_class_ii_penultimate_pair_instances(trace);
    out << render_class_ii_interior_tip_instances(trace);
    out << render_class_ii_global_round_phase_instances(trace);
    out << render_both_fixed_affine_instances(trace);

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
    out << render_first_letter_orbit_instances(trace);
    out << render_last_letter_orbit_instances(trace);
    out << render_leftmost_loop_instances(trace);
    out << render_depressed_cubic_instances(trace);

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
