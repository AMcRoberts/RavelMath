// include/adelic/prefix_automaton.hpp
//
// Prefix automaton and digit set for a Pisot substitution, per
// Minervino-Thuswaldner "The geometry of non-unit Pisot
// substitutions" §2.3.  Used as the input to the adelic tiling
// classifier's strong-coincidence and geometric-property-(F)
// checks (see include/adelic/coincidence_and_property_f.hpp).
//
// Construction (per §2.3 of the paper):
//   - For every way to write σ(c) = p · a · s (prefix p, single
//     letter a, suffix s), record a labeled edge c →^p a in the
//     prefix automaton.  Multiple (c, a, p) triples from the same
//     (c, a) collapse to a single edge (the automaton is
//     deterministic at the (c, a) level).
//   - Define δ(p) = ⟨P(p), v⟩ where P(p) is the abelianization
//     (letter-count vector) of p and v is the LEFT Perron
//     eigenvector of the substitution's incidence matrix M
//     (i.e., v^T M = β v^T), with entries in Q(β).
//   - The finite set D = {δ(p) : p is a prefix that occurs} is
//     the digit set.  It is exact in Q(β).
//
// All arithmetic is exact (Q(β) via the math library).  The
// "left vs right" eigenvector distinction matters: the digit
// definition uses the LEFT one (v^T M = β v^T, so v is the
// right Perron eigenvector of M^T).  The math library's
// `right_eigenvector_via_qbeta` returns the right Perron
// eigenvector of its input, so passing M^T to it yields the
// left Perron eigenvector of M.
//
// Eigenvector convention: v[d-1] = 1 (the math library's
// Cramer-rule normalization; any other Q(β)-proportional
// scaling would give the same geometric results, but the
// convention is needed for consistent digit-set comparison).

#pragma once

#include <algorithm>
#include <array>
#include <cstddef>
#include <utility>
#include <vector>

#include "math/qbeta.hpp"

namespace adelic {

// ===================================================================
// A single labeled edge of the prefix automaton.
// ===================================================================
//
// source_letter c, target_letter a, and the prefix p of σ(c) such
// that σ(c) = p · a · s for some suffix s.  Multiple edges may
// exist between the same (c, a) pair (if σ(c) has the inner letter
// a in multiple positions); the automaton construction collapses
// them by deduplicating on (c, a, p) — the (c, a) pair is what
// determines the edge.

template <std::size_t d>
struct PrefixEdge {
    long long source_letter;
    long long target_letter;
    std::vector<long long> prefix;
};

// ===================================================================
// The full prefix automaton: edges, per-source views, distinct
// prefixes, and the digit set D.
// ===================================================================

template <std::size_t d>
struct PrefixAutomaton {
    // All (source, target, prefix) triples (with duplicates; the
    // by_source view is the deduplicated, source-indexed one).
    std::vector<PrefixEdge<d>> edges;

    // For each source letter c, the set of distinct (target, prefix)
    // pairs reachable from c, sorted lexicographically by
    // (target, prefix) for determinism.  This is the
    // deduplicated, source-indexed view of the automaton.
    std::array<std::vector<std::pair<long long, std::vector<long long>>>, d> by_source;

    // The distinct prefixes that occur anywhere in the automaton,
    // sorted lexicographically for determinism.  Indexing into
    // this gives the indexing into `digit_set`.
    std::vector<std::vector<long long>> distinct_prefixes;

    // The digit set D: for each entry i in `distinct_prefixes`,
    // digit_set[i] = δ(p_i) = sum of v[letter] over all letters in
    // p_i (with multiplicity), where v is the left Perron
    // eigenvector scaled with v[d-1] = 1.  Entries are exact
    // Q(β) elements.
    std::vector<mathlib::QElem> digit_set;

    // The Q(β) ring the digits live in (so callers can
    // interpret them, e.g. evaluate at a real approximation of β
    // for sanity checks).
    mathlib::QBetaRing ring;

    // The left Perron eigenvector used to compute the digit set
    // (exposed so downstream code can use it without recomputing).
    std::vector<mathlib::QElem> left_eigenvector;
};

// ===================================================================
// Build the prefix automaton from a substitution's images and the
// left Perron eigenvector.
// ===================================================================
//
// `images` is the substitution σ: images[c] is the word σ(c) (a
// list of letter indices in [0, d)).
// `left_eigenvector` is the left Perron eigenvector v of the
// incidence matrix M (M[c][letter] = count of `letter` in σ(c)),
// with entries in Q(β), scaled with v[d-1] = 1.
// `ring` is the Q(β) ring the eigenvector lives in.
//
// The construction is exact: no floating point.  Each digit is
// the sum of the appropriate eigenvector components, computed
// in Q(β).

template <std::size_t d>
PrefixAutomaton<d> build_prefix_automaton(
    const std::array<std::vector<long long>, d>& images,
    const std::vector<mathlib::QElem>& left_eigenvector,
    const mathlib::QBetaRing& ring) {

    PrefixAutomaton<d> result;
    result.ring = ring;
    result.left_eigenvector = left_eigenvector;

    // Step 1: enumerate all (source, target, prefix) triples by
    // scanning each σ(c) and recording the decomposition
    // σ(c) = (prefix before position k) · (letter at k) ·
    // (suffix after k) for every position k.
    for (std::size_t c = 0; c < d; ++c) {
        const auto& img = images[c];
        for (std::size_t k = 0; k < img.size(); ++k) {
            PrefixEdge<d> edge;
            edge.source_letter = static_cast<long long>(c);
            edge.target_letter = img[k];
            edge.prefix.assign(img.begin(), img.begin() + k);
            result.edges.push_back(std::move(edge));
        }
    }

    // Step 2: deduplicate per-source.  The automaton is
    // deterministic at the (c, a) level: multiple occurrences of
    // a in σ(c) (same (c, a) pair) all produce the same set of
    // distinct prefixes.  We sort by (target, prefix) for
    // determinism.
    for (std::size_t c = 0; c < d; ++c) {
        std::vector<std::pair<long long, std::vector<long long>>> collected;
        for (const auto& edge : result.edges) {
            if (edge.source_letter == static_cast<long long>(c)) {
                collected.push_back({edge.target_letter, edge.prefix});
            }
        }
        std::sort(collected.begin(), collected.end());
        collected.erase(std::unique(collected.begin(), collected.end()),
                       collected.end());
        result.by_source[c] = std::move(collected);
    }

    // Step 3: collect all distinct prefixes (across all source
    // letters) and sort for determinism.  This is the indexing
    // for `digit_set`.
    {
        std::vector<std::vector<long long>> all_prefixes;
        for (const auto& edge : result.edges) {
            all_prefixes.push_back(edge.prefix);
        }
        std::sort(all_prefixes.begin(), all_prefixes.end());
        all_prefixes.erase(std::unique(all_prefixes.begin(), all_prefixes.end()),
                          all_prefixes.end());
        result.distinct_prefixes = std::move(all_prefixes);
    }

    // Step 4: compute the digit δ(p) for each distinct prefix.
    // δ(p) = sum_{letter in p (with multiplicity)} v[letter],
    // where v is the left Perron eigenvector in Q(β).
    result.digit_set.reserve(result.distinct_prefixes.size());
    for (const auto& p : result.distinct_prefixes) {
        mathlib::QElem digit = ring.zero();
        for (long long letter : p) {
            if (letter < 0) {
                throw std::invalid_argument(
                    "build_prefix_automaton: negative letter index in prefix");
            }
            std::size_t idx = static_cast<std::size_t>(letter);
            if (idx >= left_eigenvector.size()) {
                throw std::invalid_argument(
                    "build_prefix_automaton: letter index out of range");
            }
            digit = ring.add(digit, left_eigenvector[idx]);
        }
        result.digit_set.push_back(std::move(digit));
    }

    return result;
}

}  // namespace adelic
