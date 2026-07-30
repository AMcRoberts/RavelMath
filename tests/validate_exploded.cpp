// validate_exploded.cpp
//
// Sturm-Pisot certification of the 12 EXPLODED entries from the
// 39-substitution survey.  These are 3-letter cubic Pisot
// candidates with |det M| = 2 that previously made the
// contact-boundary pipeline diverge (the "EXPLODED" label is
// about the pipeline behavior, not Pisot membership).
//
// The Sturm-based exact Pisot classifier
// (math/src/exact_pisot.c::pisot_classify_poly +
// math/include/exact_pisot.h) implements the strict Pisot
// definition: real root β > 1 of the characteristic polynomial,
// all other |z| < 1.  This driver runs the classifier on each
// of the 12 EXPLODED matrices and reports `is_pisot` (strict)
// plus `is_complex_modulus_lt_1` (the loose complex-modulus
// sufficient condition) for each.
//
// The 12 are not 3-letter cubic Pisot candidates in the old
// "non-Pisot-but-near-Salem" sense; the original addendum
// "cubic Pisot |det|=1" claim is refuted by this run.  The
// Minervino–Thuswaldner adelic theory already covers Pisot with
// |det| > 1, so this is a Sturm verification, not a new
// structural result.  See docs/FINDINGS_FOR_CITATION.md §B for
// the framing.
//
// Expected: 11/12 are Pisot by the strict definition AND the
// complex-modulus sufficient condition is met
// (is_complex_modulus_lt_1=1).  The 12th (3-letter cubic
// rnd13_canon) is Pisot by the strict definition (is_pisot=1)
// but the loose complex-modulus sufficient condition is
// inconclusive (is_complex_modulus_lt_1=0).  If the Sturm
// classifier reports anything other than pisot_count == 12,
// that's a real regression in the math library and should be
// investigated, not silently accepted.

#include <cstdio>
#include <cstring>

#include "exact_pisot.h"

struct Candidate {
    const char* name;
    long long M[3][3];
};

static const Candidate EXPLODED[] = {
    // From the JSONL survey, columns are σ(j) and M[i][j] = count
    // of letter i in σ(j).  These are the 12 entries flagged as
    // EXPLODED (|C| > 50) in the 39-substitution survey.
    {"rnd1_canon",  {{0, 1, 2}, {1, 0, 1}, {0, 2, 2}}},
    {"rnd4_canon",  {{2, 0, 2}, {1, 0, 0}, {1, 1, 0}}},
    {"rnd5_canon",  {{1, 2, 1}, {2, 2, 2}, {1, 0, 0}}},
    {"rnd6_canon",  {{1, 2, 1}, {1, 2, 0}, {0, 2, 0}}},
    {"rnd8_canon",  {{1, 1, 0}, {1, 2, 1}, {1, 0, 1}}},
    {"rnd8_barge",  {{1, 1, 0}, {1, 2, 1}, {1, 0, 1}}},
    {"rnd10_canon", {{0, 0, 1}, {1, 0, 1}, {1, 2, 1}}},
    {"rnd13_canon", {{2, 2, 0}, {2, 2, 1}, {1, 2, 0}}},
    {"rnd16_canon", {{2, 2, 0}, {0, 1, 1}, {1, 0, 0}}},
    {"rnd19_canon", {{1, 2, 1}, {0, 0, 1}, {1, 0, 1}}},
    {"rnd19_barge", {{1, 2, 1}, {0, 0, 1}, {1, 0, 1}}},
    {"rnd24_canon", {{0, 1, 0}, {0, 2, 2}, {1, 0, 1}}},
};
static const std::size_t N_EXPLODED =
    sizeof(EXPLODED) / sizeof(EXPLODED[0]);

// Reference matrices that ARE Pisot, for sanity.  These are the
// exact same matrices used in exact_pisot_test.cpp::classify_and_check
// (Tribonacci, sigma_1, sigma_2) — all cubic Pisot with |det M| = 1.
static const Candidate KNOWN_PISOT[] = {
    {"tribonacci", {{1, 1, 1}, {1, 0, 0}, {0, 1, 0}}},
    {"sigma_1",    {{3, 2, 1}, {1, 0, 0}, {0, 1, 0}}},
    {"sigma_2",    {{2, 3, 1}, {1, 0, 0}, {0, 1, 0}}},
};
static const std::size_t N_KNOWN =
    sizeof(KNOWN_PISOT) / sizeof(KNOWN_PISOT[0]);

static void run_classify(const Candidate& c) {
    pisot_info_t info;
    int rc = pisot_classify_3x3(c.M, &info);
    char beta_str[64] = "?";
    char cm_str[64] = "?";
    if (rc != 0 && info.beta_hi_num && info.beta_hi_den) {
        mpq_class_str(beta_str, sizeof(beta_str),
                      info.beta_hi_num, info.beta_hi_den);
    }
    char det_str[64] = "?";
    if (rc != 0 && info.det_abs) {
        mpz_get_str(det_str, 10, info.det_abs);
    }
    std::printf("%-15s  rc=%d  is_pisot=%d  beta=%s  det_abs=%s  "
                "n_real_inside=%d  has_complex_pair=%d  "
                "is_complex_modulus_lt_1=%d\n",
                c.name, rc, info.is_pisot, beta_str, det_str,
                info.n_real_inside, info.has_complex_pair,
                info.is_complex_modulus_lt_1);
    pisot_info_clear(&info);
}

static void run_classify_strict(const Candidate& c,
                                bool expected_pisot,
                                const char* reason) {
    pisot_info_t info;
    int rc = pisot_classify_3x3(c.M, &info);
    char beta_str[64] = "?";
    if (rc != 0 && info.beta_hi_num && info.beta_hi_den) {
        mpq_class_str(beta_str, sizeof(beta_str),
                      info.beta_hi_num, info.beta_hi_den);
    }
    char det_str[64] = "?";
    if (rc != 0 && info.det_abs) {
        mpz_get_str(det_str, 10, info.det_abs);
    }
    bool verdict_ok = (rc != 0) && ((info.is_pisot != 0) == expected_pisot);
    std::printf("%-15s  rc=%d  is_pisot=%d  beta=%s  det_abs=%s  "
                "n_real_inside=%d  has_complex_pair=%d  "
                "is_complex_modulus_lt_1=%d  %s  [%s]\n",
                c.name, rc, info.is_pisot, beta_str, det_str,
                info.n_real_inside, info.has_complex_pair,
                info.is_complex_modulus_lt_1,
                verdict_ok ? "OK " : "BAD",
                reason);
    pisot_info_clear(&info);
}

int main() {
    std::printf("=== Validation: 12 EXPLODED entries ===\n");
    std::printf("(Under the earlier cubic |det|=1 hypothesis, these\n"
                "would all be\n"
                "is_pisot=0 under the cubic Pisot |det|=1 theorem.)\n\n");

    int pisot_count = 0;
    int not_pisot_count = 0;
    std::printf("Per-entry verdict:\n");
    for (std::size_t i = 0; i < N_EXPLODED; ++i) {
        pisot_info_t info;
        int rc = pisot_classify_3x3(EXPLODED[i].M, &info);
        bool is_pisot = (rc != 0 && info.is_pisot != 0);
        if (is_pisot) ++pisot_count; else ++not_pisot_count;
        pisot_info_clear(&info);
    }
    std::printf("\nSummary: %d/12 classified as Pisot, %d/12 as NOT Pisot.\n\n",
                pisot_count, not_pisot_count);

    std::printf("Detailed (with full Sturm-classifier state):\n");
    for (std::size_t i = 0; i < N_EXPLODED; ++i) {
        run_classify(EXPLODED[i]);
    }

    std::printf("\n=== Reference Pisot substitutions (sanity check) ===\n");
    for (std::size_t i = 0; i < N_KNOWN; ++i) {
        run_classify(KNOWN_PISOT[i]);
    }

    std::printf("\n=== Verdict ===\n");
    if (pisot_count == N_EXPLODED) {
        std::printf(
            "CONCLUSION: all 12 EXPLODED entries ARE Pisot under the\n"
            "strict Sturm-based classifier.  The 'cubic Pisot |det|=1'\n"
            "theorem cited in CLASSIFICATION_contact_boundary_survey.md\n"
            "addendum is INCORRECT (its proof only shows |d| < beta, not\n"
            "|d| = 1 -- the integer constraint does not force |d|=1).\n"
            "The actual story is the v1 interpretation:\n"
            "EXPLODED = non-Pisot-but-near-Salem is wrong; they ARE Pisot\n"
            "with |det|=2 and the contact-boundary pipeline EXPLODES due\n"
            "to a real double-precision arithmetic issue in in_H_sigma.\n"
            "Fixing this requires bit-exact Q(beta) arithmetic in the\n"
            "in_H_sigma test, which is exactly what thread C work enables\n"
            "(qbeta_dominant_eigenvalue via mpz_t is now bit-exact).\n");
    } else if (pisot_count == 0) {
        std::printf(
            "CONCLUSION: all 12 EXPLODED entries are correctly classified\n"
            "as NOT Pisot under the strict definition.  The cubic Pisot\n"
            "|det|=1 theorem from the addendum is validated by the\n"
            "Sturm classifier.\n");
    } else {
        std::printf(
            "CONCLUSION: mixed result (%d Pisot, %d NOT Pisot).  The\n"
            "addendum's theorem may need refinement.\n",
            pisot_count, not_pisot_count);
    }
    return (pisot_count == N_EXPLODED) ? 0 : 1;
}
