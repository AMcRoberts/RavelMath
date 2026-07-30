// contact_boundary_4x4.cpp
//
// Driver: takes a 4x4 substitution matrix as command-line arguments,
// constructs the canonical substitution rule (sorted-by-letter), and
// runs the full contact boundary pipeline.  Optionally, if a
// matrix_path argument is given, also computes the exact Q(β) dominant
// eigenvalue of the G_B adjacency matrix in-process (no fork+exec
// needed; this subsumes the historical dump_gb_matrix -> qbeta_eigenvalue
// pipeline that scripts/run_qbeta_survey.sh used to do in two steps).
//
// Usage:
//   contact_boundary_4x4 M00 M01 M02 M03 M10 M11 M12 M13 M20 M21 M22 M23 M30 M31 M32 M33 [beta] [name] [matrix_path]
//
// If beta is not given, it's computed from the matrix via
// ravel::classify_matrix_spectral (survey.hpp), which as of
// TODO_GENERALIZATION.md item 3 is the single shared spectral path
// for every alphabet size -- this file no longer carries its own
// private copy of the power iteration.
// If name is not given, defaults to "candidate".
// If matrix_path is not given, the exact Q(β) eigenvalue step is
// skipped (only the double-precision lambda(G_B) from the pipeline
// is reported).

#include <array>
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <string>
#include <vector>

#include "ravel/ambient_graph.hpp"
#include "ravel/balanced_pair.hpp"
#include "ravel/barge.hpp"
#include "ravel/contact_boundary.hpp"
#include "ravel/core.hpp"
#include "ravel/corona.hpp"
#include "ravel/d_cont_check.hpp"
#include "ravel/faces.hpp"
#include "ravel/qbeta_eigenvalue.hpp"
#include "ravel/substitution.hpp"
#include "ravel/survey.hpp"

namespace {

std::vector<std::vector<std::int8_t>>
matrix_to_rule(const long long M[4][4]) {
    std::vector<std::vector<std::int8_t>> sigma(4);
    for (int c = 0; c < 4; ++c) {
        for (int letter = 0; letter < 4; ++letter) {
            for (long long k = 0; k < M[letter][c]; ++k) {
                sigma[c].push_back(static_cast<std::int8_t>(letter));
            }
        }
    }
    return sigma;
}

long long det_4x4(const long long M[4][4]) {
    long long d = 0;
    for (int j = 0; j < 4; ++j) {
        long long sub[3][3];
        int sr = 0;
        for (int r = 1; r < 4; ++r) {
            int sc = 0;
            for (int c = 0; c < 4; ++c) {
                if (c == j) continue;
                sub[sr][sc] = M[r][c];
                ++sc;
            }
            ++sr;
        }
        long long sgn = (j % 2 == 0) ? 1 : -1;
        d += sgn * M[0][j] *
             (sub[0][0] * (sub[1][1] * sub[2][2] - sub[1][2] * sub[2][1])
            - sub[0][1] * (sub[1][0] * sub[2][2] - sub[1][2] * sub[2][0])
            + sub[0][2] * (sub[1][0] * sub[2][1] - sub[1][1] * sub[2][0]));
    }
    return d;
}

// beta from the matrix, via the shared spectral path (survey.hpp),
// which now handles n=4 (see TODO_GENERALIZATION.md items 1-3).
double compute_beta(const long long M[4][4]) {
    std::vector<std::vector<long long>> Mvec(4, std::vector<long long>(4));
    for (int i = 0; i < 4; ++i)
        for (int j = 0; j < 4; ++j)
            Mvec[i][j] = M[i][j];
    return ravel::classify_matrix_spectral(Mvec).beta;
}

ravel::ContactBoundaryReport
run_pipeline(const long long M[4][4], double beta, const std::string& name,
            const std::string& matrix_path) {
    using namespace ravel;
    auto sigma = matrix_to_rule(M);
    SubstitutionRule rule(sigma);

    // Compute D_cont geometrically.
    std::vector<DCandidate<4>> d_cont_cands =
        search_D_cont<4>(make_substitution<4>(rule, beta), /*bound=*/2);

    // Convert DCandidate to the triple format expected by
    // compute_contact_boundary.
    std::vector<std::tuple<long long, std::vector<long long>, long long>>
        d_cont;
    for (const auto& c : d_cont_cands) {
        std::vector<long long> xvec(c.x.begin(), c.x.end());
        d_cont.emplace_back(c.i, std::move(xvec), c.j);
    }

    std::printf("[%s] beta=%.6f, |det|=%lld, |D_cont (bound=2)|=%zu\n",
                name.c_str(), beta,
                std::llabs(det_4x4(M)), d_cont.size());

    // Run the contact boundary pipeline.
    auto rep = compute_contact_boundary_dispatch(rule, beta, 0.0, d_cont);

    // Dump G_B adjacency matrix if requested.  After dumping,
    // compute its exact spectral radius via the in-process
    // qbeta_dominant_eigenvalue_4 path (degree-4 char poly of the
    // substitution matrix defines Q(beta), then we run power
    // iteration on the G_B adjacency matrix in Q(beta) arithmetic).
    // This is the in-process dump_gb_matrix -> qbeta_eigenvalue
    // pipeline that scripts/run_qbeta_survey.sh now drives by
    // calling this binary once per candidate (instead of doing the
    // two-step dump-then-invoke in shell).
    if (!matrix_path.empty() && !rep.gb_matrix.empty()) {
        std::ofstream f(matrix_path);
        f << rep.gb_matrix.size() << "\n";
        for (const auto& row : rep.gb_matrix) {
            for (std::size_t j = 0; j < row.size(); ++j) {
                if (j > 0) f << " ";
                f << row[j];
            }
            f << "\n";
        }

        // Char poly of the substitution matrix (det(xI - M)); the
        // first 4 lower coefficients feed QBetaCharPoly4.
        std::vector<std::vector<long long>> Mvec(4, std::vector<long long>(4));
        for (int i = 0; i < 4; ++i)
            for (int j = 0; j < 4; ++j)
                Mvec[i][j] = M[i][j];
        std::vector<long long> cp = ravel::charpoly_int(Mvec);
        if (cp.size() >= 5) {
            ravel::QBetaCharPoly4 poly;
            poly.c = {cp[1], cp[2], cp[3], cp[4]};
            auto qresult = ravel::qbeta_dominant_eigenvalue_4(
                rep.gb_matrix, poly);
            std::printf("[%s] qbeta(G_B): ", name.c_str());
            if (!qresult.error.empty()) {
                std::printf("error: %s\n", qresult.error.c_str());
            } else {
                std::printf("lambda=%.6f (steps=%d, peak=%zu bytes)\n",
                            qresult.lambda, qresult.steps_taken,
                            qresult.peak_memory_bytes);
            }
        }
    }

    return rep;
}

}  // namespace

int main(int argc, char** argv) {
    if (argc < 17) {
        std::fprintf(stderr,
            "Usage: %s M00 M01 M02 M03 M10 M11 M12 M13 "
            "M20 M21 M22 M23 M30 M31 M32 M33 [beta] [name] [matrix_path]\n",
            argv[0]);
        return 1;
    }

    long long M[4][4];
    int k = 1;
    for (int i = 0; i < 4; ++i)
        for (int j = 0; j < 4; ++j)
            M[i][j] = std::atoll(argv[k++]);

    double beta = (argc > 17) ? std::atof(argv[17])
                                : compute_beta(M);
    std::string name = (argc > 18) ? argv[18] : "candidate";
    std::string matrix_path = (argc > 19) ? argv[19] : "";

    auto t0 = std::chrono::steady_clock::now();
    ravel::ContactBoundaryReport rep;
    try {
        rep = run_pipeline(M, beta, name, matrix_path);
    } catch (const std::exception& e) {
        std::printf("[%s] FAILED: %s\n", name.c_str(), e.what());
        return 1;
    }
    auto t1 = std::chrono::steady_clock::now();
    double seconds = std::chrono::duration<double>(t1 - t0).count();

    std::printf("    |G_P|=%zu  |C|=%zu  |±C|=%zu  |G_B|=%zu  "
                "converged=%d rounds=%d  closure_stopped=%d  "
                "corona_capped=%d (max_A=%zu)\n",
                rep.pre_contact_size, rep.contact_size,
                rep.signed_contact_size, rep.boundary_size,
                rep.converged ? 1 : 0, rep.convergence_rounds,
                rep.closure_stopped_early ? 1 : 0,
                rep.corona_capped ? 1 : 0, rep.max_a_size_reached);
    std::printf("    BP-rho_nc=%.6f  lambda(G_B)=%.6f  "
                "elapsed=%.2fs\n",
                rep.bp_rho_nc, rep.boundary_eigenvalue, seconds);
    std::printf("    status: %s\n", rep.conjecture_status.c_str());
    return 0;
}
