// ravel/proof/property_f_unconditional.hpp
//
// RETITLED IN EFFECT BY A LATER CORRECTION (2026-08-06, same day) --
// read this note FIRST. This header's original claim was "property
// (F), as this project implements it, can never fail". That turned
// out to be true only of a BUG: `check_property_f`'s pre-correction
// verdict logic required a violating cycle to touch BOTH a zero and a
// nonzero node, and the theorem below proves such a "mixed" cycle can
// never occur -- so the OLD code was checking a condition that could
// never trigger, regardless of whether real property-(F) failures
// existed. Reading the primary source directly (Minervino-Thuswaldner,
// Lemma 9.8) showed the real failure condition is a cycle that is
// NOT entirely zero-nodes (equivalently, given mixed cycles are
// impossible: a cycle containing ANY nonzero node). `check_property_f`
// has since been corrected to check exactly that (see its own
// 2026-08-06 comment) -- so property (F), as CURRENTLY implemented,
// is NOT unconditional; it can and does get checked meaningfully now.
//
// What survives from this header, and is still exactly what makes the
// real fix work: the THEOREM below (no mixed cycle can ever occur) is
// still true, still Lean-kernel-checked (see the bottom of this file),
// and is the load-bearing fact that lets "not entirely zero" collapse
// to the simple, cheap-to-check "contains any nonzero node" -- without
// it, the corrected verdict logic would need to search for genuinely
// mixed structure too. Kept in place, re-scoped, rather than deleted,
// since the mathematics is correct and still in active use.
//
// THEOREM (still true, now understood as a LEMMA rather than a
// standalone headline result). For the finite automaton construction
// implementing geometric property (F) in this codebase
// (adelic::check_property_f, following Minervino-Thuswaldner's "The
// geometry of non-unit Pisot substitutions" Section 2.5), for ANY
// Pisot substitution (unit or non-unit, any alphabet size, any
// structural class), no SCC can ever contain BOTH a zero node
// (gamma=0) and a nonzero node.
//
// PROOF. Recall the construction (coincidence_and_property_f.hpp):
// nodes are (gamma, a) pairs, gamma in Q(beta); start set U =
// {(0,a) : a in alphabet}; from (gamma,a), for every prefix-automaton
// edge b -p-> a, transition to (gamma', b) with
//   gamma' = beta^{-1} (gamma + delta(p)),   delta(p) = <P(p), v>,
// v the LEFT Perron eigenvector of the (irreducible, nonnegative)
// incidence matrix, normalized positive by Perron-Frobenius. A
// violation is an SCC containing BOTH a zero node (gamma=0) and a
// nonzero node.
//
// Step 1 (delta(p) >= 0, with equality iff p is empty). P(p) is a
// nonnegative integer letter-count vector; v is entrywise STRICTLY
// POSITIVE under the dominant real embedding (the field embedding
// Q(beta) -> R sending the abstract symbol beta to the actual Pisot
// number -- this is THE embedding under which v is defined, being
// the Perron eigenvector for that specific eigenvalue). So
// delta(p) = <P(p),v> (dominant embedding) is a nonnegative
// combination of positive numbers: zero iff P(p) is the zero vector,
// i.e. iff p is empty.
//
// Step 2 (gamma stays dominant-embedding-nonnegative, by induction on
// BFS discovery order). Base case: every starting node has gamma=0.
// Inductive step: gamma' = (gamma + delta(p)) / beta (dominant
// embedding). beta (dominant embedding) is the actual Pisot number,
// real and > 1. If gamma >= 0 (inductive hypothesis) and delta(p) >= 0
// (Step 1), then gamma' = (nonneg + nonneg) / positive >= 0.
//
// Step 3 (gamma' = 0 as an ABSTRACT Q(beta) element forces BOTH
// gamma = 0 and p empty). This uses only the WEAKEST possible fact
// about the dominant embedding -- not injectivity (which would need
// Q(beta) to be a field, i.e. the incidence matrix's characteristic
// polynomial to be irreducible as a polynomial; this project's own
// substitutions sometimes carry a cyclotomic cofactor, so that
// hypothesis is NOT always available and is not needed here). All
// that's used is the trivial, universally-true fact that ANY ring
// homomorphism sends 0 to 0 -- contrapositive: if "evaluate at the
// dominant root" sends an element to something NONZERO, that element
// cannot be the abstract zero of Q(beta), full stop, regardless of
// whether Q(beta) is a field or a product ring. By Steps 1-2, gamma'
// (dominant embedding) = (gamma + delta(p))/beta is a sum of two
// nonnegative reals divided by a positive real; this REAL VALUE is
// exactly 0 iff both summands are exactly 0, i.e. gamma=0 AND p is
// empty. So whenever that fails, gamma' (dominant embedding) is a
// nonzero real, hence gamma' (abstract) is not the zero element.
//
// The REAL-VALUED half of this step (the walk gamma_0=0,
// gamma_{k+1}=(gamma_k+delta_k)/beta returns to exactly 0 at step n
// iff every delta_k for k<n was itself 0) is now Lean-kernel-checked,
// not just hand-derived: see lean/generated/property_f_zero_walk.lean
// (`RavelGenerated.zeroWalk_eq_zero_iff`), checked with
// scripts/safe_lean_check.sh, no `sorry`, no extra axioms. The
// remaining step -- connecting this real-valued fact back to the
// abstract Q(beta) equality -- is the one-line, completely standard
// "a ring hom sends 0 to 0" fact used in contrapositive form above;
// it is not separately formalized because it needs no case analysis
// and is not a place an error could plausibly hide.
//
// Step 4 (no nonzero node ever has an edge into a zero node).
// Immediate from Step 3's contrapositive: any transition NOT starting
// from a zero node with an empty prefix produces gamma' != 0.
//
// Step 5 (no SCC can mix zero and nonzero nodes). If an SCC contained
// both a zero node z and a nonzero node w, mutual reachability would
// require a path from w back to some zero node -- impossible by
// Step 4 applied along every edge of that path (no edge anywhere ever
// lands on a zero node except from another zero node). Hence no
// property-(F) violation can ever be discovered by this construction.
// QED.
//
// VERIFICATION (not just the derivation above): the accompanying test
// instruments a copy of check_property_f to report any zero node
// discovered beyond the initial alphabet-sized frontier -- across 8
// structurally diverse cases (Finding 5's whole a=0..5 family
// spanning AR-exact/partial/non-AR, the simplest non-unit Pisot
// substitution x^2-2x-2, and rnd13 -- the project's flagship 4-letter
// non-unimodular case, 33185 nodes), zero such nodes are ever found,
// exactly as Step 4 predicts.
//
// =====================================================================
// SCOPE -- fully resolved, 2026-08-06, same day. History kept for the
// record rather than rewritten away.
// =====================================================================
// This file's ORIGINAL scope note (preserved below in spirit) worried
// about whether this codebase's automaton faithfully implements
// Minervino-Thuswaldner's literature definition of property (F). That
// worry turned out to be well-founded, but not for the reason it was
// raised: reading the primary source directly found a real, confirmed
// discrepancy (Finding 20, docs/FINDINGS_FOR_CITATION.md) between what
// `check_property_f` checked (a "mixed" cycle) and what the paper's
// own Lemma 9.8 actually requires (a cycle that is NOT entirely zero-
// nodes). A first attempted fix (enumerating cycles abstractly,
// `property_f_correct_cycle_criterion.hpp`) gave a false FAILS on
// Fibonacci and was abandoned. THE ACTUAL FIX was much smaller: keep
// the existing, already-correct BFS-from-U closure (it faithfully
// implements the paper's own T_ext^{-1} formula, verified against
// eq. 13 directly) and change only the verdict extracted from it, from
// "cycle touches both zero and nonzero" to "cycle touches any nonzero
// node" -- using EXACTLY the theorem proved in this file (no mixed
// cycle ever occurs) to justify why that's the same thing as "not
// entirely zero". See `include/adelic/coincidence_and_property_f.hpp`'s
// own 2026-08-06 correction comment and Finding 21 for the full
// account and the historical-verdict regression
// (tests/property_f_correct_verdict_test.cpp: every ESTABLISHED case
// this project has on record, including Fibonacci, reproduces exactly,
// same node counts, under the corrected logic).
//
// What this means for this file specifically: the model-correspondence
// question IS now resolved (the construction is confirmed faithful to
// the paper, once the verdict bug is fixed) -- but the ORIGINAL
// headline claim ("property F can never fail") is FALSE for the
// corrected code, and was only ever true of the bug. The theorem this
// file proves (no mixed cycle) remains true and is now understood as
// the load-bearing lemma inside the real fix, not a standalone result
// about property F's unconditional truth. The "entire remaining
// difficulty reduces to strong coincidence alone" claim from the
// earlier version of this note is WITHDRAWN -- property (F) is once
// again a genuine, meaningful, non-vacuous check, exactly as a
// fifty-year-open conjecture should require.

#pragma once

#include <string>

namespace ravel::proof {

struct PropertyFUnconditionalNote {
    static constexpr const char* claim =
        "For any Pisot substitution, no nonzero-gamma node in the "
        "property-(F) automaton ever has an edge into a zero-gamma node "
        "(Perron-eigenvector positivity + dominant-embedding sign "
        "induction; Lean-kernel-checked in lean/generated/property_f_"
        "zero_walk.lean). This is the load-bearing lemma inside the "
        "real check_property_f verdict fix (coincidence_and_property_f.hpp, "
        "2026-08-06) -- it is NOT, on its own, a claim that property (F) "
        "always holds. See this header's SCOPE note for the full history: "
        "an earlier version of this file claimed exactly that, and it was "
        "wrong (true only of a bug in the verdict logic, since fixed).";
};

}  // namespace ravel::proof
