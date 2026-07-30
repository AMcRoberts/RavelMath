// Test Tier 3: linear algebra over Q(β).
//
// Reference values: pre-computed from SymPy (see sympy_ref.py /
// verify_linalg_refs.py).  All tests assert EXACT mpq_t equality.

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

#include "test_common.hpp"

using namespace mathlib;






int main() {
    std::printf("== linalg::determinant (3x3 over Q(β) for Tribonacci) ==\n");
    {
        // Identity matrix
        QBetaRing R = QBetaRing::from_low_first({-1, -1, -1});
        QBetaMat I(3, QBetaVec(3));
        for (std::size_t i = 0; i < 3; ++i) {
            for (std::size_t j = 0; j < 3; ++j) {
                I[i][j] = (i == j) ? R.from_int(1) : R.from_int(0);
            }
        }
        QElem det = determinant(I, R);
        EXPECT_EQ_QELEM(det, R.from_int(1));
    }

    std::printf("== linalg::determinant of 2x2 (Fibonacci) ==\n");
    {
        // For Fibonacci, charpoly = x^2 - x - 1.  The (d-1) cofactor of
        // (M - β I) for M = [[1,1],[1,0]] is (1 - β).  Its determinant
        // is 1 - β, which is the charpoly divided by something... let's
        // just check we can compute it.
        QBetaRing R = QBetaRing::from_low_first({-1, -1});
        QBetaMat A(1, QBetaVec(1));
        A[0][0] = R.add(R.from_int(1), R.neg(R.beta_k(1)));  // 1 - β
        QElem det = determinant(A, R);
        EXPECT_EQ_QELEM(det, R.add(R.from_int(1), R.neg(R.beta_k(1))));
    }

    std::printf("== linalg::solve_linear (simple system) ==\n");
    {
        // Solve (1+β) x = 1 in Fibonacci, so x = (1+β)^{-1} = 2 - β
        QBetaRing R = QBetaRing::from_low_first({-1, -1});
        QBetaMat A(1, QBetaVec(1));
        A[0][0] = R.add(R.from_int(1), R.beta_k(1));  // 1 + β
        QBetaVec b(1);
        b[0] = R.from_int(1);
        QBetaVec x = solve_linear(A, b, R);
        EXPECT_EQ_QELEM(x[0], qbeta_vec({{2,1}, {-1,1}}));
    }

    std::printf("== linalg::right_eigenvector Tribonacci ==\n");
    {
        // M = [[1,1,1],[1,0,0],[0,1,0]], charpoly x³ - x² - x - 1
        // Expected: v = (β², β, 1)  (so v[0]=β², v[1]=β, v[2]=1)
        QBetaRing R = QBetaRing::from_low_first({-1, -1, -1});
        std::vector<std::vector<long long>> M = {{1,1,1}, {1,0,0}, {0,1,0}};
        EigenvectorResult er = right_eigenvector_via_qbeta(M, R);
        EXPECT(er.ok, "Tribonacci eigenvector found");
        if (er.ok) {
            std::printf("    v[0] = %s\n", str(er.v[0]).c_str());
            std::printf("    v[1] = %s\n", str(er.v[1]).c_str());
            std::printf("    v[2] = %s\n", str(er.v[2]).c_str());
            // v[2] should be 1
            EXPECT_EQ_QELEM(er.v[2], R.from_int(1));
            // v[1] should be β
            EXPECT_EQ_QELEM(er.v[1], R.beta_k(1));
            // v[0] should be β²
            EXPECT_EQ_QELEM(er.v[0], R.beta_k(2));
            // Verify M v = β v
            QBetaVec Mv(3);
            for (std::size_t i = 0; i < 3; ++i) {
                Mv[i] = R.from_int(0);
                for (std::size_t j = 0; j < 3; ++j) {
                    QElem prod = R.mul(R.from_int(M[i][j]), er.v[j]);
                    Mv[i] = R.add(Mv[i], prod);
                }
            }
            QBetaVec bv(3);
            for (std::size_t i = 0; i < 3; ++i) {
                bv[i] = R.mul(R.beta_k(1), er.v[i]);
            }
            for (std::size_t i = 0; i < 3; ++i) {
                EXPECT(Mv[i] == bv[i], "Tribonacci: M v = β v");
            }
        }
    }

    std::printf("== linalg::right_eigenvector σ_1 ==\n");
    {
        // M = [[3,2,1],[1,0,0],[0,1,0]], charpoly x³ - 3x² - 2x - 1
        QBetaRing R = QBetaRing::from_low_first({-3, -2, -1});
        std::vector<std::vector<long long>> M = {{3,2,1}, {1,0,0}, {0,1,0}};
        EigenvectorResult er = right_eigenvector_via_qbeta(M, R);
        EXPECT(er.ok, "σ_1 eigenvector found");
        if (er.ok) {
            std::printf("    v = [%s, %s, %s]\n",
                str(er.v[0]).c_str(), str(er.v[1]).c_str(), str(er.v[2]).c_str());
            // Verify M v = β v
            QBetaVec Mv(3);
            for (std::size_t i = 0; i < 3; ++i) {
                Mv[i] = R.from_int(0);
                for (std::size_t j = 0; j < 3; ++j) {
                    QElem prod = R.mul(R.from_int(M[i][j]), er.v[j]);
                    Mv[i] = R.add(Mv[i], prod);
                }
            }
            QBetaVec bv(3);
            for (std::size_t i = 0; i < 3; ++i) {
                bv[i] = R.mul(R.beta_k(1), er.v[i]);
            }
            for (std::size_t i = 0; i < 3; ++i) {
                EXPECT(Mv[i] == bv[i], "σ_1: M v = β v");
            }
        }
    }

    std::printf("== linalg::left_eigenvector σ_1 (dedicated, not a transpose of the right eigenvector) ==\n");
    {
        // Same M as the right-eigenvector σ_1 test above:
        // M = [[3,2,1],[1,0,0],[0,1,0]], charpoly x³ - 3x² - 2x - 1.
        QBetaRing R = QBetaRing::from_low_first({-3, -2, -1});
        std::vector<std::vector<long long>> M = {{3,2,1}, {1,0,0}, {0,1,0}};

        EigenvectorResult left = left_eigenvector_via_qbeta(M, R);
        EXPECT(left.ok, "σ_1 left eigenvector found");
        if (left.ok) {
            std::printf("    v_left  = [%s, %s, %s]\n",
                str(left.v[0]).c_str(), str(left.v[1]).c_str(), str(left.v[2]).c_str());

            // Direct check: v^T M = β v^T, i.e. for each column j,
            // sum_i v[i] * M[i][j] == β * v[j].
            QBetaVec vM(3);
            for (std::size_t j = 0; j < 3; ++j) {
                vM[j] = R.from_int(0);
                for (std::size_t i = 0; i < 3; ++i) {
                    QElem prod = R.mul(left.v[i], R.from_int(M[i][j]));
                    vM[j] = R.add(vM[j], prod);
                }
            }
            for (std::size_t j = 0; j < 3; ++j) {
                QElem beta_vj = R.mul(R.beta_k(1), left.v[j]);
                EXPECT(vM[j] == beta_vj, "σ_1: v^T M = β v^T (direct, un-transposed check)");
            }

            // Cross-check: an INDEPENDENT derivation -- build M^T
            // explicitly (obviously-correct-by-inspection transposition)
            // and feed it to the already-tested right_eigenvector_via_qbeta.
            // The two derivations (index-swapped Cramer system here vs.
            // literal transpose-then-solve there) must agree exactly.
            std::vector<std::vector<long long>> Mt = {
                {M[0][0], M[1][0], M[2][0]},
                {M[0][1], M[1][1], M[2][1]},
                {M[0][2], M[1][2], M[2][2]},
            };
            EigenvectorResult via_transpose = right_eigenvector_via_qbeta(Mt, R);
            EXPECT(via_transpose.ok, "σ_1: right_eigenvector_via_qbeta(M^T, R) succeeds");
            if (via_transpose.ok) {
                for (std::size_t i = 0; i < 3; ++i) {
                    EXPECT(left.v[i] == via_transpose.v[i],
                           "σ_1: left_eigenvector_via_qbeta agrees with right_eigenvector_via_qbeta(M^T, R)");
                }
            }

            // The point of the whole exercise: confirm the left and
            // right eigenvectors of M are genuinely DIFFERENT vectors
            // for this M (not merely a different normalization of the
            // same direction) -- this is the concrete fact that made
            // core.hpp's bug possible in the first place, so it's
            // worth asserting here rather than just assuming it.
            EigenvectorResult right = right_eigenvector_via_qbeta(M, R);
            EXPECT(right.ok, "σ_1 right eigenvector found (for the left != right comparison)");
            if (right.ok) {
                bool any_differ = false;
                for (std::size_t i = 0; i < 3; ++i) {
                    if (!(left.v[i] == right.v[i])) { any_differ = true; break; }
                }
                EXPECT(any_differ,
                       "σ_1: left and right eigenvectors are genuinely different "
                       "(not proportional under the shared v[2]=1 normalization)");
            }
        }
    }

    std::printf("== linalg::left_eigenvector on a SYMMETRIC matrix (left == right, sanity check) ==\n");
    {
        // M = [[1,1],[1,-1]], charpoly x^2 - 2 (irreducible over Q,
        // beta = sqrt(2)) -- symmetric, so left and right eigenvectors
        // must coincide exactly (up to the shared v[d-1]=1
        // normalization).  NOTE: an earlier version of this test used
        // M = [[2,1],[1,2]] with charpoly x^2-4x+3 = (x-1)(x-3), which
        // is REDUCIBLE -- Q[x]/(f) for reducible f is not a field, so
        // the whole Cramer's-rule/Gaussian-elimination machinery this
        // module relies on isn't guaranteed to behave as a genuine
        // eigenvector solve there.  That version's assertion (left.v
        // == right.v) still happened to pass, but only because it
        // checked the two computed vectors against EACH OTHER, never
        // against the actual eigenvector equation -- caught by
        // verify_left_eigenvector/verify_right_eigenvector below
        // failing on that reducible-ring input during a
        // consistency pass.  Fixed here with a genuinely irreducible
        // charpoly instead of just patching the assertion.
        QBetaRing R = QBetaRing::from_low_first({0, -2});
        std::vector<std::vector<long long>> M = {{1, 1}, {1, -1}};
        EigenvectorResult left = left_eigenvector_via_qbeta(M, R);
        EigenvectorResult right = right_eigenvector_via_qbeta(M, R);
        EXPECT(left.ok && right.ok, "symmetric-matrix eigenvectors found");
        if (left.ok && right.ok) {
            for (std::size_t i = 0; i < 2; ++i) {
                EXPECT(left.v[i] == right.v[i],
                       "symmetric matrix: left eigenvector == right eigenvector exactly");
            }
            EXPECT(verify_left_eigenvector(left.v, M, R),
                   "symmetric matrix: left eigenvector genuinely satisfies v^T M = beta v^T");
            EXPECT(verify_right_eigenvector(right.v, M, R),
                   "symmetric matrix: right eigenvector genuinely satisfies M v = beta v");
        }
    }

    std::printf("== linalg::left_eigenvector σ_1 (d=1 degenerate, structural sanity) ==\n");
    {
        // Not a real Pisot case (d must be >= 2 for anything in this
        // project), but exercises the d==1 special-case branch shared
        // with right_eigenvector_via_qbeta.
        QBetaRing R = QBetaRing::from_low_first({-5});
        std::vector<std::vector<long long>> M = {{5}};
        EigenvectorResult left = left_eigenvector_via_qbeta(M, R);
        EXPECT(left.ok, "d=1 left eigenvector found");
        if (left.ok) {
            EXPECT_EQ_QELEM(left.v[0], R.from_int(1));
        }
    }

    std::printf("== linalg::verify_left/right_eigenvector (the prevention mechanism) ==\n");
    {
        // Same σ_1 matrix as above: M = [[3,2,1],[1,0,0],[0,1,0]].
        QBetaRing R = QBetaRing::from_low_first({-3, -2, -1});
        std::vector<std::vector<long long>> M = {{3,2,1}, {1,0,0}, {0,1,0}};
        EigenvectorResult left = left_eigenvector_via_qbeta(M, R);
        EigenvectorResult right = right_eigenvector_via_qbeta(M, R);
        EXPECT(left.ok && right.ok, "σ_1 both eigenvectors found (setup)");
        if (left.ok && right.ok) {
            // Positive cases: each vector passes its OWN check.
            EXPECT(verify_left_eigenvector(left.v, M, R),
                   "verify_left_eigenvector accepts a genuine left eigenvector");
            EXPECT(verify_right_eigenvector(right.v, M, R),
                   "verify_right_eigenvector accepts a genuine right eigenvector");

            // THE critical negative cases -- this is the exact
            // scenario the historical bug produced (a right
            // eigenvector handed to code that needed a left one, and
            // vice versa).  If these two checks did NOT correctly
            // reject the mismatched vector, they would be worthless
            // as a prevention mechanism.
            EXPECT(!verify_left_eigenvector(right.v, M, R),
                   "verify_left_eigenvector REJECTS the right eigenvector "
                   "(reproduces catching the historical bug)");
            EXPECT(!verify_right_eigenvector(left.v, M, R),
                   "verify_right_eigenvector REJECTS the left eigenvector");
        }
    }
    {
        // Symmetric-matrix sanity: when left == right, both checks
        // must accept the SAME vector (no false negative just because
        // the two happen to coincide).  Same M = [[1,1],[1,-1]],
        // charpoly x^2-2 as above (irreducible -- see that block's
        // comment for why this matters).
        QBetaRing R = QBetaRing::from_low_first({0, -2});
        std::vector<std::vector<long long>> M = {{1, 1}, {1, -1}};
        EigenvectorResult v = left_eigenvector_via_qbeta(M, R);
        EXPECT(v.ok, "symmetric matrix eigenvector found");
        if (v.ok) {
            EXPECT(verify_left_eigenvector(v.v, M, R),
                   "symmetric matrix: verify_left_eigenvector accepts");
            EXPECT(verify_right_eigenvector(v.v, M, R),
                   "symmetric matrix: verify_right_eigenvector ALSO accepts "
                   "(left == right here, both checks must agree)");
        }
    }
    {
        // A vector that is neither: must be rejected by both.
        QBetaRing R = QBetaRing::from_low_first({-3, -2, -1});
        std::vector<std::vector<long long>> M = {{3,2,1}, {1,0,0}, {0,1,0}};
        QBetaVec garbage = {R.from_int(1), R.from_int(1), R.from_int(1)};
        EXPECT(!verify_left_eigenvector(garbage, M, R),
               "an arbitrary non-eigenvector fails the left check");
        EXPECT(!verify_right_eigenvector(garbage, M, R),
               "an arbitrary non-eigenvector fails the right check");
    }

    std::printf("== linalg::right_eigenvector TetrABONACCI ==\n");
    {
        // M = [[1,1,1,1],[1,0,0,0],[0,1,0,0],[0,0,1,0]], charpoly x⁴ - x³ - x² - x - 1
        QBetaRing R = QBetaRing::from_low_first({-1, -1, -1, -1});
        std::vector<std::vector<long long>> M = {
            {1,1,1,1}, {1,0,0,0}, {0,1,0,0}, {0,0,1,0}
        };
        EigenvectorResult er = right_eigenvector_via_qbeta(M, R);
        EXPECT(er.ok, "TetrABONACCI eigenvector found");
        if (er.ok) {
            std::printf("    v = [%s, %s, %s, %s]\n",
                str(er.v[0]).c_str(), str(er.v[1]).c_str(),
                str(er.v[2]).c_str(), str(er.v[3]).c_str());
            // Verify M v = β v
            QBetaVec Mv(4);
            for (std::size_t i = 0; i < 4; ++i) {
                Mv[i] = R.from_int(0);
                for (std::size_t j = 0; j < 4; ++j) {
                    QElem prod = R.mul(R.from_int(M[i][j]), er.v[j]);
                    Mv[i] = R.add(Mv[i], prod);
                }
            }
            QBetaVec bv(4);
            for (std::size_t i = 0; i < 4; ++i) {
                bv[i] = R.mul(R.beta_k(1), er.v[i]);
            }
            for (std::size_t i = 0; i < 4; ++i) {
                EXPECT(Mv[i] == bv[i], "TetrABONACCI: M v = β v");
            }
        }
    }

    std::printf("== linalg::right_eigenvector EXPLODED entry rnd1_canon ==\n");
    {
        // rnd1_canon: charpoly x³ - 2x² - 3x - 1, |det M| = 2
        QBetaRing R = QBetaRing::from_low_first({-2, -3, -1});
        std::vector<std::vector<long long>> M = {{2,3,1}, {1,0,0}, {0,1,0}};
        EigenvectorResult er = right_eigenvector_via_qbeta(M, R);
        EXPECT(er.ok, "rnd1_canon eigenvector found");
        if (er.ok) {
            std::printf("    v = [%s, %s, %s]\n",
                str(er.v[0]).c_str(), str(er.v[1]).c_str(), str(er.v[2]).c_str());
            // Verify M v = β v
            QBetaVec Mv(3);
            for (std::size_t i = 0; i < 3; ++i) {
                Mv[i] = R.from_int(0);
                for (std::size_t j = 0; j < 3; ++j) {
                    QElem prod = R.mul(R.from_int(M[i][j]), er.v[j]);
                    Mv[i] = R.add(Mv[i], prod);
                }
            }
            QBetaVec bv(3);
            for (std::size_t i = 0; i < 3; ++i) {
                bv[i] = R.mul(R.beta_k(1), er.v[i]);
            }
            for (std::size_t i = 0; i < 3; ++i) {
                EXPECT(Mv[i] == bv[i], "rnd1_canon: M v = β v");
            }
        }
    }

    std::printf("\n== summary ==\n");
    std::printf("%d passed, %d failed\n", mathlib_test::n_pass, mathlib_test::n_fail);
    return mathlib_test::n_fail == 0 ? 0 : 1;
}
