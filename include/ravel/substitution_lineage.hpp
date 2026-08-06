// ravel/substitution_lineage.hpp
//
// GENERIC machinery for a recursive geometric substitution system with
// retained ancestor addresses ("lineage"), plus the generic graph/CA
// tools built on top of it. None of this file knows about any specific
// tiling -- no tile names, no coordinates, no substitution rules. A
// concrete tiling (e.g. Spectre; see spectre_geometry.hpp) supplies its
// own geometry and rules and reuses everything here unchanged.
//
// What's here:
//   - plain 2D affine transform arithmetic;
//   - ShapeNode/ShapeSystem: a recursive tree of labelled shapes, each
//     either a leaf (base tile) or a metatile whose children carry
//     their own local transform -- generic over whatever labels and
//     transforms a caller builds;
//   - draw_recursive: walks such a tree, collecting (label, cumulative
//     transform, root-first child-index address) for every leaf;
//   - ultrametric_level: the standard p-adic-style tree distance on
//     two addresses (inflation level of their least common ancestor);
//   - build_adjacency/compute_depth/reference_neighbors/
//     all_pairs_distances: pure polygon/graph operations, needing only
//     each tile's boundary point list;
//   - run_ca: the reversible second-order CA update, parametrized by
//     which tile INDICES get the unconditional-NOT gate instead of the
//     neighbor-dependent one (a concrete tiling decides which tiles
//     that is, e.g. Spectre's Gamma/Gamma1/Gamma2 -- this file makes
//     no assumption about labels at all).

#pragma once

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <deque>
#include <map>
#include <memory>
#include <random>
#include <set>
#include <string>
#include <vector>

namespace ravel::substitution_lineage {

using Point = std::array<double, 2>;
// Affine transform (a,b,c, d,e,f): x' = a*x+b*y+c, y' = d*x+e*y+f.
using Affine = std::array<double, 6>;

inline const Affine kIdentity = {1, 0, 0, 0, 1, 0};

inline Affine affine_mul(const Affine& A, const Affine& B) {
    return {
        A[0]*B[0]+A[1]*B[3], A[0]*B[1]+A[1]*B[4], A[0]*B[2]+A[1]*B[5]+A[2],
        A[3]*B[0]+A[4]*B[3], A[3]*B[1]+A[4]*B[4], A[3]*B[2]+A[4]*B[5]+A[5]
    };
}
inline Affine affine_rot(double a) {
    double c = std::cos(a), s = std::sin(a);
    return {c, -s, 0, s, c, 0};
}
inline Affine affine_trans(double tx, double ty) { return {1, 0, tx, 0, 1, ty}; }
inline Affine affine_trans_to(const Point& p, const Point& q) {
    return affine_trans(q[0] - p[0], q[1] - p[1]);
}
inline Point affine_apply(const Affine& M, const Point& p) {
    return {M[0]*p[0] + M[1]*p[1] + M[2], M[3]*p[0] + M[4]*p[1] + M[5]};
}

// =====================================================================
// The recursive substitution tree
// =====================================================================

struct ShapeNode {
    std::string label;               // leaf label, or empty for a metatile
    std::vector<std::pair<const ShapeNode*, Affine>> children;
};

// Owns every ShapeNode ever created (leaves and metatiles at every
// inflation level) so ShapeNode pointers stay valid; `current` is
// whatever inflation level a builder has reached, keyed by label.
struct ShapeSystem {
    std::vector<std::unique_ptr<ShapeNode>> pool;
    std::map<std::string, const ShapeNode*> current;

    ShapeNode* make_node() { pool.push_back(std::make_unique<ShapeNode>()); return pool.back().get(); }
};

// =====================================================================
// Generic quad-based rep-tile supertile assembly
// =====================================================================
//
// The pattern common to "assemble a level-(k+1) supertile from
// level-k pieces by chaining edge-matching transformations, then
// derive the next quad from a few of those transformations" is not
// specific to any one tiling -- only the concrete angle/index DATA is.
// A concrete tiling supplies:
//   - an initial reference quad (4 points used to align neighbours);
//   - `assembly_rules`: an ordered list of (angle_degrees, from_index,
//     to_index) steps, each producing one more child placement
//     transform by rotating (if angle != 0) and then translating the
//     rotated quad's `to_index` corner onto the PREVIOUS transform's
//     image of the quad's `from_index` corner;
//   - a fixed `mirror` transform applied to every placement transform
//     at the end (Spectre's is a horizontal flip; a tiling with no
//     reflection symmetry in its rule can pass the identity);
//   - `next_quad_rule`: which four (transform_index, quad_corner_index)
//     pairs to apply to build the NEXT level's reference quad;
//   - `super_rules`: for each label, which child label (or empty) goes
//     in each of the resulting transform slots.
struct SupertileAssemblyRule { double angle_degrees; int from; int to; };
struct QuadCorner { int transform_index; int corner_index; };

inline std::array<Point,4> assemble_supertiles(
    ShapeSystem& sys,
    const std::array<Point,4>& quad,
    const Affine& mirror,
    const std::vector<SupertileAssemblyRule>& assembly_rules,
    const std::array<QuadCorner,4>& next_quad_rule,
    const std::map<std::string, std::vector<std::string>>& super_rules) {
    std::vector<Affine> transformations = {kIdentity};
    double total_angle = 0;
    Affine rotation = kIdentity;
    std::array<Point,4> transformed_quad = quad;
    for (const auto& r : assembly_rules) {
        if (r.angle_degrees != 0) {
            total_angle += r.angle_degrees;
            rotation = affine_rot(total_angle * M_PI / 180.0);
            for (std::size_t i = 0; i < 4; ++i) transformed_quad[i] = affine_apply(rotation, quad[i]);
        }
        Affine ttt = affine_trans_to(transformed_quad[(std::size_t)r.to],
                                      affine_apply(transformations.back(), quad[(std::size_t)r.from]));
        transformations.push_back(affine_mul(ttt, rotation));
    }
    for (auto& t : transformations) t = affine_mul(mirror, t);

    std::array<Point,4> next_quad;
    for (std::size_t i = 0; i < 4; ++i) {
        const auto& qc = next_quad_rule[i];
        next_quad[i] = affine_apply(transformations[(std::size_t)qc.transform_index], quad[(std::size_t)qc.corner_index]);
    }

    std::map<std::string, const ShapeNode*> next;
    for (const auto& [label, subs] : super_rules) {
        ShapeNode* meta = sys.make_node();
        for (std::size_t i = 0; i < subs.size(); ++i) {
            if (subs[i].empty()) continue;
            meta->children.push_back({sys.current.at(subs[i]), transformations[i]});
        }
        next[label] = meta;
    }
    sys.current = std::move(next);
    return next_quad;
}

struct LineageLeaf {
    std::string label;
    Affine T;
    std::vector<int> addr;  // root-first child-index address
};

inline void draw_recursive(const ShapeNode* node, const Affine& T, std::vector<int>& path,
                            std::vector<LineageLeaf>& out) {
    if (!node->label.empty()) {
        out.push_back({node->label, T, path});
        return;
    }
    for (std::size_t i = 0; i < node->children.size(); ++i) {
        auto& [child, shapeT] = node->children[i];
        path.push_back((int)i);
        draw_recursive(child, affine_mul(T, shapeT), path, out);
        path.pop_back();
    }
}

// Inflation level of the least common ancestor supertile of two
// addresses (only the first `iterations` slots count as substitution
// levels; a trailing sub-tile-split index, if present, is not one).
inline int ultrametric_level(const std::vector<int>& addr_a, const std::vector<int>& addr_b, int iterations) {
    int c = 0;
    int n = std::min({iterations, (int)addr_a.size(), (int)addr_b.size()});
    for (int i = 0; i < n; ++i) {
        if (addr_a[(std::size_t)i] != addr_b[(std::size_t)i]) break;
        ++c;
    }
    return iterations - c;
}

// =====================================================================
// Generic graph/geometry tools (need only each tile's polygon points)
// =====================================================================

inline std::pair<double,double> rounded(double x, double y, int precision = 3) {
    double scale = std::pow(10.0, precision);
    return {std::round(x * scale) / scale, std::round(y * scale) / scale};
}

// Builds tile-adjacency from shared (rounded) polygon edges. `polys[i]`
// is tile i's boundary points, in order.
inline void build_adjacency(const std::vector<std::vector<Point>>& polys,
                            std::vector<std::set<int>>& adjacency,
                            std::vector<bool>& is_boundary) {
    using Key = std::pair<std::pair<double,double>, std::pair<double,double>>;
    std::map<Key, std::vector<int>> edge_map;
    for (std::size_t i = 0; i < polys.size(); ++i) {
        const auto& pts = polys[i];
        std::size_t n = pts.size();
        for (std::size_t k = 0; k < n; ++k) {
            auto a = rounded(pts[k][0], pts[k][1]);
            auto b = rounded(pts[(k + 1) % n][0], pts[(k + 1) % n][1]);
            Key key = (a <= b) ? Key{a, b} : Key{b, a};
            edge_map[key].push_back((int)i);
        }
    }
    adjacency.assign(polys.size(), {});
    is_boundary.assign(polys.size(), false);
    for (auto& [key, idxs] : edge_map) {
        if (idxs.size() == 2 && idxs[0] != idxs[1]) {
            adjacency[(std::size_t)idxs[0]].insert(idxs[1]);
            adjacency[(std::size_t)idxs[1]].insert(idxs[0]);
        } else {
            for (int i : idxs) is_boundary[(std::size_t)i] = true;
        }
    }
}

inline std::vector<int> compute_depth(const std::vector<std::set<int>>& adjacency,
                                      const std::vector<bool>& is_boundary) {
    std::size_t n = adjacency.size();
    std::vector<int> depth(n, -1);
    std::deque<int> q;
    for (std::size_t i = 0; i < n; ++i) if (is_boundary[i]) { depth[i] = 0; q.push_back((int)i); }
    while (!q.empty()) {
        int u = q.front(); q.pop_front();
        for (int v : adjacency[(std::size_t)u]) {
            if (depth[(std::size_t)v] == -1) { depth[(std::size_t)v] = depth[(std::size_t)u] + 1; q.push_back(v); }
        }
    }
    for (std::size_t i = 0; i < n; ++i) if (depth[i] == -1) depth[i] = 0;
    return depth;
}

inline std::vector<std::vector<int>> reference_neighbors(const std::vector<std::set<int>>& adjacency) {
    std::vector<std::vector<int>> refs;
    refs.reserve(adjacency.size());
    for (auto& nbrs : adjacency) {
        std::vector<int> ordered(nbrs.begin(), nbrs.end());  // std::set is already sorted
        if (ordered.size() > 2) ordered.resize(2);
        refs.push_back(ordered);
    }
    return refs;
}

inline std::vector<std::vector<int>> all_pairs_distances(const std::vector<std::set<int>>& adjacency) {
    std::size_t n = adjacency.size();
    std::vector<std::vector<int>> dist(n, std::vector<int>(n, -1));
    for (std::size_t s = 0; s < n; ++s) {
        auto& d = dist[s];
        d[s] = 0;
        std::deque<int> q{(int)s};
        while (!q.empty()) {
            int u = q.front(); q.pop_front();
            for (int v : adjacency[(std::size_t)u]) {
                if (d[(std::size_t)v] == -1) { d[(std::size_t)v] = d[(std::size_t)u] + 1; q.push_back(v); }
            }
        }
    }
    return dist;
}

// =====================================================================
// Generic reversible second-order CA
// =====================================================================
//
// s(v,t+1) = s(v,t-1) XOR F(neighbors of v at time t) -- a Fredkin/
// Margolus-style reversible update. F is a Toffoli-gate AND of two
// reference neighbors (CNOT if one neighbor, identity if none), EXCEPT
// for tiles in `not_indices`, which run an unconditional NOT instead;
// which tiles those are is entirely the caller's choice (a concrete
// tiling might plant it on tiles with a特定 structural role -- this
// file has no opinion). Tiles frozen at depth 0 are boundary tiles
// (missing a real neighbor a larger tiling would supply); every other
// tile freezes once t reaches its graph distance to the nearest one.

enum class GateType { NOT_, TOFFOLI, CNOT, ID };

struct CaSnapshot {
    std::vector<int> state;
    std::vector<bool> active;
};

struct CaRun {
    std::vector<CaSnapshot> history;
    std::vector<int> depth;
    int max_depth{};
    std::vector<GateType> gate_type;
};

inline CaRun run_ca(std::size_t n, const std::vector<std::set<int>>& adjacency,
                    const std::vector<bool>& is_boundary, const std::set<int>& not_indices,
                    unsigned seed = 0) {
    auto depth = compute_depth(adjacency, is_boundary);
    auto refs = reference_neighbors(adjacency);
    int max_depth = *std::max_element(depth.begin(), depth.end());

    std::vector<GateType> gate_type(n);
    for (std::size_t i = 0; i < n; ++i) {
        if (not_indices.count((int)i)) gate_type[i] = GateType::NOT_;
        else if (refs[i].size() == 2) gate_type[i] = GateType::TOFFOLI;
        else if (refs[i].size() == 1) gate_type[i] = GateType::CNOT;
        else gate_type[i] = GateType::ID;
    }

    std::mt19937 rng(seed);
    std::uniform_int_distribution<int> bit(0, 1);
    std::vector<int> s_prev(n), s_curr(n);
    for (std::size_t i = 0; i < n; ++i) s_prev[i] = s_curr[i] = bit(rng);

    CaRun out;
    out.depth = depth;
    out.max_depth = max_depth;
    out.gate_type = gate_type;

    std::vector<bool> active0(n);
    for (std::size_t i = 0; i < n; ++i) active0[i] = depth[i] > 0;
    out.history.push_back({s_curr, active0});

    for (int t = 0; t < max_depth; ++t) {
        std::vector<bool> active(n);
        for (std::size_t i = 0; i < n; ++i) active[i] = depth[i] > t;
        std::vector<int> s_next = s_curr;
        for (std::size_t i = 0; i < n; ++i) {
            if (!active[i]) continue;
            int gate = 0;
            switch (gate_type[i]) {
                case GateType::NOT_: gate = 1; break;
                case GateType::TOFFOLI: gate = s_curr[(std::size_t)refs[i][0]] & s_curr[(std::size_t)refs[i][1]]; break;
                case GateType::CNOT: gate = s_curr[(std::size_t)refs[i][0]]; break;
                case GateType::ID: gate = 0; break;
            }
            s_next[i] = s_prev[i] ^ gate;
        }
        s_prev = s_curr;
        s_curr = s_next;
        std::vector<bool> active_after(n);
        for (std::size_t i = 0; i < n; ++i) active_after[i] = depth[i] > t + 1;
        out.history.push_back({s_curr, active_after});
    }
    return out;
}

}  // namespace ravel::substitution_lineage
