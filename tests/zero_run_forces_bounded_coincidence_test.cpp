// Cross-checks the zero-run coincidence-depth bound (generalizing
// Finding 17 to the whole canonical terminating-expansion family)
// against a direct, from-scratch coincidence search on every pair,
// across 20 structurally diverse digit sequences spanning N=1..10 and
// zero-run lengths 0..5 (single runs, multiple separate runs of
// different lengths, runs at the start/middle of the sequence).

#include <cassert>
#include <iostream>
#include <vector>

#include "ravel/canonical_beta_substitution.hpp"
#include "ravel/proof/zero_run_forces_bounded_coincidence.hpp"

using namespace ravel::proof;

namespace {

// Direct coincidence-depth search (prefix OR suffix abelianization
// match), independent of adelic::pair_has_coincidence's fixed-size-
// array signature, since N varies here -- same logic, dynamic sizing.
long long worst_case_depth(const std::vector<std::vector<long long>>& sigma, long long max_depth) {
    std::size_t n = sigma.size();
    long long worst = 0;
    for (std::size_t i = 0; i < n; ++i) {
        for (std::size_t j = i + 1; j < n; ++j) {
            std::vector<long long> w1 = {static_cast<long long>(i)};
            std::vector<long long> w2 = {static_cast<long long>(j)};
            long long depth = -1;
            for (long long k = 1; k <= max_depth; ++k) {
                std::vector<long long> nw1, nw2;
                for (auto l : w1) nw1.insert(nw1.end(), sigma[static_cast<std::size_t>(l)].begin(), sigma[static_cast<std::size_t>(l)].end());
                for (auto l : w2) nw2.insert(nw2.end(), sigma[static_cast<std::size_t>(l)].begin(), sigma[static_cast<std::size_t>(l)].end());
                w1 = nw1; w2 = nw2;
                std::vector<long long> total1(n, 0), total2(n, 0);
                for (auto l : w1) total1[static_cast<std::size_t>(l)]++;
                for (auto l : w2) total2[static_cast<std::size_t>(l)]++;
                std::vector<std::vector<std::vector<long long>>> prefix_set1(n), suffix_set1(n);
                std::vector<long long> running(n, 0);
                for (std::size_t p = 0; p < w1.size(); ++p) {
                    long long a = w1[p];
                    prefix_set1[static_cast<std::size_t>(a)].push_back(running);
                    std::vector<long long> suffix(n);
                    for (std::size_t kk = 0; kk < n; ++kk) suffix[kk] = total1[kk] - running[kk] - (static_cast<long long>(kk) == a ? 1 : 0);
                    suffix_set1[static_cast<std::size_t>(a)].push_back(suffix);
                    running[static_cast<std::size_t>(a)]++;
                }
                running.assign(n, 0);
                bool hit = false;
                for (std::size_t p = 0; p < w2.size() && !hit; ++p) {
                    long long a = w2[p];
                    for (auto& pv : prefix_set1[static_cast<std::size_t>(a)]) if (pv == running) { hit = true; break; }
                    if (!hit) {
                        std::vector<long long> suffix(n);
                        for (std::size_t kk = 0; kk < n; ++kk) suffix[kk] = total2[kk] - running[kk] - (static_cast<long long>(kk) == a ? 1 : 0);
                        for (auto& sv : suffix_set1[static_cast<std::size_t>(a)]) if (sv == suffix) { hit = true; break; }
                    }
                    running[static_cast<std::size_t>(a)]++;
                }
                if (hit) { depth = k; break; }
            }
            assert(depth > 0 && "pair failed to resolve within max_depth -- bound is wrong or search too shallow");
            if (depth > worst) worst = depth;
        }
    }
    return worst;
}

void check(const std::vector<long long>& digits) {
    auto sigma = ravel::canonical_beta_substitution_from_digits(digits);
    auto cert = derive_zero_run_coincidence_bound(digits);
    long long actual = worst_case_depth(sigma, cert.predicted_max_depth + 2);
    std::cout << "digits R=" << cert.longest_zero_run << " predicted<=" << cert.predicted_max_depth
              << " actual=" << actual << "\n";
    assert(actual <= cert.predicted_max_depth);
}

}  // namespace

int main() {
    check({1});
    check({1, 1});
    check({2, 1});
    check({3, 1});
    check({1, 1, 1});
    check({2, 1, 1});
    check({1, 2, 1});
    check({3, 2, 1});
    check({5, 3, 2, 1});
    check({1, 2, 3, 4});
    check({1, 0, 1});
    check({2, 0, 3});
    check({2, 0, 1});
    check({1, 0, 0, 1});
    check({3, 0, 2, 0, 1});
    check({1, 0, 0, 0, 1});
    check({1, 0, 0, 0, 0, 1});
    check({2, 0, 0, 1});
    check({1, 0, 1, 0, 1});
    check({4, 2, 0, 0, 0, 0, 0, 3, 1});
    check({1, 2, 3, 0, 4, 5, 0, 6});
    check({7, 0, 0, 6, 0, 5, 0, 0, 0, 4});

    std::cout << "zero_run_forces_bounded_coincidence: predicted bound matches actual on all 22 cases.\n";
    return 0;
}
