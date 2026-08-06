// corona_projection.hpp
//
// Legacy compatibility views over an already materialized corona graph.
//
// New consumers should use the request-driven CoronaSurface and
// algorithm2_projected_trace APIs in corona.hpp.  This header remains for
// workflows that genuinely begin with a complete concrete graph or an older
// serialized truth-machine image.  It does not rebuild corona arithmetic and
// does not implement another SCC algorithm: it composes canonical node/edge
// output with Tarjan from graph_divisor.hpp while retaining global vertex
// identity.

#pragma once

#include <cstddef>
#include <cstdint>
#include <fstream>
#include <map>
#include <set>
#include <span>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include "ravel/corona.hpp"
#include "ravel/graph_divisor.hpp"

namespace ravel {

template <std::size_t D>
struct CoronaTruthGraph {
    std::vector<SNode<D>> nodes;
    std::map<SNode<D>, std::size_t> index;
    WeightedDigraph graph;
    int corona_rounds = 0;
    bool converged = false;

    mutable bool scc_cached = false;
    mutable std::vector<std::vector<std::size_t>> sccs;
    mutable std::vector<std::size_t> vertex_scc;

    void rebuild_index() {
        index.clear();
        for (std::size_t k = 0; k < nodes.size(); ++k) {
            if (!index.emplace(nodes[k], k).second)
                throw std::logic_error("CoronaTruthGraph: duplicate node");
        }
        if (graph.n != nodes.size())
            throw std::logic_error("CoronaTruthGraph: node/graph size mismatch");
        scc_cached = false;
        sccs.clear();
        vertex_scc.clear();
    }

    void ensure_sccs() const {
        if (scc_cached) return;
        sccs = tarjan_scc(graph);
        vertex_scc.assign(nodes.size(), sccs.size());
        for (std::size_t c = 0; c < sccs.size(); ++c)
            for (const auto v : sccs[c]) vertex_scc.at(v) = c;
        scc_cached = true;
    }
};

template <std::size_t D>
class CoronaProjection {
public:
    CoronaProjection() = default;

    CoronaProjection(const CoronaTruthGraph<D>& owner,
                     std::vector<std::size_t> global_vertices)
        : owner_(&owner), vertices_(std::move(global_vertices)) {
        local_.reserve(vertices_.size());
        for (std::size_t k = 0; k < vertices_.size(); ++k) {
            if (vertices_[k] >= owner.nodes.size())
                throw std::out_of_range("CoronaProjection: vertex out of range");
            if (!local_.emplace(vertices_[k], k).second)
                throw std::invalid_argument("CoronaProjection: duplicate vertex");
        }
    }

    const CoronaTruthGraph<D>& owner() const {
        if (!owner_) throw std::logic_error("CoronaProjection: empty view");
        return *owner_;
    }

    std::span<const std::size_t> global_vertices() const { return vertices_; }
    std::size_t size() const { return vertices_.size(); }
    bool empty() const { return vertices_.empty(); }

    const SNode<D>& node(std::size_t local_vertex) const {
        return owner().nodes.at(vertices_.at(local_vertex));
    }

    std::size_t global(std::size_t local_vertex) const {
        return vertices_.at(local_vertex);
    }

    bool contains_global(std::size_t vertex) const {
        return local_.find(vertex) != local_.end();
    }

    std::size_t local(std::size_t global_vertex) const {
        const auto found = local_.find(global_vertex);
        if (found == local_.end())
            throw std::out_of_range("CoronaProjection: global vertex absent");
        return found->second;
    }

    WeightedDigraph materialize() const {
        WeightedDigraph result(vertices_.size());
        for (std::size_t source = 0; source < vertices_.size(); ++source) {
            for (const auto& [destination, weight] :
                 owner().graph.out_adj[vertices_[source]]) {
                const auto found = local_.find(destination);
                if (found != local_.end())
                    result.add_edge(source, found->second, weight);
            }
        }
        return result;
    }

private:
    const CoronaTruthGraph<D>* owner_ = nullptr;
    std::vector<std::size_t> vertices_;
    std::unordered_map<std::size_t, std::size_t> local_;
};

template <std::size_t D, class Predicate>
CoronaProjection<D> project_corona(const CoronaTruthGraph<D>& graph,
                                   Predicate predicate) {
    std::vector<std::size_t> vertices;
    for (std::size_t v = 0; v < graph.nodes.size(); ++v)
        if (predicate(v, graph.nodes[v])) vertices.push_back(v);
    return CoronaProjection<D>(graph, std::move(vertices));
}

template <std::size_t D>
CoronaProjection<D> project_corona_vertices(
        const CoronaTruthGraph<D>& graph,
        const std::vector<std::size_t>& vertices) {
    return CoronaProjection<D>(graph, vertices);
}

template <std::size_t D>
CoronaProjection<D> project_corona_scc(const CoronaTruthGraph<D>& graph,
                                       std::size_t scc) {
    graph.ensure_sccs();
    if (scc >= graph.sccs.size())
        throw std::out_of_range("project_corona_scc: SCC out of range");
    return CoronaProjection<D>(graph, graph.sccs[scc]);
}

template <std::size_t D>
std::vector<std::size_t> recurrent_corona_sccs(
        const CoronaTruthGraph<D>& graph) {
    graph.ensure_sccs();
    std::vector<std::size_t> result;
    for (std::size_t c = 0; c < graph.sccs.size(); ++c)
        if (is_recurrent_scc(graph.graph, graph.sccs[c])) result.push_back(c);
    return result;
}

namespace corona_projection_detail {

inline constexpr std::uint64_t cache_magic = 0x524156454c434f52ULL; // RAVELCOR
inline constexpr std::uint32_t cache_version = 1;

template <class T>
void write_scalar(std::ostream& out, const T& value) {
    out.write(reinterpret_cast<const char*>(&value), sizeof(T));
    if (!out) throw std::runtime_error("CoronaTruthGraph cache write failed");
}

template <class T>
T read_scalar(std::istream& in) {
    T value{};
    in.read(reinterpret_cast<char*>(&value), sizeof(T));
    if (!in) throw std::runtime_error("CoronaTruthGraph cache read failed");
    return value;
}

} // namespace corona_projection_detail

template <std::size_t D>
void save_corona_truth_graph(const CoronaTruthGraph<D>& graph,
                             const std::string& path) {
    std::ofstream out(path, std::ios::binary | std::ios::trunc);
    if (!out) throw std::runtime_error("cannot open corona cache for writing: " + path);
    using namespace corona_projection_detail;
    write_scalar(out, cache_magic);
    write_scalar(out, cache_version);
    write_scalar(out, static_cast<std::uint32_t>(D));
    write_scalar(out, static_cast<std::int32_t>(graph.corona_rounds));
    write_scalar(out, static_cast<std::uint8_t>(graph.converged ? 1 : 0));
    write_scalar(out, static_cast<std::uint64_t>(graph.nodes.size()));
    for (const auto& node : graph.nodes) {
        write_scalar(out, static_cast<std::int64_t>(node.i));
        write_scalar(out, static_cast<std::int64_t>(node.j));
        for (const auto x : node.x) write_scalar(out, static_cast<std::int64_t>(x));
    }
    std::uint64_t edges = 0;
    for (const auto& row : graph.graph.out_adj) edges += row.size();
    write_scalar(out, edges);
    for (std::size_t source = 0; source < graph.graph.n; ++source)
        for (const auto& [destination, weight] : graph.graph.out_adj[source]) {
            write_scalar(out, static_cast<std::uint64_t>(source));
            write_scalar(out, static_cast<std::uint64_t>(destination));
            write_scalar(out, static_cast<std::int64_t>(weight));
        }
}

template <std::size_t D>
CoronaTruthGraph<D> load_corona_truth_graph(const std::string& path) {
    std::ifstream in(path, std::ios::binary);
    if (!in) throw std::runtime_error("cannot open corona cache for reading: " + path);
    using namespace corona_projection_detail;
    if (read_scalar<std::uint64_t>(in) != cache_magic)
        throw std::runtime_error("invalid corona cache magic");
    if (read_scalar<std::uint32_t>(in) != cache_version)
        throw std::runtime_error("unsupported corona cache version");
    if (read_scalar<std::uint32_t>(in) != D)
        throw std::runtime_error("corona cache dimension mismatch");
    CoronaTruthGraph<D> result;
    result.corona_rounds = read_scalar<std::int32_t>(in);
    result.converged = read_scalar<std::uint8_t>(in) != 0;
    const auto count = read_scalar<std::uint64_t>(in);
    result.nodes.resize(static_cast<std::size_t>(count));
    for (auto& node : result.nodes) {
        node.i = read_scalar<std::int64_t>(in);
        node.j = read_scalar<std::int64_t>(in);
        for (auto& x : node.x) x = read_scalar<std::int64_t>(in);
    }
    result.graph = WeightedDigraph(result.nodes.size());
    const auto edges = read_scalar<std::uint64_t>(in);
    for (std::uint64_t e = 0; e < edges; ++e) {
        const auto source = read_scalar<std::uint64_t>(in);
        const auto destination = read_scalar<std::uint64_t>(in);
        const auto weight = read_scalar<std::int64_t>(in);
        if (source >= result.nodes.size() || destination >= result.nodes.size())
            throw std::runtime_error("corona cache edge out of range");
        result.graph.add_edge(static_cast<std::size_t>(source),
                              static_cast<std::size_t>(destination), weight);
    }
    result.rebuild_index();
    return result;
}

} // namespace ravel
