#include <cassert>
#include <iostream>
#include <vector>

#include "ravel/proof/condition_f_two_matrix_collapse.hpp"

using namespace ravel::proof;

int main() {
    // Two roles, two sheets.  Each role edge has an identity channel of type 0
    // and a swap channel of type 1.  The two generators have different twists,
    // but every binary word has the same path count upstairs and downstairs.
    std::vector<std::size_t> role{0,0,1,1};
    std::vector<std::size_t> fibre{0,1,0,1};
    std::vector<std::pair<std::size_t,std::size_t>> edges{
        {0,2},{1,3},{0,3},{1,2},
        {2,0},{3,1},{2,1},{3,0}
    };
    const auto q = derive_condition_f_twisted_component_quotient(
        edges, role, fibre, 2, 2, 16);
    assert(q.proved && q.channels.size() == 4);

    // Matching extraction returns two channels for 0->1 followed by two for
    // 1->0.  Classify identity as generator 0 and swap as generator 1 from
    // the actual permutation, not extraction order.
    std::vector<std::size_t> generator;
    for (const auto& ch : q.channels)
        generator.push_back(ch.permutation == std::vector<std::size_t>{0,1} ? 0 : 1);
    const auto c = derive_condition_f_two_matrix_collapse(q, generator, 12);
    assert(c.proved);
    assert(c.generator_zero_channels == 2);
    assert(c.generator_one_channels == 2);
    assert(c.word_by_word_growth_preserved);
    assert(c.summed_channel_count_preserved);
    assert(c.cocycle.checked_words == (std::size_t{1} << 13) - 1);

    auto malformed = generator;
    malformed[0] = 2;
    assert(!derive_condition_f_two_matrix_collapse(q, malformed, 4).proved);

    std::cout << "Condition-F two-matrix cocycle collapse PASS\n";
}
