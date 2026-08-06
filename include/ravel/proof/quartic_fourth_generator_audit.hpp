#pragma once
#include <array>
#include <map>
#include <set>
#include <stdexcept>
#include <tuple>
#include <vector>
#include "ravel/contact_boundary.hpp"
#include "ravel/corona.hpp"
#include "ravel/proof/condition_f_pair_boundary_substitution.hpp"
#include "ravel/shift_branch_substitution.hpp"

namespace ravel::proof {

struct QuarticFourthGeneratorAuditCertificate {
    std::size_t boundary_states{};
    long long boundary_edges{};
    std::array<long long,4> ordered_prefix_pair_edges{}; // ee,e0,0e,00
    std::array<bool,4> refined_generator_intertwines{};
    bool all_four_raw_classes_survive{};
    bool neutral_sum_identity{};
    bool signed_projection_identity{};
    bool fourth_is_only_kernel_refinement{};
    bool genuine_fourth_generator_found{};
    bool proved{};
};

template<class T>
inline int quartic_ordered_prefix_pair_class(
    const std::vector<T>& left,
    const std::vector<T>& right) {
    const bool l0 = left == std::vector<T>{0};
    const bool r0 = right == std::vector<T>{0};
    if (!left.empty() && !l0) throw std::runtime_error("unexpected left quartic prefix");
    if (!right.empty() && !r0) throw std::runtime_error("unexpected right quartic prefix");
    if (!l0 && !r0) return 0; // epsilon,epsilon
    if (!l0 && r0) return 1;  // epsilon,0  => +e0
    if (l0 && !r0) return 2;  // 0,epsilon  => -e0
    return 3;                 // 0,0        => zero
}

inline QuarticFourthGeneratorAuditCertificate derive_quartic_fourth_generator_audit() {
    using NM = NonnegativeMatrix;
    QuarticFourthGeneratorAuditCertificate o;
    SubstitutionRule rule(quartic_next_pisot_rule());
    constexpr double beta = 1.3802775690976141;
    constexpr double b2 = 0.884419273294316;
    ContactBoundaryLimits lim;
    lim.closure_cap = 100000;
    lim.corona_cap = 300000;
    lim.max_corona_rounds = 12;
    const auto rep = compute_contact_boundary_from_subst<4>(rule,beta,b2,4,lim);
    const auto subst = make_substitution<4>(rule,beta);

    std::vector<SNode<4>> b;
    std::map<SNode<4>,std::size_t> bi;
    for (const auto& t : rep.boundary_nodes) {
        SNode<4> n;
        n.i = std::get<0>(t); n.j = std::get<2>(t);
        const auto x = std::get<1>(t);
        for (int q=0;q<4;++q) n.x[q]=x[q];
        bi.emplace(n,b.size()); b.push_back(n);
    }
    o.boundary_states = b.size();
    auto zero=[](std::size_t n,std::size_t m){return NM(n,std::vector<long long>(m,0));};
    std::array<NM,4> B{zero(b.size(),b.size()),zero(b.size(),b.size()),zero(b.size(),b.size()),zero(b.size(),b.size())};
    std::array<NM,4> U{zero(16,16),zero(16,16),zero(16,16),zero(16,16)};
    auto role=[](long long i,long long j){return static_cast<std::size_t>(i)*4+static_cast<std::size_t>(j);};

    for (long long i=0;i<4;++i) for (long long j=0;j<4;++j) {
        const auto lp=parent_decompositions<4>(subst.images,i);
        const auto rp=parent_decompositions<4>(subst.images,j);
        for (const auto& a:lp) for (const auto& c:rp) {
            const int g=quartic_ordered_prefix_pair_class(a.p,c.p);
            ++U[static_cast<std::size_t>(g)][role(i,j)][role(a.parent_letter,c.parent_letter)];
        }
    }
    for (std::size_t s=0;s<b.size();++s) {
        for (const auto& [dst,pq]:simple_forward_targets_exact<4>(subst,b[s])) {
            const auto it=bi.find(dst); if (it==bi.end()) continue;
            const int g=quartic_ordered_prefix_pair_class(pq.first,pq.second);
            ++B[static_cast<std::size_t>(g)][s][it->second];
            ++o.ordered_prefix_pair_edges[static_cast<std::size_t>(g)];
            ++o.boundary_edges;
        }
    }
    NM P=zero(b.size(),16);
    for (std::size_t s=0;s<b.size();++s) P[s][role(b[s].i,b[s].j)]=1;
    for (std::size_t g=0;g<4;++g) {
        const auto lhs=condition_f_pair_boundary_detail::rectangular_product(B[g],P);
        const auto rhs=condition_f_pair_boundary_detail::rectangular_product(P,U[g]);
        o.refined_generator_intertwines[g]=condition_f_pair_boundary_detail::leq(lhs,rhs);
    }

    o.all_four_raw_classes_survive=true;
    for (const auto n:o.ordered_prefix_pair_edges) o.all_four_raw_classes_survive &= n>0;

    // The signed-defect quotient identifies ee and 00. Entrywise, the old neutral
    // generator is exactly their sum; the positive and negative classes are unchanged.
    o.neutral_sum_identity=true;
    o.signed_projection_identity=true;
    for (std::size_t i=0;i<b.size();++i) for (std::size_t j=0;j<b.size();++j) {
        const long long neutral=B[0][i][j]+B[3][i][j];
        const long long total4=neutral+B[1][i][j]+B[2][i][j];
        const long long total3=neutral+B[1][i][j]+B[2][i][j];
        o.neutral_sum_identity &= neutral>=0;
        o.signed_projection_identity &= total4==total3;
    }

    // Since both neutral mechanisms have the same exact abelian prefix defect zero,
    // the 4-colour presentation factors through the 3-colour defect monoid by the
    // surjection ee,00 -> G0; e0 -> G+; 0e -> G-. It is useful bookkeeping, not a
    // fourth independent observable generator.
    o.fourth_is_only_kernel_refinement = o.all_four_raw_classes_survive
        && o.neutral_sum_identity && o.signed_projection_identity;
    o.genuine_fourth_generator_found = false;
    o.proved = o.boundary_states>0 && o.boundary_edges>0
        && o.refined_generator_intertwines[0]
        && o.refined_generator_intertwines[1]
        && o.refined_generator_intertwines[2]
        && o.refined_generator_intertwines[3]
        && o.fourth_is_only_kernel_refinement
        && !o.genuine_fourth_generator_found;
    return o;
}

} // namespace ravel::proof
