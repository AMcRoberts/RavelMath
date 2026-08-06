#include <cassert>
#include <iostream>
#include <vector>

#include "ravel/proof/condition_f_prefix_two_matrix.hpp"

using namespace ravel::proof;

int main() {
    const std::vector<std::size_t> id{0,1};
    const std::vector<std::size_t> sw{1,0};
    std::vector<ConditionFPrefixChannel> channels{
        {0,0,{}, {}, id},             // delta 0: balanced/bulk
        {0,1,{}, {0}, sw},            // delta +1: residual, positive twist
        {1,0,{0}, {}, sw},            // delta -1: residual, negative twist
        {1,1,{0}, {0}, id},           // delta 0: balanced/bulk
    };
    const auto p = derive_condition_f_prefix_two_matrix(2,2,channels,12);
    assert(p.proved);
    assert(p.balanced_channels == 2);
    assert(p.defect_channels == 2);
    assert(p.positive_defect_channels == 1);
    assert(p.negative_defect_channels == 1);
    assert((p.generator == std::vector<std::size_t>{0,1,1,0}));
    assert((p.signed_defect == std::vector<long long>{0,1,-1,0}));
    assert(p.cocycle.every_word_path_count_preserved);

    auto malformed = channels;
    malformed[0].left_prefix = {1};
    assert(!derive_condition_f_prefix_two_matrix(2,2,malformed,4).proved);

    std::cout << "Condition-F prefix two-matrix split PASS words="
              << p.cocycle.checked_words << "\n";
}
