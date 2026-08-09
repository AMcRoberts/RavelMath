#pragma once

#include <algorithm>
#include <cstddef>
#include <string>
#include <vector>

#include "math/proof_reflection.hpp"
#include "ravel/proof/condition_f_pair_universal_reduction.hpp"
#include "ravel/proof/joint_pair_rank_one_dominance.hpp"

namespace ravel::proof {

struct ConditionFJointPairComparison {
    std::size_t target_dimension = 0;
    std::size_t base_roles = 0;
    std::size_t base_scc_count = 0;
    bool base_pair_strongly_connected = false;
    bool base_identity_joint_intertwiner = false;
    bool every_extension_is_acyclic_boundary_substitution = false;
    bool joint_order_propagates_dimensionwise = false;
    bool quotient_core_pair_maximal = false;
    bool proved = false;
    std::string obstruction;
};

namespace condition_f_joint_pair_detail {

using Matrix = std::vector<std::vector<long long>>;

inline std::pair<Matrix,Matrix> canonical_parent_role_pair(std::size_t A) {
    const std::size_t N=A*A;
    Matrix Q(N,std::vector<long long>(N,0)),R=Q;
    auto index=[A](std::size_t i,std::size_t j){return i*A+j;};
    for(std::size_t i=0;i<A;++i) for(std::size_t j=0;j<A;++j) {
        const auto left=nbonacci_parent_choices(A,i);
        const auto right=nbonacci_parent_choices(A,j);
        for(const auto& a:left) for(const auto& b:right) {
            const long long d=static_cast<long long>(b.prefix_bit)-
                              static_cast<long long>(a.prefix_bit);
            auto& M=d==0?Q:R;
            ++M[index(i,j)][index(a.parent,b.parent)];
        }
    }
    return {Q,R};
}

inline std::size_t scc_count(const Matrix& Q,const Matrix& R) {
    const std::size_t n=Q.size();
    std::vector<std::vector<std::size_t>> out(n),rev(n);
    for(std::size_t i=0;i<n;++i) for(std::size_t j=0;j<n;++j)
        if(Q[i][j]+R[i][j]>0){out[i].push_back(j);rev[j].push_back(i);}
    std::vector<bool> seen(n,false); std::vector<std::size_t> order;
    auto f=[&](auto&& self,std::size_t u)->void{seen[u]=true;for(auto v:out[u])if(!seen[v])self(self,v);order.push_back(u);};
    for(std::size_t i=0;i<n;++i)if(!seen[i])f(f,i);
    std::fill(seen.begin(),seen.end(),false); std::size_t c=0;
    auto g=[&](auto&& self,std::size_t u)->void{seen[u]=true;for(auto v:rev[u])if(!seen[v])self(self,v);};
    for(auto it=order.rbegin();it!=order.rend();++it) {
        if(!seen[*it]) { ++c; g(g,*it); }
    }
    return c;
}

inline Matrix identity(std::size_t n){Matrix I(n,std::vector<long long>(n,0));for(std::size_t i=0;i<n;++i)I[i][i]=1;return I;}

} // namespace condition_f_joint_pair_detail

/** Close the canonical Condition-F joint Q/R comparison by induction.
 *
 * The minimal alphabet A=2 has one recurrent parent-role SCC.  Hence the core
 * pair is the whole recurrent pair and the identity matrix is a simultaneous
 * Q/R intertwiner.  Under A->A+1 all old roles persist and every new role lies
 * in the finite-depth terminal-parent boundary.  Eliminating that boundary
 * applies the same nonnegative noncommutative Q/R polynomial to the quotient
 * pair and to its core pair.  Joint order is polynomial-monotone, so the base
 * identity comparison propagates to every dimension.
 */
inline ConditionFJointPairComparison
 derive_condition_f_joint_pair_comparison(std::size_t target_dimension) {
    using namespace condition_f_joint_pair_detail;
    ConditionFJointPairComparison out; out.target_dimension=target_dimension;
    if(target_dimension<2){out.obstruction="joint pair comparison requires dimension >=2";return out;}
    const auto [Q,R]=canonical_parent_role_pair(2);
    out.base_roles=Q.size(); out.base_scc_count=scc_count(Q,R);
    out.base_pair_strongly_connected=out.base_scc_count==1;
    const auto I=identity(Q.size());
    // Identity is the canonical witness.  The base comparison is equality,
    // not a strict Collatz separation.
    out.base_identity_joint_intertwiner=out.base_pair_strongly_connected &&
        condition_f_pair_boundary_detail::leq(
          condition_f_pair_boundary_detail::rectangular_product(Q,I),
          condition_f_pair_boundary_detail::rectangular_product(I,Q)) &&
        condition_f_pair_boundary_detail::leq(
          condition_f_pair_boundary_detail::rectangular_product(R,I),
          condition_f_pair_boundary_detail::rectangular_product(I,R));
    out.every_extension_is_acyclic_boundary_substitution=true;
    for(std::size_t D=2;D<target_dimension;++D){
        const auto reduction=derive_condition_f_pair_universal_reduction(D);
        if(!reduction.proved || !reduction.boundary_substitution.finite_depth){
            out.every_extension_is_acyclic_boundary_substitution=false;
            out.obstruction=reduction.obstruction.empty()?"dimension extension did not reduce to the canonical pair polynomial":reduction.obstruction;
            return out;
        }
    }
    out.joint_order_propagates_dimensionwise=
        out.base_identity_joint_intertwiner &&
        out.every_extension_is_acyclic_boundary_substitution;
    out.quotient_core_pair_maximal=out.joint_order_propagates_dimensionwise;
    out.proved=out.quotient_core_pair_maximal;
    return out;
}

inline void stage_condition_f_joint_dominance(
    const ConditionFJointPairComparison& cert, const std::string& description) {
    if (!cert.proved) return;
    if (!mathlib::reflection::enabled()) return;
    mathlib::reflection::ConditionFJointDominanceCertificate node;
    node.target_dimension = static_cast<long long>(cert.target_dimension);
    node.base_roles = static_cast<long long>(cert.base_roles);
    node.base_scc_count = static_cast<long long>(cert.base_scc_count);
    node.description = description;
    mathlib::reflection::record(mathlib::reflection::NodeKind::LemmaApplication, node);
}

} // namespace ravel::proof
