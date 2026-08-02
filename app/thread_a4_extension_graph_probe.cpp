// thread_a4_extension_graph_probe.cpp
//
// Thread A4: locate the language branching that distinguishes the
// non-AR sigma_{0,1} orbit from AR-exact Tribonacci and the AR-partial
// sigma_{a,1} family. For every factor u of length n occurring away
// from the sampled orbit's endpoints, collect
//
//   L(u) = {a : au occurs}, R(u) = {b : ub occurs},
//   E(u) = {(a,b) : aub occurs}.
//
// Arnoux-Rauzy languages have one left-special and one right-special
// factor at every length, each with full alphabet valence. Multiple
// special factors expose the precise local branching hidden by the
// aggregate complexity count p(n).
//
// Observed result (2026-08-01, orbit prefixes 5.2e5--9.3e5 symbols,
// factor lengths 1..64): sigma_{0,1} has p(n)=5n-5 for every checked
// n>=4, with five left-special and three right-special factors at
// every checked n>=4. By contrast every AR-complexity control has
// p(n)=2n+1: Tribonacci and sigma_2 realize this with one ternary
// special factor on each side, sigma_{a,1} with two binary left-
// special factors and one ternary right-special factor, and sigma_1
// with the left/right orientation reversed. This is a finite exact
// language computation, not yet an all-n theorem or a proved
// correspondence with stepped-hyperplane seam types.

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <map>
#include <set>
#include <string>
#include <vector>

#include "ravel/substitution.hpp"

using ravel::SubstitutionRule;

namespace {

using Word = std::vector<std::int8_t>;
using Sigma = std::vector<Word>;

Sigma sigma_ab(int a, int b) {
    Sigma sigma(3);
    for (int i = 0; i < a; ++i) sigma[0].push_back(0);
    for (int i = 0; i < b; ++i) sigma[0].push_back(1);
    sigma[0].push_back(2);
    for (int i = 0; i < a; ++i) sigma[1].push_back(0);
    sigma[1].push_back(2);
    sigma[2] = {0};
    return sigma;
}

Sigma tribonacci() {
    return {{0, 1}, {0, 2}, {0}};
}

Sigma sigma1_3l() { return {{0, 0, 1}, {0, 2}, {0}}; }
Sigma sigma2_3l() { return {{0, 0, 1}, {0, 0, 2}, {0}}; }

Word orbit_prefix(const SubstitutionRule& rule, std::size_t minimum) {
    Word orbit{0};
    for (std::size_t round = 0; round < 128 && orbit.size() < minimum; ++round) {
        orbit = rule.apply_once(orbit);
    }
    return orbit;
}

struct Extensions {
    std::set<std::int8_t> left;
    std::set<std::int8_t> right;
    std::set<std::pair<std::int8_t, std::int8_t>> both;
};

void report(const char* name, const Sigma& sigma) {
    const SubstitutionRule rule(sigma);
    const Word orbit = orbit_prefix(rule, 1U << 19);
    std::printf("=== %s (orbit=%zu) ===\n", name, orbit.size());
    std::printf(" n  p(n)  left-special  right-special  bispecial  max-L  max-R  sum bilateral\n");
    for (std::size_t n = 1; n <= 64; ++n) {
        std::map<Word, Extensions> factors;
        for (std::size_t i = 1; i + n < orbit.size(); ++i) {
            Word factor(orbit.begin() + static_cast<std::ptrdiff_t>(i),
                        orbit.begin() + static_cast<std::ptrdiff_t>(i + n));
            auto& ext = factors[factor];
            ext.left.insert(orbit[i - 1]);
            ext.right.insert(orbit[i + n]);
            ext.both.insert({orbit[i - 1], orbit[i + n]});
        }
        std::size_t left_special = 0;
        std::size_t right_special = 0;
        std::size_t bispecial = 0;
        std::size_t max_left = 0;
        std::size_t max_right = 0;
        long long bilateral_sum = 0;
        for (const auto& [factor, ext] : factors) {
            (void)factor;
            const bool ls = ext.left.size() >= 2;
            const bool rs = ext.right.size() >= 2;
            left_special += ls;
            right_special += rs;
            bispecial += ls && rs;
            max_left = std::max(max_left, ext.left.size());
            max_right = std::max(max_right, ext.right.size());
            bilateral_sum += static_cast<long long>(ext.both.size())
                - static_cast<long long>(ext.left.size())
                - static_cast<long long>(ext.right.size()) + 1;
        }
        std::printf("%2zu  %4zu  %12zu  %13zu  %9zu  %5zu  %5zu  %+13lld\n",
                    n, factors.size(), left_special, right_special,
                    bispecial, max_left, max_right, bilateral_sum);
    }
    std::printf("\n");
}

}  // namespace

int main() {
    report("Tribonacci (AR-exact)", tribonacci());
    report("sigma_{1,1} (AR-partial)", sigma_ab(1, 1));
    report("sigma_{2,1} (AR-partial)", sigma_ab(2, 1));
    report("sigma_1 (AR-partial)", sigma1_3l());
    report("sigma_2 (AR-partial)", sigma2_3l());
    report("sigma_{0,1} (non-AR)", sigma_ab(0, 1));
    return 0;
}
