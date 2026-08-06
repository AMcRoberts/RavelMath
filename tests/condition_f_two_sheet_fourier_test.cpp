#include <cassert>
#include <iostream>
#include <vector>

#include "ravel/proof/condition_f_two_sheet_fourier.hpp"

using namespace ravel::proof;

int main() {
    const std::vector<std::size_t> id{0,1};
    const std::vector<std::size_t> sw{1,0};
    std::vector<ConditionFPrefixChannel> channels{
        {0,0,{}, {}, id},
        {0,1,{}, {0}, sw},
        {1,0,{0}, {}, sw},
        {1,1,{0}, {0}, id},
    };
    const auto pair = derive_condition_f_prefix_two_matrix(2,2,channels,8);
    const auto f = derive_condition_f_two_sheet_fourier(pair);
    assert(f.proved);
    assert(f.q_balanced[0][0] == 1 && f.q_balanced[1][1] == 1);
    assert(f.r_residual[0][1] == 1 && f.r_residual[1][0] == 1);
    assert(f.even_sector[0][1] == 1);
    assert(f.odd_sector[0][1] == -1);

    auto bad = channels;
    bad[1].permutation = id;
    const auto badpair = derive_condition_f_prefix_two_matrix(2,2,bad,4);
    assert(badpair.proved); // still a valid skew product, but not orientation canonical
    assert(!derive_condition_f_two_sheet_fourier(badpair).proved);

    std::cout << "Condition-F two-sheet Fourier PASS\n";
}
