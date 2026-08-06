#include <cassert>
#include <iostream>
#include <vector>

#include "ravel/proof/condition_f_prefix_twisted_quotient.hpp"

using namespace ravel::proof;

int main() {
    // Two roles x two sheets. Balanced channels are identity; the two signed
    // residual channels are opposite-role swaps. They share role pairs but
    // must never be mixed by matching extraction.
    std::vector<std::size_t> role{0,0,1,1};
    std::vector<std::size_t> fibre{0,1,0,1};
    std::vector<ConditionFTypedConcreteEdge> edges{
        {0,0,{},{}},{1,1,{},{}},
        {2,2,{},{}},{3,3,{},{}},
        {0,3,{}, {0}},{1,2,{}, {0}},
        {2,1,{0}, {}},{3,0,{0}, {}},
    };
    const auto p = derive_condition_f_prefix_twisted_quotient(
        edges,role,fibre,2,2,12);
    assert(p.proved);
    assert(p.typed_channels == 4);
    assert(p.two_matrix.balanced_channels == 2);
    assert(p.two_matrix.defect_channels == 2);
    assert(p.two_matrix.positive_defect_channels == 1);
    assert(p.two_matrix.negative_defect_channels == 1);
    assert(p.concrete_lift_reconstructed);

    auto bad = edges;
    bad.pop_back(); // breaks regularity in the negative residual relation
    assert(!derive_condition_f_prefix_twisted_quotient(
        bad,role,fibre,2,2,4).proved);

    std::cout << "Condition-F prefix typed twisted quotient PASS channels="
              << p.typed_channels << " words="
              << p.two_matrix.cocycle.checked_words << "\n";
}
