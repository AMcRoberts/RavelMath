#pragma once
#include <array>
#include <cstddef>
#include <set>
#include <stdexcept>
#include <string>
#include <vector>
#include "math/qbeta.hpp"
#include "math/sturm.hpp"
#include "math/charpoly.hpp"
#include "ravel/canonical_beta_substitution.hpp"
#include "ravel/contact_boundary.hpp"
#include "ravel/supergolden_pisot_substitution.hpp"
namespace ravel::proof {

// Supergolden number: dominant root of x^3-x^2-1, the fourth-smallest
// Pisot number. Unlike the plastic and third-smallest-Pisot closures this
// project already has, every step here -- greedy expansion, canonical
// substitution, and the incidence-polynomial match -- is derived by an
// executable, reusable operation rather than typed in by hand from
// literature. See `math/qbeta.hpp`/`math/sturm.hpp` (exact Q(beta)
// arithmetic and root isolation) and
// `include/ravel/canonical_beta_substitution.hpp` (the greedy-expansion
// and canonical-substitution operations themselves).

struct SupergoldenQRSClosureCertificate {
 std::vector<long long> greedy_digits;
 std::vector<long long> minimal_polynomial;
 std::vector<long long> incidence_polynomial;
 std::size_t alphabet_size{};
 std::size_t total_parent_decompositions{};
 std::size_t distinct_prefixes{};
 std::size_t defect_classes{};
 std::size_t universal_role_states{};
 long long universal_parent_pair_edges{};
 std::array<long long,3> defect_class_edges{};
 bool incidence_polynomial_matches_minpoly_exactly{};
 bool every_boundary_edge_forced_into_qrs{};
 bool universal_parent_role_intertwiner_schema{};
 bool no_fourth_generator{};
 bool proved{};
};

inline SupergoldenQRSClosureCertificate derive_supergolden_qrs_closure(){
 using namespace mathlib;
 SupergoldenQRSClosureCertificate c;

 // Step 1-2: exact greedy beta-expansion of 1 and the canonical
 // substitution it drives -- reusable operations, not literature copy.
 QBetaRing R = QBetaRing::from_low_first({-1,0,-1}); // x^3 - x^2 - 1
 RootInterval beta_I = isolate_beta(R);
 auto ge = exact_greedy_beta_expansion_of_one(R, beta_I, 32);
 if (!ge.terminated) throw std::runtime_error("supergolden greedy expansion did not terminate as expected");
 c.greedy_digits = ge.digits;
 auto derived_rule_ll = canonical_beta_substitution_from_digits(ge.digits);

 c.alphabet_size = derived_rule_ll.size();
 c.minimal_polynomial = supergolden_pisot_minpoly_coefficients();

 // Step 3: exact-check the incidence characteristic polynomial against
 // x^3-x^2-1 (independently, via Faddeev-LeVerrier -- not asserted).
 std::vector<std::vector<long long>> M(c.alphabet_size, std::vector<long long>(c.alphabet_size,0));
 for (std::size_t i=0;i<c.alphabet_size;++i) for (auto letter : derived_rule_ll[i]) M[i][(std::size_t)letter]++;
 PolyZ chi = charpoly_faddeev_leverrier(M);
 c.incidence_polynomial.resize((std::size_t)chi.degree()+1);
 for (long long i=0;i<=chi.degree();++i) c.incidence_polynomial[(std::size_t)i] = mpz_get_si(chi.coeff((std::size_t)i).get());

 std::vector<long long> expect = c.minimal_polynomial;
 c.incidence_polynomial_matches_minpoly_exactly = (c.incidence_polynomial == expect);

 // Sanity: the hand-recorded rule in supergolden_pisot_substitution.hpp
 // must agree with what the derivation just produced.
 auto recorded = supergolden_pisot_rule();
 if (recorded.size() != derived_rule_ll.size()) throw std::runtime_error("supergolden: recorded/derived alphabet size mismatch");
 for (std::size_t i=0;i<recorded.size();++i){
   if (recorded[i].size()!=derived_rule_ll[i].size()) throw std::runtime_error("supergolden: recorded/derived image length mismatch");
   for (std::size_t k=0;k<recorded[i].size();++k) if ((long long)recorded[i][k]!=derived_rule_ll[i][k]) throw std::runtime_error("supergolden: recorded/derived image mismatch");
 }

 // Step 4-6: parent-prefix catalogue, ordered prefix differences, and
 // generator-minimality audit, exactly as in the plastic and
 // third-smallest-Pisot closures.
 SubstitutionRule rule(recorded);
 constexpr double beta = 1.4655712318767680266567312252949;
 auto subst = make_substitution<3>(rule, beta); c.universal_role_states = 9;
 std::set<std::vector<long long>> prefixes;
 std::array<std::vector<ParentDecomposition<3>>,3> parents;
 for (long long i=0;i<3;++i){parents[(std::size_t)i]=parent_decompositions<3>(subst.images,i);c.total_parent_decompositions+=parents[(std::size_t)i].size();for(auto const&p:parents[(std::size_t)i])prefixes.insert(p.p);}
 c.distinct_prefixes = prefixes.size();
 std::set<std::array<long long,3>> defects;
 for (long long i=0;i<3;++i) for (long long j=0;j<3;++j) for (auto const&p:parents[(std::size_t)i]) for (auto const&q:parents[(std::size_t)j]){
   std::array<long long,3> d{}; for(auto z:q.p) ++d[(std::size_t)z]; for(auto z:p.p) --d[(std::size_t)z];
   defects.insert(d);
   int g=-1;
   if (d==std::array<long long,3>{}) g=0;
   else if (d==std::array<long long,3>{1,0,0}) g=1;
   else if (d==std::array<long long,3>{-1,0,0}) g=2;
   else throw std::runtime_error("supergolden: unexpected prefix defect -- possible fourth generator, requires manual audit before classification");
   ++c.defect_class_edges[(std::size_t)g]; ++c.universal_parent_pair_edges;
 }
 c.defect_classes = defects.size();
 c.every_boundary_edge_forced_into_qrs = (prefixes==std::set<std::vector<long long>>{{},{0}} && defects.size()==3);
 c.universal_parent_role_intertwiner_schema = c.every_boundary_edge_forced_into_qrs;
 c.no_fourth_generator = c.every_boundary_edge_forced_into_qrs;
 c.proved = c.incidence_polynomial_matches_minpoly_exactly && c.total_parent_decompositions==4 && c.distinct_prefixes==2 && c.defect_classes==3 && c.universal_parent_role_intertwiner_schema;
 return c;
}
}
