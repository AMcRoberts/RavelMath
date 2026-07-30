// Test: perron_frobenius.hpp (irreducibility, primitivity, and the
// full Perron-Frobenius certificate).
//
// Reference values: golden ratio phi = (1+sqrt(5))/2 ~= 1.6180339887
// for the Fibonacci companion matrix (a standard closed-form value,
// and the same matrix used in math/tests/test_ball.cpp); Tribonacci
// beta ~= 1.8392867552, Tetrabonacci beta ~= 1.9275619754 (both
// cross-checked elsewhere in this project, e.g. test_ball.cpp and
// tests/spectral_general_test.cpp).

#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <vector>

#include "math/perron_frobenius.hpp"

#include "test_common.hpp"

using namespace mathlib;

namespace {

// Converts a RootInterval's midpoint to a double, for a loose sanity
// check against a known decimal reference (NOT the certificate's own
// correctness argument, which is exact throughout -- this is purely
// "does the bracket land in the right neighborhood").
double interval_mid_double(const RootInterval& r) {
    Rat sum;
    add(sum, r.lo, r.hi);
    Rat two;
    set_si(two, 2, 1);
    Rat mid;
    div(mid, sum, two);
    return mpq_get_d(mid.get());
}

}  // namespace

int main() {
    std::printf("== perron_frobenius: irreducibility ==\n");
    {
        std::vector<std::vector<long long>> fib = {{1, 1}, {1, 0}};
        EXPECT(is_irreducible(fib), "Fibonacci companion matrix is irreducible");
    }
    {
        std::vector<std::vector<long long>> trib = {
            {1, 1, 1}, {1, 0, 0}, {0, 1, 0}};
        EXPECT(is_irreducible(trib), "Tribonacci companion matrix is irreducible");
    }
    {
        std::vector<std::vector<long long>> tetra = {
            {1, 1, 1, 1}, {1, 0, 0, 0}, {0, 1, 0, 0}, {0, 0, 1, 0}};
        EXPECT(is_irreducible(tetra), "Tetrabonacci companion matrix is irreducible");
    }
    {
        std::vector<std::vector<long long>> swap2 = {{0, 1}, {1, 0}};
        EXPECT(is_irreducible(swap2), "the 2-cycle swap matrix is irreducible");
    }
    {
        // Upper triangular: 0 -> 1 exists (M[0][1]=1>0) but 1 -> 0
        // does not (M[1][0]=0); vertex 1 cannot reach vertex 0.
        std::vector<std::vector<long long>> upper_tri = {{1, 1}, {0, 1}};
        EXPECT(!is_irreducible(upper_tri), "upper-triangular matrix is reducible");
    }
    {
        // A pure diagonal matrix: no edges at all between the two
        // vertices (only self-loops), so neither reaches the other.
        std::vector<std::vector<long long>> diag = {{2, 0}, {0, 3}};
        EXPECT(!is_irreducible(diag), "diagonal matrix is reducible");
    }
    {
        bool threw = false;
        try {
            std::vector<std::vector<long long>> bad = {{1, 2}, {3, 4}, {5, 6}};
            is_irreducible(bad);
        } catch (const std::invalid_argument&) {
            threw = true;
        }
        EXPECT(threw, "is_irreducible on a non-square matrix throws");
    }

    std::printf("\n== perron_frobenius: primitivity ==\n");
    {
        std::vector<std::vector<long long>> fib = {{1, 1}, {1, 0}};
        EXPECT(is_primitive(fib),
               "Fibonacci companion matrix is primitive (has a self-loop)");
    }
    {
        std::vector<std::vector<long long>> swap2 = {{0, 1}, {1, 0}};
        EXPECT(is_irreducible(swap2) && !is_primitive(swap2),
               "the 2-cycle swap matrix is irreducible but NOT primitive "
               "(period 2: even powers give I, odd powers give the swap, "
               "neither is ever fully positive)");
    }
    {
        std::vector<std::vector<long long>> trib = {
            {1, 1, 1}, {1, 0, 0}, {0, 1, 0}};
        EXPECT(is_primitive(trib), "Tribonacci companion matrix is primitive");
    }

    std::printf("\n== perron_frobenius: full certificate ==\n");
    {
        std::vector<std::vector<long long>> fib = {{1, 1}, {1, 0}};
        PerronFrobeniusCertificate cert = certify_perron_frobenius(fib);
        EXPECT(cert.irreducible, "Fibonacci: irreducible");
        EXPECT(cert.eigen_equation_holds, "Fibonacci: M v = beta v holds exactly");
        EXPECT(cert.eigenvector_all_positive, "Fibonacci: eigenvector is entrywise positive");
        EXPECT(cert.ok, "Fibonacci: full certificate passes");
        double mid = interval_mid_double(cert.beta_interval);
        EXPECT(std::abs(mid - 1.6180339887) < 1e-6,
               "Fibonacci: isolated beta matches the golden ratio to 1e-6");
        EXPECT(cert.eigenvector.size() == 2, "Fibonacci: eigenvector has 2 coordinates");
    }
    {
        std::vector<std::vector<long long>> trib = {
            {1, 1, 1}, {1, 0, 0}, {0, 1, 0}};
        PerronFrobeniusCertificate cert = certify_perron_frobenius(trib);
        EXPECT(cert.ok, "Tribonacci: full certificate passes");
        double mid = interval_mid_double(cert.beta_interval);
        EXPECT(std::abs(mid - 1.8392867552) < 1e-6,
               "Tribonacci: isolated beta matches the known reference to 1e-6");
    }
    {
        std::vector<std::vector<long long>> tetra = {
            {1, 1, 1, 1}, {1, 0, 0, 0}, {0, 1, 0, 0}, {0, 0, 1, 0}};
        PerronFrobeniusCertificate cert = certify_perron_frobenius(tetra);
        EXPECT(cert.ok, "Tetrabonacci: full certificate passes");
        double mid = interval_mid_double(cert.beta_interval);
        EXPECT(std::abs(mid - 1.9275619754) < 1e-6,
               "Tetrabonacci: isolated beta matches the known reference to 1e-6");
    }
    {
        // Negative case: a reducible matrix must short-circuit cleanly
        // to `ok = false`, not throw and not silently fabricate a
        // certificate -- the certificate's whole point is to say NO
        // when the hypothesis (irreducibility) isn't met.
        std::vector<std::vector<long long>> upper_tri = {{1, 1}, {0, 1}};
        PerronFrobeniusCertificate cert = certify_perron_frobenius(upper_tri);
        EXPECT(!cert.irreducible, "reducible matrix: irreducible flag is false");
        EXPECT(!cert.ok, "reducible matrix: certificate does not claim ok");
    }
    {
        // Negative-entry input must throw, not silently misbehave.
        bool threw = false;
        try {
            std::vector<std::vector<long long>> bad = {{1, -1}, {1, 0}};
            certify_perron_frobenius(bad);
        } catch (const std::invalid_argument&) {
            threw = true;
        }
        EXPECT(threw, "a matrix with a negative entry throws invalid_argument");
    }

    std::printf("\n%d passed, %d failed\n", mathlib_test::n_pass, mathlib_test::n_fail);
    return mathlib_test::n_fail == 0 ? 0 : 1;
}
