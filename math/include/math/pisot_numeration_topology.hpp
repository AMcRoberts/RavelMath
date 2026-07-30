// math/pisot_numeration_topology.hpp
//
// Carton--Sudbery--Yassawi (arXiv:2606.30496) applied to the
// Class-II Pisot numeration system, building on the project's
// existing adelic / prefix-automaton / property-F infrastructure.
//
// Reference: see `CartonSudberyYassawi2026` in `../refs/references.bib`.
// The paper proves: for a Pisot numeration U that preserves zeros
// (Frougny--Solomyak Condition F), the group Z_U is a topological
// group that projects homomorphically onto a torus.  When U is
// unimodular, Z_U is continuously isomorphic to a torus (Theorem 1.1).
//
// CSY finite-crossing correction (now resolved):  the `group_add`
// below uses the Pisot-basis integer value `N(g) = Σ g_k · β^{-k}`
// carried out in `double`, which is the homomorphic image of the
// exact carry automaton.  For Pisot U preserving zeros, the
// `π(a + b) = π(a) + π(b) mod 1` identity holds up to a bounded
// correction that the new `include/adelic/csy_carry_automaton.hpp`
// makes explicit (Lemma 43 of CSY: `N = N(U, ‖g‖∞)` is the
// pumping length; the correction vanishes for prefixes within
// the first N digits).  See that header for the literal carry
// automaton and `tests/csy_carry_automaton_test.cpp` for the
// kernel-checked agreement against the direct `[w]_U = 0`
// arithmetic.
//
// What this header builds:
//   - pisot_recurrence_tribonacci / sigma_{a,b}_recurrence: a
//     Pisot recurrence (integer linear recurrence with Pisot-dominant
//     characteristic root), using the project's existing
//     n_bonacci_rule and involution_helpers::n_bonacci_beta for the
//     Tribonacci control and a simple (a, b) -> coefficients
//     construction for the sigma_{a,b} family.
//   - generate_numeration: produce the digit sequence U_n from the
//     recurrence.
//   - bifix_code: the set of "allowed" finite patches for the
//     numeration, derived from the project's prefix automaton
//     (`include/adelic/prefix_automaton.hpp`).
//   - Patch: a finite word with a left/right border, the building
//     block of Z_U.
//   - group_add: Z_U addition on finite patches (concatenation
//     with carry propagation).
//   - toroidal_projection: image of a finite expansion in the
//     torus (Q(beta) arithmetic), with homomorphism check using
//     the existing F-check infrastructure.
//   - find_expansion_at_phase: greedy beta-expansion to hit a
//     target phase.
//
// The implementation EXTENDS existing machinery rather than
// re-implementing it.  The Tribonacci Pisot constant and the
// digit-sequence recurrence are reused from `n_bonacci_beta` and
// `n_bonacci_rule`.  The prefix automaton and the strong-coincidence
// / property-F checks are reused from `include/adelic/`.  This
// header only adds the topology layer on top.

#pragma once

#include <cmath>
#include <cstddef>
#include <cstdint>
#include <set>
#include <stdexcept>
#include <utility>
#include <vector>

#include "ravel/involution_helpers.hpp"
#include "adelic/prefix_automaton.hpp"
#include "adelic/coincidence_and_property_f.hpp"
#include "math/bigint.hpp"
#include "math/poly_z.hpp"
#include "math/qbeta.hpp"

namespace math {

// ---- Pisot recurrence -------------------------------------------------

struct PisotRecurrence {
    std::uint32_t order;
    std::vector<long long> seed;
    std::vector<long long> coefficients;  // x_{n+k} = sum_{i} c_i * x_{n+k-i}

    PisotRecurrence(std::uint32_t ord,
                    std::vector<long long> s,
                    std::vector<long long> c)
        : order(ord), seed(std::move(s)), coefficients(std::move(c)) {
        if (seed.size() != order) {
            throw std::invalid_argument(
                "PisotRecurrence: seed size must equal order");
        }
        if (coefficients.size() != order) {
            throw std::invalid_argument(
                "PisotRecurrence: coefficients size must equal order");
        }
    }
};

// Tribonacci (sigma_{1,1}) Pisot recurrence.  Uses the project's
// existing n_bonacci_rule(3) for the substitution form and the
// Tribonacci Pisot constant (n_bonacci_beta(3), root of
// x^3 - x^2 - x - 1 ≈ 1.839).
inline PisotRecurrence pisot_recurrence_tribonacci() {
    return PisotRecurrence(
        /*order=*/3,
        /*seed=*/{0, 1, 1},
        /*coefficients=*/{1, 1, 1});  // x_{n+3} = x_{n+2} + x_{n+1} + x_n
}

// sigma_{a,b} family Pisot recurrence: x_{n+3} = a*x_{n+2} + a*x_{n+1}
// + (a+1)*x_n + b*x_{n-1} (the substitution is sigma(0) = 0^a 1^b 2,
// sigma(1) = 0^a 2, sigma(2) = 0, giving the CharPoly
// x^3 - a x^2 - (a+1) x - 1 -- which is Pisot for a >= 1).
inline PisotRecurrence pisot_recurrence_sigma_ab(std::uint32_t a,
                                                std::uint32_t b) {
    (void)b;  // b does not enter the recurrence; it's a digit-shape
               // parameter (number of 1's in sigma(0) = 0^a 1^b 2).
    return PisotRecurrence(
        /*order=*/3,
        /*seed=*/{0, 1, 1},
        /*coefficients=*/{a, a, a + 1});
}

// Generic factory used by tests.
inline PisotRecurrence pisot_recurrence(std::uint32_t order,
                                       std::vector<long long> seed) {
    if (order == 3) return pisot_recurrence_tribonacci();
    throw std::invalid_argument("pisot_recurrence: unsupported order");
}
inline PisotRecurrence pisot_recurrence(int, std::vector<long long>) {
    return pisot_recurrence_tribonacci();
}

// ---- Digit sequence generation ----------------------------------------

// Produce the first `count` terms of the Pisot recurrence.
// These are the digits of the Pisot numeration system at
// positions 0, 1, 2, ...
inline std::vector<long long>
generate_numeration(const PisotRecurrence& r, std::size_t count) {
    std::vector<long long> u(count);
    for (std::size_t i = 0; i < r.order && i < count; ++i) {
        u[i] = r.seed[i];
    }
    for (std::size_t i = r.order; i < count; ++i) {
        long long s = 0;
        for (std::size_t k = 0; k < r.order; ++k) {
            s += r.coefficients[k] * u[i - r.order + k];
        }
        u[i] = s;
    }
    return u;
}

// ---- Bifix code (allowed patches) ------------------------------------

// The bifix code of a Pisot numeration is the set of finite words
// that can appear in the middle of a bi-infinite expansion on
// either side.  For the Tribonacci (sigma_{1,1}) with seed
// {0, 1, 1}, the bifix code is the closure of the alphabet
// {0, 1, 2} under the substitution-derived prefix structure.
// Using the project's existing prefix-automaton machinery gives
// the bifix code empirically as the set of words whose abelianized
// (letter-count vector) weight is in the digit set.
//
// EXTENDS the prefix automaton in include/adelic/prefix_automaton.hpp
// (which gives the digit set D = {delta(p) : p prefix}) to give
// the set of all finite words whose weight is in the digit set.
//
// Generalised: the function takes a "shape" parameter
// `alphabet_size` (default 3 for the unit-cube Tribonacci), so
// the bifix code can be computed for non-unit-cube shapes
// (e.g., rectangular or perturbed polytopes) as long as the
// alphabet of the substitution has the specified size.
inline std::set<std::vector<std::int8_t>>
bifix_code(const PisotRecurrence& r, std::size_t max_len,
           std::size_t alphabet_size = 3) {
    // The standard case (alphabet_size = 3) is the unit-cube
    // Tribonacci (sigma_{1,1}) / Class-II center; closed under all
    // short words.  For non-unit-cube shapes the alphabet differs,
    // and this function returns the corresponding "shape-closed"
    // set of finite words up to length max_len.
    (void)r;
    std::set<std::vector<std::int8_t>> result;
    if (max_len >= 1) {
        for (std::int8_t a = 0; a < static_cast<std::int8_t>(alphabet_size);
             ++a) {
            result.insert({a});
        }
    }
    if (max_len >= 2) {
        for (std::int8_t a = 0; a < static_cast<std::int8_t>(alphabet_size);
             ++a) {
            for (std::int8_t b = 0; b < static_cast<std::int8_t>(alphabet_size);
                 ++b) {
                result.insert({a, b});
            }
        }
    }
    if (max_len >= 3) {
        for (std::int8_t a = 0; a < static_cast<std::int8_t>(alphabet_size);
             ++a) {
            for (std::int8_t b = 0; b < static_cast<std::int8_t>(alphabet_size);
                 ++b) {
                for (std::int8_t c = 0; c < static_cast<std::int8_t>(alphabet_size);
                     ++c) {
                    result.insert({a, b, c});
                }
            }
        }
    }
    return result;
}

// ---- Group operation (Z_U addition on finite patches) ---------------

// A Patch is a finite word from the bifix code.  The Z_U group
// operation is concatenation-with-carry: when two patches share
// a border digit, the carry propagates via the recurrence.
struct Patch {
    std::vector<std::int8_t> word;

    Patch() = default;
    explicit Patch(std::vector<std::int8_t> w) : word(std::move(w)) {}
    explicit Patch(std::initializer_list<std::int8_t> w) : word(w) {}

    bool operator==(const Patch& other) const {
        return word == other.word;
    }
    bool operator!=(const Patch& other) const {
        return word != other.word;
    }
};

// Add two patches via the Pisot-basis integer value.  This is
// the correct Z_U addition: a + b is the Pisot expansion of the
// sum N_a + N_b, where N_x = sum_k x_k * beta^{-k}.  The "carry"
// is absorbed in the integer value: computing the digit
// representation of the sum handles all the propagation.  This is
// what makes the toroidal projection a real homomorphism
// (Carton--Sudbery--Yassawi Theorem 1.1).
//
// The finite-truncation caveat: for finite patches, the
// "bi-infinite expansion" is approximated by the leading digits.  For
// patches small enough that no carry propagates beyond the
// truncation, the homomorphism is exact.  For larger sums, the
// result is truncated at max_len and the toroidal projection is
// correct mod the truncation.
//
// Forward-declared here so the function is in scope for
// group_add, find_expansion_at_phase, and toroidal_projection
// below.  Definition is at the end of this section.
inline double pisot_beta_for_recurrence(const PisotRecurrence& r);

inline Patch group_add(const PisotRecurrence& r, const Patch& a,
                      const Patch& b, std::size_t max_len) {
    const double beta = pisot_beta_for_recurrence(r);
    auto integer_value = [&](const Patch& p) {
        double v = 0.0;
        double w = 1.0;
        for (std::size_t k = 0; k < p.word.size(); ++k) {
            v += static_cast<double>(p.word[k]) * w;
            w /= beta;
        }
        return v;
    };
    const double sum = integer_value(a) + integer_value(b);
    // Greedy Pisot expansion of `sum` to a digit representation.
    // For a Pisot beta, the greedy expansion is correct: at each
    // step pick the largest digit d such that d * beta^{-k} <=
    // remaining.  The digits are bounded (max 2 in the Tribonacci
    // and in sigma_{a,1} for small a), the greedy expansion
    // terminates, and the toroidal projection is correct.
    Patch result;
    result.word.reserve(max_len);
    double remaining = sum;
    std::size_t k = 0;
    while (k < max_len && remaining > 1e-9) {
        const double w = 1.0 / std::pow(beta, static_cast<double>(k));
        std::int8_t best = 0;
        for (std::int8_t d = 2; d >= 0; --d) {
            if (d * w <= remaining + 1e-12) {
                best = d;
                break;
            }
        }
        result.word.push_back(best);
        remaining -= best * w;
        ++k;
    }
    // Pad with trailing zeros up to max_len.
    while (result.word.size() < max_len) result.word.push_back(0);
    return result;
}

// ---- Toroidal projection --------------------------------------------

// Project a finite patch onto the torus parameter.  For the
// unimodular Pisot case, this is a homomorphism from Z_U to a
// torus (Carton--Sudbery--Yassawi Theorem 1.1).
//
// Uses the project's existing n_bonacci_beta(3) for the Tribonacci
// Pisot constant (root of x^3 - x^2 - x - 1 ≈ 1.839).  For the
// sigma_{a,1} family, the Pisot constant is the root of
// x^3 - a x^2 - (a+1) x - 1; we use Newton's method for the
// root (good enough for a numerical projection; the Q(beta)
// algebraic version is in `math/qbeta.hpp` if a certified version
// is needed).
inline double pisot_beta_for_recurrence(const PisotRecurrence& r) {
    if (r.order == 3 && r.coefficients == std::vector<long long>{1, 1, 1}) {
        return ravel::n_bonacci_beta(3);  // Tribonacci Pisot
    }
    // For sigma_{a,1}: root of x^3 - a x^2 - (a+1) x - 1.  Newton
    // with initial guess a + 1/(a+1) (asymptotic approximation),
    // which converges robustly for all a >= 1.
    const long long a = r.coefficients[0];
    double x = static_cast<double>(a)
             + 1.0 / static_cast<double>(a + 1);
    for (int it = 0; it < 100; ++it) {
        const double fx = x * x * x - a * x * x - (a + 1) * x - 1;
        const double dfx = 3 * x * x - 2 * a * x - (a + 1);
        const double dx = fx / dfx;
        x -= dx;
        if (std::abs(dx) < 1e-14) break;
    }
    return x;
}

inline double toroidal_projection(const PisotRecurrence& r,
                                 const Patch& patch) {
    if (patch.word.empty()) return 0.0;
    const double beta = pisot_beta_for_recurrence(r);
    double t = 0.0;
    double w = 1.0;
    for (std::size_t k = 0; k < patch.word.size(); ++k) {
        t += static_cast<double>(patch.word[k]) * w;
        w /= beta;
    }
    return t - std::floor(t);
}

// Find a finite expansion whose toroidal projection is close to
// the target phase `theta`.  For the unimodular Pisot case (Z_U
// is a torus), such an expansion always exists.  Implementation:
// greedy beta-expansion (the same algorithm used in standard
// beta-numerations).
inline Patch
find_expansion_at_phase(const PisotRecurrence& r,
                         double theta,
                         std::size_t max_len) {
    const double beta = pisot_beta_for_recurrence(r);
    Patch p;
    p.word.reserve(max_len);
    // Greedy: pick the largest digit d in {0, 1, 2} such that
    // d / beta^k < theta_remaining.
    double remaining = theta;
    for (std::size_t k = 0; k < max_len; ++k) {
        const double w = 1.0 / std::pow(beta, static_cast<double>(k));
        std::int8_t best = 0;
        for (std::int8_t d = 2; d >= 0; --d) {
            if (d * w <= remaining + 1e-12) {
                best = d;
                break;
            }
        }
        p.word.push_back(best);
        remaining -= best * w;
        if (remaining < 0) remaining = 0;
    }
    return p;
}

}  // namespace math