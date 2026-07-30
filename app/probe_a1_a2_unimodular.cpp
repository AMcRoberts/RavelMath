// app/probe_a1_a2_unimodular.cpp
//
// Item A1 + Item A2 from docs/DIRECTION_AND_OPEN_THREADS.md:
//
//   (A1) nilpotent-cofactor probe on σ_{a,b}-family / σ₁-style / random
//        unimodular Pisot substitutions.  The n-bonacci proof in
//        FINDINGS_FOR_CITATION.md Finding 4 has x^k cofactor exact (k = 3,
//        13, 54 for n = 3, 4, 5).  Does this structure persist beyond
//        n-bonacci?  If yes, the conjecture is on its way to a general
//        proof.  If no, the nilpotent-cofactor mechanism is specific
//        to the n-bonacci family.
//
//   (A2) involution probe on the same data.  Does the Z/2 automorphism
//        [i, x, j] <-> [j, -x, i] on the dominant recurrent core of
//        G_B hold for non-n-bonacci unimodular Pisot substitutions?
//        If yes, the same charpoly-divisibility argument structure
//        applies.  If no, the n-bonacci match is genuinely
//        family-specific.
//
// The SAME batch is run through BOTH probes -- each Pisot candidate
// is built into a ContactBoundaryReport once, then both probes run
// on the report.  Probes are intentionally independent of each other:
//   - (A2) involution needs only the contact boundary set + G_B
//   - (A1) cofactor needs additionally the balanced-pair core's
//     charpoly and the G_B core's quotient charpoly (computed via
//     extract_dominant_recurrent_core + coarsest_equitable_partition
//     + quotient_matrix + charpoly_int, per gb_bp_matrix_equality.cpp)
//
// We use the shared involution helpers from
// include/ravel/involution_helpers.hpp -- extracted from
// app/gb_bp_involution_general_n.cpp and app/gb_bp_matrix_equality.cpp,
// so this driver reuses the exact same machinery as the n-bonacci
// case rather than reimplementing it (per session MEMORY_POLICY: be sure
// to look at underlying cpp implementations and avoid reimplementation
// and rework).
//
// Output: per Pisot, [involution: EXACT|partial|FAIL] [cofactor x^k:
// YES|no, k=...]  Tabulates at the end with a verdict: "x^k
// nilpotent cofactor structure is  {frequently-conserved,
// n-bonacci-specific, σ_{a,b}-specific, etc.}" based on the tally.

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <random>
#include <string>
#include <vector>

#include "math/charpoly.hpp"
#include "ravel/balanced_pair.hpp"
#include "ravel/contact_boundary.hpp"
#include "ravel/core.hpp"
#include "ravel/corona.hpp"
#include "ravel/d_cont_check.hpp"
#include "ravel/graph_divisor.hpp"
#include "ravel/involution_helpers.hpp"
#include "ravel/spectral.hpp"
#include "ravel/substitution.hpp"
#include "ravel/survey.hpp"

using namespace ravel;

namespace {

// =====================================================================
// σ_{a,b}-family Pisot substitutions.
//
// Convention (matches what the project's `contact_boundary.hpp` "has
// constant factor" comment calls the σ_{a,b} family):
//   σ(0) = 0^a  1^b  2       (a copies of "0", then b of "1", then "2")
//   σ(1) = 0^a  2            (a copies of "0", then "2")
//   σ(2) = 0
// Companion matrix M (row letter, col image position):
//   [a, a, 1]
//   [b, 0, 0]
//   [1, 1, 0]
// det(M) = b, so |det| = b.  Unimodular iff b = 1.  Char poly:
//   x^3 - (a+1) x^2 + (a(a+1) - 1 - b) x + b   (per the matrix
//   above; I'm not reproducing the algebra here, the project just
//   needs the substitution; the Pisot check via spectral_invariants
//   tells us empirically if each (a, b) is Pisot).
// =====================================================================
struct SigmaAB {
    int a, b;
    std::string label;
};

std::vector<std::vector<std::int8_t>> sigma_ab_rule(int a, int b) {
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

// Build the companion (incidence) matrix M from a substitution rule.
// M[r][c] = count of letter r in σ(c).  This is the format
// classify_matrix_spectral() takes (and the format the rest of the
// project's Pisot-classification machinery expects).
inline std::vector<std::vector<long long>>
matrix_from_subst(const std::vector<std::vector<std::int8_t>>& sigma) {
    std::size_t n = sigma.size();
    std::vector<std::vector<long long>> M(n, std::vector<long long>(n, 0));
    for (std::size_t c = 0; c < n; ++c) {
        for (auto letter : sigma[c]) {
            if (letter >= 0 && static_cast<std::size_t>(letter) < n) {
                M[static_cast<std::size_t>(letter)][c] += 1;
            }
        }
    }
    return M;
}

// σ_1 (3-letter, Pisot, |det| = 2) and σ_2 (3-letter, Pisot, |det| = 2)
// from the project's own FINDINGS_FOR_CITATION.md Finding 1:
//   σ_1(0) = (0,0,0,1) -- 4 letters, so this is a 4-letter run; skip
//   for the 3-letter test set.
// Actually the project has 3-letter σ_1 and σ_2 referenced in the
// "σ_1, σ_2" mismatch table; their images are 3 letters long:
//   σ_1(0) = (0,0,1), σ_1(1) = (0,2), σ_1(2) = (0)  (4-letter
//     version is the "really 4-letter" Pisot)
//   σ_2(0) = (0,0,1), σ_2(1) = (0,0,0,2), σ_2(2) = (0)  (the
//     3-letter version doesn't quite match but the project's table
//     labels both as bit-exact mismatches; we use the
//     "Constant Finals" class here for completeness).
// The construction of a 3-letter σ_1 (Constant Finals at position 3):
//   σ(0) = 0 0 2, σ(1) = 0, σ(2) = 0 2  (1+1+0 = 2+1 = 3 letters)
// is in fact the σ_{1,1} family entry.
std::vector<std::vector<std::int8_t>> sigma1_three_letter() {
    // σ_1 with 3-letter images from FINDINGS_FOR_CITATION.md (line ~125):
    //   σ_1(0) = (0,0,1), σ_1(1) = (0,2), σ_1(2) = (0)
    return {{0, 0, 1}, {0, 2}, {0}};
}
std::vector<std::vector<std::int8_t>> sigma2_three_letter() {
    // σ_2(0) = (0,0,1), σ_2(1) = (0,0,0,2), σ_2(2) = (0)
    // (4-letter image, but the second is the constant-finals case; for
    // the 3-letter test set we collapse the trailing zero to keep
    // alphabet size 3):
    //   σ_2(0) = (0,0,1), σ_2(1) = (0,0,2), σ_2(2) = (0)
    return {{0, 0, 1}, {0, 0, 2}, {0}};
}

// Helper to convert charpoly_int output to "high-first" rep.
// charpoly_int returns low-first (mathlib PolyZ convention);
// check_exact_factor expects high-first (constant term last).  Trivial
// in-place conversion.  This is the same direction as the
// involution_helpers.hpp::from_high_first but reversed.
std::vector<long long> reverse_low_to_high(std::vector<long long> low) {
    std::reverse(low.begin(), low.end());
    return low;
}

// Apply the σ_{a,b} generation to a batch, run both probes, print a
// row per candidate, then a summary at the end.
struct ProbeRow {
    std::string label;
    bool involution_exact;        // A2: full match on dominant core
    int  involution_matched;
    int  involution_total;
    bool cofactor_is_xk;          // A1: charpoly(Q_sym_GB)/chpoly(Q_sym_BP) is x^k
    long cofactor_k;              // = degree(whole) - degree(factor); -1 if not x^k
    bool was_pisot;               // sanity: beta > 1, |b2| < 1
    bool contact_boundary_converged;
    long gb_size;                 // |G_B|
};

ProbeRow run_one(const std::string& label,
                 const std::vector<std::vector<std::int8_t>>& sigma,
                 double beta) {
    ProbeRow r;
    r.label = label;
    r.cofactor_k = -1;
    SubstitutionRule rule(sigma);

    // Use the project's classify_matrix_spectral via the rule-derived
    // matrix -- this is the same Pisot check the rest of the codebase
    // uses; it's bit-exact (Sturm-based) for the 2x2/3x3 case so
    // its verdict matches the algebraic-Pisot test for the families
    // we consider.
    std::vector<std::vector<long long>> M = matrix_from_subst(sigma);
    auto spectral = classify_matrix_spectral(M);
    r.was_pisot = spectral.pisot;

    if (!r.was_pisot) {
        std::printf("\n[%s] not Pisot (alpha=%.4f); skipping probes\n",
                    label.c_str(), spectral.beta);
        return r;
    }

    auto d_cont_cands = search_D_cont<3>(make_substitution<3>(rule, beta), 2);
    std::vector<std::tuple<long long, std::vector<long long>, long long>> d_cont;
    d_cont.reserve(d_cont_cands.size());
    for (const auto& c : d_cont_cands) {
        d_cont.emplace_back(c.i, std::vector<long long>(c.x.begin(), c.x.end()), c.j);
    }
    ContactBoundaryReport rep;
    try {
        rep = compute_contact_boundary_dispatch(rule, beta, 0.0, d_cont);
    } catch (const std::exception& e) {
        std::printf("\n[%s] compute_contact_boundary threw: %s\n",
                    label.c_str(), e.what());
        return r;
    }
    r.contact_boundary_converged = rep.converged;
    r.gb_size = rep.boundary_size;

    if (!rep.converged || rep.gb_matrix.empty()) {
        std::printf("\n[%s] contact-boundary did not converge "
                    "(|G_B|=%zu, closure_stopped_early=%d corona_capped=%d)\n",
                    label.c_str(), r.gb_size,
                    rep.closure_stopped_early ? 1 : 0,
                    rep.corona_capped ? 1 : 0);
        return r;
    }

    // --- A2: involution on the dominant recurrent core ---
    auto [matched, total] = check_involution_on_core(rep, /*print_unmatched=*/false);
    r.involution_matched = matched;
    r.involution_total = total;
    r.involution_exact = (matched == total && total > 0);
    std::printf("\n[%s] beta=%.6f  |G_B|=%zu\n", label.c_str(), beta, r.gb_size);
    std::printf("  [A2 involution]  %d / %d core nodes matched%s\n",
                matched, total,
                r.involution_exact ? "  (EXACT)" : "  (partial)");

    // --- A1: nilpotent cofactor on dominant-core G_B / BP-core charpoly ---
    // Per the n-bonacci Finding 4 (gb_bp_matrix_equality.cpp): the
    // quotient matrix Q_sym_GB (orbits of the involution on the G_B
    // dominant core) has charpoly exactly x^k times Q_sym_BP's.  The
    // cofactor's degree k = (Q_sym_GB degree) - (Q_sym_BP degree).
    //
    // The Q_sym_GB construction is the same as in
    // app/gb_bp_matrix_equality.cpp: orbits of [i,x,j] ↔ [j,-x,i] on
    // the dominant core's nodes.
    auto gb_core = recurrent_core_dense_matrix(rep.gb_matrix);
    auto bp_core = recurrent_core_dense_matrix(
        balanced_pair_transition_graph(rule).matrix);

    // --- A1: nilpotent cofactor on Q_sym_GB / Q_sym_BP ---
    // Per FINDINGS_FOR_CITATION.md Finding 4 and the n-bonacci proof
    // in app/gb_bp_matrix_equality.cpp: the proper structural reason
    // for the n-bonacci's λ(G_B) = ρ_nc equality is the orbit-aware
    // quotient Q_sym_GB under the involution [i,x,j] <-> [j,-x,i] on
    // G_B's dominant recurrent core, and similarly Q_sym_BP on the BP
    // transition graph's recurrent core under the swap (u,v) <-> (v,u).
    // The claim: charpoly(Q_sym_GB) = charpoly(Q_sym_BP) * x^k exactly.
    //
    // The Q_sym constructions are lifted from
    // gb_bp_matrix_equality.cpp into
    // include/ravel/involution_helpers.hpp so this probe reuses the
    // EXACT same construction that proved Finding 4 for the
    // n-bonacci case.
    auto gb_qsym = compute_gb_sym_quotient<3>(rep, rule);
    auto bp_qsym = compute_bp_sym_quotient(rule);

    if (gb_qsym.num_orbits == 0 || bp_qsym.num_orbits == 0) {
        std::printf("  [A1 nilpotent cofactor] skipped (Q_sym construction "
                    "produced empty quotient: G_B_orbits=%zu, BP_orbits=%zu)\n",
                    gb_qsym.num_orbits, bp_qsym.num_orbits);
        r.cofactor_is_xk = false;
        r.cofactor_k = -1;
        return r;
    }
    std::printf("  [A1 prep] Q_sym_GB %zux%zu  (G_B dominant core %zu, "
                "%zu orbits); Q_sym_BP %zux%zu  (BP core %zu, %zu orbits)\n",
                gb_qsym.Qsym.size(), gb_qsym.Qsym.size(),
                gb_qsym.dominant_core_size, gb_qsym.num_orbits,
                bp_qsym.Qsym.size(), bp_qsym.Qsym.size(),
                bp_qsym.recurrent_core_size, bp_qsym.num_orbits);

    // Check if Q_sym_GB is exactly x^k * Q_sym_BP (the A1 claim).
    //   charpoly(Q_sym_GB) / charpoly(Q_sym_BP) == x^k
    // charpoly_int returns low-first (mathlib PolyZ layout, constant
    // first); check_exact_factor expects high-first (constant last).
    // The constructions are over the rationals, so charpoly_int
    // gives bit-exact integer polynomials (no precision loss).
    auto charpoly_gb_qsym = charpoly_int(gb_qsym.Qsym);
    auto charpoly_bp_qsym = charpoly_int(bp_qsym.Qsym);
    auto charpoly_gb_qsym_hf = reverse_low_to_high(charpoly_gb_qsym);
    auto charpoly_bp_qsym_hf = reverse_low_to_high(charpoly_bp_qsym);
    bool exact = check_exact_factor(
        "A1 nilpotent cofactor (Q_sym_GB / Q_sym_BP)",
        charpoly_gb_qsym_hf, charpoly_bp_qsym_hf);
    r.cofactor_is_xk = exact;
    if (exact) {
        // The cofactor's degree k = deg(whole) - deg(factor); both
        // charpoly_int outputs are deg+1 in length.
        long k = static_cast<long>(charpoly_gb_qsym.size())
                 - static_cast<long>(charpoly_bp_qsym.size());
        r.cofactor_k = k;
    } else {
        r.cofactor_k = -1;
    }

    return r;
}

}  // namespace

int main(int argc, char** argv) {
    std::printf("=== Probe (A1) nilpotent-cofactor + (A2) involution on σ_{a,b}-family ===\n\n");
    std::fflush(stdout);

    // Default: a small grid of (a, b=1) σ_{a,b} entries, which is the
    // unimodular subset of the family.  Plus a couple of σ_{a,b=2}
    // for the non-unimodular side.  Plus σ_1, σ_2 (3-letter versions
    // of the project's known Pisot mismatches).  Plus Tribonacci
    // (n=3) as a control, which we EXPECT to fully match per the
    // n-bonacci Finding 4.
    std::vector<SigmaAB> grid;
    for (int a = 0; a <= 5; ++a) grid.push_back({a, 1, "σ_{" + std::to_string(a) + ",1}"});
    grid.push_back({0, 2, "σ_{0,2}"});
    grid.push_back({1, 2, "σ_{1,2}"});
    grid.push_back({2, 2, "σ_{2,2}"});

    for (int i = 1; i < argc; ++i) {
        std::string a = argv[i];
        if (a == "--quiet" || a == "-q") { i = argc; }  // skip arg processing
    }

    std::vector<ProbeRow> rows;
    int n_involution_exact = 0;
    int n_cofactor_xk = 0;
    int n_pisot = 0;
    int n_total_probed = 0;

    // ---- Control: Tribonacci (n=3) ----
    {
        std::printf("\n--- Control: Tribonacci (n=3) ---\n");
        ProbeRow r = run_one("Tribonacci n=3 (control)",
                            n_bonacci_rule(3), n_bonacci_beta(3));
        rows.push_back(r);
        n_total_probed++;
        if (r.was_pisot) n_pisot++;
        if (r.involution_exact) n_involution_exact++;
        if (r.cofactor_is_xk) n_cofactor_xk++;
    }

    // ---- σ_{a,b}-family entries ----
    for (auto& sab : grid) {
        std::printf("\n--- σ_{%d,%d} ---\n", sab.a, sab.b);
        auto M = matrix_from_subst(sigma_ab_rule(sab.a, sab.b));
        auto spectral = classify_matrix_spectral(M);
        if (!spectral.pisot) {
            std::printf("[%s] not Pisot (alpha=%.4f, b2=%.4f); skipping probes\n",
                        sab.label.c_str(), spectral.beta, spectral.b2);
            continue;
        }
        ProbeRow r = run_one(sab.label, sigma_ab_rule(sab.a, sab.b), spectral.beta);
        rows.push_back(r);
        n_total_probed++;
        if (r.was_pisot) n_pisot++;
        if (r.involution_exact) n_involution_exact++;
        if (r.cofactor_is_xk) n_cofactor_xk++;
    }

    // ---- σ_1, σ_2 (3-letter) ----
    auto run_named = [&](const char* label,
                        const std::vector<std::vector<std::int8_t>>& sigma) {
        auto M = matrix_from_subst(sigma);
        auto spectral = classify_matrix_spectral(M);
        if (!spectral.pisot) {
            std::printf("[%s] not Pisot (alpha=%.4f, b2=%.4f); skipping probes\n",
                        label, spectral.beta, spectral.b2);
            return;
        }
        ProbeRow r = run_one(label, sigma, spectral.beta);
        rows.push_back(r);
        n_total_probed++;
        if (r.was_pisot) n_pisot++;
        if (r.involution_exact) n_involution_exact++;
        if (r.cofactor_is_xk) n_cofactor_xk++;
    };
    std::printf("\n--- σ_1 (3-letter) ---\n");
    run_named("σ_1 (3L)", sigma1_three_letter());
    std::printf("\n--- σ_2 (3-letter) ---\n");
    run_named("σ_2 (3L)", sigma2_three_letter());

    // ---- Summary ----
    std::printf("\n=== Probe summary ===\n");
    std::printf("%-12s  %-9s  %-12s  %-12s  %s\n",
                "candidate", "Pisot?",   "A2 invol.",   "A1 x^k",    "|G_B|");
    for (auto& r : rows) {
        std::printf("%-12s  %-9s  %-12s  %-12s  %zu\n",
                    r.label.c_str(),
                    r.was_pisot ? "yes" : "no",
                    (r.contact_boundary_converged ?
                        (r.involution_exact ?
                            "EXACT" : "partial") : "n/a (no G_B)"),
                    r.cofactor_is_xk
                        ? ("YES (k=" + std::to_string(r.cofactor_k) + ")").c_str()
                        : "no",
                    r.gb_size);
    }
    std::printf("\nTotals: %d probed, %d Pisot, %d A2-exact-involution, "
                "%d A1-exact-x^k-cofactor\n",
                n_total_probed, n_pisot,
                n_involution_exact, n_cofactor_xk);
    std::printf("\nInterpretation:\n");
    std::printf("  - If the control (Tribonacci) is in A2-exact + A1-x^k: "
                "driver is correctly reproducing Finding 4's mechanism.\n");
    std::printf("  - If many σ_{a,b}-family Pisots are in A2-exact + A1-x^k:\n"
                "      nilpotent-cofactor mechanism is NOT n-bonacci-specific.\n"
                "      → the conjecture is on its way to a general proof.\n");
    std::printf("  - If σ_{a,b} entries are A2-partial or A1-no:\n"
                "      mechanism is n-bonacci-family-specific.\n"
                "      → the conjecture is a real open problem,\n"
                "        and the n-bonacci proof does not generalize as-is.\n");
    return 0;
}
