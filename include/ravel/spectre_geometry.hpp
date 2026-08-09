// ravel/spectre_geometry.hpp
//
// EVERYTHING that is actually specific to the Spectre aperiodic
// monotile (Smith, Myers, Kaplan & Goodman-Strauss, "A Chiral Aperiodic
// Monotile", Combinatorial Theory 4(2) (2024) #13, arXiv:2305.17743 --
// bib key SmithMyersKaplanGoodmanStrauss2023Spectre, full text archived
// under refs/FullText/; see also its precursor "An Aperiodic Monotile",
// arXiv:2303.10798, the "hat" tile, which still needs reflections and is
// NOT what this file implements): its vertex coordinates, its
// substitution rule table, and its supertile assembly recipe --
// expressed as DATA fed into the generic machinery in
// substitution_lineage.hpp, which knows nothing about any of this.
//
// If a different substitution tiling is ever wanted (Penrose,
// Ammann-Beenker, ...), it needs a file shaped exactly like this one
// and nothing else changes: substitution_lineage.hpp's tree builder,
// ultrametric distance, adjacency/CA machinery are all already generic.

#pragma once

#include <cmath>
#include <map>
#include <string>
#include <vector>

#include "ravel/substitution_lineage.hpp"

namespace ravel::spectre {

using namespace ravel::substitution_lineage;

inline const double kS3 = std::sqrt(3.0);

inline const std::vector<Point>& spectre_points() {
    static const std::vector<Point> P = {
        {0,0}, {1,0}, {1.5,-kS3/2},
        {1.5+kS3/2, 0.5-kS3/2}, {1.5+kS3/2, 1.5-kS3/2},
        {2.5+kS3/2, 1.5-kS3/2}, {3+kS3/2, 1.5}, {3,2},
        {3-kS3/2,1.5}, {2.5-kS3/2,1.5+kS3/2}, {1.5-kS3/2,1.5+kS3/2},
        {0.5-kS3/2,1.5+kS3/2}, {-kS3/2,1.5}, {0,1}
    };
    return P;
}
inline const std::vector<Point>& chevron_points() {
    static const std::vector<Point> P = {
        {0,0}, {kS3/2,0.5}, {kS3/2,1.5}, {0,2}, {-kS3/2,1.5}, {0,1}
    };
    return P;
}

inline const std::vector<std::string>& tile_names() {
    static const std::vector<std::string> N =
        {"Gamma","Delta","Theta","Lambda","Xi","Pi","Sigma","Phi","Psi"};
    return N;
}

// For each label, the (up to 8) child labels, empty string = no child
// at that slot. Data only -- consumed by assemble_supertiles.
inline const std::map<std::string, std::vector<std::string>>& super_rules() {
    static const std::map<std::string, std::vector<std::string>> R = {
        {"Gamma",  {"Pi","Delta","","Theta","Sigma","Xi","Phi","Gamma"}},
        {"Delta",  {"Xi","Delta","Xi","Phi","Sigma","Pi","Phi","Gamma"}},
        {"Theta",  {"Psi","Delta","Pi","Phi","Sigma","Pi","Phi","Gamma"}},
        {"Lambda", {"Psi","Delta","Xi","Phi","Sigma","Pi","Phi","Gamma"}},
        {"Xi",     {"Psi","Delta","Pi","Phi","Sigma","Psi","Phi","Gamma"}},
        {"Pi",     {"Psi","Delta","Xi","Phi","Sigma","Psi","Phi","Gamma"}},
        {"Sigma",  {"Xi","Delta","Xi","Phi","Sigma","Pi","Lambda","Gamma"}},
        {"Phi",    {"Psi","Delta","Psi","Phi","Sigma","Pi","Phi","Gamma"}},
        {"Psi",    {"Psi","Delta","Psi","Phi","Sigma","Psi","Phi","Gamma"}},
    };
    return R;
}

// Spectre's supertile assembly recipe, as data for assemble_supertiles.
inline const std::vector<SupertileAssemblyRule>& assembly_rules() {
    static const std::vector<SupertileAssemblyRule> R = {
        {60,3,1}, {0,2,0}, {60,3,1}, {60,3,1}, {0,2,0}, {60,3,1}, {-120,3,3}
    };
    return R;
}
inline const Affine& mirror() {
    static const Affine M = {-1, 0, 0, 0, 1, 0};
    return M;
}
inline const std::array<QuadCorner,4>& next_quad_rule() {
    static const std::array<QuadCorner,4> Q = {{ {6,2}, {5,1}, {3,2}, {0,1} }};
    return Q;
}

inline std::array<Point,4> base_quad() {
    const auto& P = spectre_points();
    return {P[3], P[5], P[7], P[11]};
}

inline void build_spectre_base(ShapeSystem& sys) {
    for (const auto& l : tile_names()) {
        if (l == "Gamma") continue;
        ShapeNode* n = sys.make_node();
        n->label = l;
        sys.current[l] = n;
    }
    // Gamma is the "mystic" metatile: Gamma1 (identity) + Gamma2 (rotated/translated).
    ShapeNode* gamma1 = sys.make_node(); gamma1->label = "Gamma1";
    ShapeNode* gamma2 = sys.make_node(); gamma2->label = "Gamma2";
    ShapeNode* gamma = sys.make_node();
    gamma->children.push_back({gamma1, kIdentity});
    Affine g2t = affine_mul(affine_trans(spectre_points()[8][0], spectre_points()[8][1]), affine_rot(M_PI/6));
    gamma->children.push_back({gamma2, g2t});
    sys.current["Gamma"] = gamma;
}

inline std::array<Point,4> build_supertiles(ShapeSystem& sys, const std::array<Point,4>& quad) {
    return assemble_supertiles(sys, quad, mirror(), assembly_rules(), next_quad_rule(), super_rules());
}

// Tiles that run the unconditional-NOT gate in the reversible CA
// (spectre_ca.hpp's run_ca wrapper): Spectre's Gamma/Gamma1/Gamma2 are
// already the "special" tiles in this substitution system (the only
// ones that only ever appear as a locked mystic pair), so it's a
// natural, guaranteed place to plant it -- an implementation CHOICE
// for this tiling, not something the generic CA machinery assumes.
inline bool is_not_gate_label(const std::string& label) {
    return label == "Gamma1" || label == "Gamma2" || label == "Gamma";
}

struct LineageTile {
    std::string label;
    std::vector<Point> pts;    // spectre_points() transformed
    std::vector<Point> chev;   // chevron_points() transformed
    double cx{}, cy{};
    Point tip{};
    double theta{};            // degrees, [0,360)
    double dist_from_centroid{};
    std::vector<int> addr;     // root-first substitution address
};

// Generates the tiling: `iterations` levels of inflation, cropped to
// the `crop_count` tiles closest (by chevron centroid) to the overall
// centroid.
inline std::vector<LineageTile> generate_tiling_with_lineage(int iterations = 4, std::size_t crop_count = 500) {
    ShapeSystem sys;
    build_spectre_base(sys);
    std::array<Point,4> quad = base_quad();
    for (int i = 0; i < iterations; ++i) quad = build_supertiles(sys, quad);

    std::vector<LineageLeaf> collector;
    std::vector<int> path;
    draw_recursive(sys.current.at("Delta"), kIdentity, path, collector);

    std::vector<LineageTile> tiles;
    tiles.reserve(collector.size());
    for (auto& leaf : collector) {
        LineageTile t;
        t.label = leaf.label;
        t.addr = leaf.addr;
        for (auto& p : spectre_points()) t.pts.push_back(affine_apply(leaf.T, p));
        for (auto& p : chevron_points()) t.chev.push_back(affine_apply(leaf.T, p));
        double sx = 0, sy = 0;
        for (auto& p : t.chev) { sx += p[0]; sy += p[1]; }
        t.cx = sx / (double)t.chev.size();
        t.cy = sy / (double)t.chev.size();
        t.tip = t.chev[3];
        double th = std::fmod(std::atan2(leaf.T[3], leaf.T[0]) * 180.0 / M_PI, 360.0);
        if (th < 0) th += 360.0;
        t.theta = th;
        tiles.push_back(std::move(t));
    }

    double gcx = 0, gcy = 0;
    for (auto& t : tiles) { gcx += t.cx; gcy += t.cy; }
    gcx /= (double)tiles.size(); gcy /= (double)tiles.size();
    for (auto& t : tiles) t.dist_from_centroid = std::hypot(t.cx - gcx, t.cy - gcy);

    std::sort(tiles.begin(), tiles.end(),
              [](const LineageTile& a, const LineageTile& b) { return a.dist_from_centroid < b.dist_from_centroid; });
    if (tiles.size() > crop_count) tiles.resize(crop_count);
    return tiles;
}

}  // namespace ravel::spectre
