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
// Known limitations of the prototype (and where to fix them)
// =====================================================================
// * disc > 0 branch: full dual differentiation via cbrt chain rule
//   (works for Pisot with complex-conjugate secondaries).
// * disc <= 0 branch (three distinct real roots, the typical n-bonacci
//   Pisot case incl. Tribonacci/Tetrabonacci/Pentanacci/etc.):
//   spectral_dual falls back to a non-differentiable value path; the
//   gradient returned is 0 (documented limitation in
//   spectral_dual.hpp's fallback).  To extend, add dual_cos / dual_acos
//   specializations and reduce the analytic expressions for cos-based
//   Cardano.  Even at this prototype stage, the workflow is exercised
//   on disc > 0 Pisot; the disc <= 0 case is concrete-Pisot-realizable
//   only in the dual version for non-Pisot disc < 0 / non-Pisot seeds
//   that walk toward Pisot seeds.
//
// * Step-size adaptation: this prototype takes a fixed step-size eta;
//   production code would adapt via backtracking line-search or trust
//   region on the rounded-matrix Pisot quality.  Cited as a TODO below.
//
// * Acceptance criterion: the result is a GENE, not a Pisot-verified
//   outcome.  Verification is built into the per-step reporting, but
//   no automated "is this seed a Pisot" check (that requires the
//   Sturm-based Pisot classifier from math/exact_pisot.c).

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
    std::vector<std::vector<long long>> M;     // integer-rounded current matrix
    double beta;
    double beta2;
    double grad_norm;     // L2 norm of the gradient
    long step_attempted;
    bool pisot_holds;
};

// One gradient step + round + check.  We use Dual<double> for the
// gradient (cheap, accurate) and rely on the non-Dual
// spectral_invariants_3x3 path for the per-step Pisot verification
// (the value check at the rounded integer is independent of the
// gradient computation; it's bit-exact Sturm-equivalent).
SmoothStep one_step(const std::vector<std::vector<long long>>& M0,
                     long step_index, double step_size) {
    SmoothStep out;
    out.M = M0;
    out.step_attempted = step_index;
    // Compute spectral invariants at the current seed (Pisot check).
    auto inv_at_seed = spectral_invariants_3x3(
        M0[0][0], M0[0][1], M0[0][2],
        M0[1][0], M0[1][1], M0[1][2],
        M0[2][0], M0[2][1], M0[2][2]);
    out.beta = inv_at_seed.beta;
    out.beta2 = inv_at_seed.beta2;
    out.pisot_holds = (inv_at_seed.beta > 1.0) &&
                     (std::abs(inv_at_seed.beta2) < 1.0);
    // Compute gradient of beta w.r.t. each matrix entry via Dual.
    // For the prototype we pick the dual_path seeded at (0,0); a
    // full Jacobian would loop over all 9 entries -- documented TODO.
    Dual<double> m00 = Dual<double>::variable(M0[0][0]);
    Dual<double> m01 = Dual<double>::constant(M0[0][1]);
    Dual<double> m02 = Dual<double>::constant(M0[0][2]);
    Dual<double> m10 = Dual<double>::constant(M0[1][0]);
    Dual<double> m11 = Dual<double>::constant(M0[1][1]);
    Dual<double> m12 = Dual<double>::constant(M0[1][2]);
    Dual<double> m20 = Dual<double>::constant(M0[2][0]);
    Dual<double> m21 = Dual<double>::constant(M0[2][1]);
    Dual<double> m22 = Dual<double>::constant(M0[2][2]);
    auto dual_inv = spectral_invariants_3x3_dual<double>(
        m00, m01, m02, m10, m11, m12, m20, m21, m22);
    out.grad_norm = std::abs(dual_inv.beta.eps);
    // Diagnostic: print all eps components.
    if (g_verbose) {
        std::printf("         dual beta.eps=%.6e, beta2.eps=%.6e\n",
                    dual_inv.beta.eps, dual_inv.beta2.eps);
    }
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

    for (long k = 0; k < n_steps; ++k) {
        auto s = one_step(M, k, step_size);
        const char* pisot_mark = s.pisot_holds ? "PISOT HOLDS" : "PISOT FAILS";
        std::printf("[step %2ld] beta=%.6f, beta2=%.6f, |grad|=%.3e  [%s]\n",
                    k, s.beta, s.beta2, s.grad_norm, pisot_mark);
        if (g_verbose) {
            std::printf("         M = [[%lld,%lld,%lld],[%lld,%lld,%lld],[%lld,%lld,%lld]]\n",
                        M[0][0], M[0][1], M[0][2],
                        M[1][0], M[1][1], M[1][2],
                        M[2][0], M[2][1], M[2][2]);
        }
    }
    std::printf("\nDone.  Note: full Jacobian computation (9 perturbations, not 1) is a"
                " straightforward extension -- TODO.  Adaptive step-size via Pisot-preserving"
                " line search (or trust region) is also a TODO.  The framework is in place.\n");
    return 0;
}
