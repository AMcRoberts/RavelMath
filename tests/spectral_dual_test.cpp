// tests/spectral_dual_test.cpp
//
// Tests for include/ravel/spectral_dual.hpp -- the differentiable
// 3x3 spectral invariants via forward-mode autodiff.
//
// Test ladder:
//   1. Sanity at a known Pisot: Tribonacci's M = [[1,1,1],[1,0,0],[0,1,0]]
//      gives beta ~= 1.839286.  Verify spectral_dual matches this.
//   2. Cross-check the gradient via finite difference at Tribonacci.
//   3. (Optional) Same at Tetrabonacci (size 4, not directly testable
//      but Tribonacci is enough to verify the chain rule propagation.)

#include <cmath>
#include <cstdio>
#include <vector>

#include "math/bigint.hpp"
#include "math/bigfloat.hpp"

#include "ravel/spectral.hpp"
#include "ravel/spectral_dual.hpp"
#include "ravel/dual.hpp"

using namespace mathlib;
using namespace ravel;
using namespace ravel::dual;

static int n_pass = 0, n_fail = 0;

#define EXPECT(cond, msg) do { \
    if (cond) { ++n_pass; } \
    else { ++n_fail; std::fprintf(stderr, "FAIL: %s (line %d)\n", msg, __LINE__); } \
} while (0)

int main() {
    std::printf("== spectral_dual: Tribonacci (Pisot, disc < 0, fallback branch) ==\n");
    {
        // Tribonacci: M = [[1,1,1],[1,0,0],[0,1,0]].  Beta ~= 1.8393.
        // Note: Tribonacci's char poly is x^3 - x^2 - x - 1, with
        // discriminant < 0 (all three roots real).  spectral_dual
        // handles this via the documented fallback (non-differentiable
        // eps=0 on this branch); the value branch is bit-exact via
        // spectral_invariants_3x3's three-distinct-real-roots path.
        std::vector<std::vector<long long>> M = {
            {1, 1, 1}, {1, 0, 0}, {0, 1, 0}};
        // Sanity first: spectral_invariants_3x3 (non-differentiable)
        // returns the right beta.
        auto ref = ravel::spectral_invariants_3x3(
            M[0][0], M[0][1], M[0][2],
            M[1][0], M[1][1], M[1][2],
            M[2][0], M[2][1], M[2][2]);
        EXPECT(std::abs(ref.beta - 1.839286755214160) < 1e-9,
               "Tribonacci spectral_invariants_3x3 agrees with known beta");
        // Now build Dual<double> entries.
        Dual<double> m00 = Dual<double>::variable(M[0][0]);
        Dual<double> m01 = Dual<double>::constant(M[0][1]);
        Dual<double> m02 = Dual<double>::constant(M[0][2]);
        Dual<double> m10 = Dual<double>::constant(M[1][0]);
        Dual<double> m11 = Dual<double>::constant(M[1][1]);
        Dual<double> m12 = Dual<double>::constant(M[1][2]);
        Dual<double> m20 = Dual<double>::constant(M[2][0]);
        Dual<double> m21 = Dual<double>::constant(M[2][1]);
        Dual<double> m22 = Dual<double>::constant(M[2][2]);
        SpectralInvariantsDual<double> dual_inv =
            ravel::spectral_invariants_3x3_dual<double>(
                m00, m01, m02, m10, m11, m12, m20, m21, m22);
        // Compare .val with the known beta.
        std::printf("    Tribonacci: dual_inv.beta.val = %.10f, expected 1.8392867552\n",
                    dual_inv.beta.val);
        std::printf("    Tribonacci: dual_inv.beta2.val = %.10f\n",
                    dual_inv.beta2.val);
        // Diagnose which branch we entered.
        if (dual_inv.beta.val > 1.95) {
            std::printf("    Tribonacci: entered disc > 0 branch (cbrt)\n");
        } else {
            std::printf("    Tribonacci: entered disc <= 0 branch (fallback)\n");
        }
        EXPECT(std::abs(dual_inv.beta.val - 1.839286755214160) < 1e-9,
               "spectral_dual .val matches known beta at seed (Tribonacci fallback)");
    }

    std::printf("\n== spectral_dual: Pisot matrix coverage (Pisot-flavored, all-real-roots) ==\n");
    {
        // The golden-ratio Pisot matrix M = [[1,1,0],[1,0,0],[0,0,1]]
        // (extended 2x2 Fibonacci with an identity row/col) has char poly
        // (x-1)(x^2-x-1), all three roots real, discriminant <= 0 --
        // a genuine disc<=0 (three-real-roots) case, unlike Tribonacci
        // above (whose disc is actually > 0; the diagnostic print
        // above is a value-based guess, not a real branch check).
        // FIXED (2026-08-01): disc<=0 now has a real dual_cos/dual_acos
        // trigonometric-Cardano path (include/ravel/dual.hpp,
        // spectral_dual.hpp), replacing the old eps=0 fallback.
        std::vector<std::vector<long long>> M = {
            {1, 1, 0}, {1, 0, 0}, {0, 0, 1}};
        auto ref = ravel::spectral_invariants_3x3(
            M[0][0], M[0][1], M[0][2],
            M[1][0], M[1][1], M[1][2],
            M[2][0], M[2][1], M[2][2]);
        EXPECT(std::abs(ref.beta - 1.618033988749895) < 1e-9,
               "spectral_invariants_3x3: golden matrix beta = golden ratio");
        EXPECT(ref.beta > 1.0 && ref.beta2 < 1.0,
               "golden matrix: Perron > 1, secondary < 1 (Pisot-flavored)");
        Dual<double> m00 = Dual<double>::variable(M[0][0]);
        Dual<double> m01 = Dual<double>::constant(M[0][1]);
        Dual<double> m02 = Dual<double>::constant(M[0][2]);
        Dual<double> m10 = Dual<double>::constant(M[1][0]);
        Dual<double> m11 = Dual<double>::constant(M[1][1]);
        Dual<double> m12 = Dual<double>::constant(M[1][2]);
        Dual<double> m20 = Dual<double>::constant(M[2][0]);
        Dual<double> m21 = Dual<double>::constant(M[2][1]);
        Dual<double> m22 = Dual<double>::constant(M[2][2]);
        SpectralInvariantsDual<double> dual_inv =
            ravel::spectral_invariants_3x3_dual<double>(
                m00, m01, m02, m10, m11, m12, m20, m21, m22);
        EXPECT(std::abs(dual_inv.beta.val - ref.beta) < 1e-9,
               "spectral_dual .val matches on golden Pisot (disc<=0 trig-Cardano path)");
        // FIXED (2026-08-01): the disc<=0 branch now has a real gradient
        // via dual_cos/dual_acos. Cross-check against finite difference
        // on M[0][0] (seeded variable) rather than asserting eps==0.
        double h = 1e-6;
        std::vector<std::vector<long long>> Mp = {{2, 1, 0}, {1, 0, 0}, {0, 0, 1}};
        (void)Mp;
        auto beta_at = [&](double a00) {
            Dual<double> v00 = Dual<double>::constant(a00);
            Dual<double> v01 = Dual<double>::constant(1.0);
            Dual<double> v02 = Dual<double>::constant(0.0);
            Dual<double> v10 = Dual<double>::constant(1.0);
            Dual<double> v11 = Dual<double>::constant(0.0);
            Dual<double> v12 = Dual<double>::constant(0.0);
            Dual<double> v20 = Dual<double>::constant(0.0);
            Dual<double> v21 = Dual<double>::constant(0.0);
            Dual<double> v22 = Dual<double>::constant(1.0);
            return ravel::spectral_invariants_3x3_dual<double>(
                v00, v01, v02, v10, v11, v12, v20, v21, v22).beta.val;
        };
        double fd = (beta_at(1.0 + h) - beta_at(1.0 - h)) / (2 * h);
        EXPECT(std::abs(dual_inv.beta.eps - fd) < 1e-4,
               "disc<=0 branch: analytic gradient matches finite difference "
               "(fallback limitation closed via dual_cos/dual_acos)");
    }

    std::printf("\n%d passed, %d failed\n", n_pass, n_fail);
    return n_fail == 0 ? 0 : 1;
}
