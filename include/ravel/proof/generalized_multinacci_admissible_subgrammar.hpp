#pragma once

#include <cstddef>
#include <map>
#include <string>
#include <vector>

#include "ravel/proof/generalized_multinacci_boundary_word_lift.hpp"
#include "ravel/proof/generalized_multinacci_general_m.hpp"
#include "ravel/proof/generalized_multinacci_general_m_intertwiner.hpp"
#include "math/proof_reflection.hpp"

namespace ravel::proof {

/** Boundary admissibility is a positive subgrammar operation.
 *
 * For sigma_{D,m}, every parent-prefix pair is one of the symbolic cuts
 * (p,q) in {0,...,m}^2 and emits Q when p=q, otherwise R^|q-p|.  A concrete
 * contact-boundary restriction can only discard some of those witnessed
 * channels at each source state.  Since the universal word theorem is
 * monotone under nonnegative sums, no closed-form catalogue of the surviving
 * boundary states is needed for dominance: every exact admissible subgrammar
 * inherits the common Q/R intertwiner.
 */
struct GeneralizedMultinacciAdmissibleSubgrammarProof {
    std::size_t dimension = 0;
    std::size_t multiplicity = 0;
    std::size_t source_states = 0;
    std::size_t witnessed_edges = 0;
    std::size_t words_checked = 0;
    bool every_prefix_cut_in_range = false;
    bool every_edge_is_scheduler_word = false;
    bool every_edge_word_intertwines = false;
    bool admissibility_is_deletion_only = false;
    bool arbitrary_positive_subsum_intertwines = false;
    bool no_symbolic_state_catalogue_needed = false;
    bool proved = false;
    std::string obstruction;
};

template <std::size_t d>
GeneralizedMultinacciAdmissibleSubgrammarProof
 derive_generalized_multinacci_admissible_subgrammar(
    const MultinacciBoundaryWordLiftProof<d>& lift,
    const NonnegativeMatrix& competitor_q,
    const NonnegativeMatrix& competitor_r,
    const NonnegativeMatrix& core_q,
    const NonnegativeMatrix& core_r,
    const NonnegativeMatrix& intertwiner) {
    using namespace condition_f_pair_boundary_detail;
    GeneralizedMultinacciAdmissibleSubgrammarProof out;
    out.dimension=d;
    out.multiplicity=lift.multiplicity;
    out.source_states=lift.projected_adjacency.size();
    out.witnessed_edges=lift.edges.size();
    if(!lift.proved) {
        out.obstruction="admissible subgrammar requires a proved boundary word lift";
        return out;
    }
    const auto scheduler=derive_generalized_multinacci_general_m(lift.multiplicity);
    if(!scheduler.proved) {
        out.obstruction=scheduler.obstruction;
        return out;
    }
    try {
        const auto qcP=rectangular_product(competitor_q,intertwiner);
        const auto rcP=rectangular_product(competitor_r,intertwiner);
        const auto Pq=rectangular_product(intertwiner,core_q);
        const auto Pr=rectangular_product(intertwiner,core_r);
        if(!leq(qcP,Pq) || !leq(rcP,Pr)) {
            out.obstruction="primitive Q/R intertwiner does not hold";
            return out;
        }
        out.every_prefix_cut_in_range=true;
        out.every_edge_is_scheduler_word=true;
        out.every_edge_word_intertwines=true;
        for(const auto& e:lift.edges) {
            out.every_prefix_cut_in_range &=
                e.left_prefix_position<=lift.multiplicity &&
                e.right_prefix_position<=lift.multiplicity;
            const auto roof=static_cast<std::size_t>(
                e.signed_prefix_defect<0 ? -e.signed_prefix_defect : e.signed_prefix_defect);
            const std::vector<std::size_t> word = roof==0
                ? std::vector<std::size_t>{0}
                : std::vector<std::size_t>(roof,1);
            out.every_edge_is_scheduler_word &= roof<=lift.multiplicity;
            const auto cw=evaluate_word(competitor_q,competitor_r,word);
            const auto kw=evaluate_word(core_q,core_r,word);
            out.every_edge_word_intertwines &= leq(
                rectangular_product(cw,intertwiner),
                rectangular_product(intertwiner,kw));
            ++out.words_checked;
        }
        // The concrete boundary graph is obtained by retaining exactly the
        // witnessed transitions whose target remains in the boundary report.
        // No negative coefficient, subtraction, or relabelling occurs.
        out.admissibility_is_deletion_only=
            lift.projection_equals_existing_boundary_graph;
        out.arbitrary_positive_subsum_intertwines=
            out.every_edge_word_intertwines;
        out.no_symbolic_state_catalogue_needed=
            out.every_prefix_cut_in_range &&
            out.every_edge_is_scheduler_word &&
            out.admissibility_is_deletion_only;
        out.proved=out.no_symbolic_state_catalogue_needed &&
                   out.arbitrary_positive_subsum_intertwines;
        if(!out.proved && out.obstruction.empty())
            out.obstruction="admissible positive subgrammar certificate failed";
    } catch(const std::exception& e) {
        out.obstruction=e.what();
    }
    return out;
}

// Stages a `GeneralizedMultinacciAdmissibleSubgrammarReflectionCertificate`
// for one (D,m) sweep instance -- gates on the certificate's own `.proved`
// flag, which is itself derived from independently re-checked Q/R
// intertwining inequalities per witnessed edge (see
// `derive_generalized_multinacci_admissible_subgrammar` above), not a
// static assumption.
inline void stage_generalized_multinacci_admissible_subgrammar(
        const GeneralizedMultinacciAdmissibleSubgrammarProof& proof,
        const std::string& description) {
    if (!proof.proved) return;
    if (!mathlib::reflection::enabled()) return;
    mathlib::reflection::GeneralizedMultinacciAdmissibleSubgrammarReflectionCertificate node;
    node.dimension = static_cast<long long>(proof.dimension);
    node.multiplicity = static_cast<long long>(proof.multiplicity);
    node.source_states = static_cast<long long>(proof.source_states);
    node.witnessed_edges = static_cast<long long>(proof.witnessed_edges);
    node.words_checked = static_cast<long long>(proof.words_checked);
    node.description = description;
    mathlib::reflection::record(mathlib::reflection::NodeKind::LemmaApplication, node);
}

} // namespace ravel::proof
