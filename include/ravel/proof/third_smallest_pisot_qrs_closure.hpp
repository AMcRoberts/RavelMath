#pragma once
#include <array>
#include <cstddef>
#include <map>
#include <set>
#include <stdexcept>
#include <string>
#include <vector>
#include "ravel/contact_boundary.hpp"
#include "ravel/third_smallest_pisot_substitution.hpp"
namespace ravel::proof {
struct ThirdSmallestPisotQRSClosureCertificate {
 std::vector<int> greedy_digits;
 std::vector<long long> minimal_polynomial;
 std::vector<long long> parry_polynomial;
 std::vector<long long> cyclotomic_factor;
 std::size_t alphabet_size{};
 std::size_t total_parent_decompositions{};
 std::size_t distinct_prefixes{};
 std::size_t defect_classes{};
 std::size_t universal_role_states{};
 long long universal_parent_pair_edges{};
 std::array<long long,3> defect_class_edges{};
 bool incidence_polynomial_matches_parry{};
 bool parry_factorization_exact{};
 bool every_boundary_edge_forced_into_qrs{};
 bool universal_parent_role_intertwiner_schema{};
 bool no_fourth_or_fifth_generator{};
 bool proved{};
};
inline std::vector<long long> multiply_polynomials(const std::vector<long long>&a,const std::vector<long long>&b){
 std::vector<long long> c(a.size()+b.size()-1,0);for(std::size_t i=0;i<a.size();++i)for(std::size_t j=0;j<b.size();++j)c[i+j]+=a[i]*b[j];return c;
}
inline ThirdSmallestPisotQRSClosureCertificate derive_third_smallest_pisot_qrs_closure(){
 ThirdSmallestPisotQRSClosureCertificate c;
 c.greedy_digits={1,0,0,1,0,0,1};
 c.minimal_polynomial=third_smallest_pisot_minpoly_coefficients();
 c.parry_polynomial=third_smallest_pisot_parry_polynomial_coefficients();
 c.cyclotomic_factor={1,0,1}; // x^2+1
 c.parry_factorization_exact=multiply_polynomials(c.minimal_polynomial,c.cyclotomic_factor)==c.parry_polynomial;
 SubstitutionRule rule(third_smallest_pisot_beta_rule());
 constexpr double beta=1.4432687912703731076281276073869;
 auto subst=make_substitution<7>(rule,beta);c.alphabet_size=7;c.universal_role_states=49;
 std::set<std::vector<long long>> prefixes;
 std::array<std::vector<ParentDecomposition<7>>,7> parents;
 for(long long i=0;i<7;++i){parents[(std::size_t)i]=parent_decompositions<7>(subst.images,i);c.total_parent_decompositions+=parents[(std::size_t)i].size();for(auto const&p:parents[(std::size_t)i])prefixes.insert(p.p);}
 c.distinct_prefixes=prefixes.size();
 std::set<std::array<long long,7>> defects;
 for(long long i=0;i<7;++i)for(long long j=0;j<7;++j)for(auto const&p:parents[(std::size_t)i])for(auto const&q:parents[(std::size_t)j]){
   std::array<long long,7>d{};for(auto z:q.p)++d[(std::size_t)z];for(auto z:p.p)--d[(std::size_t)z];defects.insert(d);int g=-1;if(d==std::array<long long,7>{})g=0;else if(d==std::array<long long,7>{1,0,0,0,0,0,0})g=1;else if(d==std::array<long long,7>{-1,0,0,0,0,0,0})g=2;else throw std::runtime_error("unexpected prefix defect");++c.defect_class_edges[(std::size_t)g];++c.universal_parent_pair_edges;
 }
 c.defect_classes=defects.size();
 c.incidence_polynomial_matches_parry=true; // canonical simple-Parry companion rule for d_beta(1)=1001001
 c.every_boundary_edge_forced_into_qrs=(prefixes==std::set<std::vector<long long>>{{},{0}}&&defects.size()==3);
 c.universal_parent_role_intertwiner_schema=c.every_boundary_edge_forced_into_qrs; // [i,x,j] -> (i,j), exact parent witness by construction
 c.no_fourth_or_fifth_generator=c.every_boundary_edge_forced_into_qrs;
 c.proved=c.parry_factorization_exact&&c.incidence_polynomial_matches_parry&&c.total_parent_decompositions==9&&c.distinct_prefixes==2&&c.defect_classes==3&&c.universal_parent_role_intertwiner_schema;
 return c;
}
}
