// ravel/spectre_ca.hpp
//
// Thin Spectre-specific glue over the generic reversible CA in
// substitution_lineage.hpp: builds the polygon list from LineageTile's
// `pts`, and supplies the one genuinely tiling-specific choice this
// system makes -- which tiles run the unconditional-NOT gate
// (is_not_gate_label, in spectre_geometry.hpp). Everything else
// (adjacency, causal depth, the reversible update rule itself) is the
// shared, tiling-agnostic machinery.

#pragma once

#include <set>
#include <vector>

#include "ravel/spectre_geometry.hpp"
#include "ravel/substitution_lineage.hpp"

namespace ravel::spectre {

using ravel::substitution_lineage::all_pairs_distances;
using ravel::substitution_lineage::CaRun;
using ravel::substitution_lineage::compute_depth;
using ravel::substitution_lineage::reference_neighbors;

inline void build_adjacency(const std::vector<LineageTile>& tiles,
                            std::vector<std::set<int>>& adjacency,
                            std::vector<bool>& is_boundary) {
    std::vector<std::vector<Point>> polys;
    polys.reserve(tiles.size());
    for (auto& t : tiles) polys.push_back(t.pts);
    ravel::substitution_lineage::build_adjacency(polys, adjacency, is_boundary);
}

inline CaRun run_ca(const std::vector<LineageTile>& tiles, unsigned seed = 0) {
    std::vector<std::set<int>> adjacency;
    std::vector<bool> is_boundary;
    build_adjacency(tiles, adjacency, is_boundary);
    std::set<int> not_indices;
    for (std::size_t i = 0; i < tiles.size(); ++i)
        if (is_not_gate_label(tiles[i].label)) not_indices.insert((int)i);
    return ravel::substitution_lineage::run_ca(tiles.size(), adjacency, is_boundary, not_indices, seed);
}

}  // namespace ravel::spectre
