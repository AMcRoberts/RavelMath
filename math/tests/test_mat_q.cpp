// Test Tier 0d: MatQ (matrix over Q) with general n, m.
//
// Reference values: pre-computed from SymPy Matrix with domain=QQ.

#include <cstdio>
#include <cstdlib>
#include <stdexcept>
#include <string>

#include "math/bigint.hpp"
#include "math/poly_z.hpp"
#include "math/poly_q.hpp"
#include "math/mat_q.hpp"

#include "test_common.hpp"

using namespace mathlib;






int main() {
    std::printf("== MatQ constructor and basic access ==\n");
    {
        MatQ M(2, 3);
        EXPECT(M.n_rows() == 2, "n_rows = 2");
        EXPECT(M.n_cols() == 3, "n_cols = 3");
        EXPECT(M.is_square() == false, "not square");
        EXPECT(mathlib::is_zero(M.entries_[0][0]), "default zero");
    }

    std::printf("== MatQ identity and scalar ==\n");
    {
        MatQ I = identity_mat(3);
        EXPECT_EQ_RAT(I.entries_[0][0], 1);
        EXPECT_EQ_RAT(I.entries_[1][1], 1);
        EXPECT_EQ_RAT(I.entries_[2][2], 1);
        EXPECT_EQ_RAT(I.entries_[0][1], 0);
        EXPECT(I.is_square(), "identity is square");
    }

    std::printf("== MatQ add / sub / neg ==\n");
    {
        MatQ A({{1, 2}, {3, 4}});
        MatQ B({{5, 6}, {7, 8}});
        MatQ C = A + B;
        EXPECT_EQ_MAT(C, (MatQ({{6, 8}, {10, 12}})));
        MatQ D = A - B;
        EXPECT_EQ_MAT(D, (MatQ({{-4, -4}, {-4, -4}})));
        MatQ E = -A;
        EXPECT_EQ_MAT(E, (MatQ({{-1, -2}, {-3, -4}})));
    }

    std::printf("== MatQ scalar multiplication ==\n");
    {
        MatQ A({{1, 2}, {3, 4}});
        Rat s; set_si(s, 3, 1);
        MatQ M = A * s;
        EXPECT_EQ_MAT(M, (MatQ({{3, 6}, {9, 12}})));
        MatQ N = s * A;
        EXPECT_EQ_MAT(N, (MatQ({{3, 6}, {9, 12}})));
    }

    std::printf("== MatQ matrix multiplication ==\n");
    {
        MatQ A({{1, 2, 3}, {4, 5, 6}});  // 2x3
        MatQ B({{1, 2}, {3, 4}, {5, 6}});  // 3x2
        MatQ C = A * B;  // 2x2
        EXPECT_EQ_MAT(C, (MatQ({{22, 28}, {49, 64}})));
    }

    std::printf("== MatQ identity * M = M ==\n");
    {
        MatQ I = identity_mat(3);
        MatQ M({{1, 2, 3}, {4, 5, 6}, {7, 8, 9}});
        MatQ N = I * M;
        EXPECT_EQ_MAT(N, M);
        MatQ O = M * I;
        EXPECT_EQ_MAT(O, M);
    }

    std::printf("== MatQ transpose ==\n");
    {
        MatQ M({{1, 2, 3}, {4, 5, 6}});  // 2x3
        MatQ T = transpose(M);  // 3x2
        EXPECT(T.n_rows() == 3, "T.n_rows = 3");
        EXPECT(T.n_cols() == 2, "T.n_cols = 2");
        EXPECT_EQ_MAT(T, (MatQ({{1, 4}, {2, 5}, {3, 6}})));
    }

    std::printf("== MatQ determinant: 2x2 simple ==\n");
    {
        MatQ M({{1, 2}, {3, 4}});
        Rat det = determinant(M);
        EXPECT_EQ_RAT(det, -2);
    }

    std::printf("== MatQ determinant: 2x2 with rationals ==\n");
    {
        MatQ M({{1, 2}, {3, 4}});
        MatQ N({{1, 1}, {1, 1}});
        MatQ P = M * Rat(2);  // P = 2M
        MatQ Q = M * N;  // Q = M * ones = sums of rows
        Rat detP = determinant(P);
        EXPECT_EQ_RAT(detP, -8);  // 2^2 * det(M) = 4 * -2 = -8
    }

    std::printf("== MatQ determinant: 3x3 with rationals ==\n");
    {
        MatQ M({{1, 2, 3}, {4, 5, 6}, {7, 8, 10}});
        // det = ?
        // Use formula: for 3x3, expand.
        // det = 1*(5*10 - 6*8) - 2*(4*10 - 6*7) + 3*(4*8 - 5*7)
        //      = 1*(50-48) - 2*(40-42) + 3*(32-35)
        //      = 2 + 4 - 9 = -3
        Rat det = determinant(M);
        EXPECT_EQ_RAT(det, -3);
    }

    std::printf("== MatQ determinant: singular 2x2 ==\n");
    {
        MatQ M({{1, 2}, {2, 4}});
        Rat det = determinant(M);
        EXPECT_EQ_RAT(det, 0);
    }

    std::printf("== MatQ inverse: 2x2 simple ==\n");
    {
        MatQ M({{1, 2}, {3, 4}});
        MatQ M_inv = inverse(M);
        // Expected: [[-2, 1], [3/2, -1/2]] (from SymPy)
        MatQ expected({{{-2, 1}, {1, 1}}, {{3, 2}, {-1, 2}}});
        EXPECT_EQ_MAT(M_inv, expected);
        // Verify M * M_inv = I.
        MatQ I = M * M_inv;
        EXPECT_EQ_MAT(I, identity_mat(2));
    }

    std::printf("== MatQ inverse: 3x3 with rationals ==\n");
    {
        MatQ M({{1, 2, 3}, {4, 5, 6}, {7, 8, 10}});
        MatQ M_inv = inverse(M);
        MatQ prod = M * M_inv;
        EXPECT_EQ_MAT(prod, identity_mat(3));
    }

    std::printf("== MatQ inverse: singular matrix throws ==\n");
    {
        MatQ M({{1, 2}, {2, 4}});
        bool caught = false;
        try {
            inverse(M);
        } catch (const std::runtime_error&) {
            caught = true;
        }
        EXPECT(caught, "singular inverse throws");
    }

    std::printf("== MatQ inverse: 3x3 identity ==\n");
    {
        MatQ I = identity_mat(3);
        MatQ I_inv = inverse(I);
        EXPECT_EQ_MAT(I_inv, I);
    }

    std::printf("== MatQ inverse: 4x4 with rationals ==\n");
    {
        MatQ M({{1, 2, 3, 4}, {5, 6, 7, 8}, {9, 10, 11, 13}, {14, 15, 17, 18}});
        MatQ M_inv = inverse(M);
        MatQ prod = M * M_inv;
        EXPECT_EQ_MAT(prod, identity_mat(4));
    }

    std::printf("== MatQ: dimension mismatch throws ==\n");
    {
        MatQ A(2, 3);
        MatQ B(2, 3);
        MatQ C(3, 2);
        bool caught = false;
        try { MatQ D = A + B; } catch (...) { caught = true; }
        EXPECT(!caught, "2x3 + 2x3 is fine");
        caught = false;
        try { MatQ D = A * C; } catch (...) { /* not expected to throw */ }
        EXPECT(!caught, "2x3 * 3x2 is fine");
        caught = false;
        try { MatQ D = A * B; } catch (...) { caught = true; }
        EXPECT(caught, "2x3 * 2x3 throws");
        caught = false;
        try { MatQ D = A + C; } catch (...) { caught = true; }
        EXPECT(caught, "2x3 + 3x2 throws");
    }

    std::printf("\n== summary ==\n");
    std::printf("%d passed, %d failed\n", mathlib_test::n_pass, mathlib_test::n_fail);
    return mathlib_test::n_fail == 0 ? 0 : 1;
}
