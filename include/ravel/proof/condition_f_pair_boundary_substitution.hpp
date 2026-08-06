#pragma once

#include <algorithm>
#include <cstddef>
#include <map>
#include <stdexcept>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

#include "ravel/proof/condition_f_local_channel_recurrence.hpp"

namespace ravel::proof {

using NonnegativeMatrix = std::vector<std::vector<long long>>;

struct ConditionFPairWord {
    std::vector<std::size_t> generators; // 0=Q, 1=R
    std::vector<long long> signed_defects;

    bool operator<(const ConditionFPairWord& other) const {
        return std::tie(generators, signed_defects) <
               std::tie(other.generators, other.signed_defects);
    }
};

struct ConditionFBoundaryExcursion {
    std::pair<std::size_t,std::size_t> source;
    std::pair<std::size_t,std::size_t> target;
    ConditionFPairWord word;
    long long net_defect = 0;
};

struct ConditionFPairBoundarySubstitution {
    std::size_t dimension = 0;
    std::vector<ConditionFBoundaryExcursion> excursions;
    std::map<ConditionFPairWord,std::size_t> word_multiplicity;
    std::size_t length_two = 0;
    std::size_t length_three = 0;
    std::size_t net_q = 0;
    std::size_t net_r = 0;
    bool old_block_transport_exact = false;
    bool first_return_complete = false;
    bool pair_words_canonical = false;
    bool finite_depth = false;
    bool proved = false;
    std::string obstruction;
};

namespace condition_f_pair_boundary_detail {

inline bool is_old_role(std::size_t D,
                        const std::pair<std::size_t,std::size_t>& role) {
    return role.first < D && role.second < D;
}

inline std::vector<ConditionFLocalChannelCase>
 role_channels(std::size_t alphabet,
               const std::pair<std::size_t,std::size_t>& role) {
    std::vector<ConditionFLocalChannelCase> out;
    const auto left = nbonacci_parent_choices(alphabet, role.first);
    const auto right = nbonacci_parent_choices(alphabet, role.second);
    for (const auto& a : left) for (const auto& b : right) {
        const long long delta = static_cast<long long>(b.prefix_bit)-
                                static_cast<long long>(a.prefix_bit);
        out.push_back({role.first,role.second,a.parent,b.parent,
            static_cast<std::size_t>(delta < 0 ? -delta : delta),delta,false});
    }
    return out;
}

inline NonnegativeMatrix rectangular_product(const NonnegativeMatrix& A,
                                             const NonnegativeMatrix& B) {
    if (A.empty() || B.empty() || A.front().empty() || B.front().empty())
        throw std::invalid_argument("pair substitution: empty matrix product");
    const std::size_t m=A.size(), k=A.front().size(), n=B.front().size();
    if (B.size()!=k) throw std::invalid_argument("pair substitution: product mismatch");
    for (const auto& row:A) if(row.size()!=k) throw std::invalid_argument("pair substitution: ragged lhs");
    for (const auto& row:B) if(row.size()!=n) throw std::invalid_argument("pair substitution: ragged rhs");
    NonnegativeMatrix C(m,std::vector<long long>(n,0));
    for(std::size_t i=0;i<m;++i) for(std::size_t t=0;t<k;++t) {
        if(A[i][t]<0) throw std::invalid_argument("pair substitution: negative lhs");
        if(A[i][t]==0) continue;
        for(std::size_t j=0;j<n;++j) {
            if(B[t][j]<0) throw std::invalid_argument("pair substitution: negative rhs");
            C[i][j]+=A[i][t]*B[t][j];
        }
    }
    return C;
}

inline NonnegativeMatrix add_matrix(const NonnegativeMatrix& A,
                                    const NonnegativeMatrix& B) {
    if(A.size()!=B.size() || A.empty()) throw std::invalid_argument("pair substitution: add mismatch");
    NonnegativeMatrix C=A;
    for(std::size_t i=0;i<A.size();++i) {
        if(A[i].size()!=B[i].size()) throw std::invalid_argument("pair substitution: add row mismatch");
        for(std::size_t j=0;j<A[i].size();++j) C[i][j]+=B[i][j];
    }
    return C;
}

inline NonnegativeMatrix zero_like(std::size_t rows,std::size_t cols) {
    return NonnegativeMatrix(rows,std::vector<long long>(cols,0));
}

inline NonnegativeMatrix identity(std::size_t n) {
    NonnegativeMatrix I(n,std::vector<long long>(n,0));
    for(std::size_t i=0;i<n;++i) I[i][i]=1;
    return I;
}

inline bool leq(const NonnegativeMatrix& A,const NonnegativeMatrix& B) {
    if(A.size()!=B.size()) return false;
    for(std::size_t i=0;i<A.size();++i) {
        if(A[i].size()!=B[i].size()) return false;
        for(std::size_t j=0;j<A[i].size();++j) if(A[i][j]>B[i][j]) return false;
    }
    return true;
}

inline NonnegativeMatrix evaluate_word(const NonnegativeMatrix& Q,
                                       const NonnegativeMatrix& R,
                                       const std::vector<std::size_t>& word) {
    if(Q.size()!=R.size() || Q.empty()) throw std::invalid_argument("pair substitution: malformed pair");
    const std::size_t n=Q.size();
    for(const auto& row:Q) if(row.size()!=n) throw std::invalid_argument("pair substitution: nonsquare Q");
    for(const auto& row:R) if(row.size()!=n) throw std::invalid_argument("pair substitution: nonsquare R");
    auto value=identity(n);
    for(const auto g:word) {
        if(g>1) throw std::invalid_argument("pair substitution: generator outside Q/R");
        value=rectangular_product(value,g==0?Q:R);
    }
    return value;
}

} // namespace condition_f_pair_boundary_detail

/** Eliminate the new terminal-parent roles at D+1 without summing Q and R.
 *
 * Roles are parent pairs.  Old roles have both coordinates <D.  A complete
 * excursion starts with an old->boundary channel, stays in the boundary, and
 * ends at its first return to the old roles.  The boundary has depth at most
 * three.  Each excursion records its noncommutative Q/R word and its signed
 * prefix-defect cocycle.  Thus the correction is an exact polynomial in the
 * pair rather than a polynomial in Q+R.
 */
inline ConditionFPairBoundarySubstitution
 derive_condition_f_pair_boundary_substitution(std::size_t D) {
    ConditionFPairBoundarySubstitution out;
    out.dimension=D;
    if(D<2) { out.obstruction="pair boundary substitution requires D>=2"; return out; }
    const std::size_t A=D+1;
    struct Partial {
        std::pair<std::size_t,std::size_t> source;
        std::pair<std::size_t,std::size_t> role;
        ConditionFPairWord word;
        long long net=0;
    };
    std::vector<Partial> frontier;
    for(std::size_t i=0;i<D;++i) for(std::size_t j=0;j<D;++j) {
        const std::pair<std::size_t,std::size_t> source{i,j};
        for(const auto& e:condition_f_pair_boundary_detail::role_channels(A,source)) {
            const std::pair<std::size_t,std::size_t> target{e.left_parent,e.right_parent};
            if(condition_f_pair_boundary_detail::is_old_role(D,target)) continue;
            Partial p{source,target,{{e.generator},{e.signed_defect}},e.signed_defect};
            frontier.push_back(std::move(p));
        }
    }
    std::size_t guard=0;
    while(!frontier.empty()) {
        if(++guard>4) { out.obstruction="terminal-parent boundary depth exceeded three"; return out; }
        std::vector<Partial> next;
        for(const auto& p:frontier) {
            for(const auto& e:condition_f_pair_boundary_detail::role_channels(A,p.role)) {
                const std::pair<std::size_t,std::size_t> target{e.left_parent,e.right_parent};
                auto word=p.word;
                word.generators.push_back(e.generator);
                word.signed_defects.push_back(e.signed_defect);
                const long long net=p.net+e.signed_defect;
                if(condition_f_pair_boundary_detail::is_old_role(D,target)) {
                    out.excursions.push_back({p.source,target,word,net});
                    ++out.word_multiplicity[word];
                    if(word.generators.size()==2) ++out.length_two;
                    else if(word.generators.size()==3) ++out.length_three;
                    else { out.obstruction="boundary excursion has unexpected length"; return out; }
                    if(net==0) ++out.net_q;
                    else if(net==1 || net==-1) ++out.net_r;
                    else { out.obstruction="boundary excursion net defect is outside {-1,0,+1}"; return out; }
                } else {
                    next.push_back({p.source,target,std::move(word),net});
                }
            }
        }
        frontier=std::move(next);
    }
    out.old_block_transport_exact=true;
    out.first_return_complete=out.excursions.size()==8*D-1 &&
        out.length_two==8*D-5 && out.length_three==4;
    out.pair_words_canonical=true;
    for(const auto& e:out.excursions) {
        if(e.word.generators.size()!=e.word.signed_defects.size()) out.pair_words_canonical=false;
        for(std::size_t k=0;k<e.word.generators.size();++k)
            if(e.word.generators[k]!=static_cast<std::size_t>(e.word.signed_defects[k]<0?
                    -e.word.signed_defects[k]:e.word.signed_defects[k]))
                out.pair_words_canonical=false;
    }
    out.finite_depth=out.length_three==4;
    out.proved=out.old_block_transport_exact && out.first_return_complete &&
               out.pair_words_canonical && out.finite_depth;
    return out;
}

struct ConditionFPairSubstitutionMonotonicity {
    std::size_t words_checked=0;
    std::size_t polynomial_terms=0;
    bool q_intertwines=false;
    bool r_intertwines=false;
    bool every_word_intertwines=false;
    bool boundary_polynomial_intertwines=false;
    bool proved=false;
    std::string obstruction;
};

/** Joint Q/R intertwiners are closed under every nonnegative noncommutative
 * polynomial.  In particular, if Qc P<=P Qk and Rc P<=P Rk, the exact
 * terminal-parent boundary substitution preserves the comparison.
 */
inline ConditionFPairSubstitutionMonotonicity
 derive_condition_f_pair_substitution_monotonicity(
    const NonnegativeMatrix& competitor_q,
    const NonnegativeMatrix& competitor_r,
    const NonnegativeMatrix& core_q,
    const NonnegativeMatrix& core_r,
    const NonnegativeMatrix& intertwiner,
    const ConditionFPairBoundarySubstitution& substitution) {
    using namespace condition_f_pair_boundary_detail;
    ConditionFPairSubstitutionMonotonicity out;
    if(!substitution.proved) { out.obstruction="boundary substitution is not proved"; return out; }
    try {
        const auto qc_p=rectangular_product(competitor_q,intertwiner);
        const auto rc_p=rectangular_product(competitor_r,intertwiner);
        const auto p_qk=rectangular_product(intertwiner,core_q);
        const auto p_rk=rectangular_product(intertwiner,core_r);
        out.q_intertwines=leq(qc_p,p_qk);
        out.r_intertwines=leq(rc_p,p_rk);
        if(!out.q_intertwines || !out.r_intertwines) {
            out.obstruction="input pair does not satisfy the joint intertwiner";
            return out;
        }
        const std::size_t m=competitor_q.size(), n=core_q.size();
        auto lhs=zero_like(m,n), rhs=zero_like(m,n);
        out.every_word_intertwines=true;
        for(const auto& [word,multiplicity]:substitution.word_multiplicity) {
            const auto cw=evaluate_word(competitor_q,competitor_r,word.generators);
            const auto kw=evaluate_word(core_q,core_r,word.generators);
            const auto cw_p=rectangular_product(cw,intertwiner);
            const auto p_kw=rectangular_product(intertwiner,kw);
            if(!leq(cw_p,p_kw)) out.every_word_intertwines=false;
            for(std::size_t copy=0;copy<multiplicity;++copy) {
                lhs=add_matrix(lhs,cw_p);
                rhs=add_matrix(rhs,p_kw);
                ++out.polynomial_terms;
            }
            ++out.words_checked;
        }
        out.boundary_polynomial_intertwines=leq(lhs,rhs);
        out.proved=out.every_word_intertwines && out.boundary_polynomial_intertwines;
    } catch(const std::exception& e) { out.obstruction=e.what(); }
    return out;
}

} // namespace ravel::proof
