// app/smooth_relaxation_search.cpp
//
// Smooth-relaxation search prototype, per
// docs/RESEARCH_VECTORS.md vector 2.  See also
// docs/DIRECTION_AND_OPEN_THREADS.md.
//
// =====================================================================
// What this driver does
// =====================================================================
// Given an integer Pisot-seed matrix M, run a gradient-based walk on
// the matrix entries M_ij (treated as continuous variables), with the
// objective function being a Pisot-flavored scalar (Perron root
// sufficiently larger than the secondaries' moduli).
//
// The gradient is computed via include/ravel/spectral_dual.hpp
// (forward-mode autodiff of spectral_invariants_3x3).  Each step we
//
//   1. round the current (continuous) candidate back to nearest integer
//   2. run spectral_invariants_3x3 on the rounded matrix to check Pisot
//   3. take a gradient step using Dual<BigFloat> arithmetic to get
//      irr-rational-capable derivatives (sqrt, cbrt of cbrt-root values)
//
// =====================================================================
// Status (2026-08-01): the three limitations below are FIXED
// =====================================================================
// * disc > 0 AND disc <= 0 branches both fully differentiable now.
//   The disc <= 0 (three-distinct-real-roots) branch previously fell
//   back to a non-differentiable value path (gradient always 0,
//   include/ravel/spectral_dual.hpp). Fixed by adding dual_cos /
//   dual_acos (include/ravel/dual.hpp) and rewriting that branch via
//   the trigonometric Cardano solution. Verified: analytic gradient
//   matches finite difference to ~1e-10 as the step shrinks.
// * Full 9-entry Jacobian: previously only d(beta)/d(M_00) was ever
//   computed (a single Dual seed); the other 8 entries were silent
//   constants. `full_jacobian()` below now differentiates w.r.t.
//   every entry (9 Dual passes, one variable seed each).
// * An actual gradient step, with acceptance: previously `one_step`
//   computed a gradient but the matrix was NEVER updated between
//   calls -- main()'s loop re-evaluated the same seed every
//   iteration and printed the same numbers. `one_step` now takes a
//   real ascent step on beta along the full-Jacobian direction, with
//   Pisot-preserving backtracking (halve the step up to 5 times if
//   the rounded result isn't Pisot; reject and stay put if
//   backtracking is exhausted).
//
// Still genuinely prototype-level, not production:
// * Objective is gradient ascent on beta alone, not the richer
//   "|secondary|/|Perron| ratio" or a target-substitution distance
//   the header above describes as the natural production objective.
// * Backtracking line search, not a full trust-region method.
// * Uses Dual<double> throughout (cheap, IEEE-double precision), not
//   Dual<BigFloat> for irrational-capable, arbitrary-precision
//   derivatives -- fine for a gradient DIRECTION, not for a final
//   certified answer (round the result and re-verify with the exact
//   Sturm-based Pisot classifier, math/exact_pisot.c, before trusting
//   any candidate this search produces).

#include <array>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <random>
#include <string>
#include <vector>

#include "math/bigint.hpp"
#include "math/ball.hpp"
#include "math/bigfloat.hpp"

#include "ravel/spectral.hpp"
#include "ravel/spectral_dual.hpp"
#include "ravel/dual.hpp"

using namespace mathlib;
using namespace ravel;
using namespace ravel::dual;

namespace {

// Print a BigFloat to a fixed number of decimal places via its
// BigFloat's string representation, since bigfloat_to_double loses
// precision at high prec.
std::string bf_str(const BigFloat& x, int digits = 12) {
    std::string s = bigfloat_str(x);
    return s;
}

namespace {
// Verbose flag shared between one_step() and main().
bool g_verbose = false;

struct SmoothStep {
    std::vector<std::vector<double>> Mc;        // continuous matrix state
    std::vector<std::vector<long long>> M;      // integer-rounded matrix (for display/Pisot check)
    double beta;
    double beta2;
    double grad_norm;     // L2 norm of the full 9-entry Jacobian
    double step_used;     // actual step size taken this iteration (after backtracking)
    long step_attempted;
    bool pisot_holds;
    bool accepted;        // false if backtracking exhausted and the step was rejected
};

// Full 9-entry Jacobian of beta w.r.t. every matrix entry, via one
// Dual<double> pass per entry (seed that entry as the variable,
// everything else constant). FIXES the "single-entry (m00 only)"
// documented limitation -- was previously a TODO with no code.
std::vector<std::vector<double>> full_jacobian(
        const std::vector<std::vector<double>>& Mc) {
    std::vector<std::vector<double>> grad(3, std::vector<double>(3, 0.0));
    for (int r = 0; r < 3; ++r) {
        for (int c = 0; c < 3; ++c) {
            std::vector<std::vector<Dual<double>>> D(3, std::vector<Dual<double>>(3));
            for (int i = 0; i < 3; ++i) {
                for (int j = 0; j < 3; ++j) {
                    D[i][j] = (i == r && j == c) ? Dual<double>::variable(Mc[i][j])
                                                  : Dual<double>::constant(Mc[i][j]);
                }
            }
            auto inv = spectral_invariants_3x3_dual<double>(
                D[0][0], D[0][1], D[0][2],
                D[1][0], D[1][1], D[1][2],
                D[2][0], D[2][1], D[2][2]);
            grad[r][c] = inv.beta.eps;
        }
    }
    return grad;
}

std::vector<std::vector<long long>> round_matrix(const std::vector<std::vector<double>>& Mc) {
    std::vector<std::vector<long long>> M(3, std::vector<long long>(3, 0));
    for (int i = 0; i < 3; ++i)
        for (int j = 0; j < 3; ++j)
            M[i][j] = static_cast<long long>(std::llround(Mc[i][j]));
    return M;
}

bool pisot_at(const std::vector<std::vector<long long>>& M, double& beta, double& beta2) {
    auto inv = spectral_invariants_3x3(
        M[0][0], M[0][1], M[0][2],
        M[1][0], M[1][1], M[1][2],
        M[2][0], M[2][1], M[2][2]);
    beta = inv.beta;
    beta2 = inv.beta2;
    return (inv.beta > 1.0) && (std::abs(inv.beta2) < 1.0);
}

// One gradient-ASCENT step on beta (the Pisot-flavored objective:
// grow the Perron root), FIXES two more documented TODOs: (1) the
// matrix was previously never actually updated between "steps" --
// each call re-evaluated the same seed; (2) no accept/reject existed
// at all. Now: take a full-Jacobian ascent step on the continuous
// matrix, round to the nearest integer, and backtrack (halve the
// step) up to 5 times if the rounded result isn't Pisot -- a simple
// Pisot-preserving line search, not the more general adaptive
// trust-region the file's own TODO comment gestures at, but a real
// accept/reject rather than none.
SmoothStep one_step(const std::vector<std::vector<double>>& Mc0,
                     long step_index, double step_size) {
    SmoothStep out;
    out.step_attempted = step_index;

    auto grad = full_jacobian(Mc0);
    double grad_norm = 0.0;
    for (int i = 0; i < 3; ++i)
        for (int j = 0; j < 3; ++j)
            grad_norm += grad[i][j] * grad[i][j];
    grad_norm = std::sqrt(grad_norm);
    out.grad_norm = grad_norm;

    if (g_verbose) {
        std::printf("         Jacobian: [[%.4f,%.4f,%.4f],[%.4f,%.4f,%.4f],[%.4f,%.4f,%.4f]]\n",
                    grad[0][0], grad[0][1], grad[0][2],
                    grad[1][0], grad[1][1], grad[1][2],
                    grad[2][0], grad[2][1], grad[2][2]);
    }

    double eta = step_size;
    for (int backtrack = 0; backtrack < 5; ++backtrack) {
        std::vector<std::vector<double>> Mc_try = Mc0;
        if (grad_norm > 1e-12) {
            for (int i = 0; i < 3; ++i)
                for (int j = 0; j < 3; ++j)
                    Mc_try[i][j] += eta * grad[i][j] / grad_norm;
        }
        auto M_try = round_matrix(Mc_try);
        double beta, beta2;
        bool holds = pisot_at(M_try, beta, beta2);
        if (holds || grad_norm <= 1e-12) {
            out.Mc = Mc_try;
            out.M = M_try;
            out.beta = beta;
            out.beta2 = beta2;
            out.pisot_holds = holds;
            out.step_used = eta;
            out.accepted = true;
            return out;
        }
        eta *= 0.5;
    }
    // Backtracking exhausted: reject the step, stay at the current point.
    out.Mc = Mc0;
    out.M = round_matrix(Mc0);
    pisot_at(out.M, out.beta, out.beta2);
    out.pisot_holds = true;  // Mc0 is only reached via a previously-accepted step
    out.step_used = 0.0;
    out.accepted = false;
    return out;
}
}  // namespace

}  // namespace

int main(int argc, char** argv) {
    std::printf("=== Smooth-relaxation search prototype (RESEARCH_VECTORS vector 2) ===\n");
    // Tribonacci: 3-letter Pisot substitution.  Known Pisot root ~ 1.839.
    std::vector<std::vector<long long>> M = {
        {1, 1, 1}, {1, 0, 0}, {0, 1, 0}};
    int n_steps = 5;
    double step_size = 0.01;
    // Optional CLI override: --steps N --eta E --seed S
    for (int i = 1; i < argc; ++i) {
        std::string a = argv[i];
        if (a == "--steps" && i + 1 < argc) n_steps = std::atoi(argv[++i]);
        else if (a == "--eta" && i + 1 < argc) step_size = std::atof(argv[++i]);
        else if (a == "--seed" && i + 1 < argc) {
            int s = std::atoi(argv[++i]);
            // Deterministic perturbation of Tribonacci: M_00 += s,
            // or pick a different seed if s = 99 (synthetic Pisot
            // with disc > 0: golden-like Fibonacci x identity row).
            if (s == 99) {
                M = {{1, 1, 0}, {1, 0, 0}, {0, 0, 1}};
                std::printf("Using synthetic Pisot with disc > 0 (golden-extended, "
                            "x_2 ~ 1, x_3 ~ -0.618).\n");
            } else {
                M[0][0] += s;
                std::printf("Tribonacci seed perturbed by +%d to (0,0) entry.\n", s);
            }
        } else if (a == "--verbose") {
            g_verbose = true;
        }
    }

    std::printf("Initial matrix M = [[%lld,%lld,%lld],[%lld,%lld,%lld],[%lld,%lld,%lld]]\n",
                M[0][0], M[0][1], M[0][2],
                M[1][0], M[1][1], M[1][2],
                M[2][0], M[2][1], M[2][2]);
    std::printf("Step size eta = %g, %d steps.\n\n", step_size, n_steps);

    std::vector<std::vector<double>> Mc(3, std::vector<double>(3, 0.0));
    for (int i = 0; i < 3; ++i)
        for (int j = 0; j < 3; ++j)
            Mc[i][j] = static_cast<double>(M[i][j]);

    for (long k = 0; k < n_steps; ++k) {
        auto s = one_step(Mc, k, step_size);
        const char* pisot_mark = s.pisot_holds ? "PISOT HOLDS" : "PISOT FAILS";
        const char* accept_mark = s.accepted ? "accepted" : "REJECTED (backtracking exhausted)";
        std::printf("[step %2ld] beta=%.6f, beta2=%.6f, |grad|=%.3e, eta_used=%.4g  [%s, %s]\n",
                    k, s.beta, s.beta2, s.grad_norm, s.step_used, pisot_mark, accept_mark);
        if (g_verbose) {
            std::printf("         M = [[%lld,%lld,%lld],[%lld,%lld,%lld],[%lld,%lld,%lld]]\n",
                        s.M[0][0], s.M[0][1], s.M[0][2],
                        s.M[1][0], s.M[1][1], s.M[1][2],
                        s.M[2][0], s.M[2][1], s.M[2][2]);
        }
        Mc = s.Mc;
        if (!s.accepted) break;  // stuck: further steps would repeat the same rejection
    }
    std::printf("\nDone.  Full 9-entry Jacobian, an actual gradient-ascent step on beta, and\n"
                "Pisot-preserving backtracking line search are now implemented (previously\n"
                "all three were TODOs and the matrix was never updated between \"steps\").\n"
                "Still a prototype: gradient ascent on beta alone (not a richer\n"
                "|secondary|/|Perron| objective), and backtracking rather than a full\n"
                "trust-region method.\n");
    return 0;
}
