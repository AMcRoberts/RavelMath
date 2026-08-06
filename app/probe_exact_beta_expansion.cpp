// app/probe_exact_beta_expansion.cpp
//
// Resolves Finding 6.5/28's open case (sigma_{0,2}, charpoly x^3-x-2)
// EXACTLY, replacing the floating-point/mpmath approach (which hit a
// real precision wall around 2500 digits at 3000-dps and reported
// UNRESOLVED) with pure integer arithmetic. This is possible because
// the greedy Renyi/Parry recurrence r_i = beta*r_{i-1} - a_i, starting
// from r_0 = 1, NEVER leaves Z[beta] -- beta is an algebraic INTEGER
// (charpoly monic over Z) and a_i is always an integer, so every r_i
// is an exact INTEGER vector in the basis (1, beta, beta^2). No
// rational denominators ever appear, let alone floating-point error --
// this sidesteps the whole precision question that limited the
// earlier approach.
//
// Periodicity detection is then just exact set-membership on integer
// vectors (r_i as a BigInt triple), not a fuzzy period-finder over a
// finite digit prefix -- if r_i repeats a state ever seen before, the
// expansion is provably eventually periodic from there, full stop, no
// ambiguity.
//
// Digit extraction (floor(beta*r_{i-1})) uses qbeta_sign against a
// Sturm-isolated bracket for beta, refined to a very tight rational
// interval up front via isolate_real_root_rat's own bisection (still
// exact -- rational bisection, not floating point).
//
// Run under the standing 10GB / timeout memory discipline; a state
// cap aborts cleanly (not silently) if the eventually-periodic state
// space turns out to be larger than expected.

#include <cstdio>
#include <map>
#include <stdexcept>
#include <string>
#include <vector>

#include "math/qbeta.hpp"
#include "math/sturm.hpp"

using namespace mathlib;

namespace {

// Lifts a QElem to an integer PolyZ by clearing denominators (the
// same construction qbeta_sign uses internally), exposed here so this
// file can check endpoint agreement itself instead of trusting
// qbeta_sign's internal fallback.
PolyZ lift_to_polyz(const QElem& v, std::size_t d) {
    BigInt lcm_den; set_ui(lcm_den, 1);
    for (const auto& c : v.coeffs_) {
        BigInt c_den; mpz_set(c_den.get(), mpq_denref(c.get()));
        BigInt g; mpz_gcd(g.get(), lcm_den.get(), c_den.get());
        BigInt t; divexact(t, lcm_den, g);
        mul(lcm_den, t, c_den);
    }
    PolyZ A; A.ensure_size(d);
    for (std::size_t i = 0; i < d; ++i) {
        BigInt num; mpz_set(num.get(), mpq_numref(v.coeffs_[i].get()));
        BigInt den; mpz_set(den.get(), mpq_denref(v.coeffs_[i].get()));
        BigInt Ai; mul(Ai, num, lcm_den); divexact(Ai, Ai, den);
        set(A.coeff(i), Ai);
    }
    A.trim();
    return A;
}

// qbeta_sign's own fallback (pick sg_lo when the endpoints disagree)
// is only safe if the bracket is already tight enough that no OTHER
// real root of the shifted polynomial lies inside it -- otherwise it
// can silently return the wrong sign. Found that failure mode loud
// (a thrown exception from a corrupted downstream invariant, not a
// silently wrong answer) and fixed it here: check endpoint agreement
// directly, and on disagreement, refine the bracket via further exact
// rational bisection (isolate_real_root_rat, still no floating point)
// until the endpoints genuinely agree, rather than guessing.
int robust_sign(const QElem& v, const QBetaRing& R, const std::vector<PolyZ>& chain, RootInterval& bracket) {
    const std::size_t d = R.degree();
    PolyZ A = lift_to_polyz(v, d);
    for (int attempt = 0; attempt < 4000; ++attempt) {
        int sg_lo = poly_at_rational_sign(A, bracket.lo);
        int sg_hi = poly_at_rational_sign(A, bracket.hi);
        if (sg_lo == sg_hi) return sg_lo;
        bracket = isolate_real_root_rat(chain, bracket.lo, bracket.hi, 8);
    }
    throw std::runtime_error("robust_sign: failed to stabilize after 4000 refinements");
}

// The exact integer coefficients as a joined string (each
// coefficient's Rat is guaranteed to have denominator 1 throughout
// this computation -- checked, not assumed).
std::string exact_int_key(const QElem& v, std::size_t d) {
    std::string s;
    for (std::size_t i = 0; i < d; ++i) {
        BigInt num; mpz_set(num.get(), mpq_numref(v.coeff(i).get()));
        BigInt den; mpz_set(den.get(), mpq_denref(v.coeff(i).get()));
        if (!is_one(den)) throw std::runtime_error("exact_int_key: non-integer coefficient encountered (unexpected)");
        s += str(num);
        s += "|";
    }
    return s;
}

void run(const char* name, std::initializer_list<long long> low_first, long long max_steps, long long state_cap) {
    QBetaRing R = QBetaRing::from_low_first(low_first);
    const std::size_t d = R.degree();
    auto chain = sturm_chain(R.charpoly());

    RootInterval bracket = isolate_beta(R);
    // Refine to a very tight rational bracket up front -- exact
    // rational bisection, 2000 extra halvings, so the interval width
    // is far below anything that could be ambiguous against the
    // integer-coefficient elements we'll be sign-testing.
    bracket = isolate_real_root_rat(chain, bracket.lo, bracket.hi, /*tol=*/2000);

    QElem beta_elem = R.beta_k(1);
    QElem r = R.one();

    // Numeric estimate of beta, just to bound candidate digit values
    // (0..floor(beta)) -- NOT used for correctness, only to avoid
    // scanning an unbounded digit range; correctness comes entirely
    // from qbeta_sign's exact comparisons below.
    double beta_lo_d = mpq_get_d(bracket.lo.get());
    double beta_hi_d = mpq_get_d(bracket.hi.get());
    long long max_digit = static_cast<long long>(beta_hi_d) + 1;

    std::map<std::string, long long> visited;
    std::vector<long long> digits;

    std::printf("=== %s: beta in [%.10f, %.10f], max_digit<=%lld ===\n", name, beta_lo_d, beta_hi_d, max_digit);
    std::fflush(stdout);

    for (long long step = 0; step < max_steps; ++step) {
        if (static_cast<long long>(visited.size()) > state_cap) {
            throw std::runtime_error("state cap exceeded -- aborting safely (see header)");
        }
        QElem prod = R.mul(beta_elem, r);

        // Find a = floor(prod) by exact sign testing: largest k in
        // [0, max_digit] with sign(prod - k) >= 0.
        long long a = -1;
        for (long long k = max_digit; k >= 0; --k) {
            QElem shifted = R.sub(prod, R.from_int(k));
            int sg = robust_sign(shifted, R, chain, bracket);
            if (sg >= 0) { a = k; break; }
        }
        if (a < 0) throw std::runtime_error("floor search failed -- bracket or digit bound wrong");
        digits.push_back(a);

        QElem r_new = R.sub(prod, R.from_int(a));
        // Exact zero check (degree() < 0 iff every coefficient is
        // exactly zero -- no epsilon involved).
        if (r_new.is_zero()) {
            std::printf("  FINITE: terminates at length %lld. digits(first 30)=[", step + 1);
            for (std::size_t i = 0; i < digits.size() && i < 30; ++i) std::printf("%lld ", digits[i]);
            std::printf("]\n");
            return;
        }
        std::string key = exact_int_key(r_new, d);
        auto it = visited.find(key);
        if (it != visited.end()) {
            long long preperiod = it->second;
            long long period = step + 1 - preperiod;
            std::printf("  EVENTUALLY PERIODIC (exact, not floating-point-limited): preperiod=%lld period=%lld\n",
                        preperiod, period);
            std::printf("  digits(first 30)=[");
            for (std::size_t i = 0; i < digits.size() && i < 30; ++i) std::printf("%lld ", digits[i]);
            std::printf("]\n");
            std::printf("  periodic tail (length %lld starting at index %lld)=[", period, preperiod);
            for (long long i = preperiod; i < preperiod + period && i < static_cast<long long>(digits.size()); ++i)
                std::printf("%lld ", digits[i]);
            std::printf("]\n");
            return;
        }
        visited.emplace(std::move(key), step + 1);
        r = std::move(r_new);
        if (step % 500 == 0) {
            std::printf("  ...step %lld, %zu states visited\n", step, visited.size());
            std::fflush(stdout);
        }
    }
    std::printf("  NO REPEAT FOUND within %lld steps (%zu distinct states visited) -- "
                "genuinely inconclusive at this budget, not a precision artifact this time.\n",
                max_steps, visited.size());
}

}  // namespace

int main() {
    // sigma_{0,2}: x^3 - x - 2 -> low_first = {0, -1, -2}
    // (x^3 + 0*x^2 + (-1)*x + (-2)).
    run("sigma_{0,2}  x^3-x-2", {0, -1, -2}, 3000000, 3000000);
    return 0;
}
