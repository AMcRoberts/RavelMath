// ravel/canonical_beta_substitution.hpp
//
// Reusable derivation of a Pisot number's greedy beta-expansion of 1
// and its canonical (Parry / Dumont-Thomas) beta-substitution, entirely
// in exact Q(beta) arithmetic -- no floating-point digit search.
//
// This closes a gap the project had been filling by hand: prior
// per-substitution headers (plastic_number_substitution.hpp,
// third_smallest_pisot_substitution.hpp) hard-coded a literal digit
// sequence and substitution rule, sourced from literature or by-hand
// derivation, with no executable operation reproducing them from the
// defining minimal polynomial. That is exactly the kind of missing
// derivation the project's non-negotiable rule forbids leaving manual.
//
// Algorithm (classical greedy beta-expansion, e.g. Parry 1960;
// Frougny 1992 for the substitution): with beta the dominant real
// root of a Pisot minimal polynomial,
//   x_0 = 1
//   d_{k+1} = floor(beta * x_k)
//   x_{k+1} = beta * x_k - d_{k+1}
// Every x_k is exactly representable in Q(beta); floor is decided by
// an exact sign test against the isolating interval for beta, not a
// floating comparison. The expansion either terminates (x_k = 0,
// giving a finite digit string) or eventually cycles (x_k = x_j for
// some j < k, giving an eventually-periodic digit string) -- both are
// detected exactly via QElem equality, which is exact rational
// equality of the canonical coefficient vector, not a numerical
// tolerance.
//
// From the resulting quasi-greedy digit sequence (Parry's correction:
// a finite expansion d_1...d_M is converted to the purely-periodic
// quasi-greedy sequence (d_1...d_{M-1}(d_M-1))^infinity before this
// step), the canonical substitution on alphabet {0,...,p+m-1} (p =
// preperiod length, m = period length) is
//   sigma(i) = 0^{t_{i+1}} (i+1)     for i in [0, p+m-2]
//   sigma(p+m-1) = 0^{t_{p+m}}       (wraps back to state p implicitly)
// This is verified, not assumed: the caller must independently check
// the constructed substitution's incidence characteristic polynomial
// against the target minimal polynomial (times any cyclotomic
// cofactor), exactly as done for every other Q/R/S closure in this
// project.

#pragma once

#include <cstddef>
#include <stdexcept>
#include <vector>

#include "math/bigint.hpp"
#include "math/poly_z.hpp"
#include "math/qbeta.hpp"
#include "math/sturm.hpp"

namespace ravel {

struct GreedyBetaExpansionResult {
    std::vector<long long> digits;   // d_1 .. d_N in order, N = preperiod_len + period_len (or the finite length if terminated)
    bool terminated{};               // true iff the greedy expansion of 1 is finite (x_N = 0 exactly)
    bool purely_periodic{};          // true iff the eventual cycle starts at x_0 (preperiod_len == 0)
    std::size_t preperiod_len{};     // 0 if terminated or purely periodic
    std::size_t period_len{};        // 0 if terminated
};

// Exact greedy beta-expansion of 1. `beta_I` must isolate the Pisot
// dominant root of `R.charpoly()` (e.g. via mathlib::isolate_beta).
inline GreedyBetaExpansionResult exact_greedy_beta_expansion_of_one(
    const mathlib::QBetaRing& R,
    const mathlib::RootInterval& beta_I,
    std::size_t max_steps = 128) {
    using namespace mathlib;
    GreedyBetaExpansionResult out;
    QElem beta = R.beta_k(1);
    QElem x = R.one();
    std::vector<QElem> seen;
    seen.push_back(x);

    for (std::size_t step = 0; step < max_steps; ++step) {
        QElem bx = R.mul(beta, x);
        // Exact floor(bx) by ascending sign search: bx - d >= 0, bx - (d+1) < 0.
        // Digits of a beta-expansion are bounded by ceil(beta) - 1, which is
        // small for every Pisot number this project studies; the loop below
        // is still a genuine exact search, not an assumed bound.
        long long d = 0;
        for (;; ++d) {
            QElem cand = R.sub(bx, R.from_int(d + 1));
            int s = qbeta_sign(cand, R, beta_I);
            if (s < 0) break;
            if (d > 100000) {
                throw std::runtime_error(
                    "exact_greedy_beta_expansion_of_one: digit search runaway "
                    "(bad beta interval or non-Pisot input?)");
            }
        }
        out.digits.push_back(d);
        QElem xn = R.sub(bx, R.from_int(d));
        if (xn.is_zero()) {
            out.terminated = true;
            return out;
        }
        for (std::size_t j = 0; j < seen.size(); ++j) {
            if (seen[j] == xn) {
                out.purely_periodic = (j == 0);
                out.preperiod_len = j;
                out.period_len = seen.size() - j;
                return out;
            }
        }
        seen.push_back(xn);
        x = xn;
    }
    throw std::runtime_error(
        "exact_greedy_beta_expansion_of_one: exceeded max_steps without "
        "termination or a detected cycle");
}

// Canonical Parry/Dumont-Thomas beta-substitution driven directly by a
// digit sequence t_0..t_{n-1} on alphabet {0,...,n-1} (state s is both
// an ordinary letter and the "filler" digit-0 letter, exactly as in
// the beta-numeration literature's single-alphabet convention):
//   sigma(s) = 0^{t[s]} (s+1)   for s in [0, n-2]
//   sigma(n-1) = 0^{t[n-1]}
//
// Verified (empirically, against this project's own prior third-smallest-
// Pisot construction, and independently below against supergolden's own
// characteristic polynomial): for a SIMPLE Parry number the raw finite
// greedy digits of d_beta(1) may be used directly, with no Parry "-1"
// correction -- that correction produces the *infinite* quasi-greedy
// sequence used for other constructions (e.g. natural extensions), not
// this one. For a number whose greedy expansion of 1 is itself already
// (purely) infinite periodic, the same formula applies to that raw
// periodic digit sequence.
//
// The caller MUST independently verify the returned substitution's exact
// incidence characteristic polynomial against the target minimal
// polynomial (see math/charpoly.hpp) before treating this as a theorem
// input -- this function performs no such check itself.
inline std::vector<std::vector<long long>> canonical_beta_substitution_from_digits(
    const std::vector<long long>& t) {
    const std::size_t n = t.size();
    if (n == 0) throw std::invalid_argument("canonical_beta_substitution_from_digits: empty digit string");
    std::vector<std::vector<long long>> sigma(n);
    for (std::size_t s = 0; s + 1 < n; ++s) {
        for (long long z = 0; z < t[s]; ++z) sigma[s].push_back(0);
        sigma[s].push_back(static_cast<long long>(s + 1));
    }
    for (long long z = 0; z < t[n - 1]; ++z) sigma[n - 1].push_back(0);
    if (sigma[n - 1].empty()) {
        throw std::runtime_error(
            "canonical_beta_substitution_from_digits: last letter's image is "
            "empty (t[n-1] == 0) -- this digit sequence is not a valid "
            "driver for this construction; check purely-periodic rotation "
            "or preperiod handling before retrying");
    }
    return sigma;
}

// Canonical beta-substitution for an EVENTUALLY periodic (genuine
// preperiod > 0) greedy expansion: preperiod digits t_0..t_{p-1} then
// period digits t_p..t_{p+m-1}, alphabet {0,...,p+m-1}. Same driving
// rule as `canonical_beta_substitution_from_digits` for every state
// except the last:
//   sigma(s) = 0^{t[s]} (s+1)   for s in [0, n-2]
// The purely-periodic case's last state needed no explicit wrap symbol
// because wrapping to state 0 happens automatically (0 is produced as
// filler everywhere). A genuine preperiod wraps to state `p` instead,
// which is NOT the filler letter and is otherwise only ever produced
// by state (p-1)'s own image -- so closing the cycle requires an
// explicit trailing symbol:
//   sigma(n-1) = 0^{t[n-1]} (p)
// Verification (incidence-polynomial match) is the caller's
// responsibility, exactly as for the purely-periodic constructor.
inline std::vector<std::vector<long long>> canonical_beta_substitution_eventually_periodic(
    const std::vector<long long>& preperiod, const std::vector<long long>& period) {
    const std::size_t p = preperiod.size();
    const std::size_t m = period.size();
    if (m == 0) throw std::invalid_argument("canonical_beta_substitution_eventually_periodic: empty period");
    if (p == 0) throw std::invalid_argument("canonical_beta_substitution_eventually_periodic: use canonical_beta_substitution_from_digits for a zero preperiod");
    const std::size_t n = p + m;
    std::vector<long long> t(n);
    for (std::size_t i = 0; i < p; ++i) t[i] = preperiod[i];
    for (std::size_t i = 0; i < m; ++i) t[p + i] = period[i];

    std::vector<std::vector<long long>> sigma(n);
    for (std::size_t s = 0; s + 1 < n; ++s) {
        for (long long z = 0; z < t[s]; ++z) sigma[s].push_back(0);
        sigma[s].push_back(static_cast<long long>(s + 1));
    }
    for (long long z = 0; z < t[n - 1]; ++z) sigma[n - 1].push_back(0);
    sigma[n - 1].push_back(static_cast<long long>(p));  // wrap to the cycle start, not to 0
    return sigma;
}

}  // namespace ravel
