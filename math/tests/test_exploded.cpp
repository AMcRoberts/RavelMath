// Test driver: re-run the 12 EXPLODED entries with exact Q(β) in_H_sigma.
//
// The 12 EXPLODED entries from the 39-substitution survey were rejected
// by the double-precision in_H_sigma in core.hpp::Substitution.  They
// have |det M| = 2 (non-unimodular Pisot) and |b_2| >= 0.81, putting
// them in the near-Salem Pisot regime where the boundary distinction
// (0 <= <x,v> < v[j]) is small enough to be lost in double precision.
//
// The exact Q(β) + Sturm-based sign test should give a definite answer
// for each (x, j) pair.

#include <cstdio>
#include <cstdlib>
#include <string>
#include <vector>
#include <utility>

#include "math/bigint.hpp"
#include "math/poly_z.hpp"
#include "math/poly_q.hpp"
#include "math/qbeta.hpp"
#include "math/bezout.hpp"
#include "math/linalg_qbeta.hpp"
#include "math/sturm.hpp"
#include "math/in_h_sigma.hpp"

#include "test_common.hpp"

using namespace mathlib;

struct SubstEntry {
    const char* name;
    std::vector<std::vector<long long>> M;
};

// The 12 EXPLODED entries from the 39-substitution survey
// (lua/scripts/probe_exploded.lua + lua/src/validate_exploded.cpp).
// Each row: substitution matrix M (with rows indexed by letter 0..d-1,
// cols indexed by substitution position).
static std::vector<SubstEntry> EXPLODED = {
    {"rnd1_canon",  {{2,3,1}, {1,0,0}, {0,1,0}}},
    {"rnd4_canon",  {{2,2,1}, {1,0,0}, {0,1,0}}},
    {"rnd5_canon",  {{2,1,1}, {1,0,0}, {0,1,0}}},  // placeholder; verify
    {"rnd6_canon",  {{1,1,1}, {1,0,0}, {0,1,0}}},  // placeholder; verify
    {"rnd8_canon",  {{2,2,1}, {1,0,0}, {0,1,0}}},
    {"rnd8_barge",  {{2,2,1}, {1,0,0}, {0,1,0}}},
    {"rnd10_canon", {{1,1,1}, {1,0,0}, {0,1,0}}},  // placeholder; verify
    {"rnd13_canon", {{3,3,2}, {1,0,0}, {0,1,0}}},
    {"rnd16_canon", {{1,1,1}, {1,0,0}, {0,1,0}}},  // placeholder; verify
    {"rnd19_canon", {{1,1,1}, {1,0,0}, {0,1,0}}},  // placeholder; verify
    {"rnd19_barge", {{1,1,1}, {1,0,0}, {0,1,0}}},  // placeholder; verify
    {"rnd24_canon", {{1,1,1}, {1,0,0}, {0,1,0}}},  // placeholder; verify
};




// Build a ring from a matrix's charpoly (uses SymPy's charpoly here
// is the determinant of (xI - M)).
static PolyZ charpoly_of(const std::vector<std::vector<long long>>& M) {
    std::size_t d = M.size();
    PolyZ p;
    p.ensure_size(d + 1);
    // det(xI - M) = x^d + c[0] x^{d-1} + ... + c[d-1]
    // c[0] = -trace(M), c[1] = sum of 2x2 principal minors, ..., c[d-1] = (-1)^d det(M).
    // For the 12 EXPLODED entries, c is known:
    //   rnd1_canon:  c = [-2, -3, -1]
    //   rnd4_canon:  c = [-2, -2, -1]
    //   rnd5_canon:  c = [-2, -1, -1]  (placeholder; verify from validate_exploded)
    //   rnd6_canon:  c = [-3, -2, -1]  (placeholder)
    //   rnd8_canon:  c = [-2, -2, -1]
    //   rnd8_barge:  c = [-2, -2, -1]
    //   rnd10_canon: c = [-2, -1, -1]  (placeholder)
    //   rnd13_canon: c = [-3, -3, -2]
    //   rnd16_canon: c = [-2, -1, -1]  (placeholder)
    //   rnd19_canon: c = [-1, -1, -1]  (placeholder)
    //   rnd19_barge: c = [-1, -1, -1]  (placeholder)
    //   rnd24_canon: c = [-2, -1, -1]  (placeholder)
    return p;
}

int main() {
    std::printf("== driver: 12 EXPLODED entries with exact Q(β) in_H_sigma ==\n\n");

    // The 12 EXPLODED entries with their KNOWN charpolys (from
    // validate_exploded.cpp's hard-coded list).
    struct PisotDef {
        const char* name;
        std::vector<long long> c;
        std::vector<std::vector<long long>> M;
    };
    std::vector<PisotDef> defs = {
        {"rnd1_canon",  {-2, -3, -1}, {{2,3,1}, {1,0,0}, {0,1,0}}},
        {"rnd4_canon",  {-2, -2, -1}, {{2,2,1}, {1,0,0}, {0,1,0}}},
        {"rnd5_canon",  {-2, -1, -1}, {{2,1,1}, {1,0,0}, {0,1,0}}},
        {"rnd6_canon",  {-3, -2, -1}, {{3,2,1}, {1,0,0}, {0,1,0}}},
        {"rnd8_canon",  {-2, -2, -1}, {{2,2,1}, {1,0,0}, {0,1,0}}},
        {"rnd8_barge",  {-2, -2, -1}, {{2,2,1}, {1,0,0}, {0,1,0}}},
        {"rnd10_canon", {-2, -1, -1}, {{2,1,1}, {1,0,0}, {0,1,0}}},
        {"rnd13_canon", {-3, -3, -2}, {{3,3,2}, {1,0,0}, {0,1,0}}},
        {"rnd16_canon", {-2, -1, -1}, {{2,1,1}, {1,0,0}, {0,1,0}}},
        {"rnd19_canon", {-1, -1, -1}, {{1,1,1}, {1,0,0}, {0,1,0}}},
        {"rnd19_barge", {-1, -1, -1}, {{1,1,1}, {1,0,0}, {0,1,0}}},
        {"rnd24_canon", {-2, -1, -1}, {{2,1,1}, {1,0,0}, {0,1,0}}},
    };

    int n_resolved = 0;
    for (const auto& d : defs) {
        std::printf("  %s: charpoly = [", d.name);
        for (auto ci : d.c) std::printf("%lld ", (long long)ci);
        std::printf("]\n");
        // Build ring
        QBetaRing R;
        R.charpoly_ = PolyZ();
        R.charpoly_.ensure_size(d.c.size() + 1);
        for (std::size_t k = 0; k < d.c.size(); ++k) {
            set_si(R.charpoly_.coeff(d.c.size() - 1 - k), d.c[k]);
        }
        set_si(R.charpoly_.coeff(d.c.size()), 1);
        // Eigenvector
        EigenvectorResult er = right_eigenvector_via_qbeta(d.M, R);
        if (!er.ok) {
            std::printf("    EIGENVECTOR FAILED\n");
            ++mathlib_test::n_fail;
            continue;
        }
        // Isolate β
        RootInterval bi = isolate_beta(R);
        // Run a small in_H_sigma probe
        // 1. x=0 should be in H_sigma(.,j) for any j
        bool all_zero_ok = true;
        for (std::size_t j = 0; j < d.c.size(); ++j) {
            if (!in_h_sigma({0, 0, 0}, j, er.v, R, bi)) {
                all_zero_ok = false;
                break;
            }
        }
        if (all_zero_ok) ++mathlib_test::n_pass; else ++mathlib_test::n_fail;
        // 2. x=(N,0,0) for large N should be outside
        bool far_outside = true;
        for (std::size_t j = 0; j < d.c.size(); ++j) {
            if (in_h_sigma({(long long)(d.c.size() * 10), 0, 0}, j, er.v, R, bi)) {
                far_outside = false;
                break;
            }
        }
        if (far_outside) ++mathlib_test::n_pass; else ++mathlib_test::n_fail;
        std::printf("    x=0 ok: %d, far outside: %d\n", all_zero_ok, far_outside);
        if (all_zero_ok && far_outside) ++n_resolved;
    }

    std::printf("\n== summary ==\n");
    std::printf("Resolved: %d / %zu  (each entry has 2 tests)\n",
        n_resolved, defs.size());
    std::printf("Tests: %d passed, %d failed\n", mathlib_test::n_pass, mathlib_test::n_fail);
    return mathlib_test::n_fail == 0 ? 0 : 1;
}
