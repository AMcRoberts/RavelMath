// rauzy_fractal.cpp
//
// Author: Ravel
// Coordinator and project architect: AM
//
// Implementation of RauzyFractal<d>.  See rauzy_fractal.hpp.

#include "ravel/rauzy_fractal.hpp"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <map>
#include <random>
#include <set>
#include <string>

namespace ravel {

namespace {

// Compute the inverse of a small dense matrix via Gaussian elimination.
// Caller must pre-allocate inv with the same size.
template <std::size_t d>
void invert_matrix(const std::array<std::array<double, d>, d>& A,
                  std::array<std::array<double, d>, d>* inv) {
    // Build augmented matrix [A | I] and reduce.
    std::array<std::array<double, 2 * d>, d> aug{};
    for (std::size_t i = 0; i < d; ++i) {
        for (std::size_t j = 0; j < d; ++j) aug[i][j] = A[i][j];
        for (std::size_t j = 0; j < d; ++j) aug[i][d + j] = (i == j) ? 1.0 : 0.0;
    }
    // Forward elimination with partial pivoting.
    for (std::size_t k = 0; k < d; ++k) {
        // Pivot: find largest |aug[p][k]| for p >= k.
        std::size_t pivot = k;
        double max_abs = std::fabs(aug[k][k]);
        for (std::size_t p = k + 1; p < d; ++p) {
            if (std::fabs(aug[p][k]) > max_abs) {
                max_abs = std::fabs(aug[p][k]);
                pivot = p;
            }
        }
        if (max_abs < 1e-15) {
            std::fprintf(stderr, "matrix is singular\n");
            return;
        }
        if (pivot != k) std::swap(aug[k], aug[pivot]);
        // Eliminate below.
        for (std::size_t p = k + 1; p < d; ++p) {
            double factor = aug[p][k] / aug[k][k];
            if (factor != 0.0) {
                for (std::size_t j = k; j < 2 * d; ++j) {
                    aug[p][j] -= factor * aug[k][j];
                }
            }
        }
    }
    // Back substitution.
    for (std::size_t k = d; k-- > 0;) {
        double diag = aug[k][k];
        for (std::size_t j = k; j < 2 * d; ++j) aug[k][j] /= diag;
        for (std::size_t p = 0; p < k; ++p) {
            double factor = aug[p][k];
            for (std::size_t j = k; j < 2 * d; ++j) {
                aug[p][j] -= factor * aug[k][j];
            }
        }
    }
    for (std::size_t i = 0; i < d; ++i) {
        for (std::size_t j = 0; j < d; ++j) (*inv)[i][j] = aug[i][d + j];
    }
}

// Power iteration to find the right Perron eigenvector of M.
// Returns v such that Mv = β v, normalized v[0] = 1.
template <std::size_t d>
std::array<double, d> right_perron(const std::array<std::array<double, d>, d>& M,
                                   double* beta_out, int max_iter = 5000) {
    std::array<double, d> v{};
    v[0] = 1.0;
    for (int it = 0; it < max_iter; ++it) {
        std::array<double, d> w{};
        // w = M · v (RIGHT eigenvector): w[i] = sum_j M[i][j] v[j]
        for (std::size_t i = 0; i < d; ++i) {
            double s = 0;
            for (std::size_t j = 0; j < d; ++j) s += M[i][j] * v[j];
            w[i] = s;
        }
        // Normalize so w[0] = v[0] = 1.
        if (w[0] == 0.0) {
            for (std::size_t i = 0; i < d; ++i) v[i] = 1.0 / d;
        } else {
            for (std::size_t i = 0; i < d; ++i) v[i] = w[i] / w[0];
        }
    }
    // Final multiplication to get β.
    std::array<double, d> w{};
    for (std::size_t i = 0; i < d; ++i) {
        double s = 0;
        for (std::size_t j = 0; j < d; ++j) s += M[i][j] * v[j];
        w[i] = s;
    }
    // β = (v · w) / (v · v) (Rayleigh quotient).
    double vv = 0, vw = 0;
    for (std::size_t i = 0; i < d; ++i) { vv += v[i] * v[i]; vw += v[i] * w[i]; }
    *beta_out = vv > 0 ? vw / vv : 1.0;
    return v;
}

// Power iteration to find the left Perron eigenvector of M (so M^T u = β u),
// normalized u[0] = 1.
template <std::size_t d>
std::array<double, d> left_perron(const std::array<std::array<double, d>, d>& M,
                                  int max_iter = 5000) {
    std::array<double, d> u{};
    u[0] = 1.0;
    // Iteration: u_{k+1} = M u_k / (M u_k)[0] (works on M, not M^T, by
    // direct iteration on the right eigenvector of M^T).
    // Actually for the LEFT eigenvector, we use M^T in the iteration.
    std::array<std::array<double, d>, d> Mt{};
    for (std::size_t i = 0; i < d; ++i)
        for (std::size_t j = 0; j < d; ++j) Mt[i][j] = M[j][i];
    for (int it = 0; it < max_iter; ++it) {
        std::array<double, d> w{};
        for (std::size_t i = 0; i < d; ++i) {
            double s = 0;
            for (std::size_t j = 0; j < d; ++j) s += Mt[i][j] * u[j];
            w[i] = s;
        }
        if (w[0] == 0.0) {
            for (std::size_t i = 0; i < d; ++i) u[i] = 1.0 / d;
        } else {
            for (std::size_t i = 0; i < d; ++i) u[i] = w[i] / w[0];
        }
    }
    return u;
}

// Build an orthonormal basis for W = u⊥ (the contracting hyperplane).
// Uses Gram-Schmidt on a set of d vectors that span the orthogonal
// complement of u.
template <std::size_t d>
std::array<std::array<double, d>, d - 1> orthonormal_basis_w(
        const std::array<double, d>& u) {
    // Start with standard basis vectors, projected away from u.
    std::array<std::array<double, d>, d - 1> basis{};
    std::size_t idx = 0;
    for (std::size_t i = 0; i < d && idx < d - 1; ++i) {
        std::array<double, d> e_i{};
        e_i[i] = 1.0;
        // Project: e_i - <e_i, u>/<u, u> · u.
        double eiu = e_i[i] * u[i];
        double uu = 0;
        for (std::size_t k = 0; k < d; ++k) uu += u[k] * u[k];
        for (std::size_t k = 0; k < d; ++k) e_i[k] -= (eiu / uu) * u[k];
        double norm2 = 0;
        for (std::size_t k = 0; k < d; ++k) norm2 += e_i[k] * e_i[k];
        if (norm2 < 1e-15) continue;
        double norm = std::sqrt(norm2);
        for (std::size_t k = 0; k < d; ++k) basis[idx][k] = e_i[k] / norm;
        // Orthogonalize against previous basis vectors.
        for (std::size_t j = 0; j < idx; ++j) {
            double dp = 0;
            for (std::size_t k = 0; k < d; ++k) dp += basis[idx][k] * basis[j][k];
            for (std::size_t k = 0; k < d; ++k) basis[idx][k] -= dp * basis[j][k];
        }
        // Re-normalize.
        norm2 = 0;
        for (std::size_t k = 0; k < d; ++k) norm2 += basis[idx][k] * basis[idx][k];
        norm = std::sqrt(norm2);
        for (std::size_t k = 0; k < d; ++k) basis[idx][k] /= norm;
        idx++;
    }
    return basis;
}

}  // namespace

template <std::size_t d>
RauzyFractal<d>::RauzyFractal(const ImageList& images) {
    // Build M[i][j] = count of letter i in σ(j).
    for (std::size_t i = 0; i < d; ++i)
        for (std::size_t j = 0; j < d; ++j) M_[i][j] = 0;
    for (std::size_t j = 0; j < d; ++j) {
        for (auto letter : images[j]) {
            M_[static_cast<std::size_t>(letter)][j] += 1;
        }
    }

    // Find β via right power iteration.
    v_ = right_perron<d>(M_, &beta_);

    // Find u (left Perron).
    u_ = left_perron<d>(M_);

    // Compute M^{-1}.
    invert_matrix<d>(M_, &M_inv_);

    // Compute displacement vectors d_i.
    for (std::size_t i = 0; i < d; ++i) {
        Vec di{};
        for (auto letter : images[i]) di[static_cast<std::size_t>(letter)] += 1;
        d_[i] = di;
    }

    // Build orthonormal basis for W = u⊥.
    w_basis_ = orthonormal_basis_w<d>(u_);
}

template <std::size_t d>
typename RauzyFractal<d>::Vec RauzyFractal<d>::apply_psi_raw(
        const Vec& x, std::size_t i) const {
    // IFS map on the AFFINE hyperplane H = {<x, u> = c} where
    // c = <v, u> / (β - 1).  The map is
    //   ψ_i(x) = Mx + d_i + γ_i · u.
    // M restricted to the contracting hyperplane has precisely the
    // non-Perron conjugates as eigenvalues, all of modulus < 1 for a
    // Pisot substitution.  Using M^{-1} here would expand those
    // directions and make the chaos game diverge.
    Vec result{};
    for (std::size_t r = 0; r < d; ++r) {
        double s = 0;
        for (std::size_t col = 0; col < d; ++col) {
            s += M_[r][col] * x[col];
        }
        result[r] = s + d_[i][r];
    }
    // Compute γ_i.
    double uu = 0, vu = 0;
    for (std::size_t k = 0; k < d; ++k) { uu += u_[k] * u_[k]; vu += v_[k] * u_[k]; }
    double c_height = vu / (beta_ - 1.0);
    // Compute the correction from the actual floating-point result, not
    // from the symbolic assumption <x,u> == c.  Otherwise roundoff in the
    // Perron direction is multiplied by beta on every iteration and can
    // eventually overflow during long chaos-game runs.
    double result_height = 0;
    for (std::size_t k = 0; k < d; ++k) {
        result_height += result[k] * u_[k];
    }
    double gamma = (c_height - result_height) / uu;
    // Apply γ correction.
    for (std::size_t k = 0; k < d; ++k) result[k] += gamma * u_[k];
    return result;
}

template <std::size_t d>
std::vector<typename RauzyFractal<d>::Vec> RauzyFractal<d>::chaos_game(
        std::size_t n_points, std::uint32_t seed) const {
    // Choose letters with probability proportional to v (right Perron).
    std::vector<double> cum(d);
    double total = 0;
    for (std::size_t i = 0; i < d; ++i) {
        total += v_[i];
        cum[i] = total;
    }
    // Normalize.
    for (std::size_t i = 0; i < d; ++i) cum[i] /= total;

    std::mt19937 rng(seed);
    std::uniform_real_distribution<double> dist(0.0, 1.0);

    // Initialize at the fixed-point height c · u / <u, u> in the
    // affine hyperplane (so we don't waste burn-in drifting).
    double uu = 0, vu = 0;
    for (std::size_t k = 0; k < d; ++k) { uu += u_[k] * u_[k]; vu += v_[k] * u_[k]; }
    double c_height = vu / (beta_ - 1.0);
    Vec x{};
    for (std::size_t k = 0; k < d; ++k) x[k] = c_height * u_[k] / uu;

    std::vector<Vec> out;
    out.reserve(n_points);
    // Burn-in.
    for (std::size_t k = 0; k < 200; ++k) {
        double u = dist(rng);
        std::size_t i = 0;
        while (i + 1 < d && u > cum[i]) ++i;
        x = apply_psi_raw(x, i);
    }
    for (std::size_t k = 0; k < n_points; ++k) {
        double u = dist(rng);
        std::size_t i = 0;
        while (i + 1 < d && u > cum[i]) ++i;
        x = apply_psi_raw(x, i);
        out.push_back(x);
    }
    return out;
}

template <std::size_t d>
typename RauzyFractal<d>::Vec RauzyFractal<d>::project_to_W(const Vec& x) const {
    // First project to W by subtracting the u-component.
    double xu = 0, uu = 0;
    for (std::size_t k = 0; k < d; ++k) { xu += x[k] * u_[k]; uu += u_[k] * u_[k]; }
    double t = xu / uu;
    Vec xw = x;
    for (std::size_t k = 0; k < d; ++k) xw[k] -= t * u_[k];
    // Then express in the W basis.
    Vec coords{};
    for (std::size_t j = 0; j < d - 1; ++j) {
        double s = 0;
        for (std::size_t k = 0; k < d; ++k) s += xw[k] * w_basis_[j][k];
        coords[j] = s;
    }
    return coords;
}

template <std::size_t d>
std::array<double, 2> RauzyFractal<d>::project_2d(const Vec& x) const {
    Vec w = project_to_W(x);
    return {w[0], w[1]};
}

template <std::size_t d>
double RauzyFractal<d>::box_dimension(int n_grid_levels) const {
    // Generate chaos game orbit and box-count.
    auto pts = chaos_game(50000, 42);
    // Project to W coords.
    std::vector<Vec> wpts;
    wpts.reserve(pts.size());
    for (const auto& p : pts) wpts.push_back(project_to_W(p));
    // Find the bounding box.
    Vec lo{}, hi{};
    for (std::size_t k = 0; k < d - 1; ++k) { lo[k] = 1e18; hi[k] = -1e18; }
    for (const auto& p : wpts) {
        for (std::size_t k = 0; k < d - 1; ++k) {
            if (p[k] < lo[k]) lo[k] = p[k];
            if (p[k] > hi[k]) hi[k] = p[k];
        }
    }
    // Box-counting: for each grid level, count distinct cells visited.
    std::vector<std::pair<int, std::size_t>> log_data;
    for (int level = 2; level <= (1 << n_grid_levels); level <<= 1) {
        std::set<long long> cells;
        double cell_size = 0;
        for (std::size_t k = 0; k < d - 1; ++k) {
            double s = (hi[k] - lo[k]) / level;
            if (s > cell_size) cell_size = s;
        }
        // Use a simple hash: (cell_x_0, cell_x_1, ..., cell_x_{d-2}).
        // For large levels the cells might overflow; use a string key.
        std::set<std::string> cells_str;
        for (const auto& p : wpts) {
            std::string key;
            for (std::size_t k = 0; k < d - 1; ++k) {
                int cell = static_cast<int>((p[k] - lo[k]) / ((hi[k] - lo[k]) / level + 1e-15));
                key += std::to_string(cell) + ",";
            }
            cells_str.insert(key);
        }
        // Once the number of occupied boxes approaches the sample count,
        // the estimate measures sampling sparsity rather than geometry and
        // biases the slope downward.  Retain levels with at least about 20
        // orbit samples per occupied box.
        if (cells_str.size() <= wpts.size() / 20) {
            log_data.push_back({level, cells_str.size()});
        }
    }
    // Fit log(N) vs log(1/eps).
    double sum_x = 0, sum_y = 0, sum_xx = 0, sum_xy = 0;
    int n = 0;
    for (const auto& [level, count] : log_data) {
        double x = std::log(static_cast<double>(level));
        double y = std::log(static_cast<double>(count));
        sum_x += x; sum_y += y; sum_xx += x * x; sum_xy += x * y; n++;
    }
    if (n < 2) return 0;
    return (n * sum_xy - sum_x * sum_y) / (n * sum_xx - sum_x * sum_x);
}

template <std::size_t d>
typename RauzyFractal<d>::TilingStats RauzyFractal<d>::tiling_stats(
        int grid_n) const {
    auto pts = chaos_game(50000, 42);
    std::vector<Vec> wpts;
    wpts.reserve(pts.size());
    for (const auto& p : pts) wpts.push_back(project_to_W(p));
    Vec lo{}, hi{};
    for (std::size_t k = 0; k < d - 1; ++k) { lo[k] = 1e18; hi[k] = -1e18; }
    for (const auto& p : wpts) {
        for (std::size_t k = 0; k < d - 1; ++k) {
            if (p[k] < lo[k]) lo[k] = p[k];
            if (p[k] > hi[k]) hi[k] = p[k];
        }
    }
    double cell_size = 0;
    for (std::size_t k = 0; k < d - 1; ++k) {
        double s = (hi[k] - lo[k]) / grid_n;
        if (s > cell_size) cell_size = s;
    }
    // Bin points into cells.
    std::map<std::string, int> counts;
    for (const auto& p : wpts) {
        std::string key;
        for (std::size_t k = 0; k < d - 1; ++k) {
            int cell = static_cast<int>((p[k] - lo[k]) / (cell_size + 1e-15));
            key += std::to_string(cell) + ",";
        }
        counts[key]++;
    }
    TilingStats s;
    s.n_cells_visited = counts.size();
    s.n_cells_total = 1;
    for (std::size_t k = 0; k < d - 1; ++k) s.n_cells_total *= grid_n;
    long long total = 0; int mx = 0;
    for (const auto& [_, c] : counts) { total += c; if (c > mx) mx = c; }
    s.mean_occupancy = counts.empty() ? 0 : (double)total / counts.size();
    s.max_occupancy = mx;
    s.cell_size = cell_size;
    // Estimate measure: n_cells_visited * cell_size^{d-1}.
    double vol = 1;
    for (std::size_t k = 0; k < d - 1; ++k) vol *= cell_size;
    s.estimated_measure = s.n_cells_visited * vol;
    return s;
}

// Explicit instantiations.
template class RauzyFractal<3>;
template class RauzyFractal<4>;

}  // namespace ravel
