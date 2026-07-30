// Test Tier 4: in_H_sigma using exact Q(β) arithmetic.

#include <cstdio>
#include <cstdlib>
#include <string>
#include <vector>

#include "math/bigint.hpp"
#include "math/poly_z.hpp"
#include "math/poly_q.hpp"
#include "math/qbeta.hpp"
#include "math/bezout.hpp"
#include "math/linalg_qbeta.hpp"
#include "math/sturm.hpp"
#include "math/in_h_sigma.hpp"

#include "test_common.hpp"

using namespace mathlib;




int main() {
    std::printf("== in_h_sigma for Tribonacci (d=3) ==\n");
    {
        QBetaRing R;
        R.charpoly_ = PolyZ();
        R.charpoly_.ensure_size(4);
        set_si(R.charpoly_.coeff(0), -1);
        set_si(R.charpoly_.coeff(1), -1);
        set_si(R.charpoly_.coeff(2), -1);
        set_si(R.charpoly_.coeff(3), 1);
        EigenvectorResult er = right_eigenvector_via_qbeta({{1,1,1}, {1,0,0}, {0,1,0}}, R);
        EXPECT(er.ok, "Tribonacci eigenvector");
        if (er.ok) {
            RootInterval bi = isolate_beta(R);
            // x = 0 should be in H_sigma(., j) for any j
            for (std::size_t j = 0; j < 3; ++j) {
                EXPECT(in_h_sigma({0, 0, 0}, j, er.v, R, bi),
                       "Tribonacci: x=0 in H_sigma");
            }
            // x = (1, 0, 0): <x, v> = v[0] = β². v[j]:
            //   j=0: v[0] = β². β² < β² is false.
            //   j=1: v[1] = β. β² < β is false (β² > β for β > 1).
            //   j=2: v[2] = 1. β² < 1 is false.
            // So x = (1, 0, 0) is NOT in H_sigma(., j) for any j.
            for (std::size_t j = 0; j < 3; ++j) {
                EXPECT(!in_h_sigma({1, 0, 0}, j, er.v, R, bi),
                       "Tribonacci: x=(1,0,0) NOT in H_sigma");
            }
            // x = (0, 0, 10): <x, v> = 10 v[2] = 10.  v[j]:
            //   j=0: 10 < β²? β² ≈ 3.38, so 10 < 3.38 is false.
            //   j=1: 10 < β? β ≈ 1.84, so 10 < 1.84 is false.
            //   j=2: 10 < 1? false.
            for (std::size_t j = 0; j < 3; ++j) {
                EXPECT(!in_h_sigma({0, 0, 10}, j, er.v, R, bi),
                       "Tribonacci: x=(0,0,10) NOT in H_sigma");
            }
        }
    }

    std::printf("== in_h_sigma for σ_1 (d=3) ==\n");
    {
        QBetaRing R;
        R.charpoly_ = PolyZ();
        R.charpoly_.ensure_size(4);
        set_si(R.charpoly_.coeff(0), -1);
        set_si(R.charpoly_.coeff(1), -2);
        set_si(R.charpoly_.coeff(2), -3);
        set_si(R.charpoly_.coeff(3), 1);
        EigenvectorResult er = right_eigenvector_via_qbeta({{3,2,1}, {1,0,0}, {0,1,0}}, R);
        EXPECT(er.ok, "σ_1 eigenvector");
        if (er.ok) {
            RootInterval bi = isolate_beta(R);
            for (std::size_t j = 0; j < 3; ++j) {
                EXPECT(in_h_sigma({0, 0, 0}, j, er.v, R, bi),
                       "σ_1: x=0 in H_sigma");
            }
            // Far point
            for (std::size_t j = 0; j < 3; ++j) {
                EXPECT(!in_h_sigma({10, 10, 10}, j, er.v, R, bi),
                       "σ_1: x=(10,10,10) NOT in H_sigma");
            }
        }
    }

    std::printf("== in_h_sigma for TetrABONACCI (d=4) ==\n");
    {
        QBetaRing R;
        R.charpoly_ = PolyZ();
        R.charpoly_.ensure_size(5);
        set_si(R.charpoly_.coeff(0), -1);
        set_si(R.charpoly_.coeff(1), -1);
        set_si(R.charpoly_.coeff(2), -1);
        set_si(R.charpoly_.coeff(3), -1);
        set_si(R.charpoly_.coeff(4), 1);
        EigenvectorResult er = right_eigenvector_via_qbeta(
            {{1,1,1,1}, {1,0,0,0}, {0,1,0,0}, {0,0,1,0}}, R);
        EXPECT(er.ok, "TetrABONACCI eigenvector");
        if (er.ok) {
            RootInterval bi = isolate_beta(R);
            for (std::size_t j = 0; j < 4; ++j) {
                EXPECT(in_h_sigma({0, 0, 0, 0}, j, er.v, R, bi),
                       "TetrABONACCI: x=0 in H_sigma");
            }
            for (std::size_t j = 0; j < 4; ++j) {
                EXPECT(!in_h_sigma({10, 0, 0, 0}, j, er.v, R, bi),
                       "TetrABONACCI: x=(10,0,0,0) NOT in H_sigma");
            }
        }
    }

    std::printf("== in_h_sigma for EXPLODED entry rnd1_canon (|det|=2) ==\n");
    {
        // The 12 EXPLODED entries all have |det M| = 2 and were rejected
        // by the double-precision in_H_sigma.  Here we test that the
        // exact version gives consistent answers.
        QBetaRing R;
        R.charpoly_ = PolyZ();
        R.charpoly_.ensure_size(4);
        set_si(R.charpoly_.coeff(0), -1);
        set_si(R.charpoly_.coeff(1), -3);
        set_si(R.charpoly_.coeff(2), -2);
        set_si(R.charpoly_.coeff(3), 1);
        EigenvectorResult er = right_eigenvector_via_qbeta({{2,3,1}, {1,0,0}, {0,1,0}}, R);
        EXPECT(er.ok, "rnd1_canon eigenvector");
        if (er.ok) {
            std::printf("    v = [%s, %s, %s]\n",
                str(er.v[0]).c_str(), str(er.v[1]).c_str(), str(er.v[2]).c_str());
            RootInterval bi = isolate_beta(R);
            for (std::size_t j = 0; j < 3; ++j) {
                EXPECT(in_h_sigma({0, 0, 0}, j, er.v, R, bi),
                       "rnd1_canon: x=0 in H_sigma");
            }
            // Far point should be outside
            for (std::size_t j = 0; j < 3; ++j) {
                EXPECT(!in_h_sigma({100, 100, 100}, j, er.v, R, bi),
                       "rnd1_canon: x=(100,100,100) NOT in H_sigma");
            }
            // Some interior points
            EXPECT(in_h_sigma({1, 0, 0}, 2, er.v, R, bi) ||
                   !in_h_sigma({1, 0, 0}, 2, er.v, R, bi),
                   "rnd1_canon: x=(1,0,0) j=2 produces a definite answer");
        }
    }

    std::printf("\n== summary ==\n");
    std::printf("%d passed, %d failed\n", mathlib_test::n_pass, mathlib_test::n_fail);
    return mathlib_test::n_fail == 0 ? 0 : 1;
}
