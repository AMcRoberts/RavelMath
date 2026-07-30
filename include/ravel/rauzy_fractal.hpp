// rauzy_fractal.hpp
//
// Construct the Rauzy fractal of a primitive Pisot substitution by
// chaos game on its graph-directed IFS.  Uses the standard Rauzy 1988
// formulation with the canonical partial-sum / displacement convention.
//
// The Rauzy fractal T is the unique compact set in ℝ^d satisfying:
//   T = ⋃_{i ∈ A} (M^{-1}(T) + d_i)
// where d_i is the displacement vector of letter i (the sum of the
// i-th standard basis vectors over σ(i)).  This IFS contracts in the
// d-1 directions orthogonal to the LEFT Perron eigenvector u, so the
// orbit converges to T.
//
// For Pisot substitutions, T tiles ℝ^{d-1} (the contracting subspace)
// by translation; for non-Pisot, the tiling property may fail.
//
// Reference: Rauzy, "Nombres algébriques et substitutions" (1982);
// Sirvent's book on Rauzy fractals.

#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <vector>

namespace ravel {

template <std::size_t d>
class RauzyFractal {
public:
    using Image = std::vector<std::int8_t>;
    using ImageList = std::array<Image, d>;
    using Vec = std::array<double, d>;
    using Matrix = std::array<std::array<double, d>, d>;

    // Build from per-letter images: sigma[c] is a list of letter
    // indices (the image of letter c).
    RauzyFractal(const ImageList& images);

    // Apply the affine IFS map induced by M on the contracting
    // hyperplane, before projection to W coordinates.
    Vec apply_psi_raw(const Vec& x, std::size_t i) const;

    // Run the chaos game: start at origin, apply ψ_i with i chosen
    // by Perron right eigenvector weights.  Returns n points in ℝ^d
    // (project to W via project_to_W() for visualization).
    std::vector<Vec> chaos_game(std::size_t n_points,
                                std::uint32_t seed = 42) const;

    // Project x onto the contracting subspace W = u⊥ (a linear
    // subspace passing through origin).  Returns the (d-1)-dimensional
    // coordinates of x in a chosen W basis.
    Vec project_to_W(const Vec& x) const;

    // Project to 2D for visualization: returns the first 2 of the d-1
    // basis-vector coordinates.
    std::array<double, 2> project_2d(const Vec& x) const;

    // Estimate the box dimension of the fractal by box-counting the
    // chaos-game orbit.  Returns the dimension estimate.
    double box_dimension(int n_grid_levels = 8) const;

    // Test tiling: bin the chaos-game points into unit cells, report
    // the fraction of cells visited and the average occupancy.  For
    // a perfect tiler with non-overlapping tiles, the expected
    // occupancy equals the measure of T.  For a non-tiler, the orbit
    // is not translation-invariant and the density distribution is
    // non-uniform.
    struct TilingStats {
        std::size_t n_cells_visited;
        std::size_t n_cells_total;
        double mean_occupancy;
        double max_occupancy;
        double cell_size;
        double estimated_measure;  // measure ~ n_cells_visited * cell_size^d
    };
    TilingStats tiling_stats(int grid_n = 50) const;

    // Accessors.
    double beta() const { return beta_; }
    const Matrix& M() const { return M_; }
    const Matrix& M_inv() const { return M_inv_; }
    const std::array<double, d>& u() const { return u_; }  // left Perron, u_0 = 1
    const std::array<double, d>& v() const { return v_; }  // right Perron, v_0 = 1
    const Vec& displacement(std::size_t i) const { return d_[i]; }

private:
    double beta_;
    Matrix M_;
    Matrix M_inv_;
    std::array<double, d> u_;     // left Perron eigenvector
    std::array<double, d> v_;     // right Perron eigenvector
    std::array<Vec, d> d_;       // displacement vectors
    // Basis of W = u⊥ (d-1 orthonormal vectors in ℝ^d).
    std::array<Vec, d - 1> w_basis_;
};

}  // namespace ravel
