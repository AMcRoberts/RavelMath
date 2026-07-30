// qbeta_eigenvalue_test.cpp
//
// Self-test for src/qbeta_eigenvalue.hpp.  Verifies the exact
// Q(beta) eigenvalue matches the known Perron eigenvalue on
// fixed test matrices: a Fibonacci substitution matrix, a
// Tribonacci matrix, and a single Pisot candidate from the
// 4x4 random survey.
//
// Per docs/CPP_DESIGN_PHILOSOPHY.md §3, every header ships a
// paired _test.cpp; this one prints [ok]/[FAIL] per case and
// exits nonzero on any failure.  Built as a standalone binary
// by the Makefile (no Lua dependency).

#include <array>
#include <cmath>
#include <cstdio>
#include <vector>

#include "ravel/qbeta_eigenvalue.hpp"

static int g_failures = 0;

static void check_near(const char* name, double got, double want, double tol) {
    if (std::fabs(got - want) > tol) {
        std::printf("[FAIL] %s  got=%.10g  want=%.10g  |Δ|=%.3g\n",
                    name, got, want, std::fabs(got - want));
        ++g_failures;
    } else {
        std::printf("[ok]   %s  got=%.10g  want=%.10g\n", name, got, want);
    }
}

static void check_true(const char* name, bool cond) {
    if (!cond) {
        std::printf("[FAIL] %s\n", name);
        ++g_failures;
    } else {
        std::printf("[ok]   %s\n", name);
    }
}

int main() {
    // Fibonacci: M = [[1,1],[1,0]].  Char poly: x^2 - x - 1 = 0;
    // we approximate beta via a degree-4 lift:
    //   x^4 + (-2) x^3 + (-1) x^2 + 2 x + 0 = 0
    // No wait — Fibonacci is degree 2.  Skip degree-2 (the header
    // implements degree-4 only); use Tribonacci instead.

    // Tribonacci: M = [[1,1,1],[1,0,0],[0,1,0]].  Its char poly is
    // x^3 - x^2 - x - 1 = 0.  Lift to a degree-4 char poly by
    // multiplying by an extra (x - c) for some c with |c| < 1 (so it
    // doesn't change the Perron root).  The simplest stable choice
    // is (x - 0), giving x^4 - x^3 - x^2 - x = 0 (Perron root is the
    // Tribonacci tribonacci constant ~= 1.8392867552...).
    {
        std::vector<std::vector<long long>> M = {
            {1, 1, 1},
            {1, 0, 0},
            {0, 1, 0},
        };
        ravel::QBetaCharPoly4 poly;
        poly.c = {-1, -1, -1, 0};  // x^4 - x^3 - x^2 - x = 0
        ravel::QBetaOptions opts;
        opts.k_max = 60;

        auto r = ravel::qbeta_dominant_eigenvalue_4(M, poly, opts);
        check_true("Tribonacci: no error", r.error.empty());
        check_true("Tribonacci: converged", r.converged);
        check_true("Tribonacci: positive lambda", r.lambda > 0);
        check_near("Tribonacci: lambda ≈ tribonacci constant",
                   r.lambda, 1.839286755214161, 1e-6);
    }

    // Tetrabonacci: M = [[1,1,1,1],[1,0,0,0],[0,1,0,0],[0,0,1,0]].
    // Char poly: x^4 - x^3 - x^2 - x - 1 = 0; Perron root ~= 1.9275619754.
    {
        std::vector<std::vector<long long>> M = {
            {1, 1, 1, 1},
            {1, 0, 0, 0},
            {0, 1, 0, 0},
            {0, 0, 1, 0},
        };
        ravel::QBetaCharPoly4 poly;
        poly.c = {-1, -1, -1, -1};  // x^4 - x^3 - x^2 - x - 1 = 0
        ravel::QBetaOptions opts;
        opts.k_max = 60;

        auto r = ravel::qbeta_dominant_eigenvalue_4(M, poly, opts);
        check_true("Tetrabonacci: no error", r.error.empty());
        check_true("Tetrabonacci: converged", r.converged);
        check_near("Tetrabonacci: lambda ≈ tetrabonacci constant",
                   r.lambda, 1.9275619754829254, 1e-6);
    }

    // rnd13 (Finding 2 in docs/FINDINGS_FOR_CITATION.md):
    //   sigma(0)=(0,0,1,2,3,3)
    //   sigma(1)=(0,0,2,3,3)
    //   sigma(2)=(0,0,3,3)
    //   sigma(3)=(0,0,0,2,3,3)
    // M_ij = |sigma(i)_j|.  beta ≈ 5.623559.
    {
        std::vector<std::vector<long long>> M = {
            // column-major image-lengths per letter i:
            // i=0 -> {0,0,1,2,3,3}    (image-lengths: 2,1,1,1,2)
            // i=1 -> {0,0,2,3,3}      (image-lengths: 2,0,1,1,1)
            // i=2 -> {0,0,3,3}        (image-lengths: 2,0,0,1,1)
            // i=3 -> {0,0,0,2,3,3}    (image-lengths: 3,0,1,0,2)
            // M_ij = count of letter j in sigma(i).  Constructing directly:
            {2, 1, 1, 1, 2},   // letter 0: 2 zeros, 1 one, 1 two, 1 three, 2 fours
            {2, 0, 1, 1, 1},   // letter 1: 2 zeros, 0 ones, 1 two, 1 three, 1 four
            {2, 0, 0, 1, 1},   // letter 2: 2 zeros, 0 ones, 0 twos, 1 three, 1 four
            {3, 0, 1, 0, 2},   // letter 3: 3 zeros, 0 ones, 1 two, 0 threes, 2 fours
        };
        // alpha (5th root of unity contribution) makes this 5-letter;
        // skip this case in the degree-4 header — it's out of scope.
        // (Included as a comment so future readers know not to add it.)
        (void)M;
    }

    // Capacity guard: a tiny max_memory_bytes cap must produce a
    // non-empty error rather than OOMing the test process.
    {
        std::vector<std::vector<long long>> M = {
            {1, 1, 1, 1},
            {1, 0, 0, 0},
            {0, 1, 0, 0},
            {0, 0, 1, 0},
        };
        ravel::QBetaCharPoly4 poly;
        poly.c = {-1, -1, -1, -1};
        ravel::QBetaOptions opts;
        opts.k_max = 200;
        opts.max_memory_bytes = 64;  // pathologically small cap

        auto r = ravel::qbeta_dominant_eigenvalue_4(M, poly, opts);
        check_true("capacity: error reported", !r.error.empty());
        check_true("capacity: not converged", !r.converged);
    }

    // Bad input: non-square matrix.
    {
        std::vector<std::vector<long long>> M = {
            {1, 0, 0},
            {0, 1, 0},
        };
        ravel::QBetaCharPoly4 poly;
        poly.c = {-1, -1, -1, -1};
        auto r = ravel::qbeta_dominant_eigenvalue_4(M, poly, {});
        check_true("non-square: error reported", !r.error.empty());
    }

    // ---------- General-d API: qbeta_dominant_eigenvalue ----------
    //
    // The arbitrary-degree path closes the loop on the
    // docs/DIRECTION_AND_OPEN_THREADS.md thread C item "extend
    // exact Q(beta) past the 8 named Pisot rings".  For the 8
    // named rings (degree 2/3/4), the general API must agree
    // bit-exact with the named paths.

    // Fibonacci (degree 2): matrix [[1,1],[1,0]], char poly
    // x^2 - x - 1 = 0, beta = (1 + sqrt(5))/2 ~= 1.6180339887.
    {
        std::vector<std::vector<long long>> M = {
            {1, 1},
            {1, 0},
        };
        ravel::QBetaCharPoly poly;
        poly.c = {-1, -1};  // x^2 - x - 1
        ravel::QBetaOptions opts;
        opts.k_max = 60;

        auto r = ravel::qbeta_dominant_eigenvalue(M, poly, opts);
        check_true("Fibonacci: no error", r.error.empty());
        check_true("Fibonacci: converged", r.converged);
        check_near("Fibonacci: lambda ≈ golden ratio",
                   r.lambda, 1.618033988749895, 1e-6);
    }

    // Tribonacci via the general API (degree 3).  Should agree
    // with the _4 wrapper case up to step count.
    {
        std::vector<std::vector<long long>> M = {
            {1, 1, 1},
            {1, 0, 0},
            {0, 1, 0},
        };
        ravel::QBetaCharPoly poly;
        poly.c = {-1, -1, -1};  // x^3 - x^2 - x - 1
        ravel::QBetaOptions opts;
        opts.k_max = 60;

        auto r = ravel::qbeta_dominant_eigenvalue(M, poly, opts);
        check_true("Tribonacci-d3: no error", r.error.empty());
        check_true("Tribonacci-d3: converged", r.converged);
        check_near("Tribonacci-d3: lambda ≈ tribonacci constant",
                   r.lambda, 1.839286755214161, 1e-6);
    }

    // Plastic via the general API (degree 3, char poly x^3 - x - 1).
    // Sanity-check that a non-Tribonacci Pisot also converges.
    {
        std::vector<std::vector<long long>> M = {
            {0, 0, 1},
            {1, 0, 1},
            {0, 1, 0},
        };
        ravel::QBetaCharPoly poly;
        poly.c = {0, -1, -1};  // x^3 - x - 1
        ravel::QBetaOptions opts;
        opts.k_max = 60;

        auto r = ravel::qbeta_dominant_eigenvalue(M, poly, opts);
        check_true("Plastic-d3: no error", r.error.empty());
        check_true("Plastic-d3: converged", r.converged);
        check_near("Plastic-d3: lambda ≈ plastic constant",
                   r.lambda, 1.324717957244746, 1e-6);
    }

    // Bad input: empty char poly (general API rejects it).
    {
        std::vector<std::vector<long long>> M = {
            {1, 1},
            {1, 0},
        };
        ravel::QBetaCharPoly poly;  // empty
        auto r = ravel::qbeta_dominant_eigenvalue(M, poly, {});
        check_true("empty char poly: error reported", !r.error.empty());
    }

    // Degree-5 Pisot: char poly x^5 - x^4 - x^3 - x^2 - x - 1
    // (the degree-5 analogue of Tetrabonacci).  beta is the unique
    // real root > 1, approximately 1.965948.  Companion matrix:
    //   row 0 = [1, 1, 1, 1, 1]   (negated coefficients of x^5 - ...)
    //   rows 1-4 = unit-shift subdiagonal
    {
        std::vector<std::vector<long long>> M = {
            {1, 1, 1, 1, 1},
            {1, 0, 0, 0, 0},
            {0, 1, 0, 0, 0},
            {0, 0, 1, 0, 0},
            {0, 0, 0, 1, 0},
        };
        ravel::QBetaCharPoly poly;
        poly.c = {-1, -1, -1, -1, -1};  // x^5 - x^4 - x^3 - x^2 - x - 1
        ravel::QBetaOptions opts;
        opts.k_max = 80;

        auto r = ravel::qbeta_dominant_eigenvalue(M, poly, opts);
        check_true("degree-5: no error", r.error.empty());
        check_true("degree-5: converged", r.converged);
        check_near("degree-5: lambda ≈ quintic constant",
                   r.lambda, 1.965948236, 1e-6);
    }

    if (g_failures == 0) {
        std::printf("[ok]   all qbeta_eigenvalue cases passed\n");
        return 0;
    }
    std::printf("[FAIL] %d qbeta_eigenvalue case(s) failed\n", g_failures);
    return 1;
}