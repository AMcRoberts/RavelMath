// sweep_nonunit_property_f.cpp
//
// Non-unit counterpart to sweep_mismatches_property_f.cpp.  That
// sweep covered random UNIT (unimodular) Pisot substitutions and
// every single mismatch resolved trivially (strong coincidence +
// property (F) both HOLD, no exceptions).  The genuinely open
// territory per the literature is non-unit Pisot substitutions --
// this driver generates a fresh batch of those and runs the SAME
// two checks, via the SHARED `classify_tiling` entry point in
// adelic/classify_adelic.hpp (the same one
// app/classify_adelic_tiling.cpp uses, just with the short-dash
// header style and the verdict-as-const-char return for the
// summary counter at the bottom).
//
// Build: g++ -std=c++20 -O2 -Iinclude -Imath/include -Imath/include/mini-gmp \
//        app/sweep_nonunit_property_f.cpp math/out/libmath.a -o sweep_nonunit_property_f

#include <array>
#include <cstdio>
#include <string>
#include <vector>

#include "math/charpoly.hpp"
#include "math/qbeta.hpp"
#include "math/linalg_qbeta.hpp"
#include "math/poly_z.hpp"
#include "adelic/dedekind_factorization.hpp"
#include "adelic/ideal_arithmetic.hpp"
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

// classify_adelic_tiling.cpp's own charpolys are all hand-picked and
// come with the comment "(irreducibility over Q assumed/externally
// verified ... this driver does not re-derive it)" -- i.e. that
// driver silently relies on the caller having already checked this.
// wide_random_pisot_survey's own "pisot" classification does NOT
// check irreducibility (only that the matrix's own spectrum has one
// dominant real root >1 and everything else <1 in modulus -- a
// reducible charpoly with a spurious extra real eigenvalue can still
// pass that).  Feeding a reducible charpoly into QBetaRing /
// factor_prime_in_qbeta (which assume it IS the minimal polynomial
// of beta, i.e. irreducible) is undefined behavior, not a thrown
// exception -- this is exactly what crashed the first version of
// this sweep (segfault on rndW3_31, a reducible cubic).  Since the
// charpoly here is monic, any rational root must be an integer
// dividing the constant term (rational root theorem) -- cheap and
// exact to check.
bool charpoly_is_irreducible_cubic(const mathlib::PolyZ& p) {
    long long c0 = 0;
    { std::string s = mathlib::str(p.coeff(0)); c0 = std::stoll(s); }
    if (c0 == 0) return false;  // x=0 is a root
    for (long long d = 1; d * d <= std::llabs(c0); ++d) {
        if (c0 % d != 0) continue;
        for (long long cand : {d, -d, c0 / d, -c0 / d}) {
            long long v = 0, pw = 1;
            for (long long k = 0; k <= 3; ++k) {
                long long ck = std::stoll(mathlib::str(p.coeff(static_cast<std::size_t>(k))));
                v += ck * pw;
                pw *= cand;
            }
            if (v == 0) return false;
        }
    }
    return true;
}

std::vector<std::vector<long long>> transpose(const std::vector<std::vector<long long>>& M) {
    std::size_t n = M.size();
    std::vector<std::vector<long long>> T(n, std::vector<long long>(n, 0));
    for (std::size_t i = 0; i < n; ++i)
        for (std::size_t j = 0; j < n; ++j)
            T[j][i] = M[i][j];
    return T;
}

// Adapter: call the shared classify_tiling<> with the sweep's
// short-dash header style, the PisotInstance's charpoly/matrix/
// images, and the primes dividing det.  Returns the const-char
// verdict string for the summary counter.  The shared header
// prints the strong-coincidence line and the property-(F) line;
// this wrapper also prints the short-dash case header, the
// per-prime factorization lines (the sweep's own diagnostic
// format), and the verdict line in the sweep's two-up format
// (verdict + reason on consecutive lines).
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

    // Per-prime factorization diagnostic in the sweep's own format.
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

    // The shared header is the actual classifier; we just decorate
    // the per-step diagnostic lines in the sweep's own format.
    auto cls = classify_tiling<d>(name, images, charpoly, M_transpose, primes_dividing_det);

    // Emit the strong-coincidence / property-(F) / verdict lines
    // in the sweep's compact format.  (The shared header doesn't
    // print these on its own -- it returns the values.)
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

    // Return a stable const-char* that the summary counter at the
    // bottom of main can pattern-match against.
    static thread_local std::string s_v;
    s_v = verdict_label(cls.verdict);
    return s_v.c_str();
}

}  // namespace

int main() {
    std::printf("=== Sweep: random NON-UNIT (non-unimodular) Pisot substitutions ===\n\n");
    auto candidates = wide_random_pisot_survey(
        /*target=*/40, /*K_max=*/3, /*seed=*/11,
        /*max_pairs_per_certify=*/8000, /*max_len_per_certify=*/60000,
        /*max_pairs_for_rho=*/8000, /*max_len_for_rho=*/60000,
        /*max_trials=*/20000, /*alphabet_size=*/3);
    std::printf("Generated %zu Pisot 3-letter candidates.\n\n", candidates.size());

    int n_nonunit = 0, n_checked = 0;
    int n_established = 0, n_inconclusive = 0, n_fail = 0, n_skipped = 0;
    for (auto& inst : candidates) {
        long long det = inst.M[0][0] * (inst.M[1][1] * inst.M[2][2] - inst.M[1][2] * inst.M[2][1])
                       - inst.M[0][1] * (inst.M[1][0] * inst.M[2][2] - inst.M[1][2] * inst.M[2][0])
                       + inst.M[0][2] * (inst.M[1][0] * inst.M[2][1] - inst.M[1][1] * inst.M[2][0]);
        if (std::llabs(det) <= 1) continue;  // want NON-unit this time
        n_nonunit++;
        n_checked++;

        auto charpoly = mathlib::charpoly_faddeev_leverrier(inst.M);
        if (!charpoly_is_irreducible_cubic(charpoly)) {
            std::printf("--- %s ---\n  SKIPPED: charpoly %s is REDUCIBLE over Q (has a rational root) -- "
                         "not a genuine cubic field, QBetaRing's assumptions don't hold\n\n",
                         inst.name.c_str(), mathlib::str(charpoly).c_str());
            n_skipped++;
            continue;
        }
        std::array<std::vector<long long>, 3> images;
        for (std::size_t i = 0; i < 3; ++i)
            for (auto c : inst.sigma[i]) images[i].push_back(static_cast<long long>(c));

        // The multi-prime case (|det| divisible by 2+ distinct
        // rational primes) used to be skipped here, because the
        // per-prime bound-selection logic only used the first prime.
        // As of the make_combined_padic_bound refactor, the unified
        // bound is the AND of every prime's per-ideal predicate -- so
        // the multi-prime case is genuinely handled, no skip needed.
        auto primes = prime_factors(det);
        std::string verdict;
        try {
            verdict = classify<3>(inst.name.c_str(), images, charpoly, transpose(inst.M), primes);
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
    std::printf("=== Summary: %d non-unit candidates checked -- "
                "%d ESTABLISHED, %d INCONCLUSIVE, %d FAILED/does-not-tile, %d skipped ===\n",
                n_checked, n_established, n_inconclusive, n_fail, n_skipped);
    return 0;
}
