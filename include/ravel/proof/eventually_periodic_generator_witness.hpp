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

// Concrete contact-boundary check for the first EVENTUALLY PERIODIC
// (genuine preperiod > 0) Pisot number examined by this project:
// x^3-2x^2-x+1, beta~2.24698, preperiod (2), period (0,1), substitution
// 0->001, 1->2, 2->01 (wraps to state 1, the cycle start, not to 0).
//
// The abstract role-catalogue check (see the accompanying probe/diary
// entry) found the closed-form digit-difference theorem
// (generator_set_from_digit_differences.hpp) does NOT directly predict
// this case correctly: that theorem assumed only letter 0 is flexible
// (multiple parent-decomposition lengths), true for every terminating
// expansion, but false here -- the cycle-start letter (here, letter 1)
// gains a SECOND occurrence via the wraparound closing edge, making it
// flexible too. Despite that extra flexibility, a genuine +2/-2
// generator still survives, through a different, subtler mechanism:
// the only length-1 escape from the resulting bottleneck role leads to
// a role whose own only occupant letter is fully rigid with no self-
// loop, closing off the decomposition path one step later than in the
// terminating-expansion case.

namespace ravel::proof {

struct EventuallyPeriodicGeneratorWitnessCertificate {
    std::size_t boundary_states = 0;
    long long boundary_edges = 0;
    long long universal_edges = 0;
    bool boundary_realizes_defect_plus2 = false;
    bool boundary_realizes_defect_minus2 = false;
    std::array<bool,5> generator_intertwines{};  // index 0..4 = defect -2..+2
    bool simultaneous_intertwiner = false;
    bool finite_positive_grammar_ready = false;
    bool proved = false;
    std::string obstruction;
};

namespace eventually_periodic_generator_witness_detail {
inline NonnegativeMatrix zero_matrix(std::size_t n, std::size_t m) {
    return NonnegativeMatrix(n, std::vector<long long>(m, 0));
}
inline std::size_t role(long long i, long long j) { return static_cast<std::size_t>(i) * 3 + static_cast<std::size_t>(j); }
inline int gen_index(long long defect) {
    if (defect < -2 || defect > 2) throw std::runtime_error("x^3-2x^2-x+1: unexpected defect outside [-2,2]");
    return static_cast<int>(defect + 2);
}
inline int generator_from_prefixes(const std::vector<long long>& left, const std::vector<long long>& right) {
    return gen_index(static_cast<long long>(right.size()) - static_cast<long long>(left.size()));
}
using WitnessKey = std::tuple<std::size_t,std::size_t,int>;
} // namespace eventually_periodic_generator_witness_detail

inline EventuallyPeriodicGeneratorWitnessCertificate
 derive_eventually_periodic_generator_witness() {
    using namespace eventually_periodic_generator_witness_detail;
    EventuallyPeriodicGeneratorWitnessCertificate out;

    // 0->001, 1->2, 2->01
    SubstitutionRule rule(std::vector<std::vector<std::int8_t>>{{0,0,1},{2},{0,1}});
    constexpr double beta = 2.2469796037174672;
    constexpr double b2 = 0.8019377358048384;  // ravel::spectral_invariants_3x3 on M[dest][source]
    ContactBoundaryLimits lim;
    lim.closure_cap = 40000;
    lim.corona_cap = 200000;
    lim.max_corona_rounds = 14;
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
        out.obstruction = "x^3-2x^2-x+1 contact boundary unexpectedly empty";
        return out;
    }

    std::array<NonnegativeMatrix,5> boundary_generators, universal_generators;
    for (auto& g : boundary_generators) g = zero_matrix(boundary.size(),boundary.size());
    for (auto& g : universal_generators) g = zero_matrix(9,9);

    std::map<WitnessKey,long long> universal_witnesses;
    for (long long i=0;i<3;++i) for (long long j=0;j<3;++j) {
        const auto lp = parent_decompositions<3>(subst.images,i);
        const auto rp = parent_decompositions<3>(subst.images,j);
        for (const auto& a : lp) for (const auto& b : rp) {
            const int g = generator_from_prefixes(a.p,b.p);
            const auto s = role(i,j);
            const auto t = role(a.parent_letter,b.parent_letter);
            ++universal_generators[static_cast<std::size_t>(g)][s][t];
            ++universal_witnesses[{s,t,g}];
            ++out.universal_edges;
        }
    }

    bool every_boundary_edge_has_universal_witness = true;
    for (std::size_t s=0;s<boundary.size();++s) {
        const auto& src = boundary[s];
        for (const auto& [dst,pq] : simple_forward_targets_exact<3>(subst,src)) {
            const auto di = boundary_index.find(dst);
            if (di == boundary_index.end()) continue;
            const int g = generator_from_prefixes(pq.first,pq.second);
            ++boundary_generators[static_cast<std::size_t>(g)][s][di->second];
            ++out.boundary_edges;
            if (g == gen_index(2)) out.boundary_realizes_defect_plus2 = true;
            if (g == gen_index(-2)) out.boundary_realizes_defect_minus2 = true;
            const auto sr = role(src.i,src.j);
            const auto tr = role(dst.i,dst.j);
            if (!universal_witnesses.count({sr,tr,g})) every_boundary_edge_has_universal_witness = false;
        }
    }

    NonnegativeMatrix intertwiner = zero_matrix(boundary.size(),9);
    bool base_role_projection_exact = true;
    for (std::size_t s=0;s<boundary.size();++s) {
        const auto& n=boundary[s];
        if (n.i<0 || n.i>=3 || n.j<0 || n.j>=3) { base_role_projection_exact=false; break; }
        intertwiner[s][role(n.i,n.j)] = 1;
    }

    const auto check_generator = [&](std::size_t g) {
        const auto lhs = condition_f_pair_boundary_detail::rectangular_product(boundary_generators[g],intertwiner);
        const auto rhs = condition_f_pair_boundary_detail::rectangular_product(intertwiner,universal_generators[g]);
        return condition_f_pair_boundary_detail::leq(lhs,rhs);
    };
    out.simultaneous_intertwiner = true;
    for (std::size_t g=0; g<5; ++g) {
        out.generator_intertwines[g] = check_generator(g);
        out.simultaneous_intertwiner &= out.generator_intertwines[g];
    }

    std::vector<NormWeightedGrammarChannel> channels;
    for (std::size_t g=0;g<5;++g)
        for (std::size_t s=0;s<9;++s)
            for (std::size_t t=0;t<9;++t)
                if (const auto m=universal_generators[g][s][t]; m>0)
                    channels.push_back({s,t,g,static_cast<std::size_t>(m),{1,1}});
    const auto grammar = derive_finite_positive_grammar_majorant(9,5,channels);
    out.finite_positive_grammar_ready = grammar.proved;

    out.proved = every_boundary_edge_has_universal_witness && base_role_projection_exact &&
        out.simultaneous_intertwiner && out.finite_positive_grammar_ready;
    if (!out.proved && out.obstruction.empty())
        out.obstruction = "x^3-2x^2-x+1 five-generator comparison invariant failed";
    return out;
}

} // namespace ravel::proof
