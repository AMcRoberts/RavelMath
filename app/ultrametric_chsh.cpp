// app/ultrametric_chsh.cpp
//
// C++ port of python/ultrametric_chsh.py.
//
// spectre_chsh.py (the ancestor of this file, Python-only, superseded
// by this one -- not ported) certified "spacelike" with GRAPH distance
// and then sampled pairs UNIFORMLY over the crop, getting S ~ 0. Here
// the pair ensemble is instead stratified by the tiling's own
// substitution ULTRAMETRIC u(A,B) = inflation level of the least
// common ancestor supertile, restricted throughout to pairs that are
// GRAPH-SPACELIKE for the real CA run (graph_distance > max_depth).
//
// Settings: setting0[i] = sorted(neighbors[i])[:2], setting1[i] =
// sorted(neighbors[i])[-2:]. Outcome: XOR of the two referenced final
// frozen CA bits, mapped to +-1 (single-neighbor tiles use that one
// bit; both defined so |S|<=2 holds algebraically, since outcomes
// remain two fixed functions of the frozen state). This file is the
// documented NULL RESULT (S stays near 0 in every stratum when the
// readout is the CA's own frozen state, because the random initial
// bits carry no lineage information) -- the positive result is in
// ultrametric_orientation.cpp.

#include <cmath>
#include <cstdio>
#include <map>
#include <set>
#include <vector>

#include "ravel/spectre_ca.hpp"
#include "ravel/spectre_geometry.hpp"

using namespace ravel::spectre;

namespace {

std::vector<std::vector<int>> setting1_from(const std::vector<std::set<int>>& adjacency) {
    std::vector<std::vector<int>> out;
    out.reserve(adjacency.size());
    for (auto& nbrs : adjacency) {
        std::vector<int> ordered(nbrs.begin(), nbrs.end());
        if (ordered.size() >= 2) ordered = {ordered[ordered.size() - 2], ordered.back()};
        out.push_back(ordered);
    }
    return out;
}

int outcome(const std::vector<int>& final_state, const std::vector<int>& pair) {
    if (pair.size() == 2) {
        int bit = final_state[(std::size_t)pair[0]] ^ final_state[(std::size_t)pair[1]];
        return 1 - 2 * bit;
    }
    if (pair.size() == 1) return 1 - 2 * final_state[(std::size_t)pair[0]];
    return 1;
}

}  // namespace

int main(int argc, char** argv) {
    int iterations = 5;
    std::size_t crop_count = 1000;
    std::vector<unsigned> ca_seeds = {1, 7, 42};
    if (argc > 1) iterations = std::atoi(argv[1]);
    if (argc > 2) crop_count = (std::size_t)std::atoll(argv[2]);

    auto tiles = generate_tiling_with_lineage(iterations, crop_count);
    std::vector<std::set<int>> adjacency;
    std::vector<bool> is_boundary;
    build_adjacency(tiles, adjacency, is_boundary);
    std::size_t n = tiles.size();
    std::printf("tiles: %zu   (iterations=%d)\n", n, iterations);

    std::printf("computing full graph-distance matrix ...\n");
    auto dist = all_pairs_distances(adjacency);

    auto setting0 = reference_neighbors(adjacency);  // sorted(nbrs)[:2]
    auto setting1 = setting1_from(adjacency);         // sorted(nbrs)[-2:]

    std::vector<int> valid;
    for (std::size_t i = 0; i < n; ++i) {
        if (adjacency[i].size() < 2) continue;
        std::set<int> s0(setting0[i].begin(), setting0[i].end());
        std::set<int> s1(setting1[i].begin(), setting1[i].end());
        if (s0 != s1) valid.push_back((int)i);
    }
    std::printf("tiles with two distinct natural settings: %zu\n", valid.size());

    std::map<int, std::vector<std::pair<int,int>>> strata_all;  // u -> (a,b) among valid, all graph-dists
    std::map<int, std::vector<int>> strata_dist;
    for (std::size_t ia = 0; ia < valid.size(); ++ia) {
        int a = valid[ia];
        for (std::size_t ib = ia + 1; ib < valid.size(); ++ib) {
            int b = valid[ib];
            int u = ultrametric_level(tiles[(std::size_t)a].addr, tiles[(std::size_t)b].addr, iterations);
            strata_all[u].push_back({a, b});
        }
    }

    for (unsigned seed : ca_seeds) {
        auto ca = run_ca(tiles, seed);
        const auto& final_state = ca.history.back().state;
        int lightcone = ca.max_depth;
        std::printf("\n=== CA seed %u   light-cone radius (max_depth) = %d ===\n", seed, lightcone);
        std::printf("%2s %10s %10s %8s %8s %8s %8s %8s %9s\n",
                     "u", "pairs(all)", "spacelike", "E00", "E01", "E10", "E11", "S", "~1/sqrtN");
        for (auto& [u, pairs] : strata_all) {
            std::vector<std::pair<int,int>> sl;
            for (auto& [a, b] : pairs) if (dist[(std::size_t)a][(std::size_t)b] > lightcone) sl.push_back({a, b});
            if (sl.size() < 30) {
                std::printf("%2d %10zu %10zu   (too few spacelike pairs)\n", u, pairs.size(), sl.size());
                continue;
            }
            auto corr = [&](const std::vector<std::vector<int>>& A, const std::vector<std::vector<int>>& B) {
                double t = 0;
                for (auto& [a, b] : sl) t += outcome(final_state, A[(std::size_t)a]) * outcome(final_state, B[(std::size_t)b]);
                return t / (double)sl.size();
            };
            double E00 = corr(setting0, setting0), E01 = corr(setting0, setting1);
            double E10 = corr(setting1, setting0), E11 = corr(setting1, setting1);
            double S = E00 + E01 + E10 - E11;
            double se = 1.0 / std::sqrt((double)sl.size());
            std::printf("%2d %10zu %10zu %+8.4f %+8.4f %+8.4f %+8.4f %+8.4f %9.4f\n",
                        u, pairs.size(), sl.size(), E00, E01, E10, E11, S, se);
        }
    }
    return 0;
}
