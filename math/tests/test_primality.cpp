// Test: primality.hpp (is_prime, next_prime, sieve_of_eratosthenes,
// kronecker_symbol).
//
// Per this project's testing discipline (docs/CPP_DESIGN_PHILOSOPHY.md
// Sec. 3), checked against independent references, not against another
// call to the same algorithm:
//   - is_prime / sieve_of_eratosthenes / next_prime cross-checked
//     against a hardcoded list of the first primes and a from-scratch
//     trial-division sieve written directly in this test file.
//   - kronecker_symbol checked against Euler's criterion
//     (a^((p-1)/2) mod p) for odd primes p, which is a completely
//     different characterization of the Legendre symbol than the
//     reciprocity-loop algorithm primality.hpp actually implements.
//   - kronecker_symbol's multiplicativity ((a|n)*(b|n) == (ab|n)) is
//     checked directly as a structural property.

#include <cstdio>
#include <vector>

#include "math/primality.hpp"

#include "test_common.hpp"

using namespace mathlib;

namespace {

// Independent trial-division primality test, written from scratch (not
// sharing any code with primality.hpp's BPSW-based is_prime), used only
// to cross-check is_prime/sieve_of_eratosthenes below.
bool trial_division_is_prime(long long n) {
    if (n < 2) return false;
    for (long long d = 2; d * d <= n; ++d) {
        if (n % d == 0) return false;
    }
    return true;
}

// a^e mod m via BigInt, independent of kronecker_symbol's own internals
// (used only for Euler's criterion cross-check).
BigInt powmod(long long a, long long e, long long m) {
    BigInt base(a), mod(m), result;
    mpz_mod(base.get(), base.get(), mod.get());
    mpz_t exp;
    mpz_init_set_si(exp, e);
    mpz_powm(result.get(), base.get(), exp, mod.get());
    mpz_clear(exp);
    return result;
}

}  // namespace

int main() {
    std::printf("== primality: is_prime / sieve_of_eratosthenes / next_prime ==\n");
    {
        // First primes, hardcoded from a standard reference table.
        std::vector<long long> known = {
            2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37, 41, 43, 47,
            53, 59, 61, 67, 71, 73, 79, 83, 89, 97
        };
        for (long long p : known) {
            EXPECT(is_prime(p), "is_prime agrees with the known table (prime case)");
        }
        std::vector<long long> known_composite = {
            0, 1, 4, 6, 8, 9, 10, 12, 15, 21, 25, 49, 91, 100
        };
        for (long long c : known_composite) {
            EXPECT(!is_prime(c), "is_prime agrees with the known table (composite case)");
        }
    }
    {
        // is_prime vs. an independent trial-division test, every n in
        // [0, 500] -- a real parity test, not a spot check.
        bool all_agree = true;
        for (long long n = 0; n <= 500 && all_agree; ++n) {
            if (is_prime(n) != trial_division_is_prime(n)) all_agree = false;
        }
        EXPECT(all_agree, "is_prime matches independent trial division for all n in [0,500]");
    }
    {
        // sieve_of_eratosthenes(500) matches is_prime exactly, both
        // directions (every sieve entry is prime, every prime <= 500 is
        // in the sieve).
        auto primes = sieve_of_eratosthenes(500);
        bool all_prime = true;
        for (long long p : primes) if (!is_prime(p)) all_prime = false;
        EXPECT(all_prime, "every sieve_of_eratosthenes(500) entry is prime");

        size_t count_by_is_prime = 0;
        for (long long n = 2; n <= 500; ++n) if (is_prime(n)) ++count_by_is_prime;
        EXPECT(primes.size() == count_by_is_prime,
               "sieve_of_eratosthenes(500) count matches is_prime count over [2,500]");
    }
    {
        // next_prime chains reproduce the sieve exactly.
        auto primes = sieve_of_eratosthenes(200);
        long long p = 1;
        bool chain_matches = true;
        for (size_t i = 0; i < primes.size() && chain_matches; ++i) {
            p = next_prime(p);
            if (p != primes[i]) chain_matches = false;
        }
        EXPECT(chain_matches, "chained next_prime() calls reproduce sieve_of_eratosthenes(200) exactly");
    }
    {
        EXPECT(mpz_get_si(next_prime(BigInt(-100)).get()) == 2,
               "next_prime of a negative number is 2 (smallest prime overall)");
        EXPECT(mpz_get_si(next_prime(BigInt(0)).get()) == 2, "next_prime(0) == 2");
        EXPECT(mpz_get_si(next_prime(BigInt(1)).get()) == 2, "next_prime(1) == 2");
        EXPECT(mpz_get_si(next_prime(BigInt(2)).get()) == 3, "next_prime(2) == 3");
    }

    std::printf("== primality: kronecker_symbol vs. Euler's criterion ==\n");
    {
        // For every odd prime p up to 97 and every a in [1, p-1],
        // kronecker_symbol(a, p) must agree with the Legendre symbol as
        // computed via Euler's criterion -- a genuinely different
        // characterization (a^((p-1)/2) mod p), not another call to the
        // same reciprocity algorithm.
        auto primes = sieve_of_eratosthenes(97);
        bool all_agree = true;
        int checked = 0;
        for (long long p : primes) {
            if (p == 2) continue;
            for (long long a = 1; a < p; ++a) {
                BigInt euler = powmod(a, (p - 1) / 2, p);
                int expected;
                if (mpz_cmp_ui(euler.get(), 1) == 0) expected = 1;
                else expected = -1;  // must be p-1 (i.e. -1 mod p) since gcd(a,p)=1
                int actual = kronecker_symbol(a, p);
                ++checked;
                if (actual != expected) all_agree = false;
            }
        }
        EXPECT(checked > 500, "Euler's-criterion cross-check actually exercised a substantial number of (a,p) pairs");
        EXPECT(all_agree, "kronecker_symbol(a,p) agrees with Euler's criterion for every odd prime p<=97 and every a in [1,p-1]");
    }
    {
        // Known special values, independent of the general algorithm:
        // (a|p) == 0 whenever p | a.
        EXPECT(kronecker_symbol(10, 5) == 0, "kronecker_symbol(10,5) == 0 (5 | 10)");
        EXPECT(kronecker_symbol(0, 7) == 0, "kronecker_symbol(0,7) == 0");
        // (a|1) == 1 for all a, (a|-1) == sign(a) in {+1,-1} for a != 0.
        EXPECT(kronecker_symbol(-37, 1) == 1, "kronecker_symbol(a,1) == 1");
        EXPECT(kronecker_symbol(5, -1) == 1, "kronecker_symbol(5,-1) == 1 (5 >= 0)");
        EXPECT(kronecker_symbol(-5, -1) == -1, "kronecker_symbol(-5,-1) == -1 (-5 < 0)");
        // (2|p) formula: 1 iff p == +-1 mod 8.
        EXPECT(kronecker_symbol(2, 7) == 1, "kronecker_symbol(2,7) == 1 (7 == -1 mod 8)");
        EXPECT(kronecker_symbol(2, 3) == -1, "kronecker_symbol(2,3) == -1 (3 !=  +-1 mod 8)");
        EXPECT(kronecker_symbol(2, 17) == 1, "kronecker_symbol(2,17) == 1 (17 == 1 mod 8)");
    }
    {
        // Multiplicativity: (a|n)*(b|n) == (ab|n), a structural property
        // independent of how the symbol is actually computed.
        long long n = 97;
        bool all_multiplicative = true;
        for (long long a = -10; a <= 10 && all_multiplicative; ++a) {
            for (long long b = -10; b <= 10 && all_multiplicative; ++b) {
                int lhs = kronecker_symbol(a, n) * kronecker_symbol(b, n);
                int rhs = kronecker_symbol(a * b, n);
                if (lhs != rhs) all_multiplicative = false;
            }
        }
        EXPECT(all_multiplicative, "kronecker_symbol is multiplicative in its first argument over a spot-checked range");
    }

    std::printf("\n%d/%d checks passed (this tier)\n",
        ::mathlib_test::n_pass, ::mathlib_test::n_pass + ::mathlib_test::n_fail);
    return 0;
}
