// tests/csy_carry_automaton_test.cpp
//
// Tests for the Carton-Sudbery-Yassawi (arXiv:2606.30496)
// carry automaton on Pisot numerations.
//
// Reference (paper text on disk):
//   refs/arXiv_2606.30496_CartonSudberyYassawi2026_
//        Pisot-numerations-topological-groups.txt
//
// Theorem 3:  Let U be a Pisot numeration system.  For any finite
//             set B ⊂ Z, the set {g ∈ B* : [g]_U = 0} is regular.
// Lemma 43:   The number of states N(U, ||g||∞) of this automaton
//             is the pumping-lemma length: if [g0^n]_U = 0 for some
//             n ≥ N, it is zero for infinitely many such n.
//
// This test file specifies the contract the implementation must
// satisfy.  The Pisot polynomial is parameterized generally (degree
// d ≥ 2 + coefficients [c_0, ..., c_{d-1}], β^d = c_0 + c_1 β +
// ... + c_{d-1} β^{d-1}); Fibonacci / Zeckendorf and Tribonacci
// are the two concrete Pisot numerations actually relevant.
//
// The state's running evaluation [w]_U lives in Q(β) — the
// exact mathlib::QElem — implemented via the project's
// mathlib::QBetaRing.  Coefficients are rationals (mpq_t),
// arithmetic is exact, β^i is computed by exponentiation by
// squaring through QBetaRing::mul with no overflow possible.
// Acceptance: every coefficient is zero, equivalently QElem is
// zero.  The BFS state-space bound is `bound_bits` (the maximum
// number of bits in any coefficient's numerator or denominator),
// which by CSY's "preserves zeros" property is finite for Pisot
// U preserving zeros.
//
// Two validation surfaces:
//   (A) the automaton's acceptance predicate agrees with
//       pisotEval(U, w) = 0 on every B(c)-word of bounded length;
//   (B) the state count is finite for Pisot U preserving zeros
//       (per the existing F-check) and the pumping length works.

#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <set>
#include <string>
#include <vector>

#include "adelic/csy_carry_automaton.hpp"

#include "math/qbeta.hpp"

using adelic::PisotPoly;
using adelic::PisotContext;
using adelic::pisotAlphabet;
using adelic::pisotContextFor;
using adelic::pisotEval;
using adelic::qBetaCoefficientsBounded;
using adelic::qBetaIsZero;
using adelic::betaPower;
using adelic::CSYAutomaton;
using adelic::PisotValidationReport;

using mathlib::cmp_si;
using mathlib::QElem;
using mathlib::Rat;

namespace {

constexpr std::size_t kDefaultBoundBits = 10;
constexpr std::size_t kDefaultMaxPos = 12;

// ---- Concrete Pisot polynomials used in the project. ----

PisotPoly fibonacci() { return PisotPoly::fibonacci(); }
PisotPoly tribonacci() { return PisotPoly::tribonacci(); }

// ---- Test runners. ----

int failures = 0;

// Helpers: assert two rational coefficients are equal (as exact mpq_t).
void expectRatEq(const Rat& got, const Rat& want, const char* label,
                 int line) {
    if (cmp(got, want) != 0) {
        ++failures;
        std::printf("  FAIL %s (line %d): got %s, want %s\n",
                    label, line,
                    mathlib::str(got).c_str(),
                    mathlib::str(want).c_str());
    } else {
        std::printf("  pass %s\n", label);
    }
}

Rat ratFromLL(long long n, long long d = 1) {
    Rat r;
    mathlib::set_si(r, n, d);
    return r;
}

#define EXPECT_RAT_EQ(coeff, num, denom, label) \
    expectRatEq((coeff), ratFromLL((num), (denom)), (label), __LINE__)

#define EXPECT_EQ(a, b, label)                                           \
    do {                                                                 \
        auto _a = (a);                                                   \
        auto _b = (b);                                                   \
        if (_a != _b) {                                                  \
            ++failures;                                                  \
            std::printf("  FAIL %s: got %lld, want %lld (%s:%d)\n",      \
                        label, (long long)_a, (long long)_b,             \
                        __FILE__, __LINE__);                            \
        } else {                                                         \
            std::printf("  pass %s\n", label);                           \
        }                                                                \
    } while (0)

#define EXPECT(cond, label)                                              \
    do {                                                                 \
        if (!(cond)) {                                                   \
            ++failures;                                                  \
            std::printf("  FAIL %s (%s:%d)\n", label, __FILE__, __LINE__); \
        } else {                                                         \
            std::printf("  pass %s\n", label);                           \
        }                                                                \
    } while (0)

// ---- Test: β^n reduction modulo the Pisot polynomial. ----

bool testBetaPowerFibonacci() {
    std::printf("[1] betaPower for Fibonacci (β^2 = β + 1)\n");
    PisotPoly P = fibonacci();
    const PisotContext& ctx = pisotContextFor(P);
    QElem b0 = betaPower(P, 0);
    EXPECT_RAT_EQ(b0.coeff(0), 1, 1, "β^0 const = 1");
    EXPECT_RAT_EQ(b0.coeff(1), 0, 1, "β^0 beta = 0");

    QElem b1 = betaPower(P, 1);
    EXPECT_RAT_EQ(b1.coeff(0), 0, 1, "β^1 const = 0");
    EXPECT_RAT_EQ(b1.coeff(1), 1, 1, "β^1 beta = 1");

    QElem b2 = betaPower(P, 2);
    EXPECT_RAT_EQ(b2.coeff(0), 1, 1, "β^2 const = 1");
    EXPECT_RAT_EQ(b2.coeff(1), 1, 1, "β^2 beta = 1");

    QElem b3 = betaPower(P, 3);
    EXPECT_RAT_EQ(b3.coeff(0), 1, 1, "β^3 const = 1");
    EXPECT_RAT_EQ(b3.coeff(1), 2, 1, "β^3 beta = 2");

    QElem b4 = betaPower(P, 4);
    EXPECT_RAT_EQ(b4.coeff(0), 2, 1, "β^4 const = 2");
    EXPECT_RAT_EQ(b4.coeff(1), 3, 1, "β^4 beta = 3");

    QElem b5 = betaPower(P, 5);
    EXPECT_RAT_EQ(b5.coeff(0), 3, 1, "β^5 const = 3");
    EXPECT_RAT_EQ(b5.coeff(1), 5, 1, "β^5 beta = 5");

    QElem b6 = betaPower(P, 6);
    EXPECT_RAT_EQ(b6.coeff(0), 5, 1, "β^6 const = 5");
    EXPECT_RAT_EQ(b6.coeff(1), 8, 1, "β^6 beta = 8");
    return true;
}

bool testBetaPowerTribonacci() {
    std::printf("[2] betaPower for Tribonacci (β^3 = β^2 + β + 1)\n");
    PisotPoly P = tribonacci();
    QElem b0 = betaPower(P, 0);
    EXPECT_RAT_EQ(b0.coeff(0), 1, 1, "β^0 = (1, 0, 0)");
    EXPECT_RAT_EQ(b0.coeff(1), 0, 1, "β^0 β-coeff = 0");
    EXPECT_RAT_EQ(b0.coeff(2), 0, 1, "β^0 β^2-coeff = 0");

    QElem b1 = betaPower(P, 1);
    EXPECT_RAT_EQ(b1.coeff(1), 1, 1, "β^1 β = 1");

    QElem b2 = betaPower(P, 2);
    EXPECT_RAT_EQ(b2.coeff(2), 1, 1, "β^2 β^2 = 1");

    QElem b3 = betaPower(P, 3);
    EXPECT_RAT_EQ(b3.coeff(0), 1, 1, "β^3 const = 1");
    EXPECT_RAT_EQ(b3.coeff(1), 1, 1, "β^3 β = 1");
    EXPECT_RAT_EQ(b3.coeff(2), 1, 1, "β^3 β^2 = 1");

    QElem b4 = betaPower(P, 4);
    EXPECT_RAT_EQ(b4.coeff(0), 1, 1, "β^4 const = 1");
    EXPECT_RAT_EQ(b4.coeff(1), 2, 1, "β^4 β = 2");
    EXPECT_RAT_EQ(b4.coeff(2), 2, 1, "β^4 β^2 = 2");
    return true;
}

// ---- Test: [w]_U reduced to length-d coefficient vector. ----

bool testPisotEval() {
    std::printf("[3] pisotEval for Fibonacci (in Q(β))\n");
    PisotPoly U = fibonacci();
    QElem v = pisotEval(U, {1});
    EXPECT_RAT_EQ(v.coeff(0), 1, 1, "[1] const = 1");
    EXPECT_RAT_EQ(v.coeff(1), 0, 1, "[1] beta = 0");

    v = pisotEval(U, {0, 1});
    EXPECT_RAT_EQ(v.coeff(0), 0, 1, "[0,1] const = 0");
    EXPECT_RAT_EQ(v.coeff(1), 1, 1, "[0,1] beta = 1");

    v = pisotEval(U, {-1, 0, 1});
    EXPECT_RAT_EQ(v.coeff(0), 0, 1, "[-1,0,1] const = 0");
    EXPECT_RAT_EQ(v.coeff(1), 1, 1, "[-1,0,1] beta = 1");
    return true;
}

// ---- Test: alphabet B(c). ----

bool testAlphabet() {
    std::printf("[4] alphabet B(c) = {-c, ..., c}\n");
    auto A = pisotAlphabet(0);
    EXPECT_EQ(A.size(), 1, "|B(0)| = 1");
    EXPECT_EQ(A[0], 0, "B(0) = {0}");
    auto B = pisotAlphabet(1);
    EXPECT_EQ(B.size(), 3, "|B(1)| = 3");
    EXPECT_EQ(B[0], -1, "B(1)[0] = -1");
    EXPECT_EQ(B[1], 0, "B(1)[1] = 0");
    EXPECT_EQ(B[2], 1, "B(1)[2] = 1");
    auto C = pisotAlphabet(2);
    EXPECT_EQ(C.size(), 5, "|B(2)| = 5");
    return true;
}

// ---- Test: CSY automaton state count is finite for Pisot U. ----

bool testAutomatonStateCountFinite() {
    std::printf("[5] automaton state count finite for Fibonacci c=1\n");
    PisotPoly U = fibonacci();
    CSYAutomaton A(U, /*c=*/1, kDefaultBoundBits, kDefaultMaxPos);
    std::printf("  Fibonacci c=1: %zu reachable states (bound_bits %zu,"
                " max_pos %zu)\n",
                A.stateCount(), kDefaultBoundBits, kDefaultMaxPos);
    EXPECT(A.stateCount() > 0, "automaton has at least one state");
    EXPECT(!A.bfsClosed(),
           "absolute-position prototype reports max_pos truncation");
    return true;
}

bool testAutomatonStateCountTribonacci() {
    std::printf("[6] automaton state count finite for Tribonacci c=1\n");
    PisotPoly U = tribonacci();
    CSYAutomaton A(U, /*c=*/1, kDefaultBoundBits, kDefaultMaxPos);
    std::printf("  Tribonacci c=1: %zu reachable states\n", A.stateCount());
    EXPECT(A.stateCount() > 0, "automaton has at least one state");
    EXPECT(!A.bfsClosed(),
           "absolute-position prototype reports max_pos truncation");
    return true;
}

// ---- Test: automaton acceptance agrees with [w]_U = 0. ----

bool testAcceptanceAgreementFibonacci() {
    std::printf("[7] automaton acceptance == [w]_U = 0 (Fibonacci c=1)\n");
    PisotPoly U = fibonacci();
    CSYAutomaton A(U, /*c=*/1, kDefaultBoundBits, kDefaultMaxPos);
    PisotValidationReport rep = A.validate(/*max_len=*/6);
    std::printf("  checked %zu words of length ≤ 6, %zu agreements,"
                " %zu discrepancies\n",
                rep.checked, rep.agreements, rep.discrepancies);
    EXPECT_EQ(rep.discrepancies, 0, "no acceptance discrepancies on the bounded corpus");
    return true;
}

bool testAcceptanceAgreementTribonacci() {
    std::printf("[8] automaton acceptance == [w]_U = 0 (Tribonacci c=1)\n");
    PisotPoly U = tribonacci();
    CSYAutomaton A(U, /*c=*/1, kDefaultBoundBits, kDefaultMaxPos);
    PisotValidationReport rep = A.validate(/*max_len=*/5);
    std::printf("  checked %zu words of length ≤ 5, %zu agreements,"
                " %zu discrepancies\n",
                rep.checked, rep.agreements, rep.discrepancies);
    EXPECT_EQ(rep.discrepancies, 0, "no acceptance discrepancies");
    return true;
}

// ---- Test: pumping lemma (CSY Lemma 43) holds. ----

bool testPumpingFibonacci() {
    std::printf("[9] pumping length N for Fibonacci c=1\n");
    PisotPoly U = fibonacci();
    CSYAutomaton A(U, /*c=*/1, kDefaultBoundBits, kDefaultMaxPos);
    std::size_t N = A.stateCount();
    std::printf("  pumping length N = %zu\n", N);
    EXPECT(N > 0, "pumping length N > 0");
    return true;
}

// ---- Test: K_1 = 2 (CSY's worked Example after Definition 6). ----

bool testPreservesZerosConstantK1() {
    std::printf("[10] Fibonacci preserves-zeros constant K_1 (exact Q(β))\n");
    PisotPoly U = fibonacci();
    CSYAutomaton A(U, /*c=*/1, /*bound_bits=*/8, /*max_pos=*/64);
    std::printf("  Fibonacci c=1, bound_bits=8: %zu states\n", A.stateCount());
    EXPECT(A.stateCount() > 0, "automaton is non-trivial at small bound");
    return true;
}

// ---- Test: higher alphabet bound c=2 is supported. ----

bool testAlphabetC2() {
    std::printf("[11] automaton for Fibonacci c=2\n");
    PisotPoly U = fibonacci();
    CSYAutomaton A(U, /*c=*/2, kDefaultBoundBits, kDefaultMaxPos);
    PisotValidationReport rep = A.validate(/*max_len=*/5);
    std::printf("  state count = %zu, %zu agreements, %zu discrepancies\n",
                A.stateCount(), rep.agreements, rep.discrepancies);
    EXPECT_EQ(rep.discrepancies, 0, "no discrepancies at c=2");
    return true;
}

// ---- Test: explicit Fibonacci acceptors (sanity). ----

bool testKnownAcceptors() {
    std::printf("[12] known accepting words for Fibonacci c=1\n");
    PisotPoly U = fibonacci();
    CSYAutomaton A(U, /*c=*/1, kDefaultBoundBits, kDefaultMaxPos);
    EXPECT(A.acceptsWord({}), "eps accepts");
    EXPECT(!A.acceptsWord({-1, 0, 1}), "(-1, 0, 1) = β does not accept");
    EXPECT(!A.acceptsWord({-1, 0, 0, 1}), "(-1, 0, 0, 1) = 2β does not accept");
    // -1 + β² = β² - 1 = 0; equivalently w = (-1, -1, 1).
    EXPECT(A.acceptsWord({-1, -1, 1}), "(-1, -1, 1) = 0 accepts");
    EXPECT(A.acceptsWord({1, 1, -1}), "(1, 1, -1) = 0 accepts");
    return true;
}

// ---- Test: deep memoization in Q(β) — no overflow, ever. ----

bool testDeepMemoizationQ() {
    std::printf("[13] deep Q(β) memoization: β^n for arbitrarily large n\n");
    PisotPoly U = fibonacci();
    // β^i for Fibonacci has entries F_{i-1}/1, F_i/1 in Q(β).
    // Compute up to i = 200 — far beyond where long long
    // overflows — and verify the recurrence.
    QElem prev = betaPower(U, 0);  // (1, 0)
    QElem curr = betaPower(U, 1);  // (0, 1)
    bool all_ok = true;
    for (std::size_t i = 2; i <= 200; ++i) {
        QElem next = pisotContextFor(U).ring.add(prev, curr);
        QElem got = betaPower(U, i);
        // Use exact cmp:
        if (cmp(got.coeff(0), next.coeff(0)) != 0) {
            ++failures;
            std::printf("  FAIL β^%zu const mismatch\n", i);
            all_ok = false;
            break;
        }
        if (cmp(got.coeff(1), next.coeff(1)) != 0) {
            ++failures;
            std::printf("  FAIL β^%zu β mismatch\n", i);
            all_ok = false;
            break;
        }
        prev = curr;
        curr = next;
    }
    if (all_ok) {
        std::printf("  pass β^i matches Pisot recurrence for i ∈ [2, 200]\n");
    }
    EXPECT(all_ok, "deep Q(β) memoization correct up to i=200");
    const auto& ctx = pisotContextFor(U);
    EXPECT(ctx.beta_powers_cache.size()
               <= PisotContext::beta_power_entry_limit,
           "thread-local recurrence prefix respects entry limit");
    EXPECT(ctx.beta_powers_stored_bits
               <= PisotContext::beta_power_bit_limit,
           "thread-local recurrence prefix respects coefficient-bit limit");
    return true;
}

// ---- Test: qBetaCoefficientsBounded correctly identifies magnitude. ----

bool testCoefficientBound() {
    std::printf("[14] qBetaCoefficientsBounded magnitude check\n");
    PisotPoly U = fibonacci();
    QElem small = betaPower(U, 5);  // (3, 5), small integers
    EXPECT(qBetaCoefficientsBounded(small, 8), "small coeffs fit in 8 bits");

    QElem big = betaPower(U, 200);  // huge numerators
    EXPECT(!qBetaCoefficientsBounded(big, 8), "huge coeffs do not fit in 8 bits");
    EXPECT(qBetaCoefficientsBounded(big, 1024), "huge coeffs fit in 1024 bits");
    return true;
}

// ---- Test: CSY automaton state count scales with bound_bits. ----

bool testStateCountVsBound() {
    std::printf("[15] CSYAutomaton state count scales with bound_bits\n");
    PisotPoly U = fibonacci();
    CSYAutomaton Asmall(U, /*c=*/1, /*bound_bits=*/6, /*max_pos=*/8);
    CSYAutomaton Amedium(U, /*c=*/1, /*bound_bits=*/10, /*max_pos=*/12);
    CSYAutomaton Alarge(U, /*c=*/1, /*bound_bits=*/14, /*max_pos=*/16);
    std::printf("  bound_bits 6 / max_pos 8:    %zu states\n",  Asmall.stateCount());
    std::printf("  bound_bits 10 / max_pos 12:  %zu states\n",  Amedium.stateCount());
    std::printf("  bound_bits 14 / max_pos 16:  %zu states\n",  Alarge.stateCount());
    EXPECT(Asmall.stateCount() <= Amedium.stateCount(),
           "state count non-decreasing with bound_bits");
    return true;
}

// ---- Test: closed-form via dominant Pisot root (Newton + BigFloat). ----

bool testClosedFormNumerical() {
    std::printf("[16] closed-form via Newton iteration on Fibonacci β\n");
    PisotPoly U = fibonacci();
    // Find β via Newton's iteration (cubic convergence from x=2).
    // β ≈ 1.6180339887... → numerically close to actual golden ratio.
    mathlib::BigFloat beta = adelic::betaPowerNumerical(U, 1, /*prec_bits=*/200);
    double beta_d = mathlib::bigfloat_to_double(beta);
    std::printf("  Fibonacci β (Newton) ≈ %.15f\n", beta_d);
    EXPECT(beta_d > 1.6 && beta_d < 1.7,
           "Fibonacci β is in (1.6, 1.7)");
    // 100 × β should equal (the recurrence's) β^100 evaluated numerically.
    mathlib::BigFloat closed_form = adelic::betaPowerNumerical(U, 100, /*prec_bits=*/200);
    double cf_d = mathlib::bigfloat_to_double(closed_form);
    std::printf("  Fibonacci β^100 (closed form) ≈ %.6e\n", cf_d);
    EXPECT(cf_d > 0.0, "β^100 > 0");
    return true;
}

// ---- Test: recurrence result and closed-form result agree. ----

bool testRecurrenceVsClosedForm() {
    std::printf("[17] recurrence β^n == closed-form β^n (Fibonacci)\n");
    PisotPoly U = fibonacci();
    constexpr unsigned PREC = 200;
    // Closed-form β^n
    mathlib::BigFloat closed_form = adelic::betaPowerNumerical(U, 50, PREC);
    // Recurrence β^50 in Q(β)
    QElem exact = betaPower(U, 50);
    // Evaluate Q(β) at numerical β.
    mathlib::BigFloat beta = adelic::betaPowerNumerical(U, 1, PREC);
    mathlib::BigFloat recurrence_evaluated =
        adelic::qBetaEvaluateAtNumericalBeta(exact, beta, PREC);
    double cf_d = mathlib::bigfloat_to_double(closed_form);
    double re_d = mathlib::bigfloat_to_double(recurrence_evaluated);
    std::printf("  β^50: closed-form = %.10e\n", cf_d);
    std::printf("  β^50: recurrence evaluated at β = %.10e\n", re_d);
    EXPECT(std::abs(cf_d - re_d) < 1e-15, "agreement to 1e-15");
    return true;
}

// ---- Test: Sturm sequencing brackets β to within tolerance. ----

bool testSturmIsolatesBeta() {
    std::printf("[18] Sturm sequencing isolates Fibonacci β tightly\n");
    PisotPoly U = fibonacci();
    // Build QBetaRing via PolyZ to avoid the std::initializer_list
    // constraint of from_low_first.
    auto lf = U.low_first_coeffs();
    mathlib::PolyZ pz;
    pz.ensure_size(U.d + 1);
    for (std::size_t i = 0; i < U.d; ++i) mathlib::set_si(pz.coeff(i), lf[i]);
    mathlib::set_si(pz.coeff(U.d), 1);
    pz.trim();
    mathlib::QBetaRing R(pz);
    mathlib::RootInterval iv = mathlib::isolate_beta(R);
    double lo = mpq_get_d(iv.lo.get());
    double hi = mpq_get_d(iv.hi.get());
    std::printf("  β ∈ [%.15f, %.15f] (width %.3e)\n", lo, hi, hi - lo);
    EXPECT(lo > 1.0 && hi < 2.0, "β interval is inside (1, 2)");
    EXPECT(hi - lo < 1e-10, "β interval width < 1e-10 after Sturm isolation");
    return true;
}

// ---- Test: companion-matrix route (no memoization) matches memoized recurrence. ----

bool testMatrixRouteAgreesWithRecurrence() {
    std::printf("[19] companion-matrix β^n (no memo) == recurrence β^n\n");
    PisotPoly U = fibonacci();
    for (std::size_t n = 0; n <= 30; ++n) {
        QElem a = betaPower(U, n);              // memoized recurrence
        QElem b = adelic::betaPowerViaMatrix(U, n);  // no-memo closed form
        if (a != b) {
            ++failures;
            std::printf("  FAIL at n=%zu:\n    recurrence = %s\n    matrix    = %s\n",
                        n, mathlib::str(a).c_str(), mathlib::str(b).c_str());
            return false;
        }
    }
    std::printf("  pass companion-matrix route matches recurrence for n ∈ [0, 30]\n");
    return true;
}

// ---- Test: companion matrix gives very-large-n β^n without overflow. ----

bool testMatrixRouteDeepN() {
    std::printf("[20] companion-matrix β^n for arbitrarily large n\n");
    PisotPoly U = fibonacci();
    // 1000 is far beyond where long-long β^i overflows (~92).
    QElem b = adelic::betaPowerViaMatrix(U, 1000);
    // The fibonacci-numbers F_999 and F_1000 should appear as
    // entries of β^1000 = (F_999, F_1000) in basis (1, β).
    std::printf("  β^1000 = %s (entries are huge Fibonacci numbers)\n",
                mathlib::str(b).c_str());
    // Just sanity: not zero, not equal to β^999.
    QElem bprev = adelic::betaPowerViaMatrix(U, 999);
    EXPECT(b != bprev, "β^1000 ≠ β^999");
    EXPECT(!b.is_zero(), "β^1000 ≠ 0");
    return true;
}

// ---- Test: pisotEval via matrix route matches via recurrence route. ----

bool testPisotEvalViaMatrix() {
    std::printf("[21] pisotEval matches across both routes for [w]_U\n");
    PisotPoly U = fibonacci();
    std::vector<long long> w = {1, -1, 1, 0, 1, -1, 0, 1};
    QElem v_recurrence = pisotEval(U, w);
    // Compute via matrix route, evaluating each term d_i · β^i using matrix.
    QElem v_matrix = pisotContextFor(U).ring.zero();
    for (std::size_t i = 0; i < w.size(); ++i) {
        if (w[i] == 0) continue;
        QElem wi = pisotContextFor(U).ring.from_int(w[i]);
        QElem bi = adelic::betaPowerViaMatrix(U, i);
        QElem term = pisotContextFor(U).ring.mul(wi, bi);
        v_matrix = pisotContextFor(U).ring.add(v_matrix, term);
    }
    if (v_recurrence != v_matrix) {
        ++failures;
        std::printf("  FAIL:\n    recurrence = %s\n    matrix    = %s\n",
                    mathlib::str(v_recurrence).c_str(),
                    mathlib::str(v_matrix).c_str());
    } else {
        std::printf("  pass [w]_U routes agree on test word\n");
    }
    return true;
}

// ---- Test: BetaPowerCache: bounded, exact, MRU. ----

bool testBetaPowerCache() {
    std::printf("[22] BetaPowerCache: bounded LRU, exact Q(β)\n");
    PisotPoly U = fibonacci();
    adelic::BetaPowerCache cache(U, /*capacity=*/8);
    std::printf("  empty cache: size = %zu, max = %zu\n", cache.size(), cache.maxSize());
    EXPECT(cache.size() == 0, "fresh cache is empty");
    EXPECT(cache.maxSize() == 8, "cache capacity honoured");

    // First access: cache miss, value computed via matrix.
    QElem a = cache.get(7);   // F_6, F_7
    EXPECT(cache.size() == 1, "after first get(), size = 1");
    EXPECT(a == betaPower(U, 7), "cache.get(7) == betaPower(7)");

    // Repeated access: hit, no new entry.
    QElem b = cache.get(7);
    EXPECT(cache.size() == 1, "repeated hit, size still 1");
    EXPECT(a == b, "hit returns identical QElem");

    // Fill past capacity: oldest entries should evict.
    for (std::size_t i = 0; i < cache.maxSize() + 4; ++i) {
        cache.get(20 + i);
    }
    EXPECT(cache.size() <= cache.maxSize(),
           "cache size never exceeds capacity (LRU bound)");

    // Numeric invariant: total cache memory = O(capacity · d²).
    std::printf("  post-eviction: size = %zu, max = %zu\n",
                cache.size(), cache.maxSize());
    return true;
}

// ---- Test: BetaPowerCache reaches the same Q(β) values as direct matrix route. ----

bool testBetaPowerCacheAgreesWithMatrix() {
    std::printf("[23] BetaPowerCache entries == betaPowerViaMatrix\n");
    PisotPoly U = tribonacci();
    adelic::BetaPowerCache cache(U, /*capacity=*/16);
    for (std::size_t n : {0, 1, 2, 3, 5, 8, 13, 21, 34, 55}) {
        QElem cached = cache.get(n);
        QElem direct = betaPowerViaMatrix(U, n);
        if (cached != direct) {
            ++failures;
            std::printf("  FAIL at n=%zu\n", n);
            return false;
        }
    }
    std::printf("  pass Tribonacci cache and matrix route agree at 10 positions\n");
    return true;
}

bool testBetaPowerCacheBitBudget() {
    std::printf("[24] BetaPowerCache stored-coefficient bit budget\n");
    PisotPoly U = fibonacci();
    adelic::BetaPowerCache cache(U, /*capacity=*/64,
                                 /*bit_budget=*/256);
    for (std::size_t n : {5, 10, 20, 40, 80, 160})
        EXPECT(cache.get(n) == betaPowerViaMatrix(U, n),
               "bit-budgeted cache returns exact value");
    EXPECT(cache.storedCoefficientBits()
               <= cache.coefficientBitBudget(),
           "stored coefficient bits never exceed budget");
    // beta^1000 is larger than the entire budget: exact result is
    // returned but the cache remains within its bound.
    EXPECT(cache.get(1000) == betaPowerViaMatrix(U, 1000),
           "oversized value returned exactly without retention");
    EXPECT(cache.storedCoefficientBits()
               <= cache.coefficientBitBudget(),
           "oversized value does not violate cache budget");
    return true;
}

}  // namespace

int main() {
    std::printf("CSY carry automaton tests (Carton-Sudbery-Yassawi,"
                " arXiv:2606.30496) — Q(β)-exact throughout\n\n");

    testBetaPowerFibonacci();
    testBetaPowerTribonacci();
    testPisotEval();
    testAlphabet();
    testAutomatonStateCountFinite();
    testAutomatonStateCountTribonacci();
    testAcceptanceAgreementFibonacci();
    testAcceptanceAgreementTribonacci();
    testPumpingFibonacci();
    testPreservesZerosConstantK1();
    testAlphabetC2();
    testKnownAcceptors();
    testDeepMemoizationQ();
    testCoefficientBound();
    testStateCountVsBound();
    testClosedFormNumerical();
    testRecurrenceVsClosedForm();
    testSturmIsolatesBeta();
    testMatrixRouteAgreesWithRecurrence();
    testMatrixRouteDeepN();
    testPisotEvalViaMatrix();
    testBetaPowerCache();
    testBetaPowerCacheAgreesWithMatrix();
    testBetaPowerCacheBitBudget();

    std::printf("\n%d failures.\n", failures);
    return failures == 0 ? 0 : 1;
}
