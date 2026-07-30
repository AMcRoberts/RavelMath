// exact_pisot_test.cpp
//
// Self-tests for the C-side exact_pisot.c Sturm-based Pisot
// classifier.  Tests:
//   - Tribonacci (β=1.83928675521...) classifies as Pisot.
//   - σ_1 (β=3.62736508...) classifies as Pisot.
//   - σ_2 (β=3.07959562...) classifies as Pisot.
//
// Each classification asserts:
//   - is_pisot == 1
//   - beta_lo and beta_hi straddle the known β
//   - has_complex_pair == 1 (all three Pisot cases have complex
//     secondary spectrum)
//   - |det M| == 1 (the cubic Pisot theorem from the technical note:
//     Pisot polynomials of odd degree have |det|=1)

#include <cstdio>
#include <cstring>
#include <cmath>
#include <array>
#include "exact_pisot.h"

static int total_tests = 0;
static int failed = 0;

#define CHECK(expr, label) do {                              \
    ++total_tests;                                          \
    if (!(expr)) {                                          \
        printf("  [FAIL] %s\n", label);                     \
        ++failed;                                           \
    } else {                                                \
        printf("  [ok]   %s\n", label);                     \
    }                                                       \
} while (0)

static void classify_and_check(const char* name,
        const long long M[3][3],
        double expected_beta_lo,
        double expected_beta_hi) {
    printf("[%s]   M = [[%lld, %lld, %lld], [%lld, %lld, %lld], [%lld, %lld, %lld]]\n",
        name, M[0][0], M[0][1], M[0][2], M[1][0], M[1][1], M[1][2],
        M[2][0], M[2][1], M[2][2]);
    pisot_info_t info;
    int rc = pisot_classify_3x3(M, &info);
    CHECK(rc != 0, "pisot_classify_3x3 returns nonzero");
    char buf[64];
    mpq_class_str(buf, sizeof(buf), info.beta_lo_num, info.beta_lo_den);
    printf("    is_pisot=%d  beta_lo=%s  ", info.is_pisot, buf);
    mpq_class_str(buf, sizeof(buf), info.beta_hi_num, info.beta_hi_den);
    printf("beta_hi=%s\n", buf);
    printf("    n_real_inside=%d  has_complex_pair=%d  "
           "is_complex_modulus_lt_1=%d\n",
        info.n_real_inside, info.has_complex_pair,
        info.is_complex_modulus_lt_1);
    char dbuf[64];
    mpz_get_str(dbuf, 10, info.det_abs);
    printf("    |det M| = %s\n", dbuf);

    /* Strong assertions on the classification result. */
    CHECK(info.is_pisot == 1, "is_pisot == 1");
    CHECK(info.has_complex_pair == 1, "has_complex_pair == 1");
    CHECK(info.is_complex_modulus_lt_1 == 1, "is_complex_modulus_lt_1 == 1");
    CHECK(info.n_real_inside == 0, "n_real_inside == 0 (no real secondary roots)");

    /* β interval brackets the known β. */
    double beta_lo = mpz_get_d(info.beta_lo_num) / mpz_get_d(info.beta_lo_den);
    double beta_hi = mpz_get_d(info.beta_hi_num) / mpz_get_d(info.beta_hi_den);
    char label_lo[64], label_hi[64];
    snprintf(label_lo, sizeof(label_lo), "beta_lo < expected_hi %g", expected_beta_hi);
    snprintf(label_hi, sizeof(label_hi), "beta_hi > expected_lo %g", expected_beta_lo);
    CHECK(beta_lo < expected_beta_hi, label_lo);
    CHECK(beta_hi > expected_beta_lo, label_hi);

    /* |det M| = 1 for cubic Pisot (technical note theorem). */
    CHECK(mpz_cmp_ui(info.det_abs, 1) == 0, "|det M| == 1 (cubic Pisot theorem)");

    pisot_info_clear(&info);
}

int main() {
    /* Tribonacci: σ(0)=(0,1), σ(1)=(0,2), σ(2)=(0).
     * M[i][j] = number of letter i in σ(j).
     * σ(0)=(0,1)→1 zero,1 one,0 twos.  σ(1)=(0,2)→1 zero,0 ones,1 two.
     * σ(2)=(0)→1 zero,0 ones,0 twos.
     * M = [[1,1,1],[1,0,0],[0,1,0]].  β = 1.83928675521... */
    {
        const long long M[3][3] = {{1,1,1}, {1,0,0}, {0,1,0}};
        classify_and_check("tribonacci", M, 1.83, 1.85);
    }

    /* σ_1: σ(0)=(0,0,0,1), σ(1)=(0,0,2), σ(2)=(0).
     * σ(0): 3 zeros, 1 one, 0 twos.  σ(1): 2 zeros, 0 ones, 1 two.
     * σ(2): 1 zero, 0 ones, 0 twos.
     * M = [[3,2,1],[1,0,0],[0,1,0]].  β = 3.62736508... */
    {
        const long long M[3][3] = {{3,2,1}, {1,0,0}, {0,1,0}};
        classify_and_check("sigma_1", M, 3.62, 3.63);
    }

    /* σ_2: σ(0)=(0,0,1), σ(1)=(0,0,0,2), σ(2)=(0).
     * σ(0): 2 zeros, 1 one, 0 twos.  σ(1): 3 zeros, 0 ones, 1 two.
     * σ(2): 1 zero, 0 ones, 0 twos.
     * M = [[2,3,1],[1,0,0],[0,1,0]].  β = 3.07959562... */
    {
        const long long M[3][3] = {{2,3,1}, {1,0,0}, {0,1,0}};
        classify_and_check("sigma_2", M, 3.07, 3.08);
    }

    /* Tetrabonacci: σ(0)=(0,1), σ(1)=(0,2), σ(2)=(0,3), σ(3)=(0).
     * 4-letter Pisot substitution.  Char poly: x⁴ - x³ - x² - x - 1.
     * β ≈ 1.92756197548...  det = -1 (unimodular).  Three secondary
     * roots: -0.7748, ±0.3524+0.8653i (complex pair with |α|² ≈ 0.670).
     * Test verifies it's Pisot. */
    {
        const long long M[4][4] = {
            {1, 1, 1, 1},
            {1, 0, 0, 0},
            {0, 1, 0, 0},
            {0, 0, 1, 0}
        };
        pisot_info_t info;
        int rc = pisot_classify_4x4(M, &info);
        CHECK(rc != 0, "tetrabonacci: pisot_classify_4x4 returns nonzero");
        char buf[64];
        mpq_class_str(buf, sizeof(buf), info.beta_lo_num, info.beta_lo_den);
        printf("[tetrabonacci]   is_pisot=%d  beta_lo=%s  ", info.is_pisot, buf);
        mpq_class_str(buf, sizeof(buf), info.beta_hi_num, info.beta_hi_den);
        printf("beta_hi=%s\n", buf);
        printf("    n_real_inside=%d  has_complex_pair=%d  "
               "is_complex_modulus_lt_1=%d\n",
            info.n_real_inside, info.has_complex_pair,
            info.is_complex_modulus_lt_1);
        char dbuf[64];
        mpz_get_str(dbuf, 10, info.det_abs);
        printf("    |det M| = %s\n", dbuf);
        CHECK(info.is_pisot == 1, "tetrabonacci is_pisot == 1");
        CHECK(info.has_complex_pair == 1, "tetrabonacci has_complex_pair == 1");
        CHECK(info.is_complex_modulus_lt_1 == 1, "tetrabonacci complex |.|² < 1");
        CHECK(info.n_real_inside == 1, "tetrabonacci n_real_inside == 1 (-0.775 is real and in (-1,1))");
        double beta_lo = mpz_get_d(info.beta_lo_num) / mpz_get_d(info.beta_lo_den);
        double beta_hi = mpz_get_d(info.beta_hi_num) / mpz_get_d(info.beta_hi_den);
        CHECK(beta_lo < 1.93 && beta_hi > 1.92, "tetrabonacci β ∈ (1.92, 1.93)");
        CHECK(mpz_cmp_ui(info.det_abs, 1) == 0, "tetrabonacci |det| == 1 (unimodular)");
        pisot_info_clear(&info);
    }

    /* A non-Pisot 4x4 example (repeated root on unit circle) —
     * tests that the classifier correctly rejects non-Pisot
     * quartics.  M = diag(2, 1, 1, 0): β=2>1 but has |α|=1 roots.
     * Char poly = (λ-2)(λ-1)²·λ = λ⁴ - 4λ³ + 5λ² - 2λ.  det=0.
     * Should be classified as not Pisot (because the polynomial has
     * repeated roots and β=1 roots on the unit circle). */
    {
        const long long M[4][4] = {
            {2, 0, 0, 0},
            {0, 1, 0, 0},
            {0, 0, 1, 0},
            {0, 0, 0, 0}
        };
        pisot_info_t info;
        int rc = pisot_classify_4x4(M, &info);
        printf("[non_pisot_block_diag]   rc=%d  is_pisot=%d  |det M| = ", rc, info.is_pisot);
        char dbuf[64]; mpz_get_str(dbuf, 10, info.det_abs);
        printf("%s\n", dbuf);
        /* det=0 means not a substitution matrix; classifier may
         * classify based on the resulting polynomial's roots. */
        CHECK(rc == 0 || info.is_pisot == 0,
              "non-Pisot quartic (β=1 roots) rejected");
        pisot_info_clear(&info);
    }

    /* Non-unimodular Pisot candidate from random search (|det|=2).
     * Found by the historical 4x4 search (now replaced by
     * lua/scripts/pisot_search_4x4.lua) — the first match in a
     * 20k random 4x4 sweep with entries 0..3.  This is the
     * "non-unimodular Pisot" the project has been looking for:
     * degree 4, |det|=2, β ≈ 8.06. */
    {
        const long long M[4][4] = {
            {3, 3, 3, 2},
            {3, 1, 0, 2},
            {1, 2, 3, 1},
            {3, 0, 1, 3}
        };
        pisot_info_t info;
        int rc = pisot_classify_4x4(M, &info);
        printf("[non_unimod_pisot_candidate_1]   rc=%d  is_pisot=%d\n",
               rc, info.is_pisot);
        if (rc != 0) {
            char buf[64];
            mpq_class_str(buf, sizeof(buf), info.beta_lo_num, info.beta_lo_den);
            printf("    beta_lo=%s  ", buf);
            mpq_class_str(buf, sizeof(buf), info.beta_hi_num, info.beta_hi_den);
            printf("beta_hi=%s\n", buf);
            printf("    n_real_inside=%d  has_complex_pair=%d  "
                   "is_complex_modulus_lt_1=%d\n",
                info.n_real_inside, info.has_complex_pair,
                info.is_complex_modulus_lt_1);
            char dbuf[64];
            mpz_get_str(dbuf, 10, info.det_abs);
            printf("    |det M| = %s  (NUMERICALLY: β=8.055, |b2|=0.924)\n", dbuf);
        }
        CHECK(rc != 0, "non_unimod_pisot_candidate_1: pisot_classify_4x4 returns nonzero");
        CHECK(info.is_pisot == 1, "non_unimod_pisot_candidate_1 is_pisot == 1");
        CHECK(mpz_cmp_ui(info.det_abs, 2) == 0,
              "non_unimod_pisot_candidate_1 |det M| == 2 (non-unimodular)");
        pisot_info_clear(&info);
    }

    /* Second non-unimodular Pisot candidate from random search. */
    {
        const long long M[4][4] = {
            {0, 0, 1, 0},
            {0, 2, 3, 3},
            {1, 2, 1, 3},
            {0, 2, 0, 2}
        };
        pisot_info_t info;
        int rc = pisot_classify_4x4(M, &info);
        printf("[non_unimod_pisot_candidate_2]   rc=%d  is_pisot=%d\n",
               rc, info.is_pisot);
        if (rc != 0) {
            char buf[64];
            mpq_class_str(buf, sizeof(buf), info.beta_lo_num, info.beta_lo_den);
            printf("    beta_lo=%s  ", buf);
            mpq_class_str(buf, sizeof(buf), info.beta_hi_num, info.beta_hi_den);
            printf("beta_hi=%s\n", buf);
            char dbuf[64];
            mpz_get_str(dbuf, 10, info.det_abs);
            printf("    |det M| = %s  (NUMERICALLY: β=5.845, |b2|=0.810)\n", dbuf);
        }
        CHECK(rc != 0, "non_unimod_pisot_candidate_2: pisot_classify_4x4 returns nonzero");
        CHECK(info.is_pisot == 1, "non_unimod_pisot_candidate_2 is_pisot == 1");
        CHECK(mpz_cmp_ui(info.det_abs, 2) == 0,
              "non_unimod_pisot_candidate_2 |det M| == 2 (non-unimodular)");
        pisot_info_clear(&info);
    }

    printf("\n=== Note: EXPLODED entries are Pisot (validate_exploded) ===\n");
    printf("The 12 'EXPLODED' survey entries (rnd1_canon, rnd4_canon, ...)\n"
           "all have |det M| = 2.  validate_exploded.cpp runs the\n"
           "Sturm-based exact Pisot classifier on each and confirms\n"
           "they ARE Pisot (is_pisot=1) — the 2/3 of them with a\n"
           "complex secondary pair have |alpha|^2 ~ 2/beta < 1, and\n"
           "rnd13_canon has two real secondary roots in (-1, 1).\n"
           "The earlier 'cubic Pisot |det|=1 theorem' in\n"
           "CLASSIFICATION_contact_boundary_survey.md was incorrect:\n"
           "the proof only shows |d| < beta, not |d| = 1, and\n"
           "the integer constraint does NOT force |d| = 1.\n"
           "The contact-boundary pipeline EXPLODES on these entries\n"
           "due to a real double-precision issue in in_H_sigma, not\n"
           "a Pisot-membership issue.\n");

    printf("\n%d tests run, %d failed.\n", total_tests, failed);
    return failed == 0 ? 0 : 1;
}
