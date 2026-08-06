#include <cassert>
#include <iostream>
#include "ravel/proof/permutation_skew_product.hpp"
using namespace ravel::proof;
int main() {
    // Base adjacency [[1,1],[1,0]], lifted over a 3-cycle fibre.
    std::vector<PermutationSkewEdge> edges{
        {0,0,{1,2,0}},
        {0,1,{0,2,1}},
        {1,0,{2,0,1}}
    };
    auto c = derive_permutation_skew_product(2,3,edges,20);
    assert(c.valid);
    assert(c.path_counts_aggregate);
    assert(c.spectral_radius_equal);
    auto bad=edges;
    bad[0].permutation={0,0,2};
    assert(!derive_permutation_skew_product(2,3,bad).valid);
    std::cout << "permutation skew product PASS\n";
}
