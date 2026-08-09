// ravel/proof/sturm_chain_certificate.hpp
//
// Finding 30's exact classifier-to-Lean bridge. This operation accepts a
// monic integer polynomial, runs the real `pisot_classify_degree_n` path,
// independently reconstructs its ordinary Euclidean Sturm chain over Q,
// verifies every recurrence and the classifier's isolating bracket, and only
// then records the complete typed certificate.

#pragma once

#include <cstddef>
#include <string>
#include <utility>
#include <vector>

#include "exact_pisot.h"
#include "math/poly_q.hpp"
#include "math/proof_reflection.hpp"

namespace ravel::proof {

enum class SturmChainStageResult {
    staged,
    reflection_disabled,
    invalid_degree,
    classifier_failure,
    not_pisot,
    nonsquarefree,
    invalid_bracket,
    invariant_failure,
};

inline bool polyq_equal(const mathlib::PolyQ& a, const mathlib::PolyQ& b) {
    const long long da = a.degree();
    const long long db = b.degree();
    if (da != db) return false;
    if (da < 0) return true;
    for (long long i = 0; i <= da; ++i) {
        if (mathlib::cmp(a.coeff(static_cast<std::size_t>(i)),
                         b.coeff(static_cast<std::size_t>(i))) != 0) return false;
    }
    return true;
}

inline mathlib::PolyQ polyq_derivative(const mathlib::PolyQ& p) {
    mathlib::PolyQ d;
    if (p.degree() < 1) return d;
    d.ensure_size(static_cast<std::size_t>(p.degree()));
    for (long long i = 1; i <= p.degree(); ++i) {
        mathlib::Rat k(i);
        mathlib::mul(d.coeff(static_cast<std::size_t>(i - 1)),
                     p.coeff(static_cast<std::size_t>(i)), k);
    }
    d.trim();
    return d;
}

inline mathlib::reflection::ExactRationalCoefficient rational_data(const mathlib::Rat& x) {
    const mathlib::BigInt n = mathlib::num(x);
    const mathlib::BigInt d = mathlib::den(x);
    return {mathlib::str(n), mathlib::str(d)};
}

inline mathlib::reflection::ExactRationalPolynomial polynomial_data(const mathlib::PolyQ& p) {
    mathlib::reflection::ExactRationalPolynomial out;
    if (p.degree() < 0) {
        out.push_back({"0", "1"});
        return out;
    }
    out.reserve(static_cast<std::size_t>(p.degree() + 1));
    for (long long i = 0; i <= p.degree(); ++i) {
        out.push_back(rational_data(p.coeff(static_cast<std::size_t>(i))));
    }
    return out;
}

inline mathlib::Rat rational_from_mpz(mpz_srcptr numerator, mpz_srcptr denominator) {
    mathlib::Rat out;
    mpz_set(mpq_numref(out.get()), numerator);
    mpz_set(mpq_denref(out.get()), denominator);
    mpq_canonicalize(out.get());
    return out;
}

inline int polyq_sign_variations(const std::vector<mathlib::PolyQ>& chain,
                                 const mathlib::Rat& x,
                                 bool* endpoint_is_regular = nullptr,
                                 std::vector<long long>* signs_out = nullptr) {
    int variations = 0;
    int previous = 0;
    bool regular = true;
    if (signs_out != nullptr) signs_out->clear();
    for (const auto& p : chain) {
        const mathlib::Rat value = mathlib::eval(p, x);
        const int sign = mathlib::sgn(value);
        if (signs_out != nullptr) signs_out->push_back(sign);
        if (sign == 0) {
            regular = false;
            continue;
        }
        if (previous != 0 && sign != previous) ++variations;
        previous = sign;
    }
    if (endpoint_is_regular != nullptr) *endpoint_is_regular = regular;
    return variations;
}

inline SturmChainStageResult stage_sturm_chain_certificate(
    const std::vector<long long>& monic_coefficients_without_leading,
    std::string description = {}) {
    const int degree = static_cast<int>(monic_coefficients_without_leading.size());
    if (degree < 1 || degree > PISOT_MAX_POLY_DEGREE) {
        return SturmChainStageResult::invalid_degree;
    }

    pisot_info_t info;
    const int classified = pisot_classify_degree_n(
        monic_coefficients_without_leading.data(), degree, &info);
    if (!classified) {
        pisot_info_clear(&info);
        return SturmChainStageResult::classifier_failure;
    }
    if (!info.is_pisot) {
        pisot_info_clear(&info);
        return SturmChainStageResult::not_pisot;
    }

    const mathlib::Rat bracket_lo = rational_from_mpz(info.beta_lo_num, info.beta_lo_den);
    const mathlib::Rat bracket_hi = rational_from_mpz(info.beta_hi_num, info.beta_hi_den);
    const long long classifier_real_inside = info.n_real_inside;
    pisot_info_clear(&info);

    mathlib::PolyQ p;
    p.ensure_size(static_cast<std::size_t>(degree + 1));
    for (int i = 0; i < degree; ++i) {
        mathlib::set_si(p.coeff(static_cast<std::size_t>(i)),
                        monic_coefficients_without_leading[static_cast<std::size_t>(i)], 1);
    }
    mathlib::set_si(p.coeff(static_cast<std::size_t>(degree)), 1, 1);
    p.trim();
    const mathlib::PolyQ derivative = polyq_derivative(p);

    std::vector<mathlib::PolyQ> chain{p, derivative};
    std::vector<mathlib::PolyQ> quotients;
    std::vector<mathlib::PolyQ> bezout_u{mathlib::PolyQ(1), mathlib::PolyQ(0)};
    std::vector<mathlib::PolyQ> bezout_v{mathlib::PolyQ(0), mathlib::PolyQ(1)};

    while (chain.back().degree() > 0) {
        const std::size_t n = chain.size();
        const mathlib::DivModResultQ division = mathlib::divmod(chain[n - 2], chain[n - 1]);
        const mathlib::PolyQ next = -division.r;
        if (next.is_zero()) return SturmChainStageResult::nonsquarefree;
        if (!polyq_equal(chain[n - 2], division.q * chain[n - 1] - next)) {
            return SturmChainStageResult::invariant_failure;
        }
        const mathlib::PolyQ next_u = division.q * bezout_u[n - 1] - bezout_u[n - 2];
        const mathlib::PolyQ next_v = division.q * bezout_v[n - 1] - bezout_v[n - 2];
        quotients.push_back(division.q);
        chain.push_back(next);
        bezout_u.push_back(next_u);
        bezout_v.push_back(next_v);
    }
    if (chain.back().degree() != 0 || mathlib::sgn(chain.back().coeff(0)) == 0) {
        return SturmChainStageResult::nonsquarefree;
    }
    const mathlib::PolyQ bezout_check = bezout_u.back() * p + bezout_v.back() * derivative;
    if (!polyq_equal(bezout_check, chain.back())) {
        return SturmChainStageResult::invariant_failure;
    }

    bool lo_regular = false;
    bool hi_regular = false;
    std::vector<long long> signs_lo, signs_hi;
    const int variations_lo = polyq_sign_variations(chain, bracket_lo, &lo_regular, &signs_lo);
    const int variations_hi = polyq_sign_variations(chain, bracket_hi, &hi_regular, &signs_hi);
    const long long root_count = static_cast<long long>(variations_lo - variations_hi);
    if (!lo_regular || !hi_regular || mathlib::cmp(bracket_lo, bracket_hi) >= 0 || root_count != 1) {
        return SturmChainStageResult::invalid_bracket;
    }
    if (!mathlib::reflection::enabled()) return SturmChainStageResult::reflection_disabled;

    mathlib::reflection::SturmChainCertificate node;
    node.polynomial = polynomial_data(p);
    for (const auto& item : chain) node.chain.push_back(polynomial_data(item));
    for (const auto& quotient : quotients) node.quotients.push_back(polynomial_data(quotient));
    node.positive_scales.assign(quotients.size(), {"1", "1"});
    node.bezout_u = polynomial_data(bezout_u.back());
    node.bezout_v = polynomial_data(bezout_v.back());
    node.bezout_constant = rational_data(chain.back().coeff(0));
    node.bracket_lo = rational_data(bracket_lo);
    node.bracket_hi = rational_data(bracket_hi);
    node.variations_lo = variations_lo;
    node.variations_hi = variations_hi;
    node.signs_lo = std::move(signs_lo);
    node.signs_hi = std::move(signs_hi);
    node.root_count = root_count;
    node.classifier_is_pisot = true;
    node.classifier_real_inside = classifier_real_inside;
    node.description = std::move(description);
    mathlib::reflection::record(mathlib::reflection::NodeKind::LemmaApplication, std::move(node));
    return SturmChainStageResult::staged;
}

}  // namespace ravel::proof
