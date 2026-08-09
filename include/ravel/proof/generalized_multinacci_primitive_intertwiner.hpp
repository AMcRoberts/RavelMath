#pragma once

#include <cstddef>
#include <map>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

#include "ravel/contact_boundary.hpp"
#include "ravel/proof/condition_f_pair_boundary_substitution.hpp"
#include "ravel/proof/generalized_multinacci_boundary_word_lift.hpp"
#include "ravel/proof/positive_word_graph_expansion.hpp"
#include "math/proof_reflection.hpp"

namespace ravel::proof {

struct GeneralizedMultinacciParentChoice {
    std::size_t parent = 0;
    std::size_t prefix_position = 0;
};

inline std::vector<GeneralizedMultinacciParentChoice>
generalized_multinacci_parent_choices(std::size_t dimension,
                                      std::size_t multiplicity,
                                      std::size_t inner) {
    std::vector<GeneralizedMultinacciParentChoice> out;
    if (dimension < 2 || multiplicity == 0 || inner >= dimension) return out;
    if (inner == 0) {
        // Every nonterminal image 0^m(c+1) contains m zeroes, at cuts
        // 0,...,m-1.  The terminal image 0 contributes one zero at cut 0.
        for (std::size_t parent = 0; parent + 1 < dimension; ++parent)
            for (std::size_t p = 0; p < multiplicity; ++p)
                out.push_back({parent,p});
        out.push_back({dimension-1,0});
    } else {
        // The unique nonzero letter inner occurs at the end of image inner-1.
        out.push_back({inner-1,multiplicity});
    }
    return out;
}

struct GeneralizedMultinacciUniversalMacroEdge {
    std::size_t source_role = 0;
    std::size_t target_role = 0;
    std::size_t left_prefix_position = 0;
    std::size_t right_prefix_position = 0;
    long long signed_prefix_defect = 0;
    std::vector<std::size_t> word;
};

struct GeneralizedMultinacciPrimitiveIntertwinerProof {
    std::size_t dimension = 0;
    std::size_t multiplicity = 0;
    std::size_t boundary_base_states = 0;
    std::size_t boundary_expanded_states = 0;
    std::size_t universal_base_roles = 0;
    std::size_t universal_macro_edges = 0;
    std::size_t universal_expanded_states = 0;
    std::size_t mapped_phase_states = 0;
    NonnegativeMatrix intertwiner;
    NonnegativeMatrix boundary_q;
    NonnegativeMatrix boundary_r;
    NonnegativeMatrix universal_q;
    NonnegativeMatrix universal_r;
    bool parent_grammar_complete = false;
    bool every_boundary_macro_has_universal_witness = false;
    bool base_role_projection_exact = false;
    bool private_phase_projection_exact = false;
    bool q_intertwines = false;
    bool r_intertwines = false;
    bool simultaneous_qr_intertwiner = false;
    bool proved = false;
    std::string obstruction;
};

namespace generalized_multinacci_primitive_detail {

using MacroKey = std::tuple<std::size_t,std::size_t,std::size_t,std::size_t>;

inline std::vector<GeneralizedMultinacciUniversalMacroEdge>
universal_macro_edges(std::size_t D, std::size_t m) {
    std::vector<GeneralizedMultinacciUniversalMacroEdge> out;
    auto role=[D](std::size_t i,std::size_t j){return i*D+j;};
    for(std::size_t i=0;i<D;++i) for(std::size_t j=0;j<D;++j) {
        const auto left=generalized_multinacci_parent_choices(D,m,i);
        const auto right=generalized_multinacci_parent_choices(D,m,j);
        for(const auto& a:left) for(const auto& b:right) {
            const long long delta=static_cast<long long>(b.prefix_position)-
                                  static_cast<long long>(a.prefix_position);
            const auto roof=static_cast<std::size_t>(delta<0?-delta:delta);
            std::vector<std::size_t> word = roof==0
                ? std::vector<std::size_t>{0}
                : std::vector<std::size_t>(roof,1);
            out.push_back({role(i,j),role(a.parent,b.parent),
                           a.prefix_position,b.prefix_position,delta,
                           std::move(word)});
        }
    }
    return out;
}

inline NonnegativeMatrix incidence(std::size_t rows,std::size_t cols) {
    return NonnegativeMatrix(rows,std::vector<long long>(cols,0));
}

} // namespace generalized_multinacci_primitive_detail

/** Construct the missing primitive Q/R comparison for generalized multinacci.
 *
 * The concrete contact-boundary system is a deletion-only subgrammar of the
 * universal parent-role suspension.  Both are expanded into primitive Q/R
 * graphs by giving every R^d macro edge d-1 private phase states.  A boundary
 * base state (i,x,j) maps to the universal role (i,j); each private phase state
 * maps to the corresponding private phase state of the unique universal
 * parent-prefix witness.  The resulting 0/1 incidence matrix P satisfies
 *
 *     Q_B P <= P Q_U,    R_B P <= P R_U.
 *
 * This is the concrete simultaneous intertwiner required by the positive-word
 * and general-m renewal theorems.
 */
template <std::size_t d>
GeneralizedMultinacciPrimitiveIntertwinerProof
derive_generalized_multinacci_primitive_intertwiner(
    const ContactBoundaryReport& report,
    const MultinacciBoundaryWordLiftProof<d>& lift) {
    using namespace generalized_multinacci_primitive_detail;
    using namespace condition_f_pair_boundary_detail;
    GeneralizedMultinacciPrimitiveIntertwinerProof out;
    out.dimension=d; out.multiplicity=lift.multiplicity;
    out.boundary_base_states=report.boundary_nodes.size();
    out.universal_base_roles=d*d;
    if(!lift.proved || lift.multiplicity==0) {
        out.obstruction="primitive intertwiner requires a proved nonzero-m boundary lift";
        return out;
    }
    if(report.boundary_nodes.size()!=lift.projected_adjacency.size()) {
        out.obstruction="boundary report/lift size mismatch";
        return out;
    }

    // An empty contact boundary is the zero competitor and is dominated
    // trivially.  Keep a 0 x |U| intertwiner rather than asking the generic
    // path expander to represent an empty graph.
    if (report.boundary_nodes.empty()) {
        const auto universal=universal_macro_edges(d,lift.multiplicity);
        std::vector<PositiveWordGraphEdge> universal_words;
        for (const auto& e:universal)
            universal_words.push_back({e.source_role,e.target_role,e.word});
        const auto ue=derive_positive_word_graph_expansion(d*d,2,universal_words);
        if(!ue.proved) { out.obstruction=ue.obstruction; return out; }
        out.universal_macro_edges=universal.size();
        out.universal_expanded_states=ue.expanded_node_count;
        out.universal_q=ue.generator_adjacency.at(0);
        out.universal_r=ue.generator_adjacency.at(1);
        out.intertwiner=incidence(0,out.universal_expanded_states);
        out.parent_grammar_complete=true;
        out.every_boundary_macro_has_universal_witness=true;
        out.base_role_projection_exact=true;
        out.private_phase_projection_exact=true;
        out.q_intertwines=true;
        out.r_intertwines=true;
        out.simultaneous_qr_intertwiner=true;
        out.proved=true;
        return out;
    }

    std::vector<PositiveWordGraphEdge> boundary_macro;
    boundary_macro.reserve(lift.edges.size());
    for(const auto& e:lift.edges) {
        std::vector<std::size_t> word;
        for(const auto g:e.word)
            word.push_back(g==PrefixPrimitiveGenerator::balanced_q?0:1);
        boundary_macro.push_back({e.source,e.target,std::move(word)});
    }
    const auto boundary_expansion=derive_positive_word_graph_expansion(
        report.boundary_nodes.size(),2,boundary_macro);
    if(!boundary_expansion.proved) {
        out.obstruction="boundary primitive expansion failed: "+boundary_expansion.obstruction;
        return out;
    }
    out.boundary_expanded_states=boundary_expansion.expanded_node_count;
    out.boundary_q=boundary_expansion.generator_adjacency.at(0);
    out.boundary_r=boundary_expansion.generator_adjacency.at(1);

    const auto universal=universal_macro_edges(d,lift.multiplicity);
    out.universal_macro_edges=universal.size();
    std::vector<PositiveWordGraphEdge> universal_words;
    universal_words.reserve(universal.size());
    std::map<MacroKey,std::size_t> universal_index;
    bool unique=true;
    for(std::size_t k=0;k<universal.size();++k) {
        const auto& e=universal[k];
        universal_words.push_back({e.source_role,e.target_role,e.word});
        const MacroKey key{e.source_role,e.target_role,
                           e.left_prefix_position,e.right_prefix_position};
        if(!universal_index.emplace(key,k).second) unique=false;
    }
    out.parent_grammar_complete=unique;
    if(!unique) {
        out.obstruction="universal parent-prefix witness key is not unique";
        return out;
    }
    const auto universal_expansion=derive_positive_word_graph_expansion(
        d*d,2,universal_words);
    if(!universal_expansion.proved) {
        out.obstruction="universal primitive expansion failed: "+universal_expansion.obstruction;
        return out;
    }
    out.universal_expanded_states=universal_expansion.expanded_node_count;
    out.universal_q=universal_expansion.generator_adjacency.at(0);
    out.universal_r=universal_expansion.generator_adjacency.at(1);
    out.intertwiner=incidence(out.boundary_expanded_states,
                              out.universal_expanded_states);

    // Base states forget only displacement.
    out.base_role_projection_exact=true;
    for(std::size_t s=0;s<report.boundary_nodes.size();++s) {
        const auto& node=report.boundary_nodes[s];
        const auto i=std::get<0>(node), j=std::get<2>(node);
        if(i<0 || j<0 || static_cast<std::size_t>(i)>=d ||
           static_cast<std::size_t>(j)>=d) {
            out.base_role_projection_exact=false;
            out.obstruction="boundary node has role outside alphabet";
            return out;
        }
        out.intertwiner[s][static_cast<std::size_t>(i)*d+
                            static_cast<std::size_t>(j)]=1;
    }

    // Find the private intermediate node at each position of each expansion.
    std::map<std::pair<std::size_t,std::size_t>,std::size_t> bphase,uphase;
    for(const auto& e:boundary_expansion.primitive_edges)
        if(e.target>=boundary_expansion.base_node_count)
            bphase[{e.macro_edge,e.position}]=e.target;
    for(const auto& e:universal_expansion.primitive_edges)
        if(e.target>=universal_expansion.base_node_count)
            uphase[{e.macro_edge,e.position}]=e.target;

    out.every_boundary_macro_has_universal_witness=true;
    out.private_phase_projection_exact=true;
    for(std::size_t k=0;k<lift.edges.size();++k) {
        const auto& e=lift.edges[k];
        const auto& src=report.boundary_nodes[e.source];
        const auto& dst=report.boundary_nodes[e.target];
        const auto sr=static_cast<std::size_t>(std::get<0>(src))*d+
                      static_cast<std::size_t>(std::get<2>(src));
        const auto tr=static_cast<std::size_t>(std::get<0>(dst))*d+
                      static_cast<std::size_t>(std::get<2>(dst));
        const MacroKey key{sr,tr,e.left_prefix_position,e.right_prefix_position};
        const auto it=universal_index.find(key);
        if(it==universal_index.end()) {
            out.every_boundary_macro_has_universal_witness=false;
            out.obstruction="boundary macro edge has no universal parent-prefix witness";
            return out;
        }
        const auto uk=it->second;
        const auto internal=e.word.size()>0?e.word.size()-1:0;
        for(std::size_t pos=0;pos<internal;++pos) {
            const auto bi=bphase.find({k,pos});
            const auto ui=uphase.find({uk,pos});
            if(bi==bphase.end() || ui==uphase.end()) {
                out.private_phase_projection_exact=false;
                out.obstruction="private renewal phase state mismatch";
                return out;
            }
            out.intertwiner[bi->second][ui->second]=1;
            ++out.mapped_phase_states;
        }
    }

    // Every row must map to exactly one universal state.
    for(const auto& row:out.intertwiner) {
        long long sum=0; for(const auto x:row) sum+=x;
        if(sum!=1) {
            out.private_phase_projection_exact=false;
            out.obstruction="intertwiner is not a total one-hot state map";
            return out;
        }
    }

    out.q_intertwines=leq(rectangular_product(out.boundary_q,out.intertwiner),
                           rectangular_product(out.intertwiner,out.universal_q));
    out.r_intertwines=leq(rectangular_product(out.boundary_r,out.intertwiner),
                           rectangular_product(out.intertwiner,out.universal_r));
    out.simultaneous_qr_intertwiner=out.q_intertwines && out.r_intertwines;
    out.proved=out.parent_grammar_complete &&
               out.every_boundary_macro_has_universal_witness &&
               out.base_role_projection_exact &&
               out.private_phase_projection_exact &&
               out.simultaneous_qr_intertwiner;
    if(!out.proved && out.obstruction.empty())
        out.obstruction="primitive generalized-multinacci Q/R intertwiner failed";
    return out;
}

// Stages a `GeneralizedMultinacciPrimitiveIntertwinerReflectionCertificate`
// for one (D,m) instance -- gates on `proof.proved`, itself derived from
// independently re-checked base-role/private-phase projections and a
// simultaneous Q/R intertwiner check (see the derivation above).
inline void stage_generalized_multinacci_primitive_intertwiner(
        const GeneralizedMultinacciPrimitiveIntertwinerProof& proof,
        const std::string& description) {
    if (!proof.proved) return;
    if (!mathlib::reflection::enabled()) return;
    mathlib::reflection::GeneralizedMultinacciPrimitiveIntertwinerReflectionCertificate node;
    node.dimension = static_cast<long long>(proof.dimension);
    node.multiplicity = static_cast<long long>(proof.multiplicity);
    node.boundary_expanded_states = static_cast<long long>(proof.boundary_expanded_states);
    node.universal_expanded_states = static_cast<long long>(proof.universal_expanded_states);
    node.universal_macro_edges = static_cast<long long>(proof.universal_macro_edges);
    node.mapped_phase_states = static_cast<long long>(proof.mapped_phase_states);
    node.description = description;
    mathlib::reflection::record(mathlib::reflection::NodeKind::LemmaApplication, node);
}

} // namespace ravel::proof
