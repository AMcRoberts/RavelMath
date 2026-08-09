#pragma once

#include <array>
#include <cstddef>
#include <map>
#include <set>
#include <stdexcept>
#include <string>
#include <tuple>
#include <vector>

#include "ravel/ambient_graph.hpp"
#include "ravel/contact_boundary.hpp"
#include "ravel/corona.hpp"
#include "ravel/proof/condition_f_pair_boundary_substitution.hpp"
#include "ravel/proof/finite_positive_grammar_majorant.hpp"

// Concrete contact-boundary version of first_genuine_fourth_generator.hpp's
// abstract-role finding for x^3-2x^2-2 (substitution 0->001, 1->2, 2->00,
// beta~2.35930, non-unit). This is the check that caught the quartic
// family's spurious fourth colour before it was believed: does the extra
// generator survive as a genuine edge in the REAL contact-boundary graph,
// under the same displacement-forgetting parent-role intertwiner used for
// plastic and supergolden, or was it an artifact of the abstract
// universal-role catalogue alone?
//
// Five generators this time, not three: neutral (0), +-1, +-2.

namespace ravel::proof {

struct FirstGenuineFourthGeneratorIntertwinerCertificate {
    std::size_t generator_count = 5;
    std::size_t boundary_states = 0;
    std::size_t universal_roles = 9;  // 3x3 role states
    long long boundary_edges = 0;
    long long universal_edges = 0;
    NonnegativeMatrix intertwiner;
    // index 0..4 maps to defect -2,-1,0,+1,+2
    std::array<NonnegativeMatrix,5> boundary_generators;
    std::array<NonnegativeMatrix,5> universal_generators;
    bool parent_catalogue_complete = false;
    bool every_boundary_edge_has_universal_witness = false;
    bool base_role_projection_exact = false;
    std::array<bool,5> generator_intertwines{};
    bool simultaneous_five_generator_intertwiner = false;
    bool boundary_realizes_defect_plus2 = false;   // does the CONCRETE graph even contain a +2 edge?
    bool boundary_realizes_defect_minus2 = false;
    bool finite_positive_grammar_ready = false;
    bool proved = false;
    std::string obstruction;
};

namespace first_genuine_fourth_generator_intertwiner_detail {

inline NonnegativeMatrix zero_matrix(std::size_t n, std::size_t m) {
    return NonnegativeMatrix(n, std::vector<long long>(m, 0));
}
inline std::size_t role(long long i, long long j) {
    return static_cast<std::size_t>(i) * 3 + static_cast<std::size_t>(j);
}
inline int gen_index(long long defect) {
    // -2,-1,0,+1,+2 -> 0,1,2,3,4
    if (defect < -2 || defect > 2) throw std::runtime_error("x^3-2x^2-2: unexpected defect outside [-2,2]");
    return static_cast<int>(defect + 2);
}
inline int generator_from_prefixes(const std::vector<long long>& left,
                                   const std::vector<long long>& right) {
    const long long dl = static_cast<long long>(left.size());
    const long long dr = static_cast<long long>(right.size());
    return gen_index(dr - dl);
}

using WitnessKey = std::tuple<std::size_t,std::size_t,int>;

} // namespace first_genuine_fourth_generator_intertwiner_detail

inline FirstGenuineFourthGeneratorIntertwinerCertificate
 derive_first_genuine_fourth_generator_intertwiner() {
    using namespace first_genuine_fourth_generator_intertwiner_detail;
    FirstGenuineFourthGeneratorIntertwinerCertificate out;

    // 0->001, 1->2, 2->00
    SubstitutionRule rule(std::vector<std::vector<std::int8_t>>{{0,0,1},{2},{0,0}});
    constexpr double beta = 2.3593040859717767;
    constexpr double b2 = 0.92071037690446744;
    ContactBoundaryLimits lim;
    lim.closure_cap = 40000;
    lim.corona_cap = 200000;
    lim.max_corona_rounds = 14;
    // This certificate consumes only the exact boundary graph.  Retaining
    // the dense boundary adjacency/spectral matrix would add O(|B|^2)
    // storage without contributing to the intertwiner or grammar checks.
    // Keep the graph-only path enabled so larger fourth-generator candidates
    // do not silently inherit the dense-matrix memory cost.
    lim.retain_boundary_matrix = false;
    const auto report = compute_contact_boundary_from_subst<3>(rule,beta,b2,3,lim);
    const auto subst = make_substitution<3>(rule,beta);

    std::vector<SNode<3>> boundary;
    boundary.reserve(report.boundary_nodes.size());
    std::map<SNode<3>,std::size_t> boundary_index;
    for (const auto& t : report.boundary_nodes) {
        SNode<3> n;
        n.i = std::get<0>(t); n.j = std::get<2>(t);
        const auto x = std::get<1>(t);
        for (std::size_t q=0;q<3;++q) n.x[q]=x[q];
        boundary_index.emplace(n,boundary.size());
        boundary.push_back(n);
    }
    out.boundary_states = boundary.size();
    if (boundary.empty()) {
        out.obstruction = "x^3-2x^2-2 contact boundary unexpectedly empty";
        return out;
    }

    for (auto& g : out.boundary_generators) g = zero_matrix(boundary.size(),boundary.size());
    for (auto& g : out.universal_generators) g = zero_matrix(9,9);

    std::map<WitnessKey,long long> universal_witnesses;
    for (long long i=0;i<3;++i) for (long long j=0;j<3;++j) {
        const auto lp = parent_decompositions<3>(subst.images,i);
        const auto rp = parent_decompositions<3>(subst.images,j);
        for (const auto& a : lp) for (const auto& b : rp) {
            const int g = generator_from_prefixes(a.p,b.p);
            const auto s = role(i,j);
            const auto t = role(a.parent_letter,b.parent_letter);
            ++out.universal_generators[static_cast<std::size_t>(g)][s][t];
            ++universal_witnesses[{s,t,g}];
            ++out.universal_edges;
        }
    }
    out.parent_catalogue_complete = !universal_witnesses.empty();

    out.every_boundary_edge_has_universal_witness = true;
    for (std::size_t s=0;s<boundary.size();++s) {
        const auto& src = boundary[s];
        for (const auto& [dst,pq] : simple_forward_targets_exact<3>(subst,src)) {
            const auto di = boundary_index.find(dst);
            if (di == boundary_index.end()) continue;
            const int g = generator_from_prefixes(pq.first,pq.second);
            ++out.boundary_generators[static_cast<std::size_t>(g)][s][di->second];
            ++out.boundary_edges;
            if (g == gen_index(2)) out.boundary_realizes_defect_plus2 = true;
            if (g == gen_index(-2)) out.boundary_realizes_defect_minus2 = true;
            const auto sr = role(src.i,src.j);
            const auto tr = role(dst.i,dst.j);
            if (!universal_witnesses.count({sr,tr,g}))
                out.every_boundary_edge_has_universal_witness = false;
        }
    }

    out.intertwiner = zero_matrix(boundary.size(),9);
    out.base_role_projection_exact = true;
    for (std::size_t s=0;s<boundary.size();++s) {
        const auto& n=boundary[s];
        if (n.i<0 || n.i>=3 || n.j<0 || n.j>=3) {
            out.base_role_projection_exact=false;
            out.obstruction="x^3-2x^2-2 boundary role outside alphabet";
            return out;
        }
        out.intertwiner[s][role(n.i,n.j)] = 1;
    }

    const auto check_generator = [&](std::size_t g) {
        const auto lhs = condition_f_pair_boundary_detail::rectangular_product(
            out.boundary_generators[g],out.intertwiner);
        const auto rhs = condition_f_pair_boundary_detail::rectangular_product(
            out.intertwiner,out.universal_generators[g]);
        return condition_f_pair_boundary_detail::leq(lhs,rhs);
    };
    out.simultaneous_five_generator_intertwiner = true;
    for (std::size_t g=0; g<5; ++g) {
        out.generator_intertwines[g] = check_generator(g);
        out.simultaneous_five_generator_intertwiner &= out.generator_intertwines[g];
    }

    std::vector<NormWeightedGrammarChannel> channels;
    for (std::size_t g=0;g<5;++g)
        for (std::size_t s=0;s<9;++s)
            for (std::size_t t=0;t<9;++t)
                if (const auto m=out.universal_generators[g][s][t]; m>0)
                    channels.push_back({s,t,g,static_cast<std::size_t>(m),{1,1}});
    const auto grammar = derive_finite_positive_grammar_majorant(9,5,channels);
    out.finite_positive_grammar_ready = grammar.proved;

    out.proved = out.parent_catalogue_complete &&
        out.every_boundary_edge_has_universal_witness &&
        out.base_role_projection_exact &&
        out.simultaneous_five_generator_intertwiner &&
        out.finite_positive_grammar_ready;
    if (!out.proved && out.obstruction.empty())
        out.obstruction = "x^3-2x^2-2 five-generator comparison invariant failed";
    return out;
}

} // namespace ravel::proof
