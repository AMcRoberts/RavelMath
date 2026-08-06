// app/probe_ar_family_strong_coincidence.cpp
//
// Finding 5 (docs/FINDINGS_FOR_CITATION.md) classified 10 unimodular
// Pisot substitutions into AR-exact (Tribonacci/n-bonacci), AR-partial
// (sigma_{a,1} a>=1, sigma_1, sigma_2), and non-AR (sigma_{0,1},
// sigma_{0,2}), for a DIFFERENT project-internal conjecture
// (rho_nc = lambda(G_B)). That tabulation never ran the actual
// classical Pisot tiling conjecture check (strong coincidence +
// property (F), adelic/coincidence_and_property_f.hpp) on the
// AR-partial/non-AR members at all -- only Tribonacci (already
// covered by citation, Barge 2015/2018, as a beta-substitution) and
// sigma_1/sigma_2 (already checked separately, both ESTABLISHED, see
// sweep_mismatches_property_f.cpp) had this run.
//
// This closes that gap: run check_strong_coincidence + check_property_f
// on sigma_{0,1} through sigma_{5,1} and sigma_{1,2} directly, before
// attempting any proof strategy for the AR-partial/non-AR classes --
// per this project's own repeated discipline, verify computationally
// before building theory on top.
//
// sigma_{a,b} definition matches app/tabulate_pisot_properties.cpp and
// app/probe_a1_a2_unimodular.cpp exactly (3-letter alphabet):
//   sigma(0) = 0^a 1^b 2
//   sigma(1) = 0^a 2
//   sigma(2) = 0
// a=0,b=1 is the "non-AR" case; a>=1,b=1 is "AR-partial"; a=1,b=2 is
// also tabulated in Finding 5's set.

#include <array>
#include <cstdio>
#include <vector>

#include "math/charpoly.hpp"
#include "math/qbeta.hpp"
#include "math/linalg_qbeta.hpp"
#include "math/poly_z.hpp"
#include "adelic/coincidence_and_property_f.hpp"
#include "adelic/dedekind_factorization.hpp"
#include "adelic/prefix_automaton.hpp"

namespace {

std::vector<std::vector<long long>> sigma_ab(int a, int b) {
    std::vector<std::vector<long long>> sigma(3);
    for (int i = 0; i < a; ++i) sigma[0].push_back(0);
    for (int i = 0; i < b; ++i) sigma[0].push_back(1);
    sigma[0].push_back(2);
    for (int i = 0; i < a; ++i) sigma[1].push_back(0);
    sigma[1].push_back(2);
    sigma[2] = {0};
    return sigma;
}

std::array<std::vector<long long>, 3> to_images(const std::vector<std::vector<long long>>& sigma) {
    std::array<std::vector<long long>, 3> out;
    for (int i = 0; i < 3; ++i) out[static_cast<std::size_t>(i)] = sigma[static_cast<std::size_t>(i)];
    return out;
}

std::vector<std::vector<long long>> incidence_matrix(const std::vector<std::vector<long long>>& sigma) {
    std::vector<std::vector<long long>> M(3, std::vector<long long>(3, 0));
    for (std::size_t j = 0; j < 3; ++j)
        for (long long letter : sigma[j]) M[static_cast<std::size_t>(letter)][j] += 1;
    return M;
}

std::vector<std::vector<long long>> transpose(const std::vector<std::vector<long long>>& M) {
    std::size_t n = M.size();
    std::vector<std::vector<long long>> T(n, std::vector<long long>(n, 0));
    for (std::size_t i = 0; i < n; ++i)
        for (std::size_t j = 0; j < n; ++j) T[j][i] = M[i][j];
    return T;
}

long long det3(const std::vector<std::vector<long long>>& M) {
    return M[0][0] * (M[1][1] * M[2][2] - M[1][2] * M[2][1])
         - M[0][1] * (M[1][0] * M[2][2] - M[1][2] * M[2][0])
         + M[0][2] * (M[1][0] * M[2][1] - M[1][1] * M[2][0]);
}

void classify(const char* name, int a, int b) {
    auto sigma = sigma_ab(a, b);
    auto images = to_images(sigma);
    auto M = incidence_matrix(sigma);
    long long det = det3(M);
    long long absdet = det < 0 ? -det : det;
    std::printf("== %-12s (a=%d,b=%d)  |det M|=%lld  ", name, a, b, absdet);

    auto charpoly = mathlib::charpoly_faddeev_leverrier(M);
    mathlib::QBetaRing R(charpoly);
    auto Mt = transpose(M);
    auto eig = mathlib::right_eigenvector_via_qbeta(Mt, R);
    if (!eig.ok) {
        std::printf("EIGENVECTOR FAILED\n");
        return;
    }

    // Non-unit: needs the p-adic bound too, or property F's
    // archimedean-only search never terminates (see BUG 2 in
    // coincidence_and_property_f.hpp -- this is the documented, not
    // mysterious, reason it would otherwise run to budget).
    std::vector<long long> primes;
    if (absdet > 1) {
        long long n = absdet;
        for (long long p = 2; p * p <= n; ++p) { if (n % p == 0) { primes.push_back(p); while (n % p == 0) n /= p; } }
        if (n > 1) primes.push_back(n);
    }

    auto coin = adelic::check_strong_coincidence<3>(images);
    auto automaton = adelic::build_prefix_automaton<3>(images, eig.v, R);
    adelic::PropertyFResult propf;
    if (primes.empty()) {
        propf = adelic::check_property_f<3>(automaton, 300000);
    } else {
        auto [bound, trusted] = adelic::make_combined_padic_bound(primes, charpoly);
        (void)trusted;
        propf = adelic::check_property_f<3>(automaton, 300000, bound);
    }

    std::printf("coincidence=%s(depth %lld) propertyF=%s(%lld nodes)  verdict=%s\n",
                coin.holds ? "HOLDS" : (coin.inconclusive ? "INCONCLUSIVE" : "FAILS"),
                coin.depth_reached,
                propf.holds ? "HOLDS" : (propf.inconclusive ? "INCONCLUSIVE" : "FAILS"),
                propf.nodes_explored,
                (coin.holds && propf.holds) ? "ESTABLISHED"
                : (coin.inconclusive || propf.inconclusive) ? "INCONCLUSIVE"
                : "*** FAILS -- POSSIBLE COUNTEREXAMPLE ***");
}

}  // namespace

int main() {
    std::printf("Finding 5's AR-partial/non-AR family run through the actual classical\n");
    std::printf("Pisot tiling conjecture check (strong coincidence + property F), for the\n");
    std::printf("first time -- this was never done when Finding 5 was written.\n\n");

    classify("sigma_{0,1}", 0, 1);  // non-AR
    classify("sigma_{1,1}", 1, 1);  // AR-partial
    classify("sigma_{2,1}", 2, 1);
    classify("sigma_{3,1}", 3, 1);
    classify("sigma_{4,1}", 4, 1);
    classify("sigma_{5,1}", 5, 1);
    classify("sigma_{1,2}", 1, 2);
    return 0;
}
