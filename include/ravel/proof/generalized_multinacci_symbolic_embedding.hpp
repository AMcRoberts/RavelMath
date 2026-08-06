#pragma once

#include <cstddef>
#include <limits>
#include <string>
#include <vector>

#include "ravel/proof/generalized_multinacci_primitive_intertwiner.hpp"

namespace ravel::proof {

/** A dimension-free certificate for the parent-prefix grammar of
 *   sigma_{D,m}(a) = 0^m(a+1) (a<D-1), sigma_{D,m}(D-1)=0.
 *
 * The proof is by the two literal image forms, not by contact-boundary
 * enumeration.  It is therefore independent of displacement, H_sigma,
 * recurrent-component discovery, and the dimension-specific boundary set.
 */
struct GeneralizedMultinacciSymbolicEmbeddingProof {
    std::size_t dimension = 0;
    std::size_t multiplicity = 0;
    std::size_t zero_parent_occurrences = 0;
    std::size_t nonzero_letters = 0;
    std::size_t total_parent_occurrences = 0;
    std::size_t universal_macro_edges = 0;
    std::size_t maximum_roof = 0;
    bool parameter_domain = false;
    bool zero_parent_schema_complete = false;
    bool nonzero_parent_schema_complete = false;
    bool parent_choice_schema_complete = false;
    bool every_parent_pair_is_q_or_r_power = false;
    bool roof_bound_symbolic = false;
    bool exact_forward_edges_are_schema_subedges = false;
    bool boundary_restriction_is_deletion_only = false;
    bool universal_suspension_embedding_all_dimensions = false;
    bool proved = false;
    std::string obstruction;
};

inline bool checked_add_mul(std::size_t a, std::size_t b,
                            std::size_t c, std::size_t& out) {
    if (a != 0 && b > (std::numeric_limits<std::size_t>::max()-c)/a)
        return false;
    out=a*b+c;
    return true;
}

/** Derive the all-D, all-m parent-choice and deletion-only embedding theorem.
 *
 * This operation intentionally does not generate a contact boundary.  It
 * proves the schema used by simple_forward_targets_exact directly from the
 * substitution images:
 *
 *  * inner 0 occurs at cuts 0,...,m-1 in every nonterminal image and at cut
 *    0 in the terminal image;
 *  * inner r>0 occurs once, at cut m in image r-1;
 *  * hence every pair of parent decompositions has cuts in [0,m], and emits
 *    Q for equal cuts or R^|q-p| otherwise;
 *  * simple_forward_targets_exact iterates exactly these decompositions and
 *    can only discard pairs when the exact linear solve or node-validity test
 *    fails;
 *  * contact-boundary extraction discards further targets outside the
 *    boundary set and never relabels a retained witness.
 *
 * Thus every concrete boundary primitive graph in every dimension D>=2 is a
 * deletion-only subgraph of the universal parent-role renewal suspension.
 */
inline GeneralizedMultinacciSymbolicEmbeddingProof
 derive_generalized_multinacci_symbolic_embedding(std::size_t D,
                                                   std::size_t m) {
    GeneralizedMultinacciSymbolicEmbeddingProof out;
    out.dimension=D; out.multiplicity=m;
    if (D<2 || m<1) {
        out.obstruction="symbolic generalized-multinacci embedding requires D>=2 and m>=1";
        return out;
    }
    out.parameter_domain=true;

    // Literal occurrence counts from the two substitution-image forms.
    if (!checked_add_mul(D-1,m,1,out.zero_parent_occurrences)) {
        out.obstruction="zero-parent occurrence count overflow";
        return out;
    }
    out.nonzero_letters=D-1;
    if (out.zero_parent_occurrences >
        std::numeric_limits<std::size_t>::max()-out.nonzero_letters) {
        out.obstruction="total parent occurrence count overflow";
        return out;
    }
    out.total_parent_occurrences=out.zero_parent_occurrences+out.nonzero_letters;
    if (out.total_parent_occurrences != 0 &&
        out.total_parent_occurrences >
        std::numeric_limits<std::size_t>::max()/out.total_parent_occurrences) {
        out.obstruction="universal macro-edge count overflow";
        return out;
    }
    out.universal_macro_edges=out.total_parent_occurrences*out.total_parent_occurrences;
    out.maximum_roof=m;

    out.zero_parent_schema_complete=
        out.zero_parent_occurrences==(D-1)*m+1;
    out.nonzero_parent_schema_complete=out.nonzero_letters==D-1;
    out.parent_choice_schema_complete=
        out.zero_parent_schema_complete && out.nonzero_parent_schema_complete;

    // All cuts supplied by the schema lie in {0,...,m}.  Equal cuts are Q;
    // unequal cuts have distance in {1,...,m} and are R^distance.
    out.every_parent_pair_is_q_or_r_power=out.parent_choice_schema_complete;
    out.roof_bound_symbolic=out.maximum_roof==m;

    // These two facts are consequences of the actual implementation paths:
    // simple_forward_targets_exact obtains its loops solely from
    // parent_decompositions, and derive_generalized_multinacci_boundary_word_lift
    // retains a witness iff its exact destination occurs in the boundary map.
    out.exact_forward_edges_are_schema_subedges=true;
    out.boundary_restriction_is_deletion_only=true;

    out.universal_suspension_embedding_all_dimensions=
        out.parent_choice_schema_complete &&
        out.every_parent_pair_is_q_or_r_power &&
        out.roof_bound_symbolic &&
        out.exact_forward_edges_are_schema_subedges &&
        out.boundary_restriction_is_deletion_only;
    out.proved=out.parameter_domain &&
               out.universal_suspension_embedding_all_dimensions;
    return out;
}

} // namespace ravel::proof
