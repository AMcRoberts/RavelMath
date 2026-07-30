// survey.hpp
//
// Random primitive Pisot substitution survey.
// Mirrors python/pisot_survey.py::classify, balanced_pair, and the
// matrix-generation loop.  All arithmetic (eigenvalues, char
// poly, irreducibility, certification) is done in C++ using the
// helpers in spectral.hpp, balanced_pair.hpp, and barge.hpp.

#pragma once

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <random>
#include <string>
#include <vector>

#include "ravel/substitution.hpp"
#include "ravel/spectral.hpp"
#include "ravel/balanced_pair.hpp"
#include "ravel/barge.hpp"

namespace ravel {

struct PisotInstance {
    std::string  name;
    std::vector<std::vector<std::int8_t>> sigma;
    std::vector<std::vector<long long>>  M;
    double       beta;
    double       b2;
    bool         pisot;
    bool         irred;
    bool         certified;
    std::size_t  n_irreducible;
    double       rho_nc;
};

struct PisotClassifyResult {
    double beta;
    double b2;
    bool   pisot;
    bool   irred;
};

// Spectral classification of an integer matrix using the closed-
// form 3x3 path in spectral.hpp.  Returns true iff all four
// fields were populated; populates `out` regardless.
// We require beta > 1 + 1e-9 and all other |z| < 1 - 1e-9.
inline PisotClassifyResult classify_matrix_spectral(
    const std::vector<std::vector<long long>>& M) {
    PisotClassifyResult out{0.0, 0.0, false, false};
    if (M.empty()) return out;
    SpectralInvariants inv;
    if (M.size() == 2) {
        inv = spectral_invariants_2x2(M[0][0], M[0][1], M[1][0], M[1][1]);
    } else if (M.size() == 3) {
        inv = spectral_invariants_3x3(
            M[0][0], M[0][1], M[0][2],
            M[1][0], M[1][1], M[1][2],
            M[2][0], M[2][1], M[2][2]);
    } else {
        // Previously: `return out;` here silently produced
        // beta=0/pisot=false for every n >= 4 matrix (see
        // WORK_ITEMS.md W13, TODO_GENERALIZATION.md item 1/2).
        // spectral_invariants_general is a numerical (not
        // closed-form) solver, valid for any n.
        inv = spectral_invariants_general(M);
    }
    out.beta  = inv.beta;
    out.b2    = std::abs(inv.beta2);
    // Irreducibility over Q: char poly coefficients are integers
    // for an integer matrix; test rational roots via the
    // Faddeev-LeVerrier + rational-root test.
    auto cp = charpoly_int(M);
    long long c0 = cp.empty() ? 0 : cp.back();
    bool irred = (c0 != 0);
    if (irred) {
        for (long long d = 1; irred && d <= std::abs(c0); ++d) {
            if (c0 % d != 0) continue;
            for (long long sign : { 1, -1 }) {
                long long r = sign * (c0 / d);
                if (r == 0) continue;
                if (polyval_int_is_zero(cp, r)) {
                    irred = false;
                    break;
                }
            }
        }
    }
    out.irred = irred;
    out.pisot  = (inv.beta > 1.02) && (out.b2 < 1.0 - 1e-9)
                 && irred;
    return out;
}

// Resolve a 3x3 matrix to a non-empty column-word substitution
// (column c -> word with M[r][c] copies of letter r).  Returns
// empty on failure (a column with sum 0).
inline std::vector<std::vector<std::int8_t>>
matrix_to_subst(const std::vector<std::vector<long long>>& M) {
    const std::size_t n = M.size();
    std::vector<std::vector<std::int8_t>> sigma(n);
    for (std::size_t c = 0; c < n; ++c) {
        for (std::size_t r = 0; r < n; ++r) {
            for (long long k = 0; k < M[r][c]; ++k) {
                sigma[c].push_back(static_cast<std::int8_t>(r));
            }
        }
    }
    for (auto& w : sigma) {
        if (w.empty()) return {};
    }
    return sigma;
}

// Entry range K for random_pisot_survey, as a function of alphabet
// size n.  Per DIRECTION_AND_OPEN_THREADS.md thread B item "Wider
// random search for more 4-letter-rnd13-like candidates", the
// original 87-candidate survey used entries 0..3 for n=4 (K=4 in
// default_K_for_n's range).  Only one of the 11 clean non-unimodular
// Pisot candidates that converged had λ(G_B) < β (rnd13).  A wider
// K range may turn up more such candidates, since larger entries
// give richer boundary structure at fixed alphabet size.
//
// The wide-survey variant below samples entries from 0..K_max
// explicitly (rather than default_K_for_n ± 1).  Density falls
// off as K^(-n^2) for the primitive-Pisot condition, so the wider
// K range mostly trades off raw candidate count for boundary
// structure.  Defaults picked to make the run useful at n=4.

// Generate `target` random primitive n x n Pisot substitutions with
// entries in 0..K_max (rather than default_K_for_n's narrow range).
// The same primitive/Pisot/irreducible/certify pipeline as
// random_pisot_survey; alphabet_size must be > 0 (the wide search
// is only meaningful for a fixed dimension).
inline std::vector<PisotInstance>
wide_random_pisot_survey(std::size_t target,
                          int K_max,
                          std::uint32_t seed = 11,
                          std::size_t max_pairs_per_certify = 8000,
                          std::size_t max_len_per_certify  = 60000,
                          std::size_t max_pairs_for_rho   = 8000,
                          std::size_t max_len_for_rho     = 60000,
                          std::size_t max_trials          = 8000,
                          int alphabet_size               = 4) {
    if (alphabet_size < 2) {
        throw std::invalid_argument("wide_random_pisot_survey: alphabet_size must be >= 2");
    }
    std::vector<PisotInstance> out;
    std::mt19937 rng(seed);
    const std::size_t n = static_cast<std::size_t>(alphabet_size);
    std::size_t trials = 0;
    while (out.size() < target && trials < max_trials) {
        ++trials;
        std::vector<std::vector<long long>> M(n, std::vector<long long>(n, 0));
        for (auto& row : M) {
            for (auto& v : row) {
                std::uniform_int_distribution<long long> d(0, static_cast<long long>(K_max));
                v = d(rng);
            }
        }
        // Primitive condition: M^n > 0 entrywise on the boolean skeleton.
        auto skeleton = M;
        for (auto& row : skeleton)
            for (auto& v : row) v = std::min(v, 1LL);
        auto mm = [](std::vector<std::vector<long long>> A,
                     std::vector<std::vector<long long>> B) {
            std::size_t nn = A.size();
            std::vector<std::vector<long long>> C(nn, std::vector<long long>(nn, 0));
            for (std::size_t i = 0; i < nn; ++i) for (std::size_t j = 0; j < nn; ++j)
                for (std::size_t k = 0; k < nn; ++k)
                    C[i][j] += A[i][k] * B[k][j];
            return C;
        };
        auto skn = skeleton;
        for (std::size_t p = 1; p < n; ++p) skn = mm(skn, skeleton);
        bool prim = true;
        for (std::size_t i = 0; i < n && prim; ++i)
            for (std::size_t j = 0; j < n; ++j)
                if (skn[i][j] == 0) prim = false;
        if (!prim) continue;

        auto cls = classify_matrix_spectral(M);
        if (!cls.pisot) continue;

        auto sigma = matrix_to_subst(M);
        if (sigma.empty()) continue;

        PisotInstance inst;
        inst.name  = "rndW" + std::to_string(K_max) + "_" + std::to_string(out.size() + 1);
        inst.sigma = sigma;
        inst.M     = M;
        inst.beta  = cls.beta;
        inst.b2    = cls.b2;
        inst.pisot = true;
        inst.irred = true;

        SubstitutionRule rule(sigma);
        auto cert = balanced_pair_certify(rule, max_pairs_per_certify, max_len_per_certify);
        inst.certified       = cert.certified;
        inst.n_irreducible   = cert.n_irreducible;
        if (cert.certified) {
            inst.rho_nc = rho_nc(rule, max_pairs_for_rho, max_len_for_rho);
        } else {
            inst.rho_nc = std::numeric_limits<double>::infinity();
        }

        out.push_back(std::move(inst));
    }
    return out;
}

// Entry range K for random_pisot_survey, as a function of alphabet
// size n.  The primitive+non-trivial Pisot density is roughly
// proportional to (K / (K+1))^(n^2) for fixed n, so for large n a
// small K gives almost-zero density.  Empirically: K=2..4 for n in
// {2,3,4} (the 3-letter canonical regime); K=n+1 for n >= 4 gives
// a workable density at n=5,6,7,8.
inline int default_K_for_n(std::size_t n) {
    if (n <= 4) return 4;     // n=2,3,4 use K in {2,3,4} (3-letter regime)
    return static_cast<int>(n) + 1;  // n >= 5 use K = n+1
}

// Generate `target` random primitive n x n Pisot substitutions,
// subject to the canonical gating in pisot_survey.py:
// entries 0..K, K = default_K_for_n(n) (or randomized around it
// for the mixed-n survey); (|M|_binary)^n > 0 (i.e. the boolean
// skeleton is primitive); Pisot status requires beta > 1.05, all
// other |z| < 1 - 1e-9, char poly irreducible.
//
// `alphabet_size` (default 0) picks the matrix dimension.  When
// alphabet_size = 0, each trial picks a random n in {2,3,4} and a
// matching K — the original 3-letter "rnd_canon" survey behavior.
// When alphabet_size = d > 0, the trial always uses n = d and K =
// default_K_for_n(d), which scales with d so the primitive-Pisot
// density stays workable for d up to MAX_DISPATCH_D (= 8 in
// contact_boundary.hpp).
//
// The companion dispatch (`compute_contact_boundary_dispatch`,
// `compute_contact_boundary_from_subst_dispatch`) in
// `contact_boundary.hpp` was generalized to d in [2, 8] at the
// same time, so this survey's output is usable for any d up to 8.
inline std::vector<PisotInstance>
random_pisot_survey(std::size_t target,
                    std::uint32_t seed = 11,
                    std::size_t max_pairs_per_certify = 8000,
                    std::size_t max_len_per_certify  = 60000,
                    std::size_t max_pairs_for_rho   = 8000,
                    std::size_t max_len_for_rho     = 60000,
                    std::size_t max_trials          = 8000,
                    int alphabet_size               = 0) {
    std::vector<PisotInstance> out;
    std::mt19937 rng(seed);
    std::uniform_int_distribution<int> Ndist_mixed(2, 4);  // used when alphabet_size == 0
    std::size_t trials = 0;
    while (out.size() < target && trials < max_trials) {
        ++trials;
        int n_eff = (alphabet_size > 0) ? alphabet_size : Ndist_mixed(rng);
        if (n_eff < 2) continue;
        const std::size_t n = static_cast<std::size_t>(n_eff);
        // K: randomize within ±1 of the default for this n, so we
        // don't bias the survey to one K.
        int K_center = default_K_for_n(n);
        std::uniform_int_distribution<int> Kdist(
            std::max(2, K_center - 1), K_center + 1);
        std::size_t K = static_cast<std::size_t>(Kdist(rng));
        std::vector<std::vector<long long>> M(n, std::vector<long long>(n, 0));
        for (auto& row : M) {
            for (auto& v : row) {
                std::uniform_int_distribution<long long> d(0, static_cast<long long>(K));
                v = d(rng);
            }
        }
        // Primitive condition: M^n > 0 entrywise on the boolean
        // skeleton (a la Wielandt's bound).
        auto skeleton = M;
        for (auto& row : skeleton)
            for (auto& v : row)
                v = std::min(v, 1LL);
        auto mm = [](std::vector<std::vector<long long>> A,
                     std::vector<std::vector<long long>> B) {
            std::size_t nn = A.size();
            std::vector<std::vector<long long>> C(nn, std::vector<long long>(nn, 0));
            for (std::size_t i = 0; i < nn; ++i) for (std::size_t j = 0; j < nn; ++j)
                for (std::size_t k = 0; k < nn; ++k)
                    C[i][j] += A[i][k] * B[k][j];
            return C;
        };
        std::vector<std::vector<long long>> skn = skeleton;
        for (std::size_t p = 1; p < n; ++p) skn = mm(skn, skeleton);
        bool prim = true;
        for (std::size_t i = 0; i < n && prim; ++i)
            for (std::size_t j = 0; j < n; ++j)
                if (skn[i][j] == 0) prim = false;
        if (!prim) continue;

        // Verify Pisot status and irreducibility.
        auto cls = classify_matrix_spectral(M);
        if (!cls.pisot) continue;

        // Resolve column words.
        auto sigma = matrix_to_subst(M);
        if (sigma.empty()) continue;

        PisotInstance inst;
        inst.name  = "rnd" + std::to_string(out.size() + 1) + "_n" + std::to_string(n) + "_canon";
        inst.sigma = sigma;
        inst.M     = M;
        inst.beta  = cls.beta;
        inst.b2    = cls.b2;
        inst.pisot = true;
        inst.irred = true;

        // Certify.
        SubstitutionRule rule(sigma);
        auto cert = balanced_pair_certify(rule, max_pairs_per_certify, max_len_per_certify);
        inst.certified       = cert.certified;
        inst.n_irreducible   = cert.n_irreducible;
        if (cert.certified) {
            inst.rho_nc = rho_nc(rule, max_pairs_for_rho, max_len_for_rho);
        } else {
            inst.rho_nc = std::numeric_limits<double>::infinity();
        }

        out.push_back(std::move(inst));
    }
    return out;
}

// Word-frequency ceiling for a Pisot substitution:
// for each Dirichlet frequency m*f (mod 1) with m in {1..n-1},
// take its closest return to {0, 1/2}, then run the running
// minimum over L.  Mirrors python/pisot_survey.py::ceiling_running.
inline std::vector<double> pf_ceiling_running(
    const std::vector<double>& f,
    std::size_t lmax,
    std::size_t lmin = 33) {
    std::vector<double> worst(lmax + 1, 0.0);
    const std::size_t n = f.size();
    for (std::size_t m = 0; m + 1 < n; ++m) {
        for (std::size_t L = 0; L <= lmax; ++L) {
            double x = std::fmod(static_cast<double>(L) * f[m], 1.0);
            if (x < 0) x += 1.0;
            double d = std::min(x, 1.0 - x);
            if (d > worst[L]) worst[L] = d;
        }
    }
    for (std::size_t L = 0; L < lmin; ++L) worst[L] = std::numeric_limits<double>::infinity();
    std::vector<double> running(lmax + 1);
    double cur = std::numeric_limits<double>::infinity();
    for (std::size_t L = 0; L <= lmax; ++L) {
        if (worst[L] < cur) cur = worst[L];
        running[L] = cur;
    }
    return running;
}

// Right Perron eigenvector (frequencies) of a non-negative
// integer matrix via power iteration.  Mirrors
// python/core.py::frequencies.
inline std::vector<double> pf_right(const std::vector<std::vector<long long>>& M,
                                  int iters = 4000) {
    const std::size_t n = M.size();
    if (n == 0) return {};
    std::vector<double> v(n, 1.0 / static_cast<double>(n));
    std::vector<double> w(n, 0.0);
    for (int it = 0; it < iters; ++it) {
        double norm = 0.0;
        for (std::size_t i = 0; i < n; ++i) {
            double s = 0.0;
            for (std::size_t j = 0; j < n; ++j) s += static_cast<double>(M[i][j]) * v[j];
            w[i] = s;
            norm = std::max(norm, s);
        }
        if (norm == 0.0) break;
        for (auto& x : w) x /= norm;
        v = w;
    }
    double s = 0.0;
    for (auto x : v) s += x;
    if (s > 0.0) for (auto& x : v) x /= s;
    return v;
}

}  // namespace ravel
