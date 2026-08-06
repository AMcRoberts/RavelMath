#include <cassert>
#include <iostream>
#include "ravel/proof/simplest_nonunit_pisot_closure.hpp"
int main(){
 auto c = ravel::proof::derive_simplest_nonunit_pisot_closure();
 std::cout << "alphabet=" << c.alphabet_size << " prefixes=" << c.distinct_prefixes
           << " raw_defect_classes=" << c.raw_defect_classes
           << " parents=" << c.total_parent_decompositions
           << " role_edges=" << c.universal_parent_pair_edges << "\n";
 for (auto const& [d, ct] : c.defect_class_edges) std::cout << "  defect=" << d << " count=" << ct << "\n";
 std::cout << "plus2_dominated=" << c.plus2_dominated_by_plus1_squared
           << " minus2_dominated=" << c.minus2_dominated_by_minus1_squared
           << " exactly_three_primitive_generators=" << c.exactly_three_primitive_generators
           << " proved=" << c.proved << "\n";
 assert(c.proved);
 assert((c.greedy_digits == std::vector<long long>{2,2}));
 assert(c.alphabet_size==2);
 assert(c.distinct_prefixes==3);
 assert(c.raw_defect_classes==5);
 assert(c.exactly_three_primitive_generators);
 std::cout << "PASS: x^2-2x-2 (simplest non-unit Pisot substitution) is still exactly "
              "Q/R/S at the primitive-generator level -- the raw 5 defect classes "
              "collapse to 3 via reducibility, same as every other case examined.\n";
}
