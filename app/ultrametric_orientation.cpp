// app/ultrametric_orientation.cpp
//
// C++ port of python/ultrametric_orientation.py (+ the size-scan role
// of ultrametric_scaling.py, folded in as a --iterations/--crop sweep
// rather than a separate binary, since it is the same analysis run at
// different sizes).
//
// Follow-up to ultrametric_chsh.cpp, which showed S ~ 0 in every
// ultrametric stratum when the readout is the random-seeded CA state:
// the substitution hierarchy only enters the CA through its wiring, so
// random initial bits carry no lineage information, and conditioning
// on lineage cannot resurrect a correlation the observable never
// contained.
//
// Here the observable is instead the tiling's own deterministic
// structure: rotation class k(v) = round(theta/30) mod 12, with two
// fixed "polarizer" settings (0 degrees / 90 degrees). Nothing is
// injected -- the settings are two fixed functions applied identically
// to every tile, the ensemble is stratified by the substitution
// ultrametric u and restricted to graph-spacelike pairs (distance >
// CA max_depth), exactly as in ultrametric_chsh.cpp. Since outcomes
// are two fixed +-1 functions of frozen structure, |S|<=2 must hold;
// the question is whether S detaches from 0, i.e. whether the
// hierarchy is a real spacelike common cause for THIS observable.

#include <cmath>
#include <cstdio>
#include <map>
#include <set>
#include <string>
#include <vector>

#include "ravel/spectre_ca.hpp"
#include "ravel/spectre_geometry.hpp"

using namespace ravel::spectre;

namespace {

int rotation_class(const LineageTile& t) {
    long long k = (long long)std::llround(t.theta / 30.0);
    long long m = k % 12;
    if (m < 0) m += 12;
    return (int)m;
}

int orientation_outcome(int k, int setting) {
    if (setting == 0) return (k < 6) ? +1 : -1;
    int shifted = (k + 3) % 12;
    return (shifted < 6) ? +1 : -1;
}

void analyze(int iterations, std::size_t crop_count, unsigned ca_seed) {
    auto tiles = generate_tiling_with_lineage(iterations, crop_count);
    std::vector<std::set<int>> adjacency;
    std::vector<bool> is_boundary;
    build_adjacency(tiles, adjacency, is_boundary);
    std::size_t n = tiles.size();

    auto ca = run_ca(tiles, ca_seed);
    int lightcone = ca.max_depth;
    std::printf("--- iterations=%d crop=%zu ---\n", iterations, crop_count);
    std::printf("tiles: %zu   light-cone radius (max_depth) = %d\n", n, lightcone);

    auto dist = all_pairs_distances(adjacency);

    std::vector<int> kcls(n);
    for (std::size_t i = 0; i < n; ++i) kcls[i] = rotation_class(tiles[i]);

    std::map<int, std::vector<std::tuple<int,int,int>>> strata;  // u -> (a,b,graphdist)
    for (std::size_t a = 0; a < n; ++a)
        for (std::size_t b = a + 1; b < n; ++b) {
            int u = ultrametric_level(tiles[a].addr, tiles[b].addr, iterations);
            strata[u].push_back({(int)a, (int)b, dist[a][b]});
        }

    std::printf("%2s %10s %8s %8s %8s %8s %8s %9s %7s\n",
                "u", "spacelike", "E00", "E01", "E10", "E11", "S", "~1/sqrtN", "rigid%");
    std::vector<std::pair<int,int>> pooled;
    for (auto& [u, entries] : strata) {
        std::vector<std::pair<int,int>> sl;
        for (auto& [a, b, d] : entries) if (d > lightcone) sl.push_back({a, b});
        for (auto& p : sl) pooled.push_back(p);
        if (sl.size() < 30) { std::printf("%2d %10zu   (too few spacelike pairs)\n", u, sl.size()); continue; }
        auto corr = [&](int sa, int sb) {
            double t = 0;
            for (auto& [a, b] : sl) t += orientation_outcome(kcls[(std::size_t)a], sa) * orientation_outcome(kcls[(std::size_t)b], sb);
            return t / (double)sl.size();
        };
        double E00 = corr(0, 0), E01 = corr(0, 1), E10 = corr(1, 0), E11 = corr(1, 1);
        double S = E00 + E01 + E10 - E11;
        double se = 1.0 / std::sqrt((double)sl.size());
        std::map<int,int> rel_hist;
        for (auto& [a, b] : sl) {
            int rel = ((kcls[(std::size_t)a] - kcls[(std::size_t)b]) % 12 + 12) % 12;
            ++rel_hist[rel];
        }
        int best = 0; for (auto& [r, c] : rel_hist) best = std::max(best, c);
        double rigid = (double)best / (double)sl.size();
        std::printf("%2d %10zu %+8.4f %+8.4f %+8.4f %+8.4f %+8.4f %9.4f %6.1f%%\n",
                    u, sl.size(), E00, E01, E10, E11, S, se, 100.0 * rigid);
    }

    double t = 0;
    for (auto& [a, b] : pooled) t += orientation_outcome(kcls[(std::size_t)a], 0) * orientation_outcome(kcls[(std::size_t)b], 0);
    // pooled uniform spacelike ensemble control (what a graph-distance-only test would measure)
    auto pcorr = [&](int sa, int sb) {
        double tt = 0;
        for (auto& [a, b] : pooled) tt += orientation_outcome(kcls[(std::size_t)a], sa) * orientation_outcome(kcls[(std::size_t)b], sb);
        return tt / (double)pooled.size();
    };
    double E00 = pcorr(0, 0), E01 = pcorr(0, 1), E10 = pcorr(1, 0), E11 = pcorr(1, 1);
    double Sp = E00 + E01 + E10 - E11;
    std::printf("\npooled uniform spacelike ensemble (old-style): E00=%+.4f E01=%+.4f E10=%+.4f E11=%+.4f S=%+.4f (N=%zu)\n\n",
                E00, E01, E10, E11, Sp, pooled.size());
}

}  // namespace

int main(int argc, char** argv) {
    // Default: reproduces ultrametric_orientation.py's own run.
    // Pass --scaling to also reproduce ultrametric_scaling.py's sweep.
    bool scaling = false;
    for (int i = 1; i < argc; ++i) if (std::string(argv[i]) == "--scaling") scaling = true;

    analyze(5, 1000, 1);
    if (scaling) {
        analyze(6, 2000, 1);
        analyze(6, 3000, 1);
    }
    return 0;
}
