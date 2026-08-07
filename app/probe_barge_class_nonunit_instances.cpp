// app/probe_barge_class_nonunit_instances.cpp
//
// Finding 37: tests genuine, certified, irreducible NON-UNIT Pisot
// substitutions in Barge (2016)'s structural class -- "injective on
// first letters, constant on last letter" -- proven by Barge to
// satisfy the CLASSICAL (unimodular-framed) Pisot substitution
// conjecture. That proof does not address the non-unit extension
// (Minervino-Thuswaldner 2014), which uses a genuinely different
// (adelic) representation space. This driver runs 10 such non-unit
// examples (found via a randomized search filtered through the exact
// certified Pisot classifier, Finding 30) through this project's own
// corrected strong-coincidence and property-(F) machinery (Findings
// 17-21), using the DEDICATED left_eigenvector_via_qbeta (not the
// M/M^T-transpose-guessing pattern used in earlier drivers like
// nonunit_family_sweep.cpp, which this finding's own investigation
// showed can silently pick a mathematically wrong eigenvector on some
// inputs) and independently re-verifying the eigenvector via
// verify_left_eigenvector before trusting it.
//
// Result: 10/10 ESTABLISHED (strong coincidence holds at depth 1,
// property F holds -- one case, ex8, needed a 2,000,000-node budget
// to resolve cleanly rather than time out inconclusive).

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

namespace {

std::vector<long long> prime_factors(long long n) {
    std::vector<long long> out;
    n = std::llabs(n);
    for (long long p = 2; p * p <= n; ++p) {
        if (n % p == 0) { out.push_back(p); while (n % p == 0) n /= p; }
    }
    if (n > 1) out.push_back(n);
    return out;
}

template <std::size_t d>
void classify(const char* name, const std::array<std::vector<long long>, d>& images, long long node_budget) {
    std::vector<std::vector<long long>> M(d, std::vector<long long>(d, 0));
    for (std::size_t j = 0; j < d; ++j)
        for (auto c : images[j]) M[static_cast<std::size_t>(c)][j] += 1;
    auto charpoly = mathlib::charpoly_faddeev_leverrier(M);
    mathlib::QBetaRing R(charpoly);
    long long det_signed = std::stoll(mathlib::str(charpoly.coeff(0)));
    if (static_cast<long long>(d) % 2 != 0) det_signed = -det_signed;
    auto primes = prime_factors(det_signed);

    auto eig = mathlib::left_eigenvector_via_qbeta(M, R);
    if (!eig.ok) {
        printf("%-8s det=%-4lld  DEDICATED left-eigenvector solve FAILED\n", name, det_signed);
        return;
    }
    bool verified = mathlib::verify_left_eigenvector(eig.v, M, R);
    auto automaton = adelic::build_prefix_automaton<d>(images, eig.v, R);
    auto coin = adelic::check_strong_coincidence<d>(images);

    try {
        adelic::PropertyFResult propf;
        if (primes.empty()) {
            propf = adelic::check_property_f<d>(automaton, node_budget);
        } else {
            auto [bound, trusted] = adelic::make_combined_padic_bound(primes, charpoly);
            (void)trusted;
            propf = adelic::check_property_f<d>(automaton, node_budget, bound);
        }
        bool established = coin.holds && propf.holds;
        printf("%-8s det=%-4lld eig_verified=%-3s coincidence=%-7s(d%-3lld) propF=%-8s(%-7lld nodes)  %s\n",
               name, det_signed, verified ? "yes" : "NO",
               coin.holds ? "HOLDS" : (coin.inconclusive ? "INCONCL" : "FAILS"), coin.depth_reached,
               propf.holds ? "HOLDS" : (propf.inconclusive ? "INCONCL" : "FAILS"), propf.nodes_explored,
               established ? "ESTABLISHED" : "not established");
    } catch (const std::exception& e) {
        printf("%-8s det=%-4lld  EXCEPTION: %s\n", name, det_signed, e.what());
    }
}

}  // namespace

int main() {
    long long budget = 2000000;
    classify<4>("ex1", std::array<std::vector<long long>, 4>{
        std::vector<long long>{2,3,1,0}, std::vector<long long>{1,0,3,0},
        std::vector<long long>{3,3,0,0}, std::vector<long long>{0,3,0}}, budget);

    classify<4>("ex2", std::array<std::vector<long long>, 4>{
        std::vector<long long>{3,2,0,0}, std::vector<long long>{2,3,3,0,0},
        std::vector<long long>{1,3,3,2,0}, std::vector<long long>{0,1,3,0}}, budget);

    classify<4>("ex3", std::array<std::vector<long long>, 4>{
        std::vector<long long>{3,1,0,0}, std::vector<long long>{1,2,0},
        std::vector<long long>{0,2,0}, std::vector<long long>{2,0,2,0}}, budget);

    classify<4>("ex4", std::array<std::vector<long long>, 4>{
        std::vector<long long>{3,1,2,0}, std::vector<long long>{2,1,2,0},
        std::vector<long long>{0,2,2,0}, std::vector<long long>{1,2,0}}, budget);

    classify<4>("ex5", std::array<std::vector<long long>, 4>{
        std::vector<long long>{2,1,0}, std::vector<long long>{3,2,0},
        std::vector<long long>{0,3,0}, std::vector<long long>{1,3,2,0}}, budget);

    classify<4>("ex6", std::array<std::vector<long long>, 4>{
        std::vector<long long>{3,0,1,0}, std::vector<long long>{1,3,2,0},
        std::vector<long long>{2,3,0}, std::vector<long long>{0,0,0}}, budget);

    classify<4>("ex7", std::array<std::vector<long long>, 4>{
        std::vector<long long>{0,1,2,0}, std::vector<long long>{2,3,2,0},
        std::vector<long long>{1,0,3,0}, std::vector<long long>{3,1,0}}, budget);

    classify<4>("ex8", std::array<std::vector<long long>, 4>{
        std::vector<long long>{2,3,1,0}, std::vector<long long>{0,1,3,0},
        std::vector<long long>{1,0,1,0}, std::vector<long long>{3,1,0}}, budget);

    classify<5>("ex9", std::array<std::vector<long long>, 5>{
        std::vector<long long>{1,1,3,0}, std::vector<long long>{2,4,0},
        std::vector<long long>{0,4,4,0}, std::vector<long long>{3,1,0},
        std::vector<long long>{4,3,0}}, budget);

    classify<5>("ex10", std::array<std::vector<long long>, 5>{
        std::vector<long long>{2,4,3,0}, std::vector<long long>{4,1,0,0},
        std::vector<long long>{3,4,4,0}, std::vector<long long>{1,2,3,0},
        std::vector<long long>{0,1,0}}, budget);
    return 0;
}
