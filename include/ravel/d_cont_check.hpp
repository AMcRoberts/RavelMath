// d_cont_check.hpp
//
// Verify the corrected D_cont tables from the November-2025 paper
// (per the corrected paper at refs/corrected_reference_arxiv_2511.16442.pdf)
// against the geometric definition: a triple (i, x, j) is in D_cont
// iff
//   1. [x, j] lies in H_sigma (0 <= <x, v> < v[j]),
//   2. the faces [x, i] and [x, j] intersect in exactly d-2 dimensions,
//   3. if x = 0, then i < j (to avoid double-counting the mirror
//      (i, 0, j) == (j, 0, i)).
//
// The local C++ implementation uses the math library's exact
// Q(β) in_H_sigma (per core.hpp's in_H_sigma_exact, which uses
// the Sturm-isolated β interval + exact Q(β) sign test) and
// integer arithmetic for the face-intersection dimension.  This
// matches the reference's exact NFElem sign() and resolves the
// "12 EXPLODED" near-Salem Pisot candidates (the 39-substitution
// survey previously EXPLODED on these 12 because the double-
// precision in_H_sigma couldn't reliably distinguish "strictly
// less than v[j]" from "on the boundary" when the difference was
// < 1e-12).  The exact path is bit-exact.
//
// Per ANSWERS_TO_QUESTIONS.txt §3.1, σ_1 has 9 D_cont elements
// (the corrected paper's transcription-corrected table) and σ_2
// has 9 elements (with two specific corrections from the published
// version).

#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

#include "ravel/core.hpp"
#include "ravel/faces.hpp"

namespace ravel {

// One candidate D_cont entry.
template <std::size_t d>
struct DCandidate {
    long long i;
    std::array<long long, d> x;
    long long j;
};

// Check whether a candidate (i, x, j) is in D_cont for the given
// substitution.  Returns true iff the geometric definition is
// satisfied.  The substitution's right Perron eigenvector v is
// used for the in_H_sigma test (double-precision).
template <std::size_t d>
bool is_in_D_cont(const Substitution<d>& subst,
                  const DCandidate<d>& c) {
    // The face-intersection-dimension geometry below (faces.hpp) is
    // unit-cube-specific: "face [x,i] extends by the unit interval
    // [x[k],x[k]+1) in every direction k != i" is a fact about cube
    // faces, not a trivial reparametrization of Eq 2.2 the way
    // in_H_sigma's tile_faces generalization is. Refuse explicitly
    // for a non-default tile rather than silently computing a wrong
    // D_cont -- see core.hpp's Substitution::tile_faces comment.
    if (!subst.is_unit_cube_tile()) {
        throw std::domain_error(
            "is_in_D_cont: face-intersection geometry is not yet "
            "generalized beyond the unit-cube tile (subst.tile_faces "
            "is non-default); see core.hpp's tile_faces comment");
    }
    // 1. (i < j) if x = 0 (anti-symmetrization at origin)
    bool all_zero = true;
    for (std::size_t k = 0; k < d; ++k)
        if (c.x[k] != 0) { all_zero = false; break; }
    if (all_zero && !(c.i < c.j)) return false;

    // 2. in_H_sigma([x, j])  — exact via math library.
    if (!subst.in_H_sigma_exact(c.x, static_cast<std::size_t>(c.j))) return false;

    // 3. face intersection dim == d - 2.
    //    Use the SPECIFIC D_cont filter: intersect the ORIGIN face
    //    [0, i] with the x-shifted face [x, j].
    int dim = d_cont_face_intersection_dim<d>(
        c.x, static_cast<std::size_t>(c.i),
        static_cast<std::size_t>(c.j));
    if (dim != static_cast<int>(d) - 2) return false;

    return true;
}

// Verify a whole D_cont table at once.  Returns the count of
// entries in the table that are in D_cont according to the
// geometric definition.  This is the reference's
// validate_dcont.py check, ported to the local project.
template <std::size_t d>
std::size_t verify_D_cont_table(const Substitution<d>& subst,
                                const std::vector<DCandidate<d>>& table) {
    std::size_t count = 0;
    for (const auto& c : table) {
        if (is_in_D_cont<d>(subst, c)) ++count;
    }
    return count;
}

// D_cont search execution mode.  The projected face enumerator is exact for
// the unit-cube geometry: the d-2 intersection condition itself forces the
// coordinate pattern, so there is no reason to scan the full box.  The legacy
// box enumerator remains available for differential verification.
enum class DContSearchMode {
    projected_faces,
    legacy_box,
};

inline DContSearchMode default_d_cont_search_mode() {
    const char* own = std::getenv("RAVEL_D_CONT_MODE");
    const char* shared = std::getenv("RAVEL_CORONA_MODE");
    const std::string_view value = own != nullptr ? own
        : (shared != nullptr ? shared : "projected");
    if (value == "legacy" || value == "box" || value == "materialized" ||
        value == "full")
        return DContSearchMode::legacy_box;
    if (value == "projected" || value == "surface" || value == "faces" ||
        value == "lazy" || value.empty())
        return DContSearchMode::projected_faces;
    throw std::invalid_argument("unknown D_cont search mode: " +
                                std::string(value));
}

template <std::size_t d>
void search_D_cont_recursive(const Substitution<d>& subst,
                             std::array<long long, d>& x,
                             std::size_t k,
                             std::size_t i,
                             std::size_t j,
                             long long bound,
                             std::vector<DCandidate<d>>& out) {
    if (k == d) {
        bool all_zero = true;
        for (std::size_t m = 0; m < d; ++m)
            if (x[m] != 0) { all_zero = false; break; }
        if (all_zero && i >= j) return;
        if (!subst.in_H_sigma_exact(x, j)) return;
        if (d_cont_face_intersection_dim<d>(x, i, j)
                == static_cast<int>(d) - 2)
            out.push_back(DCandidate<d>{
                static_cast<long long>(i), x,
                static_cast<long long>(j)});
        return;
    }
    for (long long value = -bound; value <= bound; ++value) {
        x[k] = value;
        search_D_cont_recursive<d>(
            subst, x, k + 1, i, j, bound, out);
    }
}

template <std::size_t d>
std::vector<DCandidate<d>> search_D_cont_legacy_box(
        const Substitution<d>& subst, long long bound = 2) {
    if (!subst.is_unit_cube_tile())
        throw std::domain_error(
            "search_D_cont: face-intersection geometry is not yet "
            "generalized beyond the unit-cube tile");
    if (bound < 0)
        throw std::invalid_argument("search_D_cont: negative bound");
    std::vector<DCandidate<d>> found;
    for (std::size_t i = 0; i < d; ++i)
        for (std::size_t j = 0; j < d; ++j) {
            std::array<long long, d> x{};
            search_D_cont_recursive<d>(
                subst, x, 0, i, j, bound, found);
        }
    return found;
}

// Enumerate exactly the lattice translations whose unit-cube faces can meet
// in dimension d-2.
//
// i != j:
//   x_i in {-1,0}, x_j in {0,1}, every other coordinate is 0.
// i == j:
//   x_i = 0 and exactly one other coordinate is +1 or -1.
//
// These are not heuristic bounds.  They follow coordinate-by-coordinate from
// intersecting {0}/[0,1] with {x_k}/[x_k,x_k+1].
template <std::size_t d>
std::vector<std::array<long long, d>> d_cont_face_translation_candidates(
        std::size_t i, std::size_t j, long long bound = 2) {
    if (i >= d || j >= d)
        throw std::out_of_range("D_cont face index out of range");
    if (bound < 0)
        throw std::invalid_argument("D_cont candidate bound is negative");
    std::vector<std::array<long long, d>> result;
    if (i != j) {
        for (const long long xi : {-1LL, 0LL})
            for (const long long xj : {0LL, 1LL}) {
                if (std::llabs(xi) > bound || std::llabs(xj) > bound)
                    continue;
                std::array<long long, d> x{};
                x[i] = xi;
                x[j] = xj;
                result.push_back(x);
            }
    } else if (bound >= 1) {
        for (std::size_t k = 0; k < d; ++k) {
            if (k == i) continue;
            std::array<long long, d> positive{};
            positive[k] = 1;
            result.push_back(positive);
            auto negative = positive;
            negative[k] = -1;
            result.push_back(negative);
        }
    }
    return result;
}

template <std::size_t d>
std::vector<DCandidate<d>> search_D_cont_projected_faces(
        const Substitution<d>& subst, long long bound = 2) {
    if (!subst.is_unit_cube_tile())
        throw std::domain_error(
            "search_D_cont: face-intersection geometry is not yet "
            "generalized beyond the unit-cube tile");
    std::vector<DCandidate<d>> found;
    for (std::size_t i = 0; i < d; ++i)
        for (std::size_t j = 0; j < d; ++j)
            for (const auto& x :
                 d_cont_face_translation_candidates<d>(i, j, bound)) {
                bool all_zero = true;
                for (const auto value : x)
                    all_zero = all_zero && value == 0;
                if (all_zero && i >= j) continue;
                DCandidate<d> candidate{
                    static_cast<long long>(i), x,
                    static_cast<long long>(j)};
                // Retain both independent predicates as a replay check on the
                // symbolic candidate derivation.
                if (is_in_D_cont<d>(subst, candidate))
                    found.push_back(candidate);
            }
    return found;
}

// Geometric D_cont derivation.  Projected face enumeration is the default;
// pass legacy_box explicitly or set RAVEL_CORONA_MODE=legacy (or
// RAVEL_D_CONT_MODE=legacy) for the historical exhaustive box scan.
template <std::size_t d>
std::vector<DCandidate<d>> search_D_cont(
        const Substitution<d>& subst,
        long long bound = 2,
        DContSearchMode mode = default_d_cont_search_mode()) {
    return mode == DContSearchMode::legacy_box
        ? search_D_cont_legacy_box<d>(subst, bound)
        : search_D_cont_projected_faces<d>(subst, bound);
}

}  // namespace ravel
