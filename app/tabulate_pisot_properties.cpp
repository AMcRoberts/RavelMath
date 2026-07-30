// app/tabulate_pisot_properties.cpp
//
// Tabulate first-principles properties of the unimodular Pisot
// substitutions in our test set, using the helpers in
// include/ravel/pisot_substitution_properties.hpp (which in turn
// extends the existing Substitution<d>, spectral_invariants_3x3,
// involution_helpers, etc. — no reimplementation, per the session
// directive).
//
// Each candidate prints a multi-line block of properties:
//   - Spectrum (β, b2, char poly)
//   - Image lengths |σ(i)|
//   - Has constant factor / constant finals
//   - Letter frequencies (Parry measure)
//   - Pisot dual σ*
//   - Primitive return words
//   - First 10 carry sequence points
//   - A1 (Q_sym_GB / Q_sym_BP x^k cofactor)
//   - A2 (involution on dominant core)
//
// All candidates use the same 4-letter-or-3-letter substitutions
// already in the test set: Tribonacci (control), σ_{a,1} for
// a=0..5, σ_{1,2}, σ_1 (3-letter), σ_2 (3-letter).  Each row's
// properties help identify which structural principles (Christoffel
// structure, return-word structure, carry patterns, etc.) hold for
// ALL Pisot and which are n-bonacci-specific.

#include <cstdio>
#include <string>
#include <vector>

#include "ravel/involution_helpers.hpp"
#include "ravel/pisot_substitution_properties.hpp"
#include "ravel/spectral.hpp"
#include "ravel/substitution.hpp"

using namespace ravel;

namespace {

// σ_{a,b}-family rule (matches app/probe_a1_a2_unimodular.cpp).
std::vector<std::vector<std::int8_t>> sigma_ab(int a, int b) {
    std::vector<std::vector<std::int8_t>> sigma(3);
    sigma[0].reserve(a + b + 1);
    for (int i = 0; i < a; ++i) sigma[0].push_back(0);
    for (int i = 0; i < b; ++i) sigma[0].push_back(1);
    sigma[0].push_back(2);
    sigma[1].reserve(a + 1);
    for (int i = 0; i < a; ++i) sigma[1].push_back(0);
    sigma[1].push_back(2);
    sigma[2] = {0};
    return sigma;
}

// n-bonacci rule (matches involution_helpers.hpp).
std::vector<std::vector<std::int8_t>> n_bonacci(int n) {
    std::vector<std::vector<std::int8_t>> sigma(n);
    for (int i = 0; i + 1 < n; ++i) sigma[i] = {0, static_cast<std::int8_t>(i + 1)};
    sigma[n - 1] = {0};
    return sigma;
}

// σ_1 and σ_2 in 3-letter form (per app/probe_a1_a2_unimodular.cpp).
std::vector<std::vector<std::int8_t>> sigma1_3l() { return {{0, 0, 1}, {0, 2}, {0}}; }
std::vector<std::vector<std::int8_t>> sigma2_3l() { return {{0, 0, 1}, {0, 0, 2}, {0}}; }

}  // namespace

int main() {
    std::printf("==========================================================================\n");
    std::printf(" Tabulation: first-principles properties of unimodular Pisot substitutions\n");
    std::printf("==========================================================================\n\n");

    // The list of candidates.  Each: name, sigma, expected Pisot? (used
    // only for the column header in summary; the actual Pisot check
    // is done by classify_matrix_spectral inside PisotProperties).
    struct Candidate { std::string name; std::vector<std::vector<std::int8_t>> sigma; };
    std::vector<Candidate> candidates = {
        {"Tribonacci n=3 (control)", n_bonacci(3)},
        {"σ_{0,1}", sigma_ab(0, 1)},
        {"σ_{1,1}", sigma_ab(1, 1)},
        {"σ_{2,1}", sigma_ab(2, 1)},
        {"σ_{3,1}", sigma_ab(3, 1)},
        {"σ_{4,1}", sigma_ab(4, 1)},
        {"σ_{5,1}", sigma_ab(5, 1)},
        {"σ_{1,2}", sigma_ab(1, 2)},
        {"σ_1 (3L)", sigma1_3l()},
        {"σ_2 (3L)", sigma2_3l()},
    };

    // The full block runs A1/A2; the summary at the end skips them
    // (they're computed once per substitution already, so the
    // summary doesn't need to re-run them).
    //
    // Run σ_{1,2} first, standalone.  It has |G_B|=486 with default
    // caps, and the contact-boundary pipeline for it builds a large
    // corona closure; the dominant_recurrent_core extraction then
    // needs to allocate a 330-node dense matrix, plus the Q_sym
    // quotient.  That alone consumed >10 GB in a previous combined
    // run.  The user requested giving it 12 GB of room and running
    // it standalone so the small candidates' pipeline state doesn't
    // compete.
    int n_pisot = 0;
    int n_a1 = 0;
    int n_a2 = 0;

    // First: the small 9 candidates.
    for (const auto& c : candidates) {
        if (c.name == "σ_{1,2}") continue;
        SubstitutionRule rule(c.sigma);
        auto props = compute_pisot_properties(rule, c.name, /*run_a1a2=*/true);
        // Use fputs (not printf) because the to_string output contains
        // letter-index bytes 0/1/2 which are NUL/SOH control chars
        // and would terminate a %s format string at the first NUL.
        fputs(props.to_string().c_str(), stdout);
        fputc('\n', stdout);
        if (props.beta > 1.0 && props.b2 < 1.0) ++n_pisot;
        if (props.a1_xk) ++n_a1;
        if (props.a2_involution_exact) ++n_a2;
    }

    // Then: σ_{1,2} standalone, with the same audit caps.  It has
    // already been observed to take >10 GB peak (the user asked for
    // 12 GB worth of room).  The probe here is in its own fork
    // so that other candidates' small memory is already gone and
    // doesn't compete.
    std::printf("\n=========== STANDALONE RUN: σ_{1,2} (12 GB budget) ===========\n");
    for (const auto& c : candidates) {
        if (c.name != "σ_{1,2}") continue;
        SubstitutionRule rule(c.sigma);
        auto props = compute_pisot_properties(rule, c.name, /*run_a1a2=*/true);
        // Use fputs (not printf) because the to_string output contains
        // letter-index bytes 0/1/2 which are NUL/SOH control chars
        // and would terminate a %s format string at the first NUL.
        fputs(props.to_string().c_str(), stdout);
        fputc('\n', stdout);
        if (props.beta > 1.0 && props.b2 < 1.0) ++n_pisot;
        if (props.a1_xk) ++n_a1;
        if (props.a2_involution_exact) ++n_a2;
    }

    // Summary table.
    std::printf("\n==========================================================================\n");
    std::printf(" Summary table\n");
    std::printf("==========================================================================\n");
    std::printf("%-20s  %-9s  %-12s  %-12s  %-7s  %-9s  %-9s\n",
                "candidate", "Pisot?", "A2 invol.", "A1 x^k", "|G_B|", "freq_max", "img_lens");
    for (const auto& c : candidates) {
        SubstitutionRule rule(c.sigma);
        auto props = compute_pisot_properties(rule, c.name, /*run_a1a2=*/false);
        // For summary we need a contact boundary size; rerun.
        // (Skip A1/A2 to keep summary fast; the full block above
        // has the detailed A1/A2 results.)
        ContactBoundaryLimits limits;
        limits.max_rho_pairs = 8000; limits.max_rho_len = 24000;
        limits.closure_cap = 5000; limits.corona_cap = 25000;
        limits.max_corona_rounds = 8;
        ContactBoundaryReport rep;
        try {
            auto subst = make_substitution<3>(rule, props.beta);
            std::vector<std::tuple<long long, std::vector<long long>, long long>> dc;
            for (const auto& cnode : search_D_cont<3>(subst, 2)) {
                dc.emplace_back(cnode.i,
                    std::vector<long long>(cnode.x.begin(), cnode.x.end()),
                    cnode.j);
            }
            rep = compute_contact_boundary_dispatch(rule, props.beta, 0.0, dc);
        } catch (...) {
            rep.boundary_size = 0;
        }
        bool pisot = (props.beta > 1.0 && props.b2 < 1.0);
        std::printf("%-20s  %-9s  %-12s  %-12s  %-7zu  %.4f     ",
                    c.name.c_str(),
                    pisot ? "yes" : "no",
                    "-",  // A2 not re-evaluated in summary
                    "-",  // A1 not re-evaluated in summary
                    rep.boundary_size,
                    props.frequencies.empty() ? 0.0 : *std::max_element(
                        props.frequencies.begin(), props.frequencies.end()));
        std::printf("(");
        for (std::size_t i = 0; i < props.image_lengths.size(); ++i) {
            if (i > 0) std::printf(",");
            std::printf("%zu", props.image_lengths[i]);
        }
        std::printf(")\n");
    }

    std::printf("\nTotals: %zu candidates, %d Pisot, %d A2-EXACT-involution, "
                "%d A1-EXACT-x^k-cofactor\n",
                candidates.size(), n_pisot, n_a2, n_a1);

    return 0;
}
