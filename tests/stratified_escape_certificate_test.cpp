#include <cassert>
#include <iostream>
#include <vector>

#include "ravel/proof/finite_graph_correspondence.hpp"

int main() {
    // One recurrent two-cycle at round 3; transient rounds 1 and 2 feed
    // forward into the next shell or into the recurrent core.
    const std::vector<std::vector<std::size_t>> graph = {
        {1}, {0}, {3}, {0}, {0}};
    const std::vector<long long> components = {0, 0, 1, 2, 3};
    const std::vector<bool> recurrent = {true, false, false, false};
    const std::vector<std::size_t> rounds = {3, 3, 1, 2, 2};
    const auto good = ravel::proof::derive_stratified_escape_certificate(
        graph, components, recurrent, rounds);
    assert(good.valid);
    assert(good.transient_groups == 2);
    assert(good.transient_groups_with_escape == 2);
    assert(good.recurrent_to_earlier_transient_edges == 0);

    // A recurrent edge into an earlier transient shell is a direct falsifier.
    auto bad_return = graph;
    bad_return[0].push_back(2);
    const auto return_certificate =
        ravel::proof::derive_stratified_escape_certificate(
            bad_return, components, recurrent, rounds);
    assert(!return_certificate.valid);
    assert(return_certificate.recurrent_to_earlier_transient_edges == 1);

    // A shell with no forward/recurrent escape is also rejected.
    auto bad_escape = graph;
    bad_escape[4] = {4};
    auto bad_escape_rounds = rounds;
    bad_escape_rounds[4] = 4;
    const auto escape_certificate =
        ravel::proof::derive_stratified_escape_certificate(
            bad_escape, components, recurrent, bad_escape_rounds);
    assert(!escape_certificate.valid);
    assert(escape_certificate.transient_groups_with_escape == 2);

    // A component spanning two birth rounds cannot be a well-defined stratum.
    auto bad_round = rounds;
    bad_round[1] = 2;
    const auto round_certificate =
        ravel::proof::derive_stratified_escape_certificate(
            graph, components, recurrent, bad_round);
    assert(!round_certificate.valid);
    assert(round_certificate.recurrent_round_inconsistencies == 1);

    std::cout << "stratified_escape_certificate: PASS\n";
}
