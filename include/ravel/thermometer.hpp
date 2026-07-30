// thermometer.hpp
//
// Pisot substitution "thermometer": the maximum over the four
// CHSH-style observables of the empirical correlation function
// at spacelike lag L.  See `python/substitution_thermometer.py`
// and NOTES_PISOT.md.
//
// Provenance note: this file (and `tilt.hpp` / the `*_chsh.py`
// scripts) borrows the algebraic form of the CHSH inequality
// (the four-term sign-pattern maximization) from the Bell-
// inequality literature — the source is Hall & Branciard, Phys.
// Rev. A 102, 052228 (2020), "Simulating Bell nonlocality with
// local resources", as noted in `docs/RESEARCH_STATUS.md`.
// This is a correlation diagnostic for substitution sequences, NOT
// a claim about quantum mechanics occurring in the substitution.
// The Pisot conjecture doesn't invoke quantum mechanics; the
// CHSH-shaped observable is reused as a thermometer because the
// four-term pattern maximization is a natural scalar for
// "how correlated are the empirical lag-pair samples at distance
// L" — see NOTES_PISOT.md.  The naming is retained for
// bibliographic consistency with the python/ prototype.
//
// Theta(N) := max over 32 < L <= N of S_max(L) where S_max(L) =
//   max over the four sign patterns (g0, g1, g2, g3) in
//   {(1, 1, 1, -1), (1, 1, -1, 1), (1, -1, 1, 1), (-1, 1, 1, 1),
//    (-1, -1, -1, 1), (-1, -1, 1, -1), (-1, 1, -1, -1), (1, -1, -1, -1)}
//   of g0*E[(0,0)](L) + g1*E[(0,1)](L) + g2*E[(1,0)](L) + g3*E[(1,1)](L)
// and E[(a,b)](L) = <indicator(letter_t == k_a) * indicator(letter_{t+L}
//   == k_b)> with the choice (k_0, k_1) = (0, 1) of indicator over
// the {letter == 0} event -- the convention in the Python code.
//
// PROVENANCE NOTE (do not mistake this for a quantum-mechanics claim):
// the CHSH-named four-term sign-pattern maximization that defines
// `thermometer()` is BORROWED from the CHSH / Bell-inequality
// literature as a CONVENIENT CORRELATION STATISTIC on substitution
// sequences.  Nothing in this header makes any claim that the
// Pisot substitutions exhibit quantum non-locality, Bell-inequality
// violations, or any quantum-mechanical property; the function
// only computes a signed correlation sum over the realised word.
// The CHSH naming is a historical accident from a literature-review
// pass on Hall & Branciard (PRA 102, 052228, 2020), recorded in
// `docs/RESEARCH_STATUS.md`.  See also `tilt.hpp`
// for the same provenance caveat on the Hall-Branciard /
// Tsirelson-named helpers.  The companion test
// `test_thermometer.py::test_tilt_tribonacci_silver_mean_id` and
// the "0.046274 bits" cited in the file comment are LITERATURE
// values from Hall & Branciard (2020), not Spectre discoveries.
//
// We provide a direct O(N * lmax) correlation accumulator; a
// production implementation should swap in an FFT (see
// `python/substitution_thermometer.py::corr_all`).  Direct
// accumulation is simpler and accurate to N ~= 2^16 on commodity
// hardware (a few hundred ms in release build).

#pragma once

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <limits>
#include <utility>
#include <vector>

#include "ravel/substitution.hpp"

namespace ravel {

struct ThermometerResult {
    double theta;                 // maximum over the eight CHSH signs at any lag
    std::vector<std::pair<std::size_t, double>>
        records;                  // (lag, 2 - smax_at_lag) of each strict improvement
};

// Compute the four E[(a,b)](L) for a, b in {0, 1}.  A0(i) is +1
// iff the i-th letter of the (truncated) word equals 0, else -1.
// A1(i) is +1 iff the (i+1)-th letter of the word equals 0 -- the
// radius-1 XOR-convolved indicator in the Python code.
struct ThermObservables {
    std::vector<double> A0;
    std::vector<double> A1;
    std::size_t n;
};

inline ThermObservables therm_observables(const std::vector<std::int8_t>& word) {
    ThermObservables obs;
    const std::size_t n = word.size();
    obs.A0.assign(n, 0.0);
    obs.A1.assign(n, 0.0);
    for (std::size_t i = 0; i < n; ++i) obs.A0[i] = (word[i] == 0) ? 1.0 : -1.0;
    if (n >= 2) {
        for (std::size_t i = 0; i + 1 < n; ++i) obs.A1[i] = (word[i + 1] == 0) ? 1.0 : -1.0;
        // python convention: A1[i] = XOR(letter_i == 0, letter_{i+1} == 0)
        for (std::size_t i = 0; i + 1 < n; ++i) {
            double p = (word[i]     == 0) ? 1.0 : -1.0;
            double q = (word[i + 1] == 0) ? 1.0 : -1.0;
            obs.A1[i] = p * q;
        }
    }
    obs.n = n;
    return obs;
}

// Eight CHSH sign patterns from python/substitution_thermometer.py.
inline std::vector<std::array<int, 4>> chsh_signs() {
    return {{
        { 1,  1,  1, -1},
        { 1,  1, -1,  1},
        { 1, -1,  1,  1},
        {-1,  1,  1,  1},
        {-1, -1, -1,  1},
        {-1, -1,  1, -1},
        {-1,  1, -1, -1},
        { 1, -1, -1, -1},
    }};
}

inline ThermometerResult thermometer(const SubstitutionRule& rule,
                                     std::int8_t seed_letter = 0,
                                     std::size_t n_seq = 1u << 16,
                                     std::size_t lmax = 1u << 13,
                                     std::size_t lmin = 33) {
    auto word = rule.expand(seed_letter, n_seq);
    auto obs  = therm_observables(word);
    if (obs.n < 2) return { -std::numeric_limits<double>::infinity(), {} };

    auto cor = [&](const std::vector<double>& u,
                   const std::vector<double>& v) -> std::vector<double> {
        std::vector<double> E(lmax + 1, 0.0);
        for (std::size_t l = 0; l <= lmax && l < obs.n; ++l) {
            double s = 0.0;
            const std::size_t N = obs.n - l;
            for (std::size_t i = 0; i < N; ++i) s += u[i] * v[i + l];
            E[l] = s / static_cast<double>(N);
        }
        return E;
    };

    std::vector<double> E00 = cor(obs.A0, obs.A0);
    std::vector<double> E01 = cor(obs.A0, obs.A1);
    std::vector<double> E10 = cor(obs.A1, obs.A0);
    std::vector<double> E11 = cor(obs.A1, obs.A1);

    auto signs = chsh_signs();
    std::vector<double> smax(lmax + 1, -std::numeric_limits<double>::infinity());
    for (std::size_t l = 0; l <= lmax; ++l) {
        for (auto& g : signs) {
            double v =
                g[0] * E00[l] + g[1] * E01[l] + g[2] * E10[l] + g[3] * E11[l];
            if (v > smax[l]) smax[l] = v;
        }
    }
    for (std::size_t l = 0; l < lmin; ++l) smax[l] = -std::numeric_limits<double>::infinity();

    double theta = -std::numeric_limits<double>::infinity();
    std::vector<std::pair<std::size_t, double>> records;
    for (std::size_t l = lmin; l <= lmax; ++l) {
        if (smax[l] > theta + 1e-12) {
            theta = smax[l];
            records.emplace_back(l, 2.0 - smax[l]);
        }
    }
    return { theta, std::move(records) };
}

}  // namespace ravel
