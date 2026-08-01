// sweep_nonunit_property_f_4letter.cpp
//
// Item B1 (docs/DIRECTION_AND_OPEN_THREADS.md): "wider non-unimodular
// survey... wider alphabet size (3-letter, 5-letter, 6-letter)... the
// code currently hardcodes 3 via an explicit 3x3 determinant and
// alphabet_size=3, so this genuinely needs a small code change."
//
// This is that code change, for the 4-letter case specifically.
// app/sweep_nonunit_property_f.cpp explicitly overrode
// wide_random_pisot_survey's own alphabet_size default (which is
// already 4 -- "Defaults picked to make the run useful at n=4" per
// include/ravel/survey.hpp's own comment) down to 3, so this driver
// is mostly just NOT overriding it, plus two real generalizations:
//
//   1. The explicit hand-expanded 3x3 determinant becomes
//      adelic::integer_determinant (already general, already used
//      elsewhere in this codebase for exactly this).
//   2. app/sweep_nonunit_property_f.cpp's `charpoly_is_irreducible_cubic`
//      (rational-root theorem only) is REPLACED, not reused --  it is
//      correct at degree 3 only (no rational root implies irreducible
//      there) but not at degree 4 (e.g. (x^2-2)(x^2-3) has no rational
//      roots yet factors), and would silently pass reducible quartics
//      into machinery (QBetaRing / factor_prime_in_qbeta) that assumes
//      irreducibility -- undefined behavior, not a caught exception,
//      per that file's own comment about what crashed its first
//      version. adelic::is_irreducible_over_q_via_small_primes
//      (include/adelic/fp_poly_factor.hpp, built for exactly this) is
//      used instead: a rigorous mod-p sufficient condition, correct at
//      any degree.
//
// RESULTS (2026-08-01, target=8, reduced certify/rho budgets for
// tractability -- see main()).
//
// FIRST RUN: the machinery did NOT generalize to 4-letter the way
// docs/DIRECTION_AND_OPEN_THREADS.md's Item B1 previously assumed
// ("the contact-boundary pipeline, the combined p-adic bound, and the
// spectral filter already handle all these"). Of 7 non-unit quartic
// candidates that passed irreducibility, only 1 reached a verdict; the
// other 6 hit "check_property_f: secondary root modulus >= 1" (4
// cases) or "local_polynomial_cofactor: computed m_k has wrong degree"
// (2 cases).
//
// The first exception traced to its ACTUAL root cause, one layer up,
// not a check_property_f bug at all: include/ravel/spectral.hpp's
// spectral_invariants_general (the n>=4 Pisot classifier
// wide_random_pisot_survey itself uses to decide which candidates are
// Pisot in the FIRST place) underestimated the second-largest
// eigenvalue's modulus via Wielandt-deflation power iteration when
// it's part of a genuinely dominant complex-conjugate pair -- e.g.
// rndW3_1's original matrix got beta2=0.926 (passing the Pisot filter)
// when the true value is 1.376. check_property_f's exception was
// correctly catching a non-Pisot matrix that never should have been
// admitted. FIXED (same session): spectral_invariants_general now uses
// a Rayleigh-Ritz step on the Wielandt-deflated matrix's dominant
// invariant subspace instead of a bare norm-growth ratio -- see
// tests/spectral_general_test.cpp and
// docs/DIRECTION_AND_OPEN_THREADS.md Item B1 for the full trace.
//
// SECOND RUN, after the fix: 6/7 candidates now reach a verdict (all
// ESTABLISHED, tiles) -- only rndW3_5 still hits the SEPARATE,
// unrelated local_polynomial_cofactor exception, which remains
// genuinely open (not investigated). The spectral-filter fix alone
// unblocked the large majority of the 4-letter pipeline.
//
// Build (unregistered probe, matching sibling
// app/sweep_nonunit_property_f.cpp's own convention):
//   g++ -std=c++20 -O2 -Iinclude -Imath/include -Imath/include/mini-gmp \
//       app/sweep_nonunit_property_f_4letter.cpp math/out/libmath.a \
//       -o sweep_nonunit_property_f_4letter

#include <array>
#include <cstdio>
#include <string>
#include <vector>

#include "math/charpoly.hpp"
#include "math/qbeta.hpp"
#include "math/linalg_qbeta.hpp"
#include "math/poly_z.hpp"
#include "adelic/dedekind_factorization.hpp"
#include "adelic/fp_poly_factor.hpp"
#include "adelic/ideal_arithmetic.hpp"
#include "adelic/maximal_order.hpp"
#include "adelic/prefix_automaton.hpp"
#include "adelic/coincidence_and_property_f.hpp"
#include "adelic/classify_adelic.hpp"
#include "ravel/survey.hpp"
#include "ravel/substitution.hpp"

using namespace ravel;
using adelic::classify_tiling;
using adelic::HeaderStyle;
using adelic::print_case_header;
using adelic::TilingVerdict;
using adelic::verdict_label;

namespace {

constexpr std::size_t kAlphabet = 4;

std::vector<long long> prime_factors(long long n) {
    std::vector<long long> out;
    n = std::llabs(n);
    for (long long p = 2; p * p <= n; ++p) {
        if (n % p == 0) {
            out.push_back(p);
            while (n % p == 0) n /= p;
        }
    }
    if (n > 1) out.push_back(n);
    return out;
}

std::vector<std::vector<long long>> transpose(const std::vector<std::vector<long long>>& M) {
    std::size_t n = M.size();
    std::vector<std::vector<long long>> T(n, std::vector<long long>(n, 0));
    for (std::size_t i = 0; i < n; ++i)
        for (std::size_t j = 0; j < n; ++j)
            T[j][i] = M[i][j];
    return T;
}

template <std::size_t d>
const char* classify(const char* name,
                      const std::array<std::vector<long long>, d>& images,
                      const mathlib::PolyZ& charpoly,
                      const std::vector<std::vector<long long>>& M_transpose,
                      const std::vector<long long>& primes_dividing_det) {
    print_case_header(HeaderStyle::SHORT_DASH, name);
    std::printf("charpoly: %s   primes dividing det: ", mathlib::str(charpoly).c_str());
    for (auto p : primes_dividing_det) std::printf("%lld ", p);
    if (primes_dividing_det.empty()) std::printf("(none -- unit)");
    std::printf("\n");

    for (long long p : primes_dividing_det) {
        auto fac = adelic::factor_prime_in_qbeta(charpoly, p);
        bool cross_ok = adelic::cross_check_dedekind_factorization(fac, charpoly, static_cast<long long>(d));
        std::printf("  Factoring (%lld): maximal=%s cross-checked=%s\n", p,
                    fac.maximal ? "yes" : "no", cross_ok ? "match" : "MISMATCH");
        for (const auto& pi : fac.prime_ideals) {
            std::printf("    p=%lld e=%lld f=%lld ef=%lld\n",
                        pi.p, pi.e, pi.f, pi.e * pi.f);
        }
    }
    std::printf("\n");

    auto cls = classify_tiling<d>(name, images, charpoly, M_transpose, primes_dividing_det);

    std::printf("  Strong coincidence: %s (depth %lld%s)\n",
                cls.strong_coincidence_holds ? "HOLDS"
                    : cls.strong_coincidence_inconclusive ? "INCONCLUSIVE" : "FAILS",
                cls.strong_coincidence_depth,
                cls.strong_coincidence_inconclusive ? " [cutoff]" : "");
    std::printf("  Property (F) [%s]: %s (%lld nodes%s)\n",
                primes_dividing_det.empty() ? "unit, VALIDATED"
                    : (primes_dividing_det.size() == 1
                          ? "combined p-adic bound, VALIDATED"
                          : "combined p-adic bound across multiple primes, VALIDATED"),
                cls.property_f_holds ? "HOLDS" : (cls.property_f_inconclusive ? "INCONCLUSIVE" : "FAILS"),
                cls.property_f_nodes,
                cls.property_f_inconclusive ? " [budget]" : "");
    std::printf("  VERDICT: %s\n\n", verdict_label(cls.verdict));

    static thread_local std::string s_v;
    s_v = verdict_label(cls.verdict);
    return s_v.c_str();
}

}  // namespace

int main() {
    std::printf("=== Sweep: random NON-UNIT (non-unimodular) Pisot substitutions, %zu-letter ===\n\n", kAlphabet);
    auto candidates = wide_random_pisot_survey(
        /*target=*/8, /*K_max=*/3, /*seed=*/11,
        /*max_pairs_per_certify=*/2000, /*max_len_per_certify=*/15000,
        /*max_pairs_for_rho=*/2000, /*max_len_for_rho=*/15000,
        /*max_trials=*/20000, /*alphabet_size=*/static_cast<int>(kAlphabet));
    std::printf("Generated %zu Pisot %zu-letter candidates.\n\n", candidates.size(), kAlphabet);

    int n_nonunit = 0, n_checked = 0;
    int n_established = 0, n_inconclusive = 0, n_fail = 0, n_skipped = 0;
    for (auto& inst : candidates) {
        long long det = adelic::integer_determinant(inst.M);
        if (std::llabs(det) <= 1) continue;  // want NON-unit this time
        n_nonunit++;
        n_checked++;

        auto charpoly = mathlib::charpoly_faddeev_leverrier(inst.M);
        if (!adelic::is_irreducible_over_q_via_small_primes(charpoly)) {
            std::printf("--- %s ---\n  SKIPPED: charpoly %s is REDUCIBLE (or inconclusive) over Q -- "
                         "not a genuine quartic field, QBetaRing's assumptions don't hold\n\n",
                         inst.name.c_str(), mathlib::str(charpoly).c_str());
            n_skipped++;
            continue;
        }
        std::array<std::vector<long long>, kAlphabet> images;
        for (std::size_t i = 0; i < kAlphabet; ++i)
            for (auto c : inst.sigma[i]) images[i].push_back(static_cast<long long>(c));

        auto primes = prime_factors(det);
        std::string verdict;
        try {
            verdict = classify<kAlphabet>(inst.name.c_str(), images, charpoly, transpose(inst.M), primes);
        } catch (const std::exception& e) {
            std::printf("--- %s ---\n  EXCEPTION: %s\n  VERDICT: SKIPPED (exception)\n\n",
                         inst.name.c_str(), e.what());
            verdict = "SKIPPED (exception)";
        }
        if (verdict.find("ESTABLISHED") != std::string::npos) n_established++;
        else if (verdict.find("INCONCLUSIVE") != std::string::npos) n_inconclusive++;
        else if (verdict.find("SKIPPED") != std::string::npos) n_skipped++;
        else n_fail++;
    }
    std::printf("=== Summary: %d non-unit %zu-letter candidates checked -- "
                "%d ESTABLISHED, %d INCONCLUSIVE, %d FAILED/does-not-tile, %d skipped ===\n",
                n_checked, kAlphabet, n_established, n_inconclusive, n_fail, n_skipped);
    return 0;
}
