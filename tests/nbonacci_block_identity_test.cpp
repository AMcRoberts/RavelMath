// Exact n+1-step carry identity.
//
// If A=M^{-1} is the homogeneous n-bonacci carry map, then
//     A^(n+1) = 2 A - I.
// The digit forcing over the same block is
//   (-e0+2e_last) d0 + sum_{r=1}^{n-1}(e_{r-1}-e_r) d_r
//                         + e_last d_n.
// This test checks both formulas with arbitrary signed digits, independently
// of the beta-window and corona code.

#include <cstdio>
#include <stdexcept>
#include <vector>

using Scalar = long long;
using Matrix = std::vector<std::vector<Scalar>>;
using Vector = std::vector<Scalar>;

Matrix multiply(const Matrix& lhs, const Matrix& rhs) {
    const std::size_t n = lhs.size();
    Matrix result(n, std::vector<Scalar>(n));
    for (std::size_t i = 0; i < n; ++i)
        for (std::size_t k = 0; k < n; ++k)
            for (std::size_t j = 0; j < n; ++j)
                result[i][j] += lhs[i][k] * rhs[k][j];
    return result;
}

void add_in_place(Matrix& target, const Matrix& source) {
    for (std::size_t i = 0; i < target.size(); ++i)
        for (std::size_t j = 0; j < target.size(); ++j)
            target[i][j] += source[i][j];
}

Vector multiply(const Matrix& matrix, const Vector& vector) {
    Vector result(matrix.size());
    for (std::size_t i = 0; i < matrix.size(); ++i)
        for (std::size_t j = 0; j < vector.size(); ++j)
            result[i] += matrix[i][j] * vector[j];
    return result;
}

Matrix carry_matrix(std::size_t n) {
    Matrix matrix(n, std::vector<Scalar>(n));
    for (std::size_t j = 0; j < n; ++j) matrix[0][j] = 1;
    for (std::size_t j = 0; j + 1 < n; ++j) matrix[j + 1][j] = 1;
    return matrix;
}

Matrix inverse_carry_matrix(std::size_t n) {
    Matrix matrix(n, std::vector<Scalar>(n));
    for (std::size_t j = 0; j + 1 < n; ++j) matrix[j][j + 1] = 1;
    matrix[n - 1][0] = 1;
    for (std::size_t j = 1; j < n; ++j) matrix[n - 1][j] = -1;
    return matrix;
}

void require(bool condition, const char* message) {
    if (!condition) throw std::runtime_error(message);
}

int main() {
    std::size_t checks = 0;
    for (std::size_t n = 2; n <= 40; ++n) {
        const Matrix A = inverse_carry_matrix(n);
        Matrix geometric(n, std::vector<Scalar>(n));
        Matrix running(n, std::vector<Scalar>(n));
        for (std::size_t i = 0; i < n; ++i) running[i][i] = 1;
        for (std::size_t k = 1; k <= n; ++k) {
            running = multiply(running, A);
            add_in_place(geometric, running);
        }
        Matrix identity(n, std::vector<Scalar>(n));
        for (std::size_t i = 0; i < n; ++i) identity[i][i] = 1;
        require(geometric == identity,
                "geometric sum A+...+A^n != I");

        Matrix power(n, std::vector<Scalar>(n));
        for (std::size_t i = 0; i < n; ++i) power[i][i] = 1;
        for (std::size_t k = 0; k < n + 1; ++k) power = multiply(power, A);

        Matrix expected(n, std::vector<Scalar>(n));
        for (std::size_t i = 0; i < n; ++i)
            for (std::size_t j = 0; j < n; ++j)
                expected[i][j] = 2 * A[i][j] - (i == j ? 1 : 0);
        require(power == expected, "A^(n+1) != 2A-I");

        Vector last(n); last[n - 1] = 1;
        for (std::size_t r = 0; r <= n; ++r) {
            Matrix ar(n, std::vector<Scalar>(n));
            for (std::size_t i = 0; i < n; ++i) ar[i][i] = 1;
            for (std::size_t k = 0; k < n - r; ++k) ar = multiply(ar, A);
            const Vector actual = multiply(ar, last);
            Vector formula(n);
            if (r == 0) {
                formula[0] = -1;
                formula[n - 1] += 2;
            } else if (r < n) {
                formula[r - 1] = 1;
                formula[r] = -1;
            } else {
                formula[n - 1] = 1;
            }
            require(actual == formula, "n+1-step digit forcing mismatch");
            ++checks;
        }
    }
    std::printf("nbonacci_block_identity_test: %zu checks, 0 failures\n", checks);
    return 0;
}
