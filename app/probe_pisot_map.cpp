// app/probe_pisot_map.cpp
//
// Builds a genuine, certified map of Pisot numbers in a bounded window,
// to ground AM's "harmonics/poles" question: where do Pisot numbers
// actually appear, where are the points of lowest complexity, and how
// does complexity branch.
//
// Grounding in the actual (classical, not rediscovered here) theory,
// confirmed via web search this session:
//   - S (the set of Pisot numbers) is closed (Salem 1944).
//   - The smallest Pisot number is the plastic constant ~1.32471796,
//     root of x^3-x-1 (Salem 1944 identified it, Siegel 1944 proved
//     minimality).
//   - Dufresnoy & Pisot (1955) completely classified EVERY Pisot
//     number below the golden ratio phi=(1+sqrt5)/2~1.61803 -- that
//     whole region is a finite, fully solved, discrete list.
//   - phi is the smallest LIMIT POINT of S (Dufresnoy-Pisot) -- the
//     transition from "discrete/solved" to "accumulating" happens
//     exactly there, not at some vaguer boundary.
//   - S has its own derived set S' (limit points of S), which has ITS
//     OWN derived set S''=derived(S'), etc. -- an actual nested
//     "harmonics" structure, studied by Dufresnoy-Pisot and continued
//     by Boyd ("Pisot numbers in the neighborhood of a limit point",
//     I & II, 1984-85). The minima of these successive derived sets
//     satisfy sqrt(k) <= min S^(k) <= 2^(k/2) -- a genuine quantitative
//     bound on how far up the number line each "level" of structure
//     starts.
//   - Finding 29 (this project, same session): every integer >= 2 is
//     a constructive accumulation point via the a-bonacci family.
//
// This probe does NOT try to reproduce Boyd's exact derived-set tables
// (paywalled, not independently reproducible here) -- it instead
// builds an honest, CERTIFIED (via pisot_classify_degree_n, Finding 30)
// empirical map: every Pisot number found in a bounded coefficient
// sweep across degrees 3-7, tagged with the LOWEST degree at which it
// was found (a real, if crude, complexity proxy -- lower degree with
// smaller coefficients is a more "accessible" Pisot number in exactly
// the sense AM means by "easy to locate").

#include <algorithm>
#include <cstdio>
#include <functional>
#include <map>
#include <set>
#include <stdexcept>
#include <vector>

#include "exact_pisot.h"

namespace {

struct Hit {
    double beta;
    int degree;
    std::vector<long long> coeffs;  // low-to-high, excluding leading 1
};

double mid(const pisot_info_t& info) {
    double lo = mpz_get_d(info.beta_lo_num) / mpz_get_d(info.beta_lo_den);
    double hi = mpz_get_d(info.beta_hi_num) / mpz_get_d(info.beta_hi_den);
    return (lo + hi) / 2.0;
}

void sweep_degree(int degree, long long bound, double lo_window, double hi_window,
                   std::vector<Hit>& out, long long state_cap) {
    std::vector<long long> coeffs(static_cast<std::size_t>(degree));
    // Enumerate coeffs[0..degree-1] each in [-bound, bound], skipping
    // the all-zero-but-last (already covered by lower degree) trivial
    // cases only implicitly -- no special-casing, just a plain
    // bounded grid search, capped.
    long long total = 0;
    std::function<void(int)> rec = [&](int idx) {
        if (idx == degree) {
            ++total;
            if (total > state_cap) throw std::runtime_error("sweep_degree: state cap exceeded");
            pisot_info_t info;
            int rc = pisot_classify_degree_n(coeffs.data(), degree, &info);
            if (rc && info.is_pisot) {
                double b = mid(info);
                if (b > lo_window && b < hi_window) out.push_back({b, degree, coeffs});
            }
            if (rc) pisot_info_clear(&info);
            return;
        }
        for (long long c = -bound; c <= bound; ++c) {
            coeffs[static_cast<std::size_t>(idx)] = c;
            rec(idx + 1);
        }
    };
    rec(0);
}

}  // namespace

int main() {
    std::vector<Hit> all;
    // Degree 3: bound 4 (5^3=... actually (2*4+1)^3=729, trivial).
    sweep_degree(3, 4, 1.0, 4.5, all, 2000000);
    // Degree 4: bound 3 ((7)^4=2401).
    sweep_degree(4, 3, 1.0, 4.5, all, 2000000);
    // Degree 5: bound 2 ((5)^5=3125).
    sweep_degree(5, 2, 1.0, 4.5, all, 2000000);
    // Degree 6: bound 2 ((5)^6=15625).
    sweep_degree(6, 2, 1.0, 4.5, all, 2000000);
    // Degree 7: bound 1 ((3)^7=2187) -- keep small, cost grows fast.
    sweep_degree(7, 1, 1.0, 4.5, all, 2000000);

    // For each DISTINCT beta value (dedupe by rounding), keep only the
    // lowest-degree hit (the "simplest" way to reach it) plus a count
    // of how many (degree,coeff) combinations reproduce it.
    std::map<long long, Hit> best;  // key = round(beta * 1e9)
    std::map<long long, int> count;
    for (auto& h : all) {
        long long key = static_cast<long long>(h.beta * 1e9 + 0.5);
        ++count[key];
        auto it = best.find(key);
        if (it == best.end() || h.degree < it->second.degree) best[key] = h;
    }

    std::printf("beta,degree,coeff_count_at_this_beta,coeffs\n");
    std::vector<std::pair<double, Hit>> rows;
    for (auto& [key, h] : best) rows.push_back({h.beta, h});
    std::sort(rows.begin(), rows.end(), [](auto& a, auto& b) { return a.first < b.first; });
    for (auto& [b, h] : rows) {
        std::printf("%.10f,%d,%d,\"", b, h.degree, count[static_cast<long long>(b * 1e9 + 0.5)]);
        for (std::size_t i = 0; i < h.coeffs.size(); ++i) std::printf("%s%lld", i ? " " : "", h.coeffs[i]);
        std::printf("\"\n");
    }
    std::fprintf(stderr, "total distinct Pisot values found: %zu\n", rows.size());
    return 0;
}
