// mathlib_test.cpp
//
// Driver that runs all tier tests in the mathlib as a single binary.
// Each tier's test main() is wrapped in a function and called from
// here.  Tier tests are #included directly (rather than linked) so
// they share the same translation unit and we can avoid linker
// symbol collisions on `n_pass` / `n_fail` and helper types.

#include <cstdio>
#include <cstdlib>

// Each tier test defines its own main(); we rename them via macros
// and call them from a single main() below.
#define main test_poly_main
#include "test_poly.cpp"
#undef main

#define main test_qbeta_main
#include "test_qbeta.cpp"
#undef main

#define main test_bezout_main
#include "test_bezout.cpp"
#undef main

#define main test_linalg_qbeta_main
#include "test_linalg_qbeta.cpp"
#undef main

#define main test_sturm_main
#include "test_sturm.cpp"
#undef main

#define main test_in_h_sigma_main
#include "test_in_h_sigma.cpp"
#undef main

#define main test_exploded_main
#include "test_exploded.cpp"
#undef main

#define main test_mat_q_main
#include "test_mat_q.cpp"
#undef main

#define main test_ball_main
#include "test_ball.cpp"
#undef main

#define main test_bigfloat_main
#include "test_bigfloat.cpp"
#undef main

#define main test_charpoly_main
#include "test_charpoly.cpp"
#undef main

#define main test_perron_frobenius_main
#include "test_perron_frobenius.cpp"
#undef main

#define main test_primality_main
#include "test_primality.cpp"
#undef main

int main() {
    int total_pass = 0, total_fail = 0;
    struct { const char* name; int (*fn)(); } tests[] = {
        {"poly_z / poly_q", test_poly_main},
        {"qbeta arithmetic", test_qbeta_main},
        {"bezout / Q(β) inverse", test_bezout_main},
        {"linear algebra over Q(β)", test_linalg_qbeta_main},
        {"Sturm / real root isolation", test_sturm_main},
        {"in_H_sigma application", test_in_h_sigma_main},
        {"12 EXPLODED entries driver", test_exploded_main},
        {"MatQ (matrix over Q)", test_mat_q_main},
        {"Ball (exact interval arithmetic) + Collatz-Wielandt bracket", test_ball_main},
        {"BigFloat (tunable-precision float) + switchable Perron certification", test_bigfloat_main},
        {"charpoly (Faddeev-LeVerrier)", test_charpoly_main},
        {"Perron-Frobenius certificate (irreducibility, primitivity, exact eigenvector)", test_perron_frobenius_main},
        {"primality (is_prime, next_prime, sieve, Kronecker symbol)", test_primality_main},
    };
    for (const auto& t : tests) {
        std::printf("\n== mathlib tier: %s ==\n", t.name);
        int rc = t.fn();
        if (rc == 0) ++total_pass; else ++total_fail;
    }
    std::printf("\n== mathlib overall: %d/%d tier tests passed ==\n",
        total_pass, total_pass + total_fail);
    return total_fail == 0 ? 0 : 1;
}
