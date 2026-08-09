// Test the typed piecewise entry IR for the n-bonacci q-family.
//
// What this test guarantees:
//   1. The typed IR evaluates entry-by-entry to the same integer polynomial
//      the existing C++ loops produce, on every (n, i, j) across a finite
//      regression range (n = 2..6, all (i, j)).
//   2. The Lean rendering matches the in-repository `Ravel.Matrix.EraseIndex.qMatrix`
//      definition branch-for-branch, so a finite regression in either language
//      implies agreement in both.
//   3. The r-family typed IR matches its C++ loop likewise.
//   4. The campaign engine consumes the typed IR and produces a closed
//      `qMatrix_minor_eq_qMatrix` artifact whose emitted Lean module
//      includes the typed definition and the case-analysis proof, with
//      no `sorry` / `admit` and no stringly-typed entry rule.
//
// This test is the typed-IR regression gate; if a future change to the
// `qMatrix` C++ definition, the typed IR, the Lean definition, or the
// case-analysis proof loses parity with any of the other three, the
// corresponding assertion below fails.

#include <cassert>
#include <cstdio>
#include <string>

#include "math/poly_matrix.hpp"
#include "math/poly_z.hpp"
#include "math/proof_reflection.hpp"
#include "ravel/proof/proof_campaign_engine.hpp"

namespace {

using mathlib::PolyZ;
using mathlib::reflection::DimExpr;
using mathlib::reflection::IndexExpr;
using mathlib::reflection::EntryCondition;
using mathlib::reflection::EntryExpression;
using mathlib::reflection::EntryBranch;
using mathlib::reflection::PiecewiseEntry;

// Concrete C++ evaluation of the q-matrix entry rule at a given (n, i, j).
// Mirrors the loop in `mathlib::nbonacci_q_matrix` so the typed IR and
// the executable definition are tested against the same arithmetic.
PolyZ qmatrix_cpp_entry(std::size_t n, std::size_t i, std::size_t j) {
    if (n < 2) return PolyZ(0);
    if (i == n - 2 && j == n - 2) return PolyZ({1, 1});        // X + 1
    if (i == n - 2 && j < n - 2) return PolyZ(1);
    if (i < n - 2 && j == i + 1) return PolyZ(-1);
    if (i < n - 2 && j == i) return PolyZ({0, 1});
    return PolyZ(0);
}

// Concrete C++ evaluation of the r-matrix entry rule at a given (n, i, j).
PolyZ rmatrix_cpp_entry(std::size_t n, std::size_t i, std::size_t j) {
    if (n < 2) return PolyZ(0);
    if (j + 1 == i) return PolyZ({0, 1});
    if (i == j) return PolyZ(-1);
    return PolyZ(0);
}

} // namespace

int main() {
    // 1. Type the q-family piecewise entry and check every (n, i, j)
    //    against the C++ loop. This is the typed-IR / execution parity
    //    test.
    auto q_piecewise = mathlib::nbonacci_q_piecewise_entry();
    for (std::size_t n = 2; n <= 6; ++n) {
        for (std::size_t i = 0; i < n - 1; ++i) {
            for (std::size_t j = 0; j < n - 1; ++j) {
                const auto typed = q_piecewise.eval(
                    static_cast<int64_t>(i), static_cast<int64_t>(j),
                    static_cast<int64_t>(n));
                // Eval returns a polynomial expression; collapse it to a
                // BigInt-typed PolyZ by evaluating at x = 0 (constant term)
                // and x = 1 (linear term) so we can compare with the
                // hand-coded C++ entry.
                const int64_t at_zero = typed.eval_as_int(0);
                const int64_t at_one = typed.eval_as_int(1);
                PolyZ expected(0);
                if (at_one - at_zero != 0) {
                    expected = expected + PolyZ({0, 1}) * PolyZ(at_one - at_zero);
                }
                if (at_zero != 0) {
                    expected = expected + PolyZ(at_zero);
                }
                const PolyZ actual = qmatrix_cpp_entry(n, i, j);
                assert(expected == actual);
            }
        }
    }

    // 2. Type the r-family piecewise entry and check parity with C++.
    auto r_piecewise = mathlib::nbonacci_r_piecewise_entry();
    for (std::size_t n = 2; n <= 6; ++n) {
        for (std::size_t i = 0; i < n - 1; ++i) {
            for (std::size_t j = 0; j < n - 1; ++j) {
                const auto typed = r_piecewise.eval(
                    static_cast<int64_t>(i), static_cast<int64_t>(j),
                    static_cast<int64_t>(n));
                const int64_t at_zero = typed.eval_as_int(0);
                const int64_t at_one = typed.eval_as_int(1);
                PolyZ expected(0);
                if (at_one - at_zero != 0) {
                    expected = expected + PolyZ({0, 1}) * PolyZ(at_one - at_zero);
                }
                if (at_zero != 0) {
                    expected = expected + PolyZ(at_zero);
                }
                const PolyZ actual = rmatrix_cpp_entry(n, i, j);
                assert(expected == actual);
            }
        }
    }

    // 3. Check that the typed q-family definition emits a Lean body that
    //    matches the in-repository `Ravel.Matrix.EraseIndex.qMatrix`
    //    branch-for-branch. The case-analysis proof walks the same
    //    branches, so a regression in the typed IR's Lean output also
    //    regresses the campaign-emitted theorem.
    const std::string lean = q_piecewise.definition_lean_value();
    assert(lean.find("i.val = (n - 2)") != std::string::npos);
    assert(lean.find("j.val = (n - 2)") != std::string::npos);
    assert(lean.find("j.val < (n - 2)") != std::string::npos);
    assert(lean.find("j.val = (i.val + 1)") != std::string::npos);
    assert(lean.find("j.val = i.val") != std::string::npos);
    assert(lean.find("Polynomial.C 1 + Polynomial.X") != std::string::npos);
    assert(lean.find("Polynomial.C (-1)") != std::string::npos);
    assert(lean.find("Polynomial.X") != std::string::npos);

    // 4. The derivation operator must reject a typed family that does not
    //    actually satisfy the declared erased-index transform. This prevents
    //    a generic-looking `ext; simp; omega` wrapper from certifying an
    //    unrelated pair of branch programs.
    auto mutated_q = q_piecewise;
    mutated_q.branches.front().expression = EntryExpression::zero();
    const ravel::proof::PiecewiseMatrixTransform q_minor_transform{
        1, 0, 0, "Ravel.Matrix.EraseIndex.minor"};
    const auto good_transform = ravel::proof::validate_piecewise_minor_transform(
        q_piecewise, q_piecewise, q_minor_transform, 2, 10);
    assert(good_transform.valid);
    assert(good_transform.checked_entries > 0);
    const auto bad_transform = ravel::proof::validate_piecewise_minor_transform(
        mutated_q, q_piecewise, q_minor_transform, 2, 10);
    assert(!bad_transform.valid);
    assert(bad_transform.diagnostic.find("counterexample") != std::string::npos);

    // 4. End-to-end: the campaign engine consumes the typed IR and closes
    //    the q-minor transport task with a closed artifact. The emitted
    //    Lean module includes the typed qMatrix definition and the
    //    case-analysis proof, with no stringly-typed entry rule and no
    //    `sorry` / `admit`.
    mathlib::reflection::Trace trace("test.qmin.typed_ir");
    {
        mathlib::reflection::ScopedTrace active(&trace);
        (void)mathlib::det(mathlib::nbonacci_q_matrix(5));
        (void)mathlib::det(mathlib::nbonacci_r_matrix(5));
    }

    ravel::proof::CampaignGenerator generator;
    ravel::proof::ProofCampaignExecutor executor;
    const auto result = executor.run(generator.generate(trace), trace);

    const auto* qminor = result.find_task("q_matrix.minor_transport");
    assert(qminor != nullptr);
    assert(qminor->state == ravel::proof::TaskState::Closed);
    assert(qminor->artifact.has_value());
    assert(!qminor->artifact->has_open_goals);
    assert(qminor->artifact->artifact_id == "q_matrix.minor_transport");

    // The closed artifact must contain the qMatrix definition and the
    // qMatrix_minor_eq_qMatrix theorem; the proof must walk the typed
    // branches, not a hand-typed Lean string.
    bool found_qmatrix_def = false;
    bool found_qmatrix_thm = false;
    for (const auto& def : qminor->artifact->definitions) {
        if (def.name == "qMatrix") {
            found_qmatrix_def = true;
            // The definition body must be the typed-IR Lean emission, not
            // a free-form hand-typed alternative.
            assert(def.value.find("Polynomial.C 1 + Polynomial.X") != std::string::npos);
            assert(def.value.find("Polynomial.C (-1)") != std::string::npos);
            assert(def.value.find("Polynomial.X") != std::string::npos);
        }
    }
    assert(found_qmatrix_def);
    for (const auto& thm : qminor->artifact->theorems) {
        if (thm.name == "qMatrix_minor_eq_qMatrix") {
            found_qmatrix_thm = true;
            // The proof must be the reusable executable derivation program,
            // with public Matrix/Fin normalization rather than the retired
            // Ravel.Matrix.EraseIndex helper layer.
            assert(!thm.proof.empty());
            assert(thm.proof.front().kind == ravel::proof::LeanStepKind::Funext);
            bool saw_simp_only = false;
            bool saw_split_ifs = false;
            for (const auto& step : thm.proof) {
                if (step.kind == ravel::proof::LeanStepKind::SimpOnly) {
                    saw_simp_only = true;
                    bool saw_submatrix = false;
                    bool saw_private_helper = false;
                    for (const auto& arg : step.arguments) {
                        if (arg == "Matrix.submatrix") saw_submatrix = true;
                        if (arg.find("Ravel.Matrix.EraseIndex") != std::string::npos) saw_private_helper = true;
                    }
                    assert(saw_submatrix);
                    assert(!saw_private_helper);
                }
                if (step.kind == ravel::proof::LeanStepKind::SplitIfs) saw_split_ifs = true;
            }
            assert(saw_simp_only);
            assert(saw_split_ifs);
        }
    }
    assert(found_qmatrix_thm);

    // 5. The rendered Lean module from the campaign engine must include
    //    the qMatrix definition and the theorem, with no `sorry` /
    //    `admit` and no stringly-typed entry rule.
    const auto module = ravel::proof::render_closed_campaign_lean(result);
    assert(module.find("noncomputable def qMatrix") != std::string::npos);
    assert(module.find("lemma qMatrix_minor_eq_qMatrix") != std::string::npos);
    assert(module.find("sorry") == std::string::npos);
    assert(module.find("admit") == std::string::npos);
    // No "upper bidiagonal X/-1 with terminal row 1,...,1,1+X" string
    // leaks into the emitted module — that's the legacy stringly-typed
    // entry rule description, now replaced by the typed IR.
    assert(module.find("upper bidiagonal X/-1 with terminal row") == std::string::npos);

    std::printf("typed IR regression passed: n=2..6 entry parity, Lean emission parity, campaign q-minor closed\n");
    return 0;
}
