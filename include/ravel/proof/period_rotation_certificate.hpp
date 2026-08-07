// ravel/proof/period_rotation_certificate.hpp
//
// C++ certificate for Finding 35's algebraic core, wired through the
// reflection pipeline exactly like Finding 42's Barge-Diamond
// certificate: no Lean text is authored per substitution here, only
// a reflection trace node recorded once every fact below is verified
// EXACTLY.
//
// SETUP. Build the substitution's full-alphabet incidence matrix M
// (M[i][j] = # occurrences of letter i in sigma(j)'s image). Every
// nonzero entry M[i][j] is literally a single forced step of the
// substitution (one letter of sigma(j)'s image is i), so the directed
// graph "j -> i whenever M[i][j] != 0" has every edge a genuine unit
// step. g = gcd of every jump size in the junction graph (Findings
// 25/26/35's own construction). Every jump size is, by definition,
// the LENGTH of a chain of such unit steps between two junction
// letters, so every jump size -- and hence every cycle length in the
// full-alphabet graph, being a sum of jump sizes -- is divisible by
// g.
//
// COLORING. Fix a root letter r = 0 and let dist0[v] = the length of
// ANY forward path r -> v in the full-alphabet graph (computed here
// by BFS, which gives the length of ONE such path; any other forward
// path r -> v has a length that differs from dist0[v] by a multiple
// of g -- because appending a shared forward path v -> r, guaranteed
// to exist by the substitution's irreducibility, turns the length
// difference into the difference of two cycle lengths through r, and
// every cycle length is divisible by g, established above). So
// `coloring[v] := dist0[v]` is a well-defined integer level modulo g,
// and for every edge j -> i (M[i][j] != 0), coloring[i] and
// coloring[j] + 1 both equal a length of a path r -> i (one directly,
// one via j), hence agree modulo g: coloring[i] = coloring[j] + 1 +
// k*g for an EXPLICIT, exactly computable integer k = (coloring[i] -
// coloring[j] - 1) / g.
//
// This is exactly the hypothesis `period_coloring_rotates_eigenvalue`
// (lean/period_rotation_forces_equal_modulus.lean) needs. The
// certificate below verifies g >= 2 (the interesting case -- g = 1
// gives no obstruction, matching genuine Pisot substitutions per
// Finding 36) and that every edge's k divides exactly (a sanity check
// on the mathematical argument above, not merely trusting it), then
// records a `PeriodRotationCertificate` reflection node. The renderer
// (reflective_lean_renderer.hpp) discharges the coloring hypothesis
// itself, per instance, via a finite case split + `omega` -- fully
// mechanical, no hand-authored Lean at the instance level.

#pragma once

#include <array>
#include <cstddef>
#include <deque>
#include <numeric>
#include <string>
#include <vector>

#include "math/proof_reflection.hpp"
#include "ravel/proof/coincidence_closure.hpp"  // build_junction_graph, JunctionEdge

namespace ravel::proof {

struct PeriodRotationCertificate {
    bool applicable{};   // g >= 2 (g == 1 has nothing to certify)
    long long g{};
    std::string conclusion;
};

template <std::size_t d>
inline PeriodRotationCertificate certify_period_rotation(
    const std::array<std::vector<long long>, d>& images) {
    PeriodRotationCertificate out;

    std::vector<std::vector<long long>> M(d, std::vector<long long>(d, 0));
    for (std::size_t j = 0; j < d; ++j)
        for (auto c : images[j]) M[static_cast<std::size_t>(c)][j] += 1;

    auto edges = build_junction_graph<d>(images);
    long long g = 0;
    for (auto& e : edges) g = std::gcd(g, e.jump_size);
    out.g = g;

    if (g < 2) {
        out.applicable = false;
        out.conclusion = "g=1: no period obstruction (matches genuine Pisot substitutions, "
                          "e.g. Finding 36).";
        return out;
    }
    out.applicable = true;

    // BFS from root 0 over the full-alphabet graph: edge j -> i iff M[i][j] != 0.
    std::vector<long long> dist(d, -1);
    dist[0] = 0;
    std::deque<std::size_t> q;
    q.push_back(0);
    while (!q.empty()) {
        std::size_t j = q.front();
        q.pop_front();
        for (std::size_t i = 0; i < d; ++i) {
            if (M[i][j] != 0 && dist[i] == -1) {
                dist[i] = dist[j] + 1;
                q.push_back(i);
            }
        }
    }
    for (std::size_t v = 0; v < d; ++v) {
        if (dist[v] == -1) {
            out.applicable = false;
            out.conclusion = "root letter 0 does not reach letter " + std::to_string(v) +
                              " -- not irreducible, coloring not constructed.";
            return out;
        }
    }

    // Sanity-check the mathematical argument for THIS matrix, exactly:
    // every edge's k = (dist[i] - dist[j] - 1) / g must divide evenly.
    for (std::size_t j = 0; j < d; ++j) {
        for (std::size_t i = 0; i < d; ++i) {
            if (M[i][j] == 0) continue;
            long long diff = dist[i] - dist[j] - 1;
            if (diff % g != 0) {
                out.applicable = false;
                out.conclusion = "coloring consistency check FAILED at edge (" + std::to_string(i) +
                                  "," + std::to_string(j) + ") -- not recording (should be "
                                  "impossible for an irreducible graph; see header comment).";
                return out;
            }
        }
    }

    std::vector<long long> M_flat(d * d, 0);
    std::vector<long long> k_flat(d * d, 0);
    for (std::size_t i = 0; i < d; ++i) {
        for (std::size_t j = 0; j < d; ++j) {
            M_flat[i * d + j] = M[i][j];
            if (M[i][j] != 0) k_flat[i * d + j] = (dist[i] - dist[j] - 1) / g;
        }
    }

    mathlib::reflection::PeriodRotationCertificate node;
    node.n = static_cast<long long>(d);
    node.p = g;
    node.matrix_flat = M_flat;
    node.coloring = dist;
    node.k_flat = k_flat;
    node.description = "junction jump-gcd g=" + std::to_string(g) + " on " + std::to_string(d) + " letters";
    mathlib::reflection::record(mathlib::reflection::NodeKind::LemmaApplication, node);

    out.conclusion = "g=" + std::to_string(g) + " >= 2: coloring verified exactly, recorded for "
                      "mechanical Lean rendering of period_coloring_rotates_eigenvalue.";
    return out;
}

}  // namespace ravel::proof
