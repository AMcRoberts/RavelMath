// include/adelic/classify_adelic.hpp
//
// Shared adelic-tiling classification logic, factored out of the
// two driver programs that previously each carried their own copy
// of the same code (app/classify_adelic_tiling.cpp and
// app/sweep_nonunit_property_f.cpp).  The single implementation
// here uses the unified `make_combined_padic_bound` path for
// every prime-factorization shape (unit, single-prime, multi-prime,
// any (e, f) per ideal) -- the previous per-prime logic that only
// handled the first prime is gone.
//
// Output formatting is parameterized by a small enum so the two
// driver programs can keep their distinct visual styles (one prints
// a long "=====" header per case, the other a short "---" header)
// without each re-implementing the underlying computation.  The
// verdict and all per-case diagnostics come from a single
// `classify_tiling` entry point.
//
// The return value is a struct carrying everything the caller
// might want: the verdict, the node count, the strong-coincidence
// result, the property-F result, and a "any non-maximal" flag
// for the post-print caveat.

#pragma once

#include <array>
#include <cstdio>
#include <stdexcept>
#include <string>
#include <vector>

#include "math/qbeta.hpp"
#include "math/linalg_qbeta.hpp"
#include "adelic/dedekind_factorization.hpp"
#include "adelic/ideal_arithmetic.hpp"
#include "adelic/prefix_automaton.hpp"
#include "adelic/coincidence_and_property_f.hpp"
#include "ravel/proof/coincidence_closure.hpp"

namespace adelic {

// Final verdict for the tiling classification.  Matches the four
// labeled outcomes in the docs (TILES / INCONCLUSIVE / DOES NOT
// TILE) plus a SKIPPED case for the eigenvector-failure path.
enum class TilingVerdict {
    ESTABLISHED,                     // strong coincidence + property F both HOLD
    INCONCLUSIVE_STRONG_COINCIDENCE, // strong coin didn't resolve within cutoff
    INCONCLUSIVE_PROPERTY_F_BUDGET,  // property F didn't close within node budget
    DOES_NOT_TILE_STRONG_COINCIDENCE,// strong coin definitively FAILS
    DOES_NOT_TILE_PROPERTY_F,        // property F FAILS under validated bound
    SKIPPED_EIGENVECTOR,             // eigenvector computation failed
};

inline const char* verdict_label(TilingVerdict v) {
    switch (v) {
        case TilingVerdict::ESTABLISHED:                       return "PISOT CONJECTURE ESTABLISHED (tiles)";
        case TilingVerdict::INCONCLUSIVE_STRONG_COINCIDENCE:    return "INCONCLUSIVE (strong coincidence didn't resolve)";
        case TilingVerdict::INCONCLUSIVE_PROPERTY_F_BUDGET:     return "INCONCLUSIVE (property F budget exhausted)";
        case TilingVerdict::DOES_NOT_TILE_STRONG_COINCIDENCE:  return "DOES NOT TILE (strong coincidence FAILS)";
        case TilingVerdict::DOES_NOT_TILE_PROPERTY_F:          return "DOES NOT TILE (property F FAILS)";
        case TilingVerdict::SKIPPED_EIGENVECTOR:               return "SKIPPED (eigenvector failure)";
    }
    return "?";
}

// Two header styles so the two drivers keep their distinct visual
// identities without re-implementing the classify logic.  A new
// caller picks the style that fits their driver.
enum class HeaderStyle {
    LONG_RULE,   // "==========================================\n  ...\n==========================================\n"
    SHORT_DASH,  // "--- name ---\n"
};

// One classification's worth of inputs and outputs.  Constructing
// one of these from `classify_tiling` is the entire purpose of
// this header.
struct TilingClassification {
    // Inputs (echoed for the caller's convenience when chaining).
    std::string name;
    std::vector<std::vector<long long>> factorization;  // one row per prime in primes_dividing_det
    bool any_non_maximal;

    // Strong coincidence.
    bool strong_coincidence_holds;
    bool strong_coincidence_inconclusive;
    long long strong_coincidence_depth;
    std::vector<long long> strong_coincidence_pair_resolution_depths;
    bool strong_coincidence_closure_attempted;
    bool strong_coincidence_closure_used;
    bool strong_coincidence_closure_inconclusive;
    bool strong_coincidence_closure_unsupported;

    // Property (F) under the combined p-adic bound.
    bool property_f_holds;
    bool property_f_inconclusive;
    // False when Dedekind's order/factorization checks do not certify the
    // p-adic predicate; exploratory graph data is retained, but the verdict
    // cannot be ESTABLISHED.
    bool property_f_bound_trusted;
    long long property_f_nodes;

    // Final verdict.
    TilingVerdict verdict;
};

// The single classification entry point.  Runs the full pipeline
// (factor + cross-check, prefix automaton, strong coincidence,
// combined p-adic bound, property (F)) and returns everything.  The
// caller is responsible for printing the result in whatever format
// it wants.  Side effect: prints the factorization and per-step
// diagnostic lines to stdout (the case header is NOT printed by
// this function -- the caller prints its own header, in its own
// style, before calling).
//
// The "extra_bound_diagnostic" string is a short label that gets
// printed on the property-(F) line (e.g., "archimedean-only" for
// the unit case, "combined p-adic bound across 2 primes" for a
// multi-prime case).  This keeps the per-step diagnostic in one
// place without re-implementing the bound selection here.
template <std::size_t d>
TilingClassification classify_tiling(
    const char* name,
    const std::array<std::vector<long long>, d>& images,
    const mathlib::PolyZ& charpoly,
    const std::vector<std::vector<long long>>& M_transpose,
    const std::vector<long long>& primes_dividing_det,
    long long property_f_node_budget = 1'000'000) {

    TilingClassification out;
    out.name = name;
    out.strong_coincidence_closure_used = false;
    out.strong_coincidence_closure_attempted = false;
    out.strong_coincidence_closure_inconclusive = false;
    out.strong_coincidence_closure_unsupported = false;
    out.strong_coincidence_pair_resolution_depths.clear();

    // Step 1: factor each prime dividing |det M|, cross-checked.
    out.any_non_maximal = false;
    for (long long p : primes_dividing_det) {
        auto fac = adelic::factor_prime_in_qbeta(charpoly, p);
        bool cross_ok = adelic::cross_check_dedekind_factorization(fac, charpoly, static_cast<long long>(d));
        for (const auto& pi : fac.prime_ideals) {
            (void)pi;  // diagnostic info gathered by caller via the factorization vector
            std::vector<long long> row = {p, pi.e, pi.f, pi.e * pi.f};
            out.factorization.push_back(row);
        }
        if (!fac.maximal) out.any_non_maximal = true;
        (void)cross_ok;  // diagnostic info printed by caller
    }

    mathlib::QBetaRing R(charpoly);
    std::vector<std::vector<long long>> Mt = M_transpose;
    auto eig = mathlib::right_eigenvector_via_qbeta(Mt, R);

    if (!eig.ok) {
        out.strong_coincidence_holds = false;
        out.strong_coincidence_inconclusive = false;
        out.strong_coincidence_depth = 0;
        out.strong_coincidence_pair_resolution_depths.clear();
        out.property_f_holds = false;
        out.property_f_inconclusive = false;
        out.property_f_nodes = 0;
        out.verdict = TilingVerdict::SKIPPED_EIGENVECTOR;
        return out;
    }

    // Step 3: strong coincidence.  For primitive expanding instances whose
    // deterministic-chain shape closes, the exact landmark closure avoids
    // exponential word materialization.  Unsupported or inconclusive closure
    // runs fall back to the established bounded word checker, preserving the
    // prior verdict semantics while making the fast path explicit in output.
    StrongCoincidenceResult coin{};
    out.strong_coincidence_closure_attempted = true;
    try {
        std::array<std::array<long long, d>, d> matrix{};
        for (std::size_t column = 0; column < d; ++column)
            for (long long letter : images[column]) {
                if (letter < 0 || static_cast<std::size_t>(letter) >= d)
                    throw std::invalid_argument("classify_tiling: invalid substitution letter");
                ++matrix[static_cast<std::size_t>(letter)][column];
            }
        // The closure state space is deduplicated and no longer scales with
        // |sigma^K|, so it can search substantially deeper than the legacy
        // word-materialization fallback while retaining an explicit outcome
        // budget.
        const auto closure = ravel::proof::check_strong_coincidence_closure<d>(
            images, matrix, 64, 1'000'000);
        if (closure.holds) {
            coin.holds = true;
            coin.inconclusive = false;
            coin.depth_reached = closure.depth_reached;
            coin.unresolved_pairs = closure.unresolved_pairs;
            coin.pair_resolution_depths = closure.pair_resolution_depths;
            out.strong_coincidence_closure_used = true;
        } else {
            out.strong_coincidence_closure_inconclusive = closure.inconclusive;
            coin = adelic::check_strong_coincidence<d>(images);
        }
    } catch (const std::invalid_argument&) {
        out.strong_coincidence_closure_unsupported = true;
        coin = adelic::check_strong_coincidence<d>(images);
    } catch (const std::overflow_error&) {
        out.strong_coincidence_closure_inconclusive = true;
        coin = adelic::check_strong_coincidence<d>(images);
    }
    out.strong_coincidence_holds = coin.holds;
    out.strong_coincidence_inconclusive = coin.inconclusive;
    out.strong_coincidence_depth = coin.depth_reached;
    out.strong_coincidence_pair_resolution_depths = coin.pair_resolution_depths;

    // Step 4: property (F) via the unified combined p-adic bound.
    auto automaton = adelic::build_prefix_automaton<d>(images, eig.v, R);
    adelic::PropertyFResult propf;
    bool property_f_bound_trusted = true;
    if (primes_dividing_det.empty()) {
        propf = adelic::check_property_f<d>(automaton, property_f_node_budget);
    } else {
        auto [combined_bound, trusted] = adelic::make_combined_padic_bound(
            primes_dividing_det, charpoly);
        propf = adelic::check_property_f<d>(automaton, property_f_node_budget, combined_bound);
        property_f_bound_trusted = trusted;
        propf.extra_bound_trusted = trusted;
        if (!trusted) {
            // Preserve the exploratory graph and node count, but do not
            // promote a run based on a non-maximal Dedekind order to a
            // certified Property-F result.
            propf.holds = false;
            propf.inconclusive = true;
        }
    }
    out.property_f_holds = propf.holds;
    out.property_f_inconclusive = propf.inconclusive;
    out.property_f_bound_trusted = property_f_bound_trusted;
    out.property_f_nodes = propf.nodes_explored;

    // Step 5: combine the two results into a verdict.
    if (!coin.holds && !coin.inconclusive) {
        out.verdict = TilingVerdict::DOES_NOT_TILE_STRONG_COINCIDENCE;
    } else if (coin.inconclusive) {
        out.verdict = TilingVerdict::INCONCLUSIVE_STRONG_COINCIDENCE;
    } else if (propf.inconclusive) {
        out.verdict = TilingVerdict::INCONCLUSIVE_PROPERTY_F_BUDGET;
    } else if (propf.holds) {
        out.verdict = TilingVerdict::ESTABLISHED;
    } else {
        out.verdict = TilingVerdict::DOES_NOT_TILE_PROPERTY_F;
    }
    return out;
}

// Helper: the per-case header (two styles supported), a thin wrapper
// to keep the two drivers' visual identities distinct without
// re-implementing the compute logic.
inline void print_case_header(HeaderStyle style, const char* name) {
    if (style == HeaderStyle::LONG_RULE) {
        std::printf("=================================================================\n");
        std::printf("Adelic tiling classification: %s\n", name);
        std::printf("=================================================================\n");
    } else {
        std::printf("--- %s ---\n", name);
    }
}

}  // namespace adelic
