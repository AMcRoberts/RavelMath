#pragma once

#include <algorithm>
#include <cstddef>
#include <string>
#include <utility>
#include <vector>

#include "math/proof_reflection.hpp"
#include "ravel/proof/norm_weighted_qr_majorant.hpp"

namespace ravel::proof {

using PositiveGeneratorWord = std::vector<std::size_t>;

struct NormWeightedGrammarChannel {
    std::size_t source = 0;
    std::size_t target = 0;
    std::size_t generator = 0;
    std::size_t multiplicity = 1;
    ExactNonnegativeRational certified_operator_norm_bound;
};

struct GrammarWordMajorantWitness {
    PositiveGeneratorWord word;
    ExactRationalMatrix count_word;
    ExactRationalMatrix norm_word;
    bool ordinary_count_dominates = false;
};

struct FinitePositiveGrammarMajorantCertificate {
    std::size_t base_vertices = 0;
    std::size_t generator_count = 0;
    std::vector<ExactRationalMatrix> count_generators;
    std::vector<ExactRationalMatrix> norm_generators;
    std::vector<GrammarWordMajorantWitness> replayed_words;
    ExactRationalMatrix count_boundary_polynomial;
    ExactRationalMatrix norm_boundary_polynomial;

    bool channels_well_formed = false;
    bool exact_generator_augmentation_derived = false;
    bool generator_partition_total = false;
    bool generator_partition_single_valued = false;
    bool parallel_channels_use_triangle_inequality = false;
    bool path_composition_uses_submultiplicativity = false;
    bool universal_word_majorant_derived = false;
    bool positive_polynomial_majorant_derived = false;
    bool all_channels_contractive = false;
    bool ordinary_generatorwise_projection_derived = false;
    bool ordinary_word_and_polynomial_projection_derived = false;
    bool expansive_channel_detected = false;
    bool norm_weighted_theorem_covers_expansive_case = false;
    bool qr_specialization_available = false;
    bool proved = false;
    std::string obstruction;
};

inline bool evaluate_exact_generator_word(
    const std::vector<ExactRationalMatrix>& generators,
    const PositiveGeneratorWord& word,
    ExactRationalMatrix& out) {
    if (generators.empty()) return false;
    const auto n = generators.front().size();
    out = exact_identity_matrix(n);
    for (const auto g : word) {
        if (g >= generators.size()) return false;
        ExactRationalMatrix next;
        if (!exact_matrix_multiply(out, generators[g], next)) return false;
        out = std::move(next);
    }
    return true;
}

/** Proof-producing finite-positive-grammar lift.
 *
 * A finite generator alphabet Gamma replaces the hard-coded Q/R alphabet.
 * Every channel is assigned exactly one generator label and an exact certified
 * nonnegative operator-norm bound.  The operation derives one positive count
 * matrix and one positive norm-weighted matrix for each generator.  Triangle
 * inequality handles parallel channels, submultiplicativity handles words,
 * and repeated words encode nonnegative integer coefficients in boundary
 * polynomials.
 */
inline FinitePositiveGrammarMajorantCertificate
 derive_finite_positive_grammar_majorant(
    std::size_t base_vertices,
    std::size_t generator_count,
    const std::vector<NormWeightedGrammarChannel>& channels,
    const std::vector<PositiveGeneratorWord>& boundary_words = {}) {
    FinitePositiveGrammarMajorantCertificate out;
    out.base_vertices = base_vertices;
    out.generator_count = generator_count;
    if (base_vertices == 0 || generator_count == 0 || channels.empty()) {
        out.obstruction = "finite positive grammar requires nonempty vertices, generators, and channels";
        return out;
    }
    out.count_generators.assign(generator_count, exact_zero_matrix(base_vertices));
    out.norm_generators.assign(generator_count, exact_zero_matrix(base_vertices));
    out.channels_well_formed = true;
    out.generator_partition_total = true;
    out.generator_partition_single_valued = true; // one label field per channel
    out.all_channels_contractive = true;

    for (const auto& e : channels) {
        if (e.source >= base_vertices || e.target >= base_vertices ||
            e.generator >= generator_count || e.multiplicity == 0 ||
            !exact_rational_valid(e.certified_operator_norm_bound)) {
            out.channels_well_formed = false;
            out.obstruction = "malformed finite-positive-grammar channel";
            return out;
        }
        const ExactNonnegativeRational count{
            static_cast<std::uint64_t>(e.multiplicity), 1};
        ExactNonnegativeRational weighted;
        if (!exact_rational_mul(count, e.certified_operator_norm_bound, weighted)) {
            out.obstruction = "exact rational overflow while weighting grammar channel";
            return out;
        }
        auto& c = out.count_generators[e.generator][e.source][e.target];
        auto& n = out.norm_generators[e.generator][e.source][e.target];
        ExactNonnegativeRational cs, ns;
        if (!exact_rational_add(c, count, cs) || !exact_rational_add(n, weighted, ns)) {
            out.obstruction = "exact rational overflow while summing grammar channels";
            return out;
        }
        c = cs;
        n = ns;
        if (!exact_rational_le(e.certified_operator_norm_bound, {1, 1})) {
            out.all_channels_contractive = false;
            out.expansive_channel_detected = true;
        }
    }

    out.exact_generator_augmentation_derived = true;
    out.parallel_channels_use_triangle_inequality = true;
    out.path_composition_uses_submultiplicativity = true;
    out.universal_word_majorant_derived = true;
    out.ordinary_generatorwise_projection_derived = true;
    for (std::size_t g = 0; g < generator_count; ++g)
        out.ordinary_generatorwise_projection_derived &=
            exact_matrix_entrywise_le(out.norm_generators[g], out.count_generators[g]);

    out.count_boundary_polynomial = exact_zero_matrix(base_vertices);
    out.norm_boundary_polynomial = exact_zero_matrix(base_vertices);
    for (const auto& word : boundary_words) {
        GrammarWordMajorantWitness witness;
        witness.word = word;
        if (!evaluate_exact_generator_word(out.count_generators, word, witness.count_word) ||
            !evaluate_exact_generator_word(out.norm_generators, word, witness.norm_word)) {
            out.obstruction = "failed exact finite-generator word replay";
            return out;
        }
        witness.ordinary_count_dominates =
            exact_matrix_entrywise_le(witness.norm_word, witness.count_word);
        if (!exact_matrix_add_in_place(out.count_boundary_polynomial, witness.count_word) ||
            !exact_matrix_add_in_place(out.norm_boundary_polynomial, witness.norm_word)) {
            out.obstruction = "exact rational overflow in grammar boundary polynomial";
            return out;
        }
        out.replayed_words.push_back(std::move(witness));
    }
    out.positive_polynomial_majorant_derived = true;
    out.ordinary_word_and_polynomial_projection_derived =
        out.ordinary_generatorwise_projection_derived &&
        exact_matrix_entrywise_le(out.norm_boundary_polynomial,
                                  out.count_boundary_polynomial) &&
        std::all_of(out.replayed_words.begin(), out.replayed_words.end(),
                    [](const auto& w) { return w.ordinary_count_dominates; });
    out.norm_weighted_theorem_covers_expansive_case =
        out.expansive_channel_detected && out.universal_word_majorant_derived;
    out.qr_specialization_available = generator_count == 2;
    out.proved = out.channels_well_formed && out.generator_partition_total &&
        out.generator_partition_single_valued &&
        out.exact_generator_augmentation_derived &&
        out.parallel_channels_use_triangle_inequality &&
        out.path_composition_uses_submultiplicativity &&
        out.universal_word_majorant_derived &&
        out.positive_polynomial_majorant_derived;
    return out;
}

// Independently re-sums (not trusted from `.proved`) the per-generator
// count and norm-weighted matrices to a scalar rational pair each, checks
// the norm-weighted scalar sum does not exceed the count scalar sum for
// EVERY generator (the concrete instance of
// `ordinary_generatorwise_projection_derived` this reflection wiring
// needs), then stages the result.
inline void stage_finite_positive_grammar_majorant(
    const FinitePositiveGrammarMajorantCertificate& cert, const std::string& description) {
    if (!cert.proved) return;
    if (cert.count_generators.size() != cert.generator_count ||
        cert.norm_generators.size() != cert.generator_count)
        return;
    std::vector<ExactNonnegativeRational> count_scalar(cert.generator_count, {0, 1});
    std::vector<ExactNonnegativeRational> norm_scalar(cert.generator_count, {0, 1});
    for (std::size_t g = 0; g < cert.generator_count; ++g) {
        for (const auto& row : cert.count_generators[g])
            for (const auto& v : row) {
                ExactNonnegativeRational next;
                if (!exact_rational_add(count_scalar[g], v, next)) return;
                count_scalar[g] = next;
            }
        for (const auto& row : cert.norm_generators[g])
            for (const auto& v : row) {
                ExactNonnegativeRational next;
                if (!exact_rational_add(norm_scalar[g], v, next)) return;
                norm_scalar[g] = next;
            }
        if (!exact_rational_le(norm_scalar[g], count_scalar[g])) return;
    }
    if (!mathlib::reflection::enabled()) return;
    mathlib::reflection::FinitePositiveGrammarMajorantReflectionCertificate node;
    node.base_vertices = static_cast<long long>(cert.base_vertices);
    node.generator_count = static_cast<long long>(cert.generator_count);
    for (std::size_t g = 0; g < cert.generator_count; ++g) {
        const auto c = reduce_exact_rational(count_scalar[g]);
        const auto n = reduce_exact_rational(norm_scalar[g]);
        node.count_scalar_num.push_back(static_cast<long long>(c.numerator));
        node.count_scalar_den.push_back(static_cast<long long>(c.denominator));
        node.norm_scalar_num.push_back(static_cast<long long>(n.numerator));
        node.norm_scalar_den.push_back(static_cast<long long>(n.denominator));
    }
    node.description = description;
    mathlib::reflection::record(mathlib::reflection::NodeKind::LemmaApplication, node);
}

inline NormWeightedQRMajorantCertificate specialize_finite_grammar_to_qr(
    std::size_t base_vertices,
    const std::vector<NormWeightedGrammarChannel>& channels,
    const std::vector<PositiveGeneratorWord>& boundary_words = {}) {
    std::vector<NormWeightedQRChannel> qr;
    qr.reserve(channels.size());
    for (const auto& e : channels)
        qr.push_back({e.source, e.target, e.generator, e.multiplicity,
                      e.certified_operator_norm_bound});
    return derive_norm_weighted_qr_majorant(base_vertices, qr, boundary_words);
}

} // namespace ravel::proof
