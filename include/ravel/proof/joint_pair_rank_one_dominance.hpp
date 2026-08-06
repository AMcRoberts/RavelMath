#pragma once

#include <cstddef>
#include <string>
#include <utility>
#include <vector>

#include "ravel/proof/paired_matrix_dominance.hpp"

namespace ravel::proof {

struct JointPairRankOneDominance {
    std::size_t competitor_vertices = 0;
    std::size_t core_vertices = 0;
    std::size_t alpha = 0;
    std::size_t beta = 0;
    std::vector<mathlib::BigInt> competitor_weight;
    std::vector<mathlib::BigInt> core_left_weight;
    bool q_intertwiner = false;
    bool r_intertwiner = false;
    bool q_strict = false;
    bool r_strict = false;
    bool positive = false;
    bool proved = false;
    std::string obstruction;
};

namespace joint_pair_rank_one_detail {

using Matrix = std::vector<std::vector<long long>>;

inline bool same_square(const Matrix& A, const Matrix& B) {
    if (A.empty() || A.size() != B.size()) return false;
    const auto n=A.size();
    for (const auto& row:A) if(row.size()!=n) return false;
    for (const auto& row:B) if(row.size()!=n) return false;
    return true;
}

inline Matrix linear_combination(const Matrix& Q,const Matrix& R,
                                 std::size_t alpha,std::size_t beta) {
    if(!same_square(Q,R)) throw std::invalid_argument("joint pair: malformed pair");
    Matrix A=Q;
    for(std::size_t i=0;i<A.size();++i) for(std::size_t j=0;j<A.size();++j) {
        if(Q[i][j]<0 || R[i][j]<0) throw std::invalid_argument("joint pair: negative entry");
        A[i][j]=static_cast<long long>(alpha)*Q[i][j]+static_cast<long long>(beta)*R[i][j];
    }
    return A;
}

inline bool positive(const std::vector<mathlib::BigInt>& v) {
    for(const auto& x:v) if(mathlib::sgn(x)<=0) return false;
    return !v.empty();
}

struct Check { bool leq=true; bool strict=false; };
inline Check check_rank_one(const Matrix& A,const Matrix& B,
                            const std::vector<mathlib::BigInt>& u,
                            const std::vector<mathlib::BigInt>& v) {
    Check out;
    const auto Au=exact_matrix_vector_product(A,u);
    const auto Bt=transpose_nonnegative_matrix(B);
    const auto Btv=exact_matrix_vector_product(Bt,v);
    for(std::size_t i=0;i<u.size();++i) for(std::size_t j=0;j<v.size();++j) {
        mathlib::BigInt lhs,rhs;
        mathlib::mul(lhs,Au[i],v[j]);
        mathlib::mul(rhs,u[i],Btv[j]);
        const auto c=mathlib::cmp(lhs,rhs);
        if(c>0) out.leq=false;
        if(c<0) out.strict=true;
    }
    return out;
}

} // namespace joint_pair_rank_one_detail

/** Derive one positive rank-one rectangular intertwiner for a Q/R pair.
 *
 * P=u v^T must satisfy both
 *
 *   Qc P <= P Qk,   Rc P <= P Rk.
 *
 * The positive factors are derived from a small exact family of weighted
 * combinations alpha*Q+beta*R.  The combination is only a weight generator;
 * the two generator inequalities are replayed separately and exactly.
 */
inline JointPairRankOneDominance derive_joint_pair_rank_one_dominance(
    const joint_pair_rank_one_detail::Matrix& competitor_q,
    const joint_pair_rank_one_detail::Matrix& competitor_r,
    const joint_pair_rank_one_detail::Matrix& core_q,
    const joint_pair_rank_one_detail::Matrix& core_r,
    std::size_t iterations=120,
    std::size_t coefficient_bound=16) {
    using namespace joint_pair_rank_one_detail;
    JointPairRankOneDominance best;
    if(!same_square(competitor_q,competitor_r) || !same_square(core_q,core_r)) {
        best.obstruction="joint pair comparison requires two square matrix pairs";
        return best;
    }
    best.competitor_vertices=competitor_q.size();
    best.core_vertices=core_q.size();
    for(std::size_t alpha=1;alpha<=coefficient_bound;++alpha)
      for(std::size_t beta=1;beta<=coefficient_bound;++beta) {
        const auto A=linear_combination(competitor_q,competitor_r,alpha,beta);
        const auto B=linear_combination(core_q,core_r,alpha,beta);
        const auto aw=derive_reflective_collatz_weight(A,iterations);
        const auto bw=derive_reflective_collatz_weight(transpose_nonnegative_matrix(B),iterations);
        if(!aw.replayed || !bw.replayed || !positive(aw.weight) || !positive(bw.weight)) continue;
        const auto q=check_rank_one(competitor_q,core_q,aw.weight,bw.weight);
        const auto r=check_rank_one(competitor_r,core_r,aw.weight,bw.weight);
        if(q.leq && r.leq) {
            best.alpha=alpha; best.beta=beta;
            best.competitor_weight=aw.weight;
            best.core_left_weight=bw.weight;
            best.q_intertwiner=true; best.r_intertwiner=true;
            best.q_strict=q.strict; best.r_strict=r.strict;
            best.positive=true; best.proved=true;
            return best;
        }
    }
    best.obstruction="no common positive rank-one Q/R intertwiner found in exact coefficient search";
    return best;
}

} // namespace ravel::proof
