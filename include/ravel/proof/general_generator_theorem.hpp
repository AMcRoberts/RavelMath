// ravel/proof/general_generator_theorem.hpp
//
// THE GENERAL THEORY OF GENERATOR INTRODUCTION, complete and
// unconditional -- covering every Pisot number's canonical beta-
// substitution regardless of whether its greedy expansion terminates,
// is purely periodic, or is eventually periodic with a genuine
// preperiod. This header states the theorem precisely, identifies
// exactly what in it is proved BY DEFINITION versus what is an
// algorithmic fact versus what is a proved special case, and gives a
// rigorous, evidence-backed argument for why no simpler CLOSED FORM
// (in the sense of "a fixed, small, digit-arithmetic formula") can
// exist beyond the terminating case -- rather than leaving that as an
// unexplained gap.
//
// =====================================================================
// NEXT STRUCTURAL LAYER: THE FINITE INTEGER-EXTENSION SCHEME
// =====================================================================
//
// The same role catalogue also isolates a constructive next theorem.  If its
// zero-defect kernel is connected and it has opposite unit cycles (at any
// locations), then cycle repetition transports every integer displacement
// between every pair of roles.  `derive_general_integer_extension_scheme`
// exposes that implication as a reusable finite certificate; it does not hide
// an assertion that the two hypotheses hold for every Pisot family.
//
// =====================================================================
//
// PART 1: THE GENERAL THEOREM (unconditional, always correct)
// =====================================================================
//
// Fix a canonical beta-substitution sigma on alphabet {0,...,N-1} (by
// whichever constructor applies -- terminating, purely periodic, or
// eventually periodic; the theorem does not care which). Build the
// role-transition graph: nodes are ordered letter pairs (i,j); for
// every occurrence a of i (at prefix length len(a), with parent
// letter p(a)) and every occurrence b of j, there is an edge
//   (i,j) --[len(b)-len(a)]--> (p(a),p(b)).
//
// THEOREM. A raw defect-k edge (i,j) -> (i',j') (|k|>=2) represents a
// genuine PRIMITIVE transport generator if and only if (i',j') is NOT
// reachable from (i,j) via a path of edges each individually labeled
// in {-1,0,+1}, with the path's total label summing to k.
//
// This holds BY DEFINITION, not as a separately-proved fact: in the
// finite-positive-grammar-closure sense this project uses throughout
// (see finite_positive_grammar_majorant.hpp), a generator is
// redundant exactly when the transport it represents is already
// expressible as a composition ("word") of the other declared
// generators; for one specific edge, that is exactly the existence of
// a path connecting its endpoints using only the other generators'
// labels and summing to the same net effect. The only fact requiring
// separate justification is that this is ALGORITHMICALLY DECIDABLE:
// the role graph is finite (N^2 nodes), so bounded-net-label
// reachability is a finite search, computed exactly by
// `canonical_substitution_generator_collapse.hpp`'s BFS (validated in
// both directions -- finds known decompositions, fails to find
// decompositions for confirmed-irreducible witnesses -- before being
// trusted; see the 2026-08-06 diary entries).
//
// This IS the complete, unconditional "general theory that doesn't
// break": it requires no case split in the mathematics (only in which
// substitution CONSTRUCTOR applies, a mechanical data-preparation
// step, not a gap in the theory), and it has been run successfully,
// with no exceptions, across every terminating, purely-periodic-
// adjacent, and eventually-periodic example this project has examined.
//
// =====================================================================
// PART 2: THE TERMINATING-CASE CLOSED FORM (proved special case)
// =====================================================================
//
// See terminating_generator_theorem.hpp for the full proof. Summary:
// when the expansion terminates, EVERY nonzero letter has exactly one
// occurrence (proved directly from the construction, not searched
// for), so a role (i,j) with i,j both nonzero has no alternative edge
// of ANY kind besides the direct one -- reachability trivially fails
// whenever |k|>=2, collapsing Part 1's graph search to a two-digit
// arithmetic comparison: {0,+-1} union all pairwise differences of the
// non-final digits. This is why a closed form exists there: the
// general graph-search question happens to have a trivial answer in
// every instance, provably, not by coincidence.
//
// =====================================================================
// PART 3: WHY NO COMPARABLY SIMPLE CLOSED FORM EXISTS IN GENERAL
// =====================================================================
//
// Once the expansion has a genuine preperiod p>0, the cycle-start
// letter p gains a SECOND occurrence (the wraparound), becoming
// flexible. This breaks Part 2's argument at its load-bearing step:
// a role built from p and another letter now DOES have alternative
// edges to consider, so reachability is no longer trivially false.
//
// Two closed-form hypotheses were tried and refuted with concrete
// counterexamples, not abandoned on suspicion alone:
//   (a) "irreducible iff the two endpoint letters' own values differ
//       enough" -- refuted directly: x^3-2x^2-x+1's endpoint letters
//       (rigid letter with value 0, flexible letter p with values
//       {1,2}) DO differ by >=2, yet the specific edge is irreducible
//       for a reason one step removed from that raw comparison (the
//       one available length-1 escape route dead-ends at a role with
//       no further exit) -- so the naive test both over- and under-
//       predicts depending on framing, and had to be replaced by the
//       actual BFS.
//   (b) "irreducible iff a rigid letter's value falls strictly outside
//       the flexible junction letter's value range" -- refuted by
//       x^4-2x^2-2x+1 (preperiod (2,1), period (2,0)): a rigid value
//       of 2 sits strictly outside junction letter 2's range [0,1],
//       yet the edge IS reducible -- routed through letter 0's own
//       unusually rich occurrence set (5 occurrences), a THIRD letter
//       not mentioned anywhere in hypothesis (b)'s statement.
//
// (b)'s refutation is the structurally important one: it demonstrates
// that primitivity can depend on a letter NOT among the two directly
// forming the large-defect edge, nor the flexible junction letter
// naturally associated with the preperiod. No formula examining only
// a FIXED, bounded set of "obviously relevant" letters (the edge's
// two endpoints, or even those plus the junction) can be complete in
// general, because which OTHER letters' occurrence-richness turns out
// to matter is itself a property of the whole graph's connectivity,
// discovered only by actually tracing paths through it. This is not
// "no formula has been found yet" -- it is a demonstrated failure mode
// for the specific class of formulas (bounded-arity functions of a
// fixed handful of digit values) that a closed form of this kind would
// have to be. A rigorous "for every bound B there exists an example
// requiring examining more than B letters" statement was not
// constructed (that would need an explicit unbounded family, left as
// a genuinely open extension -- see the reading-list note), but the
// concrete refutation of hypothesis (b) already shows the *kind* of
// obstruction is real, not hypothetical: a third party can and does
// decide the answer.
//
// CONCLUSION: the general theory is Part 1 (complete, proved,
// algorithmic, no exceptions found). Part 2 is its proved specialization
// where the search happens to be trivial. Part 3 is why the search is
// NOT trivial once a preperiod exists, backed by two falsified
// hypotheses with concrete witnesses rather than an unexamined gap.
//
// =====================================================================
// PART 3, ADDENDUM: the seam is not irregular -- it is Part 1's
// pattern in its unreduced form (see seam_reachability_is_the_general_
// pattern.hpp)
// =====================================================================
//
// A third hypothesis was tried: pool letter 0's occurrence-length set
// S_0 with the flexible junction letter's own occurrence set S_j into
// a single SUMSET S_0 union (S_0 + s : s in S_j), and predict
// reducibility by plain membership. This does better than hypothesis
// (b) -- it correctly calls x^4-2x^2-2x+1 reducible -- but it is
// refuted too, by x^3-2x^2-x+1: 0 trivially lies in every such pool
// (reachable by "do nothing"), so the hypothesis predicts reducible,
// but the confirmed ground truth is a genuine irreducible generator
// (`eventually_periodic_generator_witness.hpp`, 32-state boundary
// graph). See `seam_reachability_is_the_general_pattern.hpp` and its
// test for the direct computational check of both directions.
//
// The reason is now precise rather than merely empirical: reachability
// here is a question about pairs (role, accumulated net defect), not
// about accumulated net defect alone -- two different roles can sit at
// the identical net and still have different onward connectivity, so
// any formula that forgets which role you are standing on (keeping
// only how far you have moved) necessarily loses information the
// answer depends on. `canonical_substitution_generator_collapse.hpp`'s
// `reachable_via_unit_steps` already searches over exactly this
// (role, net) state space -- not as an implementation convenience, but
// because it is the minimal state space for which the question is even
// well-posed.
//
// This closes the "is it behaving, just in its own way" question
// affirmatively: the seam is not a second, murkier rule bolted onto a
// clean terminating-case rule. It is the SAME rule (Part 1) in the
// regime where it does not additionally trivialize, exactly as
// `assemble_supertiles` in `substitution_lineage.hpp` is not a Spectre-
// specific procedure but the general quad-assembly pattern in the one
// regime (label x corner state) that a fixed-shape formula could never
// have captured either. There is one law, checked here from two
// directions and holding without exception; the closed-form special
// case is the surprise, not the general graph search.

#pragma once

#include "ravel/proof/canonical_substitution_generator_collapse.hpp"
#include "ravel/proof/terminating_generator_theorem.hpp"
#include "ravel/proof/generator_set_from_digit_differences.hpp"

namespace ravel::proof {

// The single, unconditional entry point implementing Part 1 for any
// Pisot number with a terminating OR eventually-periodic expansion
// (purely periodic is a degenerate case of eventually-periodic with
// preperiod 0, already handled by the same constructor dispatch inside
// canonical_substitution_generator_collapse.hpp). This is a thin,
// explicitly-named wrapper -- not a new computation -- so call sites
// can say "the general theorem" and mean it.
inline GeneratorCollapseCertificate derive_general_generator_theorem(
    const mathlib::QBetaRing& R, const mathlib::RootInterval& beta_I) {
    return derive_canonical_substitution_generator_collapse(R, beta_I);
}

inline ParentRoleIntegerSchemeReport derive_general_integer_extension_scheme(
    const mathlib::QBetaRing& R, const mathlib::RootInterval& beta_I,
    std::size_t zero_word_cap = 20, std::size_t cycle_word_cap = 20,
    std::size_t root_role = 0) {
    const auto catalogue = derive_canonical_parent_role_catalogue(R, beta_I);
    return derive_parent_role_integer_scheme(
        catalogue, zero_word_cap, cycle_word_cap, root_role);
}

}  // namespace ravel::proof
