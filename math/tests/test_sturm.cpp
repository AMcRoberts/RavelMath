// Test Tier 3.5: Sturm sequencing and real root isolation.
//
// Reference: SymPy count_roots and Sturm's theorem applied to Pisot
// charpolys.

#include <cstdio>
#include <cstdlib>
#include <string>

#include "math/bigint.hpp"
#include "math/poly_z.hpp"
#include "math/poly_q.hpp"
#include "math/qbeta.hpp"
#include "math/sturm.hpp"

#include "test_common.hpp"

using namespace mathlib;





int main() {
    std::printf("== sturm::chain construction (Tribonacci x³ - x² - x - 1) ==\n");
    {
        PolyZ p({-1, -1, -1, 1});  // Tribonacci charpoly
        auto chain = sturm_chain(p);
        std::printf("    chain size = %zu\n", chain.size());
        for (size_t i = 0; i < chain.size(); ++i) {
            std::printf("      [%zu] %s\n", i, str(chain[i]).c_str());
        }
        EXPECT(chain.size() >= 2, "chain has at least 2 elements");
    }

    std::printf("== sturm::chain sign variations at endpoints ==\n");
    {
        PolyZ p({-1, -1, -1, 1});
        auto chain = sturm_chain(p);
        // V(0): should count sign changes
        int v0 = sturm_sign_variations(chain, B(0));
        int v2 = sturm_sign_variations(chain, B(2));
        std::printf("    V(0) = %d, V(2) = %d, V(0) - V(2) = %d\n", v0, v2, v0 - v2);
        // For Tribonacci, β ≈ 1.839, so 1 real root in (0, 2)
        EXPECT(v0 - v2 == 1, "exactly 1 real root in (0, 2)");
    }

    std::printf("== sturm::root count in (1, X) for Pisot charpolys ==\n");
    {
        struct TC { const char* name; std::vector<long long> c; };
        std::vector<TC> tests = {
            {"Tribonacci", {-1, -1, -1}},
            {"sigma_1", {-3, -2, -1}},
            {"sigma_2", {-2, -1, -1}},
            {"TetrABONACCI", {-1, -1, -1, -1}},
            {"rnd1_canon", {-2, -3, -1}},
        };
        for (const auto& t : tests) {
            QBetaRing R;
            R.charpoly_ = PolyZ();
            R.charpoly_.ensure_size(static_cast<std::size_t>(t.c.size() + 1));
            for (std::size_t k = 0; k < t.c.size(); ++k) {
                set_si(R.charpoly_.coeff(t.c.size() - 1 - k), t.c[k]);
            }
            set_si(R.charpoly_.coeff(t.c.size()), 1);
            auto chain = sturm_chain(R.charpoly());
            BigInt cb = cauchy_bound(R.charpoly());
            long long n = sturm_root_count(chain, B(1), cb);
            std::printf("    %s: roots in (1, %s) = %lld\n",
                t.name, str(cb).c_str(), n);
            EXPECT(n == 1, "exactly 1 real root > 1 (Pisot)");
        }
    }

    std::printf("== sturm::isolate_beta for each Pisot ring ==\n");
    {
        struct TC { const char* name; std::vector<long long> c; double expected_beta; };
        std::vector<TC> tests = {
            {"Tribonacci", {-1, -1, -1}, 1.839286755},
            {"sigma_1", {-3, -2, -1}, 3.627365085},
            {"sigma_2", {-2, -1, -1}, 3.079595624},
            {"TetrABONACCI", {-1, -1, -1, -1}, 1.927561975},
            {"rnd1_canon", {-2, -3, -1}, 3.152757602},
        };
        for (const auto& t : tests) {
            QBetaRing R;
            R.charpoly_ = PolyZ();
            R.charpoly_.ensure_size(static_cast<std::size_t>(t.c.size() + 1));
            for (std::size_t k = 0; k < t.c.size(); ++k) {
                set_si(R.charpoly_.coeff(t.c.size() - 1 - k), t.c[k]);
            }
            set_si(R.charpoly_.coeff(t.c.size()), 1);
            RootInterval ri = isolate_beta(R);
            std::printf("    %s: β ∈ [%s, %s]\n",
                t.name, str(ri.lo).c_str(), str(ri.hi).c_str());
            // Just check that the interval is non-trivial
            EXPECT(cmp(ri.lo, ri.hi) < 0, "interval is non-empty");
        }
    }

    std::printf("== sturm::qbeta_sign: (1+β) > 0 in each ring ==\n");
    {
        struct TC { const char* name; std::vector<long long> c; };
        std::vector<TC> tests = {
            {"Fibonacci", {-1, -1}},
            {"Tribonacci", {-1, -1, -1}},
            {"sigma_1", {-3, -2, -1}},
            {"sigma_2", {-2, -1, -1}},
            {"TetrABONACCI", {-1, -1, -1, -1}},
        };
        for (const auto& t : tests) {
            QBetaRing R;
            R.charpoly_ = PolyZ();
            R.charpoly_.ensure_size(static_cast<std::size_t>(t.c.size() + 1));
            for (std::size_t k = 0; k < t.c.size(); ++k) {
                set_si(R.charpoly_.coeff(t.c.size() - 1 - k), t.c[k]);
            }
            set_si(R.charpoly_.coeff(t.c.size()), 1);
            RootInterval ri = isolate_beta(R);
            QElem one_plus_b = R.add(R.one(), R.beta_k(1));
            int sg = qbeta_sign(one_plus_b, R, ri);
            std::printf("    %s: sign(1+β) = %d  (expected +1)\n", t.name, sg);
            EXPECT(sg > 0, "1+β > 0");
        }
    }

    std::printf("== sturm::qbeta_sign: (β - 1) > 0 in each ring ==\n");
    {
        struct TC { const char* name; std::vector<long long> c; };
        std::vector<TC> tests = {
            {"Fibonacci", {-1, -1}},
            {"Tribonacci", {-1, -1, -1}},
            {"sigma_1", {-3, -2, -1}},
            {"sigma_2", {-2, -1, -1}},
            {"TetrABONACCI", {-1, -1, -1, -1}},
        };
        for (const auto& t : tests) {
            QBetaRing R;
            R.charpoly_ = PolyZ();
            R.charpoly_.ensure_size(static_cast<std::size_t>(t.c.size() + 1));
            for (std::size_t k = 0; k < t.c.size(); ++k) {
                set_si(R.charpoly_.coeff(t.c.size() - 1 - k), t.c[k]);
            }
            set_si(R.charpoly_.coeff(t.c.size()), 1);
            RootInterval ri = isolate_beta(R);
            QElem b_minus_1 = R.sub(R.beta_k(1), R.one());
            int sg = qbeta_sign(b_minus_1, R, ri);
            std::printf("    %s: sign(β - 1) = %d  (expected +1)\n", t.name, sg);
            EXPECT(sg > 0, "β > 1");
        }
    }

    std::printf("\n== summary ==\n");
    std::printf("%d passed, %d failed\n", mathlib_test::n_pass, mathlib_test::n_fail);
    return mathlib_test::n_fail == 0 ? 0 : 1;
}
