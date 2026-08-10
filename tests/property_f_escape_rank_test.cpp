#include <cassert>
#include <stdexcept>

#include "adelic/property_f_escape_rank.hpp"

int main() {
    adelic::PropertyFGraph graph;
    graph.nodes.resize(3);
    graph.nodes[0].successors = {1};
    graph.nodes[1].successors = {0, 2};
    graph.nodes[2].successors = {};
    graph.scc_labels = {0, 0, 1};
    graph.scc_sizes = {2, 1};
    const auto rank = adelic::derive_property_f_escape_rank(graph);
    assert(rank.valid);
    assert(rank.condensation_acyclic);
    assert(rank.terminal_sccs == 1);
    assert(rank.node_height == std::vector<std::size_t>({1, 1, 0}));
    assert(rank.maximum_height == 1);

    adelic::PropertyFGraph bad_labels = graph;
    bad_labels.scc_labels.clear();
    bool rejected = false;
    try {
        (void)adelic::derive_property_f_escape_rank(bad_labels);
    } catch (const std::invalid_argument&) {
        rejected = true;
    }
    assert(rejected);

    adelic::PropertyFGraph bad_edge = graph;
    bad_edge.nodes[2].successors = {9};
    rejected = false;
    try {
        (void)adelic::derive_property_f_escape_rank(bad_edge);
    } catch (const std::invalid_argument&) {
        rejected = true;
    }
    assert(rejected);
    return 0;
}
