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

// search_D_cont(subst, bound): geometric D_cont derivation by
// exhaustive search over x in [-bound, bound]^d and (i, j) in A^2.
// Returns every (i, x, j) satisfying:
//   * if x == 0 then i < j (anti-symmetrization);
//   * [x, j] in H_sigma;
//   * face intersection dim of [x, i] with [x, j] equals d - 2
//     (a (d-2)-dimensional face contact).
//
// For d = 3 and bound = 2 the search space is 5^3 * 9 = 1125
// candidates, runs in milliseconds.  bound = 3 takes ~10x longer
// (5^3 -> 7^3 = 343 per letter pair).
//
// For unimodular cubic Pisot substitutions (σ_{a,b} family and
// Tribonacci/Plastic/Supergolden-class), the d_cont lies entirely
// in [-2, 2]^d -- bound = 2 is sufficient.  For non-unimodular or
// higher-degree substitutions you may need bound = 3 or 4 to find
// the full D_cont; the search is geometric and will find the right
// elements if they're in the box.
//
// This is the missing piece that lets `ravel.contact_boundary.*`
// run end-to-end on arbitrary Pisot substitutions without hand-
// curated D_cont inputs: pair `search_D_cont` with
// `compute_contact_boundary_dispatch` and the only required caller
// input is the substitution itself.
template <std::size_t d>
std::vector<DCandidate<d>> search_D_cont(const Substitution<d>& subst,
                                          long long bound = 2) {
    // See is_in_D_cont's identical guard: face-intersection geometry
    // is unit-cube-specific and not yet generalized.
    if (!subst.is_unit_cube_tile()) {
        throw std::domain_error(
            "search_D_cont: face-intersection geometry is not yet "
            "generalized beyond the unit-cube tile (subst.tile_faces "
            "is non-default); see core.hpp's tile_faces comment");
    }
    std::vector<DCandidate<d>> found;
    for (std::size_t i = 0; i < d; ++i) {
        for (std::size_t j = 0; j < d; ++j) {
            // Iterate x in [-bound, bound]^d.  We use a recursive
            // helper to keep the type signature clean for any d.
            std::array<long long, d> x{};
            search_D_cont_recursive<d>(subst, x, 0, i, j, bound, found);
        }
    }
    return found;
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
        // Anti-symmetrization at the origin: skip (i, 0, i).
        bool all_zero = true;
        for (std::size_t m = 0; m < d; ++m) if (x[m] != 0) { all_zero = false; break; }
        if (all_zero && i >= j) return;
        if (!subst.in_H_sigma_exact(x, j)) return;
        // Use the SPECIFIC D_cont filter: origin face [0, i] ∩ [x, j]
        int dim = d_cont_face_intersection_dim<d>(x, i, j);
        if (dim == static_cast<int>(d) - 2) {
            DCandidate<d> c;
            c.i = static_cast<long long>(i);
            c.x = x;
            c.j = static_cast<long long>(j);
            out.push_back(c);
        }
        return;
    }
    for (long long v = -bound; v <= bound; ++v) {
        x[k] = v;
        search_D_cont_recursive<d>(subst, x, k + 1, i, j, bound, out);
    }
}

}  // namespace ravel
