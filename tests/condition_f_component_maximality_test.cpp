#include <cassert>
#include <iostream>
#include <utility>
#include <vector>

#include "ravel/proof/condition_f_component_maximality.hpp"
#include "ravel/proof/universal_dominance_reduction.hpp"

using namespace ravel::proof;

int main() {
    // Fibre one: SCC matrices are derived from the quotient lift itself.
    // Core = Fibonacci matrix on roles 0,1.  Role 2 is terminal.  Roles 3,4
    // form a pure-grade copy of the core and are discharged by path injection.
    std::vector<std::size_t> role{0,1,2,3,4};
    std::vector<std::size_t> fibre(role.size(), 0);
    std::vector<std::pair<std::size_t,std::size_t>> edges{
        {0,0},{0,1},{1,0},
        {2,2},
        {3,3},{3,4},{4,3}
    };
    const auto quotient = derive_condition_f_twisted_component_quotient(
        edges, role, fibre, 5, 1, 12);
    assert(quotient.proved);
    const std::vector<std::size_t> grade{1,1,3,2,2};
    const auto proof = derive_condition_f_component_maximality(
        quotient, grade, 0, 64);
    assert(proof.core_spectrally_maximal);
    assert(proof.recurrent_components == 3);
    assert(proof.terminal_components == 1);
    assert(proof.pure_grade_components == 1);
    assert(proof.path_injection_components == 1);
    assert(proof.unsupported_components == 0);
    const auto universal = compose_universal_dominance_from_finite_quotient(
        7, true, true, proof);
    assert(universal.equality_derived);

    // Identity-plus-swap channels glue grades 1 and 2 into one actual mixed
    // SCC.  Selecting it as core verifies that SCC/grade assembly happens
    // after holonomy, not sheetwise before quotient construction.
    std::vector<std::size_t> twist_role{0,0,1,1};
    std::vector<std::size_t> twist_fibre{0,1,0,1};
    std::vector<std::pair<std::size_t,std::size_t>> twist_edges{
        {0,2},{1,3},{0,3},{1,2},
        {2,0},{3,1},{2,1},{3,0}
    };
    const auto twisted = derive_condition_f_twisted_component_quotient(
        twist_edges, twist_role, twist_fibre, 2, 2, 12);
    assert(twisted.proved && twisted.holonomy_glues_sheets);
    const auto twisted_proof = derive_condition_f_component_maximality(
        twisted, std::vector<std::size_t>{1,2,1,2}, 0, 32);
    assert(twisted_proof.core_spectrally_maximal);
    assert(twisted_proof.recurrent_components == 1);
    assert(twisted_proof.core_component < twisted_proof.lifted_components);

    // A larger-growth competitor must remain unsupported.
    std::vector<std::size_t> bad_role{0,1};
    std::vector<std::size_t> bad_fibre{0,0};
    std::vector<std::pair<std::size_t,std::size_t>> bad_edges{
        {0,0}, {1,1}, {1,1}
    };
    const auto bad_q = derive_condition_f_twisted_component_quotient(
        bad_edges, bad_role, bad_fibre, 2, 1, 4);
    assert(bad_q.proved);
    const auto bad = derive_condition_f_component_maximality(
        bad_q, std::vector<std::size_t>{1,2}, 0, 16);
    assert(!bad.core_spectrally_maximal);
    assert(bad.unsupported_components == 1);


    // Locally-fibered classes need not share one global sheet set.  The same
    // assembled-SCC theorem applies after unique path lifting is certified.
    const std::vector<std::size_t> state_class{0,0,1,1};
    const std::vector<FiberedQuotientChannel> channels{{10,0,1},{11,1,0}};
    const std::vector<FiberedConcreteEdge> lifted_edges{
        {0,2,10},{1,3,10},{2,1,11},{3,0,11}
    };
    const auto local = derive_fibered_twisted_quotient(
        state_class, 2, channels, lifted_edges, 8);
    assert(local.valid && local.spectral_radius_preserved);
    const auto local_proof = derive_condition_f_component_maximality(
        local, std::vector<std::size_t>{1,2,1,2}, 0, 32);
    assert(local_proof.core_spectrally_maximal);
    assert(local_proof.twisted_assembly_replayed);

    std::cout << "Condition-F component maximality PASS recurrent="
              << proof.recurrent_components << " terminal="
              << proof.terminal_components << " pure="
              << proof.pure_grade_components << " mixed="
              << proof.mixed_grade_components << "\n";
}
