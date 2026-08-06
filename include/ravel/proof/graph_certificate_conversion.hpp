#pragma once

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#include "ravel/proof/graph_structural_campaign.hpp"

namespace ravel::proof::graph_conversion {

using graph_structural::AdjacencyMatrixCertificate;
using graph_structural::FeederCycleBlockCertificate;

struct ReindexBijectionCertificate {
    std::string graph_id;
    std::vector<std::size_t> new_to_old;
    std::vector<std::size_t> old_to_new;
    bool covers_all_vertices = false;
    bool injective = false;
    bool inverse_laws = false;
    bool valid = false;
};

struct ReindexedBlockEqualityCertificate {
    std::string graph_id;
    std::vector<std::vector<std::int64_t>> reindexed_matrix;
    std::vector<std::vector<std::int64_t>> expected_block_matrix;
    bool dimensions_match = false;
    bool entrywise_equal = false;
    bool valid = false;
};

struct GraphToCharpolyConversionArtifact {
    std::string graph_id;
    ReindexBijectionCertificate reindex;
    ReindexedBlockEqualityCertificate block_equality;
    std::string lean_premise_shape;
    std::string lean_conclusion_shape;
    bool premise_produced = false;
};

inline ReindexBijectionCertificate certify_reindex_bijection(
    const std::string& graph_id,
    std::size_t vertex_count,
    const std::vector<std::size_t>& new_to_old) {
    ReindexBijectionCertificate cert;
    cert.graph_id = graph_id;
    cert.new_to_old = new_to_old;
    cert.old_to_new.assign(vertex_count, vertex_count);
    cert.covers_all_vertices = new_to_old.size() == vertex_count;
    cert.injective = cert.covers_all_vertices;

    if (cert.covers_all_vertices) {
        std::vector<bool> seen(vertex_count, false);
        for (std::size_t new_index = 0; new_index < vertex_count; ++new_index) {
            const auto old_index = new_to_old[new_index];
            if (old_index >= vertex_count || seen[old_index]) {
                cert.injective = false;
                continue;
            }
            seen[old_index] = true;
            cert.old_to_new[old_index] = new_index;
        }
        cert.covers_all_vertices =
            std::all_of(seen.begin(), seen.end(), [](bool x) { return x; });
    }

    cert.inverse_laws = cert.covers_all_vertices && cert.injective;
    if (cert.inverse_laws) {
        for (std::size_t i = 0; i < vertex_count; ++i) {
            cert.inverse_laws &= cert.old_to_new[cert.new_to_old[i]] == i;
            cert.inverse_laws &= cert.new_to_old[cert.old_to_new[i]] == i;
        }
    }
    cert.valid = cert.covers_all_vertices && cert.injective && cert.inverse_laws;
    return cert;
}

inline std::vector<std::vector<std::int64_t>> reindex_square_matrix(
    const std::vector<std::vector<std::int64_t>>& matrix,
    const ReindexBijectionCertificate& reindex) {
    if (!reindex.valid) throw std::logic_error("invalid reindex certificate");
    const auto n = reindex.new_to_old.size();
    if (matrix.size() != n) throw std::logic_error("matrix/reindex dimension mismatch");
    std::vector<std::vector<std::int64_t>> result(
        n, std::vector<std::int64_t>(n, 0));
    for (std::size_t i = 0; i < n; ++i) {
        if (matrix[reindex.new_to_old[i]].size() != n)
            throw std::logic_error("non-square matrix evidence");
        for (std::size_t j = 0; j < n; ++j)
            result[i][j] = matrix[reindex.new_to_old[i]][reindex.new_to_old[j]];
    }
    return result;
}

inline std::vector<std::vector<std::int64_t>> expected_feeder_cycle_block(
    const FeederCycleBlockCertificate& block) {
    const auto core_size = block.core_matrix.size();
    std::vector<std::vector<std::int64_t>> expected(
        core_size + 1, std::vector<std::int64_t>(core_size + 1, 0));
    for (std::size_t j = 0; j < core_size; ++j)
        expected[0][j + 1] = block.feeder_to_core[j];
    for (std::size_t i = 0; i < core_size; ++i)
        for (std::size_t j = 0; j < core_size; ++j)
            expected[i + 1][j + 1] = block.core_matrix[i][j];
    return expected;
}

inline ReindexedBlockEqualityCertificate certify_reindexed_block_equality(
    const AdjacencyMatrixCertificate& adjacency,
    const FeederCycleBlockCertificate& block,
    const ReindexBijectionCertificate& reindex) {
    ReindexedBlockEqualityCertificate cert;
    cert.graph_id = adjacency.graph_id;
    cert.reindexed_matrix = reindex_square_matrix(adjacency.entries, reindex);
    cert.expected_block_matrix = expected_feeder_cycle_block(block);
    cert.dimensions_match =
        cert.reindexed_matrix.size() == cert.expected_block_matrix.size();
    cert.entrywise_equal = cert.dimensions_match &&
        cert.reindexed_matrix == cert.expected_block_matrix;
    cert.valid = adjacency.exact && block.block_form_exact &&
                 reindex.valid && cert.entrywise_equal;
    return cert;
}

inline GraphToCharpolyConversionArtifact convert_graph_certificate_to_charpoly_premise(
    const AdjacencyMatrixCertificate& adjacency,
    const FeederCycleBlockCertificate& block) {
    GraphToCharpolyConversionArtifact artifact;
    artifact.graph_id = adjacency.graph_id;
    artifact.reindex = certify_reindex_bijection(
        adjacency.graph_id, adjacency.entries.size(), block.reindex_order);
    artifact.block_equality = certify_reindexed_block_equality(
        adjacency, block, artifact.reindex);
    artifact.lean_premise_shape =
        "Matrix.reindex e e adjacency = Matrix.fromBlocks 0 feeder 0 core";
    artifact.lean_conclusion_shape =
        "adjacency.charpoly = Polynomial.X * core.charpoly";
    artifact.premise_produced = artifact.reindex.valid && artifact.block_equality.valid;
    return artifact;
}

inline std::string render_lean_graph_charpoly_conversion_module() {
    return R"LEAN(import Mathlib.Tactic
import Mathlib.LinearAlgebra.Matrix.Charpoly.Basic
import Mathlib.LinearAlgebra.Matrix.Reindex

namespace RavelGenerated

open Matrix Polynomial

/-- Characteristic polynomials are transported across a certificate-produced
simultaneous row/column reindexing. -/
theorem charpoly_transport_of_reindex_eq
    {R : Type*} [CommRing R]
    {ι κ : Type*}
    [Fintype ι] [DecidableEq ι]
    [Fintype κ] [DecidableEq κ]
    (e : ι ≃ κ)
    (A : Matrix ι ι R)
    (B : Matrix κ κ R)
    (htransport : Matrix.reindex e e A = B) :
    A.charpoly = B.charpoly := by
  rw [← Matrix.charpoly_reindex e A, htransport]

/-- A graph certificate that reindexes an adjacency matrix into a feeder/core
block form produces the spectral factor `X * core.charpoly`. -/
theorem charpoly_factor_of_reindex_feeder_core
    {R : Type*} [CommRing R]
    {ι κ : Type*}
    [Fintype ι] [DecidableEq ι]
    [Fintype κ] [DecidableEq κ]
    (e : ι ≃ (Fin 1 ⊕ κ))
    (A : Matrix ι ι R)
    (feederToCore : Matrix (Fin 1) κ R)
    (core : Matrix κ κ R)
    (htransport :
      Matrix.reindex e e A =
        Matrix.fromBlocks
          (0 : Matrix (Fin 1) (Fin 1) R)
          feederToCore
          (0 : Matrix κ (Fin 1) R)
          core) :
    A.charpoly = Polynomial.X * core.charpoly := by
  rw [charpoly_transport_of_reindex_eq e A _ htransport]
  rw [Matrix.charpoly_fromBlocks_zero₂₁]
  simp

/-- Composition with a closed cycle-core identity. -/
theorem charpoly_closed_of_reindex_cycle_core
    {R : Type*} [CommRing R]
    {ι κ : Type*}
    [Fintype ι] [DecidableEq ι]
    [Fintype κ] [DecidableEq κ]
    (n : ℕ)
    (e : ι ≃ (Fin 1 ⊕ κ))
    (A : Matrix ι ι R)
    (feederToCore : Matrix (Fin 1) κ R)
    (core : Matrix κ κ R)
    (htransport :
      Matrix.reindex e e A =
        Matrix.fromBlocks
          (0 : Matrix (Fin 1) (Fin 1) R)
          feederToCore
          (0 : Matrix κ (Fin 1) R)
          core)
    (hcore : core.charpoly = Polynomial.X ^ n - 1) :
    A.charpoly = Polynomial.X * (Polynomial.X ^ n - 1) := by
  rw [charpoly_factor_of_reindex_feeder_core e A feederToCore core htransport]
  rw [hcore]

end RavelGenerated
)LEAN";
}

} // namespace ravel::proof::graph_conversion
