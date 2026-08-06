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
#include "ravel/supergolden_pisot_substitution.hpp"
#include "ravel/proof/condition_f_pair_boundary_substitution.hpp"
#include "ravel/proof/finite_positive_grammar_majorant.hpp"

// Folds the supergolden number (root of x^3-x^2-1) into the same concrete
// three-generator finite-positive-grammar theorem that plastic and the
// quartic next-Pisot substitution already use -- same shape as
// plastic_three_generator_intertwiner.hpp, retargeted at the substitution
// derived (not copied) in supergolden_pisot_substitution.hpp /
// supergolden_qrs_closure.hpp.

namespace ravel::proof {

struct SupergoldenThreeGeneratorIntertwinerCertificate {
    std::size_t generator_count = 3;
    std::size_t boundary_states = 0;
    std::size_t universal_roles = 9;
    long long boundary_edges = 0;
    long long universal_edges = 0;
    NonnegativeMatrix intertwiner;
    std::array<NonnegativeMatrix,3> boundary_generators;
    std::array<NonnegativeMatrix,3> universal_generators;
    bool parent_catalogue_complete = false;
    bool every_boundary_edge_has_universal_witness = false;
    bool base_role_projection_exact = false;
    bool g0_intertwines = false;
    bool gplus_intertwines = false;
    bool gminus_intertwines = false;
    bool simultaneous_three_generator_intertwiner = false;
    bool finite_positive_grammar_ready = false;
    bool proved = false;
    std::string obstruction;
};

namespace supergolden_three_generator_intertwiner_detail {

inline NonnegativeMatrix zero_matrix(std::size_t n, std::size_t m) {
    return NonnegativeMatrix(n, std::vector<long long>(m, 0));
}

inline std::size_t role(long long i, long long j) {
    return static_cast<std::size_t>(i) * 3 + static_cast<std::size_t>(j);
}

inline int generator_from_prefixes(const std::vector<long long>& left,
                                   const std::vector<long long>& right) {
    const auto dl = abelianization<3>(left);
    const auto dr = abelianization<3>(right);
    const std::array<long long,3> delta{
        dr[0]-dl[0], dr[1]-dl[1], dr[2]-dl[2]};
    if (delta == std::array<long long,3>{0,0,0}) return 0;
    if (delta == std::array<long long,3>{1,0,0}) return 1;
    if (delta == std::array<long long,3>{-1,0,0}) return 2;
    throw std::runtime_error("supergolden prefix defect outside {0,+a,-a}");
}

using WitnessKey = std::tuple<std::size_t,std::size_t,int>;

} // namespace supergolden_three_generator_intertwiner_detail

/** Exact three-generator comparison for the supergolden substitution.
 *
 * Same construction as `derive_plastic_three_generator_intertwiner`: the
 * universal core has one state per ordered parent-letter role (i,j); every
 * ordered pair of parent decompositions contributes one edge labelled by its
 * exact prefix defect 0,+a,-a; the concrete contact-boundary graph is a
 * deletion-only subgrammar under the displacement-forgetting role
 * projection P; and G0_B P <= P G0_U, G+_B P <= P G+_U, G-_B P <= P G-_U are
 * checked directly, not assumed.
 */
inline SupergoldenThreeGeneratorIntertwinerCertificate
 derive_supergolden_three_generator_intertwiner() {
    using namespace supergolden_three_generator_intertwiner_detail;
    SupergoldenThreeGeneratorIntertwinerCertificate out;

    SubstitutionRule rule(supergolden_pisot_rule());
    constexpr double beta = 1.4655712318767669;
    constexpr double b2 = 0.8260313576541878;
    ContactBoundaryLimits lim;
    lim.closure_cap = 20000;
    lim.corona_cap = 100000;
    lim.max_corona_rounds = 10;
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
        out.obstruction = "supergolden contact boundary unexpectedly empty";
        return out;
    }

    for (auto& g : out.boundary_generators)
        g = zero_matrix(boundary.size(),boundary.size());
    for (auto& g : out.universal_generators)
        g = zero_matrix(9,9);

    // Universal ordered parent-pair catalogue.
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

    // Concrete exact boundary graph and witness containment.
    out.every_boundary_edge_has_universal_witness = true;
    for (std::size_t s=0;s<boundary.size();++s) {
        const auto& src = boundary[s];
        for (const auto& [dst,pq] : simple_forward_targets_exact<3>(subst,src)) {
            const auto di = boundary_index.find(dst);
            if (di == boundary_index.end()) continue;
            const int g = generator_from_prefixes(pq.first,pq.second);
            ++out.boundary_generators[static_cast<std::size_t>(g)][s][di->second];
            ++out.boundary_edges;
            const auto sr = role(src.i,src.j);
            const auto tr = role(dst.i,dst.j);
            if (!universal_witnesses.count({sr,tr,g}))
                out.every_boundary_edge_has_universal_witness = false;
        }
    }

    // Forget displacement; preserve ordered parent role.
    out.intertwiner = zero_matrix(boundary.size(),9);
    out.base_role_projection_exact = true;
    for (std::size_t s=0;s<boundary.size();++s) {
        const auto& n=boundary[s];
        if (n.i<0 || n.i>=3 || n.j<0 || n.j>=3) {
            out.base_role_projection_exact=false;
            out.obstruction="supergolden boundary role outside alphabet";
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
    out.g0_intertwines = check_generator(0);
    out.gplus_intertwines = check_generator(1);
    out.gminus_intertwines = check_generator(2);
    out.simultaneous_three_generator_intertwiner =
        out.g0_intertwines && out.gplus_intertwines && out.gminus_intertwines;

    // The universal matrices are already a concrete finite positive grammar.
    std::vector<NormWeightedGrammarChannel> channels;
    for (std::size_t g=0;g<3;++g)
        for (std::size_t s=0;s<9;++s)
            for (std::size_t t=0;t<9;++t)
                if (const auto m=out.universal_generators[g][s][t]; m>0)
                    channels.push_back({s,t,g,static_cast<std::size_t>(m),{1,1}});
    const auto grammar = derive_finite_positive_grammar_majorant(9,3,channels);
    out.finite_positive_grammar_ready = grammar.proved;

    out.proved = out.parent_catalogue_complete &&
        out.every_boundary_edge_has_universal_witness &&
        out.base_role_projection_exact &&
        out.simultaneous_three_generator_intertwiner &&
        out.finite_positive_grammar_ready;
    if (!out.proved && out.obstruction.empty())
        out.obstruction = "supergolden three-generator comparison invariant failed";
    return out;
}

} // namespace ravel::proof
