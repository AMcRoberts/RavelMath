// Sparse, bounded analysis of bp_dump_provenance text output.
#pragma once

#include <algorithm>
#include <cstddef>
#include <fstream>
#include <map>
#include <queue>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

namespace ravel {

struct BpDumpLimits {
    std::size_t max_bytes = 64ULL << 20;
    std::size_t max_states = 100000;
    std::size_t max_edges = 2000000;
};

struct BpDumpAnalysis {
    std::size_t n = 0, core_size = 0, edge_records = 0, extra_rows = 0;
    bool correction_nilpotent = false;
    std::size_t nilpotency_index = 0;
    bool correction_lower_triangular_by_length = false;
    std::size_t branching_states = 0;
    bool chunk_counts_are_powers_of_two = false;
    bool position_zero_is_transient = false;
    bool designated_is_max_v2_position = false;
};

namespace bp_dump_detail {
struct Edge { std::size_t parent, child, pos, chunks; };
inline std::size_t v2(std::size_t value) {
    std::size_t result = 0;
    while (value && (value & 1U) == 0) {
        ++result;
        value >>= 1U;
    }
    return result;
}
}  // namespace bp_dump_detail

inline BpDumpAnalysis analyze_bp_dump(
        std::istream& input, const BpDumpLimits& limits = {}) {
    using bp_dump_detail::Edge;
    BpDumpAnalysis result;
    std::vector<std::size_t> lengths;
    std::vector<Edge> edges;
    std::size_t bytes = 0;
    enum class Section { none, states, edges } section = Section::none;
    std::string line;
    while (std::getline(input, line)) {
        bytes += line.size() + 1;
        if (bytes > limits.max_bytes)
            throw std::runtime_error("analyze_bp_dump: byte cap exceeded");
        if (line.empty()) continue;
        if (line[0] == '#') {
            std::istringstream header(line.substr(1));
            std::string token;
            while (header >> token) {
                if (token.rfind("n=", 0) == 0)
                    result.n = std::stoull(token.substr(2));
                else if (token.rfind("recurrent_core_size=", 0) == 0)
                    result.core_size = std::stoull(token.substr(20));
            }
            continue;
        }
        if (line == "STATES") { section = Section::states; continue; }
        if (line == "EDGES") { section = Section::edges; continue; }
        std::istringstream row(line);
        char kind = 0;
        row >> kind;
        if (section == Section::states && kind == 'S') {
            std::size_t index = 0, length = 0;
            std::string u, v;
            if (!(row >> index >> u >> v >> length))
                throw std::runtime_error("analyze_bp_dump: malformed state row");
            if (index >= limits.max_states)
                throw std::runtime_error("analyze_bp_dump: state cap exceeded");
            if (lengths.size() <= index) lengths.resize(index + 1);
            lengths[index] = length;
        } else if (section == Section::edges && kind == 'E') {
            Edge edge{};
            std::size_t child_length = 0;
            if (!(row >> edge.parent >> edge.child >> edge.pos
                      >> edge.chunks >> child_length))
                throw std::runtime_error("analyze_bp_dump: malformed edge row");
            if (edges.size() >= limits.max_edges)
                throw std::runtime_error("analyze_bp_dump: edge cap exceeded");
            edges.push_back(edge);
        } else {
            throw std::runtime_error("analyze_bp_dump: invalid section row");
        }
    }
    if (result.core_size == 0 || lengths.size() != result.core_size)
        throw std::runtime_error("analyze_bp_dump: incomplete state table");
    for (const auto& e : edges)
        if (e.parent >= result.core_size || e.child >= result.core_size)
            throw std::runtime_error("analyze_bp_dump: edge endpoint out of range");
    result.edge_records = edges.size();

    std::vector<std::map<std::size_t, std::size_t>> weights(result.core_size);
    std::vector<std::map<std::size_t, std::vector<std::size_t>>> positions(
        result.core_size);
    std::vector<std::size_t> chunks(result.core_size, 0);
    for (const auto& e : edges) {
        ++weights[e.parent][e.child];
        positions[e.parent][e.child].push_back(e.pos);
        if (chunks[e.parent] && chunks[e.parent] != e.chunks)
            throw std::runtime_error("analyze_bp_dump: inconsistent chunk count");
        chunks[e.parent] = e.chunks;
    }

    std::vector<std::size_t> permutation(result.core_size);
    std::vector<std::size_t> extra;
    for (std::size_t row = 0; row < result.core_size; ++row) {
        std::size_t unit_count = 0;
        bool has_extra = false;
        for (const auto& [child, weight] : weights[row]) {
            if (weight == 1) {
                permutation[row] = child;
                ++unit_count;
            } else if (weight > 1) {
                has_extra = true;
            }
        }
        if (unit_count != 1)
            throw std::runtime_error(
                "analyze_bp_dump: row lacks unique weight-one successor");
        if (has_extra) extra.push_back(row);
    }
    std::vector<std::size_t> column_hits(result.core_size, 0);
    for (auto child : permutation) ++column_hits[child];
    if (std::any_of(column_hits.begin(), column_hits.end(),
                    [](std::size_t x) { return x != 1; }))
        throw std::runtime_error(
            "analyze_bp_dump: weight-one part is not a permutation");
    result.extra_rows = extra.size();

    std::vector<std::vector<std::size_t>> correction(extra.size());
    std::vector<std::size_t> indegree(extra.size(), 0);
    for (std::size_t a = 0; a < extra.size(); ++a) {
        for (std::size_t b = 0; b < extra.size(); ++b) {
            auto it = weights[extra[a]].find(permutation[extra[b]]);
            if (it != weights[extra[a]].end() && it->second > 1) {
                correction[a].push_back(b);
                ++indegree[b];
            }
        }
    }

    std::queue<std::size_t> ready;
    std::vector<std::size_t> path_vertices(extra.size(), 1);
    for (std::size_t i = 0; i < extra.size(); ++i)
        if (indegree[i] == 0) ready.push(i);
    std::size_t visited = 0, longest = extra.empty() ? 0 : 1;
    while (!ready.empty()) {
        auto u = ready.front();
        ready.pop();
        ++visited;
        longest = std::max(longest, path_vertices[u]);
        for (auto v : correction[u]) {
            path_vertices[v] =
                std::max(path_vertices[v], path_vertices[u] + 1);
            if (--indegree[v] == 0) ready.push(v);
        }
    }
    result.correction_nilpotent = visited == extra.size();
    result.nilpotency_index =
        result.correction_nilpotent ? (extra.empty() ? 1 : longest) : 0;

    std::vector<std::size_t> order(extra.size());
    for (std::size_t i = 0; i < extra.size(); ++i) order[i] = i;
    std::stable_sort(order.begin(), order.end(), [&](auto a, auto b) {
        return lengths[extra[a]] < lengths[extra[b]];
    });
    std::vector<std::size_t> rank(extra.size());
    for (std::size_t i = 0; i < order.size(); ++i) rank[order[i]] = i;
    result.correction_lower_triangular_by_length = true;
    for (std::size_t a = 0; a < correction.size(); ++a)
        for (auto b : correction[a])
            if (rank[a] < rank[b])
                result.correction_lower_triangular_by_length = false;

    std::size_t pow2_ok = 0, pos0_transient = 0, designated_ok = 0;
    for (std::size_t parent = 0; parent < result.core_size; ++parent) {
        if (chunks[parent] <= 1) continue;
        ++result.branching_states;
        if ((chunks[parent] & (chunks[parent] - 1)) == 0) ++pow2_ok;
        bool has_zero = false;
        for (const auto& [child, pos] : positions[parent])
            for (auto p : pos) if (p == 0) has_zero = true;
        if (!has_zero) ++pos0_transient;
        std::size_t max_value = 0;
        for (std::size_t p = 1; p < chunks[parent]; ++p)
            max_value = std::max(max_value, bp_dump_detail::v2(p));
        std::vector<std::size_t> expected;
        for (std::size_t p = 1; p < chunks[parent]; ++p)
            if (bp_dump_detail::v2(p) == max_value) expected.push_back(p);
        auto actual = positions[parent][permutation[parent]];
        std::sort(actual.begin(), actual.end());
        if (actual == expected) ++designated_ok;
    }
    result.chunk_counts_are_powers_of_two =
        pow2_ok == result.branching_states;
    result.position_zero_is_transient =
        pos0_transient == result.branching_states;
    result.designated_is_max_v2_position =
        designated_ok == result.branching_states;
    return result;
}

inline BpDumpAnalysis analyze_bp_dump_file(
        const std::string& path, const BpDumpLimits& limits = {}) {
    std::ifstream input(path);
    if (!input)
        throw std::runtime_error("analyze_bp_dump_file: cannot open file");
    return analyze_bp_dump(input, limits);
}

}  // namespace ravel
