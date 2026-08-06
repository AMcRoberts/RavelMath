#include <cassert>
#include <cstddef>
#include <iostream>

#include "ravel/proof/graph_structural_campaign.hpp"

int main() {
    using namespace ravel::proof::graph_structural;
    for (std::size_t n = 2; n <= 512; ++n) {
        const auto graph = cycle_with_transient_feeder(n);
        const auto scc = certify_strong_components(graph);
        assert(scc.exhaustive);
        assert(scc.mutually_reachable);
        assert(scc.components.size() == 2);

        const auto condensation = certify_condensation(graph, scc);
        assert(condensation.acyclic);
        assert(condensation.edges.size() == 1);
        assert(condensation.sink_components.size() == 1);

        const auto attractor = certify_unique_sink_attractor(graph, scc, condensation);
        assert(attractor.unique_sink);
        assert(attractor.basin_is_all_vertices);
        assert(scc.components.at(attractor.attractor_component).size() == n);

        const auto matrix = certify_adjacency_matrix(graph);
        assert(matrix.exact);
        assert(matrix.one_count == n + 1);

        const auto cycle = certify_canonical_cycle_core(graph, scc, attractor);
        assert(cycle.canonical_cycle);
        assert(cycle.cycle_order.size() == n);
        assert(cycle.canonical_start == 0);

        const auto block = certify_feeder_cycle_block(graph, scc, attractor, cycle);
        assert(block.block_form_exact);
        assert(block.reindex_order.size() == n + 1);
        assert(block.reindex_order.front() == n);
        assert(block.closed_charpoly_factor ==
               "charpoly(full) = X * charpoly(cycleCore)");
        assert(block.open_core_charpoly_obligation ==
               "charpoly(cycleCore_n) = X^n - 1");

        const auto seam = attract_to_structural_seam(graph, scc, condensation, attractor, matrix);
        assert(seam.seam_found);
        assert(!seam.open_bridges.empty());
    }
    std::cout << "graph structural campaign: closed certificates through n=512\n";
    std::cout << "closed: canonical cycle core + transient X factor\n";
    std::cout << "remaining seam: charpoly(cycleCore_n) = X^n - 1\n";
}
