#include <cstdio>
#include <filesystem>
#include <vector>

#include "ravel/corona_projection.hpp"

using namespace ravel;

int main() {
    CoronaTruthGraph<3> graph;
    graph.nodes.resize(4);
    for (std::size_t i = 0; i < graph.nodes.size(); ++i) {
        graph.nodes[i].i = static_cast<long long>(i % 3);
        graph.nodes[i].j = static_cast<long long>((i + 1) % 3);
        graph.nodes[i].x[0] = static_cast<long long>(i);
    }
    graph.graph = WeightedDigraph(4);
    graph.graph.add_edge(0, 1, 1);
    graph.graph.add_edge(1, 0, 1);
    graph.graph.add_edge(1, 2, 1);
    graph.graph.add_edge(2, 3, 2);
    graph.graph.add_edge(3, 2, 2);
    graph.corona_rounds = 4;
    graph.converged = true;
    graph.rebuild_index();

    const auto recurrent = recurrent_corona_sccs(graph);
    if (recurrent.size() != 2) return 1;
    auto first = project_corona_scc(graph, recurrent.front());
    if (first.size() != 2 || first.materialize().n != 2) return 2;

    auto even = project_corona(graph, [](std::size_t v, const auto&) {
        return v % 2 == 0;
    });
    if (even.size() != 2) return 3;

    const std::string path = "out/corona_projection_test.cache";
    std::filesystem::create_directories("out");
    save_corona_truth_graph(graph, path);
    auto replay = load_corona_truth_graph<3>(path);
    if (replay.nodes != graph.nodes || replay.graph.n != graph.graph.n
        || replay.corona_rounds != 4 || !replay.converged) return 4;
    std::size_t original_edges = 0, replay_edges = 0;
    for (const auto& row : graph.graph.out_adj) original_edges += row.size();
    for (const auto& row : replay.graph.out_adj) replay_edges += row.size();
    if (original_edges != replay_edges) return 5;

    std::printf("corona projection PASS nodes=%zu recurrent_sccs=%zu\n",
                replay.nodes.size(), recurrent.size());
    return 0;
}
