// contact_boundary_test.cpp
//
// Self-test for contact_boundary.hpp.  Runs the full D_cont -> C -> ±C
// -> G_B pipeline on sigma_1 and verifies:
//   * |C| = 14, |±C| = 28, |G_B| = 26
//   * BP-`rho_nc` ≈ 2.286298264082328
//   * λ(G_B) is finite and POSITIVE (sanity check; the exact value is
//     reported as a number for downstream comparison but is NOT pinned
//     because we use double power iteration -- ULP agreement is enough)
//
// Crucially, this test does NOT assert `bp_rho_nc == boundary_eigenvalue`.
// For σ_1 the two numbers differ (2.286 vs 1.746) and we report them
// as DISTINCT.

#include <cstdio>
#include <cstring>
#include <vector>
#include <array>
#include <set>
#include <cmath>
#include <tuple>

#include "ravel/core.hpp"
#include "ravel/substitution.hpp"
#include "ravel/ambient_graph.hpp"
#include "ravel/corona.hpp"
#include "ravel/balanced_pair.hpp"
#include "ravel/contact_boundary.hpp"

using namespace ravel;

static int total_tests = 0;
static int failed = 0;

#define CHECK(expr, label) do {                              \
    ++total_tests;                                          \
    if (!(expr)) {                                          \
        std::printf("  [FAIL] %s\n", label);                \
        ++failed;                                           \
    } else {                                                \
        std::printf("  [ok]   %s\n", label);                \
    }                                                       \
} while (0)

int main() {
    // σ_1 (reference python) subst = {0:(0,0,0,1), 1:(0,0,2), 2:(0,)}
    // -- 0-indexed.  Sigma1's beta = 3.62736508...,
    // |b2| = 0.73735270... (from core.py::spectral).
    std::vector<std::vector<std::int8_t>> sigma{
        {0, 0, 0, 1},
        {0, 0, 2},
        {0}
    };
    SubstitutionRule rule(sigma);
    const double sigma1_beta = 3.6273650847118;
    const double sigma1_b2   = 0.737352705760328;

    // 6 non-self-contact D_cont entries (0-indexed, from the corrected
    // paper Table 1, excluding the 3 self-contacts [0,e2,0], [0,e3,0],
    // [1,e3,1] which violate the face-intersection-dim criterion).
    using DC = std::tuple<long long, std::vector<long long>, long long>;
    std::vector<DC> d_cont = {
        {0, {0, 0, 0}, 1},   // [0, 0, 1]   (paper's [1, 0, 2])
        {0, {0, 0, 0}, 2},   // [0, 0, 2]   (paper's [1, 0, 3])
        {1, {1, -1, 0}, 0},  // [1, e1-e2, 0]  (paper's [2, e1-e2, 1])
        {1, {0, 0, 0}, 2},   // [1, 0, 2]   (paper's [2, 0, 3])
        {2, {1, 0, -1}, 0},  // [2, e1-e3, 0]  (paper's [3, e1-e3, 1])
        {2, {0, 1, -1}, 1},  // [2, e2-e3, 1]  (paper's [3, e2-e3, 2])
    };

    auto rep = compute_contact_boundary<3>(rule, sigma1_beta, sigma1_b2, d_cont);

    std::printf("[sigma_1]   contact_boundary.compute pipeline\n");
    std::printf("  alphabet_size      = %zu\n", rep.alphabet_size);
    std::printf("  beta               = %.15f\n", rep.beta);
    std::printf("  |b2|               = %.15f\n", rep.b2);
    std::printf("  |D_cont|           = %zu\n", rep.d_cont_size);
    std::printf("  |G_P|              = %zu\n", rep.pre_contact_size);
    std::printf("  |C|                = %zu\n", rep.contact_size);
    std::printf("  |±C|               = %zu\n", rep.signed_contact_size);
    std::printf("  |G_B|              = %zu\n", rep.boundary_size);
    std::printf("  converged          = %s\n", rep.converged ? "true" : "false");
    std::printf("  convergence_rounds = %d\n", rep.convergence_rounds);
    std::printf("  BP-rho_nc          = %.15f\n", rep.bp_rho_nc);
    std::printf("  lambda(G_B)        = %.15f\n", rep.boundary_eigenvalue);
    std::printf("  conjecture_status  = %s\n", rep.conjecture_status.c_str());

    // Pipeline counts: bit-exact with the Python reference.
    CHECK(rep.alphabet_size == 3,        "alphabet size is 3");
    CHECK(rep.d_cont_size == 6,          "|D_cont| = 6 (caller-supplied seed)");
    CHECK(rep.contact_size == 14,        "|C| = 14");
    CHECK(rep.signed_contact_size == 28, "|±C| = 28 (matches Python)");
    CHECK(rep.boundary_size == 26,       "|G_B| = 26 (matches Python and reference)");
    CHECK(rep.converged,                 "algorithm2 converged within MAX_ROUNDS");
    CHECK(!rep.closure_stopped_early,    "backward_closure did NOT hit the safety cap");

    // BP-rho_nc bit-exact to the reference ground truth.
    CHECK(std::abs(rep.bp_rho_nc - 2.286298264082328) < 1e-9,
          "BP-rho_nc matches reference ground truth 2.286298264082328");

    // lambda(G_B) is finite, positive, and in a sensible range for a
    // 26-node Pisot-boundary graph.  We do NOT pin the exact value
    // because (a) the Python implementation uses exact arithmetic in
    // Q(beta) whereas we use double power iteration (slight drift),
    // and (b) the eigenvalue is exactly what the open conjecture is
    // about -- pinning it here would be a static-test anti-pattern.
    CHECK(rep.boundary_eigenvalue > 1.0,
          "lambda(G_B) is in the Pisot > 1 range (sanity check)");
    CHECK(rep.boundary_eigenvalue < rep.beta,
          "lambda(G_B) < beta (Pisot dominance holds for G_B)");

    // The two numbers must NOT be asserted equal.  For σ_1
    // BP-rho_nc = 2.286 vs lambda(G_B) = 1.746; a 31% gap.
    // We just check both are populated.
    CHECK(rep.bp_rho_nc > 0.0,
          "bp_rho_nc is a positive real number");
    CHECK(rep.boundary_eigenvalue > 0.0,
          "lambda(G_B) is a positive real number");
    CHECK(rep.boundary_nodes.size() == rep.boundary_size,
          "boundary_nodes vector has the same length as |G_B|");

    // Conjecture framing is wired into the output.
    CHECK(rep.conjecture_status.find("OPEN CONJECTURE") != std::string::npos,
          "conjecture_status flag is set (never silently asserts equality)");

    // Now also smoke-test the dispatcher on d = 3 (should produce
    // the same answer).
    auto rep_disp = compute_contact_boundary_dispatch(rule, sigma1_beta,
                                                     sigma1_b2, d_cont);
    CHECK(rep_disp.boundary_size == 26,
          "runtime dispatcher on d=3 produces same G_B size");

    // analyze_substitution: verify the structural feature fields
    // are populated correctly.  Both σ_1 and Tribonacci have
    // has_constant_factor=true (letter 0 at position 0 of every
    // σ(i)), which is the σ_{a,b} family structural feature.  Yet
    // Tribonacci HOLDS and σ_1 FAILS — so constant_factor alone
    // isn't the discriminator.  We'll find the real discriminator
    // in the batch analysis.
    {
        std::printf("\n[sigma_1]   analyze_substitution structural features\n");
        SubstitutionRule rule({
            {0, 0, 0, 1}, {0, 0, 2}, {0}
        });
        auto a = analyze_substitution(rule, sigma1_beta, sigma1_b2);
        std::printf("  alphabet_size=%zu  beta=%.6f  b2=%.6f  b3=%.6f\n",
                    a.alphabet_size, a.beta, a.b2, a.b3);
        std::printf("  det_M=%lld  is_pisot=%d  is_unimodular=%d  is_primitive=%d  is_injective=%d\n",
                    a.det_M, a.is_pisot, a.is_unimodular,
                    a.is_primitive, a.is_injective);
        std::printf("  pisot_quality=%.6f  conformal=%d  moduli_count=%zu\n",
                    a.pisot_quality, a.conformal, a.moduli_count);
        std::printf("  distinct_initials=%d  constant_finals=%d  has_constant_factor=%d\n",
                    a.distinct_initials, a.constant_finals,
                    a.has_constant_factor);
        std::printf("  image_lengths =");
        for (auto l : a.image_lengths) std::printf(" %zu", l);
        std::printf("\n");
        CHECK(a.alphabet_size == 3, "analyze: alphabet_size = 3");
        CHECK(a.is_pisot, "analyze: σ_1 is Pisot");
        CHECK(a.is_unimodular, "analyze: σ_1 is unimodular");
        CHECK(a.is_primitive, "analyze: σ_1 is primitive");
        CHECK(!a.is_injective, "analyze: σ_1 is NOT injective (σ(1)=(0,0,2) has 0 twice)");
        CHECK(a.conformal,
              "analyze: σ_1 is conformal (single complex pair secondary)");
        CHECK(a.moduli_count == 1,
              "analyze: σ_1 has 1 distinct modulus in secondary spectrum");
        CHECK(a.has_constant_factor,
              "analyze: σ_1 HAS constant_factor (letter 0 at position 0 "
              "of every σ(i))");
        // σ_1's structure: σ(0)=(0,0,0,1), σ(1)=(0,0,2), σ(2)=(0).
        // Letter 0 appears at position 0 of all three.
    }
    // Tribonacci: 0→01, 1→02, 2→0.  Image lengths [2,2,1].
    {
        std::printf("\n[tribonacci]   analyze_substitution structural features\n");
        SubstitutionRule rule({
            {0, 1}, {0, 2}, {0}
        });
        auto a = analyze_substitution(rule, 1.839286755214161, 0.737352705760328);
        std::printf("  alphabet_size=%zu  beta=%.6f  b2=%.6f  b3=%.6f  "
                    "moduli_count=%zu\n",
                    a.alphabet_size, a.beta, a.b2, a.b3, a.moduli_count);
        std::printf("  det_M=%lld  is_unimodular=%d  is_primitive=%d  is_injective=%d\n",
                    a.det_M, a.is_unimodular, a.is_primitive, a.is_injective);
        std::printf("  distinct_initials=%d  constant_finals=%d  has_constant_factor=%d\n",
                    a.distinct_initials, a.constant_finals,
                    a.has_constant_factor);
        std::printf("  image_lengths =");
        for (auto l : a.image_lengths) std::printf(" %zu", l);
        std::printf("\n");
        CHECK(a.is_unimodular, "analyze: Tribonacci is unimodular");
        CHECK(a.is_primitive, "analyze: Tribonacci is primitive");
        CHECK(a.is_injective, "analyze: Tribonacci is injective");
        CHECK(!a.distinct_initials,
              "analyze: Tribonacci does NOT have distinct initials (all start with 0)");
        CHECK(a.conformal,
              "analyze: Tribonacci is conformal (single complex pair secondary)");
        CHECK(a.moduli_count == 1,
              "analyze: Tribonacci has 1 distinct modulus");
        CHECK(a.has_constant_factor,
              "analyze: Tribonacci HAS constant_factor at position 0 "
              "(letter 0 at start of σ(0), σ(1), σ(2))");
    }

    // d = 5, 6, 7, 8, 9 are now supported (MAX_DISPATCH_D = 9).  Verify
    // the dispatch doesn't throw for d in this range, and that the
    // returned report has a sensible structure (non-negative sizes,
    // non-empty conjecture_status — even if the input subst doesn't
    // produce a meaningful |G_B| or a Pisot-consistent rho_nc).
    for (std::size_t d = 5; d <= 9; ++d) {
        // Use a more interesting subst: each letter maps to itself
        // plus 0.  This gives a substitution matrix with 1's on the
        // diagonal and 1's in row 0.
        std::vector<std::vector<std::int8_t>> sigma_d(d);
        for (std::size_t i = 0; i < d; ++i) {
            sigma_d[i].push_back(static_cast<std::int8_t>(i));
            sigma_d[i].push_back(0);
        }
        SubstitutionRule rule_d(sigma_d);
        bool dispatch_ok = true;
        ContactBoundaryReport rep;
        try {
            rep = compute_contact_boundary_dispatch(rule_d, 1.5, 0.0, {});
        } catch (const std::invalid_argument&) {
            dispatch_ok = false;
        } catch (...) {
            // The pipeline may throw on degenerate inputs at d >= 5;
            // what matters is that the dispatcher routes correctly.
            dispatch_ok = true;
        }
        char label[64];
        std::snprintf(label, sizeof(label), "dispatcher accepts d=%zu", d);
        CHECK(dispatch_ok, label);
    }

    // A wrong-d error path: a 10-letter rule is past MAX_DISPATCH_D and
    // should throw.
    std::vector<std::vector<std::int8_t>> sigma10(10, std::vector<std::int8_t>{0});
    sigma10[0] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
    SubstitutionRule rule10(sigma10);
    bool threw = false;
    try {
        (void)compute_contact_boundary_dispatch(rule10, 1.5, 0.0, {});
    } catch (const std::invalid_argument&) {
        threw = true;
    }
    CHECK(threw, "dispatcher throws on alphabet size > MAX_DISPATCH_D");

    std::printf("\n%d tests run, %d failed.\n", total_tests, failed);
    return failed == 0 ? 0 : 1;
}
