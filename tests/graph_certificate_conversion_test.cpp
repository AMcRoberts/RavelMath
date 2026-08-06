#include <cassert>
#include <cstddef>
#include <fstream>
#include <iostream>
#include <string>

#include "ravel/proof/graph_certificate_conversion.hpp"

int main() {
    using namespace ravel::proof::graph_structural;
    using namespace ravel::proof::graph_conversion;

    for (std::size_t n = 2; n <= 512; ++n) {
        const auto graph = cycle_with_transient_feeder(n);
        const auto scc = certify_strong_components(graph);
        const auto condensation = certify_condensation(graph, scc);
        const auto attractor = certify_unique_sink_attractor(graph, scc, condensation);
        const auto adjacency = certify_adjacency_matrix(graph);
        const auto cycle = certify_canonical_cycle_core(graph, scc, attractor);
        const auto block = certify_feeder_cycle_block(graph, scc, attractor, cycle);

        const auto artifact =
            convert_graph_certificate_to_charpoly_premise(adjacency, block);
        assert(artifact.premise_produced);
        assert(artifact.reindex.valid);
        assert(artifact.block_equality.valid);
        assert(artifact.block_equality.entrywise_equal);
        assert(artifact.reindex.new_to_old.front() == n);
        for (std::size_t i = 0; i < n + 1; ++i) {
            assert(artifact.reindex.old_to_new[artifact.reindex.new_to_old[i]] == i);
            assert(artifact.reindex.new_to_old[artifact.reindex.old_to_new[i]] == i);
        }
    }

    const auto lean = render_lean_graph_charpoly_conversion_module();
    assert(lean.find("Matrix.charpoly_reindex") != std::string::npos);
    assert(lean.find("Matrix.charpoly_fromBlocks_zero₂₁") != std::string::npos);
    assert(lean.find("charpoly_closed_of_reindex_cycle_core") != std::string::npos);
    assert(lean.find("sorry") == std::string::npos);

    std::ofstream out("out/graph_certificate_conversion_validation.lean");
    out << lean;
    out.close();

    std::cout << "graph certificate conversion: exact reindex/block premises through n=512\n";
    std::cout << "emitted out/graph_certificate_conversion_validation.lean\n";
}
