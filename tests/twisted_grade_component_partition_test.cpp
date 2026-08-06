#include <cassert>
#include <iostream>
#include <utility>
#include <vector>

#include "ravel/proof/twisted_grade_component_partition.hpp"

using namespace ravel::proof;

int main() {
    std::vector<std::size_t> role{0,0,1,1};
    std::vector<std::size_t> fibre{0,1,0,1};
    // Fibre 0 carries grade one and fibre 1 carries grade two.
    std::vector<std::size_t> grade{1,2,1,2};

    // Identity transport: two separate pure-grade permutation SCCs.
    std::vector<std::pair<std::size_t,std::size_t>> identity{
        {0,2},{1,3},{2,0},{3,1}};
    auto q0 = derive_condition_f_twisted_component_quotient(
        identity, role, fibre, 2, 2);
    assert(q0.proved && q0.lifted_scc_count == 2);
    auto p0 = derive_twisted_grade_component_partition(q0, grade);
    assert(p0.proved);
    assert(p0.lifted_components == 2);
    assert(p0.terminal_components == 2);

    // Add swap transport in both directions.  Holonomy glues the sheets.  To
    // make the assembled SCC branching rather than terminal, retain both the
    // identity and swap channels.  The resulting component contains both
    // concrete grades and must be routed as mixed-grade.
    std::vector<std::pair<std::size_t,std::size_t>> twisted{
        {0,2},{1,3},{0,3},{1,2},
        {2,0},{3,1},{2,1},{3,0}};
    auto q1 = derive_condition_f_twisted_component_quotient(
        twisted, role, fibre, 2, 2);
    assert(q1.proved && q1.holonomy_glues_sheets);
    auto p1 = derive_twisted_grade_component_partition(q1, grade);
    assert(p1.proved);
    assert(p1.lifted_components == 1);
    assert(p1.mixed_grade_components == 1);
    assert(p1.holonomy_mixed_components == 1);
    assert(p1.pure_grade_components == 0);

    std::cout << "twisted grade component partition PASS mixed="
              << p1.mixed_grade_components << "\n";
}
