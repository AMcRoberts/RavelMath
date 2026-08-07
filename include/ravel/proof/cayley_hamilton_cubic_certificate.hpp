// ravel/proof/cayley_hamilton_cubic_certificate.hpp
//
// Finding 23 (single_junction_coincidence_composition.hpp) relies on
// the observation that sigma_{0,1}'s incidence matrix M satisfies
// M^3 = M + I exactly -- the Cayley-Hamilton-style relation coming
// from its minimal polynomial x^3 - x - 1 (the plastic number) having
// coefficients in {-1,0,1}. That claim was previously only checked by
// hand/prose; this stages the exact integer-arithmetic check and, if
// it passes, records the CONCRETE matrix so the reflection pipeline
// can independently re-derive M^3 inside Lean and `decide` the
// identity -- not merely restate the same nine numbers.

#pragma once

#include <array>
#include <string>
#include <vector>

#include "math/proof_reflection.hpp"

namespace ravel::proof {

using Mat3 = std::array<std::array<long long, 3>, 3>;

inline Mat3 mat3_mul(const Mat3& A, const Mat3& B) {
    Mat3 C{};
    for (int i = 0; i < 3; ++i)
        for (int j = 0; j < 3; ++j) {
            long long s = 0;
            for (int k = 0; k < 3; ++k) s += A[static_cast<std::size_t>(i)][static_cast<std::size_t>(k)]
                                             * B[static_cast<std::size_t>(k)][static_cast<std::size_t>(j)];
            C[static_cast<std::size_t>(i)][static_cast<std::size_t>(j)] = s;
        }
    return C;
}

// The incidence matrix of a 3-letter substitution: M[i][j] = count of
// letter i in the image of letter j.
inline Mat3 incidence_matrix_3(const std::array<std::vector<long long>, 3>& images) {
    Mat3 M{};
    for (std::size_t j = 0; j < 3; ++j)
        for (auto letter : images[j])
            M[static_cast<std::size_t>(letter)][j] += 1;
    return M;
}

// Stages a check of M^3 = M + I for a concrete 3x3 integer matrix;
// records nothing unless the exact integer-arithmetic check passes.
inline void stage_cayley_hamilton_cubic(const Mat3& M, const std::string& description) {
    const Mat3 M2 = mat3_mul(M, M);
    const Mat3 M3 = mat3_mul(M2, M);
    Mat3 m_plus_i = M;
    for (int i = 0; i < 3; ++i) m_plus_i[static_cast<std::size_t>(i)][static_cast<std::size_t>(i)] += 1;
    if (M3 != m_plus_i) return;
    if (!mathlib::reflection::enabled()) return;
    mathlib::reflection::CayleyHamiltonCubicCertificate node;
    node.description = description;
    std::size_t idx = 0;
    for (int i = 0; i < 3; ++i)
        for (int j = 0; j < 3; ++j)
            node.matrix[idx++] = M[static_cast<std::size_t>(i)][static_cast<std::size_t>(j)];
    mathlib::reflection::record(mathlib::reflection::NodeKind::LemmaApplication, node);
}

}  // namespace ravel::proof
