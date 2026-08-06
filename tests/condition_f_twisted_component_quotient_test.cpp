#include <cassert>
#include <iostream>
#include <utility>
#include <vector>

#include "ravel/proof/condition_f_twisted_component_quotient.hpp"

using namespace ravel::proof;

int main() {
    // Two base roles, two sheets.  Two component channels connect the same
    // ordered role pair: one preserves sheets and one swaps them.  A flat Z/2
    // quotient would call this ambiguous; the multigraph quotient must retain
    // both parallel edges.
    std::vector<std::size_t> role{0,0,1,1};
    std::vector<std::size_t> fibre{0,1,0,1};
    std::vector<std::pair<std::size_t,std::size_t>> edges{
        {0,2},{1,3}, // identity channel 0 -> 1
        {0,3},{1,2}, // swap channel 0 -> 1
        {2,0},{3,1}, // identity channel 1 -> 0
        {2,1},{3,0}  // swap channel 1 -> 0
    };
    const auto p = derive_condition_f_twisted_component_quotient(
        edges, role, fibre, 2, 2, 12);
    assert(p.proved);
    assert(p.multiply_glued_role_pairs == 2);
    assert(p.channel_count == 4);
    assert(p.genuinely_twisted);
    assert(p.concrete_lift_reconstructed);
    assert(p.path_growth_preserved);
    assert(p.base_scc_count == 1);
    assert(p.lifted_scc_count == 1);
    assert(p.holonomy_glues_sheets);
    // Each base state has two outgoing parallel edges; each concrete state has
    // exactly two outgoing lifts, so row path counts are 2^k on both levels.
    assert(p.skew_product.base_adjacency[0].size() == 2);
    assert(p.skew_product.lifted_adjacency[0].size() == 2);

    // With identity channels only, the same strongly connected base has two
    // separate lifted sheet SCCs.  The swap channel is exactly what glues them.
    const std::vector<std::pair<std::size_t,std::size_t>> untwisted_edges{
        {0,2},{1,3},{2,0},{3,1}};
    const auto untwisted = derive_condition_f_twisted_component_quotient(
        untwisted_edges, role, fibre, 2, 2, 8);
    assert(untwisted.proved);
    assert(untwisted.base_scc_count == 1);
    assert(untwisted.lifted_scc_count == 2);
    assert(!untwisted.holonomy_glues_sheets);

    // One irregular role-pair relation is not a permutation-fibre quotient and
    // must be rejected instead of silently losing an assembled component.
    edges.pop_back();
    const auto bad = derive_condition_f_twisted_component_quotient(
        edges, role, fibre, 2, 2, 4);
    assert(!bad.proved);
    assert(!bad.role_pair_regular);

    std::cout << "Condition-F twisted component quotient PASS channels="
              << p.channel_count << " glued=" << p.multiply_glued_role_pairs
              << "\n";
}
