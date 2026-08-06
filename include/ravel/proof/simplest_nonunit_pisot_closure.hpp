#pragma once
#include <array>
#include <cstddef>
#include <map>
#include <set>
#include <stdexcept>
#include <string>
#include <vector>
#include "math/qbeta.hpp"
#include "math/sturm.hpp"
#include "math/charpoly.hpp"
#include "ravel/canonical_beta_substitution.hpp"
#include "ravel/contact_boundary.hpp"
#include "ravel/simplest_nonunit_pisot_substitution.hpp"

// The simplest non-unit Pisot substitution: x^2-2x-2, digits (2,2),
// substitution 0->001, 1->00 -- derived via the same reusable
// canonical-beta-substitution operation as supergolden, not hand-typed.
//
// This is also the first substitution examined by this project whose
// greedy digits are NOT all 0/1: with beta in (2,3), a digit value of 2
// is possible, and it appears here at BOTH positions (the only way to
// get a length-2 non-unit sequence at all). That makes the raw parent-
// prefix classification richer than the Q/R/S cases: THREE distinct
// prefixes ([], [0], [0,0]), not two, and FIVE raw defect classes
// {-2,-1,0,+1,+2}, not three. The question this closure actually answers
// is whether all five survive as genuine primitive generators, or
// whether (as with quartic's spurious fourth colour) some are neutral-
// kernel refinements -- here, reducible words in the +-1 generators.

namespace ravel::proof {

struct SimplestNonunitPisotClosureCertificate {
 std::vector<long long> greedy_digits;
 std::vector<long long> minimal_polynomial;
 std::vector<long long> incidence_polynomial;
 std::size_t alphabet_size{};
 std::size_t total_parent_decompositions{};
 std::size_t distinct_prefixes{};
 std::size_t raw_defect_classes{};
 std::size_t universal_role_states{};
 long long universal_parent_pair_edges{};
 std::map<long long,long long> defect_class_edges; // defect value -> edge count
 bool incidence_polynomial_matches_minpoly_exactly{};
 // Domination check: is every |defect|>=2 class entrywise dominated by
 // the corresponding power of the +-1 generator, i.e. reducible (not a
 // genuine new primitive generator)?
 bool plus2_dominated_by_plus1_squared{};
 bool minus2_dominated_by_minus1_squared{};
 bool exactly_three_primitive_generators{}; // conclusion: still Q/R/S, not five
 bool proved{};
};

inline SimplestNonunitPisotClosureCertificate derive_simplest_nonunit_pisot_closure(){
 using namespace mathlib;
 SimplestNonunitPisotClosureCertificate c;

 QBetaRing R = QBetaRing::from_low_first({-2,-2}); // x^2 - 2x - 2
 RootInterval beta_I = isolate_beta(R);
 auto ge = exact_greedy_beta_expansion_of_one(R, beta_I, 32);
 if (!ge.terminated) throw std::runtime_error("simplest nonunit: greedy expansion did not terminate as expected");
 c.greedy_digits = ge.digits;
 auto derived_rule_ll = canonical_beta_substitution_from_digits(ge.digits);
 c.alphabet_size = derived_rule_ll.size();
 c.minimal_polynomial = simplest_nonunit_pisot_minpoly_coefficients();

 std::vector<std::vector<long long>> M(c.alphabet_size, std::vector<long long>(c.alphabet_size,0));
 for (std::size_t i=0;i<c.alphabet_size;++i) for (auto letter : derived_rule_ll[i]) M[i][(std::size_t)letter]++;
 PolyZ chi = charpoly_faddeev_leverrier(M);
 c.incidence_polynomial.resize((std::size_t)chi.degree()+1);
 for (long long i=0;i<=chi.degree();++i) c.incidence_polynomial[(std::size_t)i] = mpz_get_si(chi.coeff((std::size_t)i).get());
 c.incidence_polynomial_matches_minpoly_exactly = (c.incidence_polynomial == c.minimal_polynomial);

 auto recorded = simplest_nonunit_pisot_rule();
 if (recorded.size() != derived_rule_ll.size()) throw std::runtime_error("simplest nonunit: recorded/derived alphabet size mismatch");
 for (std::size_t i=0;i<recorded.size();++i){
   if (recorded[i].size()!=derived_rule_ll[i].size()) throw std::runtime_error("simplest nonunit: recorded/derived image length mismatch");
   for (std::size_t k=0;k<recorded[i].size();++k) if ((long long)recorded[i][k]!=derived_rule_ll[i][k]) throw std::runtime_error("simplest nonunit: recorded/derived image mismatch");
 }

 // Parent-prefix catalogue (generic, dimension-independent scan).
 const std::size_t n = c.alphabet_size;
 std::vector<std::vector<std::pair<long long,std::vector<long long>>>> parents(n); // [inner] -> (parent_letter, prefix)
 std::set<std::vector<long long>> prefixes;
 for (std::size_t cletter=0; cletter<n; ++cletter) {
   const auto& img = recorded[cletter];
   for (std::size_t k=0;k<img.size();++k) {
     auto inner = (std::size_t)img[k];
     std::vector<long long> pre(img.begin(), img.begin()+(long long)k);
     parents[inner].push_back({(long long)cletter, pre});
     prefixes.insert(pre);
     ++c.total_parent_decompositions;
   }
 }
 c.distinct_prefixes = prefixes.size();
 c.universal_role_states = n*n;

 auto role=[&](long long i,long long j){ return (std::size_t)(i*(long long)n+j); };
 std::map<long long, std::vector<std::vector<long long>>> G;
 for (long long d=-2; d<=2; ++d) G[d] = std::vector<std::vector<long long>>(n*n, std::vector<long long>(n*n,0));
 std::set<long long> defects;
 for (long long i=0;i<(long long)n;++i) for (long long j=0;j<(long long)n;++j)
   for (auto const& [pc,pp] : parents[(std::size_t)i]) for (auto const& [qc,qp] : parents[(std::size_t)j]) {
     long long defect = (long long)qp.size() - (long long)pp.size();
     if (defect < -2 || defect > 2) throw std::runtime_error("simplest nonunit: unexpected defect outside [-2,2]");
     defects.insert(defect);
     ++c.defect_class_edges[defect];
     ++c.universal_parent_pair_edges;
     G[defect][role(i,j)][role(pc,qc)]++;
   }
 c.raw_defect_classes = defects.size();

 auto matmul=[&](const std::vector<std::vector<long long>>&A, const std::vector<std::vector<long long>>&B){
   std::vector<std::vector<long long>> C(n*n, std::vector<long long>(n*n,0));
   for (std::size_t i=0;i<n*n;++i) for (std::size_t j=0;j<n*n;++j){ long long s=0; for (std::size_t k=0;k<n*n;++k) s+=A[i][k]*B[k][j]; C[i][j]=s; }
   return C;
 };
 auto sq1 = matmul(G[1],G[1]);
 c.plus2_dominated_by_plus1_squared = true;
 for (std::size_t i=0;i<n*n;++i) for (std::size_t j=0;j<n*n;++j) if (G[2][i][j] > sq1[i][j]) c.plus2_dominated_by_plus1_squared = false;
 auto sqm1 = matmul(G[-1],G[-1]);
 c.minus2_dominated_by_minus1_squared = true;
 for (std::size_t i=0;i<n*n;++i) for (std::size_t j=0;j<n*n;++j) if (G[-2][i][j] > sqm1[i][j]) c.minus2_dominated_by_minus1_squared = false;

 c.exactly_three_primitive_generators = c.plus2_dominated_by_plus1_squared && c.minus2_dominated_by_minus1_squared;

 c.proved = c.incidence_polynomial_matches_minpoly_exactly &&
   c.distinct_prefixes==3 && c.raw_defect_classes==5 &&
   c.exactly_three_primitive_generators;
 return c;
}
}
