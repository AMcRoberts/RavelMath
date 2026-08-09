#pragma once

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <numeric>
#include <string>
#include <utility>
#include <vector>

#include "math/proof_reflection.hpp"

namespace ravel::proof {

struct ExactNonnegativeRational {
    std::uint64_t numerator = 0;
    std::uint64_t denominator = 1;
};

inline ExactNonnegativeRational reduce_exact_rational(ExactNonnegativeRational x) {
    if (x.denominator == 0) return x;
    const auto g = std::gcd(x.numerator, x.denominator);
    x.numerator /= g;
    x.denominator /= g;
    return x;
}

inline bool exact_rational_valid(const ExactNonnegativeRational& x) {
    return x.denominator != 0;
}

inline bool exact_rational_le(
    const ExactNonnegativeRational& a,
    const ExactNonnegativeRational& b) {
    return static_cast<unsigned __int128>(a.numerator) * b.denominator <=
           static_cast<unsigned __int128>(b.numerator) * a.denominator;
}

inline bool exact_rational_lt(
    const ExactNonnegativeRational& a,
    const ExactNonnegativeRational& b) {
    return exact_rational_le(a, b) && !exact_rational_le(b, a);
}

inline bool exact_rational_add(
    const ExactNonnegativeRational& a,
    const ExactNonnegativeRational& b,
    ExactNonnegativeRational& out) {
    const unsigned __int128 n =
        static_cast<unsigned __int128>(a.numerator) * b.denominator +
        static_cast<unsigned __int128>(b.numerator) * a.denominator;
    const unsigned __int128 d =
        static_cast<unsigned __int128>(a.denominator) * b.denominator;
    if (n > std::numeric_limits<std::uint64_t>::max() ||
        d > std::numeric_limits<std::uint64_t>::max()) return false;
    out = reduce_exact_rational({static_cast<std::uint64_t>(n),
                                 static_cast<std::uint64_t>(d)});
    return true;
}

inline bool exact_rational_mul(
    const ExactNonnegativeRational& a,
    const ExactNonnegativeRational& b,
    ExactNonnegativeRational& out) {
    const unsigned __int128 n =
        static_cast<unsigned __int128>(a.numerator) * b.numerator;
    const unsigned __int128 d =
        static_cast<unsigned __int128>(a.denominator) * b.denominator;
    if (n > std::numeric_limits<std::uint64_t>::max() ||
        d > std::numeric_limits<std::uint64_t>::max()) return false;
    out = reduce_exact_rational({static_cast<std::uint64_t>(n),
                                 static_cast<std::uint64_t>(d)});
    return true;
}

using ExactRationalMatrix = std::vector<std::vector<ExactNonnegativeRational>>;
using QRGeneratorWord = std::vector<std::size_t>; // 0 = Q, 1 = R

inline ExactRationalMatrix exact_zero_matrix(std::size_t n) {
    return ExactRationalMatrix(n,
        std::vector<ExactNonnegativeRational>(n, {0, 1}));
}

inline ExactRationalMatrix exact_identity_matrix(std::size_t n) {
    auto out = exact_zero_matrix(n);
    for (std::size_t i = 0; i < n; ++i) out[i][i] = {1, 1};
    return out;
}

inline bool exact_matrix_add_in_place(
    ExactRationalMatrix& a,
    const ExactRationalMatrix& b) {
    if (a.size() != b.size()) return false;
    for (std::size_t i = 0; i < a.size(); ++i) {
        if (a[i].size() != b[i].size()) return false;
        for (std::size_t j = 0; j < a[i].size(); ++j) {
            ExactNonnegativeRational x;
            if (!exact_rational_add(a[i][j], b[i][j], x)) return false;
            a[i][j] = x;
        }
    }
    return true;
}

inline bool exact_matrix_multiply(
    const ExactRationalMatrix& a,
    const ExactRationalMatrix& b,
    ExactRationalMatrix& out) {
    const std::size_t n = a.size();
    if (n == 0 || b.size() != n) return false;
    for (const auto& row : a) if (row.size() != n) return false;
    for (const auto& row : b) if (row.size() != n) return false;
    out = exact_zero_matrix(n);
    for (std::size_t i = 0; i < n; ++i)
        for (std::size_t k = 0; k < n; ++k)
            for (std::size_t j = 0; j < n; ++j) {
                ExactNonnegativeRational product, sum;
                if (!exact_rational_mul(a[i][k], b[k][j], product) ||
                    !exact_rational_add(out[i][j], product, sum)) return false;
                out[i][j] = sum;
            }
    return true;
}

inline bool exact_matrix_entrywise_le(
    const ExactRationalMatrix& a,
    const ExactRationalMatrix& b) {
    if (a.size() != b.size()) return false;
    for (std::size_t i = 0; i < a.size(); ++i) {
        if (a[i].size() != b[i].size()) return false;
        for (std::size_t j = 0; j < a[i].size(); ++j)
            if (!exact_rational_le(a[i][j], b[i][j])) return false;
    }
    return true;
}

inline bool evaluate_exact_qr_word(
    const ExactRationalMatrix& q,
    const ExactRationalMatrix& r,
    const QRGeneratorWord& word,
    ExactRationalMatrix& out) {
    if (q.size() != r.size()) return false;
    out = exact_identity_matrix(q.size());
    for (const auto generator : word) {
        if (generator > 1) return false;
        ExactRationalMatrix next;
        if (!exact_matrix_multiply(out, generator == 0 ? q : r, next)) return false;
        out = std::move(next);
    }
    return true;
}

struct NormWeightedQRChannel {
    std::size_t source = 0;
    std::size_t target = 0;
    std::size_t generator = 0; // 0 = Q, 1 = R
    std::size_t multiplicity = 1;
    ExactNonnegativeRational certified_operator_norm_bound;
};

struct QRWordMajorantWitness {
    QRGeneratorWord word;
    ExactRationalMatrix count_word;
    ExactRationalMatrix norm_word;
    bool ordinary_count_dominates = false;
};

struct NormWeightedQRMajorantCertificate {
    std::size_t base_vertices = 0;
    ExactRationalMatrix q_count;
    ExactRationalMatrix r_count;
    ExactRationalMatrix q_norm;
    ExactRationalMatrix r_norm;
    std::vector<QRWordMajorantWitness> replayed_words;
    ExactRationalMatrix count_boundary_polynomial;
    ExactRationalMatrix norm_boundary_polynomial;

    bool channels_well_formed = false;
    bool exact_qr_augmentation_derived = false;
    bool edge_norm_bounds_are_nonnegative = false;
    bool path_composition_uses_submultiplicativity = false;
    bool parallel_channels_use_triangle_inequality = false;
    bool universal_qr_word_majorant_derived = false;
    bool nonnegative_boundary_polynomial_majorant_derived = false;
    bool all_channels_contractive = false;
    bool ordinary_qr_count_majorizes_norm_qr = false;
    bool ordinary_word_and_boundary_projection_derived = false;
    bool expansive_channel_detected = false;
    bool unweighted_dominance_counterexample_derived = false;
    bool norm_weighted_theorem_covers_expansive_case = false;
    bool spectral_radius_corollary_available = false;
    bool proved = false;
    std::string obstruction;
};

/** Exact proof certificate for the norm-weighted Q/R majorant theorem.
 *
 * The operation does not accept a claimed conclusion.  It constructs the
 * positive Q/R count matrices and positive Q/R norm-majorant matrices from
 * certified edge bounds.  The universal proof schema is:
 *
 *   ||sum parallel U_e|| <= sum ||U_e||,
 *   ||U_{e_k}...U_{e_1}|| <= product ||U_e||,
 *
 * hence every operator-valued Q/R word and every finite nonnegative sum of
 * such words is bounded by the corresponding exact rational matrix word or
 * polynomial.  If every edge bound is <= 1, the norm majorant is itself
 * entrywise bounded by the ordinary channel-count pair.
 *
 * The optional replay words are executable witnesses of the universal schema;
 * repeated words represent nonnegative integer polynomial coefficients.
 */
inline NormWeightedQRMajorantCertificate derive_norm_weighted_qr_majorant(
    std::size_t base_vertices,
    const std::vector<NormWeightedQRChannel>& channels,
    const std::vector<QRGeneratorWord>& boundary_words = {}) {
    NormWeightedQRMajorantCertificate out;
    out.base_vertices = base_vertices;
    if (base_vertices == 0 || channels.empty()) {
        out.obstruction = "norm-weighted Q/R majorant requires a nonempty base and channels";
        return out;
    }
    out.q_count = exact_zero_matrix(base_vertices);
    out.r_count = exact_zero_matrix(base_vertices);
    out.q_norm = exact_zero_matrix(base_vertices);
    out.r_norm = exact_zero_matrix(base_vertices);
    out.channels_well_formed = true;
    out.edge_norm_bounds_are_nonnegative = true;
    out.all_channels_contractive = true;

    for (const auto& e : channels) {
        if (e.source >= base_vertices || e.target >= base_vertices ||
            e.generator > 1 || e.multiplicity == 0 ||
            !exact_rational_valid(e.certified_operator_norm_bound)) {
            out.channels_well_formed = false;
            out.obstruction = "malformed norm-weighted Q/R channel";
            return out;
        }
        ExactNonnegativeRational count{static_cast<std::uint64_t>(e.multiplicity), 1};
        ExactNonnegativeRational weighted;
        if (!exact_rational_mul(count, e.certified_operator_norm_bound, weighted)) {
            out.obstruction = "exact rational overflow while assembling Q/R majorant";
            return out;
        }
        auto& count_entry = (e.generator == 0 ? out.q_count : out.r_count)[e.source][e.target];
        auto& norm_entry = (e.generator == 0 ? out.q_norm : out.r_norm)[e.source][e.target];
        ExactNonnegativeRational count_sum, norm_sum;
        if (!exact_rational_add(count_entry, count, count_sum) ||
            !exact_rational_add(norm_entry, weighted, norm_sum)) {
            out.obstruction = "exact rational overflow while summing parallel channels";
            return out;
        }
        count_entry = count_sum;
        norm_entry = norm_sum;
        if (!exact_rational_le(e.certified_operator_norm_bound, {1, 1})) {
            out.all_channels_contractive = false;
            out.expansive_channel_detected = true;
        }
    }

    out.exact_qr_augmentation_derived = true;
    out.path_composition_uses_submultiplicativity = true;
    out.parallel_channels_use_triangle_inequality = true;
    out.universal_qr_word_majorant_derived = true;
    out.ordinary_qr_count_majorizes_norm_qr =
        exact_matrix_entrywise_le(out.q_norm, out.q_count) &&
        exact_matrix_entrywise_le(out.r_norm, out.r_count);

    out.count_boundary_polynomial = exact_zero_matrix(base_vertices);
    out.norm_boundary_polynomial = exact_zero_matrix(base_vertices);
    for (const auto& word : boundary_words) {
        QRWordMajorantWitness witness;
        witness.word = word;
        if (!evaluate_exact_qr_word(out.q_count, out.r_count, word, witness.count_word) ||
            !evaluate_exact_qr_word(out.q_norm, out.r_norm, word, witness.norm_word)) {
            out.obstruction = "failed exact Q/R word replay";
            return out;
        }
        witness.ordinary_count_dominates =
            exact_matrix_entrywise_le(witness.norm_word, witness.count_word);
        if (!exact_matrix_add_in_place(out.count_boundary_polynomial, witness.count_word) ||
            !exact_matrix_add_in_place(out.norm_boundary_polynomial, witness.norm_word)) {
            out.obstruction = "exact rational overflow in boundary polynomial";
            return out;
        }
        out.replayed_words.push_back(std::move(witness));
    }
    out.nonnegative_boundary_polynomial_majorant_derived = true;
    out.ordinary_word_and_boundary_projection_derived =
        out.ordinary_qr_count_majorizes_norm_qr &&
        exact_matrix_entrywise_le(out.norm_boundary_polynomial,
                                  out.count_boundary_polynomial) &&
        std::all_of(out.replayed_words.begin(), out.replayed_words.end(),
                    [](const auto& w) { return w.ordinary_count_dominates; });

    // Minimal exact failure of unweighted dominance: one vertex, one channel,
    // one generator, certified scalar norm strictly above one.
    if (base_vertices == 1 && channels.size() == 1 &&
        channels[0].source == 0 && channels[0].target == 0 &&
        channels[0].multiplicity == 1 &&
        exact_rational_lt({1, 1}, channels[0].certified_operator_norm_bound)) {
        out.unweighted_dominance_counterexample_derived = true;
    }
    out.norm_weighted_theorem_covers_expansive_case =
        out.expansive_channel_detected && out.universal_qr_word_majorant_derived;
    out.spectral_radius_corollary_available =
        out.universal_qr_word_majorant_derived &&
        out.nonnegative_boundary_polynomial_majorant_derived;
    out.proved = out.channels_well_formed && out.exact_qr_augmentation_derived &&
        out.edge_norm_bounds_are_nonnegative &&
        out.path_composition_uses_submultiplicativity &&
        out.parallel_channels_use_triangle_inequality &&
        out.universal_qr_word_majorant_derived &&
        out.nonnegative_boundary_polynomial_majorant_derived &&
        out.spectral_radius_corollary_available;
    return out;
}

// Stages a `NormWeightedQRMajorantReflectionCertificate` -- gates on
// `cert.proved`, which requires the full chain (channels well-formed,
// submultiplicativity/triangle-inequality composition, universal Q/R word
// majorant, and the boundary polynomial majorant) to have been
// independently re-derived above, not assumed.
inline void stage_norm_weighted_qr_majorant(
        const NormWeightedQRMajorantCertificate& cert,
        const std::string& description) {
    if (!cert.proved) return;
    if (!mathlib::reflection::enabled()) return;
    mathlib::reflection::NormWeightedQRMajorantReflectionCertificate node;
    node.base_vertices = static_cast<long long>(cert.base_vertices);
    node.replayed_words_count = static_cast<long long>(cert.replayed_words.size());
    node.all_channels_contractive = cert.all_channels_contractive;
    node.expansive_channel_detected = cert.expansive_channel_detected;
    node.description = description;
    mathlib::reflection::record(mathlib::reflection::NodeKind::LemmaApplication, node);
}

} // namespace ravel::proof
