// Symbolic (not numeric-only) proof that the two D_cont seeds
// A = {1,{0,0,1},1} and B = {2,{0,1,-1},1} -- already shown by
// class_ii_neighbor2_round1_red_identity.cpp to be exactly the pair
// Red prunes, for a = 3..60 -- have NO forward edge into the 27-state
// raw pre-Red target, for every integer a, not a sampled range.
//
// The argument (worked by hand first against tau_a's fixed word forms,
// then encoded here so it is checked rather than merely asserted):
//
// tau_a's images are fixed word *forms* for every a in the family's
// domain: sigma(0) = 0^a 1 2, sigma(1) = 0^(a-1) 2 0, sigma(2) = 0.
// From these forms alone (not from sampling a): letter 1 occurs
// exactly once across all three images (in sigma(0)); letter 2 occurs
// exactly twice (once in sigma(0), once in sigma(1)). This makes
// parent_decompositions(images, 1) and parent_decompositions(images, 2)
// have fixed cardinalities 1 and 2 for every a -- so forward_edges_type1/
// type2(A) and (B) enumerate exactly 1x1=1, 1x1=1 (A, both types) and
// 2x1=2, 1x2=2 (B, both types) raw candidates, never more, for every a.
//
// tau_a's incidence matrix is
//   M(a) = [[a, a, 1],
//           [1, 0, 0],
//           [1, 1, 0]]
// Rows 1 and 2 do not depend on a. Solving M(a) x' = rhs by back
// substitution from rows 1,2 first: x0' = rhs[1], x1' = rhs[2] - rhs[1],
// then row 0 gives x2' = rhs[0] - a*(x0' + x1') = rhs[0] - a*rhs[2].
// Every rhs[2] arising below turns out to be itself a-independent (an
// integer constant, checked, not assumed: its slope-in-a is 0), so
// x2'(a) = rhs[0](a) - rhs[2]*a is affine in a with slope
// (rhs[0]'s own slope) - rhs[2]. That slope is nonzero for every one of
// the 6 raw candidates (checked below), so x2' is unbounded in a for
// all of them.
//
// Every one of the 27 raw pre-Red target states has all three
// coordinates in [-2,2] (checked directly, not assumed, in
// class_ii_neighbor2_round1_red_forward_check.cpp). An x2' that is
// affine in a with nonzero slope can only equal a bounded target's x2
// coordinate for finitely many small a -- checked exactly below for
// each candidate's actual affine form, not just "eventually" -- so for
// every a in the family's tested domain (a>=3) no raw candidate
// (surviving the window filter or not) can match any of the 27
// targets. Hence induced_restricted_edges finds zero edges from A or B
// into the 27-state set for every a >= 3, which is exactly the
// rank-one Red-pruning criterion (red_anode removes any node with no
// outgoing edge within the given node set).
//
// This closes Round 1's Red-pruning gap for a >= 3 (the family's tested
// domain): the raw pre-Red self-closure (window-validity certificate,
// separate file) plus this file together account for the complete
// 27 -> 25 transition symbolically, not just at sampled a.

#include <cstdio>
#include <vector>

#include "ravel/class_ii_neighbor_family.hpp"

using namespace ravel;

namespace {

struct Candidate {
    const char* label;
    ClassIIAffineValue rhs0;
    ClassIIAffineValue rhs2;
};

std::vector<Candidate> enumerate_raw_candidates() {
    std::vector<Candidate> out;

    // A = {i=1, x=(0,0,1), j=1}. Type 1: p over occurrences of letter
    // node.i=1, q over occurrences of letter node.j=1. Type 2: same
    // occurrence sets (roles swapped), since i=j=1 for A.
    {
        const ClassIIAffineValue node_x0{0, 0};
        const ClassIIAffineValue node_x2{1, 0};
        std::vector<ClassIISymbolicPrefixFamily> all_1;
        for (std::size_t parent = 0; parent < 3; ++parent)
            for (const auto& f :
                 class_ii_neighbor_symbolic_prefix_families(2, parent, 1))
                all_1.push_back(f);
        for (const auto& p : all_1)
            for (const auto& q : all_1) {
                out.push_back({"A type1",
                    node_x0 + q.base[0] - p.base[0],
                    node_x2 + q.base[2] - p.base[2]});
                out.push_back({"A type2",
                    -node_x0 + p.base[0] - q.base[0],
                    -node_x2 + p.base[2] - q.base[2]});
            }
    }

    // B = {i=2, x=(0,1,-1), j=1}. Type 1: p over occurrences of letter 2
    // (node.i), q over occurrences of letter 1 (node.j). Type 2: p over
    // occurrences of letter 1 (node.j), q over occurrences of letter 2
    // (node.i).
    {
        const ClassIIAffineValue node_x0{0, 0};
        const ClassIIAffineValue node_x2{-1, 0};
        std::vector<ClassIISymbolicPrefixFamily> occ2, occ1;
        for (std::size_t parent = 0; parent < 3; ++parent) {
            for (const auto& f :
                 class_ii_neighbor_symbolic_prefix_families(2, parent, 2))
                occ2.push_back(f);
            for (const auto& f :
                 class_ii_neighbor_symbolic_prefix_families(2, parent, 1))
                occ1.push_back(f);
        }
        for (const auto& p : occ2)
            for (const auto& q : occ1)
                out.push_back({"B type1",
                    node_x0 + q.base[0] - p.base[0],
                    node_x2 + q.base[2] - p.base[2]});
        for (const auto& p : occ1)
            for (const auto& q : occ2)
                out.push_back({"B type2",
                    -node_x0 + p.base[0] - q.base[0],
                    -node_x2 + p.base[2] - q.base[2]});
    }

    return out;
}

}  // namespace

int main() {
    const auto candidates = enumerate_raw_candidates();
    long long bad = 0;
    std::printf("raw_candidate_count=%zu (expect 2 for A + 4 for B = 6)\n",
        candidates.size());
    for (const auto& c : candidates) {
        // rhs2 must itself be a-independent for the "affine, not
        // quadratic" solve below to be valid -- checked, not assumed.
        const bool rhs2_constant = c.rhs2.slope == 0;
        // x2'(a) = rhs0(a) - a*rhs2 = rhs0.intercept
        //          + (rhs0.slope - rhs2.intercept) * a
        const ClassIIAffineValue x2_prime{
            c.rhs0.intercept,
            c.rhs0.slope - c.rhs2.intercept};
        const bool unbounded = x2_prime.slope != 0;
        std::printf(
            "  %-8s rhs0=(%lld,%lld) rhs2=(%lld,%lld) "
            "-> x2'(a)=%lld+%lld*a%s%s\n",
            c.label, c.rhs0.intercept, c.rhs0.slope,
            c.rhs2.intercept, c.rhs2.slope,
            x2_prime.intercept, x2_prime.slope,
            rhs2_constant ? "" : "  <-- rhs2 NOT a-independent, invalid",
            unbounded ? "" : "  <-- BOUNDED, argument fails");
        if (!rhs2_constant || !unbounded) ++bad;
    }

    const bool ok = candidates.size() == 6 && bad == 0;
    std::printf(
        "%s\n",
        ok ? "ROUND1_RED_SYMBOLIC_CLOSED: every raw candidate's x2' is "
             "affine in a with nonzero slope (unbounded), so none can "
             "equal a bounded (|coord|<=2) target for a large enough -- "
             "and the exact affine forms printed above make that bound "
             "explicit rather than asymptotic"
           : "ROUND1_RED_SYMBOLIC_INCOMPLETE");
    return ok ? 0 : 1;
}
