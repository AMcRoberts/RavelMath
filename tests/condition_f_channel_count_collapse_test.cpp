#include <cassert>
#include <iostream>
#include <vector>

#include "ravel/proof/condition_f_channel_count_collapse.hpp"
#include "ravel/proof/universal_dominance_reduction.hpp"

using namespace ravel::proof;

int main() {
    // Two transport channels (identity and swap) over each base edge. The
    // twist glues sheets, but the channel-count quotient has adjacency [[0,2],[2,0]].
    std::vector<std::size_t> role{0,0,1,1};
    std::vector<std::size_t> fibre{0,1,0,1};
    std::vector<std::pair<std::size_t,std::size_t>> edges{
        {0,2},{1,3},{0,3},{1,2},
        {2,0},{3,1},{2,1},{3,0}
    };
    const auto q = derive_condition_f_twisted_component_quotient(
        edges, role, fibre, 2, 2, 16);
    assert(q.proved && q.genuinely_twisted && q.path_growth_preserved);
    const auto collapse = derive_condition_f_channel_count_collapse(q, 0, 64);
    assert(collapse.spectral_radius_preserved);
    assert(collapse.core_spectrally_maximal_in_lift);
    assert(q.skew_product.base_adjacency[0].size() == 2);
    assert(q.skew_product.base_adjacency[1].size() == 2);

    // Core Fibonacci SCC plus terminal loop competitor.
    std::vector<std::size_t> role2{0,1,2};
    std::vector<std::size_t> fibre2(3,0);
    std::vector<std::pair<std::size_t,std::size_t>> edges2{
        {0,0},{0,1},{1,0},{2,2}
    };
    const auto q2 = derive_condition_f_twisted_component_quotient(
        edges2, role2, fibre2, 3, 1, 12);
    const auto c2 = derive_condition_f_channel_count_collapse(q2, 0, 64);
    assert(c2.core_spectrally_maximal_in_lift);
    assert(c2.quotient_maximality.terminal_components == 1);
    const auto universal = compose_universal_dominance_from_finite_quotient(
        7, true, true, c2.quotient_maximality);
    assert(universal.equality_derived);

    // A larger-growth competitor remains visible after forgetting twists.
    std::vector<std::size_t> role3{0,1};
    std::vector<std::size_t> fibre3(2,0);
    std::vector<std::pair<std::size_t,std::size_t>> edges3{
        {0,0},{1,1},{1,1}
    };
    const auto q3 = derive_condition_f_twisted_component_quotient(
        edges3, role3, fibre3, 2, 1, 8);
    const auto c3 = derive_condition_f_channel_count_collapse(q3, 0, 32);
    assert(!c3.core_spectrally_maximal_in_lift);

    std::cout << "Condition-F channel-count collapse PASS\n";
}
