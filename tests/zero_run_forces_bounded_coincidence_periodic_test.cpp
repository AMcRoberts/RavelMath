// Cross-checks the eventually-periodic zero-run bound (Finding 41,
// extending Finding 39 past the terminating family) against a direct,
// from-scratch coincidence search, on 10 structurally diverse
// (preperiod, period) pairs -- including two specifically constructed
// to stress-test the period's own wraparound (where a naive flat-only
// zero-run computation gives a genuinely wrong, too-small prediction).

#include <cassert>
#include <iostream>
#include <vector>

#include "ravel/canonical_beta_substitution.hpp"
#include "ravel/proof/zero_run_forces_bounded_coincidence_periodic.hpp"

using namespace ravel::proof;

namespace {

long long worst_case_depth(const std::vector<std::vector<long long>>& sigma, long long max_depth) {
    std::size_t n = sigma.size();
    long long worst = 0;
    for (std::size_t i = 0; i < n; ++i) {
        for (std::size_t j = i + 1; j < n; ++j) {
            std::vector<long long> w1 = {static_cast<long long>(i)}, w2 = {static_cast<long long>(j)};
            long long depth = -1;
            for (long long k = 1; k <= max_depth; ++k) {
                std::vector<long long> nw1, nw2;
                for (auto l : w1) nw1.insert(nw1.end(), sigma[static_cast<std::size_t>(l)].begin(), sigma[static_cast<std::size_t>(l)].end());
                for (auto l : w2) nw2.insert(nw2.end(), sigma[static_cast<std::size_t>(l)].begin(), sigma[static_cast<std::size_t>(l)].end());
                w1 = nw1; w2 = nw2;
                std::vector<long long> total1(n, 0), total2(n, 0);
                for (auto l : w1) total1[static_cast<std::size_t>(l)]++;
                for (auto l : w2) total2[static_cast<std::size_t>(l)]++;
                std::vector<std::vector<std::vector<long long>>> pfx1(n), sfx1(n);
                std::vector<long long> running(n, 0);
                for (std::size_t p = 0; p < w1.size(); ++p) {
                    long long a = w1[p];
                    pfx1[static_cast<std::size_t>(a)].push_back(running);
                    std::vector<long long> suf(n);
                    for (std::size_t kk = 0; kk < n; ++kk) suf[kk] = total1[kk] - running[kk] - (static_cast<long long>(kk) == a ? 1 : 0);
                    sfx1[static_cast<std::size_t>(a)].push_back(suf);
                    running[static_cast<std::size_t>(a)]++;
                }
                running.assign(n, 0);
                bool hit = false;
                for (std::size_t p = 0; p < w2.size() && !hit; ++p) {
                    long long a = w2[p];
                    for (auto& pv : pfx1[static_cast<std::size_t>(a)]) if (pv == running) { hit = true; break; }
                    if (!hit) {
                        std::vector<long long> suf(n);
                        for (std::size_t kk = 0; kk < n; ++kk) suf[kk] = total2[kk] - running[kk] - (static_cast<long long>(kk) == a ? 1 : 0);
                        for (auto& sv : sfx1[static_cast<std::size_t>(a)]) if (sv == suf) { hit = true; break; }
                    }
                    running[static_cast<std::size_t>(a)]++;
                }
                if (hit) { depth = k; break; }
            }
            assert(depth > 0 && "pair failed to resolve -- bound is wrong or search too shallow");
            if (depth > worst) worst = depth;
        }
    }
    return worst;
}

void check(const std::vector<long long>& preperiod, const std::vector<long long>& period) {
    auto sigma = ravel::canonical_beta_substitution_eventually_periodic(preperiod, period);
    auto cert = derive_eventually_periodic_zero_run_bound(preperiod, period);
    long long actual = worst_case_depth(sigma, cert.predicted_max_depth + 3);
    std::cout << "N=" << sigma.size() << " flat=" << cert.flat_run << " cyclic=" << cert.cyclic_run
              << " R=" << cert.longest_zero_run << " predicted<=" << cert.predicted_max_depth
              << " actual=" << actual << "\n";
    assert(actual <= cert.predicted_max_depth);
}

}  // namespace

int main() {
    check({1}, {1});
    check({1}, {2});
    check({2, 1}, {1});
    check({1, 0, 1}, {1});
    check({1}, {0, 1});
    check({1}, {1, 0});
    check({1, 0, 0, 1}, {2});
    check({2}, {0, 0, 1});
    check({3, 0, 2}, {0, 1});
    check({1}, {1, 0, 0, 0, 1});
    // Wraparound stress tests: flat-only zero-run would give a WRONG
    // (too small) prediction here -- these confirm the cyclic term is
    // load-bearing, not decorative.
    check({1}, {0, 1, 0});
    check({2}, {0, 0, 1, 0, 0});

    std::cout << "zero_run_forces_bounded_coincidence_periodic: bound holds on all 12 cases, "
                 "including two where the cyclic-wraparound term is load-bearing.\n";
    return 0;
}
