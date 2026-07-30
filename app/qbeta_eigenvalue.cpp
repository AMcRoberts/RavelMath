// qbeta_eigenvalue.cpp
//
// CLI driver for the exact Q(beta) dominant-eigenvalue computation.
// Reads an integer matrix and a degree-4 monic minimal polynomial
// from argv, invokes the algorithm in src/qbeta_eigenvalue.hpp,
// and prints the converged lambda.
//
// This is the script-form of the qbeta_dominant_eigenvalue_4
// function; scripts/contact_boundary_4x4.cpp calls the same
// function in-process so the dump_gb_matrix -> qbeta_eigenvalue
// pipeline runs end-to-end in one driver invocation.  This CLI
// exists for shell-orchestrated workflows (the historical
// scripts/run_qbeta_survey.sh) and for ad-hoc debugging.
//
// Usage:
//   qbeta_eigenvalue c0 c1 c2 c3 matrix_file [k_max]
//   where the char poly is x^4 + c0 x^3 + c1 x^2 + c2 x + c3 = 0
//   and matrix_file has N on the first line then N lines of N ints.
//   k_max defaults to 80 (matches the original CLI behaviour).

#include <array>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <vector>

#include "ravel/qbeta_eigenvalue.hpp"

int main(int argc, char** argv) {
    if (argc < 5) {
        std::fprintf(stderr,
            "Usage: %s c0 c1 c2 c3 matrix_file [k_max]\n"
            "  matrix_file has N on first line, then N lines of N integers.\n"
            "Char poly: x^4 + c0 x^3 + c1 x^2 + c2 x + c3 = 0\n",
            argv[0]);
        return 1;
    }

    ravel::QBetaCharPoly4 poly;
    for (int i = 0; i < 4; ++i) poly.c[i] = std::atoll(argv[1 + i]);
    const char* matrix_path = argv[5];

    int k_max = (argc > 6) ? std::atoi(argv[6]) : 80;

    std::FILE* f = std::fopen(matrix_path, "r");
    if (!f) { std::perror("fopen"); return 1; }
    int N = 0;
    if (std::fscanf(f, "%d", &N) != 1 || N <= 0) {
        std::fprintf(stderr, "bad file (N=%d)\n", N);
        return 1;
    }
    std::vector<std::vector<long long>> A(N, std::vector<long long>(N));
    for (int i = 0; i < N; ++i) {
        for (int j = 0; j < N; ++j) {
            if (std::fscanf(f, "%lld", &A[i][j]) != 1) {
                std::fprintf(stderr, "short read at (%d,%d)\n", i, j);
                std::fclose(f);
                return 1;
            }
        }
    }
    std::fclose(f);

    ravel::QBetaOptions opts;
    opts.k_max = k_max;

    auto result = ravel::qbeta_dominant_eigenvalue_4(A, poly, opts);
    if (!result.error.empty()) {
        std::fprintf(stderr, "qbeta: %s\n", result.error.c_str());
        return 2;
    }
    std::printf("%.15g\n", result.lambda);
    return 0;
}