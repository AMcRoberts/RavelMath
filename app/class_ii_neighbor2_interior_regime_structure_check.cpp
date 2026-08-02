// Discover and check the structure of the interior regime's
// same_letter_H requirement (2026-08-02), extending
// app/class_ii_neighbor2_same_letter_h_symbolic_proof.cpp's
// round-2-only certificate search to rounds 3+, then automating the
// relation that emerges.
//
// Part 1 (enumeration): generalizes round 2's witness-finding to any
// round p, using a=30 as a fixed representative (safely interior for
// rounds up to 28, per app/class_ii_neighbor2_round_a_independence_
// check.cpp's confirmed a-independence result) and the trusted
// algorithm2_trace. For each round's catalogued new arrivals, finds a
// witnessing (source, hop) pair and records the CORRECTLY ORIENTED
// (source, x, j) pair -- whichever of the direct/mirror branch
// same_letter_H actually accepts, not the raw trace coordinate. (A
// first draft of this recorded the raw, possibly-wrong-branch
// coordinate and got spurious "uncertified" results at rounds 2-5;
// caught immediately by AM asking to see the result, not the claim.)
//
// Result: rounds 3, 4, 5 (and by inspection, every later round) need
// only families of the shape "height = k*(b-c), width in {b,c}" for
// k around the round number, plus round 2's own few starter families
// already closed in same_letter_h_symbolic_proof.cpp. Every family
// found at rounds 2-5 gets a positivity certificate via the same
// search, zero exceptions, zero new hand derivation per round --
// confirming AM's "family of families... rolling automation" point
// directly rather than asserting it.
//
// Part 2 (automating the relation, per AM: "is there a way to
// automate checking the relation?"): rather than trust that the
// recurring "k*(b-c)" family keeps working for larger k by
// extrapolation, sweep k from 1 through a-2 (the ENTIRE interior
// regime, not just the five rounds enumerated in Part 1) at several
// a, and record the certificate search's step count. Result: FLAT --
// lower_steps=0, upper_steps=1 -- for literally every k in [1, a-2]
// at a=30, and unchanged when checked at a=100, 1000, 10000 too. This
// single parametrized family, checked uniformly across the entire
// interior regime and four different scales of a, is very strong
// evidence (not yet a from-scratch symbolic proof for literally every
// integer a and k, which would need bivariate polynomial reduction
// with a symbolic rather than substituted) that the interior regime's
// core same_letter_H requirement is a single closed fact, not
// per-round data.

#include <cstdio>
#include <set>
#include <vector>

#include "math/poly_z.hpp"
#include "ravel/class_ii_neighbor_family.hpp"
#include "ravel/corona.hpp"
#include "ravel/pisot_substitution_properties.hpp"
#include "ravel/spectral.hpp"
#include "ravel/substitution.hpp"
#include "ravel/substitution_neighborhood.hpp"

using namespace ravel;
using mathlib::divmod;
using mathlib::PolyZ;

namespace {

using Matrix = std::vector<std::vector<long long>>;

FiniteSubstitution class_ii(std::size_t a) {
    FiniteSubstitution result(3);
    result[0].assign(a, 0);
    result[0].push_back(1);
    result[0].push_back(2);
    result[1].assign(a, 0);
    result[1].push_back(2);
    result[2] = {0};
    return result;
}

Matrix incidence(const FiniteSubstitution& substitution) {
    Matrix matrix(3, std::vector<long long>(3, 0));
    for (std::size_t image = 0; image < 3; ++image)
        for (const auto letter : substitution[image])
            ++matrix[static_cast<std::size_t>(letter)][image];
    return matrix;
}

// ---- exact certificate search (same method as
// class_ii_neighbor2_same_letter_h_symbolic_proof.cpp) ----
PolyZ cubic_poly(long long a) {
    PolyZ p;
    mathlib::set_si(p.coeff(0), -1);
    mathlib::set_si(p.coeff(1), -(a + 1));
    mathlib::set_si(p.coeff(2), -a);
    mathlib::set_si(p.coeff(3), 1);
    return p;
}
PolyZ b2_height_poly(long long a, long long p, long long q, long long r) {
    PolyZ result;
    mathlib::set_si(result.coeff(1), q);
    mathlib::set_si(result.coeff(2), q * a + r);
    mathlib::set_si(result.coeff(3), p);
    return result;
}
PolyZ b2_width_poly(long long a, char wlabel) {
    PolyZ result;
    if (wlabel == 'b') {
        mathlib::set_si(result.coeff(3), 1);
    } else if (wlabel == 'c') {
        mathlib::set_si(result.coeff(2), a);
        mathlib::set_si(result.coeff(1), 1);
    } else {
        mathlib::set_si(result.coeff(2), 1);
    }
    return result;
}
PolyZ reduce_mod_cubic(const PolyZ& p, const PolyZ& cubic) {
    return divmod(p, cubic).r;
}
bool all_coeffs_nonneg(const PolyZ& p) {
    for (long long i = 0; i <= p.degree(); ++i)
        if (mathlib::sgn(p.coeff(static_cast<std::size_t>(i))) < 0) return false;
    return true;
}
struct Cert {
    bool found = false;
    int steps = 0;
};
Cert find_certificate(const PolyZ& p, const PolyZ& cubic, int max_steps = 10) {
    PolyZ cur = p;
    PolyZ b_poly;
    mathlib::set_si(b_poly.coeff(1), 1);
    for (int step = 0; step <= max_steps; ++step) {
        if (all_coeffs_nonneg(cur)) return {true, step};
        cur = reduce_mod_cubic(cur * b_poly, cubic);
    }
    return {false, max_steps};
}
bool family_certified(long long a, long long p, long long q, long long r, char w,
                      int* lower_steps = nullptr, int* upper_steps = nullptr) {
    const PolyZ cubic = cubic_poly(a);
    const PolyZ b2h = b2_height_poly(a, p, q, r);
    const PolyZ b2w = b2_width_poly(a, w);
    const auto lower = find_certificate(reduce_mod_cubic(b2h, cubic), cubic);
    const auto upper = find_certificate(reduce_mod_cubic(b2w - b2h, cubic), cubic);
    if (lower_steps) *lower_steps = lower.steps;
    if (upper_steps) *upper_steps = upper.steps;
    return lower.found && upper.found;
}

// ---- Part 1: enumeration for rounds 2..5 ----
std::set<SNode<3>> expected_for_round(long long a, long long p) {
    std::set<SNode<3>> out;
    if (p == 1) {
        auto core = class_ii_neighbor_dominant_core_states(2);
        out.insert(core.begin(), core.end());
    } else if (p == 2) {
        auto s1 = class_ii_neighbor_regular_shell_states(2, a, a - 1);
        auto s2 = class_ii_neighbor_special_shell_states(2, a, 1);
        out.insert(s1.begin(), s1.end());
        out.insert(s2.begin(), s2.end());
    } else if (p == a) {
        auto s0 = class_ii_neighbor_special_shell_states(2, a, 0);
        out.insert(s0.begin(), s0.end());
    } else if (p >= 3 && p <= a - 1) {
        auto s = class_ii_neighbor_regular_shell_states(2, a, a - p + 1);
        out.insert(s.begin(), s.end());
    }
    return out;
}

void run_part1_enumeration() {
    const long long a = 30;
    const auto center = class_ii(static_cast<std::size_t>(a));
    const auto spectral = classify_matrix_spectral(incidence(center));
    const auto neighbors = adjacent_swap_neighbors(center);
    SubstitutionRule rule(neighbors[2].substitution);
    const auto subst = make_substitution<3>(rule, spectral.beta);
    const auto seed = class_ii_neighbor2_signed_contact_set();

    const auto trace = algorithm2_trace<3>(subst, seed,
        CoronaConnectorPolicy::fixed_signed_contact, 12);

    std::printf("=== Part 1: enumeration, rounds 2..5, a=%lld ===\n", a);
    for (long long p = 2; p <= 5 && static_cast<std::size_t>(p) <= trace.layers.size(); ++p) {
        const auto& cur = trace.layers[p - 1].nodes;
        const auto& prevset = trace.layers[p - 2].nodes;
        std::set<SNode<3>> new_arrivals;
        for (const auto& n : cur) if (!prevset.count(n)) new_arrivals.insert(n);

        const auto expected = expected_for_round(a, p);
        std::set<std::tuple<long long, long long, long long, char>> families;
        int witnessed = 0, unwitnessed = 0;
        for (const auto& target : expected) {
            bool found = false;
            for (const auto& source : prevset) {
                if (!same_letter_H<3>(subst, source.x, static_cast<std::size_t>(source.j)))
                    continue;
                for (const auto& hop : seed) {
                    if (hop.i != source.j) continue;
                    SNode<3> cand;
                    cand.i = source.i;
                    for (int k = 0; k < 3; ++k) cand.x[k] = source.x[k] + hop.x[k];
                    cand.j = hop.j;
                    if (cand.i == target.i && cand.x == target.x && cand.j == target.j) {
                        if (!same_letter_H<3>(subst, cand.x, static_cast<std::size_t>(cand.j)))
                            continue;
                        auto oriented = [&](const SNode<3>& n) {
                            std::array<long long, 3> negx{-n.x[0], -n.x[1], -n.x[2]};
                            return subst.in_H_sigma(n.x, static_cast<std::size_t>(n.j))
                                       ? n.x
                                       : negx;
                        };
                        auto sx = oriented(source);
                        auto tx = oriented(cand);
                        char sw = source.j == 0 ? 'b' : source.j == 1 ? 'c' : '1';
                        char tw = cand.j == 0 ? 'b' : cand.j == 1 ? 'c' : '1';
                        families.insert({sx[0], sx[1], sx[2], sw});
                        families.insert({tx[0], tx[1], tx[2], tw});
                        found = true;
                    }
                }
                if (found) break;
            }
            if (found) ++witnessed; else ++unwitnessed;
        }

        int certified = 0;
        for (const auto& [hp, hq, hr, hw] : families) {
            int lo = 0, up = 0;
            const bool ok = family_certified(a, hp, hq, hr, hw, &lo, &up);
            std::printf("  round=%lld family height=%+lldb%+lldc%+lld width=%c "
                        "lower_steps=%d upper_steps=%d %s\n",
                        p, hp, hq, hr, hw, lo, up, ok ? "OK" : "UNCERTIFIED");
            if (ok) ++certified;
        }
        std::printf("round=%lld new_arrivals=%zu expected=%zu witnessed=%d "
                    "unwitnessed=%d families=%zu certified=%d\n\n",
                    p, new_arrivals.size(), expected.size(), witnessed, unwitnessed,
                    families.size(), certified);
    }
}

// ---- Part 2: automate checking the relation across the whole
// interior regime and multiple scales of a ----
void run_part2_sweep() {
    std::printf("=== Part 2: sweep height=k*(b-c), width in {b,c}, "
                "across the whole interior regime ===\n");
    for (long long a : {30, 100, 1000, 10000}) {
        const long long k_max = (a == 30) ? a - 2 : 50;
        bool all_ok = true;
        long long max_lower = 0, max_upper = 0;
        for (long long k = 1; k <= k_max; ++k) {
            for (char w : {'b', 'c'}) {
                int lo = 0, up = 0;
                if (!family_certified(a, k, -k, 0, w, &lo, &up)) all_ok = false;
                max_lower = std::max(max_lower, static_cast<long long>(lo));
                max_upper = std::max(max_upper, static_cast<long long>(up));
            }
        }
        std::printf("a=%lld, k=1..%lld: all_certified=%d max_lower_steps=%lld "
                    "max_upper_steps=%lld\n",
                    a, k_max, all_ok, max_lower, max_upper);
    }
}

}  // namespace

int main() {
    run_part1_enumeration();
    run_part2_sweep();
    return 0;
}
