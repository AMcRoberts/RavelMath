#pragma once
#include <cstdint>
#include <stdexcept>
#include <string>
#include <vector>

#include "math/proof_reflection.hpp"

namespace ravel::proof {

using IntegerVector = std::vector<std::int64_t>;
using IntegerMatrix = std::vector<IntegerVector>;

inline IntegerVector mat_vec(const IntegerMatrix& a, const IntegerVector& x) {
    if (a.empty()) return {};
    if (a.front().size() != x.size()) throw std::invalid_argument("dimension mismatch");
    IntegerVector out(a.size(), 0);
    for (std::size_t i=0;i<a.size();++i) {
        if (a[i].size()!=x.size()) throw std::invalid_argument("ragged matrix");
        for (std::size_t j=0;j<x.size();++j) out[i]+=a[i][j]*x[j];
    }
    return out;
}

inline IntegerVector add(const IntegerVector& x, const IntegerVector& y) {
    if (x.size()!=y.size()) throw std::invalid_argument("dimension mismatch");
    IntegerVector z(x.size());
    for (std::size_t i=0;i<x.size();++i) z[i]=x[i]+y[i];
    return z;
}

inline IntegerVector sub(const IntegerVector& x, const IntegerVector& y) {
    if (x.size()!=y.size()) throw std::invalid_argument("dimension mismatch");
    IntegerVector z(x.size());
    for (std::size_t i=0;i<x.size();++i) z[i]=x[i]-y[i];
    return z;
}

struct TranslationDefectCertificate {
    IntegerVector translated_output;
    IntegerVector base_plus_linear_translation;
    IntegerVector same_translation_defect;
    bool affine_transport_exact = false;
};

inline TranslationDefectCertificate certify_translation_defect(
    const IntegerMatrix& block,
    const IntegerVector& state,
    const IntegerVector& translation,
    const IntegerVector& forcing) {
    const auto base = add(mat_vec(block,state),forcing);
    const auto translated = add(mat_vec(block,add(state,translation)),forcing);
    const auto linear_translation = mat_vec(block,translation);
    const auto expected = add(base,linear_translation);
    const auto same_translation = add(base,translation);
    return {translated, expected, sub(translated,same_translation), translated==expected};
}

inline void stage_translation_defect(
    const IntegerMatrix& block, const IntegerVector& state,
    const IntegerVector& translation, const IntegerVector& forcing,
    const std::string& description) {
    const auto cert = certify_translation_defect(block, state, translation, forcing);
    if (!cert.affine_transport_exact) return;
    if (!mathlib::reflection::enabled()) return;
    const auto to_ll = [](const IntegerVector& v) {
        return std::vector<long long>(v.begin(), v.end());
    };
    mathlib::reflection::RadialTranslationDefectCertificate node;
    for (const auto& row : block) node.block.push_back(to_ll(row));
    node.state = to_ll(state);
    node.translation = to_ll(translation);
    node.forcing = to_ll(forcing);
    node.same_translation_defect = to_ll(cert.same_translation_defect);
    node.description = description;
    mathlib::reflection::record(mathlib::reflection::NodeKind::LemmaApplication, node);
}

} // namespace ravel::proof
