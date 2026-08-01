// sawin_exponent_test.cpp
//
// Reproduces Sawin's Proposition 10 exponent computation (arXiv
// 2605.20579, proof of Theorem 1, formula (11), p. 12) for the paper's
// own published T, S_Q, k(p), e(p), R -- an independent numeric
// verification that delta = 0.014114..., using this session's own
// bigfloat_log/bigfloat_exp (math/bigfloat_trig.hpp), not Sage or any
// other external computer algebra system.
//
// The paper's own words: "the numerator in (11) is 3.8822... and the
// denominator is 275.055... so we can take delta = .014114..." -- both
// intermediate values are checked here too, not just the final delta,
// so a coincidentally-close-but-wrong delta from a compensating pair of
// errors would still be caught.

#include <cstdio>
#include <vector>

#include "adelic/sawin_exponent.hpp"

using namespace std;
using namespace mathlib;

namespace {
int n_pass = 0, n_fail = 0;

#define CHECK(cond, msg) do { \
    if (cond) { ++n_pass; fprintf(stderr, "  [ok]   %s\n", (msg)); } \
    else { ++n_fail; fprintf(stderr, "  [FAIL] %s\n", (msg)); } \
} while (0)

bool close_to(const BigFloat& x, double expected, double tol) {
    double got = bigfloat_to_double(x);
    return std::fabs(got - expected) < tol;
}

}  // namespace

int main() {
    // T = {3,5,...,43}, 13 primes -- same set golod_shafarevich_test.cpp
    // uses, product recomputed independently here rather than shared,
    // so a bug in one file's BigInt product can't silently cancel
    // against the other's.
    std::vector<long long> T = {3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37, 41, 43};
    BigInt prod_T(1);
    for (long long q : T) {
        BigInt r;
        mul_si(r, prod_T, q);
        prod_T = r;
    }

    // e(p) = 2 if p in T or p == 2, else 1 (Sawin's Lemma 12).
    auto e_of = [&](long long p) -> long long {
        if (p == 2) return 2;
        for (long long q : T) if (q == p) return 2;
        return 1;
    };

    // k(p), taken verbatim from the paper's own worked example (p. 12).
    std::vector<adelic::SawinExponentTerm> S_Q_terms;
    auto add = [&](long long p, long long k) {
        S_Q_terms.push_back({p, k, e_of(p)});
    };
    add(2, 50); add(3, 31); add(5, 21); add(7, 17); add(11, 14);
    add(13, 13); add(17, 12); add(19, 11); add(23, 10); add(29, 10);
    add(47, 8);
    add(71, 7); add(79, 7); add(97, 7); add(101, 7); add(107, 7); add(109, 7);
    add(139, 6); add(151, 6); add(163, 6); add(167, 6); add(179, 6);

    fprintf(stderr, "=== sanity: e(p) table matches the paper's stated partition ===\n");
    CHECK(S_Q_terms.size() == 22, "22 primes in S_Q, matching the paper");
    int e2_count = 0;
    for (const auto& t : S_Q_terms) if (t.e_p == 2) ++e2_count;
    CHECK(e2_count == 10, "exactly 10 primes in S_Q have e(p) == 2 (the 9 T-members plus p=2)");

    fprintf(stderr, "\n=== computing delta via Proposition 10's own formula (11) ===\n");
    auto result = adelic::compute_sawin_delta(prod_T, /*R=*/72, S_Q_terms, /*prec=*/200);

    CHECK(close_to(result.numerator, 3.8822, 1e-3),
          "numerator matches the paper's stated value 3.8822...");
    CHECK(close_to(result.denominator, 275.055, 1e-2),
          "denominator matches the paper's stated value 275.055...");
    CHECK(close_to(result.delta, 0.014114, 1e-5),
          "delta matches the paper's stated value 0.014114..., reproduced independently");

    // Theorem 1's actual claimed exponent is 1 + delta = 1.014114.
    BigFloat one_plus_delta = bigfloat_add(bigfloat_from_ll(1), result.delta, 200);
    CHECK(close_to(one_plus_delta, 1.014114, 1e-5),
          "1 + delta matches Theorem 1's published exponent 1.014114 exactly (to the paper's own precision)");

    fprintf(stderr, "\n=== error handling ===\n");
    {
        bool threw = false;
        try { adelic::compute_sawin_delta(prod_T, /*R=*/1, S_Q_terms); }
        catch (const std::invalid_argument&) { threw = true; }
        CHECK(threw, "R <= 1 throws");
    }
    {
        bool threw = false;
        std::vector<adelic::SawinExponentTerm> bad = {{2, 0, 2}};
        try { adelic::compute_sawin_delta(prod_T, 72, bad); }
        catch (const std::invalid_argument&) { threw = true; }
        CHECK(threw, "k(p) == 0 throws");
    }

    fprintf(stderr, "\n%d/%d checks passed\n", n_pass, n_pass + n_fail);
    return n_fail == 0 ? 0 : 1;
}
