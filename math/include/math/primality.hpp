// math/primality.hpp
//
// Primality testing, prime enumeration, and the Legendre/Jacobi/Kronecker
// symbol. Tier 0: general, no Pisot assumption, depends only on
// math/bigint.hpp.
//
// Built to close a confirmed gap: neither primality testing/prime
// enumeration nor the Legendre/Kronecker symbol existed anywhere in this
// codebase before this file (checked via full-repo grep for every
// standard name and pattern -- not documentation drift, a genuine
// first-time absence). This is the actual blocker for Sawin's
// Golod-Shafarevich criterion (arXiv 2605.20579, Lemma 11): enumerating
// the prime set S_Q needs prime generation, and checking whether a prime
// splits in a quadratic subfield needs the Kronecker symbol -- neither
// piece needs a computed class group, contrary to an earlier (wrong,
// corrected) task dependency.
//
// is_prime: a thin wrapper around mini-gmp's mpz_probab_prime_p, which is
// NOT a naive fixed-rep Miller-Rabin -- it runs a full BPSW test
// (Baillie-PSW: base-2 Miller-Rabin composed with a strong Lucas test)
// plus additional deterministic Miller-Rabin rounds with bases from
// Euler's polynomial j^2+j+41. No composite number is known to pass
// BPSW, and mini-gmp itself is fully deterministic here (no random state
// exists anywhere in mini-gmp), so results are exactly reproducible.
// Matches GMP's own convention: primality is tested on |n|, so
// is_prime(-5) is true (since 5 is prime).

#pragma once

#include <vector>

#include "math/bigint.hpp"
#include "mini-gmp/mini-gmp.h"

namespace mathlib {

inline bool is_prime(const BigInt& n) {
    return mpz_probab_prime_p(n.get(), 25) != 0;
}

inline bool is_prime(long long n) { return is_prime(BigInt(n)); }

// Smallest prime strictly greater than n (n may be negative, zero, or
// composite; matches GMP's mpz_nextprime semantics, though mini-gmp
// doesn't provide that function so this reimplements it directly on top
// of is_prime).
inline BigInt next_prime(const BigInt& n) {
    BigInt cand;
    add_si(cand, n, 1);
    if (cmp_si(cand, 2) <= 0) {
        set_si(cand, 2);
        return cand;
    }
    if (mpz_even_p(cand.get())) add_si(cand, cand, 1);
    while (!is_prime(cand)) add_si(cand, cand, 2);
    return cand;
}

inline long long next_prime(long long n) {
    BigInt r = next_prime(BigInt(n));
    return mpz_get_si(r.get());
}

// All primes p with 2 <= p <= limit, in increasing order. Sieve of
// Eratosthenes: intended for the small, fixed ranges Golod-Shafarevich
// verification actually needs (Sawin's T and S_Q top out at 179), not
// arbitrary-precision enumeration -- next_prime is the tool for that.
inline std::vector<long long> sieve_of_eratosthenes(long long limit) {
    std::vector<long long> primes;
    if (limit < 2) return primes;
    std::vector<bool> composite(static_cast<size_t>(limit) + 1, false);
    for (long long i = 2; i <= limit; ++i) {
        if (composite[static_cast<size_t>(i)]) continue;
        primes.push_back(i);
        if (i <= limit / i) {
            for (long long j = i * i; j <= limit; j += i) {
                composite[static_cast<size_t>(j)] = true;
            }
        }
    }
    return primes;
}

namespace detail {
inline unsigned long mod8(const BigInt& x) {
    mpz_t tmp;
    mpz_init(tmp);
    unsigned long r = mpz_fdiv_r_ui(tmp, x.get(), 8);
    mpz_clear(tmp);
    return r;
}
}  // namespace detail

// Kronecker symbol (a|n) for any integers a, n -- generalizes the
// Legendre symbol (n an odd prime) and Jacobi symbol (n odd positive) to
// arbitrary n via multiplicativity over n's sign and factors of 2.
// Cohen, "A Course in Computational Algebraic Number Theory," Algorithm
// 1.4.10 (a binary-GCD-style reciprocity loop, not factorization of n --
// mini-gmp has no Jacobi/Kronecker symbol at all, so this is written
// from scratch and checked against Euler's criterion independently, see
// tests/primality_test.cpp).
inline int kronecker_symbol(BigInt a, BigInt n) {
    if (is_zero(n)) {
        BigInt aa;
        abs_(aa, a);
        return is_one(aa) ? 1 : 0;
    }

    if (mpz_even_p(a.get()) && mpz_even_p(n.get())) return 0;

    int k = 1;
    BigInt b = n;

    unsigned long v = 0;
    while (mpz_even_p(b.get())) {
        BigInt half;
        mpz_fdiv_q_2exp(half.get(), b.get(), 1);
        b = half;
        ++v;
    }
    if (v % 2 == 1) {
        unsigned long r8 = detail::mod8(a);
        if (r8 == 3 || r8 == 5) k = -k;
    }

    if (sgn(b) < 0) {
        neg(b, b);
        if (sgn(a) < 0) k = -k;
    }

    {
        BigInt r;
        mpz_mod(r.get(), a.get(), b.get());
        a = r;
    }

    while (!is_zero(a)) {
        unsigned long e = 0;
        while (mpz_even_p(a.get())) {
            BigInt half;
            mpz_fdiv_q_2exp(half.get(), a.get(), 1);
            a = half;
            ++e;
        }
        if (e % 2 == 1) {
            unsigned long r8 = detail::mod8(b);
            if (r8 == 3 || r8 == 5) k = -k;
        }
        if (detail::mod8(a) % 4 == 3 && detail::mod8(b) % 4 == 3) k = -k;

        BigInt r;
        mpz_mod(r.get(), b.get(), a.get());
        b = a;
        a = r;
    }

    return cmp_si(b, 1) > 0 ? 0 : k;
}

inline int kronecker_symbol(long long a, long long n) {
    return kronecker_symbol(BigInt(a), BigInt(n));
}

}  // namespace mathlib
