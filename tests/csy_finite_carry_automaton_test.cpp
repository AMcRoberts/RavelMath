// tests/csy_finite_carry_automaton_test.cpp
//
// Q3 of docs/RECOVERY_AUDIT_2026-07-29.md.
//
// Verify the actual finite carry automaton built from CSY
// Theorem 3 — distinct from the bounded-prefix-tree
// `CSYAutomaton` in include/adelic/csy_carry_automaton.hpp —
// agrees with direct Pisot evaluation on the canonical
// Tribonacci Pisot numeration and on the Class-II sigma_{a,1}
// family for a in {3, 4, 5}.
//
// The automaton's state count is the a-posteriori upper bound on
// the bounded-correction length N(U, ‖g‖∞) (Lemma 43).

#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <random>
#include <set>
#include <vector>

#include "adelic/csy_carry_automaton.hpp"
#include "adelic/csy_finite_carry_automaton.hpp"

using namespace adelic;

namespace {

PisotPoly tribonacci_poly() { return PisotPoly::tribonacci(); }

PisotPoly sigma_a1_poly(long long a) {
    // sigma_{a,1}: x^3 = a x^2 + (a+1) x + 1
    return PisotPoly::fromCoefficients({a + 1, a, 1});
    // c[d-1] = a, c[d-2] = a+1, c[d-3] = 1, but stored in low form?
    // The fromCoefficients call expects c in low form: x^d = c_0 + c_1 x + ... + c_{d-1} x^{d-1}
    // For Tribonacci x^3 = x^2 + x + 1: c = {1, 1, 1} (low to high)
    // For sigma_{a,1} x^3 = a x^2 + (a+1) x + 1: c = {1, a+1, a} (low to high)
}

// Generate a random finite word of length n from B.
std::vector<long long> random_word(std::size_t n,
                                   const std::vector<long long>& B,
                                   std::mt19937& rng) {
    std::uniform_int_distribution<std::size_t> pick(0, B.size() - 1);
    std::vector<long long> w(n);
    for (std::size_t i = 0; i < n; ++i) w[i] = B[pick(rng)];
    return w;
}

bool run_case(const char* label, const PisotPoly& P,
              const std::vector<long long>& B,
              std::size_t max_prefix_length = 8,
              std::size_t bound_bits = 14) {
    std::printf("\n=== %s ===\n", label);
    CSYZeroAutomaton aut;
    aut.build(P, B, max_prefix_length, bound_bits);
    std::printf("  raw_nodes=%zu  states=%zu  transitions=%zu  "
                "max_visited_position=%zu  truncated=%d\n",
                aut.raw_node_count(),
                aut.state_count(),
                aut.transition_count(),
                aut.max_visited_position,
                aut.truncated);

    PisotContext ctx(P);
    const auto direct_zero = [&](const std::vector<long long>& w) {
        return ctx.eval(w).is_zero();
    };
    const auto aut_zero = [&](const std::vector<long long>& w) {
        return aut.accepts(w);
    };

    // Brute-force agreement on all words of length 0..4.
    std::size_t n_words = 0, n_disagree = 0;
    for (std::size_t len = 0; len <= 4; ++len) {
        std::size_t cur = 1;
        for (std::size_t k = 0; k < len; ++k) cur *= B.size();
        std::vector<long long> w(len, 0);
        for (std::size_t idx = 0; idx < cur; ++idx) {
            std::size_t t = idx;
            for (std::size_t k = 0; k < len; ++k) {
                w[k] = B[t % B.size()];
                t /= B.size();
            }
            const bool a = aut_zero(w);
            const bool d = direct_zero(w);
            ++n_words;
            if (a != d) {
                ++n_disagree;
                std::printf("  [DISAGREE] len=%zu idx=%zu: "
                            "aut=%d direct=%d\n", len, idx, a, d);
            }
        }
    }
    std::printf("  words checked: %zu  disagreements: %zu\n",
                n_words, n_disagree);

    // Random longer-word agreement (within max_prefix_length).
    std::mt19937 rng(20260730);
    std::size_t n_random = 0, n_random_disagree = 0;
    for (std::size_t i = 0; i < 200; ++i) {
        const std::size_t len = 4 + (i % (max_prefix_length - 4));
        const auto w = random_word(len, B, rng);
        const bool a = aut_zero(w);
        const bool d = direct_zero(w);
        ++n_random;
        if (a != d) {
            ++n_random_disagree;
            std::printf("  [RANDOM DISAGREE] len=%zu: "
                        "aut=%d direct=%d\n", len, a, d);
        }
    }
    std::printf("  random words checked: %zu  disagreements: %zu\n",
                n_random, n_random_disagree);

    const CSYBoundedCorrection bc =
        csy_bounded_correction(P, B, max_prefix_length, bound_bits);
    std::printf("  bounded-correction N upper bound: %zu\n",
                bc.N_upper_bound());

    return n_disagree == 0 && n_random_disagree == 0;
}

}  // namespace

int main() {
    std::printf("CSY Theorem 3 finite carry automaton test\n");
    std::printf("(Q3 of docs/RECOVERY_AUDIT_2026-07-29.md)\n");

    const std::vector<long long> tribonacci_digits = {-1, 0, 1};
    const std::vector<long long> tribonacci_wide = {-2, -1, 0, 1, 2};

    bool ok = true;
    ok = run_case("Tribonacci Pisot alphabet {-1, 0, 1}",
                  tribonacci_poly(), tribonacci_digits) && ok;
    ok = run_case("Tribonacci Pisot alphabet {-2, -1, 0, 1, 2}",
                  tribonacci_poly(), tribonacci_wide) && ok;
    ok = run_case("Class-II sigma_{3,1} alphabet {-1, 0, 1}",
                  sigma_a1_poly(3), tribonacci_digits) && ok;
    ok = run_case("Class-II sigma_{4,1} alphabet {-1, 0, 1}",
                  sigma_a1_poly(4), tribonacci_digits) && ok;
    ok = run_case("Class-II sigma_{5,1} alphabet {-1, 0, 1}",
                  sigma_a1_poly(5), tribonacci_digits) && ok;

    std::printf("\ncsy_finite_carry_automaton_test: %s\n",
                ok ? "PASS" : "FAIL");
    return ok ? 0 : 1;
}
