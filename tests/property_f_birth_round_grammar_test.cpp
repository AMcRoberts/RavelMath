#include <cassert>

#include "adelic/property_f_birth_round_grammar.hpp"

int main() {
    adelic::PropertyFGraph graph;
    graph.nodes.resize(4);
    graph.nodes[0].successors = {1, 3};
    graph.nodes[1].successors = {0, 1, 2};
    graph.nodes[2].successors = {3};
    graph.nodes[3].successors = {};
    graph.scc_labels = {0, 0, 1, 2};
    graph.scc_sizes = {2, 1, 1};

    const auto grammar = adelic::derive_property_f_birth_round_grammar(graph);
    assert(grammar.valid);
    assert(grammar.rank_valid);
    assert(grammar.terminal_layer_zero);
    assert(grammar.edge_rank_strictly_decreasing);
    assert(grammar.node_round == std::vector<std::size_t>({2, 2, 1, 0}));
    assert(grammar.layer_sizes.at(0) == 1);
    assert(grammar.layer_sizes.at(1) == 1);
    assert(grammar.layer_sizes.at(2) == 2);
    assert(grammar.internal_edges == 3);
    assert(grammar.cross_scc_edges == 3);
    assert(grammar.drop_histogram.at(1) == 2);
    assert(grammar.drop_histogram.at(2) == 1);

    adelic::PropertyFGraph bad = graph;
    bad.nodes[3].successors = {2};
    const auto bad_grammar =
        adelic::derive_property_f_birth_round_grammar(bad);
    assert(!bad_grammar.valid);
    return 0;
}
