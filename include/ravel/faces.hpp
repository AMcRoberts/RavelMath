// faces.hpp
//
// Face-intersection-dimension geometry for the stepped hypersurface
// H_sigma (Eq 2.2 in the November-2025 paper).  A face [x, i] is
// the (d-1)-cube
//
//   [x, i] = { x + Sum_{k != i} theta_k e_k : theta_k in [0,1) for k != i }
//
// so its extent in direction i is the single point {x[i]}, and its
// extent in every other direction k != i is the unit interval
// [x[k], x[k] + 1).
//
// We compute the dimension of the intersection [x_1, i_1] ∩ [x_2, i_2]
// via 1D interval products (per face_extent and interval_intersection).
// Returns the dimension d as an integer; -1 if the intersection is empty.
//
// The implementation here uses integer arithmetic; all
// intersection dimensions are exact in Z, no floating point.

#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <utility>

namespace ravel {

// One-dimensional closed interval [lo, hi] as a pair of
// long-longs.  Always lo <= hi; represents a point when lo == hi.
struct IInterval {
    long long lo, hi;
    bool is_point() const { return lo == hi; }
};

// Extent of face [x, i] in direction k.  For k == i the extent is
// a single point; for k != i the extent is the unit interval
// [x[k], x[k] + 1).
template <std::size_t d>
IInterval face_extent(const std::array<long long, d>& x,
                      std::size_t i, std::size_t k) {
    if (k == i) return { x[k], x[k] };
    return { x[k], x[k] + 1 };
}

inline std::pair<bool, IInterval>
interval_intersection(const IInterval& a, const IInterval& b) {
    long long lo = std::max(a.lo, b.lo);
    long long hi = std::min(a.hi, b.hi);
    if (lo > hi) return { false, {0, 0} };
    return { true, {lo, hi} };
}

// Dimension of the intersection of face [x_1, i_1] with face
// [x_2, i_2].  Returns the dimension as an integer; returns -1
// if the intersection is empty.  The dimension equals the count
// of coordinate directions k in which the 1D interval
// intersection is a point (i.e. is_positive + (lo == hi) for
// the standard 1D case where a face extent in a direction is
// either a unit interval [a, a+1) or a single point {a}).
//
// The November-2025 paper's D_cont table is the set of triples
// (i, x, j) such that:
//   - x is in H_sigma
//   - the intersection [x, i] ∩ [x, j] has dimension exactly
//     (d - 2)
//   - if x = 0, then i < j (to avoid double-counting the pair
//     (i, 0, j) and (j, 0, i)).
//
// The condition "intersection dimension == d - 2" is equivalent
// to: there are exactly 2 directions k in which the 1D intervals
// intersect at a single point (the other d - 2 directions either
// have empty intersection or both ranges are intervals that
// overlap in an interval).  For Pisot matrices, the standard case
// is: k = i (single point) and k = j (single point) are the
// two point-intersection directions; all other k in {0..d-1} except
// {i, j} have empty intersection.
template <std::size_t d>
int face_intersection_dim(const std::array<long long, d>& x_1,
                          std::size_t i_1,
                          const std::array<long long, d>& x_2,
                          std::size_t i_2) {
    int positive_length = 0;
    for (std::size_t k = 0; k < d; ++k) {
        IInterval e1 = face_extent<d>(x_1, i_1, k);
        IInterval e2 = face_extent<d>(x_2, i_2, k);
        auto [nonempty, inter] = interval_intersection(e1, e2);
        if (!nonempty) {
            // Box intersection is empty if ANY coordinate
            // intersection is empty (the two boxes don't overlap
            // in that direction).
            return -1;
        }
        if (!inter.is_point()) {
            // Positive-length interval: contributes 1 to dim.
            ++positive_length;
        }
        // For our Pisot substitution case, the intersection of two
        // faces is either a single point (0-D) or a non-point
        // segment (1-D), never a 2D region; so we only count
        // positive-length intervals.
    }
    return positive_length;
}

// D_cont face intersection dimension (the SPECIFIC filter):
// the dimension of the intersection of the origin face [0, i]
// with the x-shifted face [x, j].  Used to test whether
// (i, x, j) is in D_cont:
// in_D_cont iff in_H_sigma([x, j]) AND B_intersection_dim == d - 2
// AND (x != 0 OR i < j).
//
// The "d - 2" rule comes from the November-2025 paper: a
// (d-2)-dimensional face contact is the right object to look at.
// For d = 3 this is a 1-D (line) intersection.
template <std::size_t d>
int d_cont_face_intersection_dim(const std::array<long long, d>& x,
                                 std::size_t i, std::size_t j) {
    std::array<long long, d> zero{};
    return face_intersection_dim<d>(zero, i, x, j);
}

}  // namespace ravel
