// include/adelic/golod_shafarevich.hpp
//
// Verification of the Golod-Shafarevich criterion (Sawin, arXiv
// 2605.20579, "An explicit lower bound for the unit distance problem",
// Lemma 11 part (4) / equation (9)) for a chosen T (a finite set of odd
// primes) and S_Q (a finite set of primes):
//
//   #T + #S_Q + #{p in S_Q : p splits in Q} + 1 <= (#T - 1)^2 / 4
//
// where Q = Q(sqrt(prod_{q in T} q)). Via the refined Golod-Shafarevich
// theorem (Gaschutz-Vinberg), this inequality implies that the pro-2
// Galois group controlling a certain class-field tower over Q is
// infinite -- the mechanism the paper uses to build totally real fields
// F of arbitrarily large degree and bounded root discriminant, which
// then supply the CM number fields the whole unit-distance construction
// runs on (Proposition 10, Lemma 12).
//
// Deliberately narrow scope: this checks ONLY the combinatorial
// Lemma-11 inequality, using primality testing and the Kronecker symbol
// (math/primality.hpp). Neither piece needs a computed class group --
// an earlier, wrong task dependency assumed it did; rereading the paper
// directly (Lemma 9's class-number bound, used inside Proposition 10,
// not Lemma 11 itself) corrected that. Proposition 10's own exponent
// computation is now verified separately, in
// include/adelic/sawin_exponent.hpp (it needed math/bigfloat_trig.hpp's
// bigfloat_log, which didn't exist until this was written). Lemma 9's
// class-number bound itself is now checked too, for the base case (F=Q,
// K imaginary quadratic), in include/adelic/sawin_lemma9.hpp -- it
// needed include/adelic/quadratic_class_group.hpp's class number
// computation, which also didn't exist until this session.
//
// Splitting test: for p odd and not dividing the fundamental
// discriminant Delta_Q, p splits in Q(sqrt(N)) iff the Kronecker symbol
// (Delta_Q | p) == 1 (Cohen, "A Course in Computational Algebraic
// Number Theory," Theorem 3.5.5); this formula also correctly handles
// p = 2 and p | Delta_Q (ramified, symbol 0) without a special case.
// Since T's own elements force N = prod(T) squarefree and N == 3 mod 4
// (see the constructor's check below), Delta_Q = 4N (Cohen Prop 5.1.3,
// matching the derivation in Sawin's own Lemma 12 proof).

#pragma once

#include <stdexcept>
#include <vector>

#include "math/bigint.hpp"
#include "math/primality.hpp"

namespace adelic {

struct GolodShafarevichCheck {
    long long T_count = 0;
    long long S_Q_count = 0;
    long long split_count = 0;
    std::vector<long long> ramified_primes;
    std::vector<long long> split_primes;
    std::vector<long long> inert_primes;
    mathlib::BigInt N;        // prod_{q in T} q
    mathlib::BigInt Delta_Q;  // fundamental discriminant of Q(sqrt(N)), == 4N here
    bool inequality_holds = false;
};

inline GolodShafarevichCheck check_golod_shafarevich(
    const std::vector<long long>& T, const std::vector<long long>& S_Q) {
    for (long long q : T) {
        if (q <= 2 || !mathlib::is_prime(q)) {
            throw std::invalid_argument(
                "check_golod_shafarevich: T must contain only odd primes");
        }
    }
    for (long long p : S_Q) {
        if (!mathlib::is_prime(p)) {
            throw std::invalid_argument(
                "check_golod_shafarevich: S_Q must contain only primes");
        }
    }

    long long count_3mod4 = 0;
    for (long long q : T) {
        if (q % 4 == 3) ++count_3mod4;
    }
    if (count_3mod4 % 2 == 0) {
        throw std::invalid_argument(
            "check_golod_shafarevich: the number of elements of T congruent "
            "to 3 mod 4 must be odd (Lemma 11's hypothesis, needed so that "
            "prod(T) == 3 mod 4 and hence Delta_Q = 4*prod(T))");
    }

    GolodShafarevichCheck result;
    mathlib::set_si(result.N, 1);
    for (long long q : T) {
        mathlib::BigInt r;
        mathlib::mul_si(r, result.N, q);
        result.N = r;
    }
    {
        mathlib::BigInt four(4);
        mathlib::mul(result.Delta_Q, four, result.N);
    }

    result.T_count = static_cast<long long>(T.size());
    result.S_Q_count = static_cast<long long>(S_Q.size());

    for (long long p : S_Q) {
        int sym = mathlib::kronecker_symbol(result.Delta_Q, mathlib::BigInt(p));
        if (sym == 0) {
            result.ramified_primes.push_back(p);
        } else if (sym == 1) {
            result.split_primes.push_back(p);
        } else {
            result.inert_primes.push_back(p);
        }
    }
    result.split_count = static_cast<long long>(result.split_primes.size());

    long long lhs = result.T_count + result.S_Q_count + result.split_count + 1;
    long long rhs_num = (result.T_count - 1) * (result.T_count - 1);
    // Compare cross-multiplied (4*lhs <= rhs_num) rather than dividing,
    // so the check is exact even when (#T-1)^2 isn't a multiple of 4.
    result.inequality_holds = (4 * lhs <= rhs_num);

    return result;
}

}  // namespace adelic
