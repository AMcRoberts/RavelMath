// ravel/proof/reverse_engineer_minimal_polynomial_from_returns.hpp
//
// A methodological tool proposed directly by AM (2026-08-06): rather
// than deriving a substitution's algebraic structure (incidence matrix,
// characteristic/minimal polynomial) by building the matrix and running
// linear algebra, RECOVER it purely from combinatorial "which walks
// finish" data -- count how many Dumont-Thomas walks of each length,
// starting from a fixed reference letter, land EXACTLY back on that
// letter with zero depth remaining (a "clean return"), then fit the
// minimal integer linear recurrence this counting sequence satisfies.
//
// VERIFIED (2026-08-06) against two substitutions with genuinely
// different minimal polynomials, using only the counting sequence, no
// matrix ever built:
//   - sigma_{0,1} (sigma(0)=1,2 sigma(1)=2 sigma(2)=0): recovered
//     g(K) = g(K-2) + g(K-3) exactly, i.e. characteristic polynomial
//     x^3 - x - 1, EXACTLY the plastic number's own minimal polynomial.
//   - x^3-2x^2-x+1's substitution (sigma(0)=0,0,1 sigma(1)=2
//     sigma(2)=0,1): recovered g(K) = 2 g(K-1) + g(K-2) - g(K-3)
//     exactly, i.e. characteristic polynomial x^3 - 2x^2 - x + 1,
//     EXACTLY that substitution's own known minimal polynomial.
//
// WHY THIS WORKS (not just an empirical curiosity): the number of
// letters at "distance K" from a reference point grows at the same
// rate as the substitution's own word-length growth -- the dominant
// Perron eigenvalue beta. A "clean return" count is a renewal-type
// sequence counting closed walks of length K in the substitution's own
// transition structure back to a fixed state; such counts satisfy
// EXACTLY the linear recurrence whose characteristic polynomial is the
// (minimal polynomial of the) transition structure's dominant
// eigenvalue, by the standard transfer-matrix argument (closed-walk
// counts in a graph are governed by powers of its adjacency matrix,
// and the recurrence a graph's own adjacency matrix satisfies is
// exactly its characteristic polynomial, via Cayley-Hamilton). This is
// not a coincidence discovered twice -- it is a general fact, now
// confirmed on two independent, structurally different examples.
//
// CONSEQUENCE for the walk-realizability program (Findings 22, 23):
// this gives a way to recover the exact algebraic relation Cayley-
// Hamilton would supply (the input to Finding 22's landmark-vector-
// cancellation reduction) directly from simulation, without first
// building the incidence matrix by hand -- useful when the junction
// structure is complex enough (multiple junctions, large alphabets)
// that hand-deriving the matrix is itself error-prone, exactly the
// kind of derivation this project's own discipline distrusts until
// checked. It does not, by itself, resolve the harder
// walk-REALIZABILITY question (Finding 23 still needed the explicit
// jump-size graph for that) -- it is a complementary tool for the
// algebraic layer, not a replacement for the combinatorial one.

#pragma once

#include <array>
#include <cmath>
#include <cstddef>
#include <vector>

namespace ravel::proof {

// Counts, for each depth 0..max_depth, the number of Dumont-Thomas
// walks of that total depth starting at `reference_letter` that land
// EXACTLY back on `reference_letter` with zero remaining depth (a
// "clean return"). O(alphabet_size * max_depth) via direct DP -- no
// matrix, no prior structural knowledge of the substitution required.
template <std::size_t d>
inline std::vector<long long> count_clean_returns(
    const std::array<std::vector<long long>, d>& images,
    long long reference_letter,
    int max_depth) {
    // memo[letter][depth] = number of ways to reach reference_letter
    // after exactly `depth` more substitution steps, starting from `letter`.
    std::vector<std::vector<long long>> memo(d, std::vector<long long>(static_cast<std::size_t>(max_depth) + 1, -1));
    // iterative (avoid recursion depth issues): fill depth=0 row, then build up.
    for (std::size_t letter = 0; letter < d; ++letter)
        memo[letter][0] = (static_cast<long long>(letter) == reference_letter) ? 1 : 0;
    for (int depth = 1; depth <= max_depth; ++depth) {
        for (std::size_t letter = 0; letter < d; ++letter) {
            long long total = 0;
            for (auto child : images[letter]) total += memo[static_cast<std::size_t>(child)][static_cast<std::size_t>(depth - 1)];
            memo[letter][static_cast<std::size_t>(depth)] = total;
        }
    }
    std::vector<long long> g;
    for (int depth = 0; depth <= max_depth; ++depth) g.push_back(memo[static_cast<std::size_t>(reference_letter)][static_cast<std::size_t>(depth)]);
    return g;
}

struct RecurrenceFitResult {
    bool found{};
    int order{};
    std::vector<long long> coefficients;  // g(K) = sum_i coefficients[i-1] * g(K-i)
};

// Fits the minimal integer linear recurrence the sequence `g` satisfies,
// trying orders 1..max_order. Solves via Gaussian elimination (double
// precision) on one window of equations, then re-verifies the fitted
// integer coefficients EXACTLY against the ENTIRE sequence before
// accepting -- a numerically-solved candidate is never trusted without
// this exact re-check (the same discipline this project applies
// everywhere else: a plausible-looking numeric answer is not evidence
// until independently confirmed).
inline RecurrenceFitResult fit_minimal_linear_recurrence(const std::vector<long long>& g, int max_order = 8) {
    RecurrenceFitResult out;
    const int N = static_cast<int>(g.size()) - 1;
    for (int order = 1; order <= max_order; ++order) {
        if (N < 2 * order) break;
        std::vector<std::vector<double>> A(static_cast<std::size_t>(order), std::vector<double>(static_cast<std::size_t>(order) + 1));
        for (int eq = 0; eq < order; ++eq) {
            int K = order + eq;
            for (int i = 1; i <= order; ++i) A[static_cast<std::size_t>(eq)][static_cast<std::size_t>(i - 1)] = static_cast<double>(g[static_cast<std::size_t>(K - i)]);
            A[static_cast<std::size_t>(eq)][static_cast<std::size_t>(order)] = static_cast<double>(g[static_cast<std::size_t>(K)]);
        }
        bool singular = false;
        for (int col = 0; col < order; ++col) {
            int piv = -1;
            double best = 1e-9;
            for (int r = col; r < order; ++r) {
                if (std::fabs(A[static_cast<std::size_t>(r)][static_cast<std::size_t>(col)]) > best) { best = std::fabs(A[static_cast<std::size_t>(r)][static_cast<std::size_t>(col)]); piv = r; }
            }
            if (piv < 0) { singular = true; break; }
            std::swap(A[static_cast<std::size_t>(col)], A[static_cast<std::size_t>(piv)]);
            for (int r = 0; r < order; ++r) {
                if (r == col) continue;
                double f = A[static_cast<std::size_t>(r)][static_cast<std::size_t>(col)] / A[static_cast<std::size_t>(col)][static_cast<std::size_t>(col)];
                for (int c = col; c <= order; ++c) A[static_cast<std::size_t>(r)][static_cast<std::size_t>(c)] -= f * A[static_cast<std::size_t>(col)][static_cast<std::size_t>(c)];
            }
        }
        if (singular) continue;
        std::vector<long long> coeffs;
        bool all_integer = true;
        for (int i = 0; i < order; ++i) {
            double val = A[static_cast<std::size_t>(i)][static_cast<std::size_t>(order)] / A[static_cast<std::size_t>(i)][static_cast<std::size_t>(i)];
            long long rounded = static_cast<long long>(std::llround(val));
            if (std::fabs(val - static_cast<double>(rounded)) > 1e-6) { all_integer = false; break; }
            coeffs.push_back(rounded);
        }
        if (!all_integer) continue;
        bool verified = true;
        for (int K = order; K <= N; ++K) {
            long long pred = 0;
            for (int i = 1; i <= order; ++i) pred += coeffs[static_cast<std::size_t>(i - 1)] * g[static_cast<std::size_t>(K - i)];
            if (pred != g[static_cast<std::size_t>(K)]) { verified = false; break; }
        }
        if (!verified) continue;
        out.found = true;
        out.order = order;
        out.coefficients = coeffs;
        return out;
    }
    return out;
}

}  // namespace ravel::proof
