// bp_dump_provenance.cpp
//
// "Generate the input once" tool for the thread-A determinant
// investigation (see docs/RESEARCH_STATUS.md "Attacking det(Q_sym_BP)
// directly" and the follow-up in bp_correction_reduction.cpp).
//
// Running the balanced-pair BFS + recurrent-core extraction is the
// expensive, slow part of every hypothesis test in this
// investigation; every previous driver re-ran it from scratch for
// every question. This tool runs it ONCE per n and dumps everything
// a downstream analysis could need as plain text:
//
//   - every recurrent-core state: index, u, v, word length, whether
//     it's a coincidence state (excluded already, kept for context)
//   - every edge in the recurrent core: parent index, child index,
//     the child's POSITION in the parent's own reduce_pair(sigma_pair
//     (parent)) chunk list (0-based), the child's word length, and
//     the multiplicity (how many times this exact child recurs in the
//     parent's own chunk list -- this IS the "weight" in A_full).
//
// This lets any further question (which chunk position is the
// "designated"/weight-1 successor; whether extra edges always point
// to strictly shorter states; whatever hypothesis comes up next) be
// answered by a fast Python pass over the dump, without recompiling
// or re-running this (multi-second, corona-pipeline-adjacent) BFS
// again. Run once per n; analyze many times.
//
// Usage: bp_dump_provenance <n> <output_file>
//
// Build with `make bp_dump_provenance`.

#include <cstdio>
#include <cstdlib>
#include <deque>
#include <fstream>
#include <map>
#include <string>
#include <vector>

#include "ravel/balanced_pair.hpp"
#include "ravel/graph_divisor.hpp"
#include "ravel/substitution.hpp"

using namespace ravel;

namespace {

std::vector<std::vector<std::int8_t>> n_bonacci_rule(std::size_t n) {
    std::vector<std::vector<std::int8_t>> sigma(n);
    for (std::size_t i = 0; i + 1 < n; ++i) sigma[i] = {0, static_cast<std::int8_t>(i + 1)};
    sigma[n - 1] = {0};
    return sigma;
}

std::string word_str(const std::vector<std::int8_t>& w) {
    std::string s;
    for (auto c : w) s += std::to_string(static_cast<int>(c));
    return s;
}

}  // namespace

int main(int argc, char** argv) {
    if (argc < 3) {
        std::fprintf(stderr, "usage: %s <n> <output_file>\n", argv[0]);
        return 1;
    }
    std::size_t n = static_cast<std::size_t>(std::atoi(argv[1]));
    std::string outpath = argv[2];

    auto sigma = n_bonacci_rule(n);
    SubstitutionRule rule(sigma);
    const std::size_t nl = rule.alphabet_size();

    // Full BFS over irreducible balanced pairs (same as
    // balanced_pair.hpp's internals, duplicated per this project's
    // existing convention -- see that file's top-of-file note),
    // but this time recording, per parent, the FULL ORDERED chunk
    // list from reduce_pair (not just the deduplicated child index
    // set), so chunk position and multiplicity are both recoverable.
    std::map<std::string, std::size_t> seen;
    std::map<std::string, std::vector<std::size_t>> succ_ordered;  // parent -> ordered chunk indices (with repeats)
    std::deque<std::size_t> queue;
    std::vector<std::string> order;
    {
        auto initial = reduce_pair({0, 1}, {1, 0}, nl);
        for (auto& ch : initial) {
            std::string k = pair_key(ch.first, ch.second);
            if (!seen.count(k)) { seen[k] = order.size(); order.push_back(k); queue.push_back(order.size() - 1); }
        }
    }
    constexpr std::size_t max_pairs = 20000, max_len = 60000;
    bool ok = true;
    while (!queue.empty()) {
        std::size_t idx = queue.front(); queue.pop_front();
        std::string k = order[idx];
        std::vector<std::int8_t> u, v;
        unpair_key(k, u, v);
        auto img = sigma_pair(rule, {u, v});
        if (img.first.size() > max_len || seen.size() > max_pairs) { ok = false; break; }
        auto chunks = reduce_pair(img.first, img.second, nl);
        std::vector<std::size_t> children;  // ORDERED, with repeats
        for (auto& ch : chunks) {
            std::string ck = pair_key(ch.first, ch.second);
            auto it = seen.find(ck);
            std::size_t ci;
            if (it == seen.end()) { ci = order.size(); seen[ck] = ci; order.push_back(ck); queue.push_back(ci); }
            else ci = it->second;
            children.push_back(ci);
        }
        succ_ordered[k] = std::move(children);
    }
    if (!ok) {
        std::fprintf(stderr, "BFS did not terminate within caps for n=%zu\n", n);
        return 1;
    }

    // Identify noncoincidence states (the recurrent-core candidates).
    std::map<std::size_t, std::size_t> idx_map;
    std::vector<std::size_t> noncoin;
    for (std::size_t i = 0; i < order.size(); ++i) {
        std::vector<std::int8_t> u, v;
        unpair_key(order[i], u, v);
        if (!(u.size() == 1 && u == v)) { idx_map[i] = noncoin.size(); noncoin.push_back(i); }
    }
    std::size_t nn = noncoin.size();

    // Build the dense (deduplicated-weight) noncoincidence transition
    // graph, same as every previous driver, to run extract_recurrent_core.
    std::vector<std::vector<long long>> full_dense(nn, std::vector<long long>(nn, 0));
    for (std::size_t i : noncoin) {
        auto it = succ_ordered.find(order[i]);
        if (it == succ_ordered.end()) continue;
        for (std::size_t cj : it->second) {
            auto jt = idx_map.find(cj);
            if (jt != idx_map.end()) full_dense[idx_map[i]][jt->second] += 1;
        }
    }
    auto full_graph = WeightedDigraph::from_dense(full_dense);
    auto [core, orig_idx] = extract_recurrent_core(full_graph);

    // orig_idx[c] = index into `noncoin` for recurrent-core state c.
    // Map back to the ORIGINAL BFS index (into `order`) for each core state.
    std::vector<std::size_t> core_to_bfs(core.n);
    for (std::size_t c = 0; c < core.n; ++c) core_to_bfs[c] = noncoin[orig_idx[c]];
    std::map<std::size_t, std::size_t> bfs_to_core;
    for (std::size_t c = 0; c < core.n; ++c) bfs_to_core[core_to_bfs[c]] = c;

    std::ofstream out(outpath);
    out << "# n=" << n << " recurrent_core_size=" << core.n << "\n";
    out << "STATES\n";
    for (std::size_t c = 0; c < core.n; ++c) {
        std::vector<std::int8_t> u, v;
        unpair_key(order[core_to_bfs[c]], u, v);
        out << "S " << c << " " << word_str(u) << " " << word_str(v) << " " << u.size() << "\n";
    }

    out << "EDGES\n";
    // For each recurrent-core state (parent), walk its ORIGINAL ordered
    // chunk list (with repeats), and for every chunk landing back in
    // the recurrent core, emit (parent, child, chunk_position, mult
    // computed downstream in Python from repeated rows).
    for (std::size_t c = 0; c < core.n; ++c) {
        std::size_t bfs_idx = core_to_bfs[c];
        auto it = succ_ordered.find(order[bfs_idx]);
        if (it == succ_ordered.end()) continue;
        const auto& chunks = it->second;
        for (std::size_t pos = 0; pos < chunks.size(); ++pos) {
            std::size_t child_bfs = chunks[pos];
            auto ct = bfs_to_core.find(child_bfs);
            if (ct == bfs_to_core.end()) continue;  // child fell outside the recurrent core
            std::size_t child_core = ct->second;
            std::vector<std::int8_t> cu, cv;
            unpair_key(order[child_bfs], cu, cv);
            out << "E " << c << " " << child_core << " " << pos << " " << chunks.size() << " " << cu.size() << "\n";
        }
    }
    out.close();
    std::printf("wrote %s (n=%zu, recurrent core size %zu)\n", outpath.c_str(), n, core.n);
    return 0;
}
