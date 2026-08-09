#pragma once

#include <cstddef>
#include <exception>
#include <string>
#include <vector>

#include "ravel/proof/condition_f_pair_boundary_substitution.hpp"
#include "ravel/proof/generalized_multinacci_general_m.hpp"
#include "math/proof_reflection.hpp"

namespace ravel::proof {

/** Uniform transport of a simultaneous Q/R comparison through the
 * generalized-multinacci renewal scheduler.
 *
 * The only symbolic states added by the thick prefix are cut pairs
 * (p,q) in {0,...,m}^2.  Their defect d=q-p emits Q for d=0 and R^|d|
 * otherwise.  Therefore a common intertwiner for Q and R automatically
 * intertwines every cut channel, every roof word R^d (d<=m), and the full
 * triangular scheduler S_m.
 */
struct GeneralizedMultinacciGeneralMIntertwinerProof {
    std::size_t multiplicity = 0;
    std::size_t symbolic_cut_states = 0;
    std::size_t words_checked = 0;
    std::size_t polynomial_terms = 0;
    bool symbolic_state_catalogue_complete = false;
    bool q_intertwines = false;
    bool r_intertwines = false;
    bool every_roof_word_intertwines = false;
    bool every_cut_channel_intertwines = false;
    bool scheduler_polynomial_intertwines = false;
    bool side_voltage_is_order_invisible = false;
    bool proved = false;
    NonnegativeMatrix competitor_scheduler;
    NonnegativeMatrix core_scheduler;
    std::string obstruction;
};

inline NonnegativeMatrix scalar_multiple(const NonnegativeMatrix& A,
                                         std::size_t c) {
    NonnegativeMatrix out=A;
    for(auto& row:out) for(auto& x:row) x*=static_cast<long long>(c);
    return out;
}

inline GeneralizedMultinacciGeneralMIntertwinerProof
 derive_generalized_multinacci_general_m_intertwiner(
    std::size_t multiplicity,
    const NonnegativeMatrix& competitor_q,
    const NonnegativeMatrix& competitor_r,
    const NonnegativeMatrix& core_q,
    const NonnegativeMatrix& core_r,
    const NonnegativeMatrix& intertwiner) {
    using namespace condition_f_pair_boundary_detail;
    GeneralizedMultinacciGeneralMIntertwinerProof out;
    out.multiplicity=multiplicity;
    if(multiplicity==0) {
        out.obstruction="general-m intertwiner requires m>=1";
        return out;
    }
    const auto scheduler=derive_generalized_multinacci_general_m(multiplicity);
    if(!scheduler.proved) {
        out.obstruction=scheduler.obstruction;
        return out;
    }
    try {
        out.symbolic_cut_states=scheduler.ordered_channels;
        out.symbolic_state_catalogue_complete=
            scheduler.prefix_positions==multiplicity+1 &&
            scheduler.ordered_channels==(multiplicity+1)*(multiplicity+1);

        const auto qcP=rectangular_product(competitor_q,intertwiner);
        const auto rcP=rectangular_product(competitor_r,intertwiner);
        const auto Pqk=rectangular_product(intertwiner,core_q);
        const auto Prk=rectangular_product(intertwiner,core_r);
        out.q_intertwines=leq(qcP,Pqk);
        out.r_intertwines=leq(rcP,Prk);
        if(!out.q_intertwines || !out.r_intertwines) {
            out.obstruction="input matrices do not share a Q/R intertwiner";
            return out;
        }

        const std::size_t cr=competitor_q.size(), kr=core_q.size();
        out.competitor_scheduler=zero_like(cr,cr);
        out.core_scheduler=zero_like(kr,kr);
        auto lhs=zero_like(cr,kr), rhs=zero_like(cr,kr);
        out.every_roof_word_intertwines=true;
        out.every_cut_channel_intertwines=true;

        for(std::size_t d=0;d<=multiplicity;++d) {
            const std::vector<std::size_t> word = d==0
                ? std::vector<std::size_t>{0}
                : std::vector<std::size_t>(d,1);
            const auto cw=evaluate_word(competitor_q,competitor_r,word);
            const auto kw=evaluate_word(core_q,core_r,word);
            const auto cwP=rectangular_product(cw,intertwiner);
            const auto Pkw=rectangular_product(intertwiner,kw);
            out.every_roof_word_intertwines &= leq(cwP,Pkw);
            ++out.words_checked;

            const auto coefficient=scheduler.unsigned_scheduler_coefficients.at(d);
            out.competitor_scheduler=add_matrix(
                out.competitor_scheduler,scalar_multiple(cw,coefficient));
            out.core_scheduler=add_matrix(
                out.core_scheduler,scalar_multiple(kw,coefficient));
            lhs=add_matrix(lhs,scalar_multiple(cwP,coefficient));
            rhs=add_matrix(rhs,scalar_multiple(Pkw,coefficient));
            out.polynomial_terms+=coefficient;
        }

        // The signed side voltage changes only the fibre decoration.  The
        // positive transport word depends on |q-p|, hence both signs use the
        // same matrix inequality.
        out.side_voltage_is_order_invisible=scheduler.side_voltage_is_sign_of_defect;
        for(const auto& channel : scheduler.macro_words)
            out.every_cut_channel_intertwines &= !channel.empty();

        out.scheduler_polynomial_intertwines=leq(lhs,rhs) &&
            leq(rectangular_product(out.competitor_scheduler,intertwiner),
                rectangular_product(intertwiner,out.core_scheduler));
        out.proved=out.symbolic_state_catalogue_complete &&
                   out.every_roof_word_intertwines &&
                   out.every_cut_channel_intertwines &&
                   out.scheduler_polynomial_intertwines &&
                   out.side_voltage_is_order_invisible;
        if(!out.proved && out.obstruction.empty())
            out.obstruction="general-m renewal transport certificate failed";
    } catch(const std::exception& e) {
        out.obstruction=e.what();
    }
    return out;
}

// Stages a `GeneralizedMultinacciGeneralMIntertwinerReflectionCertificate`
// for one multiplicity m -- gates on `proof.proved`, which independently
// rechecks every roof-word channel's Q/R intertwining inequality (see
// derive_generalized_multinacci_general_m_intertwiner above).
inline void stage_generalized_multinacci_general_m_intertwiner(
        const GeneralizedMultinacciGeneralMIntertwinerProof& proof,
        const std::string& description) {
    if (!proof.proved) return;
    if (!mathlib::reflection::enabled()) return;
    mathlib::reflection::GeneralizedMultinacciGeneralMIntertwinerReflectionCertificate node;
    node.multiplicity = static_cast<long long>(proof.multiplicity);
    node.symbolic_cut_states = static_cast<long long>(proof.symbolic_cut_states);
    node.words_checked = static_cast<long long>(proof.words_checked);
    node.description = description;
    mathlib::reflection::record(mathlib::reflection::NodeKind::LemmaApplication, node);
}

} // namespace ravel::proof
