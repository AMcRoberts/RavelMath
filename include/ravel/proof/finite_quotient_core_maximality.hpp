#pragma once

#include <algorithm>
#include <cstddef>
#include <string>
#include <utility>
#include <vector>

#include "ravel/proof/graded_transfer.hpp"
#include "ravel/proof/paired_matrix_dominance.hpp"
#include "ravel/proof/path_injective_simulation.hpp"
#include "ravel/proof/higher_block_path_injection.hpp"

namespace ravel::proof {

enum class QuotientDominanceMechanism {
    Core,
    TerminalPermutation,
    PathInjection,
    HigherBlockPathInjection,
    ReversePathInjection,
    RenewalTransfer,
    PairedMatrix,
    Unproved
};

struct QuotientComponentDominance {
    std::size_t index = 0;
    std::size_t vertices = 0;
    QuotientDominanceMechanism mechanism = QuotientDominanceMechanism::Unproved;
    PathInjectiveSimulationCertificate path_injection;
    HigherBlockPathInjection higher_block_injection;
    PathInjectiveSimulationCertificate reverse_path_injection;
    GradedTransferProof renewal_transfer;
    RankOnePairedDominance paired_matrix;
    bool spectral_radius_at_most_core = false;
    bool spectral_radius_strictly_below_core = false;
    std::string obstruction;
};

struct FiniteQuotientCoreMaximality {
    std::size_t components = 0;
    std::size_t terminal_components = 0;
    std::size_t path_injection_components = 0;
    std::size_t higher_block_components = 0;
    std::size_t reverse_path_injection_components = 0;
    std::size_t renewal_components = 0;
    std::size_t paired_matrix_components = 0;
    std::vector<QuotientComponentDominance> evidence;
    bool every_component_bounded = false;
    bool every_noncore_component_strict_or_terminal = false;
    bool core_spectrally_maximal = false;
    std::string obstruction;
};

inline MultiAdjacency matrix_multi_adjacency(
    const std::vector<std::vector<long long>>& matrix) {
    MultiAdjacency result(matrix.size());
    for (std::size_t i = 0; i < matrix.size(); ++i) {
        if (matrix[i].size() != matrix.size())
            throw std::invalid_argument("finite quotient maximality: nonsquare matrix");
        for (std::size_t j = 0; j < matrix.size(); ++j) {
            if (matrix[i][j] < 0)
                throw std::invalid_argument("finite quotient maximality: negative entry");
            for (long long k = 0; k < matrix[i][j]; ++k)
                result[i].push_back(j);
        }
    }
    return result;
}

inline bool matrix_is_terminal_permutation(
    const std::vector<std::vector<long long>>& matrix) {
    if (matrix.empty()) return false;
    std::vector<long long> incoming(matrix.size(), 0);
    for (std::size_t i = 0; i < matrix.size(); ++i) {
        if (matrix[i].size() != matrix.size()) return false;
        long long outgoing = 0;
        for (std::size_t j = 0; j < matrix.size(); ++j) {
            if (matrix[i][j] < 0) return false;
            outgoing += matrix[i][j];
            incoming[j] += matrix[i][j];
        }
        if (outgoing != 1) return false;
    }
    return std::all_of(incoming.begin(), incoming.end(),
                       [](long long value) { return value == 1; });
}

/** Close a finite Condition-F quotient catalogue against its selected core.
 *
 * Each non-core component is proved by the strongest reusable mechanism that
 * succeeds without accepting a Boolean certificate:
 *
 *  1. exact terminal permutation recognition (rho = 1);
 *  2. greatest multiplicity-preserving path injection into the core;
 *  3. exact renewal/cone transfer;
 *  4. exact implicit rank-one paired-matrix intertwiner.
 *
 * The final paired-matrix branch is deliberately last: structural descent is
 * retained whenever available, while no finite SCC is left justified only by
 * a floating Perron estimate.
 */
inline FiniteQuotientCoreMaximality derive_finite_quotient_core_maximality(
    const std::vector<std::vector<long long>>& core,
    const std::vector<std::vector<std::vector<long long>>>& competitors,
    std::size_t horizon = 160) {
    FiniteQuotientCoreMaximality out;
    if (core.empty()) {
        out.obstruction = "finite quotient maximality requires a nonempty core";
        return out;
    }
    const auto core_adjacency = matrix_multi_adjacency(core);
    out.components = competitors.size() + 1;

    QuotientComponentDominance core_evidence;
    core_evidence.index = 0;
    core_evidence.vertices = core.size();
    core_evidence.mechanism = QuotientDominanceMechanism::Core;
    core_evidence.spectral_radius_at_most_core = true;
    out.evidence.push_back(std::move(core_evidence));

    bool all_bounded = true;
    bool all_strict_or_terminal = true;
    for (std::size_t k = 0; k < competitors.size(); ++k) {
        const auto& matrix = competitors[k];
        QuotientComponentDominance e;
        e.index = k + 1;
        e.vertices = matrix.size();
        if (matrix.empty()) {
            e.obstruction = "empty competitor component";
            all_bounded = all_strict_or_terminal = false;
            out.evidence.push_back(std::move(e));
            continue;
        }

        if (matrix_is_terminal_permutation(matrix)) {
            e.mechanism = QuotientDominanceMechanism::TerminalPermutation;
            e.spectral_radius_at_most_core = true;
            e.spectral_radius_strictly_below_core = false;
            ++out.terminal_components;
            out.evidence.push_back(std::move(e));
            continue;
        }

        const auto adjacency = matrix_multi_adjacency(matrix);
        e.path_injection = derive_path_injective_simulation(
            adjacency, core_adjacency);
        if (e.path_injection.spectral_radius_nonstrict) {
            e.mechanism = QuotientDominanceMechanism::PathInjection;
            e.spectral_radius_at_most_core = true;
            ++out.path_injection_components;
            out.evidence.push_back(std::move(e));
            continue;
        }

        const auto reverse_adjacency = matrix_multi_adjacency(
            transpose_nonnegative_matrix(matrix));
        const auto reverse_core_adjacency = matrix_multi_adjacency(
            transpose_nonnegative_matrix(core));
        e.reverse_path_injection = derive_path_injective_simulation(
            reverse_adjacency, reverse_core_adjacency);
        if (e.reverse_path_injection.spectral_radius_nonstrict) {
            e.mechanism = QuotientDominanceMechanism::ReversePathInjection;
            e.spectral_radius_at_most_core = true;
            ++out.reverse_path_injection_components;
            out.evidence.push_back(std::move(e));
            continue;
        }

        e.higher_block_injection = derive_synchronous_higher_block_path_injection(
            adjacency, core_adjacency, 4);
        if (e.higher_block_injection.spectral_radius_nonstrict) {
            e.mechanism = QuotientDominanceMechanism::HigherBlockPathInjection;
            e.spectral_radius_at_most_core = true;
            ++out.higher_block_components;
            out.evidence.push_back(std::move(e));
            continue;
        }

        e.renewal_transfer = derive_graded_transfer_proof(matrix, core, horizon);
        if (e.renewal_transfer.proved) {
            e.mechanism = QuotientDominanceMechanism::RenewalTransfer;
            e.spectral_radius_at_most_core = true;
            e.spectral_radius_strictly_below_core = true;
            ++out.renewal_components;
            out.evidence.push_back(std::move(e));
            continue;
        }

        e.paired_matrix = derive_rank_one_paired_dominance(matrix, core, horizon);
        if (e.paired_matrix.replayed) {
            e.mechanism = QuotientDominanceMechanism::PairedMatrix;
            e.spectral_radius_at_most_core = true;
            e.spectral_radius_strictly_below_core = true;
            ++out.paired_matrix_components;
            out.evidence.push_back(std::move(e));
            continue;
        }

        e.obstruction = "no terminal, path-injective, renewal, or paired-matrix proof";
        all_bounded = false;
        all_strict_or_terminal = false;
        out.evidence.push_back(std::move(e));
    }

    out.every_component_bounded = all_bounded;
    out.every_noncore_component_strict_or_terminal = all_strict_or_terminal;
    out.core_spectrally_maximal = all_bounded;
    if (!out.core_spectrally_maximal)
        out.obstruction = "at least one finite quotient SCC lacks exact dominance evidence";
    return out;
}

inline const char* quotient_dominance_mechanism_name(
    QuotientDominanceMechanism mechanism) {
    switch (mechanism) {
        case QuotientDominanceMechanism::Core: return "core";
        case QuotientDominanceMechanism::TerminalPermutation: return "terminal-permutation";
        case QuotientDominanceMechanism::PathInjection: return "path-injection";
        case QuotientDominanceMechanism::HigherBlockPathInjection: return "higher-block-path-injection";
        case QuotientDominanceMechanism::ReversePathInjection: return "reverse-path-injection";
        case QuotientDominanceMechanism::RenewalTransfer: return "renewal-transfer";
        case QuotientDominanceMechanism::PairedMatrix: return "paired-matrix";
        default: return "unproved";
    }
}

} // namespace ravel::proof
